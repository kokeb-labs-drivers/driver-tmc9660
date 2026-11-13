/**
 * @file tmc9660_bootconfig.hpp
 * @brief The bootloader's configuration memory (64 bytes, datasheet "Bootloader Configuration") as
 * a typed `BootConfig`, and the external-memory partition table builder.
 * @copyright Copyright (c) 2026 The driver-tmc9660 authors. SPDX-License-Identifier: MIT
 *
 * @details
 * The configuration memory is 16 little-endian 32-bit words. Every documented field is a row of
 * `kBootFields` (byte offset, shift and width as the datasheet prints them), so a field is reached
 * by name, range checked, and the table is the single source of the layout. A field the documents
 * leave unclear (the SPI-encoder chip-select polarity, ambiguity A16) is reachable through the raw
 * `SetWord()`.
 */
#ifndef TMC9660_BOOTCONFIG_HPP
#define TMC9660_BOOTCONFIG_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "tmc9660_detail.hpp"
#include "tmc9660_frames.hpp"
#include "tmc9660_types.hpp"

namespace tmc9660 {

/** @brief Named fields of the configuration memory. */
enum class BootField : uint8_t {
  LdoVext1 = 0,       ///< VEXT1 output voltage select (offset 0, bits 0-1).
  LdoVext2,           ///< VEXT2 output voltage select (bits 2-3).
  LdoSsVext1,         ///< VEXT1 start-up select (bits 4-5).
  LdoSsVext2,         ///< VEXT2 start-up select (bits 6-7).
  LdoShortFault,      ///< Signal an LDO short on FAULTN (bit 8).
  DeviceAddress,      ///< UART device address (offset 2, bits 0-7).
  HostAddress,        ///< UART host address (bits 8-15).
  UartTxenPostDelay,  ///< TX-enable release delay (offset 4, bits 0-7).
  UartTxenPreDelay,   ///< TX-enable lead (bits 8-15).
  BlDisableUart,      ///< Disable the bootloader UART (offset 6, bit 0).
  BlDisableSpi,       ///< Disable the bootloader SPI (bit 1).
  BlSpiSelect,        ///< Bootloader SPI instance (bit 2).
  BlUartRx,           ///< Bootloader UART RX pin (bit 3).
  BlUartTx,           ///< Bootloader UART TX pin (bit 4).
  BlUartTxen,         ///< Bootloader UART TX-enable pin (bits 5-6).
  BlUartBaudrate,     ///< Bootloader UART baud rate (bits 7-9).
  BlSpi0Sck,          ///< SPI0 SCK pin (bit 10).
  BootMode,           ///< BOOT_MODE (offset 8, bits 0-1): see `BootMode`.
  BlReadyFault,       ///< Assert FAULTN until the bootloader is ready (bit 2).
  BlExitFault,        ///< Assert FAULTN while leaving the bootloader (bit 3).
  DisableSelftest,    ///< Skip the start-up self-test (bit 8).
  BlConfigFault,      ///< Assert FAULTN while a configuration change applies (bit 9).
  StartMotorCtrl,     ///< Start the motor control application (bit 12).
  SpiFlashEn,         ///< SPI flash enable (offset 10, bit 0).
  SpiFlashCs,         ///< SPI flash chip-select GPIO (bits 3-7).
  SpiFlashFreq,       ///< SPI flash divider: 40 MHz / (N + 1), N of 3 or more (bits 8-11).
  I2cEepromEn,        ///< I2C EEPROM enable (offset 12, bit 0).
  I2cEepromSda,       ///< I2C SDA pin select (bits 1-2).
  I2cEepromScl,       ///< I2C SCL pin select (bits 3-4).
  I2cEepromAddr,      ///< I2C EEPROM address bits (bits 5-7).
  I2cEepromFreq,      ///< I2C clock select (bits 8-10).
  GpioOut,            ///< Output level of GPIO 0 to 15 (offset 14).
  GpioOutEn,          ///< Output enable of GPIO 0 to 15 (offset 16).
  GpioPullUp,         ///< Pull-up of GPIO 0 to 15 (offset 18).
  GpioPullDown,       ///< Pull-down of GPIO 0 to 15 (offset 20).
  Gpio16To18Out,      ///< Output level of GPIO 16 to 18 (offset 22, bits 0-2).
  Gpio16To18OutEn,    ///< Output enable of GPIO 16 to 18 (bits 3-5).
  Gpio16To18PullDown, ///< Pull-down of GPIO 16 to 18 (bits 6-8).
  Gpio16To18PullUp,   ///< Pull-up of GPIO 16 to 18 (bits 9-11).
  GpioAnalog2To5,     ///< Analog input of GPIO 2 to 5 (bits 12-15).
  ClockReserved1,     ///< Reserved, 99 (offset 24, bits 0-6).
  ClockExtNotInt,     ///< External clock instead of the internal oscillator (bit 8).
  ClockXtalCfg,       ///< Crystal configuration (bits 9-11).
  ClockXtalBoost,     ///< Crystal boost (bit 12).
  ClockExtNotXtal,    ///< External clock instead of a crystal (bit 13).
  ClockPllOutSel,     ///< System clock source: 0 oscillator, 1 PLL (bits 16-17).
  ClockRdiv,          ///< PLL input divider: input MHz minus one (bits 18-22).
  ClockSysClkDiv,     ///< System clock: 0 is 40 MHz, 3 is 15 MHz (bits 23-24).
  HallEnable,         ///< Hall sensor enable (offset 32, bit 0).
  HallU,              ///< Hall U pin select (bits 4-5).
  HallV,              ///< Hall V pin select (bits 6-7).
  HallW,              ///< Hall W pin select (bits 8-9).
  Abn1Enable,         ///< ABN encoder 1 enable (bit 1).
  Abn1A,              ///< ABN1 A pin select (bits 10-11).
  Abn1B,              ///< ABN1 B pin select (bits 12-13).
  Abn1N,              ///< ABN1 N pin select (bits 14-15).
  RefL,               ///< Left reference switch pin (offset 34, bits 0-1).
  RefR,               ///< Right reference switch pin (bits 2-3).
  RefH,               ///< Home switch pin (bits 4-6).
  StepDirEnable,      ///< STEP/DIR input enable (bit 8).
  StepPin,            ///< STEP pin select (bits 9-10).
  DirPin,             ///< DIR pin select (bit 11).
  Abn2Enable,         ///< ABN encoder 2 enable (bit 12).
  Abn2A,              ///< ABN2 A pin select (bit 13).
  Abn2B,              ///< ABN2 B pin select (bits 14-15; ambiguity A10).
  BrakeChopperEnable, ///< Brake chopper enable (offset 36, bit 4).
  BrakeChopperOutput, ///< Brake chopper output select (bits 5-9).
  MechBrakeEnable,    ///< Mechanical brake enable (bit 12).
  MechBrakeOutput,    ///< Mechanical brake output select (bits 13-14).
  SpiEncEnable,       ///< SPI encoder enable (offset 38, bit 0).
  SpiEncBlock,        ///< SPI encoder block (bit 1).
  SpiEncMode,         ///< SPI encoder mode (bits 2-3).
  SpiEncFreq,         ///< SPI encoder divider: 40 MHz / (N + 4) (bits 4-7).
  SpiEncCsPin,        ///< SPI encoder chip-select pin (bits 8-9).
  MemTmclScript,      ///< Script storage (offset 40, bits 0-1).
  MemParameters,      ///< Parameter storage (bits 2-3).
};

namespace detail {
/// `BootField` is contiguous from 0.
template <>
struct EnumLast<BootField> {
  /// The last enumerator.
  static constexpr BootField kValue = BootField::MemParameters;
};

/// Position of a field: byte offset in the configuration memory, shift within the 16-bit word that
/// starts there (or within the 32-bit word at offset 24), width in bits.
struct BootFieldInfo {
  uint8_t offset; ///< Byte offset as printed in the datasheet.
  uint8_t shift;  ///< First bit.
  uint8_t width;  ///< Width in bits.
};

/// The field table, in the order of `BootField`.
inline constexpr std::array<BootFieldInfo, 76> kBootFields{{
    {.offset = 0, .shift = 0, .width = 2},   {.offset = 0, .shift = 2, .width = 2},
    {.offset = 0, .shift = 4, .width = 2},   {.offset = 0, .shift = 6, .width = 2},
    {.offset = 0, .shift = 8, .width = 1},   {.offset = 2, .shift = 0, .width = 8},
    {.offset = 2, .shift = 8, .width = 8},   {.offset = 4, .shift = 0, .width = 8},
    {.offset = 4, .shift = 8, .width = 8},   {.offset = 6, .shift = 0, .width = 1},
    {.offset = 6, .shift = 1, .width = 1},   {.offset = 6, .shift = 2, .width = 1},
    {.offset = 6, .shift = 3, .width = 1},   {.offset = 6, .shift = 4, .width = 1},
    {.offset = 6, .shift = 5, .width = 2},   {.offset = 6, .shift = 7, .width = 3},
    {.offset = 6, .shift = 10, .width = 1},  {.offset = 8, .shift = 0, .width = 2},
    {.offset = 8, .shift = 2, .width = 1},   {.offset = 8, .shift = 3, .width = 1},
    {.offset = 8, .shift = 8, .width = 1},   {.offset = 8, .shift = 9, .width = 1},
    {.offset = 8, .shift = 12, .width = 1},  {.offset = 10, .shift = 0, .width = 1},
    {.offset = 10, .shift = 3, .width = 5},  {.offset = 10, .shift = 8, .width = 4},
    {.offset = 12, .shift = 0, .width = 1},  {.offset = 12, .shift = 1, .width = 2},
    {.offset = 12, .shift = 3, .width = 2},  {.offset = 12, .shift = 5, .width = 3},
    {.offset = 12, .shift = 8, .width = 3},  {.offset = 14, .shift = 0, .width = 16},
    {.offset = 16, .shift = 0, .width = 16}, {.offset = 18, .shift = 0, .width = 16},
    {.offset = 20, .shift = 0, .width = 16}, {.offset = 22, .shift = 0, .width = 3},
    {.offset = 22, .shift = 3, .width = 3},  {.offset = 22, .shift = 6, .width = 3},
    {.offset = 22, .shift = 9, .width = 3},  {.offset = 22, .shift = 12, .width = 4},
    {.offset = 24, .shift = 0, .width = 7},  {.offset = 24, .shift = 8, .width = 1},
    {.offset = 24, .shift = 9, .width = 3},  {.offset = 24, .shift = 12, .width = 1},
    {.offset = 24, .shift = 13, .width = 1}, {.offset = 24, .shift = 16, .width = 2},
    {.offset = 24, .shift = 18, .width = 5}, {.offset = 24, .shift = 23, .width = 2},
    {.offset = 32, .shift = 0, .width = 1},  {.offset = 32, .shift = 4, .width = 2},
    {.offset = 32, .shift = 6, .width = 2},  {.offset = 32, .shift = 8, .width = 2},
    {.offset = 32, .shift = 1, .width = 1},  {.offset = 32, .shift = 10, .width = 2},
    {.offset = 32, .shift = 12, .width = 2}, {.offset = 32, .shift = 14, .width = 2},
    {.offset = 34, .shift = 0, .width = 2},  {.offset = 34, .shift = 2, .width = 2},
    {.offset = 34, .shift = 4, .width = 3},  {.offset = 34, .shift = 8, .width = 1},
    {.offset = 34, .shift = 9, .width = 2},  {.offset = 34, .shift = 11, .width = 1},
    {.offset = 34, .shift = 12, .width = 1}, {.offset = 34, .shift = 13, .width = 1},
    {.offset = 34, .shift = 14, .width = 2}, {.offset = 36, .shift = 4, .width = 1},
    {.offset = 36, .shift = 5, .width = 5},  {.offset = 36, .shift = 12, .width = 1},
    {.offset = 36, .shift = 13, .width = 2}, {.offset = 38, .shift = 0, .width = 1},
    {.offset = 38, .shift = 1, .width = 1},  {.offset = 38, .shift = 2, .width = 2},
    {.offset = 38, .shift = 4, .width = 4},  {.offset = 38, .shift = 8, .width = 2},
    {.offset = 40, .shift = 0, .width = 2},  {.offset = 40, .shift = 2, .width = 2},
}};
static_assert(kBootFields.size() ==
              (static_cast<std::size_t>(ToUnderlying(EnumLast<BootField>::kValue)) + 1U));
} // namespace detail

/**
 * @brief The 64-byte configuration memory of the bootloader, as 16 little-endian 32-bit words.
 *
 * Plain data: no I/O. Read it with `TMC9660::BlReadConfig()`, change fields, write it with
 * `TMC9660::BlWriteConfig()`.
 */
class BootConfig {
public:
  /// Size of the configuration memory in bytes.
  static constexpr std::size_t kBytes = 64;
  /// Number of 32-bit words.
  static constexpr std::size_t kWords = 16;

