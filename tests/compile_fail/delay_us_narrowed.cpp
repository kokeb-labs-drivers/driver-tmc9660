/**
 * @file delay_us_narrowed.cpp
 * @brief Must NOT compile: a DelayUs hook with a narrower parameter would truncate long waits, so
 * the driver rejects it (KB-CPP20-007).
 *
 * EXPECT: DelayUs hook must be
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
  void DelayUs(uint16_t us) noexcept {
    static_cast<void>(us);
  }
};

int main() {
  P port;
  tmc9660::TMC9660<P> chip(port);
  return static_cast<int>(chip.EnsureInitialized().error);
}
