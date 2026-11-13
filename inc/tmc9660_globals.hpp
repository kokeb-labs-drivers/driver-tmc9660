/**
 * @file tmc9660_globals.hpp
 * @brief The global parameters of parameter mode (SGP, GGP): banks 0, 2 and 3 (Tables 61 to 63).
 * @copyright Copyright (c) 2026 The driver-tmc9660 authors. SPDX-License-Identifier: MIT
 */
#ifndef TMC9660_GLOBALS_HPP
#define TMC9660_GLOBALS_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "tmc9660_params.hpp"

namespace tmc9660 {

/// One global parameter: its bank, number, access and range.
struct GlobalInfo {
  uint8_t bank;    ///< Bank 0 (system), 2 (user variables) or 3 (interrupt configuration).
  uint16_t number; ///< Parameter number in the bank.
  Access access;   ///< Access class.
  int64_t min;     ///< Smallest value.
  int64_t max;     ///< Largest value.
  bool odd_only;   ///< Only odd values are valid (SERIAL_ADDRESS).
};

/** @brief Banks of global parameters. */
enum class GlobalBank : uint8_t {
  System = 0,    ///< Bank 0: communication, I/O masks, heartbeat, power-down, auto start.
  UserVars = 2,  ///< Bank 2: 16 user variables of the scripting feature.
  Interrupts = 3 ///< Bank 3: timers and trigger transitions.
};

/// Bank 0 numbers.
namespace global0 {
inline constexpr uint16_t SERIAL_ADDRESS = 1;               ///< Module address, odd values only.
inline constexpr uint16_t SERIAL_HOST_ADDRESS = 2;          ///< Host address.
inline constexpr uint16_t HEARTBEAT_MONITORING_CONFIG = 3;  ///< 0 off, 1 UART, 2 SPI, 3 both.
inline constexpr uint16_t HEARTBEAT_MONITORING_TIMEOUT = 4; ///< Timeout in ms.
inline constexpr uint16_t IO_DIRECTION_MASK = 5;            ///< GPIO direction mask.
inline constexpr uint16_t IO_INPUT_PULLUP_PULLDOWN_ENABLE_MASK = 6; ///< Pull resistor enable mask.
inline constexpr uint16_t IO_INPUT_PULLUP_PULLDOWN_DIRECTION_MASK = 7; ///< Pull direction mask.
inline constexpr uint16_t WAKE_PIN_CONTROL_ENABLE = 10; ///< Enable the WAKE pin function.
inline constexpr uint16_t GO_TO_TIMEOUT_POWER_DOWN_STATE =
    11;                                              ///< Write only: power down for 250 ms to 32 s.
inline constexpr uint16_t MAIN_LOOPS = 12;           ///< Read only: main loops per second.
inline constexpr uint16_t TORQUE_LOOPS = 13;         ///< Read only: torque loops per second.
inline constexpr uint16_t VELOCITY_LOOPS = 14;       ///< Read only: velocity loops per second.
inline constexpr uint16_t AUTO_START_ENABLE = 77;    ///< Start the script after power-up.
inline constexpr uint16_t CLEAR_USER_VARIABLES = 85; ///< Clear the user variables at start-up.
} // namespace global0

namespace detail {

/// The rows of Tables 61, 62 and 63.
constexpr std::array<GlobalInfo, 55> MakeGlobals() noexcept {
  std::array<GlobalInfo, 55> t{};
  std::size_t n = 0U;
  const auto add = [&t, &n](uint8_t bank, uint16_t number, Access access, int64_t lo, int64_t hi,
                            bool odd) noexcept {
    t[n++] = GlobalInfo{
        .bank = bank, .number = number, .access = access, .min = lo, .max = hi, .odd_only = odd};
  };
  add(0, 1, Access::ReadWriteStored, 1, 255, true);
  add(0, 2, Access::ReadWriteStored, 1, 255, false);
  add(0, 3, Access::ReadWriteStored, 0, 3, false);
  add(0, 4, Access::ReadWriteStored, 1, 4294967295LL, false);
  add(0, 5, Access::ReadWriteStored, 0, 524287, false);
  add(0, 6, Access::ReadWriteStored, 0, 524287, false);
  add(0, 7, Access::ReadWriteStored, 0, 524287, false);
  add(0, 10, Access::ReadWriteStored, 0, 1, false);
  add(0, 11, Access::WriteOnly, 0, 7, false);
  add(0, 12, Access::ReadOnly, 0, 4294967295LL, false);
  add(0, 13, Access::ReadOnly, 0, 4294967295LL, false);
  add(0, 14, Access::ReadOnly, 0, 4294967295LL, false);
  add(0, 77, Access::ReadWriteStored, 0, 1, false);
  add(0, 85, Access::ReadWriteStored, 0, 1, false);
  for (uint16_t i = 0U; i < 16U; ++i) {
    add(2, i, Access::ReadWriteStored, -2147483648LL, 2147483647LL, false);
  }
  for (uint16_t i = 0U; i < 3U; ++i) {
    add(3, i, Access::ReadWrite, 0, 2147483647LL, false);
  }
  for (uint16_t i = 10U; i < 32U; ++i) {
    add(3, i, Access::ReadWrite, 0, 3, false);
  }
  return t;
}

} // namespace detail

/// Tables 61 to 63, sorted by bank and number.
inline constexpr std::array<GlobalInfo, 55> kGlobals = detail::MakeGlobals();

/**
 * @brief The row of a global parameter.
 * @param bank The bank number (0, 2 or 3).
 * @param number The parameter number in the bank.
 * @return The row, or `nullptr` when the manual lists none.
 */
[[nodiscard]] constexpr const GlobalInfo* FindGlobal(uint8_t bank, uint32_t number) noexcept {
  for (const GlobalInfo& g : kGlobals) {
    if ((g.bank == bank) && (g.number == number)) {
      return &g;
    }
  }
  return nullptr;
}

} // namespace tmc9660

#endif // TMC9660_GLOBALS_HPP
