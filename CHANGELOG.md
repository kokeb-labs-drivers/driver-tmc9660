# Changelog

All notable changes to this repository are listed here. Versions follow [Semantic Versioning](https://semver.org/).
Entries under **Breaking** need source changes in code that used the affected API; any of them calls for a major
version bump (a minor bump before 1.0).

## [Unreleased]

### Added

- TMC9660 driver, first version: the bootloader (information, memory, typed configuration, guarded application start,
  flash erase), parameter mode (289 checked parameters, global parameters, scripts, RAMDebug, I/O) and register mode
  (163 checked registers, fields, RAMDebug, guarded gate-driver start), over SPI or UART.
- Safety guards: the drive is enabled only through `EnableDrive()` and `GateDriverStart()` after the error flags are
  read, `DisableDrive()` is never refused, no write follows a reset the driver saw, and the OTP is never written.
- Documented ambiguities (A1 to A24), most with a configuration knob (`Config`, `Quirks`).
- `Quirks::tmcl_layout` (`Auto`, `MotorHigh`, `TypeHigh`): the TMCL byte layout is probed with two reads at start-up
  and the start is refused when it is not decided; `Config::uart_resync_us`: the gap after a failed UART exchange
  before the line is drained.
- `GateDriverStart()` writes `GDRV_PROT`, `GDRV_OCP_UVW`, `GDRV_OCP_Y2`, `GDRV_STATUS_EN` and `GDRV_PROT_EN`
  (`GateDriverRegConfig::gdrv_prot`, `gdrv_ocp_uvw`, `gdrv_ocp_y2`, `status_en`, `prot_en`) and reads the latched
  status before clearing it.
- `Config::allow_pwm_in_system_off`: `SetParam(IDLE_MOTOR_PWM_BEHAVIOR, 0)` is `Refused` without it, and the stop of a
  port with no DRV_EN hook sets the parameter to 1 (PM "System Off") and fails rather than return `Ok` with the PWM
  active. A stop is sent before a UART line is resynchronised, a failed register-mode stop write is repeated once, and
  `GateDriverStart()` reads the latched evidence again after the protections are on (`DeviceFault` on a trip).
- `PowerDown()` and `ReturnToBootloader()` return `UnexpectedReset` and send nothing after their stop when the stop
  saw a reset (found by the fuzz gate: `PowerDown()` returned `NotInitialized` after sending, `ReturnToBootloader()`
  sent `Boot`). A UART drain that fails leaves the line dirty and the call returns `Bus` without sending (a stale reply
  could answer the next read). The fuzz target's reset promise now follows the driver's current mode.
- The parameter-mode stop and `MotorStop()` are sent once more after any failure, not only after a reported reset
  (the repeat resynchronises a dirty UART line first); a failed `EnableDrive()` runs the full stop of
  `DisableDrive()`, so it leaves the drive stopped also without a DRV_EN hook.
- A failed `EnsureInitialized()` keeps the protocol the driver last proved, so a stop after a failed restart still
  reaches a started application; without a proved protocol in bootloader mode and without a DRV_EN hook,
  `DisableDrive()` returns `NotInitialized` instead of `Ok`. `Config::uart_resync_us` is now 150 ms (50 % over the
  chip's discard gap, A6).
- A start of the application from the bootloader whose write fails other than by a bootloader error status, or any
  write of the boot-mode word with `START_MOTOR_CTRL` set (A24), is followed as a start, so a later stop reaches the
  application. With the bootloader adopted and no DRV_EN hook, `DisableDrive()` is `Ok` only when the bootloader still
  answers.
- The identity wait after a reset, a start or `Boot` is bounded by `boot_timeout_ms`: each failed UART poll is charged
  its reply wait, resync gap and drains. In bootloader mode without a DRV_EN hook, a stop that finds no bootloader
  sends the stop of the application the last BOOT_MODE read named (or `Config::mode`) and leaves the driver
  uninitialised in that protocol.
- `docs/deviations.yml`: the repository's (empty) deviation record file, read by the MISRA workflow.
- `BlEraseFlashSector(address, allow_partition_table)` checks alignment and range and refuses sector 0 unless asked.
- `Service()` sees a `WATCHDOG_EVENT` as a restart in parameter mode.
- `tools/gen_tables/extract_check.py`: re-derives the table counts from the manuals' text.
- The generator of the parameter and register tables (`tools/gen_tables`), the model-based tests, a fuzz target and
  the mutation tool (`tools/mutants`).
- Documentation: hardware figures and tables, bootloader, parameter mode, register mode, safety, ambiguities, coverage
  of the chip's features.
