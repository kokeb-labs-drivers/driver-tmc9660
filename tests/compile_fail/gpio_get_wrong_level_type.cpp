/**
 * @file gpio_get_wrong_level_type.cpp
 * @brief Must NOT compile: GpioGet must report the level through a bool reference; a by-value
 * parameter could not return it.
 *
 * EXPECT: GpioGet hook must be
 */
#include <cstddef>
#include <cstdint>

#include "tmc9660.hpp"

class P : public tmc9660::SpiInterface<P> {
public:
  bool EnsureInitialized() noexcept {
    return true;
  }
  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    static_cast<void>(tx);
    static_cast<void>(rx);
    return len > 0U;
  }
  void DelayUs(uint32_t us) noexcept {
    static_cast<void>(us);
  }
  bool GpioGet(tmc9660::CtrlPin pin, bool high) noexcept {
    static_cast<void>(pin);
    return high;
  }
};

int main() {
  P port;
  tmc9660::TMC9660<P> chip(port);
  return static_cast<int>(chip.EnsureInitialized().error);
}
