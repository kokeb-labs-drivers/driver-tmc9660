/**
 * @file test_frames.cpp
 * @brief Frame encoders, CRCs, tables and the boot configuration against the datasheet's worked
 * examples.
 */
#include "fixture.hpp"
#include "tmc9660_bootconfig.hpp"

using namespace tmc9660;

TEST_CASE("CRC-8 matches both datasheet examples and the model's independent algorithm") {
  const std::array<uint8_t, 7> req{0x55, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01};
  const std::array<uint8_t, 7> rep{0xFF, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00};
  CHECK_EQ(detail::Crc8(req, false), 0x1D);
  CHECK_EQ(detail::Crc8(rep, false), 0xA3);
  tmc9660_test::Tmc9660Model m;
  CHECK_EQ(m.Crc8(req.data(), req.size()), 0x1D);
  CHECK_EQ(m.Crc8(rep.data(), rep.size()), 0xA3);
  for (unsigned i = 0; i < 256U; ++i) { // the two implementations agree on every single-byte frame
    const auto b = static_cast<uint8_t>(i);
    CHECK_EQ(detail::Crc8(std::span<const uint8_t>(&b, 1U), false), m.Crc8(&b, 1U));
  }
  m.knobs.crc_reflect_out = true;
  CHECK_EQ(m.Crc8(req.data(), req.size()), detail::Crc8(req, true));
}

TEST_CASE("Bootloader frames") {
  const auto r = detail::BlUartRequest(1, 0, 1, false);
  CHECK_EQ(r[0], 0x55);
  CHECK_EQ(r[7], 0x1D);
  detail::BlReply out{};
  const std::array<uint8_t, 8> reply{0xFF, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};
  std::array<uint8_t, 8> good = reply;
  good[7] = detail::Crc8(std::span<const uint8_t>(good).first(7), false);
  std::array<uint8_t, 8> bad_addr = good;
  bad_addr[0] = 0xFE;
  CHECK(detail::BlUartDecode(good, 255, 1, false, out));
  CHECK_EQ(out.value, 0x100U);
  CHECK(!detail::BlUartDecode(bad_addr, 255, 1, false, out));
  const auto s = detail::BlSpiRequest(18, 0x01020304U);
  CHECK_EQ(s[1], 1);
  CHECK_EQ(s[4], 4);
  const std::array<uint8_t, 5> sr{19, 0, 1, 0, 2};
  CHECK_EQ(detail::BlSpiDecode(sr).status, 19);
}

