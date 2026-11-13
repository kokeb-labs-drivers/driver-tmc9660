/**
 * @file both_buses.cpp
 * @brief Must NOT compile: a port is on the SPI bus or on the UART, never both: the chip answers on
 * one at a time.
 *
 * EXPECT: AnyPort|constraints not satisfied
 */
#include <cstddef>
#include <cstdint>

#include "tmc9660.hpp"

class P : public tmc9660::SpiInterface<P>, public tmc9660::UartInterface<P> {
public:
  bool EnsureInitialized() noexcept {
    return true;
  }
  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    static_cast<void>(tx);
    static_cast<void>(rx);
    return len > 0U;
  }
  tmc9660::BusStatus Exchange(const uint8_t* tx, std::size_t tx_len, uint8_t* rx,
                              std::size_t rx_len, uint32_t timeout_us) noexcept {
    static_cast<void>(tx);
    static_cast<void>(rx);
    static_cast<void>(tx_len);
    static_cast<void>(rx_len);
    static_cast<void>(timeout_us);
    return tmc9660::BusStatus::Ok;
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
