/**
 * @file tmc9660_kconfig.hpp
 * @brief Optional ESP-IDF layer: a `Config` built from the `CONFIG_TMC9660_*` Kconfig options.
 * @copyright Copyright (c) 2026 The driver-tmc9660 authors. SPDX-License-Identifier: MIT
 *
 * @details
 * Include this header only where you want menuconfig to choose the defaults; the driver never
 * includes it and no driver signature depends on it. Under ESP-IDF the values come from
 * `sdkconfig.h`. Without Kconfig every option falls back to the `Config{}` default, so
 * `KconfigDefaults()` equals `Config{}`.
 *
 * Kconfig writes a bool option set to "n" as an absent macro, so an absent option means off.
 *
 * One configuration per program: the inline function below depends on the `CONFIG_` macros, so
 * every translation unit of one program must see the same values (handbook KB-CPP20-018).
 */
#ifndef TMC9660_KCONFIG_HPP
#define TMC9660_KCONFIG_HPP

#ifdef __has_include
#if __has_include("sdkconfig.h")
#include "sdkconfig.h"
#endif
#endif

#include "tmc9660_types.hpp"

#ifndef CONFIG_TMC9660_DEVICE_ADDRESS
/** UART device address when Kconfig does not set it: 1, the chip's default. */
#define CONFIG_TMC9660_DEVICE_ADDRESS 1
#endif
#ifndef CONFIG_TMC9660_BOOT_TIMEOUT_MS
/** Wait for the chip after a reset when Kconfig does not set it. */
#define CONFIG_TMC9660_BOOT_TIMEOUT_MS 500
#endif

namespace tmc9660 {

/**
 * @brief The configuration menuconfig selected, on top of the `Config{}` defaults.
 * @return `Config{}` with the mode, device address, boot timeout, reset and canary options from
 * Kconfig.
 */
constexpr Config KconfigDefaults() noexcept {
  Config c{};
#ifdef CONFIG_TMC9660_MODE_BOOTLOADER
  c.mode = Mode::Bootloader;
#elif defined(CONFIG_TMC9660_MODE_REGISTER)
  c.mode = Mode::Register;
#else
  c.mode = Mode::Parameter;
#endif
  c.device_address = static_cast<uint8_t>(CONFIG_TMC9660_DEVICE_ADDRESS);
  c.boot_timeout_ms = static_cast<uint32_t>(CONFIG_TMC9660_BOOT_TIMEOUT_MS);
#if defined(CONFIG_TMC9660_HARDWARE_RESET) && CONFIG_TMC9660_HARDWARE_RESET
  c.hardware_reset = true;
#endif
#if defined(CONFIG_TMC9660_NO_UART_CANARY) && CONFIG_TMC9660_NO_UART_CANARY
  c.uart_canary = false;
#endif
  return c;
}

} // namespace tmc9660

#endif // TMC9660_KCONFIG_HPP
