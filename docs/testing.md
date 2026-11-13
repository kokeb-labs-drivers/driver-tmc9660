# Testing and CI

How the driver is checked: host tests against an independent model of the chip, safety tests that assert on the
chip, a fuzzer that corrupts what the chip sends back, one-edit mutants of every guard, generated and formatted
tables, compiled documentation, an install test, ESP32 and Cortex-M builds, static analysis and measured quality
gates, on every pull request. Nothing here has run on a real chip.

## Host tests

The tests build the real driver against a model of the TMC9660: the bootloader with its configuration memory and
flash, parameter mode over SPI and UART, register mode over SPI and UART, resets, and injected faults. They build with
the firmware's flags, `-fno-exceptions -fno-rtti`, and warnings as errors, on GCC and Clang.

```bash
cmake -S . -B build -DTMC9660_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The unit tests are 206 cases in one executable (`tmc9660_host_tests`); the other CTest entries are listed below.

| File | Covers |
| --- | --- |
| [`tests/test_frames.cpp`](../tests/test_frames.cpp) | Every frame encoder and decoder against the documents' worked examples, and the CRC-8 |
| [`tests/test_bootloader.cpp`](../tests/test_bootloader.cpp) | Bootloader over SPI and UART: info, memory, configuration write, start, erase, delayed replies, PLL |
| [`tests/test_parameter.cpp`](../tests/test_parameter.cpp) | Parameter mode: ranges, access, coded values, globals, scripts, resets on SPI and UART |
| [`tests/test_register.cpp`](../tests/test_register.cpp) | Register mode: access, masks, the energising guard, `GateDriverStart()`, the datagram counter |
| [`tests/test_safety.cpp`](../tests/test_safety.cpp) | The rules of [Safety](safety.md), each asserted on the chip's record of what happened to it |
| [`tests/test_gate_driver.cpp`](../tests/test_gate_driver.cpp) | `GateDriverStart()`: the latched-evidence masks bit by bit, CHOP 1 before the enable, the bootstrap wait, a trip after the protections, every failure ends switched off |
| [`tests/test_hardening.cpp`](../tests/test_hardening.cpp) | Reply validation, scripts that set `COMMUTATION_MODE`, `WATCHDOG_EVENT`, bootloader SPI `BUSY` and dead lines |
| [`tests/test_stop_paths.cpp`](../tests/test_stop_paths.cpp) | The stop after an unseen reset on SPI and UART, parameter and register mode; the read-back; the buffers |
| [`tests/test_idle_pwm.cpp`](../tests/test_idle_pwm.cpp) | `IDLE_MOTOR_PWM_BEHAVIOR` and the stop, a stop sent before the UART resync, the dirty flag over a restart, the repeat of a failed register-stop write |
| [`tests/test_start_edges.cpp`](../tests/test_start_edges.cpp) | What the start write returns unchanged, a parameter start failing inside the follow, the UART bank and address before the start write, the bounded identity wait, the recovery stop after an unseen restart into an application |
| [`tests/test_start_follow.cpp`](../tests/test_start_follow.cpp) | A start of the application whose write fails or comes late is followed, and a boot-mode write with `START_MOTOR_CTRL` already set counts as a start (A24) |
| [`tests/test_restart_mode.cpp`](../tests/test_restart_mode.cpp) | A failed restart keeps the proved protocol so the stop still reaches the chip; no proved protocol and no DRV_EN hook is not a silent `Ok`; register read-back, pin and probe guards |
| [`tests/test_stop_repeat.cpp`](../tests/test_stop_repeat.cpp) | The parameter-mode stop and `MotorStop()` repeated after any failure, a failed `EnableDrive()` that stops, the register stop not gated by FAULTN, a bootloader read after a reset, a failed `ReturnToBootloader()` |
| [`tests/test_resync.cpp`](../tests/test_resync.cpp) | A late UART reply never answers the next request |
| [`tests/test_layout.cpp`](../tests/test_layout.cpp) | The TMCL layout probe (A2) |
| [`tests/test_survivors.cpp`](../tests/test_survivors.cpp) | Bootloader SPI commands run once after a probe, every place that marks a UART line dirty, two stale replies, the reset pin, the reply retry, the boot-mode read-back, `PowerDown()`, the register baseline |
| [`tests/test_errors.cpp`](../tests/test_errors.cpp) | The base `Error` values are 0 to 9; every chip-specific value is returned by some call |
| [`tests/test_version.cpp`](../tests/test_version.cpp) | The version getters follow the generated header (also against a 9.8.7 probe) |
| [`tests/test_kconfig.cpp`](../tests/test_kconfig.cpp) | `KconfigDefaults()` with and without `CONFIG_` values |
| [`tests/compile_fail/`](../tests/compile_fail) | 14 files that must not compile, each for its `EXPECT:` diagnostic: a port on both buses, a port without `DelayUs()`, a hook with the wrong type or without `noexcept`, an invalid `CheckedConfig`, a class that is not a port |
| [`tests/mock_tmc9660.hpp`](../tests/mock_tmc9660.hpp) | The chip model; the bus framing is in `mock_tmc9660_bus.hpp` |
| [`tests/docs/`](../tests/docs/my_port.hpp) | The snippets of these pages, compiled |
| [`examples/host/`](../examples/host/README.md) | The host example, built with the test flags and run (`tmc9660_host_example`, expects `done: 0 errors`) |
| [`tests/check_install.cmake`](../tests/check_install.cmake) | Installs into a scratch prefix and builds [`tests/install_consumer/`](../tests/install_consumer/CMakeLists.txt) with `find_package(tmc9660)` |
| [`tests/check_doxygen.cmake`](../tests/check_doxygen.cmake) | Doxygen with `_config/Doxyfile` and warnings as errors, when `doxygen` is installed |

### The model stands alone

`tests/mock_tmc9660.hpp` is written from the documents, not from the driver. It includes the generated parameter and
register tables only as data of the chip and has its own implementation of every protocol: the bootloader state
machine with the delayed SPI reply, TMCL with `FIRST_CMD`, register mode with the boot bit and the datagram counter,
the PLL, the flash, the flags. A wrong constant in the driver cannot hide behind the same wrong constant in the model
(handbook KB-TST-030). Every behaviour found on hardware becomes a fault switch of the model and a test that fails on
the code before the fix.

The model records what happened to the chip, and the safety tests assert on that record, not on the driver's own return
values: writes since the last reset, energise events with an error flag latched or DRV_EN low, `Boot` with the drive on,
reserved-mode starts, OTP and configuration writes, all-zero frames.

## Fuzz tests

Everything the driver reads comes from the chip over the bus. [`tests/fuzz/fuzz_tmc9660.cpp`](../tests/fuzz/fuzz_tmc9660.cpp)
lets libFuzzer choose a mode, a bus and a configuration, then up to 64 driver calls, and on every bus transfer what the
bus does: pass it to the model, fail it, flip bits or replace what the chip sent. Between calls it can reset the chip,
raise a fault or change the chip identity. After every call it checks the driver's promises and aborts, naming the
line, when one breaks:

- a failed call returns an `Error` the enum defines and `T{}`;
- before `EnsureInitialized()` no call makes a transfer, and `InvalidArgument` means nothing was sent;
- the gate driver is never energised with an error flag latched or DRV_EN low, and `DisableDrive()` always sends its
  stop, also after a reported reset;
- a UART reply that arrives late never answers the next request (the model keeps a receive FIFO and the fuzz target
  can delay a reply);
- no state-changing command follows a reset the driver could have seen (`UnexpectedReset` leaves it uninitialised);
  whether it could see it depends on the mode the driver is in at that call (a started application is a fresh start);
  noise that hides the evidence is excluded and documented as residual risk;
- a stop that returns `Ok` in parameter mode leaves the PWM off (or DRV_EN low on a port with the hook), a stop with
  no lost or corrupted transfer returns `Ok`, and a failed `EnableDrive()` with at most one lost transfer leaves the
  drive stopped; the fuzzer also picks ports without `GpioSet()` and lets an application energise by itself;
- every log line is a non-empty plain-text message.

```bash
# libFuzzer needs Clang (on macOS, Homebrew's LLVM)
cmake -S . -B build-fuzz -DCMAKE_CXX_COMPILER=clang++ -DTMC9660_BUILD_TESTS=ON -DTMC9660_BUILD_FUZZERS=ON
cmake --build build-fuzz --target fuzz_tmc9660
./build-fuzz/tests/fuzz/fuzz_tmc9660 -max_total_time=120 tests/fuzz/corpus/fuzz_tmc9660
```

The same file also builds without libFuzzer as `fuzz_replay_fuzz_tmc9660`, and CTest replays every input in
[`tests/fuzz/corpus/fuzz_tmc9660/`](../tests/fuzz/corpus/fuzz_tmc9660) on GCC and Clang
(`tmc9660_fuzz_tmc9660_corpus`): the seeds, one per mode and bus and fault family, and every input that ever
broke a promise (named `regress-*`), so a fixed bug stays fixed. The seeds and 7000 random inputs also ran under
ASan and UBSan. CI fuzzes for 120 s on every pull request (job `Host quality / fuzz`).

## Mutation testing

A test suite that passes proves little unless it fails when the guard it claims to test is removed.
[`tools/mutants/mutants.py`](../tools/mutants/mutants.py) applies 169 one-edit mutants to a scratch copy of `inc/`,
`src/` and `tests/`, one at a time, builds the unit tests and runs them. A mutant is killed when a test fails or a
`static_assert` stops the build; an edit that does not compile is reported as broken and fails the run. The mutants are
the safety guards of [Safety](safety.md) (the flag reads before enabling, the commutation-mode range, the refusals,
the pin-first stop, the reset evidence on every transport, the retry boundaries, the configuration guards, the energising
masks and guards, the datagram counter and boot bit), the argument checks, and the CRC polynomial.

```bash
TMC9660_GENERATED=<dir with tmc9660_version.h> python3 tools/mutants/mutants.py --jobs 4
```

Seven mutants are listed as equivalent in the script with their reason, among them: removing the reserved-boot-mode
check of `BlStartApplication()` (the configuration check refuses the same start), and two probes in front of a stop
(`disable-reg-uart-probe`, `canary-probe-on-stop`: the repeat after a reported reset sends the same stop). The last
full run killed 169 of 169.

## Generated tables and formatting

The parameter table (289 rows) and the register table (163 rows) are generated, not typed:
[`tools/gen_tables/gen_tables.py`](../tools/gen_tables/gen_tables.py) reads `params.json` and `regs.json` (transcribed
from Table 57 of the Parameter Mode Reference Manual and from the Register Mode Reference Manual) and writes
`inc/tmc9660_params.hpp` and `inc/tmc9660_registers.hpp`, with the documents' citations and Doxygen comments.

- The output goes through the repository's clang-format (version 20, checked by the script), then the script runs
  clang-format a second time and fails if the text changes (idempotence) or a line is longer than 100 columns. The
  committed headers contain no `clang-format off` region.
- `gen_tables.py --check` exits 1 when a committed header differs from the generator's output; CTest runs it as
  `tmc9660_generated_tables`, so a hand edit of a generated header or a changed input without regeneration fails
  the build. The tables are laid out by clang-format 20: with another version (the CI runners have 18) or none the
  script prints `SKIPPED` and exits 77, which CTest reports as skipped, not failed; the local gate
  (`tools/preflight.sh`, clang-format 20) runs it as a real check.
- Rows are designated initialisers, namespaces are lower case and type aliases CamelCase, as the repository's
  clang-tidy configuration asks; each header passes clang-tidy without a blanket `NOLINT`.
- The change from the first tables to the generated, formatted ones was checked token by token: the value tokens of
  the two versions are identical; only whitespace and comments differ.

## Quality gates

The `Host quality` job measures the library code, `inc/` and `src/` only (tests and examples are left out), and
fails when a value is over the limits of this repository's assurance level, `high` (set in
[`.kokeb/handbook.yml`](../.kokeb/handbook.yml), defined by the handbook's ADR-0020):

| Gate | Tool and how it is measured | Limit at `high` |
| --- | --- | --- |
| Complexity | lizard 1.24.1, modified cyclomatic complexity (`-m`: a `switch` counts once) per function | CCN at most 15 |
| Function length | lizard, lines of code per function | at most 60 |
| Parameters | lizard, per function | at most 6 |
| Line coverage | GCC 14 `--coverage -O0`, every CTest, gcovr 8.7 `--merge-lines` (a template's lines count once across its instances), `--exclude-unreachable-branches --exclude-throw-branches` | at least 85 % |
| Branch coverage | same run | at least 75 % |
| MC/DC | GCC `-fcondition-coverage`, same run | measured, not gated at `high` (100 % at `safety`) |
| Sanitizers | Clang, `-fsanitize=address,undefined -fno-sanitize-recover=all`, every CTest | no report |
| Fuzzing | libFuzzer with ASan and UBSan, 120 s per target | no broken promise |

The reports are in the run's `host-quality-reports` artifact and its step summary. To run the gates locally:

```bash
# Coverage (GCC 14 or later), then the complexity limits
cmake -S . -B build-cov -DCMAKE_CXX_COMPILER=g++-14 -DTMC9660_BUILD_TESTS=ON \
  -DCMAKE_CXX_FLAGS="--coverage -O0 -fcondition-coverage"