TEST_CASE("TMCL frames: layout, checksum, nibble order, reply request") {
  // Golden bytes. Parameter Mode manual, Table 1 (UART): the bits count from the least significant
  // bit of each byte (the sync bit is bit 0, the address bits 1-7), so byte 2 is type[7:0] and byte
  // 3 is motor << 4 | type[11:8]: bits 24-27 type, 28-31 motor.
  const detail::TmclCommand c{5, 0x123, 0, 0x01020304U};
  const auto mh = detail::TmclSpiRequest(c, TmclLayout::MotorHigh);
  CHECK_EQ(mh[0], 5);
  CHECK_EQ(mh[1], 0x23);
  CHECK_EQ(mh[2], 0x01);
  CHECK_EQ(mh[3], 0x01);
  CHECK_EQ(mh[6], 0x04);
  CHECK_EQ(mh[7], detail::Sum8(std::span<const uint8_t>(mh).first(7)));
  const auto th = detail::TmclSpiRequest(c, TmclLayout::TypeHigh);
  CHECK_EQ(th[1], 0x23);
  CHECK_EQ(th[2], 0x10);
  // A motor other than 0 shows which nibble it is in: type 0x104, motor 2.
  const detail::TmclCommand d{5, 0x104, 2, 0U};
  CHECK_EQ(detail::TmclSpiRequest(d, TmclLayout::MotorHigh)[2], 0x21);
  CHECK_EQ(detail::TmclSpiRequest(d, TmclLayout::TypeHigh)[2], 0x12);
  const auto rr = detail::TmclSpiReplyRequest();
  CHECK_EQ(rr[0], 0xFF);
  CHECK_EQ(rr[7], 0xFF);
  CHECK_EQ(rr[3], 0);
  const auto u = detail::TmclUartRequest(1, c, TmclLayout::MotorHigh);
  CHECK_EQ(u[0], 1);
  CHECK_EQ(u[1], 5);
  CHECK_EQ(u[2], 0x23);
  CHECK_EQ(u[3], 0x01);
  CHECK_EQ(detail::TmclUartRequest(1, c, TmclLayout::TypeHigh)[3], 0x10);
  CHECK_EQ(detail::TmclUartRequest(3, d, TmclLayout::MotorHigh)[3], 0x21);
  CHECK_EQ(detail::TmclUartRequest(3, d, TmclLayout::MotorHigh)[0], 3);
  CHECK_EQ(u[8], detail::Sum8(std::span<const uint8_t>(u).first(8)));
  for (const TmclLayout l : {TmclLayout::MotorHigh, TmclLayout::TypeHigh}) {
    const auto b = detail::TmclSpiRequest(detail::TmclCommand{242, 0x81, 0x92, 0xA3B4C5D6U}, l);
    CHECK_EQ(b[1], 0x81); // the Boot command is the same bytes under every layout
    CHECK_EQ(b[2], 0x92);
  }
  const auto boot = detail::TmclSpiRequest(detail::TmclCommand{242, 0x81, 0x92, 0xA3B4C5D6U},
                                           TmclLayout::MotorHigh);
  CHECK_EQ(boot[1], 0x81);
  CHECK_EQ(boot[2], 0x92);
  detail::TmclReply rep{};
  std::array<uint8_t, 8> f{0xFF, 100, 6, 0, 0, 0, 7, 0};
  f[7] = detail::Sum8(std::span<const uint8_t>(f).first(7));
  CHECK(detail::TmclSpiDecode(f, rep));
  CHECK_EQ(rep.value, 7U);
  f[7] = static_cast<uint8_t>(f[7] + 1U);
  CHECK(!detail::TmclSpiDecode(f, rep));
}

TEST_CASE("Register frames: table layout, wide layout, UART layouts") {
  std::array<uint8_t, 6> f{};
  CHECK(detail::RegSpiRequest(0, 0x1E3, true, 0x03300FFFU, false, f));
  CHECK_EQ(f[0], 0x03); // write bit 41 and offset bit 8
  CHECK_EQ(f[1], 0xE3);
  CHECK(
      !detail::RegSpiRequest(0, 0x208, false, 0, false, f)); // 10-bit offset: only the wide layout
  CHECK(detail::RegSpiRequest(0, 0x208, false, 0, true, f));
  CHECK(!detail::RegSpiRequest(0, 0x400, false, 0, true, f));
  CHECK(!detail::RegSpiRequest(32, 0, false, 0, true, f));
  const std::array<uint8_t, 6> r{0x10, 0x01, 0x54, 0x4D, 0x00, 0x01};
  const auto d = detail::RegSpiDecode(r);
  CHECK(!d.boot);
  CHECK_EQ(d.counter, 1);
  CHECK(d.ok);
  CHECK_EQ(d.value, 0x544D0001U);
  const std::array<uint8_t, 6> first{0x80, 0, 0, 0, 0, 0};
  CHECK(detail::RegSpiDecode(first).boot);
  std::array<uint8_t, 9> u{};
  CHECK(detail::RegUartRequest({.module_address = 1, .command = 148, .block = 1, .offset = 0x7FF},
                               u));
  CHECK_EQ(u[2], 0xFF);
  CHECK_EQ(u[3], (1U << 3U) | 7U);
  CHECK(detail::RegUartRequest(
      {.module_address = 1, .command = 148, .block = 1, .offset = 0x7FF, .msb_fields = true}, u));
  CHECK_EQ(u[3], (7U << 5U) | 1U);
  CHECK(!detail::RegUartRequest({.module_address = 1, .command = 148, .block = 32}, u));
  CHECK(!detail::RegUartRequest({.module_address = 1, .command = 148, .offset = 0x800}, u));
}