  /** @brief All zero. A configuration for the chip comes from `BlReadConfig()`, not from here. */
  constexpr BootConfig() noexcept = default;

  /**
   * @brief A configuration from 16 raw words (for example read from the chip).
   * @param words The words, little endian per the manual's byte layout.
   */
  explicit constexpr BootConfig(const std::array<uint32_t, kWords>& words) noexcept
      : words_(words) {}

  /**
   * @brief Set a field.
   * @param field Which field.
   * @param value The new value; at most `2^width - 1`.
   * @return `Error::InvalidArgument` (nothing changed) when `field` is not a field or `value` does
   * not fit.
   */
  [[nodiscard]] constexpr Error Set(BootField field, uint32_t value) noexcept {
    if (!detail::IsEnumerator(field)) {
      return Error::InvalidArgument;
    }
    const detail::BootFieldInfo info = fieldInfo(field);
    if (value > maxValue(info)) {
      return Error::InvalidArgument;
    }
    const uint32_t shift = bitShift(info);
    const uint32_t mask = maxValue(info) << shift;
    uint32_t& word = words_[wordIndex(info)];
    word = (word & ~mask) | ((value << shift) & mask);
    return Error::Ok;
  }

  /**
   * @brief The value of a field.
   * @param field Which field.
   * @return The value; 0 for a value that is not a `BootField`.
   */
  [[nodiscard]] constexpr uint32_t Get(BootField field) const noexcept {
    if (!detail::IsEnumerator(field)) {
      return 0U;
    }
    const detail::BootFieldInfo info = fieldInfo(field);
    return (words_[wordIndex(info)] >> bitShift(info)) & maxValue(info);
  }

