/**
 * @file tmc9660_spi_interface.hpp
 * @brief The SPI port of the TMC9660 driver: the CRTP base, the `SpiPort` concept and the bus
 * requirements.
 * @copyright Copyright (c) 2026 The driver-tmc9660 authors. SPDX-License-Identifier: MIT
 *
 * @details
 * The driver never calls an SDK. It calls hooks on a class you derive from
 * `tmc9660::SpiInterface<Self>`:
 *
 * - `bool EnsureInitialized() noexcept`: bring the bus up on the first call; `true` at once
 * afterwards.
 * - `bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept`: one full-duplex
 *   transfer, CS low for all `len` bytes, mode 3, MSB first; `rx` is always given.
 * - `void DelayUs(uint32_t us) noexcept`: wait; used for the reply gap and the boot wait (required:
 * a port without it does not compile).
 *
 * The same SPI port carries the bootloader (5-byte datagrams), parameter mode (8 bytes) and
 * register mode (6 bytes): the port moves raw bytes, never retries, never interprets a frame. What
 * the chip needs from the bus is below as `constexpr` values (datasheet "SPI Interface", electrical
 * characteristics); a port checks its settings against them with `static_assert`. The optional
 * hooks (`GpioSet`, `GpioGet`, `SckHz`, `Log`) are described in `tmc9660.hpp`.
 */
#ifndef TMC9660_SPI_INTERFACE_HPP
#define TMC9660_SPI_INTERFACE_HPP

#include <concepts>
#include <cstddef>
#include <cstdint>

#include "tmc9660_detail.hpp"
#include "tmc9660_types.hpp"

namespace tmc9660 {

/**
 * @name Bus requirements
 * What the TMC9660 needs from the SPI bus. A port and a board configuration check their settings
 * against these with `static_assert` (handbook KB-CPP-123).
 * @{
 */
/// SPI mode: CPOL 1, CPHA 1 (the default of the chip's SPI peripheral).
inline constexpr uint8_t kSpiMode = 3;
/// Each byte is clocked most significant bit first; multi-byte values are big endian.
inline constexpr bool kMsbFirst = true;
/// Highest SCK frequency.
inline constexpr uint32_t kMaxClockHz = 10'000'000;
/// CS low to the first SCK edge, t_CSS.
inline constexpr uint32_t kCsSetupNs = 100;
/// SCK edge to CS change, t_CC (data sheet p. 27: one SCK period, 100 ns at 10 MHz).
inline constexpr uint32_t kCsHoldNs = 100;
/// `true`: a datagram is clocked in one CS low period, without a CS pulse between its bytes.
inline constexpr bool kContinuousWord = true;
/// Shortest time CS stays high between two datagrams, t_CSH (4 system clocks at 40 MHz).
inline constexpr uint32_t kMinCsHighNs = 100;
/// SCK low and high time, t_CL and t_CH.
inline constexpr uint32_t kClockPhaseNs = 20;
/// Data setup and hold time, t_DU and t_DH.
inline constexpr uint32_t kDataSetupNs = 20;
/// Bootloader datagram length in bytes.
inline constexpr std::size_t kBootloaderDatagramBytes = 5;
/// Parameter-mode (TMCL) datagram length in bytes.
inline constexpr std::size_t kParameterDatagramBytes = 8;
/// Register-mode datagram length in bytes.
inline constexpr std::size_t kRegisterDatagramBytes = 6;
/** @} */

/**
 * @brief CRTP base of the platform SPI port.
 *
 * Derive `class MyPort : public tmc9660::SpiInterface<MyPort>` and implement the hooks in this
 * file's description. The base declares no hook itself (a forwarding hook would turn a missing
 * override into infinite recursion) and no data; the driver checks your class with the `SpiPort`
 * concept, so a missing or misdeclared hook is a compile error that names it.
 *
 * @tparam Derived Your port class.
 */
template <typename Derived>
class SpiInterface {
public:
  /** @brief Ports are not copied: the driver holds a reference. */
  SpiInterface(const SpiInterface&) = delete;
  /** @brief Ports are not copied: the driver holds a reference. */
  SpiInterface& operator=(const SpiInterface&) = delete;

protected:
  /** @brief Only the derived port is constructed. */
  SpiInterface() noexcept = default;
  /** @brief Destroyed through the derived port only. */
  ~SpiInterface() noexcept = default;
};

/**
 * @brief Satisfied by a class that derives from `SpiInterface<Port>` and implements the required
 * hooks with exactly the types above, `noexcept` and returning `bool`.
 *
 * Each hook's address is matched with `detail::ExactHook`, which takes a member function of exactly
 * the signature and converts no value: a hook declared with a narrower length (`uint16_t len`) or
 * without `noexcept` does not satisfy the concept, instead of truncating every transfer silently
 * (KB-CPP20-006).
 *
 * @tparam Port The port class.
 */
template <typename Port>
concept SpiPort = std::derived_from<Port, SpiInterface<Port>> && requires {
  detail::ExactHook<Port, bool() noexcept>::Take(&Port::EnsureInitialized);
  detail::ExactHook<Port, bool(const uint8_t*, uint8_t*, std::size_t) noexcept>::Take(
      &Port::Transfer);
};

} // namespace tmc9660

#endif // TMC9660_SPI_INTERFACE_HPP
