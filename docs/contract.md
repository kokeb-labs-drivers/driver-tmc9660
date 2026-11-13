# Contract

What the TMC9660 driver promises, what it needs, and the rules a caller follows.

The driver inherits the [Kokeb Labs driver
contract](https://github.com/kokeb-labs/.github/blob/main/standards/driver.md); this page states only what is specific
to it. A change to anything here is a breaking change, listed under **Breaking** in the [changelog](changelog.md).

## Promises

- The constructor is `noexcept`, stores the configuration, does no I/O and cannot fail.
- `EnsureInitialized()` is the only way to start the driver. Its first call checks the stored configuration
  (`InvalidArgument`, nothing sent), brings the bus up, optionally pulses RSTN (`Config::hardware_reset`, needs
  `GpioSet()`), and checks the chip identity in the mode the chip is expected to be in. On any failure the driver
  stays uninitialised and the next call starts again; once it has succeeded, later calls return `Ok` with no I/O.
- Every call that touches the chip returns `Error::NotInitialized` before `EnsureInitialized()` succeeds and after a
  reset the driver detected. No call initialises or resets the chip as a side effect.
- A call that belongs to another mode returns `Error::WrongMode` and sends nothing.
- Every argument is checked against the datasheet tables before a byte is sent; a bad one returns
  `Error::InvalidArgument` and sends nothing.
- A safety guard that says no returns `Error::Refused` and sends nothing, or leaves the chip as it was.
- Nothing energises by accident, a stop is never refused, and no write follows a reset the driver could have seen:
  [Safety](safety.md) lists each rule and its test.
- Every port failure propagates as `Error::Bus`; no getter returns a stale or zero value after a failed read, and a
  value from a reply that failed its check is discarded (`Error::BadReply`, `Error::Crc`).
- Every wait is bounded: the boot wait by `Config::boot_timeout_ms` (each failed identity poll is charged the 10 ms
  pause and, on UART, `reply_timeout_us`, `uart_resync_us` and the three 5 ms drains it can take), the SPI reply by
  `Config::spi_reply_retries`, the UART reply by `Config::reply_timeout_us`, the PLL by a fixed number of polls.
- `Config{}` is the chip's reset configuration.
- A missing `DelayUs()` is a compile error; a missing `GpioSet()` makes `Config::hardware_reset` return
  `Error::Unsupported`; a missing `GpioGet()` or `Log()` does nothing. A misdeclared hook is a compile error.
- `Deinitialize()` always sends the full stop (DRV_EN low, `COMMUTATION_MODE` 0 or the register-mode zeroes), also
  after a reported reset or before any start, and then leaves the driver uninitialised. The stop speaks the protocol
  the driver last proved (`GetMode()`): a failed `EnsureInitialized()` keeps it. Before any start it is the protocol
  of `Config::mode`; on SPI a parameter-mode stop that reaches a chip in register mode is read by it as a register
  frame; which registers that frame addresses depends on the register SPI layout (A3: 9-bit offsets in the table,
  block 1 offset 9 and block 63 offset 0x100; with the wide layout the same bits are MCC offsets 0x104, 0x109 and
  0x22b, or RAMDebug 0x300). The datasheet does not say what such a write does, so this is a bench item, not a proved
  fact. A start of the application whose write fails (a lost, late, corrupt or BUSY reply) is followed as if it
  happened: the driver adopts the application, or finds the bootloader still answering and stays in it, or leaves the
  driver uninitialised in the application's protocol. With no proved protocol in bootloader mode and no `GpioSet()`
  hook it returns `NotInitialized`, never a silent `Ok`. In bootloader mode without a hook it is `Ok` only when the
  bootloader still answers its chip type. If it does not (a timeout, a bad frame, a wrong chip type; BUSY, an error
  status and a seen reset are the bootloader answering and change nothing), the chip may run an application that an
  unseen restart started: the driver becomes uninitialised, speaks that application's protocol from then on, sends its
  stop once, best effort, and still returns the error. The application is the one BOOT_MODE names in the configuration
  read at adoption (nothing was written since the reset, so that is what the OTP loaded, and after a restart the OTP
  decides) or in the last `BlReadConfig()`; with none named it is the protocol of `Config::mode`. A protocol stop is
  sent before a UART line is resynchronised, so it is never delayed by `Config::uart_resync_us`; the bootloader
  chip-type check is, after an earlier failed exchange. The stop never returns `Ok` while the PWM may still be driven:
  without a `GpioSet()` hook it also makes sure `IDLE_MOTOR_PWM_BEHAVIOR` is 1 (PM "System Off"), and
  `SetParam(IDLE_MOTOR_PWM_BEHAVIOR, 0)` is `Refused` unless `Config::allow_pwm_in_system_off` is set.
- UART `Exchange()`: a call with `tx_len == 0` sends nothing and discards what is waiting on the line (at most
  `rx_len` bytes, within `timeout_us`); the driver uses it to drain a late reply after a failed exchange, after
  waiting `Config::uart_resync_us`. A call with data must discard stale received bytes before it sends. The driver
  passes its own member buffers, so the bytes live for the whole call.

## Depends on

| Dependency | Version | Why |
| --- | --- | --- |
| A C++20 compiler | GCC 10, Clang 13 or later | Concepts constrain the port |
| CMake | 3.16 or later | Generates `tmc9660_version.h` |
| Your port | | The bus, within the bus requirements; the delay; optionally RSTN, DRV_EN, WAKE and FAULTN; and logging ([Porting](porting.md)) |

Nothing else: no RTOS, no SDK, no other repository of the org. The public headers include only `<algorithm>`,
`<array>`, `<bit>`, `<concepts>`, `<cstddef>`, `<cstdint>`, `<limits>`, `<span>`, `<type_traits>` and `<utility>`.

## Threading

The driver is not thread-safe and takes no lock. Use each driver object from one task at a time, or guard every
call on it with one mutex; the layer that owns the driver owns that lock, and the bus lock when several chips
share a bus. Nothing may run in an ISR: every call can block for a bus transfer or a delay. Separate objects
share no state.

## Errors

`tmc9660::Error` inside `tmc9660::Result<T>`. Values 0 to 9 are the shared base set; values from 16 are specific to
this driver.

| `Error` | Value | Returned when |
| --- | --- | --- |
| `Ok` | 0 | The call succeeded |
| `NotInitialized` | 1 | `EnsureInitialized()` has not succeeded, `Deinitialize()` ran, or a reset was detected |
| `InvalidArgument` | 2 | A `Config` field or an argument is out of range; nothing was sent |
| `Bus` | 3 | A port hook returned `false` or `BusStatus::Fault` |
| `Timeout` | 4 | No reply, the chip did not become ready, or the PLL did not lock |
| `DeviceIdMismatch` | 5 | The chip ID is not `0x544D0001` |
| `Crc` | 6 | The CRC-8 of a bootloader UART reply does not match |
| `Busy` | 7 | The bootloader answered BUSY and the retries ran out |
| `DeviceFault` | 8 | A latched error flag forbids the call |
| `Unsupported` | 9 | A register offset the SPI layout cannot reach (A3), or `hardware_reset` without `GpioSet()` |
| `UnexpectedReset` | 16 | The chip reset since the last validated reply; the driver is uninitialised |
| `Nack` | 17 | The chip answered with an error status |
| `WrongMode` | 18 | The call belongs to another mode than the chip is in |
| `Refused` | 19 | A safety guard said no |
| `VerifyFailed` | 20 | A read back after a write did not match |
| `BadReply` | 21 | A reply failed its checksum, address or opcode check |

The [API guide](api-guide.md#handle-errors) says what to do about each.

## Memory

No heap, no exceptions, no RTTI, no `std::function`, no `std::string`, no `printf` and no static mutable state. A
driver object holds a reference to the port, a `Config`, two 9-byte frame buffers and a few flags: 96 bytes on a 64-bit
host (`Config` is 44 bytes of it). The frame buffers are members, never locals, so a port that hands them to DMA has
them for the whole call. The parameter and register tables are `constexpr` and live in flash.

## Platform

Platform-free: the same code runs on any target with a C++20 compiler, including the host tests. The ESP-IDF port
in [`examples/esp32`](examples.md) is an example, not part of the driver.

## Deviations from the standard

One. The standard's setters take a trailing `Verify verify = Verify::None` argument; this driver does not: a parameter
write is checked by the chip's reply, a register write by the energising guard, and a configuration write by reading it
back. The gap is recorded in [Ambiguities and unverified items](ambiguities.md#not-verified).