cmake --build build-cov && ctest --test-dir build-cov
gcovr --root . --filter inc/ --filter src/ --merge-lines --exclude-unreachable-branches --txt build-cov
lizard -m -C 15 -L 60 -a 6 inc src

# Sanitizers (Clang)
cmake -S . -B build-san -DCMAKE_CXX_COMPILER=clang++ -DTMC9660_BUILD_TESTS=ON \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=all -O1 -g"
cmake --build build-san && ctest --test-dir build-san --output-on-failure
```

The fuzz gate runs locally as shown in [Fuzz tests](#fuzz-tests).

clang-tidy 20 ([`_config/.clang-tidy`](../_config/.clang-tidy): bugprone, CERT, clang-analyzer,
cppcoreguidelines, modernize, performance, portability and readability, each disabled check with its reason) and
cppcheck (strict) run in the `C++ lint` and `Repository checks` jobs and fail on any finding.

The `MISRA` job ([`ci-misra.yml`](../.github/workflows/ci-misra.yml)) runs the shared `ru-misra` workflow of
ci-tools at level `high`: it traces a host build and the Cortex-M build (`tmc9660_optimised_build`) of every
translation unit, checks them against MISRA C++:2023 and the handbook's C++20 profile (the CodeQL coding-standards
and profile packs, plus the validated clang-tidy and cppcheck checks), and reports each finding against the
handbook's compliance matrix and deviation records. A finding is fixed in the code; a deliberate exception names
its deviation on the line before it (`// codeql::misra_deviation_next_line(MD-CPP-009)` above the `.ipp` include
of `inc/tmc9660.hpp`). Until the repository is public the job runs with `open_source_prerelease: true`, the
declaration that the code is released under the licence in `LICENSE` and is to be published.

