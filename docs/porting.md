# Porting

Run the driver on a new MCU by writing one class: the port, on SPI or on UART.

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    From Analog Devices (Trinamic) 19-101926 1:

    - Pin functions at power-up and the UART and SPI of the bootloader: General-Purpose Input/Output Description, p. 19

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

The driver never calls your SDK. It calls hooks on a class you derive from `tmc9660::SpiInterface<Self>` (in
[`inc/tmc9660_spi_interface.hpp`](../inc/tmc9660_spi_interface.hpp)) or from `tmc9660::UartInterface<Self>` (in
[`inc/tmc9660_uart_interface.hpp`](../inc/tmc9660_uart_interface.hpp)), not both. The base uses CRTP and declares no
hook itself; the driver template is constrained by the concept `AnyPort`, so calls are resolved at compile time (no
virtual functions, nothing to allocate) and a missing or misdeclared hook is a compile error at the point of use.

## What you implement

| Method | Bus | Required | Contract |
| --- | --- | --- | --- |
| `bool EnsureInitialized() noexcept` | both | yes | Bring the bus up on the first call; return `true` at once afterwards. `false` makes the driver's `EnsureInitialized()` return `Bus` |
| `bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept` | SPI | yes | One full-duplex transfer of `len` bytes with chip select low for all of them; `rx` is always given. `false` on any bus error |
| `BusStatus Exchange(const uint8_t* tx, std::size_t tx_len, uint8_t* rx, std::size_t rx_len, uint32_t timeout_us) noexcept` | UART | yes | Send `tx_len` bytes without gaps, then receive exactly `rx_len` bytes within the timeout. `BusStatus::Ok`, `Timeout` or `Fault`. Remove the echo of your own bytes on a shared wire and drive the transceiver direction |
| `void DelayUs(uint32_t us) noexcept` | both | yes | Wait at least `us` microseconds. A port without it does not compile |
| `bool GpioSet(CtrlPin, GpioSignal) noexcept` | both | no | Drive RSTN, DRV_EN or WAKE at the logical level; `false` if the pin write failed |
| `bool GpioGet(CtrlPin, bool& high) noexcept` | both | no | Read FAULTN (or another control pin); `false` if the read failed |
| `void Log(LogLevel, const char* message) noexcept` | both | no | Print one line; `message` is plain text, never a format string |

## The SPI port

```cpp title="my_port.hpp"
--8<-- "tests/docs/my_port.hpp:port"
```

Rules the transfer must follow:

- **One call, one frame.** Keep chip select low for all `len` bytes; the chip decodes the frame when it rises. The
  frames are 5 (bootloader), 8 (parameter mode) and 6 (register mode) bytes.
- **The bus requirements below**: mode 3, MSB first, up to `kMaxClockHz`. Start at 1 MHz on jumper wires.
- **Raw bytes only.** Do not interpret, retry or reorder bytes; the driver owns the framing and the retries.
- **Return `false` for a bus error.** The driver turns it into `Error::Bus` and never ignores it.
- **Bounded time-outs.** A stuck bus fails the call; it does not block forever.
- **Idle levels.** Chip select high and SCK high (mode 3) before the first transfer, so that the chip never sees a
  partial frame.

## The UART port

```cpp title="my_port.hpp"
--8<-- "tests/docs/my_port.hpp:uart-port"
```

Rules the exchange must follow:

- **One call, one request.** Send the whole request without a pause between bytes; two documents disagree on how long
  a gap may be before the chip drops a partial message (ambiguity A6), and a request without gaps avoids the question.
- **Receive exactly `rx_len` bytes**, or report `Timeout` (nothing arrived) or `Fault` (framing or overrun). Never
  return partial data as `Ok`.
- **8N1, least significant bit first**, at the rate the chip has: it detects the rate of the first bytes (9600 to
  250000 at the 40 MHz clock), or the boot configuration sets it.
- **Half duplex.** On a shared RS-485 wire, enable the transmitter for the request and the receiver for the reply, and
  discard the echo.

## Bus requirements

The driver states what the TMC9660 needs from the bus as `constexpr` values in the two interface headers
(handbook KB-CPP-123). Check your port's settings against them with `static_assert`, as both example ports do, so a
port that clocks too fast or in the wrong mode does not compile:

