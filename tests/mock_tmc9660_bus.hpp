/**
 * @file mock_tmc9660_bus.hpp
 * @brief Mock SPI and UART ports over the chip model, with the optional hooks in the combinations
 * the tests need.
 */
#ifndef TMC9660_MOCK_TMC9660_BUS_HPP
#define TMC9660_MOCK_TMC9660_BUS_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "mock_tmc9660.hpp"
#include "tmc9660_spi_interface.hpp"
#include "tmc9660_types.hpp"
#include "tmc9660_uart_interface.hpp"

namespace tmc9660_test {

/// Shared state of the mock ports: the chip, fault injection and counters.
struct BusState {
  Tmc9660Model chip;
  bool bus_up_ok{true};   ///< EnsureInitialized() result.
  int fail_transfer{-1};  ///< Fail the transfer with this index (0-based); -1 never.
  int fail_span{1};       ///< How many consecutive transfers from `fail_transfer` fail.
  int late_at{-1};        ///< UART: the reply to the transfer with this index arrives late.
  int transfers{0};       ///< Transfers so far.
  int bus_up_calls{0};    ///< EnsureInitialized() calls so far.
  uint64_t delay_us{0};   ///< Total time waited.
  uint64_t wake_us{0};    ///< After the reset pin is released the chip is silent for this long.
  uint64_t wake_until{0}; ///< `delay_us` at which the chip answers again.
  bool faultn_high{true}; ///< Level GpioGet reports for FAULTN.
  bool pin_fail{false};   ///< GpioSet returns false.
  int pin_fail_at{-1};    ///< GpioSet returns false for the pin call with this index (0-based).
  int pin_calls{0};       ///< GpioSet calls so far.
  int logs{0};            ///< Log lines.
  int drv_en_writes{0};   ///< GpioSet(DrvEn) calls.
  tmc9660::BusStatus uart_fault{tmc9660::BusStatus::Ok}; ///< A UART failure to report.
  std::array<uint8_t, 16> last_tx{};                     ///< The last request.
  std::size_t last_len{0};                               ///< Its length.
  UartWire wire;             ///< The UART receive FIFO (late and stale replies).
  int reset_at_transfer{-1}; ///< The chip resets (unseen) just before this transfer; -1 never.
  const uint8_t* last_tx_ptr{nullptr}; ///< Buffer the port was given for sending.
  const uint8_t* last_rx_ptr{nullptr}; ///< Buffer the port was given for receiving.
  int reset_op{-1}; ///< Command byte of the UART request that the reset preceded.
  std::array<uint8_t, 512>
      op_log{}; ///< Byte 2 of each UART request, by transfer index (0 for a drain).
  std::array<uint8_t, 512> cmd_log{}; ///< Byte 1 of each UART request (0 for a drain).
  std::array<uint8_t, 512> spi_log{}; ///< Byte 0 of each SPI transfer, by transfer index.
  int drains{0}; ///< UART exchanges with no bytes to send (a drain of the line).
  std::array<int, 4> pin_levels{-1, -1, -1, -1}; ///< Last level written per `CtrlPin` (-1 never).

  [[nodiscard]] bool Failing(int index) const noexcept {
    return (fail_transfer >= 0) && (index >= fail_transfer) &&
           (index < (fail_transfer + fail_span));
  }
  void Wait(uint32_t us) noexcept {
    delay_us += us;
    chip.now_us = delay_us;
  }

