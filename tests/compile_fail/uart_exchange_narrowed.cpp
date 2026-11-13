/**
 * @file uart_exchange_narrowed.cpp
 * @brief Must NOT compile: an Exchange hook with a narrower timeout would truncate it, so the port
 * concept rejects it.
 *
 * EXPECT: UartPort|AnyPort
 */
#include <cstddef>
#include <cstdint>

#include "tmc9660.hpp"

class P : public tmc9660::UartInterface<P> {
public:
  bool EnsureInitialized() noexcept {
    return true;
  }
  tmc9660::BusStatus Exchange(const uint8_t* tx, std::size_t tx_len, uint8_t* rx,
                              std::size_t rx_len, uint16_t timeout_us) noexcept {
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
