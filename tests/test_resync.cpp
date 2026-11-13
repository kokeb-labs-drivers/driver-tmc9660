/**
 * @file test_resync.cpp
 * @brief UART resynchronisation: a reply that arrives after the timeout must never answer the next
 * request (the TMCL reply carries no request identity).
 */
#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

namespace {
Config Cfg() {
  return ConfigFor(Mode::Parameter);
}
} // namespace

TEST_CASE("A late reply does not shift the next read, with a port that does not flush") {
  UartRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  r.state.chip.SetParamValue(static_cast<unsigned>(Param::MAX_TORQUE), 111);
  r.state.chip.SetParamValue(static_cast<unsigned>(Param::MAX_FLUX), 222);
  r.state.wire.port_flushes = false;
  r.state.wire.late_next = true;
  CHECK_EQ(r.drv.GetParam(Param::MAX_TORQUE).error, Error::Timeout);
  const uint64_t waited = r.state.delay_us;
  const auto b = r.drv.GetParam(Param::MAX_FLUX);
  REQUIRE(b.ok());
  CHECK_EQ(b.value, 222); // not the stale 111
  CHECK(r.state.drains >= 1);
  CHECK((r.state.delay_us - waited) >= r.drv.GetConfig().uart_resync_us);
  CHECK_EQ(r.state.wire.Pending(), 0U);
  const auto c = r.drv.GetParam(Param::MAX_TORQUE); // and every later reply is in step
  CHECK(c.ok());
  CHECK_EQ(c.value, 111);
  CHECK_EQ(r.state.wire.late_replies, 1);
}

TEST_CASE("A late reply with a port that flushes: nothing stale is left") {
  UartRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  r.state.chip.SetParamValue(static_cast<unsigned>(Param::MAX_FLUX), 222);
  r.state.wire.port_flushes = true;
  r.state.wire.late_next = true;
  CHECK(!r.drv.GetParam(Param::MAX_TORQUE).ok());
  const auto b = r.drv.GetParam(Param::MAX_FLUX);
  CHECK_EQ(b.value, 222);
  CHECK_EQ(r.state.wire.stale_bytes_seen, 0);
}

TEST_CASE("A corrupted reply also resynchronises before the next request") {
  UartRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  r.state.chip.corrupt_uart_reply = true;
  CHECK_EQ(r.drv.GetParam(Param::MAX_TORQUE).error, Error::BadReply);
  r.state.chip.corrupt_uart_reply = false;
  const int drains = r.state.drains;
  const uint64_t waited = r.state.delay_us;
  CHECK(r.drv.GetParam(Param::MAX_TORQUE).ok());
  CHECK(r.state.drains > drains);
  CHECK((r.state.delay_us - waited) >= r.drv.GetConfig().uart_resync_us);
}

TEST_CASE("A successful exchange does not drain or wait") {
  UartRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  const int drains = r.state.drains;
  const uint64_t waited = r.state.delay_us;
  CHECK(r.drv.GetParam(Param::MAX_TORQUE).ok());
  CHECK_EQ(r.state.drains, drains);
  CHECK_EQ(r.state.delay_us, waited);
}

TEST_CASE("EnableDrive never reads one flag word as the other, whichever reply is late") {
  // GDRV_ERROR_FLAGS is latched; GENERAL_ERROR_FLAGS is clear. A stale GENERAL reply taken for the
  // GDRV read would let the drive energise with a latched flag.
  for (int k = 0; k < 6; ++k) {
    UartFullRig r(Phase::Parameter, Cfg());
    REQUIRE(r.Start());
    r.state.chip.SetParamValue(300U, 1);
    r.state.wire.port_flushes = false;
    r.state.wire.late_next = false;
    // the k-th exchange of EnableDrive is late: arm the fault by skipping k exchanges first
    for (int i = 0; i < k; ++i) {
      CHECK(r.drv.GetParam(Param::MAX_TORQUE).ok());
    }
    r.state.wire.late_next = true;
    const auto e = r.drv.EnableDrive(3);
    CHECK(!e.ok());
    CHECK_EQ(r.state.chip.energise_events, 0);
    CHECK_EQ(r.state.chip.energise_with_flag, 0);
    CHECK_EQ(r.state.chip.ParamValue(4U), 0);
  }
}

TEST_CASE("Register and bootloader UART resynchronise too") {
  UartRig r(Phase::Register, ConfigFor(Mode::Register));
  REQUIRE(r.Start());
  r.state.wire.late_next = true;
  CHECK(!r.drv.ReadReg(regs::MCC_INFO_CHIP).ok());
  r.state.chip.SetRegValue(regs::MCC_GDRV_CFG, 0x55);
  const auto v = r.drv.ReadReg(regs::MCC_GDRV_CFG);
  CHECK_EQ(v.value, 0x55U); // not the chip id of the late reply
  UartRig b(Phase::Bootloader, ConfigFor(Mode::Bootloader));
  REQUIRE(b.Start());
  b.state.wire.late_next = true;
  CHECK(!b.drv.BlGetInfo(BlInfo::ChipType).ok());
  const auto m = b.drv.BlGetInfo(BlInfo::BlVersion);
  REQUIRE(m.ok());
  CHECK(m.value != kChipId);
}

TEST_CASE("A drain that fails leaves the line dirty: no stale reply answers a later read") {
  UartRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  r.state.chip.SetParamValue(static_cast<unsigned>(Param::MAX_TORQUE), 111);
  r.state.chip.SetParamValue(static_cast<unsigned>(Param::MAX_FLUX), 222);
  r.state.wire.late_next = true;
  CHECK(!r.drv.GetParam(Param::MAX_TORQUE).ok()); // its reply arrives late
  r.state.fail_transfer = r.state.transfers;      // the first drain of the resync fails
  const auto a = r.drv.GetParam(Param::MAX_FLUX);
  CHECK(!a.ok() || (a.value == 222));
  const auto b = r.drv.GetParam(Param::MAX_FLUX);
  CHECK(!b.ok() || (b.value == 222));
  const auto c = r.drv.GetParam(Param::MAX_FLUX);
  REQUIRE(c.ok());
  CHECK_EQ(c.value, 222);
}
