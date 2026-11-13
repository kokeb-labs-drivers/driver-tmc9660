# Register mode

In register mode the application exposes the hardware registers directly, with none of the parameter-mode software.
The driver exposes it as `Mode::Register`: `ReadReg()`, `WriteReg()`, `UpdateReg()` and `GateDriverStart()`.

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    - Application modes: Analog Devices (Trinamic) 19-101926 1, Application Modes, p. 16
    - Protections and the fault pin: Analog Devices (Trinamic) 19-101926 1, Protections Description, p. 56
    - Register-mode SPI and UART datagrams: Analog Devices (Trinamic) n/a 0, Communication interfaces, p. 8
    - Register map: Analog Devices (Trinamic) n/a 0, Register Map, p. 57

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

## Frames

| Bus | Frame | Driver |
| --- | --- | --- |
| UART | 9 bytes, with the register address, a write or read code (146, 148, 142), data and a checksum | One `Exchange()` |
| SPI | 6 bytes: block, offset, write bit, data (4 bytes); the reply to a datagram arrives with the next one | A read is two `Transfer()` calls (request, then a harmless fetch); a write is three (a probe read comes first) |

On SPI every reply starts with a boot bit and a 3-bit datagram counter. The first reply after a reset has the boot
bit set, and it is the baseline `EnsureInitialized()` takes. The driver checks both on every later reply: a set boot
bit, or a counter that skips a number, means the chip restarted or a datagram was lost, and the call returns
`Error::UnexpectedReset`. A write is preceded by a probe datagram (a read of `MCC_INFO_CHIP`), so that a restart is
seen before the write is sent, not after it.

The SPI register layout has a 9-bit offset (ambiguity A3), but 8 registers lie above offset `0x1FF`. In the table
layout these return `Error::Unsupported`; `Quirks::reg_spi_wide_offset` selects the 10-bit layout and reaches all
163. UART reaches all of them. The bit order of the UART fields is A4.

## Registers

`inc/tmc9660_registers.hpp` has 163 registers generated from the Register Mode Reference Manual: block, offset,
access, writable mask, reset value, and in the `fields::<register>` namespaces the bit fields. `ReadReg()` and
`WriteReg()` reject an address that is not in the table, a write to a read-only register, and a write that touches
bits outside the writable mask (`Error::InvalidArgument`, nothing sent). `UpdateReg(addr, mask, value)` reads,
changes the masked bits and writes back.

## Starting the gate driver

The gate driver of the TMC9660 can switch a motor's power stage. `WriteReg()` and `UpdateReg()` therefore refuse any
write that would turn it on: the energising bits of `MCC_GDRV_HW` (mask `0x0300'080F`) and of `MCC_PWM_CONFIG` (mask
`0x0000'FF07`) return `Error::Refused`. A stop is not refused: writing zero to them is allowed.

`GateDriverStart(GateDriverRegConfig)` is the one way on. It works in this order:

1. It checks the mode and the configuration before any byte is sent: every value inside the register's writable
   mask, the chop value 1 to 7 (`Error::InvalidArgument`, nothing sent).
2. It reads `MCC_GDRV_STATUS` and `MCC_GDRV_FAULT` before anything is written or cleared. A latched status bit (the
   bootstrap flags excepted) or an active fault or undervoltage is `Error::DeviceFault` and the evidence stays in
   the chip for you to read and clear.
3. It writes the configuration with nothing energised (PWM enables with CHOP 1, gate current, timing, break-before-
   make, `MCC_GDRV_PROT`, the overcurrent words), writes zero to the status enable register and clears the status
   flags, then reads status and fault again: a condition that latches again is `Error::DeviceFault`.
4. Only then it raises DRV_EN, writes `MCC_GDRV_HW`, reads it back (`Error::VerifyFailed` on a difference) and
   writes the PWM enables with the chop value last.
5. After the bootstrap wait (`Config::settle_us`) it writes `MCC_GDRV_STATUS_EN` and `MCC_GDRV_PROT_EN` (the manual's
   protection setup; defaults `0xEFFFFFFF` and `0xAFFFFFFF` in `GateDriverRegConfig`) and reads both back
   (`Error::VerifyFailed`). Without them a shorted bridge would be neither reported nor shut off by the chip.

A fault, a failed write or a read-back that differs makes the driver write zero to the enables (`DisableDrive()`)
before it returns. `DisableDrive()` in register mode writes zero to `MCC_GDRV_HW` and `MCC_PWM_CONFIG`, writes again
after a reported reset, and reads both back (`Error::VerifyFailed` if an energising bit is still set).

## Detecting a reset on UART

UART register mode has no reset indication in the reply. The driver reads `MCC_INFO_CHIP` at start and otherwise
relies on FAULTN through `Service()` (A18). This is the weakest reset detection of the driver and is listed under
[not verified](ambiguities.md#not-verified).
