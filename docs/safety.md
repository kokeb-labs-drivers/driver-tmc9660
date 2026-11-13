# Safety

The TMC9660 can energise a motor's power stage: 70 V and amperes. This page lists what the driver does so that a bug
in your application, a lost reply or a chip that restarted cannot turn the gate driver on by surprise, and what it
does not cover. Each rule is a test in `tests/test_safety.cpp`, `tests/test_parameter.cpp` or `tests/test_register.cpp`,
and each guard has a mutant that the tests must kill (see [Testing](testing.md#mutation-testing)).

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    From Analog Devices (Trinamic) 19-101926 1:

    - Supply ranges and the hibernation mode: Power Supply - Electrical Characteristics, p. 17
    - Protections and the fault pin: Protections Description, p. 56
    - Clock, reset, DRV_EN, WAKE and FAULTN: Clock and Control Description, p. 63
    - Boot configuration list, LDO and bootstrap configuration: Bootloader, Configuration, p. 83
    - Errata (OTP_BURN workaround, SPI slave MISO): Errata, p. 99

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

## The rules

| Rule | How the driver keeps it |
| --- | --- |
| Nothing energises by accident | Parameter mode: only `EnableDrive()` leaves `COMMUTATION_MODE` at a non-zero value (a stored script excepted: see below); `SetParam()` refuses it (`Refused`). Register mode: `WriteReg()` and `UpdateReg()` refuse the energising bits of `MCC_GDRV_HW` and `MCC_PWM_CONFIG`; only `GateDriverStart()` sets them |
| The guards complete before the energising write | `EnableDrive()` reads `GENERAL_ERROR_FLAGS` and `GDRV_ERROR_FLAGS` first (`DeviceFault` if either is non-zero), raises DRV_EN, and only then writes `COMMUTATION_MODE`; when that write fails (the chip may have run it all the same) it runs the full stop of `DisableDrive()`. `GateDriverStart()` reads `GDRV_STATUS` and `GDRV_FAULT` before it clears anything (a latched bit is `DeviceFault` and the evidence stays), writes the protection words, clears, reads again, and only then raises DRV_EN. After the bootstrap wait it writes `GDRV_STATUS_EN` and `GDRV_PROT_EN` and reads them back (`VerifyFailed`), so a shorted bridge is reported and shut off by the chip. It then reads the latched evidence once more: a protection that trips at once (for example with the default overcurrent words of 0, the lowest threshold with no blanking, and every protection on) makes the start `DeviceFault` and the stage is switched off again. Set `gdrv_ocp_uvw` and `gdrv_ocp_y2` for the board |
| A stop is never refused | `DisableDrive()` and `Deinitialize()` always send the full stop (also after a reported reset or before any start): `DisableDrive()` drops DRV_EN first, then writes `COMMUTATION_MODE` 0 (parameter mode) or zero to `MCC_GDRV_HW` and `MCC_PWM_CONFIG` (register mode), in the protocol the driver last proved: `Config::mode` before any start, the started application after `BlStartApplication()`, and a failed restart keeps it. In bootloader mode without a successful start and without a `GpioSet()` hook the driver cannot know what the chip runs: the stop returns `NotInitialized` instead of `Ok`; with the bootloader adopted and no hook, the stop asks the bootloader for its chip type (with the SPI reset evidence checked) and is `Ok` only when it answers; when it does not, the driver sends the stop of the application the last BOOT_MODE read named (or `Config::mode`), becomes uninitialised in that protocol and returns the error, so this and later stops reach a running application. A start write of the boot-mode word that fails other than by a bootloader error status, and any write of that word with `START_MOTOR_CTRL` set (A24), is followed as a start: the driver adopts the application, or confirms the bootloader still runs, or stays uninitialised in the application's protocol, so a later stop speaks to what may be running. It runs in any mode, after an unseen reset, without the canary check, and when a step fails it still runs the next. `MotorStop()` is not gated by a probe either. A parameter-mode stop or `MotorStop()` that fails for any reason (a reported reset, a lost frame, a stale or late UART reply, a frame merged with a partial request) is sent once more after the line is resynchronised, then the `IDLE_MOTOR_PWM_BEHAVIOR` check runs; a failed write of the register-mode stop is repeated once, and the register-mode stop reads both words back (`VerifyFailed` if one still has an energising bit). On a UART line that is out of step the stop is sent first and the line is resynchronised afterwards, so a parameter-mode or register-mode stop is not delayed by `uart_resync_us` (the bootloader check below is an ordinary request and waits for it) |
| System off really switches the PWM off | PM "System Off": with `IDLE_MOTOR_PWM_BEHAVIOR` (parameter 9, stored) at 0 the PWM stays active when `COMMUTATION_MODE` is 0. On a port with a `GpioSet()` hook the dropped DRV_EN switches the stage off. On a port without it the parameter-mode stop reads parameter 9, sets it to 1 and enters System off again when it was 0, and reads it back; a stop that cannot establish this returns an error (`VerifyFailed` or the bus error), never `Ok` while the PWM may still be driven. `SetParam(IDLE_MOTOR_PWM_BEHAVIOR, 0)` is `Refused` unless `Config::allow_pwm_in_system_off` is set (the stop without a DRV_EN hook sets it to 1 whatever that says) |
| No write after a reset the driver did not see | Every transport checks the reset evidence of its bus on each reply (see below). A write is only sent after a probe or canary check passed; a write the chip may have lost returns `UnexpectedReset` and the driver is uninitialised until `EnsureInitialized()` succeeds again. `PowerDown()` and `ReturnToBootloader()` stop first; when that stop saw a reset they return `UnexpectedReset` and send nothing more (no power-down command, no `Boot`) |
| Nothing is sent on a bad argument | Range, access, motor number, mask, address and alignment are checked against the generated tables first: `InvalidArgument`, zero transfers (`tests/test_safety.cpp`) |
| Boot with the drive on is impossible | `ReturnToBootloader()` switches the drive off, reads `COMMUTATION_MODE` back, and sends `Boot` only when it reads 0 (erratum 3). A chip that acknowledges the stop but stays on gets `Refused` and no `Boot` |
| Reconfiguring the motor needs the system off | `SetParam(MOTOR_TYPE)` and `FactoryDefault()` are `Refused` while `COMMUTATION_MODE` is not 0; `FactoryDefault()` also needs `confirm` |
| Configuration cannot start an unsafe application | `BlWriteConfig()` refuses a start with a reserved boot mode or a clock other than 40 MHz, refuses an LDO change without `allow_ldo_change`, writes the boot-mode word last, verifies by reading back, and waits for the PLL |
| The OTP is never written | `BlWrite32(Bank::Otp, ...)` is `Refused`; `OTP_BURN` is not exposed |
| Transfer buffers belong to the driver | The 9-byte and 8-byte frame buffers are members of the object, so the bytes live as long as the call; a test checks that the port is given addresses inside the object |
| The frame layout is proved before a write | At start-up in parameter mode the driver reads two flag parameters under each TMCL layout (A2); no write leaves before exactly one layout answers, otherwise `Refused` |
| A late reply cannot answer the next request | After a UART failure the next exchange first waits `Config::uart_resync_us` and drains the line (a drain that fails itself leaves the line dirty and the call returns `Bus` without sending); a reply with a wrong operation, address or host is `BadReply` and marks the line dirty again |

## Reset evidence by transport

| Transport | How an unseen reset shows | What the driver does |
| --- | --- | --- |
| Bootloader SPI | The status of a reply is `SESSION_START` or `BOOTLOADER_RESUMED` | `UnexpectedReset`, uninitialised |
| Bootloader UART | No indicator; FAULTN is the only evidence (`Service()`, A18) | Not detected without the pin; the bank is selected again before every word written, so a restart between words cannot send a word to the flash bank |
| Parameter SPI | `FIRST_CMD` (0x0C) in the status byte of a frame | `UnexpectedReset`; checked on the command frame, its reply request and the write probe |
| Parameter UART | The canary variable (A7); `Service()` also sees `WATCHDOG_EVENT` in `GENERAL_ERROR_FLAGS` when it was not set at the start | The write is not sent; `UnexpectedReset` |
| Register SPI | The boot bit or a skipped datagram counter | `UnexpectedReset` |
| Register UART | No indicator; FAULTN, checked before every write when `faultn_is_reset` is set | `UnexpectedReset` and no frame; not detected without the pin |

The Not-detected rows are a limit of the chip's replies, not of the driver; see
[Ambiguities and unverified items](ambiguities.md#not-verified). A noisy SPI line can corrupt the one status byte that
carries `FIRST_CMD`; the fuzz target documents this as residual risk.

## What the driver does not cover

- **Scripts.** A TMCL script downloaded with `ScriptDownload()` runs on the chip and can set parameters. The driver
  refuses a script that contains `SAP` of `COMMUTATION_MODE` with a non-zero value or `AAP` of it (`Refused`, nothing
  sent), but it does not analyse anything else, and `ScriptRun()` starts whatever the chip already holds.
- **The power stage.** Wrong MOSFET or motor data, a shorted bridge or an overcurrent setting that is too high are
  outside the driver. The chip's own protections (overcurrent, VGS, undervoltage, thermal) are configured by you and
  watched by the flag guards above only at the moment of starting.
- **Hardware not wired.** Without the `GpioSet()` hook the driver cannot drop DRV_EN; a board that needs a hardware
  stop must wire DRV_EN to something that can.
- **Concurrency.** The driver is not thread-safe. Call `DisableDrive()` from the same context as the other calls, or
  guard the object with one mutex; the stop path itself needs no more than the port's own hooks.
- **Verification on hardware.** Everything here is verified against a model of the chip built from the documents
  ([Testing](testing.md)); nothing has run on a real TMC9660 yet.

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
