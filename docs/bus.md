# On the bus

What the TMC9660 needs from its host link, signal by signal, with every time and rate taken from the datasheet and the
two reference manuals. The datagram contents are on the [Parameter mode](parameter-mode.md),
[Register mode](register-mode.md) and [Bootloader](bootloader.md) pages; this page is the waveform under them.

## SPI timing

The TMC9660 is an SPI peripheral in mode 3 on SPI0_CS0, SPI0_SCK, SPI0_MOSI and SPI0_MISO: it reads MOSI on the rising
edge of SCK and shifts MISO out on the falling edge, up to 10 MHz. The bootloader takes 40-bit datagrams, parameter
mode 64-bit and register mode 48-bit ones, all MSB first and all with the same timing.

<!-- figure: spi-timing -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="diagrams/spi-timing.dark.svg">
  <img src="diagrams/spi-timing.svg"
    alt="SPI timing of the TMC9660 in mode 3: SPI0_CS0 low frames a datagram, MOSI is read on each rising
    SPI0_SCK edge and MISO changes after each falling edge, with the select, setup and hold times the
    datasheet gives.">
</picture>
<!-- /figure -->

In parameter mode every 64-bit datagram returns the reply to the one before it, so a command needs a second datagram
for its reply; while the chip is busy the status says not ready and the datagram is sent again.

<!-- figure: tmcl-spi -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="diagrams/tmcl-spi.dark.svg">
  <img src="diagrams/tmcl-spi.svg"
    alt="A parameter-mode TMCL exchange on SPI: the reply to a 64-bit command comes back on MISO during the
    next 64-bit datagram, while the first datagram returns the reply to the one before it.">
</picture>
<!-- /figure -->

## UART

The UART is 8N1, least significant bit first, idle high. The chip detects the baud rate on its own from 9600 to
250000 bit/s at its 40 MHz system clock; higher rates up to 5 Mbit/s need a fixed setting. A TMCL datagram is nine
characters in both directions, and the chip only ever answers a command. For an RS-485 transceiver the chip drives
UART_TXEN around its own characters.

<!-- figure: uart-timing -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="diagrams/uart-timing.dark.svg">
  <img src="diagrams/uart-timing.svg"
    alt="One UART character sent by the TMC9660: a start bit, eight data bits least significant first and a
    stop bit, with UART_TXEN raised before the start bit and dropped after the stop bit for an RS-485
    transceiver.">
</picture>
<!-- /figure -->

## Several devices

SPI has no daisy chain: each TMC9660 needs its own chip select (the peripheral supports CS0 only). On UART several
chips can share one RS-485 bus: each answers only its own module address, an odd value from 1 to 255.

<!-- figure: uart-topology -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="diagrams/uart-topology.dark.svg">
  <img src="diagrams/uart-topology.svg"
    alt="Several TMC9660 on one RS-485 bus: the host and every chip share the DATA+ and DATA- pair through
    transceivers, and each chip answers only commands sent to its own odd module address.">
</picture>
<!-- /figure -->

## How these figures are made

Each figure is a model in `docs/model/` (`spi-timing.timing.yml`, `tmcl-spi.timing.yml`, `uart-timing.timing.yml` and
`uart-topology.topology.yml`) that cites the document section and page of every number; `kokeb-docs figures` draws it
and the build fails on a number without a citation. The values table under each waveform is also written as
`docs/diagrams/<id>.table.html`.
