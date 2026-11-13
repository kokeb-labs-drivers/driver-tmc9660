# API guide

The calls of the driver, task by task. Every call returns `Result<T>`: check `ok()` before you use `value`.

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    From Analog Devices (Trinamic) 19-101926 1:

    - Bootloader commands and reply status codes: Bootloader, Commands, p. 76

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

## Start

```cpp
--8<-- "tests/docs/api_tasks.cpp:lifecycle"
```

`EnsureInitialized()` is the only call that starts the driver; it returns `Ok` with no I/O once it has succeeded. It
checks the identity of the chip in the mode named by `Config::mode`. The chip is not reset unless you set
`Config::hardware_reset`. After a failure the next call starts again.

## Start the application from the bootloader

```cpp
--8<-- "tests/docs/api_tasks.cpp:bootloader"
```

A freshly powered chip is in its bootloader (`Mode::Bootloader`). `BlWriteConfig()` writes only the words that differ,
the boot-mode word last, verifies by reading back, and follows the chip into the new mode. It refuses a start that
would use a reserved boot mode or a clock other than 40 MHz, and a change of the LDO outputs unless you allow it.
`BlStartApplication(BootMode::Parameter)` does the same in one call. More in [Bootloader](bootloader.md).

## Set and read parameters

```cpp
--8<-- "tests/docs/api_tasks.cpp:parameters"
```

`SetParam()` and `GetParam()` check the parameter against Table 57 first: access, range, coded values and the motor
number. `StoreParams()` saves the settings in the chip's external memory. More in [Parameter mode](parameter-mode.md).

## Drive the motor

```cpp
--8<-- "tests/docs/api_tasks.cpp:drive"
```

`EnableDrive(mode)` is the only way to a non-zero commutation mode: it reads the error flags (`DeviceFault` if one is
set), raises DRV_EN and only then writes the mode. `DisableDrive()` drops DRV_EN, then writes mode 0, and is never
refused; `Deinitialize()` does the same stop before it forgets the chip. See [Safety](safety.md).

## Register mode

```cpp
--8<-- "tests/docs/api_tasks.cpp:register-mode"
```

`ReadReg()`, `WriteReg()` and `UpdateReg()` work on the 163 registers; `ReadField()` and `WriteField()` take a
`fields::<register>::<Field>` type. `GateDriverStart()` runs the manual's start sequence with the fault guards before
the energising write, and the energising bits are refused anywhere else. See [Register mode](register-mode.md).

## Notice a reset

```cpp
--8<-- "tests/docs/api_tasks.cpp:reset"
```

A reset of the chip loses its configuration and, in parameter mode, the drive settings. The driver watches each
transport for evidence (the status byte on SPI, the canary variable on UART, the boot bit and counter in register mode)
and returns `UnexpectedReset` instead of sending a write that might act on a chip it no longer knows. Call `Service()`
from your main loop: it reads FAULTN when the port has `GpioGet()` and `Config::faultn_is_reset` is set, and polls one
status parameter in parameter mode.

## Handle errors

```cpp
--8<-- "tests/docs/api_tasks.cpp:errors"
```

| Signal | Meaning | What to do |
| --- | --- | --- |
| `Error::Bus` | The port failed a transfer or the bus bring-up | Check the port and the wiring; retry, or `Deinitialize()` and `EnsureInitialized()` |
| `Error::Timeout` | No reply, or the PLL did not lock | Check VCC_IO and RSTN; raise `boot_timeout_ms` or `reply_timeout_us` |
| `Error::DeviceIdMismatch` | The chip ID is wrong | Check the mode, the device address and the wiring |
| `Error::Crc`, `Error::BadReply` | A reply was corrupted on the bus | Lower the bus clock; check grounding; the value was discarded |
| `Error::Busy` | The bootloader stayed busy | Retry after a delay; raise `spi_reply_retries` |
| `Error::DeviceFault` | An error flag is latched | Read and clear the flags after fixing the cause |
| `Error::Nack` | The chip refused the command | `LastStatus()` has the chip's status code |
| `Error::WrongMode` | The call belongs to another mode | Nothing was sent; start the right mode |
| `Error::Refused` | A safety guard said no | Read the guard on [Safety](safety.md); nothing was changed |
| `Error::VerifyFailed` | A read back differed | The chip did not take the write; do not start the application |
| `Error::UnexpectedReset` | The chip restarted | `EnsureInitialized()`, then configure again |
| `Error::Unsupported` | An offset or hook this setup cannot reach | See A3 or add the `GpioSet()` hook |

Every value, with the calls that return it, is on the [Contract](contract.md#errors) page.
`tmc9660::ToString()` names an error for a log.

## Diagnostics

| Call | I/O | What it gives |
| --- | --- | --- |
| `IsInitialized()` | none | Whether the driver is started |
| `GetMode()` | none | The mode the driver speaks |
| `LastStatus()` | none | The status code of the last reply (TMCL status, bootloader status, register-mode flag) |
| `GetLastReply()` | none | Status, command number and value of the last valid parameter-mode reply |
| `ReadRawParam()` | one command | A GAP with a parameter number taken as given; the unsigned reply value; nothing is written |
| `GetConfig()` | none | The configuration in use |
| `GetChipId()`, `GetVersion()` | one command | The chip ID and version (parameter mode); `BlGetInfo()` in the bootloader |

## Shut down

```cpp
--8<-- "tests/docs/api_tasks.cpp:deinit"
```

After `Deinitialize()` every call returns `NotInitialized` until the next `EnsureInitialized()`. The drive is switched
off first, with DRV_EN dropped before the chip is told.

Next: [Configuration](configuration.md).
