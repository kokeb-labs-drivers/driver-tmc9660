/**
 * @file tmc9660_basic_example.cpp
 * @brief Start the TMC9660 from the menuconfig values, log its identity and the actual velocity
 * twice a second.
 *
 * Wiring: examples/esp32/README.md. The example never switches the gate driver on: to run a motor,
 * call `EnableDrive()` after the motor parameters are set, as docs/parameter-mode.md shows. Every
 * Result is checked and logged by name.
 */
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp32_tmc9660_port.hpp"
#include "tmc9660.hpp"
#include "tmc9660_kconfig.hpp"

namespace {
constexpr const char* kTag = "tmc9660_example";
}

extern "C" void app_main(void) {
  static Esp32Tmc9660Port port; // static: the driver keeps a reference
  static tmc9660::TMC9660<Esp32Tmc9660Port> chip(port, tmc9660::KconfigDefaults()); // no I/O

  ESP_LOGI(kTag, "TMC9660 driver %s", tmc9660::GetDriverVersion());
  const auto init = chip.EnsureInitialized();
  if (!init) {
    ESP_LOGE(kTag, "EnsureInitialized failed: %s", tmc9660::ToString(init.error));
    return;
  }
  if (chip.GetMode() == tmc9660::Mode::Parameter) {
    const auto id = chip.GetChipId();
    ESP_LOGI(kTag, "initialised, chip type 0x%08X", static_cast<unsigned>(id.value));
  } else {
    ESP_LOGI(kTag, "initialised in mode %d", static_cast<int>(chip.GetMode()));
  }

  while (true) {
    const auto alive = chip.Service(); // FAULTN and a heartbeat read: the way to notice a reset
    if (!alive) {
      ESP_LOGW(kTag, "service: %s", tmc9660::ToString(alive.error));
      if (alive.error == tmc9660::Error::UnexpectedReset) {
        static_cast<void>(chip.DisableDrive()); // the pin first: never leave the outputs on
        static_cast<void>(
            chip.EnsureInitialized()); // the chip lost its configuration: start it again
      } else {
        // Any other error: the next call reports again.
      }
    } else if (chip.GetMode() == tmc9660::Mode::Parameter) {
      const auto v = chip.GetParam(tmc9660::Param::ACTUAL_VELOCITY);
      if (v) {
        ESP_LOGI(kTag, "actual velocity %d", static_cast<int>(v.value));
      } else {
        ESP_LOGW(kTag, "read failed: %s", tmc9660::ToString(v.error));
      }
    } else {
      // Bootloader or register mode: see the other pages of the documentation.
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