| Constant | Value | Meaning |
| --- | --- | --- |
| `kSpiMode` | `3` | CPOL 1, CPHA 1 |
| `kMsbFirst` | `true` | Most significant bit first |
| `kMaxClockHz` | `10'000'000` | f_SCK max |
| `kCsHoldNs` | `100` | SCK edge to chip select change, t_CC |
| `kContinuousWord` | `true` | A datagram is one chip select low period |
| `kMinCsHighNs` | `100` | Chip select high between two datagrams |
| `kClockPhaseNs` | `20` | SCK low and high time |
| `kDataSetupNs` | `20` | Data setup and hold |
| `kUartDataBits` | `8` | 8N1 |
| `kMaxBaudRate` | `5'000'000` | Highest baud rate |
| `kTurnaroundUs` | `1000` | Assumed reply wait (ambiguity A19); the port timeout adds the frame time |
| `kMaxAutobaudRate` | `250'000` | Highest rate the chip detects at 40 MHz |

## Control pins

`GpioSet()` takes a `CtrlPin` and a logical `GpioSignal`; the port maps them to the physical level:

| Pin | `ACTIVE` means | The port writes |
| --- | --- | --- |
| `CtrlPin::Reset` | The chip is held in reset | RSTN low |
| `CtrlPin::DrvEn` | The gate driver outputs are allowed | DRV_EN high |
| `CtrlPin::Wake` | The wake condition is asserted | WAKE high |

Make the pins idle in their safe state (RSTN released, DRV_EN low) before the first transfer; the driver never
assumes it. A hardware reset at start is `Config::hardware_reset`:

```cpp title="Hardware reset at start"
--8<-- "tests/docs/ports.cpp:hardware-reset"
```

A full port with every hook:

```cpp title="A port with every optional hook"
--8<-- "tests/docs/ports.cpp:full-port"
```

A `GpioSet()` that returns `void` is a compile error: the driver must see a failed pin write. The
[compile-fail tests](testing.md) check this rule and the others.

## Reference implementations

Every port lives with its example, in `examples/<platform>/`; the driver has none. [Platforms](platforms.md) lists
them and how to add one.

- [`examples/host/host_tmc9660_port.hpp`](../examples/host/host_tmc9660_port.hpp): any PC, against a simulated
  chip. Start from it for a host build or a test bench; it is also the smallest complete port.
- [`examples/esp32/main/esp32_tmc9660_port.hpp`](../examples/esp32/main/esp32_tmc9660_port.hpp): ESP-IDF, with
  every hook. Start from it for any ESP-IDF target.

## Check your port

1. Build it: the ports on this page are compiled by the host CI from [`tests/docs/`](../tests/docs/my_port.hpp),
   with the driver instantiated over them.
2. Call `EnsureInitialized()` with `Mode::Bootloader` on a freshly powered chip: `Ok` proves that the frames are
   moved correctly and that the chip identity read `0x544D0001`.
3. Call `BlGetInfo(BlInfo::ChipFrequency)`: a plausible clock confirms a second, different frame.
4. Only then start the application (`BlStartApplication()`) and read a parameter or a register.

## Threads

The driver is not thread-safe. Give each driver object one owner task, or guard every call with one mutex. When
several chips share a bus, the port must also serialise the bus. See [Contract](contract.md#threading).

## Editor setup

The clang-format and clang-tidy configuration is in `_config/`, and the repository root has no copy of it. Tools
that take a path use it directly; CI does the same:

| Tool | Setting |
| --- | --- |
| Command line | `clang-format --style=file:_config/.clang-format`, `clang-tidy --config-file=_config/.clang-tidy -p build` |
| VS Code (C/C++ extension) | `"C_Cpp.clang_format_style": "file:${workspaceFolder}/_config/.clang-format"` and `"C_Cpp.codeAnalysis.clangTidy.args": ["--config-file=${workspaceFolder}/_config/.clang-tidy"]` |
| clangd (VS Code clangd, Neovim, Emacs), CLion | These find `.clang-format` and `.clang-tidy` only by name, from the file upwards: copy both to the root once (`cp _config/.clang-format _config/.clang-tidy .`). `.gitignore` lists the copies, so they never reach a commit; copy again after `_config/` changes |

Configure with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` (clangd: `--compile-commands-dir=build`) so the editor sees the
same flags as the build.
