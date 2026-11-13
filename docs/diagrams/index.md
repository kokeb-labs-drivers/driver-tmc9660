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