  bool Spi(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    const int index = transfers++;
    last_tx_ptr = tx;
    last_rx_ptr = rx;
    if ((static_cast<std::size_t>(index) < spi_log.size()) && (tx != nullptr) && (len > 0U)) {
      spi_log[static_cast<std::size_t>(index)] = tx[0];
    }
    if (index == reset_at_transfer) {
      chip.Reset();
    }
    if (Failing(index) || (len == 0U) || (tx == nullptr) || (rx == nullptr)) {
      return false;
    }
    last_len = len;
    for (std::size_t i = 0; (i < len) && (i < last_tx.size()); ++i) {
      last_tx[i] = tx[i];
    }
    if (delay_us < wake_until) {
      for (std::size_t i = 0; i < len; ++i) {
        rx[i] = 0U; // not awake yet: the line reads zero
      }
      return true;
    }
    chip.SpiTransfer(tx, rx, len);
    return true;
  }
  tmc9660::BusStatus Uart(const uint8_t* tx, std::size_t tx_len, uint8_t* rx,
                          std::size_t rx_len) noexcept {
    const int index = transfers++;
    last_tx_ptr = tx;
    last_rx_ptr = rx;
    if (index == late_at) {
      wire.late_next = true;
    }
    if (tx_len == 0U) {
      ++drains;
    }
    if (static_cast<std::size_t>(index) < op_log.size()) {
      op_log[static_cast<std::size_t>(index)] = (tx_len > 2U) ? tx[2] : 0U;
      cmd_log[static_cast<std::size_t>(index)] = (tx_len > 1U) ? tx[1] : 0U;
    }
    if (index == reset_at_transfer) {
      chip.Reset();
      reset_op = (tx_len > 2U) ? static_cast<int>(tx[2]) : -1;
    }
    if (Failing(index)) {
      return tmc9660::BusStatus::Fault;
    }
    if (uart_fault != tmc9660::BusStatus::Ok) {
      return uart_fault;
    }
    if (tx_len > 0U) {
      last_len = tx_len;
      for (std::size_t i = 0; (i < tx_len) && (i < last_tx.size()); ++i) {
        last_tx[i] = tx[i];
      }
    }
    if (delay_us < wake_until) {
      return tmc9660::BusStatus::Timeout; // not awake yet: no reply
    }
    return wire.Exchange(chip, tx, tx_len, rx, rx_len) ? tmc9660::BusStatus::Ok
                                                       : tmc9660::BusStatus::Timeout;
  }
  bool Pin(tmc9660::CtrlPin pin, tmc9660::GpioSignal s) noexcept {
    if (pin_fail || (pin_calls++ == pin_fail_at)) {
      return false;
    }
    const bool active = (s == tmc9660::GpioSignal::ACTIVE);
    pin_levels[static_cast<std::size_t>(pin) % pin_levels.size()] = active ? 1 : 0;
    if (pin == tmc9660::CtrlPin::Reset) {
      chip.ResetPin(active);
      if (!active) {
        wake_until = delay_us + wake_us;
      }
    } else if (pin == tmc9660::CtrlPin::DrvEn) {
      ++drv_en_writes;
      chip.SetDrvEn(active);
    } else {
      // Wake: not modelled
    }
    return true;
  }
};

/// SPI, required hooks and `DelayUs()` only.
class SpiMin : public tmc9660::SpiInterface<SpiMin> {
public:
  explicit SpiMin(BusState& s) noexcept : s_(s) {}
  bool EnsureInitialized() noexcept {
    ++s_.bus_up_calls;
    return s_.bus_up_ok;
  }
  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    return s_.Spi(tx, rx, len);
  }
  void DelayUs(uint32_t us) noexcept {
    s_.Wait(us);
  }

private:
  BusState& s_;
};

/// SPI with every optional hook.
class SpiFull : public tmc9660::SpiInterface<SpiFull> {
public:
  explicit SpiFull(BusState& s) noexcept : s_(s) {}
  bool EnsureInitialized() noexcept {
    ++s_.bus_up_calls;
    return s_.bus_up_ok;
  }
  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    return s_.Spi(tx, rx, len);
  }
  void DelayUs(uint32_t us) noexcept {
    s_.Wait(us);
  }
  bool GpioSet(tmc9660::CtrlPin pin, tmc9660::GpioSignal s) noexcept {
    return s_.Pin(pin, s);
  }
  bool GpioGet(tmc9660::CtrlPin pin, bool& high) const noexcept {
    static_cast<void>(pin);
    high = s_.faultn_high;
    return true;
  }
  void Log(tmc9660::LogLevel level, const char* message) noexcept {
    static_cast<void>(level);
    static_cast<void>(message);
    ++s_.logs;
  }

private:
  BusState& s_;
};

/// UART, required hooks and `DelayUs()` only.
class UartMin : public tmc9660::UartInterface<UartMin> {
public:
  explicit UartMin(BusState& s) noexcept : s_(s) {}
  bool EnsureInitialized() noexcept {
    ++s_.bus_up_calls;
    return s_.bus_up_ok;
  }
  tmc9660::BusStatus Exchange(const uint8_t* tx, std::size_t tx_len, uint8_t* rx,
                              std::size_t rx_len, uint32_t timeout_us) noexcept {
    static_cast<void>(timeout_us);
    return s_.Uart(tx, tx_len, rx, rx_len);
  }
  void DelayUs(uint32_t us) noexcept {
    s_.Wait(us);
  }

private:
  BusState& s_;
};

/// UART with every optional hook.
class UartFull : public tmc9660::UartInterface<UartFull> {
public:
  explicit UartFull(BusState& s) noexcept : s_(s) {}
  bool EnsureInitialized() noexcept {
    ++s_.bus_up_calls;
    return s_.bus_up_ok;
  }
  tmc9660::BusStatus Exchange(const uint8_t* tx, std::size_t tx_len, uint8_t* rx,
                              std::size_t rx_len, uint32_t timeout_us) noexcept {
    static_cast<void>(timeout_us);
    return s_.Uart(tx, tx_len, rx, rx_len);
  }
  void DelayUs(uint32_t us) noexcept {
    s_.Wait(us);
  }
  bool GpioSet(tmc9660::CtrlPin pin, tmc9660::GpioSignal s) noexcept {
    return s_.Pin(pin, s);
  }
  bool GpioGet(tmc9660::CtrlPin pin, bool& high) const noexcept {
    static_cast<void>(pin);
    high = s_.faultn_high;
    return true;
  }
  void Log(tmc9660::LogLevel level, const char* message) noexcept {
    static_cast<void>(level);
    static_cast<void>(message);
    ++s_.logs;
  }

private:
  BusState& s_;
};

} // namespace tmc9660_test

#endif // TMC9660_MOCK_TMC9660_BUS_HPP
