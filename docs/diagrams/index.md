# Figures

<!-- Written by kokeb-docs figures from docs/diagrams/figures/*.py. Do not edit by hand. -->

Every figure on this site is drawn by a Python module from facts extracted from the code, so it is rebuilt,
not redrawn, when the code changes. CI runs `kokeb-docs figures --check` and fails when a
committed figure no longer matches the code. Each figure below lists the files it is drawn from.

## Inside the TMC9660

<!-- figure: chip-blocks -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-blocks.dark.svg">
  <img src="chip-blocks.svg"
    alt="Block diagram of the TMC9660: its function blocks, how they connect and which package pins reach
    each block.">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/tmc9660_spi_interface.hpp`](../../inc/tmc9660_spi_interface.hpp)
- [`inc/tmc9660_types.hpp`](../../inc/tmc9660_types.hpp)
- [`inc/tmc9660_uart_interface.hpp`](../../inc/tmc9660_uart_interface.hpp)

## What the TMC9660 does

<!-- figure: chip-capabilities -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-capabilities.dark.svg">
  <img src="chip-capabilities.svg"
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

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/tmc9660_spi_interface.hpp`](../../inc/tmc9660_spi_interface.hpp)
- [`inc/tmc9660_types.hpp`](../../inc/tmc9660_types.hpp)
- [`inc/tmc9660_uart_interface.hpp`](../../inc/tmc9660_uart_interface.hpp)

## TMC9660: Typical application

<!-- figure: chip-circuit-typical -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-circuit-typical.dark.svg">
  <img src="chip-circuit-typical.svg"
    alt="Schematic of the TMC9660 typical application: the chip with U2 MCU, C1 capacitor ≥ 0.5 µF, C11
    capacitor 10 µF, C2 capacitor ≥ 1 µF, C3 capacitor 1 µF, C4 capacitor 10 µF, C5 capacitor 10 µF, C6
    capacitor 8 µF, C7 capacitor 4.7 µF, C8 capacitor 220 nF, C9 capacitor 1.6 µF, C10 capacitor 1.6 µF,
    L1 inductor 27 µH, R1 resistor 10 kΩ; the MCU wires carry the port hooks GpioGet(CtrlPin::Faultn),
    GpioSet(CtrlPin::DrvEn), GpioSet(CtrlPin::Reset), GpioSet(CtrlPin::Wake), Transfer().">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/tmc9660_spi_interface.hpp`](../../inc/tmc9660_spi_interface.hpp)
- [`inc/tmc9660_types.hpp`](../../inc/tmc9660_types.hpp)
- [`inc/tmc9660_uart_interface.hpp`](../../inc/tmc9660_uart_interface.hpp)

## TMC9660: Parameter-mode SPI datagram

<!-- figure: chip-frame-tmcl-spi -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-frame-tmcl-spi.dark.svg">
  <img src="chip-frame-tmcl-spi.svg"
    alt="Parameter-mode SPI datagram of the TMC9660, 64 bits, most significant bit first. Command: operation
    (bits 63 to 56), type and motor or bank (bits 55 to 40), data (big endian) (bits 39 to 8), checksum
    (bits 7 to 0); Reply: SPI status (bits 63 to 56), TMCL status (bits 55 to 48), operation (bits 47 to
    40), data (big endian) (bits 39 to 8), checksum (bits 7 to 0). Below, the SPI mode 3 waveform of one
    transfer in the same bit grid.">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/tmc9660_spi_interface.hpp`](../../inc/tmc9660_spi_interface.hpp)
- [`inc/tmc9660_types.hpp`](../../inc/tmc9660_types.hpp)
- [`inc/tmc9660_uart_interface.hpp`](../../inc/tmc9660_uart_interface.hpp)

## TMC9660: Parameter-mode UART datagram

<!-- figure: chip-frame-tmcl-uart -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-frame-tmcl-uart.dark.svg">
  <img src="chip-frame-tmcl-uart.svg"
    alt="Parameter-mode UART datagram of the TMC9660, 72 bits, sent byte by byte, each byte least significant
    bit first, bit 0 first. Command: sync bit and module address (bits 0 to 7), operation (bits 8 to
    15), type (bits 16 to 27), motor or bank (bits 28 to 31), data (big endian) (bits 32 to 63),
    checksum (bits 64 to 71); Reply: host address (bits 0 to 7), sync bit and module address (bits 8 to
    15), TMCL status (bits 16 to 23), operation (bits 24 to 31), data (big endian) (bits 32 to 63),
    checksum (bits 64 to 71).">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/tmc9660_spi_interface.hpp`](../../inc/tmc9660_spi_interface.hpp)
- [`inc/tmc9660_types.hpp`](../../inc/tmc9660_types.hpp)
- [`inc/tmc9660_uart_interface.hpp`](../../inc/tmc9660_uart_interface.hpp)

## TMC9660

<!-- figure: chip-hero -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-hero.dark.svg">
  <img src="chip-hero.svg"
    alt="Overview of the TMC9660, a gate driver and motor controller for BLDC, PMSM, stepper and brushed DC
    motors: field-oriented control, the servo loops and a ramp generator run in hardware, and a buck
    converter powers the chip from the motor supply. A MCU talks to it over SPI or UART; it drives
    MOSFET half-bridges, then Motor; it reads back Phase currents, Position sensor, Reference switches.
    Headline figures: 7.7 to 70 V single-supply operating range; 1 A / 2 A gate drive source and sink,
    up to four half-bridges; 10 MHz SPI clock, mode 3; 9 x 9 mm 64-pin TQFN body.">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/tmc9660_spi_interface.hpp`](../../inc/tmc9660_spi_interface.hpp)
- [`inc/tmc9660_types.hpp`](../../inc/tmc9660_types.hpp)
- [`inc/tmc9660_uart_interface.hpp`](../../inc/tmc9660_uart_interface.hpp)

## TMC9660 package, TQFN-64

<!-- figure: chip-outline -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-outline.dark.svg">
  <img src="chip-outline.svg"
    alt="Outline of the TMC9660 in TQFN-64 drawn to scale from above: body 9.0 x 9.0 mm, pitch 0.5 mm, 64
    pins and an exposed pad.">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/tmc9660_spi_interface.hpp`](../../inc/tmc9660_spi_interface.hpp)
- [`inc/tmc9660_types.hpp`](../../inc/tmc9660_types.hpp)
- [`inc/tmc9660_uart_interface.hpp`](../../inc/tmc9660_uart_interface.hpp)

## TMC9660 pinout, TQFN-64

<!-- figure: chip-pinout -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-pinout.dark.svg">
  <img src="chip-pinout.svg"
    alt="Pinout of the TMC9660 in TQFN-64, top view: 64 pins and an exposed pad, each with its name, its
    electrical type as a colour and the driver hook that uses it.">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/tmc9660_spi_interface.hpp`](../../inc/tmc9660_spi_interface.hpp)
- [`inc/tmc9660_types.hpp`](../../inc/tmc9660_types.hpp)
- [`inc/tmc9660_uart_interface.hpp`](../../inc/tmc9660_uart_interface.hpp)

## Wiring: Typical application

<!-- figure: chip-wiring-typical -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-wiring-typical.dark.svg">
  <img src="chip-wiring-typical.svg"
    alt="Wiring of the TMC9660 for the Typical application: MCU SPI SCK to GPIO11 (Transfer()); MCU SPI MOSI
    to GPIO10 (Transfer()); MCU SPI MISO to GPIO9 (Transfer()); MCU GPIO / CS to GPIO12 (Transfer());
    MCU GPIO to RSTN (GpioSet(CtrlPin::Reset)); MCU GPIO to DRV_EN (GpioSet(CtrlPin::DrvEn)); MCU GPIO
    (IRQ) to FAULTN (GpioGet(CtrlPin::Faultn)). Supplies, grounds and the support parts are listed under
    the drawing.">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/tmc9660_spi_interface.hpp`](../../inc/tmc9660_spi_interface.hpp)
- [`inc/tmc9660_types.hpp`](../../inc/tmc9660_types.hpp)
- [`inc/tmc9660_uart_interface.hpp`](../../inc/tmc9660_uart_interface.hpp)

## The layers, one of them yours

<!-- figure: driver-layers -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="driver-layers.dark.svg">
  <img src="driver-layers.svg"
    alt="Layer diagram: your application calls the TMC9660 core, which sends register reads and writes
    through the SPI or UART port contract to the port class you write, which drives the chip; the core
    and the contract are the same on every MCU.">
</picture>
<!-- /figure -->

Figure module: [`docs/diagrams/figures/driver-layers.py`](../../docs/diagrams/figures/driver-layers.py)

Drawn from:

- `public calls` in [`inc/tmc9660.hpp`](../../inc/tmc9660.hpp)
- `SpiInterface` in [`inc/tmc9660_spi_interface.hpp`](../../inc/tmc9660_spi_interface.hpp)
- `UartInterface` in [`inc/tmc9660_uart_interface.hpp`](../../inc/tmc9660_uart_interface.hpp)

## SPI bus timing

<!-- figure: spi-timing -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="spi-timing.dark.svg">
  <img src="spi-timing.svg"
    alt="SPI timing of the TMC9660 in mode 3: SPI0_CS0 low frames a datagram, MOSI is read on each rising
    SPI0_SCK edge and MISO changes after each falling edge, with the select, setup and hold times the
    datasheet gives.">
</picture>
<!-- /figure -->

Hardware description: [`docs/model/spi-timing.timing.yml`](../model/spi-timing.timing.yml)

Drawn from:

- [`docs/model/spi-timing.timing.yml`](../model/spi-timing.timing.yml)

## TMCL command and reply on SPI

<!-- figure: tmcl-spi -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="tmcl-spi.dark.svg">
  <img src="tmcl-spi.svg"
    alt="A parameter-mode TMCL exchange on SPI: the reply to a 64-bit command comes back on MISO during the
    next 64-bit datagram, while the first datagram returns the reply to the one before it.">
</picture>
<!-- /figure -->

Hardware description: [`docs/model/tmcl-spi.timing.yml`](../model/tmcl-spi.timing.yml)

Drawn from:

- [`docs/model/tmcl-spi.timing.yml`](../model/tmcl-spi.timing.yml)

## UART character and transceiver enable

<!-- figure: uart-timing -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="uart-timing.dark.svg">
  <img src="uart-timing.svg"
    alt="One UART character sent by the TMC9660: a start bit, eight data bits least significant first and a
    stop bit, with UART_TXEN raised before the start bit and dropped after the stop bit for an RS-485
    transceiver.">
</picture>
<!-- /figure -->

Hardware description: [`docs/model/uart-timing.timing.yml`](../model/uart-timing.timing.yml)

Drawn from:

- [`docs/model/uart-timing.timing.yml`](../model/uart-timing.timing.yml)

## TMC9660 nodes on RS-485

<!-- figure: uart-topology -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="uart-topology.dark.svg">
  <img src="uart-topology.svg"
    alt="Several TMC9660 on one RS-485 bus: the host and every chip share the DATA+ and DATA- pair through
    transceivers, and each chip answers only commands sent to its own odd module address.">
</picture>
<!-- /figure -->

Hardware description: [`docs/model/uart-topology.topology.yml`](../model/uart-topology.topology.yml)

Drawn from:

- [`docs/model/uart-topology.topology.yml`](../model/uart-topology.topology.yml)
