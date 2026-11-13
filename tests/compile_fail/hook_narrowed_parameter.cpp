/**
 * @file hook_narrowed_parameter.cpp
 * @brief Must NOT compile: a Transfer hook with a narrower length would truncate every transfer, so
 * the port concept rejects it (KB-CPP20-006).
 *
 * EXPECT: SpiPort|AnyPort
 */
#include <cstddef>
#include <cstdint>

#include "tmc9660.hpp"

class P : public tmc9660::SpiInterface<P> {
public:
  bool EnsureInitialized() noexcept {
    return true;
  }
  bool Transfer(const uint8_t* tx, uint8_t* rx, uint16_t len) noexcept {
    static_cast<void>(tx);
    static_cast<void>(rx);
    return len > 0U;
  }
  void DelayUs(uint32_t us) noexcept {
    static_cast<void>(us);
  }
};

int main() {
  P port;
  tmc9660::TMC9660<P> chip(port);
  return static_cast<int>(chip.EnsureInitialized().error);
}