### Rehearsing the MISRA scan and the ESP32 builds locally

A first push is green only when the two CI checks a workstation used to leave to CI have passed locally.
`tools/preflight.sh --container` runs them in Docker, in addition to everything else it runs, and its last lines are
`== N failure(s)` and the gates line; both must show 0 failures.

- **`misra`** runs the `ru-misra` workflow's own scripts (`tools/ci-container/misra.sh`, image
  `tools/ci-container/Dockerfile.misra`): the caller's inputs are read from `.github/workflows/ci-misra.yml`; the
  tools are the versions and SHA-256 hashes of `tools/misra/toolchain.json` of the pinned ci-tools commit
  (CodeQL CLI bundle, `codeql/misra-cpp-coding-standards` pack, Arm GNU toolchain, clang-tidy, cppcheck); the
  configure, traced build, analysis, deviation records and report are the CI's. It passes when the report passes:
  a finding of a mandatory or required guideline that no record deviates, a scan problem or a suppression error
  fails; advisory findings are warnings. Each blocking finding is listed as `file:line failure Rule message`.
  The SARIF, JSON and CSV reports and the full log are kept in `$PREFLIGHT_OUT/misra` (default
  `$TMPDIR/kokeb-preflight/<repository>-<time>`).
- **`esp32`** builds what `ru-build.yml` builds: the matrix of `examples/esp32/app_config.yml` (the submodule's
  `generate_matrix.py`), every app and build type with `build_app.sh`, on ESP-IDF `release/v5.5` for the target of
  the examples. An app listed for another ESP-IDF version or target is not built in the image; the last line names
  it.

