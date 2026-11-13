# TMC9660 Driver API reference {#mainpage}

The classes, types and registers of the TMC9660 driver, generated from the headers in `inc/`.

Start with tmc9660::TMC9660, the driver, and tmc9660::SpiInterface or tmc9660::UartInterface, the base of the
port you write. Errors are tmc9660::Error inside tmc9660::Result; the configuration is tmc9660::Config.

The guides, with examples that compile, are on the documentation site: <a href="../">TMC9660 Driver
documentation</a>.
