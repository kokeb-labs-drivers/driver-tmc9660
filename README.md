# TMC9660 Driver

<!-- generated: chip-summary -->
The **TMC9660** is a gate driver and motor controller for BLDC, PMSM, stepper and brushed DC motors: field-oriented
control, the servo loops and a ramp generator run in hardware, and a buck converter powers the chip from the motor
supply (General Description, p. 1).
<!-- /generated -->

C++20 driver for the TMC9660 servo controller and gate driver (bootloader, parameter mode and register mode over SPI or UART)

[![CI](https://github.com/kokeb-labs-drivers/driver-tmc9660/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/kokeb-labs-drivers/driver-tmc9660/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-informational)](docs/installation.md#requirements)

**Documentation: [`docs/`](docs/index.md)**, with the API reference built by Doxygen ([how](docs/api-reference.md)).
The site at <https://kokeb-labs-drivers.github.io/driver-tmc9660/> opens when the repository is public.

<!-- figure: chip-hero -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/diagrams/chip-hero.dark.svg">
  <img src="docs/diagrams/chip-hero.svg"
    alt="Overview of the TMC9660, a gate driver and motor controller for BLDC, PMSM, stepper and brushed DC
    motors: field-oriented control, the servo loops and a ramp generator run in hardware, and a buck
    converter powers the chip from the motor supply. A MCU talks to it over SPI or UART; it drives
    MOSFET half-bridges, then Motor; it reads back Phase currents, Position sensor, Reference switches.
    Headline figures: 7.7 to 70 V single-supply operating range; 1 A / 2 A gate drive source and sink,
    up to four half-bridges; 10 MHz SPI clock, mode 3; 9 x 9 mm 64-pin TQFN body.">
</picture>
<!-- /figure -->

## What it does

<!-- figure: chip-capabilities -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/diagrams/chip-capabilities.dark.svg">
  <img src="docs/diagrams/chip-capabilities.svg"
    alt="Capability map of the TMC9660: FOC and servo loops (supported): Runs field-oriented current control
    and the torque, velocity and position loops in hardware; Ramp generator and homing (supported):
    Plans eight-point acceleration ramps in real time and searches the reference switches for home;
    Position feedback (supported): Reads Hall sensors, ABN encoders or an SPI encoder for commutation
    and position; Smart gate driver (supported): Drives up to four NMOS half-bridges, 1 A source and 2 A
    sink; Current sensing and measurement (supported): Amplifies the bottom-shunt currents and measures
    the supply voltage and the temperatures; Protections (supported): Overcurrent, gate-short,
    undervoltage and thermal protection; the flags are read before every start; Bootloader and
    application modes (supported): Sets up pins, clock and LDOs at power-up, then starts parameter mode
    (TMCL) or register mode; Hibernation and wake-up (supported): Switches all but the always-on block
    off; the WAKE pin or a timer powers the chip up again; OTP configuration (not covered): Stores the
    boot configuration permanently; the driver loads it and never burns it.">
</picture>
<!-- /figure -->

## Where it is used

<!-- generated: chip-uses -->
- Robotics
- Power tools
- Gardening equipment
- Automated guided vehicles and warehouse automation
- Pumps, such as peristaltic pumps
- Industrial 3D printing
- Factory automation
- Desktop manufacturing
- E-bikes and light electric vehicles

Uses named by the datasheet (General Description, p. 1).
<!-- /generated -->

## Inside the chip

<!-- figure: chip-blocks -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/diagrams/chip-blocks.dark.svg">
  <img src="docs/diagrams/chip-blocks.svg"
    alt="Block diagram of the TMC9660: its function blocks, how they connect and which package pins reach
    each block.">
</picture>
<!-- /figure -->

## Pins and wiring

<!-- figure: chip-pinout -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/diagrams/chip-pinout.dark.svg">
  <img src="docs/diagrams/chip-pinout.svg"
    alt="Pinout of the TMC9660 in TQFN-64, top view: 64 pins and an exposed pad, each with its name, its
    electrical type as a colour and the driver hook that uses it.">
</picture>
<!-- /figure -->

<!-- figure: chip-wiring-typical -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/diagrams/chip-wiring-typical.dark.svg">
  <img src="docs/diagrams/chip-wiring-typical.svg"
    alt="Wiring of the TMC9660 for the Typical application: MCU SPI SCK to GPIO11 (Transfer()); MCU SPI MOSI
    to GPIO10 (Transfer()); MCU SPI MISO to GPIO9 (Transfer()); MCU GPIO / CS to GPIO12 (Transfer());
    MCU GPIO to RSTN (GpioSet(CtrlPin::Reset)); MCU GPIO to DRV_EN (GpioSet(CtrlPin::DrvEn)); MCU GPIO
    (IRQ) to FAULTN (GpioGet(CtrlPin::Faultn)). Supplies, grounds and the support parts are listed under
    the drawing.">
</picture>
<!-- /figure -->

| MCU | TMC9660 | Notes |
| --- | --- | --- |
| SPI SCK | GPIO11 (pin 3) | Mode 3, up to 10 MHz; 1 MHz for bring-up on jumper wires |
| SPI MOSI | GPIO10 (pin 2) | |
| SPI MISO | GPIO9 (pin 1) | Stays driven after chip select rises (erratum 2): no other SPI device on this line |
| GPIO (chip select) | GPIO12 (pin 4) | Low for the whole frame: one transfer per frame |
| GPIO | RSTN (pin 22), DRV_EN (pin 21) | Optional; the driver drives them only through `GpioSet()` |
| 3.3 V | VCC_IO (pin 9) | 2.2 V to 5.5 V; 1 µF to GND close to the pin |
| GND | GND (pin 8) | |

On UART the pins are GPIO6 (TX, pin 62) and GPIO7 (RX, pin 63) of the chip.

<!-- figure: chip-frame-tmcl-spi -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/diagrams/chip-frame-tmcl-spi.dark.svg">
  <img src="docs/diagrams/chip-frame-tmcl-spi.svg"
    alt="Parameter-mode SPI datagram of the TMC9660, 64 bits, most significant bit first. Command: operation
    (bits 63 to 56), type and motor or bank (bits 55 to 40), data (big endian) (bits 39 to 8), checksum
    (bits 7 to 0); Reply: SPI status (bits 63 to 56), TMCL status (bits 55 to 48), operation (bits 47 to
    40), data (big endian) (bits 39 to 8), checksum (bits 7 to 0). Below, the SPI mode 3 waveform of one
    transfer in the same bit grid.">
</picture>
<!-- /figure -->

<!-- figure: chip-outline -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/diagrams/chip-outline.dark.svg">
  <img src="docs/diagrams/chip-outline.svg"
    alt="Outline of the TMC9660 in TQFN-64 drawn to scale from above: body 9.0 x 9.0 mm, pitch 0.5 mm, 64
    pins and an exposed pad.">
</picture>
<!-- /figure -->

<!-- generated: chip-card -->
| | | Datasheet |
| --- | --- | --- |
| **Package** | TQFN-64, exposed pad | Pin Configurations, p. 11 |
| **Pins** | 64 | Pin Configurations, p. 11 |
| **Body size** | 9.0 x 9.0 mm | Package Information, p. 10 |
| **Pin pitch** | 0.5 mm | Package Information, p. 10 |
| **Supply VSA** | 7.7 V to 80.0 V | Power Supply - Electrical Characteristics, p. 17 |
| **Supply VS** | 7.7 V to 70.0 V | Power Supply - Electrical Characteristics, p. 17 |
| **Supply VCC_IO** | 2.5 V to 5.0 V | Table 1. Recommended Operating Conditions, p. 9 |

From [`docs/hardware.yml`](docs/hardware.yml); pins, circuits and the block diagram: [Hardware](docs/hardware.md).
<!-- /generated -->

## Quick start

**1. Wire it** as in [Pins and wiring](#pins-and-wiring).

**2. Write the port**: a class with `EnsureInitialized()` (bring the bus up), `Transfer()` (one full-duplex frame
with chip select low; on UART, `Exchange()`) and `DelayUs()`, about 20 lines on most SDKs.
[Porting](docs/porting.md) has the contract and a complete example.

**3. Read a parameter:**

<!-- generated: quickstart-code -->
```cpp
#include "tmc9660.hpp"

int read_actual_velocity(MyPort& port, int32_t& velocity) {
  tmc9660::Config config{};
  config.mode =
      tmc9660::Mode::Parameter; // the chip boots into the mode its boot configuration says
  tmc9660::TMC9660<MyPort> chip(port, config); // no I/O yet

  if (auto r = chip.EnsureInitialized(); !r) { // 1. bus up, identity check, first handshake
    return static_cast<int>(r.error); //    Bus, Timeout or DeviceIdMismatch: check wiring and mode
  }
  const auto v =
      chip.GetParam(tmc9660::Param::ACTUAL_VELOCITY); // 2. a checked read of one parameter
  if (!v) {
    return static_cast<int>(v.error); //    UnexpectedReset: the chip restarted, start again
  }
  velocity = static_cast<int32_t>(v.value); // 3. in the unit of the parameter
  return 0;
}
```
<!-- /generated -->

`EnsureInitialized()` is the only call that starts the driver. Every other call returns `Error::NotInitialized`
until it has succeeded, and no call ever starts or resets the chip on its own.

## Driver coverage

<!-- generated: chip-coverage -->
| Capability | What the chip does | Driver API | Registers | Status | Datasheet |
| --- | --- | --- | --- | --- | --- |
| [FOC and servo loops](docs/parameter-mode.md) | Runs field-oriented current control and the torque, velocity and position loops in hardware | `SetTargetTorque`, `SetTargetVelocity`, `SetTargetPosition`, `MotorStop` | `TARGET_TORQUE`, `TARGET_VELOCITY`, `TARGET_POSITION`, `MCC_PID_CONFIG` | Supported | Motor Control Core Description, Field-Oriented Control, p. 29 |
| [Ramp generator and homing](docs/parameter-mode.md) | Plans eight-point acceleration ramps in real time and searches the reference switches for home | `SetParam`, `ReferenceSearchCommand` | `RAMP_ENABLE`, `RAMP_VMAX`, `RAMP_AMAX`, `REFERENCE_SWITCH_SEARCH_MODE`, `MCC_RAMPER_SWITCH_MODE` | Supported | Motor Control Core Description, Ramp Generator, p. 31 |
| [Position feedback](docs/parameter-mode.md) | Reads Hall sensors, ABN encoders or an SPI encoder for commutation and position | `GetParam`, `SetParam` | `HALL_PHI_E`, `ABN_1_STEPS`, `SPI_ENCODER_TRANSFER`, `MCC_HALL_MODE`, `MCC_ABN_MODE` | Supported | Motor Control Core Description, Encoder Feedback, p. 32 |
| [Smart gate driver](docs/safety.md) | Drives up to four NMOS half-bridges, 1 A source and 2 A sink (switched on only after the fault flags read clear) | `EnableDrive`, `GateDriverStart`, `DisableDrive` | `COMMUTATION_MODE`, `UVW_SOURCE_CURRENT`, `MCC_GDRV_HW`, `MCC_GDRV_CFG` | Supported | Gate Driver Unit Description, p. 41 |
| [Current sensing and measurement](docs/parameter-mode.md) | Amplifies the bottom-shunt currents and measures the supply voltage and the temperatures | `GetParam`, `SetParam` | `CSA_GAIN_ADC_I0_TO_ADC_I2`, `ADC_I0`, `SUPPLY_VOLTAGE`, `CHIP_TEMPERATURE`, `MCC_ADC_I1_I0_RAW` | Supported | Measurement Unit Description, p. 50 |
| [Protections](docs/safety.md) | Overcurrent, gate-short, undervoltage and thermal protection; the flags are read before every start | `ClearFlags`, `Service`, `GetParam` | `GENERAL_ERROR_FLAGS`, `GDRV_ERROR_FLAGS`, `MCC_GDRV_FAULT`, `MCC_GDRV_PROT` | Supported | Protections Description, p. 56 |
| [Bootloader and application modes](docs/bootloader.md) | Sets up pins, clock and LDOs at power-up, then starts parameter mode (TMCL) or register mode | `BlGetInfo`, `BlWriteConfig`, `BlStartApplication`, `GetParam`, `SetParam`, `ReadReg`, `WriteReg` | - | Supported | Bootloader, p. 68 |
| [Hibernation and wake-up](docs/safety.md) | Switches all but the always-on block off; the WAKE pin or a timer powers the chip up again (the drive is switched off first) | `PowerDown`, `CtrlPin::Wake` | - | Supported | Power-Saving Modes, p. 16 |
| [OTP configuration](docs/bootloader.md) | Stores the boot configuration permanently; the driver loads it and never burns it | `BlOtpLoad` | - | Not covered | Bootloader, OTP Memory, p. 76 |

**Supported**: the driver covers it and a host test checks it; **Not covered**: the chip has it and the driver leaves it
alone.

From [`docs/hardware.yml`](docs/hardware.yml); `kokeb-docs hardware --check` confirms that every API name is in
`inc/tmc9660.hpp`, `inc/tmc9660_types.hpp` and every register in `inc/tmc9660_params.hpp`, `inc/tmc9660_registers.hpp`.
<!-- /generated -->

This driver speaks the chip's three protocols (the bootloader, parameter mode and register mode), checks every value
against the reference manuals' tables before it sends a byte, watches for chip resets, and keeps the power stage off
until a guarded call and the chip's own flags say otherwise. It runs on any MCU: you give it a small class that moves
bytes on your bus, and it does the rest.

| | |
| --- | --- |
| **Modes** | Bootloader, parameter mode (289 parameters, TMCL) and register mode (163 registers) |
| **Bus** | SPI mode 3 up to 10 MHz, or UART with automatic baud detection; one bus per driver object |
| **Drives** | RSTN, DRV_EN and WAKE through optional `GpioSet()`; reads FAULTN through optional `GpioGet()` |
| **Safety** | `EnableDrive()` and `GateDriverStart()` are the only ways on; `DisableDrive()` is never refused ([Safety](docs/safety.md)) |
| **Driver** | Header-only C++20; no heap, exceptions, RTTI or virtual calls; every call returns `Result<T>` |
| **Runs on** | Any MCU with SPI or UART and a C++20 compiler; the host example runs on any PC ([Platforms](docs/platforms.md)) |
| **Status** | 0.1.0, not yet verified on hardware: host-tested against an independent model of the chip |

- Refuses a call that belongs to another mode before it sends anything.
- Checks every parameter and register access against the reference manuals' tables: access, range, coded values.
- Starts the application safely: a configuration is written word by word, the boot-mode word last, read back, with
  the clock and PLL checked.
- Switches the drive on only through `EnableDrive()` or `GateDriverStart()`, which read the error flags first, and
  off through `DisableDrive()`, which drops DRV_EN before it tells the chip and is never refused.
- Watches each transport for a chip reset and sends no write after one it saw.

## Documentation

| Page | Read it when you want to |
| --- | --- |
| [Quickstart](docs/quickstart.md) | Go from a bare chip to a parameter read on your MCU |
| [Hardware](docs/hardware.md) | Wire it, check the pins, the bus settings, the timing and the power |
| [Porting](docs/porting.md) | Write the port class for your MCU's SDK, on SPI or UART |
| [API guide](docs/api-guide.md) | Use each call: start, bootloader, parameters, drive, registers, errors |
| [Bootloader](docs/bootloader.md), [Parameter mode](docs/parameter-mode.md), [Register mode](docs/register-mode.md) | The frames and calls of each mode |
| [Safety](docs/safety.md) | What keeps the power stage off and what the driver does not cover |
| [Configuration](docs/configuration.md) | Choose every setting |
| [Ambiguities](docs/ambiguities.md) | Where the documents disagree, the reading chosen and its knob |
| [Troubleshooting](docs/troubleshooting.md) | Find the cause of an error |
| [Contract](docs/contract.md) | Know exactly what the driver promises, needs and returns |
| [Datasheet map](docs/datasheet.md) | Find the document section behind each part of the driver, and the feature coverage |
| [API reference](docs/api-reference.md) | Look up a class or member (Doxygen; built by CI and locally) |

## Datasheet

<!-- generated: datasheet-card -->
| Document | Revision | Get it |
| --- | --- | --- |
| Datasheet: TMC9660 70V Smart Gate Driver with Servo (FOC) Controller in HW and Buck Converter (Analog Devices (Trinamic) 19-101926) | 1, 2024-11 | [vendor page](https://www.analog.com/en/products/tmc9660.html) |
| Datasheet: TMC9660 Parameter Mode Reference Manual (Analog Devices (Trinamic) n/a) | 0, 2025-02 | [vendor page](https://www.analog.com/en/products/tmc9660.html) |
| Datasheet: TMC9660 Register Mode Reference Manual (Analog Devices (Trinamic) n/a) | 0, 2025-04 | [vendor page](https://www.analog.com/en/products/tmc9660.html) |
| Datasheet: TMC9660 Configuration and Bootstrapping (Analog Devices (Trinamic) AN-2601) | 0, 2025-01-08 | [vendor page](https://www.analog.com/en/products/tmc9660.html) |

Written against these revisions; the SHA-256 of each file is in [`docs/datasheets.yml`](docs/datasheets.yml). Where each
part of the driver is in the datasheet: [Datasheet map](docs/datasheet.md).
<!-- /generated -->

Every docs page lists the datasheet sections it relies on, and code comments cite the section they implement.

## Try it on your PC

No hardware needed: [`examples/host`](examples/host/README.md) runs the driver against a simulated chip.
You need git, CMake 3.16+ and a C++20 compiler (GCC 10+, Clang 13+ or MSVC 2022).

1. Get the source:

   ```bash
   git clone https://github.com/kokeb-labs-drivers/driver-tmc9660.git && cd driver-tmc9660
   ```

2. Build and run the example:

   ```bash
   cmake -S examples/host -B build-example
   cmake --build build-example
   ./build-example/tmc9660_host_example
   ```

3. You should see the driver start the simulated chip in its bootloader, start the application in parameter mode, set
   limits, switch the drive on through the guarded call and off again:

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

To run the whole test suite instead: `cmake -S . -B build -DTMC9660_BUILD_TESTS=ON && cmake --build build && ctest
--test-dir build`.

## Add it to your project

Pick the row that matches your build; each is a few steps, and [Installation](docs/installation.md) has the
details.

| Your build | Steps below |
| --- | --- |
| Plain CMake: STM32 (CubeMX), RP2040 (Pico SDK), nRF Connect, a host app | [A](#a-cmake-any-mcu) |
| ESP-IDF | [B](#b-esp-idf) |
| An SDK or OS image that installs libraries | [C](#c-installed-package) |

### A. CMake, any MCU

1. Add the driver as a submodule, pinned to the version you chose:

   ```bash
   git submodule add https://github.com/kokeb-labs-drivers/driver-tmc9660.git external/driver-tmc9660
   ```

2. In your `CMakeLists.txt`, after `project()`:

   ```cmake
   add_subdirectory(external/driver-tmc9660)
   target_link_libraries(my_app PRIVATE tmc9660::tmc9660)   # brings the headers and C++20
   ```

3. In your code, `#include "tmc9660.hpp"`, write the port ([Porting](docs/porting.md)) and build. A first build
   that compiles the quickstart above is the check that the driver is wired in.

### B. ESP-IDF

1. Add the driver under your project's `components/`; ESP-IDF finds it there and names it after the folder:

   ```bash
   git submodule add https://github.com/kokeb-labs-drivers/driver-tmc9660.git components/tmc9660
   ```

2. In `main/CMakeLists.txt`, list it: `idf_component_register(SRCS "main.cpp" REQUIRES tmc9660)`.
3. `idf.py menuconfig` now has a **TMC9660 servo controller** menu (mode, device address, boot timeout, reset),
   and `idf.py build` builds the driver with your app. [`examples/esp32`](examples/esp32) is a complete
   project with a port for the board.

### C. Installed package

1. Install it once into a prefix:

   ```bash
   cmake -S external/driver-tmc9660 -B build-tmc9660 && cmake --install build-tmc9660 --prefix /opt/sdk
   ```

2. In your project, with `CMAKE_PREFIX_PATH=/opt/sdk`:

   ```cmake
   find_package(tmc9660 0.1 REQUIRED)
   target_link_libraries(my_app PRIVATE tmc9660::tmc9660)
   ```

The target is `tmc9660::tmc9660` every way, so switching later changes nothing else.

## Tests and quality

```bash
cmake -S . -B build -DTMC9660_BUILD_TESTS=ON && cmake --build build && ctest --test-dir build
```

The host tests run the driver against an independent model of the chip, fail every bus transfer in turn, and produce
each error the driver can return. A fuzzer fails, corrupts and replaces what the chip sends back and checks the
driver's promises after every call; 55 one-edit mutants of the safety guards must each be caught by a test
(`tools/mutants/mutants.py`); the parameter and register tables are generated and format-stable
(`tools/gen_tables`). CI also builds for Cortex-M and gates coverage, complexity, sanitizers, fuzzing, clang-tidy,
cppcheck and the handbook rules (`.kokeb/handbook.yml`). See [Testing and CI](docs/testing.md), which lists every gate
and its limit.

## License

MIT. See [LICENSE](LICENSE).