  /**
   * @brief The largest value a field holds.
   * @param field Which field.
   * @return `2^width - 1`; 0 for a value that is not a `BootField`.
   */
  [[nodiscard]] static constexpr uint32_t MaxOf(BootField field) noexcept {
    return detail::IsEnumerator(field) ? maxValue(fieldInfo(field)) : 0U;
  }

  /**
   * @brief One raw word.
   * @param index The word, 0 to 15.
   * @return The word as stored; 0 beyond the end.
   */
  [[nodiscard]] constexpr uint32_t Word(std::size_t index) const noexcept {
    return (index < kWords) ? words_[index] : 0U;
  }

  /**
   * @brief Replace one raw word.
   * @param index The word, 0 to 15.
   * @param value The new word.
   * @return `Ok`, or `InvalidArgument` beyond the end.
   */
  [[nodiscard]] constexpr Error SetWord(std::size_t index, uint32_t value) noexcept {
    if (index >= kWords) {
      return Error::InvalidArgument;
    }
    words_[index] = value;
    return Error::Ok;
  }

  /** @return The 16 raw words. */
  [[nodiscard]] constexpr const std::array<uint32_t, kWords>& Words() const noexcept {
    return words_;
  }

  /**
   * @brief Check the rules that make a configuration unsafe to apply: the application must not
   * start with a reserved BOOT_MODE, the motor-control clock must be 40 MHz and the device address
   * odd when it starts.
   * @return `Error::Ok`, or `Error::Refused`.
   */
  [[nodiscard]] constexpr Error CheckStart() const noexcept {
    const uint32_t mode = Get(BootField::BootMode);
    if (Get(BootField::StartMotorCtrl) == 0U) {
      return Error::Ok;
    }
    if ((mode != static_cast<uint32_t>(BootMode::Register)) &&
        (mode != static_cast<uint32_t>(BootMode::Parameter))) {
      return Error::Refused;
    }
    if (Get(BootField::ClockPllOutSel) != 1U) {
      return Error::Refused;
    }
    if (Get(BootField::ClockSysClkDiv) != 0U) {
      return Error::Refused;
    }
    return Error::Ok;
  }

