/**
 * @file esp32_tmc9660_port.hpp
 * @brief ESP-IDF port for the TMC9660 driver: spi_master for the bus, GPIO for RSTN, DRV_EN and
 * FAULTN, esp_rom delays and ESP_LOG for the log hook.
 *
 * Start from this file for any ESP-IDF target. The pins are in Esp32Tmc9660Pins; the defaults match
 * the wiring table in examples/esp32/README.md (an ESP32-C6 development board). DRV_EN is driven
 * low before its output is enabled, so that configuring the pin never energises the gate driver.
 */
#ifndef TMC9660_ESP32_TMC9660_PORT_HPP
#define TMC9660_ESP32_TMC9660_PORT_HPP

#include <cstddef>
#include <cstdint>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_rom_sys.h"

#include "tmc9660.hpp"

/// The port's default SPI clock: 1 MHz for bring-up on jumper wires.
inline constexpr int kEsp32Tmc9660ClockHz = 1'000'000;
// The bus requirements of the driver, checked at compile time.
static_assert(kEsp32Tmc9660ClockHz <= static_cast<int>(tmc9660::kMaxClockHz),
              "the default clock is above the TMC9660's maximum SPI clock");
static_assert(tmc9660::kSpiMode == 3, "the TMC9660 uses SPI mode 3");
static_assert(tmc9660::kMsbFirst, "this port sends MSB first (no SPI_DEVICE_BIT_LSBFIRST)");

/// Pins and bus settings of the ESP-IDF port; -1 leaves a pin unused.
struct Esp32Tmc9660Pins {
  spi_host_device_t host = SPI2_HOST;
  int sclk = 6;
  int mosi = 7;
  int miso = 2;
  int cs = 10;
  int reset_n = 11; ///< RSTN, active low. -1 when tied high.
  int drv_en = 4;   ///< DRV_EN, active high. -1 when wired by hand: the driver then cannot drop it.
  int faultn = 5;   ///< FAULTN, open drain, active low. -1 when not wired.
  int clock_hz = kEsp32Tmc9660ClockHz; ///< Up to `tmc9660::kMaxClockHz`; checked at start.
};

/// The TMC9660 SPI port on ESP-IDF, with every optional hook.
class Esp32Tmc9660Port : public tmc9660::SpiInterface<Esp32Tmc9660Port> {
public:
  explicit Esp32Tmc9660Port(const Esp32Tmc9660Pins& pins = {}) noexcept : pins_(pins) {}

  bool EnsureInitialized() noexcept {
    if (device_ != nullptr) {
      return true;
    }
    if ((pins_.clock_hz <= 0) || (pins_.clock_hz > static_cast<int>(tmc9660::kMaxClockHz))) {
      ESP_LOGE(kTag, "clock outside the TMC9660's bus requirements");
      return false;
    }
    // Outputs first, at their safe levels: DRV_EN low (gate driver off), RSTN high (no reset
    // pulse).
    if (!outputPin(pins_.drv_en, 0U) || !outputPin(pins_.reset_n, 1U)) {
      return false;
    }
    if (pins_.faultn >= 0) {
      gpio_config_t io{};
      io.pin_bit_mask = 1ULL << static_cast<unsigned>(pins_.faultn);
      io.mode = GPIO_MODE_INPUT;
      io.pull_up_en = GPIO_PULLUP_ENABLE; // FAULTN is open drain
      if (gpio_config(&io) != ESP_OK) {
        return false;
      }
    }
    spi_bus_config_t bus{};
    bus.mosi_io_num = pins_.mosi;
    bus.miso_io_num = pins_.miso;
    bus.sclk_io_num = pins_.sclk;
    bus.quadwp_io_num = -1;
    bus.quadhd_io_num = -1;
    bus.max_transfer_sz = 16;
    const esp_err_t bus_err = spi_bus_initialize(pins_.host, &bus, SPI_DMA_DISABLED);
    if ((bus_err != ESP_OK) &&
        (bus_err != ESP_ERR_INVALID_STATE)) { // INVALID_STATE: bus already up
      ESP_LOGE(kTag, "spi_bus_initialize: %s", esp_err_to_name(bus_err));
      return false;
    }
    spi_device_interface_config_t dev{};
    dev.clock_speed_hz = pins_.clock_hz;
    dev.mode = tmc9660::kSpiMode;
    dev.spics_io_num = pins_.cs;
    dev.queue_size = 1;
    if (spi_bus_add_device(pins_.host, &dev, &device_) != ESP_OK) {
      device_ = nullptr;
      return false;
    }
    return true;
  }

  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    if ((device_ == nullptr) || (len == 0U)) {
      return false;
    }
    spi_transaction_t t{};
    t.length = len * 8U;
    t.tx_buffer = tx;
    t.rx_buffer = rx;
    return spi_device_polling_transmit(device_, &t) == ESP_OK;
  }

  void DelayUs(uint32_t us) noexcept {
    esp_rom_delay_us(us);
  }

  bool GpioSet(tmc9660::CtrlPin pin, tmc9660::GpioSignal signal) noexcept {
    const bool active = (signal == tmc9660::GpioSignal::ACTIVE);
    if (pin == tmc9660::CtrlPin::Reset) {
      return setLevel(pins_.reset_n, active ? 0U : 1U); // RSTN is active low
    }
    if (pin == tmc9660::CtrlPin::DrvEn) {
      return setLevel(pins_.drv_en, active ? 1U : 0U); // DRV_EN is active high
    }
    return false; // WAKE is not wired in this port
  }

  bool GpioGet(tmc9660::CtrlPin pin, bool& high) noexcept {
    if ((pin != tmc9660::CtrlPin::Faultn) || (pins_.faultn < 0)) {
      return false;
    }
    high = (gpio_get_level(static_cast<gpio_num_t>(pins_.faultn)) != 0);
    return true;
  }

  void Log(tmc9660::LogLevel level, const char* message) noexcept {
    esp_log_level_t esp_level = ESP_LOG_DEBUG;
    if (level == tmc9660::LogLevel::Error) {
      esp_level = ESP_LOG_ERROR;
    } else if (level == tmc9660::LogLevel::Warning) {
      esp_level = ESP_LOG_WARN;
    } else if (level == tmc9660::LogLevel::Info) {
      esp_level = ESP_LOG_INFO;
    } else {
      // Debug
    }
    esp_log_write(esp_level, kTag, "%s\n", message); // plain text, never a format
  }

private:
  static bool outputPin(int pin, uint32_t level) noexcept {
    if (pin < 0) {
      return true;
    }
    gpio_config_t io{};
    io.pin_bit_mask = 1ULL << static_cast<unsigned>(pin);
    io.mode = GPIO_MODE_OUTPUT;
    // The level is set before the output is enabled, so configuring the pin never glitches it.
    return (gpio_set_level(static_cast<gpio_num_t>(pin), level) == ESP_OK) &&
           (gpio_config(&io) == ESP_OK);
  }
  static bool setLevel(int pin, uint32_t level) noexcept {
    return (pin >= 0) && (gpio_set_level(static_cast<gpio_num_t>(pin), level) == ESP_OK);
  }

  static constexpr const char* kTag = "tmc9660";
  Esp32Tmc9660Pins pins_;
  spi_device_handle_t device_ = nullptr;
};

#endif // TMC9660_ESP32_TMC9660_PORT_HPP