The CodeQL CLI and the pinned Arm toolchain exist for Linux x86-64 only. On an arm64 host (Apple silicon) the MISRA
image is `linux/amd64` and runs emulated; the ESP32 image is native on both architectures. A host without Docker
cannot run either; `tools/preflight.sh` then names both as skipped.

Differences from CI to know: the ESP-IDF commit is pinned in `Dockerfile.esp32` (CI clones the tip of the release
branch each run) and the ESP32 image uses Ubuntu's Python 3.12; the MISRA image has Ubuntu's `cmake` and `ninja`
where the runner has newer ones. `MISRA_ARGS=--host-only` scans the host build only, which is faster and leaves
the Cortex-M preprocessor paths to CI.

#### Bumping the pins

`Dockerfile.misra`: `CI_TOOLS_REF` is the commit `ru-misra.yml@v1` resolves to (`git ls-remote
https://github.com/kokeb-labs/ci-tools v1`), `HANDBOOK_REF` the default `handbook_ref` of that workflow; the tool
versions follow from `toolchain.json` of that commit. `Dockerfile.esp32`: `IDF_COMMIT` is the commit CI prints as
`ESP-IDF release/v5.5 is <sha>`. Rebuild with `docker rmi kokeb-ci-misra:local kokeb-ci-esp32:local`, then run
`--container`.

