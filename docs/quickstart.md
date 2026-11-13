# Quickstart

Run the driver on your PC first, with no hardware; then read a parameter from your MCU: wire the chip, write a small
port class, call two driver functions.

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    From Analog Devices (Trinamic) 19-101926 1:

    - Package, pins and power-up pin functions: Pin Configurations; Pin Descriptions, p. 11

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

## Try it on your PC

[`examples/host/`](../examples/host/README.md) runs the real driver against a simulated TMC9660, through a host
port. From the repository root, with CMake and a C++20 compiler:

```bash
cmake -S examples/host -B build-example
cmake --build build-example
./build-example/tmc9660_host_example
```

```text
TMC9660 driver 0.1.0 on the host
tmc9660: initialised
bootloader: chip type 544D0001
parameter mode: chip type 544D0001
MAX_TORQUE 70000: InvalidArgument
drive on, DRV_EN high
MAX_TORQUE 1500
drive off, DRV_EN low
done: 0 errors
```

The calls in [`main.cpp`](../examples/host/main.cpp) take the chip from its bootloader to a running drive and back to
off; only the port differs on an MCU. CI builds and runs this example on every pull request.

## 1. Wire it

The bootloader listens on UART and SPI at once, on the pins below. Use one bus; the driver is built for one.

| MCU | TMC9660 | Notes |
| --- | --- | --- |
| SPI SCK | GPIO11 (pin 3) | Mode 3, up to 10 MHz; 1 MHz for bring-up on jumper wires |
| SPI MOSI | GPIO10 (pin 2) | |
| SPI MISO | GPIO9 (pin 1) | Stays driven after chip select rises (erratum 2): no other SPI device on this line |
| GPIO (chip select) | GPIO12 (pin 4) | Low for the whole frame: one transfer per frame |
| GPIO | RSTN (pin 22) | Optional: hardware reset; leave the chip's pull-down alone when unused |
| GPIO | DRV_EN (pin 21) | Optional for this example; the driver raises it only through the guarded call |
| 3.3 V | VCC_IO (pin 9) | 2.2 V to 5.5 V; 1 µF to GND close to the pin |
| GND | GND (pin 8) | Share the ground with the power stage |

VSA and VS carry the motor supply and the chip's own supplies need their capacitors: [Hardware](hardware.md) has the
full pin list, the reference circuit and the timing. Do not connect a motor for the first steps.

## 2. Add the driver

Add the repository as a submodule and link the `tmc9660::tmc9660` CMake target. [Installation](installation.md)
covers CMake on any MCU, the installed package and ESP-IDF.

```bash
git submodule add https://github.com/kokeb-labs-drivers/driver-tmc9660.git external/driver-tmc9660
```

## 3. Write the port

The port is the only platform code: it brings the bus up and moves bytes. Replace the `platform_*` calls with your
SDK's.

```cpp title="my_port.hpp"
--8<-- "tests/docs/my_port.hpp:port"
```

[Porting](porting.md) explains each method, the optional hooks, the UART variant and the transfer rules.

## 4. Read a parameter

```cpp title="main.cpp"
--8<-- "tests/docs/quickstart.cpp:quickstart"
```

What each step does:

1. `EnsureInitialized()` calls your port's `EnsureInitialized()`, and checks the chip identity in the mode you named.
   It does not reset the chip unless `Config::hardware_reset` is set: it adopts the chip as it is.
2. `GetParam()` checks the parameter and the motor number, then runs one TMCL command.
3. The value is in the unit of the parameter, as Table 57 of the Parameter Mode Reference Manual defines it.

A chip that has just powered up is in its bootloader: start with `Config::mode = Mode::Bootloader` and
`BlStartApplication()` ([Bootloader](bootloader.md)) to reach parameter mode, as the host example does.

This example is built by the host CI, so it compiles against the current headers.

## If it does not work

| Symptom | First thing to check |
| --- | --- |
| `EnsureInitialized()` returns `Bus` | Your port's `EnsureInitialized()`: the bus did not come up |
| `EnsureInitialized()` returns `DeviceIdMismatch` | The mode (the chip may be in its bootloader), the wiring and the device address ([Troubleshooting](troubleshooting.md)) |
| `EnsureInitialized()` returns `Timeout` | VCC_IO and RSTN: the chip never answered |
| A call returns `WrongMode` | The chip is in another mode than `Config::mode` says |
| `EnableDrive()` returns `DeviceFault` | A latched error flag: read `GENERAL_ERROR_FLAGS` and `GDRV_ERROR_FLAGS`, fix the cause, clear them |

## Next

- [API guide](api-guide.md): start the application, set parameters, drive, handle errors.
- [Safety](safety.md): what keeps the power stage off.
- [Configuration](configuration.md): every `Config` field, the readings of ambiguous documents, Kconfig.
- [Examples](examples.md): the host example and the MCU examples.
- [Platforms](platforms.md): which MCUs have a port and an example.
