# Datasheet map

The driver follows the TMC9660 datasheet and the two reference manuals. This page says which revisions, where to get
them, where each part of the driver is described in them, and, for every feature of the chip, whether the driver
exposes it, guards it or leaves it out. Section names and page numbers are the documents' own (the printed page
numbers).

<!-- generated: datasheet-map -->
- **Datasheet: TMC9660 70V Smart Gate Driver with Servo (FOC) Controller in HW and Buck Converter**, Analog Devices
  (Trinamic) 19-101926, revision 1 (2024-11). Get it from the [vendor
  page](https://www.analog.com/en/products/tmc9660.html).
  SHA-256 of the file used: `bfa29e7710b6b6a145aae98ff714342f393d8d8c4a7d14416b9e65202c07287d`.
- **Datasheet: TMC9660 Parameter Mode Reference Manual**, Analog Devices (Trinamic) n/a, revision 0 (2025-02). Get it
  from the [vendor page](https://www.analog.com/en/products/tmc9660.html).
  SHA-256 of the file used: `52f5bbc9f860c378ba9adedf76f78c22b5c1da0e643218599af3bb0abbd85a5e`.
- **Datasheet: TMC9660 Register Mode Reference Manual**, Analog Devices (Trinamic) n/a, revision 0 (2025-04). Get it
  from the [vendor page](https://www.analog.com/en/products/tmc9660.html).
  SHA-256 of the file used: `47d8da381b6bd8dbdcda9d941cc8affa8265d70ec361cc52f39bfb00a0c0665b`.
- **Datasheet: TMC9660 Configuration and Bootstrapping**, Analog Devices (Trinamic) AN-2601, revision 0 (2025-01-08).
  Get it from the [vendor page](https://www.analog.com/en/products/tmc9660.html).
  SHA-256 of the file used: `0fb66a609303e11896be6fbc05972b9dd4884b17f72d5e58c65915dc748d6c2f`.

| Topic | Datasheet section | Page | In the driver | Explained on |
| --- | --- | --- | --- | --- |
| Package, pins and power-up pin functions | 19-101926: Pin Configurations; Pin Descriptions | p. 11 | `CtrlPin`, `Config::mode` | [Hardware](hardware.md), [Quickstart](quickstart.md) |
| Application modes | 19-101926: Application Modes | p. 16 | `Mode`, `GetMode()`, `Error::WrongMode` | [Architecture](architecture.md), [Parameter mode](parameter-mode.md), [Register mode](register-mode.md) |
| Supply ranges and the hibernation mode | 19-101926: Power Supply - Electrical Characteristics | p. 17 | `CtrlPin::Wake`, `PowerDown()` | [Hardware](hardware.md), [Safety](safety.md) |
| Pin functions at power-up and the UART and SPI of the bootloader | 19-101926: General-Purpose Input/Output Description | p. 19 | `SpiPort`, `UartPort`, `Transfer()`, `Exchange()` | [Hardware](hardware.md), [Porting](porting.md) |
| Protections and the fault pin | 19-101926: Protections Description | p. 56 | `GateDriverStart()`, `Error::DeviceFault` | [Safety](safety.md), [Register mode](register-mode.md) |
| Clock, reset, DRV_EN, WAKE and FAULTN | 19-101926: Clock and Control Description | p. 63 | `CtrlPin`, `Config::hardware_reset`, `Service()` | [Hardware](hardware.md), [Safety](safety.md), [Troubleshooting](troubleshooting.md) |
| UART bootloader frame and its CRC-8 | 19-101926: Bootloader, UART Communication | p. 70 | `detail::BlUartEncode()`, `Error::Crc` | [Bootloader](bootloader.md) |
| SPI bootloader frame and delayed reply | 19-101926: Bootloader, SPI Communication | p. 72 | `detail::BlSpiEncode()`, `Config::spi_reply_retries` | [Bootloader](bootloader.md) |
| Bootloader commands and reply status codes | 19-101926: Bootloader, Commands | p. 76 | `BlGetInfo()`, `BlRead32()`, `BlWrite32()`, `BlEraseFlashSector()`, `BlStartApplication()` | [Bootloader](bootloader.md), [API guide](api-guide.md) |
| Boot configuration list, LDO and bootstrap configuration | 19-101926: Bootloader, Configuration | p. 83 | `BootConfig`, `BootField`, `BlWriteConfig()` | [Bootloader](bootloader.md), [Safety](safety.md), [Configuration](configuration.md) |
| Errata (OTP_BURN workaround, SPI slave MISO) | 19-101926: Errata | p. 99 | `BlWrite32()` (OTP never written), `ReturnToBootloader()` | [Bootloader](bootloader.md), [Safety](safety.md) |
| TMCL frames over UART and SPI | n/a: Communication Interfaces | p. 8 | `SetParam()`, `GetParam()`, `detail::TmclSpiEncode()` | [Parameter mode](parameter-mode.md) |
| Parameter table | n/a: Table 57, Full list of parameters | p. 102 | `inc/tmc9660_params.hpp`, `FindParam()` | [Parameter mode](parameter-mode.md), [Datasheet map](datasheet.md) |
| Register-mode SPI and UART datagrams | n/a: Communication interfaces | p. 8 | `ReadReg()`, `WriteReg()`, `UpdateReg()` | [Register mode](register-mode.md) |
| Register map | n/a: Register Map | p. 57 | `inc/tmc9660_registers.hpp`, `FindRegister()` | [Register mode](register-mode.md), [Datasheet map](datasheet.md) |
| Bootstrapping with the UblTools software | AN-2601: Bootstrapping the TMC9660 IC using UblTools | p. 1 | `BlWriteConfig()`, `BlStartApplication()` | [Bootloader](bootloader.md) |
<!-- /generated -->

## Coverage

Every feature of the chip is in one of three states:

- **Exposed**: reachable through a typed call, with the checks of the generated tables (access, range, coded values).
  The driver does not decide what a value means for your motor.
- **Guarded**: reachable, and the driver adds a rule so that the call cannot do harm by itself ([Safety](safety.md)).
- **Out of scope**: not in the driver, with the reason.

| Feature | Datasheet or manual | State | How |
| --- | --- | --- | --- |
| Bootloader: `GET_INFO` | Bootloader, Commands (p. 78) | Exposed | `BlGetInfo()` |
| Bootloader: bank and address, 32-bit reads | Commands (p. 79) | Exposed | `BlRead32()` |
| Bootloader: 32-bit writes to SPI flash and I2C EEPROM | Commands (p. 80) | Guarded | `BlWrite32()`: OTP and configuration banks refused |
| Bootloader: 8-bit and 16-bit reads and writes | Commands (p. 79 to 81) | Out of scope | Words are enough; a partial write adds no capability |
| Bootloader: `NO_OP` | Commands (p. 81) | Exposed | Used inside the SPI transport to fetch a delayed reply |
| Bootloader: `OTP_LOAD` | Commands (p. 81) | Exposed | `BlOtpLoad()` |
| Bootloader: `OTP_BURN` | Commands (p. 81), Erratum 1 | Out of scope | Permanent, and the erratum needs a multi-step workaround; use the vendor's tool |
| Bootloader: memory status | Commands (p. 82) | Exposed | `BlMemoryStatus()` |
| Bootloader: `FLASH_SEND_CMD` | Commands (p. 82) | Out of scope | A raw command to the external flash; the partition and erase calls cover the supported uses |
| Bootloader: `FLASH_ERASE_SECTOR` | Commands (p. 82) | Guarded | `BlEraseFlashSector()`: flash must be configured, connected and idle |
| Bootloader: `BOOTSTRAP_RS485` | Commands (p. 83) | Out of scope | A multi-drop bring-up procedure for RS-485 chains, outside the one-chip driver |
| Boot configuration: LDO, UART, SPI, flash, EEPROM, clock, bootstrap fields | Bootloader, Configuration (p. 83) | Exposed | `BootConfig` and `BootField`, all documented fields; the SPI-encoder select polarity (A16) through `SetWord()` |
| Boot configuration: LDO outputs VEXT1 and VEXT2 | Configuration (p. 84) | Guarded | `BlWriteConfig()` needs `allow_ldo_change` |
| Boot configuration: start the application | Configuration (p. 84) | Guarded | `BlStartApplication()`: no reserved mode, 40 MHz clock, PLL locked, boot-mode word last, read back |
| External memory partition table | Memory Access (p. 73) | Exposed | The partition builder in `tmc9660_bootconfig.hpp` |
| Parameter mode: 289 parameters | Parameter Mode Reference Manual, Table 57 (p. 102) | Exposed | `GetParam()`, `SetParam()` with range, access and coded-value checks |
| Parameter mode: `COMMUTATION_MODE` | Commutation Modes (p. 60) | Guarded | Only `EnableDrive()` sets a non-zero mode |
| Parameter mode: flags | Flags (p. 15) | Exposed | `GetParam()`; `ClearFlags()` for write-to-clear |
| Parameter mode: global parameters, four banks | Global Parameters (p. 143) | Exposed | `GetGlobal()`, `SetGlobal()` with the table's checks |
| Parameter mode: store and factory default | Storing System Settings (p. 15) | Guarded | `StoreParams()`; `FactoryDefault()` needs `confirm` and the system off |
| Parameter mode: GPIO control | GPIO Control (p. 16) | Exposed | `SetIo()`, `GetIo()` (pin numbering is not verified, see [Ambiguities](ambiguities.md#not-verified)) |
| Parameter mode: RAMDebug | RamDebug (p. 16) | Exposed | `RamDebug()` |
| Parameter mode: scripts | Script | Exposed | Download, run, stop, step, reset, status, read memory; not analysed (see [Safety](safety.md)) |
| Parameter mode: reference search | Automatic Homing | Exposed | `ReferenceSearchCommand()` |
| Parameter mode: motion targets | Position, Velocity, Torque Mode | Exposed | `SetTargetPosition()`, `SetTargetVelocity()`, `SetTargetTorque()` |
| Parameter mode: motor stop | Supported commands (p. 15) | Exposed | `MotorStop()`, never gated |
| Parameter mode: hibernation | Hibernation and Wakeup | Guarded | `PowerDown()` switches the drive off first |
| Parameter mode: return to the bootloader | Erratum 3 | Guarded | `ReturnToBootloader()` reads the commutation mode back |
| Parameter mode: SPI slave on SPI1 | Errata | Exposed | `DisableSpi1Slave()`, explicit only (A14) |
| Parameter mode: erratum 1 (script GPIO input) and 2 (IIT calculation) | Errata (p. 150) | Out of scope | Behaviour of the chip firmware; the parameters are exposed as the table states them |
| Parameter mode: brake chopper, mechanical brake, step/direction, heartbeat, second ABN encoder | Parameter Mode Reference Manual | Exposed | The parameters of Table 57 |
| Parameter mode: ABN, Hall and SPI encoder feedback, LUT correction, biquad filters | Feedback Sensor Configuration | Exposed | The parameters of Table 57 |
| Register mode: 163 registers | Register Mode Reference Manual, Register Map (p. 57) | Exposed | `ReadReg()`, `WriteReg()`, `UpdateReg()`, `ReadField()`, `WriteField()` |
| Register mode: gate driver and PWM enables | Gate driver setup (p. 20) | Guarded | Energising bits refused in `WriteReg()` and `UpdateReg()`; `GateDriverStart()` is the way |
| Register mode: registers above offset `0x1FF` on SPI | Communication interfaces (p. 9) | Guarded | `Unsupported` in the table layout; `Quirks::reg_spi_wide_offset` (A3) |
| Register mode: RAMDebug | RAMDebug (p. 54) | Exposed | `RegRamDebug()` |
| Gate driver: strength, timing, break-before-make, adaptive drive | Gate Driver Unit (p. 41) | Exposed | Parameters and `GateDriverRegConfig`; energising guarded |
| Gate driver: charge pump, trickle charge pump | Gate Driver Unit (p. 41) | Exposed | Bits of `MCC_GDRV_HW`; energising bits guarded |
| Protections: overcurrent, VGS, undervoltage, thermal | Protections (p. 56) | Exposed | Parameters and registers; the flags are read before every start. The driver does not check that your limits protect your power stage |
| Measurement: current-sense amplifiers, ADCs, analog inputs | Measurement Unit (p. 50) | Exposed | Parameters and registers |
| Measurement: chip and external temperature, supply voltage | Measurement Unit (p. 53) | Exposed | Parameters (read) |
| FOC, ramp generator, velocity and position loops, SVPWM | Motor Control Core (p. 29) | Exposed | Parameters and registers; the loops run on the chip |
| Motor types: BLDC, stepper, DC | Motor Types (p. 32) | Guarded | `MOTOR_TYPE` is refused while the system is on |
| Buck converter and charge pump supply | Power Management Unit (p. 34) | Out of scope | Fixed hardware without a driver-side control; supply voltages are measured through parameters |
| Clock: internal oscillator, crystal, external clock, PLL | Clock and Control (p. 63) | Guarded | Boot configuration fields; a start needs 40 MHz and a locked PLL |
| Reset (RSTN) | Clock and Control (p. 64) | Exposed | `GpioSet(CtrlPin::Reset)` with `Config::hardware_reset` |
| Driver enable (DRV_EN) | Clock and Control (p. 64) | Guarded | `GpioSet(CtrlPin::DrvEn)`; raised only by the guarded calls, dropped first on a stop |
| Wake-up (WAKE) | Clock and Control (p. 65) | Exposed | `GpioSet(CtrlPin::Wake)` for your own power-down logic |
| Fault output (FAULTN) | Clock and Control (p. 65) | Exposed | `GpioGet(CtrlPin::Faultn)` and `Service()` |
| UART interface and RS-485 transceiver direction | Communication (p. 23) | Exposed | `UartInterface`; the transceiver enable timing is a boot configuration field and the port's job |
| SPI interface | SPI (p. 25) | Exposed | `SpiInterface` |
| Erratum 2 (SPI MISO stays driven) | Errata (p. 99) | Out of scope | A board matter: hold RSTN during other transfers or add the switch ([Hardware](hardware.md#bus-settings)) |
| Watchdog and timers | Timer (p. 23) | Out of scope | Chip firmware peripherals with no driver-side control |

## When the datasheet and the driver differ

The driver is written against the revisions above. If you have a newer revision, compare its revision history with
the topics above; a change to a pin, a register, a timing or a value the driver uses is a bug report for this
repository. Where the documents disagree among themselves, [Ambiguities](ambiguities.md) says what the driver does.
Where the driver deliberately goes beyond or stays short of the documents, the [Contract](contract.md) says so.

The repository never contains a PDF: vendors own their documents. Download them from the vendor, and compare their
SHA-256 with the ones above to confirm you have the same files.