TEST_CASE("Partition table matches the datasheet example") {
  PartitionEntry e;
  const std::array<uint8_t, 9> name{0x6D, 0x6F, 0x74, 0x6F, 0x72,
                                    0x5F, 0x63, 0x66, 0x67}; // motor_cfg
  for (std::size_t i = 0; i < name.size(); ++i) {
    e.name[i] = name[i];
  }
  e.type = 2;
  e.writable = true;
  e.offset = 0x1000;
  e.size = 0x1000;
  std::array<uint8_t, 40> out{};
  const auto r = BuildPartitionTable(std::span<const PartitionEntry>(&e, 1U), 19, 12, out);
  REQUIRE(r.ok());
  CHECK_EQ(r.value, 40U);
  const std::array<uint8_t, 40> want{0x28, 0,    0,    0,    0x5D, 0x26, 0xA0, 0x6C, 0xFE, 0xCA,
                                     1,    1,    0x13, 0x0C, 0,    0,    0x6D, 0x6F, 0x74, 0x6F,
                                     0x72, 0x5F, 0x63, 0x66, 0x67, 0,    0,    0,    0x82, 0,
                                     0,    0,    0,    0x10, 0,    0,    0,    0x10, 0,    0};
  CHECK(out == want);
  std::array<uint8_t, 40> blank{};
  e.type = 3;
  CHECK_EQ(BuildPartitionTable(std::span<const PartitionEntry>(&e, 1U), 19, 12, blank).error,
           Error::InvalidArgument);
  e.type = 2;
  e.size = 0x800;
  CHECK_EQ(BuildPartitionTable(std::span<const PartitionEntry>(&e, 1U), 19, 12, blank).error,
           Error::InvalidArgument);
  e.size = 0x1000;
  e.offset = 0x10;
  CHECK_EQ(BuildPartitionTable(std::span<const PartitionEntry>(&e, 1U), 19, 12, blank).error,
           Error::InvalidArgument);
  e.offset = 0x1000;
  std::array<uint8_t, 8> small{};
  CHECK_EQ(BuildPartitionTable(std::span<const PartitionEntry>(&e, 1U), 19, 12, small).error,
           Error::InvalidArgument);
  const std::array<uint8_t, 40> zeros{};
  CHECK(blank == zeros);
  const std::array<PartitionEntry, 2> two{e, e};
  std::array<uint8_t, 64> big{};
  CHECK_EQ(BuildPartitionTable(two, 19, 12, big).error, Error::InvalidArgument);
}

