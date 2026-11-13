/**
 * @file host_tmc9660_port.hpp
 * @brief The host port: the driver's bus hooks over a simulated TMC9660, so the example runs on any
 * PC.
 *
 * `tmc9660_test::Tmc9660Model` (tests/mock_tmc9660.hpp) is the chip model the host tests use. This
 * class connects the driver's hooks to it, as a port for a real bus does: `Transfer()` clocks a
 * datagram through the model, `GpioSet()` drives the model's DRV_EN pin, `DelayUs()` does not sleep
 * and `Log()` writes to the console. A port for a real bus on a PC (Linux spidev, a USB bridge)
 * implements the same hooks: see docs/porting.md.
 */
#ifndef TMC9660_HOST_TMC9660_PORT_HPP
#define TMC9660_HOST_TMC9660_PORT_HPP

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "mock_tmc9660.hpp"
#include "tmc9660.hpp"

/// The host port: simulated chip, real console.
class HostTmc9660Port : public tmc9660::SpiInterface<HostTmc9660Port> {
public:
  /// The bus settings a real port would use, checked against the driver's bus requirements.
  static constexpr uint32_t kClockHz = 1'000'000;
  static constexpr uint8_t kSpiMode = 3;
  static_assert(kClockHz <= tmc9660::kMaxClockHz, "clock above the TMC9660's maximum SPI clock");
  static_assert(kSpiMode == tmc9660::kSpiMode, "the TMC9660 uses SPI mode 3");

  /// The simulated chip; it comes up in its bootloader, as the silicon does.
  tmc9660_test::Tmc9660Model chip;

  /// Nothing to bring up on the host.
  bool EnsureInitialized() noexcept {
    return true;
  }

  /// One datagram, full duplex.
  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    if ((tx == nullptr) || (rx == nullptr) || (len == 0U)) {
      return false;
    }
    chip.SpiTransfer(tx, rx, len);
    return true;
  }

  /// Time does not pass on the host; nothing sleeps.
  void DelayUs(uint32_t us) noexcept {
    static_cast<void>(us);
  }

  /// DRV_EN and RSTN of the simulated chip.
  bool GpioSet(tmc9660::CtrlPin pin, tmc9660::GpioSignal signal) noexcept {
    const bool active = (signal == tmc9660::GpioSignal::ACTIVE);
    if (pin == tmc9660::CtrlPin::DrvEn) {
      chip.SetDrvEn(active);
    } else if (pin == tmc9660::CtrlPin::Reset) {
      chip.ResetPin(active);
    } else {
      // WAKE is not modelled.
    }
    return true;
  }

  /// FAULTN is high: no fault.
  bool GpioGet(tmc9660::CtrlPin pin, bool& high) noexcept {
    static_cast<void>(pin);
    high = true;
    return true;
  }

  /// Driver messages (they carry their own `tmc9660:` prefix) go to the console, one line each.
  void Log(tmc9660::LogLevel level, const char* message) noexcept {
    static_cast<void>(level);
    static_cast<void>(std::puts(message)); // plain text: never pass it as a format string
  }
};

#endif // TMC9660_HOST_TMC9660_PORT_HPP
