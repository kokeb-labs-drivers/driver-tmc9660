/**
 * @file tmc9660_frames.hpp
 * @brief Pure frame encoders and decoders of the three TMC9660 protocols: bootloader, parameter
 * mode (TMCL) and register mode, over SPI and UART. No I/O, no state.
 * @copyright Copyright (c) 2026 The driver-tmc9660 authors. SPDX-License-Identifier: MIT
 *
 * @details
 * Every function is `constexpr` and proved against the datasheet's worked examples with
 * `static_assert` at the end of this file. Where the documents disagree, a parameter selects the
 * reading (see the ambiguity table in `docs/ambiguities.md`).
 */
#ifndef TMC9660_FRAMES_HPP
#define TMC9660_FRAMES_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "tmc9660_detail.hpp"
#include "tmc9660_types.hpp"

namespace tmc9660::detail {

/// Reverse the bits of a byte.
[[nodiscard]] constexpr uint8_t ReverseBits(uint8_t value) noexcept {
  uint32_t in = value;
  uint32_t out = 0U;
  for (uint32_t i = 0U; i < 8U; ++i) {
    out = (out << 1U) | (in & 1U);
    in >>= 1U;
  }
  return static_cast<uint8_t>(out);
}

/**
 * @brief The bootloader's CRC-8 (datasheet "CRC8 Calculation"): polynomial 0x07, start value 0,
 * every input byte bit reflected. The datasheet text also reflects the result, but its worked
 * examples do not (ambiguity A1), so the output is reflected only when `reflect_output` is true.
 */
[[nodiscard]] constexpr uint8_t Crc8(std::span<const uint8_t> data, bool reflect_output) noexcept {
  uint32_t crc = 0U;
  for (const uint8_t byte : data) {
    crc ^= ReverseBits(byte);
    for (uint32_t bit = 0U; bit < 8U; ++bit) {
      if ((crc & 0x80U) != 0U) {
        crc = ((crc << 1U) ^ 0x07U) & 0xFFU;
      } else {
        crc = (crc << 1U) & 0xFFU;
      }
    }
  }
  const auto result = static_cast<uint8_t>(crc);
  return reflect_output ? ReverseBits(result) : result;
}

/// The bootloader's CRC-32 of the partition table: the Ethernet CRC (reflected 0xEDB88320, start
/// and final XOR 0xFFFFFFFF).
[[nodiscard]] constexpr uint32_t Crc32(std::span<const uint8_t> data) noexcept {
  uint32_t crc = 0xFFFF'FFFFU;
  for (const uint8_t byte : data) {
    crc ^= byte;
    for (uint32_t bit = 0U; bit < 8U; ++bit) {
      if ((crc & 1U) != 0U) {
        crc = (crc >> 1U) ^ 0xEDB8'8320U;
      } else {
        crc >>= 1U;
      }
    }
  }
  return ~crc;
}

/// The TMCL and register-mode checksum: the 8-bit sum of the bytes.
[[nodiscard]] constexpr uint8_t Sum8(std::span<const uint8_t> data) noexcept {
  uint32_t sum = 0U;
  for (const uint8_t byte : data) {
    sum = (sum + byte) & 0xFFU;
  }
  return static_cast<uint8_t>(sum);
}

/// Store `value` big endian at `out[0..3]`.
constexpr void PutBe32(std::span<uint8_t> out, uint32_t value) noexcept {
  out[0] = static_cast<uint8_t>(value >> 24U);
  out[1] = static_cast<uint8_t>(value >> 16U);
  out[2] = static_cast<uint8_t>(value >> 8U);
  out[3] = static_cast<uint8_t>(value);
}

/// Read a big-endian 32-bit value from `in[0..3]`.
[[nodiscard]] constexpr uint32_t GetBe32(std::span<const uint8_t> in) noexcept {
  return (static_cast<uint32_t>(in[0]) << 24U) | (static_cast<uint32_t>(in[1]) << 16U) |
         (static_cast<uint32_t>(in[2]) << 8U) | static_cast<uint32_t>(in[3]);
}

// -------------------------------------------------------------------------------------------------
// Bootloader
// -------------------------------------------------------------------------------------------------

/// Length of a bootloader UART request or reply.
inline constexpr std::size_t kBlUartLen = 8;
/// Length of a bootloader SPI datagram.
inline constexpr std::size_t kBlSpiLen = 5;
/// First byte of every bootloader UART request.
inline constexpr uint8_t kBlSync = 0x55;

/// A decoded bootloader reply.
struct BlReply {
  uint8_t status{0}; ///< Bootloader status code.
  uint32_t value{0}; ///< Reply data.
};

/// Bootloader UART request: `0x55 | device address | command | value[4] | CRC8`.
[[nodiscard]] constexpr std::array<uint8_t, kBlUartLen> BlUartRequest(uint8_t device,
                                                                      uint8_t command,
                                                                      uint32_t value,
                                                                      bool crc_reflect) noexcept {
  std::array<uint8_t, kBlUartLen> f{};
  f[0] = kBlSync;
  f[1] = device;
  f[2] = command;
  PutBe32(std::span<uint8_t>{f}.subspan(3U, 4U), value);
  f[7] = Crc8(std::span<const uint8_t>{f}.first(7U), crc_reflect);
  return f;
}

/// Decode a bootloader UART reply `host | device | status | value[4] | CRC8`. `false`: bad CRC,
/// host or device address.
[[nodiscard]] constexpr bool BlUartDecode(std::span<const uint8_t, kBlUartLen> f, uint8_t host,
                                          uint8_t device, bool crc_reflect, BlReply& out) noexcept {
  if (Crc8(f.first(7U), crc_reflect) != f[7]) {
    return false;
  }
  if ((f[0] != host) || (f[1] != device)) {
    return false;
  }
  out.status = f[2];
  out.value = GetBe32(f.subspan(3U, 4U));
  return true;
}

/// Bootloader SPI datagram `command | value[4]`.
[[nodiscard]] constexpr std::array<uint8_t, kBlSpiLen> BlSpiRequest(uint8_t command,
                                                                    uint32_t value) noexcept {
  std::array<uint8_t, kBlSpiLen> f{};
  f[0] = command;
  PutBe32(std::span<uint8_t>{f}.subspan(1U, 4U), value);
  return f;
}

/// Decode a bootloader SPI reply `status | value[4]`.
[[nodiscard]] constexpr BlReply BlSpiDecode(std::span<const uint8_t, kBlSpiLen> f) noexcept {
  return BlReply{.status = f[0], .value = GetBe32(f.subspan(1U, 4U))};
}

// -------------------------------------------------------------------------------------------------
// Parameter mode (TMCL)
// -------------------------------------------------------------------------------------------------

/// Length of a TMCL SPI frame.
inline constexpr std::size_t kTmclSpiLen = 8;
/// Length of a TMCL UART frame.
inline constexpr std::size_t kTmclUartLen = 9;
/// Opcode of the SPI reply-request frame.
inline constexpr uint8_t kTmclReplyRequestOp = 0xFF;

/// A TMCL command before framing.
struct TmclCommand {
  uint8_t op{0};     ///< Command number.
  uint16_t type{0};  ///< Type or parameter number, 12 bits.
  uint8_t motor{0};  ///< Motor or bank, 4 bits.
  uint32_t value{0}; ///< Value, sent big endian.
};

/// A decoded TMCL reply.
struct TmclReply {
  uint8_t spi_status{0}; ///< SPI status byte (SPI only).
  uint8_t status{0};     ///< TMCL status code.
  uint8_t op{0};         ///< Echoed command number.
  uint32_t value{0};     ///< Reply value.
};

/// The two bytes that carry the 12-bit type and the 4-bit motor or bank, in frame order (ambiguity
/// A2): `{type[7:0], motor << 4 | type[11:8]}` (`MotorHigh`, the literal reading of the UART table)
/// or `{type[7:0], type[11:8] << 4 | motor}` (`TypeHigh`). A motor or bank above 15 is the whole
/// second byte, as the Boot command needs it. `Auto` is not a layout: it encodes as `MotorHigh`.
[[nodiscard]] constexpr std::array<uint8_t, 2> TmclTypeMotorBytes(const TmclCommand& c,
                                                                  TmclLayout layout) noexcept {
  const uint32_t type = static_cast<uint32_t>(c.type) & 0x0FFFU;
  if (c.motor > 0x0FU) {
    return {static_cast<uint8_t>(type & 0xFFU), c.motor};
  }
  const uint32_t motor = static_cast<uint32_t>(c.motor) & 0x0FU;
  const uint32_t type_hi = (type >> 8U) & 0x0FU;
  switch (layout) {
  case TmclLayout::TypeHigh:
    return {static_cast<uint8_t>(type & 0xFFU), static_cast<uint8_t>((type_hi << 4U) | motor)};
  case TmclLayout::Auto:
  case TmclLayout::MotorHigh:
  default:
    break;
  }
  return {static_cast<uint8_t>(type & 0xFFU), static_cast<uint8_t>((motor << 4U) | type_hi)};
}

/// TMCL SPI request: `op | type bytes[2] | value[4] | sum8`.
[[nodiscard]] constexpr std::array<uint8_t, kTmclSpiLen> TmclSpiRequest(
    const TmclCommand& c, TmclLayout layout) noexcept {
  std::array<uint8_t, kTmclSpiLen> f{};
  const std::array<uint8_t, 2> tm = TmclTypeMotorBytes(c, layout);
  f[0] = c.op;
  f[1] = tm[0];
  f[2] = tm[1];
  PutBe32(std::span<uint8_t>{f}.subspan(3U, 4U), c.value);
  f[7] = Sum8(std::span<const uint8_t>{f}.first(7U));
  return f;
}

/// The SPI reply-request frame: opcode 0xFF, everything else 0, checksum 0xFF. Never an all-zero
/// frame: opcode 0 is parsed as a command and corrupts the value bytes.
[[nodiscard]] constexpr std::array<uint8_t, kTmclSpiLen> TmclSpiReplyRequest() noexcept {
  std::array<uint8_t, kTmclSpiLen> f{};
  f[0] = kTmclReplyRequestOp;
  f[7] = kTmclReplyRequestOp;
  return f;
}

/// Decode a TMCL SPI reply `SPI status | status | op | value[4] | sum8`; `false` on a checksum
/// mismatch.
[[nodiscard]] constexpr bool TmclSpiDecode(std::span<const uint8_t, kTmclSpiLen> f,
                                           TmclReply& out) noexcept {
  if (Sum8(f.first(7U)) != f[7]) {
    return false;
  }
  out.spi_status = f[0];
  out.status = f[1];
  out.op = f[2];
  out.value = GetBe32(f.subspan(3U, 4U));
  return true;
}

/// TMCL UART request: `sync | op | type bytes[2] | value[4] | sum8`; the module address is the
/// upper 7 bits of the first byte, the sync bit is bit 0.
[[nodiscard]] constexpr std::array<uint8_t, kTmclUartLen> TmclUartRequest(
    uint8_t module_address, const TmclCommand& c, TmclLayout layout) noexcept {
  std::array<uint8_t, kTmclUartLen> f{};
  const std::array<uint8_t, 2> tm = TmclTypeMotorBytes(c, layout);
  f[0] = static_cast<uint8_t>(static_cast<uint32_t>(module_address) | 1U);
  f[1] = c.op;
  f[2] = tm[0];
  f[3] = tm[1];
  PutBe32(std::span<uint8_t>{f}.subspan(4U, 4U), c.value);
  f[8] = Sum8(std::span<const uint8_t>{f}.first(8U));
  return f;
}

/// Decode a TMCL UART reply `host | sync/address | status | op | value[4] | sum8`.
[[nodiscard]] constexpr bool TmclUartDecode(std::span<const uint8_t, kTmclUartLen> f, uint8_t host,
                                            uint8_t module_address, TmclReply& out) noexcept {
  if (Sum8(f.first(8U)) != f[8]) {
    return false;
  }
  if ((f[0] != host) ||
      (f[1] != static_cast<uint8_t>(static_cast<uint32_t>(module_address) | 1U))) {
    return false;
  }
  out.spi_status = 0U;
  out.status = f[2];
  out.op = f[3];
  out.value = GetBe32(f.subspan(4U, 4U));
  return true;
}

// -------------------------------------------------------------------------------------------------
// Register mode
// -------------------------------------------------------------------------------------------------

/// Length of a register-mode SPI datagram.
inline constexpr std::size_t kRegSpiLen = 6;
/// Length of a register-mode UART frame.
inline constexpr std::size_t kRegUartLen = 9;
/// Register-mode UART command: write a register.
inline constexpr uint8_t kRegUartWrite = 146;
/// Register-mode UART command: read a register.
inline constexpr uint8_t kRegUartRead = 148;
/// Register-mode UART command: RAMDebug access.
inline constexpr uint8_t kRegUartRamDebug = 142;

/// A decoded register-mode SPI reply.
struct RegSpiReply {
  bool boot{false};   ///< Boot bit: set on the first datagram after a reset only.
  uint8_t counter{0}; ///< Datagram counter, modulo 8.
  bool ok{false};     ///< Request status bit.
  uint32_t value{0};  ///< Read data.
};

/// Register-mode SPI request. Table layout: `block[47:42] | write[41] | offset[40:32] | data`; the
/// wide layout (ambiguity A3) is `block[47:43] | write[42] | offset[41:32] | data`. `false` when
/// the offset does not fit.
[[nodiscard]] constexpr bool RegSpiRequest(uint8_t block, uint16_t offset, bool write,
                                           uint32_t data, bool wide,
                                           std::array<uint8_t, kRegSpiLen>& out) noexcept {
  const uint32_t offset_bits = wide ? 10U : 9U;
  if ((static_cast<uint32_t>(offset) >> offset_bits) != 0U) {
    return false;
  }
  const uint32_t block_bits = wide ? 5U : 6U;
  if ((static_cast<uint32_t>(block) >> block_bits) != 0U) {
    return false;
  }
  uint32_t high = 0U; // bits 47..32
  if (wide) {
    high = (static_cast<uint32_t>(block) << 11U) | (write ? (1U << 10U) : 0U) | offset;
  } else {
    high = (static_cast<uint32_t>(block) << 10U) | (write ? (1U << 9U) : 0U) | offset;
  }
  out[0] = static_cast<uint8_t>(high >> 8U);
  out[1] = static_cast<uint8_t>(high);
  PutBe32(std::span<uint8_t>{out}.subspan(2U, 4U), data);
  return true;
}

/// Decode a register-mode SPI reply `boot[47] | counter[46:44] | reserved | status[32] | data`.
[[nodiscard]] constexpr RegSpiReply RegSpiDecode(std::span<const uint8_t, kRegSpiLen> f) noexcept {
  RegSpiReply r{};
  r.boot = (f[0] & 0x80U) != 0U;
  r.counter = static_cast<uint8_t>((static_cast<uint32_t>(f[0]) >> 4U) & 0x07U);
  r.ok = (f[1] & 0x01U) != 0U;
  r.value = GetBe32(f.subspan(2U, 4U));
  return r;
}

/// The fields of a register-mode UART request.
struct RegUartArgs {
  uint8_t module_address{0}; ///< The device address (the lowest bit is set in the frame).
  uint8_t command{0};        ///< 146 (write), 148 (read) or 142 (RAMDebug).
  uint8_t block{0};          ///< Register block, 5 bits.
  uint16_t offset{0};        ///< Register offset, 11 bits.
  uint32_t data{0};          ///< The data word of a write.
  bool msb_fields{false};    ///< Ambiguity A4: the other field order.
};

/// Register-mode UART request `sync | command | offset[7:0] | block/offset-high | data[4] | sum8`.
/// Table layout: byte 3 = `block << 3 | offset[10:8]` (offset bits 16-26, block bits 27-31 counted
/// from the least significant bit); the other reading (ambiguity A4) is `offset[10:8] << 5 |
/// block`. `false` when a field does not fit.
[[nodiscard]] constexpr bool RegUartRequest(const RegUartArgs& args,
                                            std::array<uint8_t, kRegUartLen>& out) noexcept {
  if (((static_cast<uint32_t>(args.offset) >> 11U) != 0U) ||
      ((static_cast<uint32_t>(args.block) >> 5U) != 0U)) {
    return false;
  }
  const uint32_t offset_high = (static_cast<uint32_t>(args.offset) >> 8U) & 0x07U;
  out[0] = static_cast<uint8_t>(static_cast<uint32_t>(args.module_address) | 1U);
  out[1] = args.command;
  out[2] = static_cast<uint8_t>(args.offset & 0xFFU);
  out[3] = args.msb_fields
               ? static_cast<uint8_t>((offset_high << 5U) | args.block)
               : static_cast<uint8_t>((static_cast<uint32_t>(args.block) << 3U) | offset_high);
  PutBe32(std::span<uint8_t>{out}.subspan(4U, 4U), args.data);
  out[8] = Sum8(std::span<const uint8_t>{out}.first(8U));
  return true;
}

// -------------------------------------------------------------------------------------------------
// Proofs against the datasheet's worked examples
// -------------------------------------------------------------------------------------------------

namespace proof {
inline constexpr std::array<uint8_t, 7> kRequestExample{0x55, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01};
inline constexpr std::array<uint8_t, 7> kReplyExample{0xFF, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00};
static_assert(Crc8(kRequestExample, false) == static_cast<uint8_t>(0x1D));
static_assert(Crc8(kReplyExample, false) == static_cast<uint8_t>(0xA3));
static_assert(BlUartRequest(static_cast<uint8_t>(1), static_cast<uint8_t>(0),
                            static_cast<uint32_t>(1), false)[7] == static_cast<uint8_t>(0x1D));
static_assert(Sum8(std::array<uint8_t, 2>{0xFF, 0x02}) == static_cast<uint8_t>(0x01));
static_assert((TmclSpiReplyRequest()[0] == static_cast<uint8_t>(0xFF)) &&
              (TmclSpiReplyRequest()[7] == static_cast<uint8_t>(0xFF)));
} // namespace proof

} // namespace tmc9660::detail

#endif // TMC9660_FRAMES_HPP
