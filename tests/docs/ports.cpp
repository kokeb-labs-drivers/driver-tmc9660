/**
 * @file ports.cpp
 * @brief The ports with every optional hook, as shown on the Porting page. Compiled, never run.
 */
#include <cstddef>
#include <cstdint>

#include "tmc9660.hpp"

bool platform_spi_init() noexcept;
bool platform_spi_transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept;
void platform_delay_us(uint32_t us) noexcept;
bool platform_gpio_write(int pin, bool high) noexcept;
bool platform_gpio_read(int pin, bool& high) noexcept;
void platform_log(const char* line) noexcept;

// --8<-- [start:full-port]
class FullPort : public tmc9660::SpiInterface<FullPort> {
public:
  bool EnsureInitialized() noexcept {
    return platform_spi_init();
  }
  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    return platform_spi_transfer(tx, rx, len);
  }

  // Required by every call that waits (the reply gap, boot and PLL times).
  void DelayUs(uint32_t us) noexcept {
    platform_delay_us(us);
  }

  // Optional: RSTN and DRV_EN on GPIOs. ACTIVE means "asserted": RSTN is active low, DRV_EN active
  // high.
  bool GpioSet(tmc9660::CtrlPin pin, tmc9660::GpioSignal signal) noexcept {
    const bool active = (signal == tmc9660::GpioSignal::ACTIVE);
    if (pin == tmc9660::CtrlPin::Reset) {
      return platform_gpio_write(kResetGpio, !active);
    }
    if (pin == tmc9660::CtrlPin::DrvEn) {
      return platform_gpio_write(kDrvEnGpio, active);
    }
    return false; // WAKE is not wired
  }

  // Optional: FAULTN, an open-drain output of the chip. `high` is the pin level.
  bool GpioGet(tmc9660::CtrlPin pin, bool& high) noexcept {
    if (pin != tmc9660::CtrlPin::Faultn) {
      return false;
    }
    return platform_gpio_read(kFaultnGpio, high);
  }

  // Optional: one line per lifecycle event or failure. The driver never formats; the port does.
  void Log(tmc9660::LogLevel level, const char* message) noexcept {
    if (level > tmc9660::LogLevel::Warning) {
      return;
    }
    platform_log(message); // plain text: never use it as a format string
  }

private:
  static constexpr int kResetGpio = 7;
  static constexpr int kDrvEnGpio = 8;
  static constexpr int kFaultnGpio = 9;
};
// --8<-- [end:full-port]

// --8<-- [start:hardware-reset]
tmc9660::Config WithHardwareReset() {
  tmc9660::Config c{};
  c.hardware_reset =
      true; // the start pulses RSTN (after DRV_EN low); a port without GpioSet gets Unsupported
  return c;
}
// --8<-- [end:hardware-reset]