  /**
   * @brief Equality of all words.
   * @param lhs One configuration.
   * @param rhs The configuration to compare with.
   * @return true when every word is equal.
   */
  [[nodiscard]] friend constexpr bool operator==(const BootConfig& lhs,
                                                 const BootConfig& rhs) noexcept {
    return lhs.words_ == rhs.words_;
  }

private:
  [[nodiscard]] static constexpr detail::BootFieldInfo fieldInfo(BootField f) noexcept {
    return detail::kBootFields[static_cast<std::size_t>(detail::ToUnderlying(f))];
  }
  [[nodiscard]] static constexpr uint32_t maxValue(const detail::BootFieldInfo& i) noexcept {
    return (uint32_t{1} << i.width) - 1U;
  }
  [[nodiscard]] static constexpr std::size_t wordIndex(const detail::BootFieldInfo& i) noexcept {
    return static_cast<std::size_t>(i.offset / 4U);
  }
  [[nodiscard]] static constexpr uint32_t bitShift(const detail::BootFieldInfo& i) noexcept {
    return (static_cast<uint32_t>(i.offset % 4U) * 8U) + i.shift;
  }

  std::array<uint32_t, kWords> words_{};
};

/** @brief Address of the configuration memory in the bootloader's address space. */
inline constexpr uint32_t kConfigMemAddress = 0x0002'0000U;
/** @brief Word index of the clock configuration (byte offset 24). */
inline constexpr std::size_t kClockWordIndex = 6;
/** @brief Word index of the BOOT_MODE word (byte offset 8): written last. */
inline constexpr std::size_t kBootModeWordIndex = 2;
/** @brief Word index of the LDO configuration (byte offset 0). */
inline constexpr std::size_t kLdoWordIndex = 0;

/** @brief One entry of an external-memory partition table. */
struct PartitionEntry {
  std::array<uint8_t, 12> name{}; ///< UTF-8 name bytes, unused bytes zero.
  uint8_t type{2};                ///< 2: parameter data, 4: script data.
  bool writable{false};           ///< Hint only: the bootloader can always write.
  uint32_t offset{0};             ///< Start of the data; a multiple of the sector size.
  uint32_t size{
      0}; ///< Size of the data; a multiple of the sector size (parameters: at least 0x400).
};

/// Header size of a partition table.
inline constexpr std::size_t kPartitionHeaderBytes = 16;
/// Size of one partition entry.
inline constexpr std::size_t kPartitionEntryBytes = 24;

namespace detail {

/// Whether every partition is valid for the sector size and none overlaps another.
[[nodiscard]] constexpr bool PartitionsValid(std::span<const PartitionEntry> entries,
                                             uint64_t sector) noexcept {
  for (std::size_t i = 0U; i < entries.size(); ++i) {
    const PartitionEntry& e = entries[i];
    if (((e.type != 2U) && (e.type != 4U)) || (e.size == 0U) || ((e.offset % sector) != 0U) ||
        ((e.size % sector) != 0U) || ((e.type == 2U) && (e.size < 0x400U))) {
      return false;
    }
    for (std::size_t j = 0U; j < i; ++j) {
      const PartitionEntry& o = entries[j];
      const uint64_t a0 = e.offset;
      const uint64_t a1 = a0 + e.size;
      const uint64_t b0 = o.offset;
      const uint64_t b1 = b0 + o.size;
      if ((a0 < b1) && (b0 < a1)) {
        return false;
      }
    }
  }
  return true;
}

/// Store a 32-bit value little endian at `at`.
constexpr void PutLe32(std::span<uint8_t> out, std::size_t at, uint32_t v) noexcept {
  out[at] = static_cast<uint8_t>(v);
  out[at + 1U] = static_cast<uint8_t>(v >> 8U);
  out[at + 2U] = static_cast<uint8_t>(v >> 16U);
  out[at + 3U] = static_cast<uint8_t>(v >> 24U);
}

/// Write the entries of a validated table after its header.
constexpr void WritePartitionEntries(std::span<const PartitionEntry> entries,
                                     std::span<uint8_t> out) noexcept {
  for (std::size_t i = 0U; i < entries.size(); ++i) {
    const std::size_t at = kPartitionHeaderBytes + (kPartitionEntryBytes * i);
    for (std::size_t k = 0U; k < 12U; ++k) {
      out[at + k] = entries[i].name[k];
    }
    out[at + 12U] = static_cast<uint8_t>(entries[i].type | (entries[i].writable ? 0x80U : 0U));
    PutLe32(out, at + 16U, entries[i].offset);
    PutLe32(out, at + 20U, entries[i].size);
  }
}

} // namespace detail

/**
 * @brief Build a partition table (datasheet Tables 21 and 22): header, entries, CRC-32 over the
 * bytes from the magic number on.
 * @param entries The partitions.
 * @param size_index Memory size is `2^size_index` bytes, 0 if unknown (at most 31).
 * @param sector_index Erase sector size is `2^sector_index` bytes, 0 if unknown (at most 31).
 * @param out Receives the table; must hold `16 + 24 * entries.size()` bytes.
 * @return The table size, or `Error::InvalidArgument` when an entry is invalid (type not 2 or 4,
 * size not a multiple of the sector size, parameter partition under 0x400 bytes, offset or size not
 * sector aligned, overlap) or `out` is too small. Nothing is written on error.
 */
[[nodiscard]] constexpr Result<std::size_t> BuildPartitionTable(
    std::span<const PartitionEntry> entries, uint8_t size_index, uint8_t sector_index,
    std::span<uint8_t> out) noexcept {
  const std::size_t total = kPartitionHeaderBytes + (kPartitionEntryBytes * entries.size());
  if (entries.empty() || (out.size() < total) || (size_index > 31U) || (sector_index > 31U)) {
    return Result<std::size_t>::Failure(Error::InvalidArgument);
  }
  const uint64_t sector = (sector_index == 0U) ? 1ULL : (1ULL << sector_index);
  if (!detail::PartitionsValid(entries, sector)) {
    return Result<std::size_t>::Failure(Error::InvalidArgument);
  }
  for (std::size_t i = 0U; i < total; ++i) {
    out[i] = 0U;
  }
  detail::PutLe32(out, 0U, static_cast<uint32_t>(total));
  out[8] = static_cast<uint8_t>(0xFE);
  out[9] = static_cast<uint8_t>(0xCA);
  out[10] = static_cast<uint8_t>(1);
  out[11] = static_cast<uint8_t>(1);
  out[12] = size_index;
  out[13] = sector_index;
  detail::WritePartitionEntries(entries, out);
  detail::PutLe32(out, 4U, detail::Crc32(std::span<const uint8_t>{out}.subspan(8U, total - 8U)));
  return Result<std::size_t>::Success(total);
}

} // namespace tmc9660

#endif // TMC9660_BOOTCONFIG_HPP
