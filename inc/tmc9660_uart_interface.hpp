/**
 * @file tmc9660_uart_interface.hpp
 * @brief The UART port of the TMC9660 driver: the CRTP base, the `UartPort` concept and the bus
 * requirements.
 * @copyright Copyright (c) 2026 The driver-tmc9660 authors. SPDX-License-Identifier: MIT
 *
 * @details
 * The chip's host UART is 8N1, least significant bit first, and detects the baud rate from the sync
 * bit of each request (standard rates from 9600 to 250000 at a 40 MHz system clock; up to 5 Mbit/s
 * when the rate is configured). Derive from `tmc9660::UartInterface<Self>` and implement:
 *
 * - `bool EnsureInitialized() noexcept`: bring the UART up on the first call; `true` at once
 * afterwards.
 * - `BusStatus Exchange(const uint8_t* tx, std::size_t tx_len, uint8_t* rx, std::size_t rx_len,
 *   uint32_t timeout_us) noexcept`: send `tx_len` bytes without gaps between bytes, then receive
 *   exactly `rx_len` bytes (0 when no reply is expected). The port discards stale received bytes
 *   before it sends and removes the echo of its own bytes on a shared wire, and handles
 *   transceiver direction. A call with `tx_len == 0` sends nothing: it only discards what is
 *   waiting on the line, reading up to `rx_len` bytes within `timeout_us` (the driver does this to
 *   drain a late reply after a failed exchange).
 * - `void DelayUs(uint32_t us) noexcept`: wait (required: a port without it does not compile).
 *
 * Two documents disagree on how long a gap inside a request may be before the chip discards the
 * partial message (about 100 ms in one chapter, 10 ms in another, ambiguity A6): a port sends each
 * request without gaps, so the question does not arise.
 */
#ifndef TMC9660_UART_INTERFACE_HPP
#define TMC9660_UART_INTERFACE_HPP

#include <concepts>
#include <cstddef>
#include <cstdint>

#include "tmc9660_detail.hpp"
#include "tmc9660_types.hpp"

namespace tmc9660 {

/**
 * @name Bus requirements
 * What the TMC9660 needs from the UART. @{
 */
/// Data bits of a frame: 8N1, one start bit and one stop bit.
inline constexpr uint8_t kUartDataBits = 8;
/// Highest baud rate.
inline constexpr uint32_t kMaxBaudRate = 5'000'000;
/// Assumed longest time from the last request byte to the first reply byte, in microseconds. The
/// manuals give no bound (ambiguity A19); the chip adds `UART_TXEN_PRE_DELAY` only when UART_TXEN
/// is used. A port sets its reply timeout above this value plus the frame time.
inline constexpr uint32_t kTurnaroundUs = 1'000;
/// Lowest standard baud rate the chip detects automatically.
inline constexpr uint32_t kMinAutobaudRate = 9'600;
/// Highest standard baud rate the chip detects automatically at a 40 MHz system clock.
inline constexpr uint32_t kMaxAutobaudRate = 250'000;
/** @} */

/**
 * @brief CRTP base of the platform UART port.
 * @tparam Derived Your port class.
 */
template <typename Derived>
class UartInterface {
public:
  /** @brief Ports are not copied: the driver holds a reference. */
  UartInterface(const UartInterface&) = delete;
  /** @brief Ports are not copied: the driver holds a reference. */
  UartInterface& operator=(const UartInterface&) = delete;

protected:
  /** @brief Only the derived port is constructed. */
  UartInterface() noexcept = default;
  /** @brief Destroyed through the derived port only. */
  ~UartInterface() noexcept = default;
};

/**
 * @brief Satisfied by a class that derives from `UartInterface<Port>` and implements the required
 * hooks with exactly the types above.
 * @tparam Port The port class.
 */
template <typename Port>
concept UartPort = std::derived_from<Port, UartInterface<Port>> && requires {
  detail::ExactHook<Port, bool() noexcept>::Take(&Port::EnsureInitialized);
  detail::ExactHook<Port, BusStatus(const uint8_t*, std::size_t, uint8_t*, std::size_t,
                                    uint32_t) noexcept>::Take(&Port::Exchange);
};

} // namespace tmc9660

#endif // TMC9660_UART_INTERFACE_HPP
