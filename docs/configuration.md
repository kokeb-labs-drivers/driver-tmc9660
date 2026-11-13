# Configuration

`Config` is the whole configuration of the driver; `Config{}` is the chip's reset configuration. Nothing in it is
written to the chip: it says how the driver talks and waits.

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    From Analog Devices (Trinamic) 19-101926 1:

    - Boot configuration list, LDO and bootstrap configuration: Bootloader, Configuration, p. 83

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

## Fields

| Field | Default | Range | What it does |
| --- | --- | --- | --- |
| `mode` | `Mode::Parameter` | `Bootloader`, `Parameter`, `Register` | The protocol the chip is in at `EnsureInitialized()` |
| `device_address` | 1 | 1 to 255, odd for TMCL and register mode on UART | The UART device address (the upper 7 bits are the TMCL address) |
| `host_address` | 255 | 1 to 255 | Host address in bootloader UART replies |
| `tmcl_host_address` | 2 | 1 to 255 | Host address in TMCL and register-mode UART replies |
| `reply_timeout_us` | 50 000 | 1 or more | How long the UART port waits for a reply |
| `boot_timeout_ms` | 500 | 1 or more | Bounds every identity wait after a reset, a start or `Boot` (A9). Each failed poll is charged the 10 ms pause and, on UART, the longest it can take: `reply_timeout_us`, `uart_resync_us` and three 5 ms drains (225 ms with the defaults), so a chip that never answers is given up after about `boot_timeout_ms` plus one poll: two or three polls on UART, 50 on SPI |
| `settle_us` | 20 000 | | Wait after a write that reconfigures the chip (A13) |
| `spi_gap_us` | 1 000 | | Gap between a TMCL SPI command and its reply request |
| `spi_reply_retries` | 5 | 0 to 50 | Repeats of the reply request on `NOT_READY` or `CHECKSUM_ERROR` (A5) |
| `uart_canary` | true | | Parameter mode over UART: detect an unseen reset (A7) |
| `canary_variable` | 15 | 0 to 15 | The user variable that holds the canary |
| `canary_value` | `0x5AC3A55A` | | The canary |
| `hardware_reset` | false | needs `GpioSet()` | Pulse RSTN at start instead of adopting the chip, then wait 50 ms (T_WAKE is 44 ms typical, A9) before the first identity poll; that wait is not charged to `boot_timeout_ms` |
| `faultn_is_reset` | false | needs `GpioGet()` | `Service()` treats FAULTN low outside the bootloader as a reset (A18) |
| `allow_pwm_in_system_off` | false | | Allow `SetParam(IDLE_MOTOR_PWM_BEHAVIOR, 0)` (the PWM stays active in System off). The stop of a port without a DRV_EN hook sets the parameter to 1 whatever this says |
| `uart_resync_us` | 150 000 | | The gap after a failed UART exchange before the line is drained: the chip's discard gap of about 100 ms (A6) with a 50 % margin; a stop is sent before it |
| `quirks` | see below | | Readings of ambiguous documentation |

```cpp
--8<-- "tests/docs/configuration.cpp:custom"
```

## Readings of ambiguous documents

Where the documents disagree the driver reads them one way by default; `Quirks` selects the other
([Ambiguities](ambiguities.md)):

| Field | Default | Ambiguity |
| --- | --- | --- |
| `crc8_reflect_output` | false | A1: the bootloader CRC-8 output reflection |
| `tmcl_layout` | `Auto` | A2: the type bits of a TMCL frame; `Auto` probes with two reads, `MotorHigh` or `TypeHigh` is verified the same way |
| `reg_spi_wide_offset` | false | A3: 9-bit or 10-bit register offset on SPI |
| `reg_uart_msb_fields` | false | A4: field order of register UART datagrams |
| `ram_debug_index_bits` | 5 | A17: 5 or 6 |

```cpp
--8<-- "tests/docs/configuration.cpp:quirks"
```

## Validation

`tmc9660::Validate(config)` checks a configuration without touching the chip, and `EnsureInitialized()` calls it
before any I/O. It returns `Error::InvalidArgument` when `mode` is not an enumerator, an address, timeout or the
canary variable is out of range, or `ram_debug_index_bits` is not 5 or 6. `EnsureInitialized()` also rejects an even
device address on UART in parameter or register mode, and `hardware_reset` without a `GpioSet()` hook (`Unsupported`).

A configuration can be proven at build time, so that a wrong one does not compile:

```cpp
--8<-- "tests/docs/configuration.cpp:checked"
```

## Kconfig

Under ESP-IDF, `idf.py menuconfig` shows **TMC9660 servo controller** when the driver is a component of the project
([Installation](installation.md)); the menu is [`_config/Kconfig`](../_config/Kconfig). The options reach the code only
through [`inc/tmc9660_kconfig.hpp`](../inc/tmc9660_kconfig.hpp), never through a driver signature:

```cpp
--8<-- "tests/docs/configuration.cpp:kconfig"
```

| Option | Default | `Config` field |
| --- | --- | --- |
| `CONFIG_TMC9660_MODE_*` | Parameter mode | `mode` |
| `CONFIG_TMC9660_DEVICE_ADDRESS` | 1 | `device_address` |
| `CONFIG_TMC9660_BOOT_TIMEOUT_MS` | 500 | `boot_timeout_ms` |
| `CONFIG_TMC9660_HARDWARE_RESET` | n | `hardware_reset` |
| `CONFIG_TMC9660_NO_UART_CANARY` | n | `uart_canary` (inverted) |

Without Kconfig (plain CMake, other platforms) `KconfigDefaults()` returns `Config{}`. Both cases are host tests.

Next: [Porting](porting.md).