TEST_CASE("BootConfig fields: placement, range, round trip") {
  BootConfig c;
  CHECK_EQ(c.Set(BootField::DeviceAddress, 0x55), Error::Ok);
  CHECK_EQ(c.Set(BootField::HostAddress, 0xAA), Error::Ok);
  CHECK_EQ(c.Word(0), 0xAA55'0000U);
  CHECK_EQ(c.Set(BootField::BootMode, 2), Error::Ok);
  CHECK_EQ(c.Set(BootField::StartMotorCtrl, 1), Error::Ok);
  CHECK_EQ(c.Word(2), 0x1002U);
  CHECK_EQ(c.Set(BootField::ClockRdiv, 14), Error::Ok);
  CHECK_EQ(c.Word(6), 14U << 18);
  CHECK_EQ(c.Set(BootField::ClockPllOutSel, 1), Error::Ok);
  CHECK_EQ(c.Set(BootField::ClockSysClkDiv, 0), Error::Ok);
  CHECK_EQ(c.Get(BootField::ClockRdiv), 14U);
  CHECK_EQ(c.Set(BootField::BootMode, 4), Error::InvalidArgument);
  CHECK_EQ(c.Get(BootField::BootMode), 2U);
  CHECK_EQ(c.Set(BootField::Gpio16To18PullUp, 7), Error::Ok);
  CHECK_EQ(c.Word(5), 7U << (16U + 9U));
  CHECK_EQ(c.Set(BootField::HallU, 3), Error::Ok);
  CHECK_EQ(c.Word(8), 3U << 4);
  CHECK_EQ(c.Set(BootField::Abn2B, 3), Error::Ok);
  CHECK_EQ(c.Word(8), (3U << 4) | (3U << (16U + 14U)));
  CHECK_EQ(c.Set(BootField::MemParameters, 3), Error::Ok);
  CHECK_EQ(c.Word(10), 3U << 2);
  CHECK_EQ(c.CheckStart(), Error::Ok);
  CHECK_EQ(c.SetWord(16, 0), Error::InvalidArgument);
  CHECK_EQ(c.Word(16), 0U);
  const BootConfig d = c;
  CHECK(d == c);
  CHECK_EQ(BootConfig::MaxOf(BootField::GpioOut), 0xFFFFU);
  const auto last =
      static_cast<unsigned>(detail::ToUnderlying(detail::EnumLast<BootField>::kValue));
  for (unsigned i = 0; i <= last; ++i) { // every field accepts its maximum and rejects one more
    BootConfig t;
    const auto f = static_cast<BootField>(i);
    CHECK_EQ(t.Set(f, BootConfig::MaxOf(f)), Error::Ok);
    CHECK_EQ(t.Get(f), BootConfig::MaxOf(f));
    CHECK_EQ(t.Set(f, BootConfig::MaxOf(f) + 1U), Error::InvalidArgument);
  }
}

TEST_CASE("CheckStart refuses an unsafe start") {
  BootConfig c;
  CHECK_EQ(c.Set(BootField::StartMotorCtrl, 1), Error::Ok);
  CHECK_EQ(c.CheckStart(), Error::Refused); // reserved boot mode
  CHECK_EQ(c.Set(BootField::BootMode, 3), Error::Ok);
  CHECK_EQ(c.CheckStart(), Error::Refused);
  CHECK_EQ(c.Set(BootField::BootMode, 1), Error::Ok);
  CHECK_EQ(c.CheckStart(), Error::Refused); // clock not 40 MHz
  CHECK_EQ(c.Set(BootField::ClockPllOutSel, 1), Error::Ok);
  CHECK_EQ(c.Set(BootField::ClockSysClkDiv, 3), Error::Ok);
  CHECK_EQ(c.CheckStart(), Error::Refused);
  CHECK_EQ(c.Set(BootField::ClockSysClkDiv, 0), Error::Ok);
  CHECK_EQ(c.CheckStart(), Error::Ok);
}

TEST_CASE("Parameter, global and register tables") {
  CHECK_EQ(kParams.size(), 289U);
  const ParamInfo* p = FindParam(4);
  REQUIRE(p != nullptr);
  CHECK(p->coded);
  CHECK_EQ(p->max, 8);
  CHECK(FindParam(2) != nullptr);
  CHECK(FindParam(10000) == nullptr);
  CHECK(FindParam(289)->access == Access::ReadWriteClear);
  CHECK(FindGlobal(0, 1) != nullptr);
  CHECK(FindGlobal(0, 8) == nullptr);
  CHECK(FindGlobal(2, 15) != nullptr);
  CHECK(FindGlobal(2, 16) == nullptr);
  CHECK(FindGlobal(3, 31) != nullptr);
  CHECK(FindGlobal(3, 5) == nullptr);
  for (std::size_t i = 1; i < kParams.size(); ++i) {
    CHECK(kParams[i - 1U].id < kParams[i].id); // sorted: the binary search relies on it
  }
  const RegInfo* r = FindRegister(regs::MCC_INFO_CHIP);
  REQUIRE(r != nullptr);
  CHECK_EQ(r->reset, 0x544D0001U);
  CHECK_EQ(r->writable_mask, 0U);
  CHECK_EQ(kRegisters.size(), 163U);
  CHECK(FindRegister(RegAddr{Block::Mcc, 0x7FF}) == nullptr);
  const RegInfo* st = FindRegister(regs::MCC_GDRV_STATUS);
  REQUIRE(st != nullptr);
  CHECK(st->w1c_mask != 0U);
  CHECK_EQ(fields::mcc_pwm_config::Chop::kMask, 7U);
}
