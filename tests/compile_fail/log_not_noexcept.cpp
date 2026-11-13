/**
 * @file log_not_noexcept.cpp
 * @brief Must NOT compile: an optional Log hook that is not noexcept is rejected.
 *
 * EXPECT: Log hook must be
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
  void Log(tmc9660::LogLevel level, const char* message) {
    static_cast<void>(level);
    static_cast<void>(message);
  }
};

int main() {
  P port;
  tmc9660::TMC9660<P> chip(port);
  return static_cast<int>(chip.EnsureInitialized().error);
}
