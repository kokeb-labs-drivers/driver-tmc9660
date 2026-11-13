/**
 * @file test_layout.cpp
 * @brief The TMCL type/motor layout (A2): golden frames on the wire, the start-up probe, and that
 * a wrong layout never lets a write reach the chip.
 */
#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

namespace {

Config Cfg(TmclLayout l = TmclLayout::Auto) {
  Config c = ConfigFor(Mode::Parameter);
  c.quirks.tmcl_layout = l;
  return c;
}

} // namespace

TEST_CASE("Auto picks the chip's layout on UART and the wire bytes match the manual") {
  UartRig m(Phase::Parameter, Cfg());
  m.state.chip.knobs.tmcl_wire = TmclWire::MotorHigh;
  REQUIRE(m.Start());
  REQUIRE(m.drv.ReadRawParam(299U, 0U).ok()); // GENERAL_ERROR_FLAGS
  CHECK_EQ(m.state.last_tx[2], 0x2B);         // type[7:0]
  CHECK_EQ(m.state.last_tx[3], 0x01);         // motor 0 << 4 | type[11:8]
  UartRig t(Phase::Parameter, Cfg());
  t.state.chip.knobs.tmcl_wire = TmclWire::TypeHigh;
  REQUIRE(t.Start());
  REQUIRE(t.drv.ReadRawParam(299U, 0U).ok());
  CHECK_EQ(t.state.last_tx[3], 0x10);
}

TEST_CASE("Auto picks the chip's layout on SPI") {
  SpiRig m(Phase::Parameter, Cfg());
  REQUIRE(m.Start());
  REQUIRE(m.drv.SetParam(Param::OVERCURRENT_PROTECTION_Y2_LOW_SIDE_THRESHOLD, 5).ok());
  CHECK_EQ(m.state.chip.ParamValue(260U), 5);
  CHECK_EQ(m.state.chip.ParamValue(4U), 0);
  SpiRig t(Phase::Parameter, Cfg());
  t.state.chip.knobs.tmcl_wire = TmclWire::TypeHigh;
  REQUIRE(t.Start());
  REQUIRE(t.drv.SetParam(Param::OVERCURRENT_PROTECTION_Y2_LOW_SIDE_THRESHOLD, 5).ok());
  CHECK_EQ(t.state.chip.ParamValue(260U), 5);
  CHECK_EQ(t.state.chip.ParamValue(4U), 0);
}

TEST_CASE("A configured layout the chip does not answer refuses the start; nothing is written") {
  for (const TmclWire wire : {TmclWire::MotorHigh, TmclWire::TypeHigh}) {
    const TmclLayout wrong =
        (wire == TmclWire::MotorHigh) ? TmclLayout::TypeHigh : TmclLayout::MotorHigh;
    SpiRig s(Phase::Parameter, Cfg(wrong));
    s.state.chip.knobs.tmcl_wire = wire;
    CHECK_EQ(s.drv.EnsureInitialized().error, Error::Refused);
    CHECK(!s.drv.IsInitialized());
    CHECK_EQ(s.state.chip.writes_since_reset, 0);
    CHECK_EQ(s.drv.SetParam(Param::OVERCURRENT_PROTECTION_Y2_LOW_SIDE_THRESHOLD, 5).error,
             Error::NotInitialized);
    CHECK_EQ(s.state.chip.ParamValue(4U), 0);
    CHECK_EQ(s.state.chip.energise_events, 0);
    UartRig u(Phase::Parameter, Cfg(wrong));
    u.state.chip.knobs.tmcl_wire = wire;
    CHECK_EQ(u.drv.EnsureInitialized().error, Error::Refused);
    CHECK_EQ(u.state.chip.writes_since_reset, 0);
  }
}

TEST_CASE(
    "A chip that ignores the motor nibble: the wrong layout would alias 260 to COMMUTATION_MODE") {
  // The scenario of the review: with the wrong layout SetParam(260, 5) would arrive as
  // COMMUTATION_MODE = 5. The start-up probe stops it before any write.
  UartRig u(Phase::Parameter, Cfg(TmclLayout::TypeHigh));
  u.state.chip.knobs.tmcl_wire = TmclWire::MotorHigh;
  u.state.chip.knobs.reject_motor = false;
  CHECK_EQ(u.drv.EnsureInitialized().error, Error::Refused);
  CHECK_EQ(u.drv.SetParam(Param::OVERCURRENT_PROTECTION_Y2_LOW_SIDE_THRESHOLD, 5).error,
           Error::NotInitialized);
  CHECK_EQ(u.state.chip.energise_events, 0);
  CHECK_EQ(u.state.chip.ParamValue(4U), 0);
}

TEST_CASE("The SPI Table 3 literal layout is not offered: a chip that uses it is refused") {
  SpiRig s(Phase::Parameter, Cfg());
  s.state.chip.knobs.tmcl_wire = TmclWire::SpiMsbFirst;
  CHECK_EQ(s.drv.EnsureInitialized().error, Error::Refused);
  CHECK_EQ(s.state.chip.writes_since_reset, 0);
}

TEST_CASE("Auto refuses when both layouts are answered; an explicit one is then accepted") {
  SpiRig a(Phase::Parameter, Cfg());
  a.state.chip.knobs.tmcl_wire = TmclWire::Lenient;
  CHECK_EQ(a.drv.EnsureInitialized().error, Error::Refused);
  CHECK_EQ(a.state.chip.writes_since_reset, 0);
  SpiRig b(Phase::Parameter, Cfg(TmclLayout::MotorHigh));
  b.state.chip.knobs.tmcl_wire = TmclWire::Lenient;
  CHECK(b.Start());
}

TEST_CASE("The probe sends reads only, and a bus failure in it is reported as such") {
  UartRig u(Phase::Parameter, Cfg());
  REQUIRE(u.Start());
  CHECK_EQ(u.state.chip.writes_since_reset, 1); // only the canary the driver arms afterwards
  for (int k = 1; k < 8; ++k) {
    UartRig f(Phase::Parameter, Cfg());
    f.state.fail_transfer = k;
    const auto r = f.drv.EnsureInitialized();
    if (!r.ok()) {
      CHECK(r.error != Error::Refused); // a lost frame is not a verdict on the layout
      CHECK(!f.drv.IsInitialized());
    }
  }
}

TEST_CASE("The layout is probed again after a restart") {
  UartRig u(Phase::Parameter, Cfg());
  REQUIRE(u.Start());
  u.state.chip.Reset();
  CHECK_EQ(u.drv.SetParam(Param::MAX_TORQUE, 5).error, Error::UnexpectedReset);
  u.state.chip.knobs.tmcl_wire = TmclWire::TypeHigh; // the new firmware packs it the other way
  CHECK(u.drv.EnsureInitialized().ok());
  REQUIRE(u.drv.ReadRawParam(299U, 0U).ok());
  CHECK_EQ(u.state.last_tx[3], 0x10);
}