## Documentation snippets

Every C++ block on this site is included from [`tests/docs/`](../tests/docs/my_port.hpp), which the host build
compiles (target `tmc9660_doc_snippets`) with the driver instantiated over the documented port. A change to
the API that breaks a page breaks the build first. The README's example is a generated copy of the same file;
`kokeb-docs check` fails when it drifts.

## CI workflows

Everything runs from [`ci.yml`](../.github/workflows/ci.yml), which calls one workflow file per check and ends in
the `CI gate` job, the single required check (`CI gate / gate`).

| Job | What it checks |
| --- | --- |
| Host tests | GCC, then Clang (one job), build and run every test above, then build and run `examples/host` on its own |
| Host quality | The gates above: complexity, coverage, sanitizers, fuzzing |
| Cross-compile | `arm-none-eabi-g++` for Cortex-M0+, M4F and M7F with the strict flags, warnings as errors (nothing linked) |
| C++ lint | clang-format and clang-tidy over the sources |
| MISRA | MISRA C++:2023 and the C++20 profile on traced host and Cortex-M builds, at level `high` (see [Quality gates](#quality-gates)) |
| Docs | Strict site build (`_config/mkdocs.yml`), figure and generated-region drift, Doxygen (`_config/Doxyfile`), offline link check, Lighthouse |
| Repository checks | One job: DCO on pull requests, the org standard, Markdown and YAML lint, the [Kokeb Labs handbook](https://github.com/kokeb-labs/handbook) rules for `.kokeb/handbook.yml` (by rule ID), template drift (warns only) and cppcheck over the driver sources; a second job runs the handbook's source checks (include guards, bus requirements, raw access) |
| CI gate | Passes only if every job above passed: the one required check |

The **ESP32 examples** job builds every app in `examples/esp32/app_config.yml`, for each listed ESP-IDF version and
build type, with the repository root as the driver's component, and lints the example sources. The gate needs it
too.

The workflows call the shared, versioned workflows of
[kokeb-labs/ci-tools](https://github.com/kokeb-labs/ci-tools) at `@v1`.

## On hardware

Nothing in this repository has run on a chip. An MCU example ([Examples](examples.md)) is the bench test: it starts the
chip, reads its identity and its actual velocity twice a second. The bench steps that settle each reading of
[Ambiguities](ambiguities.md) are open work: start with A2, A3 and A4 (the frame layouts), then the reset evidence of
UART register mode. Record the board and the result in the pull request's bench-status section.
