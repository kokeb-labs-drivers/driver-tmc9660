# TMC9660 ESP32 examples

ESP-IDF applications that run the TMC9660 driver on an ESP32-C6, built by CI on every pull request. They need real
hardware to run; nothing here has been run on a chip yet (see `docs/datasheet.md`, "Not verified").

## Wiring (an ESP32-C6 development board)

| ESP32-C6 | TMC9660 | Notes |
| --- | --- | --- |
| GPIO6 | SCK | SPI2, mode 3, 1 MHz in the example (up to 10 MHz) |
| GPIO7 | SDI (MOSI) | |
| GPIO2 | SDO (MISO) | |
| GPIO10 | CSN | Driven by spi_master, one transfer per datagram |
| GPIO11 | RSTN | Optional; set `reset_n = -1` in `Esp32Tmc9660Pins` when tied high |
| GPIO4 | DRV_EN | Driven low at start; the driver raises it only inside `EnableDrive()` and `GateDriverStart()` |
| GPIO5 | FAULTN | Optional input with pull-up (open drain output of the chip) |
| 3V3 | VIO / logic supply | As the chip's data sheet requires |
| GND | GND | |

The bridge and motor supply, the gate driver wiring and the motor are not part of this example, which never switches
the gate driver on.

## Build and flash

```bash
git submodule update --init scripts      # the ESP-IDF build scripts
. $IDF_PATH/export.sh                     # ESP-IDF v5.5
./scripts/build_app.sh list
./scripts/build_app.sh tmc9660_basic_example Debug
./scripts/flash_app.sh flash_monitor tmc9660_basic_example Debug
```

`idf.py menuconfig` shows "TMC9660 servo controller" under Component config: the start mode, the UART device address, the
boot timeout, the hardware reset and the UART canary that `tmc9660::KconfigDefaults()` returns.

## Expected output

```text
I (312) tmc9660_example: TMC9660 driver 0.1.0
tmc9660: initialised
I (330) tmc9660_example: initialised, chip type 0x544D0001
I (832) tmc9660_example: actual velocity 0
```

An `EnsureInitialized failed: DeviceIdMismatch` line means the chip type did not read 0x544D0001: check CSN, the
SPI mode (3) and the MISO wire first. `Timeout` means the chip is not in the mode the Kconfig value expects.

## Files

| File | What it is |
| --- | --- |
| [`main/tmc9660_basic_example.cpp`](main/tmc9660_basic_example.cpp) | The example app |
| [`main/esp32_tmc9660_port.hpp`](main/esp32_tmc9660_port.hpp) | The ESP-IDF port: bus, RSTN, DRV_EN, FAULTN, delay, log |
| [`CMakeLists.txt`](CMakeLists.txt) | The ESP-IDF project; `EXTRA_COMPONENT_DIRS` points at the repository root, which is the component |
| [`app_config.yml`](app_config.yml) | The app matrix for the scripts, CI and the docs |
