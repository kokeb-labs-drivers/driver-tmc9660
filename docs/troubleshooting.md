# Troubleshooting

Find the cause of an error from the driver, by the call that returned it.

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    From Analog Devices (Trinamic) 19-101926 1:

    - Clock, reset, DRV_EN, WAKE and FAULTN: Clock and Control Description, p. 63

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

## EnsureInitialized returns Bus

The port reported a failure: either `EnsureInitialized()` returned `false` or a transfer did.

1. Check that your port's `EnsureInitialized()` returns `true` on its own (log it).
2. Check the bus pins in your port against the board ([Hardware](hardware.md#which-pins-carry-the-host-link)).
3. With a logic analyser, check that the first frame appears on the bus, with chip select low for the whole frame.

## EnsureInitialized returns DeviceIdMismatch

The chip identity did not read `0x544D0001`. The chip answered with something else, or in another protocol.

| Read value | Likely cause | Fix |
| --- | --- | --- |
| `0x00` | MISO not connected, or chip select not reaching the chip | Check MISO and chip select continuity |
| `0xFF` | MISO floating high, or the chip unpowered | Check VCC_IO, VSA and the supplies; check MISO is not pulled up hard |
| An identity read of another mode | `Config::mode` does not match the chip's boot configuration | The chip is in its bootloader (`Mode::Bootloader`), or already in an application |
| Anything else | Wrong SPI mode, a shifted bit, or the wrong UART address | Use SPI mode 3; check the UART device address is odd and matches |

## EnsureInitialized returns Timeout

The chip did not answer within `boot_timeout_ms`. Check the supplies, that RSTN is released (or released by your
`GpioSet()` with `GpioSignal::INACTIVE`), and FAULTN: low means the chip is busy bootstrapping or reports a severe
error such as an undervoltage on the 1.8 V rail.

## EnsureInitialized returns Unsupported or InvalidArgument

`Unsupported`: `hardware_reset` is set but the port has no `GpioSet()`. `InvalidArgument`: the configuration is out of
range, or the UART device address is even in parameter or register mode ([Configuration](configuration.md)).

## A call returns WrongMode

The call belongs to another mode than the chip is in; nothing was sent. After `BlStartApplication()` the driver follows
the chip into the new mode on its own. If you started the application by other means, create a driver with the right
`Config::mode`.

## EnableDrive returns DeviceFault

`GENERAL_ERROR_FLAGS` or `GDRV_ERROR_FLAGS` has a latched bit. Read them with `GetParam()`, find the cause
(undervoltage, overcurrent, gate-driver fault), fix it, and clear the flags with `ClearFlags()`. The driver does not
clear them for you.

## A call returns Refused

A safety guard said no, and nothing was sent or changed. The text logged by `Log()` says which one; the guards are on
the [Safety](safety.md) page. The usual ones: `SetParam(COMMUTATION_MODE, ...)` (use `EnableDrive()`), a write of
`MOTOR_TYPE` or `FactoryDefault()` while the system is on (call `DisableDrive()` first), a register write with an
energising bit (use `GateDriverStart()`), a configuration that would start with the wrong clock.

## A call returns UnexpectedReset

The driver saw evidence that the chip restarted since the last reply it trusted: a brown-out, the watchdog, or your own
reset. The driver is uninitialised again and sent no write after the evidence. Find out why the chip restarted (supply,
`GENERAL_STATUS_FLAGS` after the restart), then call `EnsureInitialized()` and configure the chip again.

## DisableDrive returns an error in bootloader mode without a DRV_EN hook

Without a `GpioSet()` hook the driver checks that the bootloader still answers before it calls a stop done. When it
does not, an unseen restart may have started an application: the driver sends that application's stop (the one
BOOT_MODE names in the configuration read at adoption or the last `BlReadConfig()`, or `Config::mode`; BUSY, an error
status and a seen reset are the bootloader answering, so they cause no such stop), marks itself uninitialised and
speaks that protocol from then on, so `DisableDrive()` again reaches the application. To recover, call
`EnsureInitialized()`: it starts from `Config::mode`, so use a `Config` in the mode the chip boots into, or reset the
chip into the bootloader.

## A reply fails: Crc, BadReply, Busy

`Crc` and `BadReply` are single corrupted frames on the bus: lower the clock, shorten the wires, add a ground next to
the clock line; the value was discarded. A `Crc` on every bootloader UART reply points at the CRC-8 reading (A1,
`Quirks::crc8_reflect_output`). `Busy` means the bootloader was still working; raise `spi_reply_retries`.

## Register reads of some registers return Unsupported

On SPI with the default layout, registers above offset `0x1FF` cannot be addressed (A3). Use UART, or set
`Quirks::reg_spi_wide_offset` once a bench check shows the chip uses the wide layout.

## Another SPI device on the same bus misbehaves

The TMC9660's MISO is not released when chip select rises (erratum 2). Hold the chip in reset during the other device's
transfers, or add the switch the datasheet shows ([Hardware](hardware.md#bus-settings)).
