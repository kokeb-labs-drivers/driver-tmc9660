/**
 * @file my_port.hpp
 * @brief The SPI and UART ports shown on the Quickstart and Porting pages, compiled by the host
 * build.
 *
 * Every platform call goes through the `platform_*` functions declared below; a real port replaces
 * them with its SDK (ESP-IDF, STM32 HAL, Zephyr, Linux spidev).
 */
#ifndef TMC9660_MY_PORT_HPP
#define TMC9660_MY_PORT_HPP

#include <cstddef>
#include <cstdint>

#include "tmc9660_spi_interface.hpp"
#include "tmc9660_uart_interface.hpp"

// Your SDK's calls. Declared only: the docs build checks that the ports compile.
bool platform_spi_init() noexcept; // mode 3, up to 10 MHz
bool platform_spi_transfer(const uint8_t* tx, uint8_t* rx,
                           std::size_t len) noexcept; // CS low for all bytes
bool platform_uart_init() noexcept;                   // 8N1, the baud rate the chip was set to
// Send `tx`, then collect exactly `rx_len` bytes within `timeout_us`: 0 ok, 1 timeout, 2 line
// fault.
int platform_uart_exchange(const uint8_t* tx, std::size_t tx_len, uint8_t* rx, std::size_t rx_len,
                           uint32_t timeout_us) noexcept;
void platform_delay_us(uint32_t us) noexcept; // busy-wait or sleep

// --8<-- [start:port]
class MyPort : public tmc9660::SpiInterface<MyPort> {
public:
  // Bring the bus up once; later calls return true at once.
  bool EnsureInitialized() noexcept {
    if (!ready_) {
      ready_ = platform_spi_init();
    }
    return ready_;
  }

  // One datagram, CS held low for all `len` bytes. `tx` and `rx` both have `len` bytes.
  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    return platform_spi_transfer(tx, rx, len);
  }

  // Wait at least `us` microseconds: the chip's reply gap and boot times depend on it.
  void DelayUs(uint32_t us) noexcept {
    platform_delay_us(us);
  }

private:
  bool ready_ = false;
};
// --8<-- [end:port]

// --8<-- [start:uart-port]
class MyUartPort : public tmc9660::UartInterface<MyUartPort> {
public:
  bool EnsureInitialized() noexcept {
    if (!ready_) {
      ready_ = platform_uart_init();
    }
    return ready_;
  }

  // Throw away stale received bytes, send the request, then wait for exactly `rx_len` bytes. With
  // `tx_len == 0` send nothing and only drain what is waiting (the driver does this after a failed
  // exchange). The driver tells timeouts from line faults.
  tmc9660::BusStatus Exchange(const uint8_t* tx, std::size_t tx_len, uint8_t* rx,
                              std::size_t rx_len, uint32_t timeout_us) noexcept {
    const int r = platform_uart_exchange(tx, tx_len, rx, rx_len, timeout_us);
    if (r == 0) {
      return tmc9660::BusStatus::Ok;
    }
    if (r == 1) {
      return tmc9660::BusStatus::Timeout;
    }
    return tmc9660::BusStatus::Fault;
  }

  void DelayUs(uint32_t us) noexcept {
    platform_delay_us(us);
  }

private:
  bool ready_ = false;
};
// --8<-- [end:uart-port]

#endif // TMC9660_MY_PORT_HPP
