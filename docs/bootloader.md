# Bootloader

After power-up the TMC9660 runs its bootloader. It answers on UART and on SPI at the same time, holds a 64-byte
configuration (16 little-endian words) and starts the motion-control application in parameter mode or register mode
when that configuration says so. The driver exposes the bootloader as `Mode::Bootloader`: the `Bl*` calls.

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    - UART bootloader frame and its CRC-8: Analog Devices (Trinamic) 19-101926 1, Bootloader, UART Communication, p. 70
    - SPI bootloader frame and delayed reply: Analog Devices (Trinamic) 19-101926 1, Bootloader, SPI Communication, p. 72
    - Bootloader commands and reply status codes: Analog Devices (Trinamic) 19-101926 1, Bootloader, Commands, p. 76
    - Boot configuration list, LDO and bootstrap configuration: Analog Devices (Trinamic) 19-101926 1, Bootloader, Configuration, p. 83
    - Errata (OTP_BURN workaround, SPI slave MISO): Analog Devices (Trinamic) 19-101926 1, Errata, p. 99
    - Bootstrapping with the UblTools software: Analog Devices (Trinamic) AN-2601 0, Bootstrapping the TMC9660 IC using UblTools, p. 1

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

## Frames

| Bus | Request | Reply | Driver |
| --- | --- | --- | --- |
| UART | 8 bytes: `0x55`, device address, command, type, value (4 bytes), CRC-8 | 8 bytes in the same layout with the host address, a status and the CRC-8 | `Exchange()`, one call per frame |
| SPI | 5 bytes: command, value (4 bytes) | The reply to a command comes with the next transfer; a `NO_OP` fetches it | `Transfer()`, two or more per command |

The CRC-8 uses polynomial `0x07`, start value 0, input bytes reflected. Whether the result is reflected too is
ambiguity A1: the prose says yes and the worked examples say no. The driver follows the examples and proves them
with `static_assert` in `inc/tmc9660_frames.hpp`; `Quirks::crc8_reflect_output` selects the other reading.

On SPI the status of a reply can be BUSY (the chip is still working). The driver repeats the `NO_OP` fetch up to
`Config::spi_reply_retries` times and then returns `Error::Busy`; it never repeats the command itself, because a
write must not run twice. `SESSION_START` (19) and `BOOTLOADER_RESUMED` (21) in a status tell the driver that the
chip restarted since the last frame: it returns `Error::UnexpectedReset` and sends nothing else until
`EnsureInitialized()` ran again.

## Calls

| Call | What it does | Guards |
| --- | --- | --- |
| `BlGetInfo(BlInfo)` | `GET_INFO`: chip type, bootloader version, clock, memory sizes | `WrongMode` outside the bootloader |
| `BlRead32(Bank, address)` | Reads a word from SPI flash, I2C EEPROM, OTP or the configuration memory | Address must be a multiple of 4 |
| `BlWrite32(Bank, address, value)` | Writes a word to SPI flash or I2C EEPROM | `Refused` for the OTP bank and for the configuration memory |
| `BlReadConfig()` | Reads the 64 bytes into a typed `BootConfig` | |
| `BlWriteConfig(config, options)` | Writes a configuration and verifies it | See below |
| `BlStartApplication(BootMode)` | Sets `BOOT_MODE` and `START_MOTOR_CTRL`, writes, follows the chip into the new mode | `Refused` for a reserved mode |
| `BlMemoryStatus(Bank, MemQuery)` | `MEM_IS_CONFIGURED`, `MEM_IS_CONNECTED`, `MEM_IS_BUSY` | |
| `BlEraseFlashSector(address)` | `FLASH_ERASE_SECTOR` | `Refused` unless the flash is configured, connected and idle |
| `BlOtpLoad()` | `OTP_LOAD`: load the OTP configuration | |

`OTP_BURN` is not exposed: it is permanent, and the datasheet's erratum 1 needs a multi-step workaround. Writing to
the OTP bank through `BlWrite32()` is `Refused`. If you need to burn the OTP, use the vendor's tool.

## Writing a configuration

`BootConfig` has one named field for every documented option (`BootField`), range checked, with the layout in a
single table. A field the documents leave unclear, such as the polarity of the SPI encoder's chip select (A16), is
reached through the raw `SetWord()`.

`BlWriteConfig()` is built to refuse the configurations that are known to leave the chip in a state the board did not
ask for (the checks below are the ones the documents allow; a configuration the documents do not describe is not
judged):

1. It refuses, before writing anything, a configuration that would start the application with a reserved boot mode
   or with a system clock other than 40 MHz (`BootConfig::CheckStart()`), and a change of the LDO word unless
   `BlWriteOptions::allow_ldo_change` is set (both outputs VEXT1 and VEXT2 move at once).
2. It writes only the words that differ, the clock word with one 32-bit write, and the boot-mode word last.
3. After a clock change it waits for the PLL to lock (`Error::Timeout` if it does not) and reads every word back
   (`Error::VerifyFailed` on a difference, the lock status bit excepted).
4. If the write starts the application, it checks that the PLL is locked first, then switches the driver to the new
   mode and checks the chip ID there.

`BlStartApplication(target)` is `BlReadConfig()`, two field changes and `BlWriteConfig()`.

## Returning to the bootloader

`ReturnToBootloader()` (parameter mode) sends the `Boot` command. Erratum 3 of the Parameter Mode Reference Manual
says that exiting to the bootloader with the motor running can leave the gate driver on, so the driver switches the
drive off first, reads `COMMUTATION_MODE` back, and sends nothing if it is not zero (`Error::Refused`). See
[Safety](safety.md).

<!-- generated: datasheet-card -->
| Document | Revision | Get it |
| --- | --- | --- |
| Datasheet: TMC9660 70V Smart Gate Driver with Servo (FOC) Controller in HW and Buck Converter (Analog Devices (Trinamic) 19-101926) | 1, 2024-11 | [vendor page](https://www.analog.com/en/products/tmc9660.html) |
| Datasheet: TMC9660 Parameter Mode Reference Manual (Analog Devices (Trinamic) n/a) | 0, 2025-02 | [vendor page](https://www.analog.com/en/products/tmc9660.html) |
| Datasheet: TMC9660 Register Mode Reference Manual (Analog Devices (Trinamic) n/a) | 0, 2025-04 | [vendor page](https://www.analog.com/en/products/tmc9660.html) |
| Datasheet: TMC9660 Configuration and Bootstrapping (Analog Devices (Trinamic) AN-2601) | 0, 2025-01-08 | [vendor page](https://www.analog.com/en/products/tmc9660.html) |

Written against these revisions; the SHA-256 of each file is in [`docs/datasheets.yml`](datasheets.yml). Where each part
of the driver is in the datasheet: [Datasheet map](datasheet.md).
<!-- /generated -->
