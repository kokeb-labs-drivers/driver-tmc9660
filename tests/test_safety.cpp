/**
 * @file test_safety.cpp
 * @brief Properties that hold across every mode and bus: resets, stop paths, nothing sent on bad
 * input.
 */
#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

namespace {

/// Every state-changing call of parameter mode, by number.
template <typename Drv>
Error ParamWrite(Drv& d, int which) {
  switch (which) {
  case 0:
    return d.SetParam(Param::MAX_TORQUE, 10).error;
  case 1:
    return d.SetTargetVelocity(100).error;
  case 2:
    return d.SetGlobal(GlobalBank::UserVars, 2, 5).error;
  case 3:
    return d.StoreParams().error;
  case 4:
    return d.EnableDrive(3).error;
  case 5:
    return d.SetIo(1, true).error;
  case 6:
    return d.ClearFlags(Param::GENERAL_ERROR_FLAGS, 1U).error;
  case 7:
    return d.ScriptRun(false, 0U).error;
  default:
    break;
  }
  return Error::InvalidArgument;
}

} // namespace

TEST_CASE("Parameter mode over SPI: after an unseen reset no call writes") {
  for (int which = 0; which < 8; ++which) {
    SpiFullRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
    REQUIRE(r.Start());
    r.state.chip.Reset();
    const Error e = ParamWrite(r.drv, which);
    CHECK_EQ(e, Error::UnexpectedReset);
    CHECK_EQ(r.state.chip.writes_since_reset, 0);
    CHECK_EQ(r.state.chip.energise_events, 0);
    CHECK(!r.drv.IsInitialized());
  }
}

TEST_CASE("Parameter mode over UART: the canary stops every write after an unseen reset") {
  for (int which = 0; which < 8; ++which) {
    UartFullRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
    REQUIRE(r.Start());
    r.state.chip.Reset();
    const Error e = ParamWrite(r.drv, which);
    CHECK_EQ(e, Error::UnexpectedReset);
    // Only a stop may pass: EnableDrive() refused by the canary still sends the full stop.
    CHECK_EQ(r.state.chip.writes_since_reset, r.state.chip.stop_writes);
    CHECK_EQ(r.state.chip.energise_events, 0);
  }
}

TEST_CASE("Register mode over SPI: after an unseen reset no write follows") {
  SpiFullRig r(Phase::Register, ConfigFor(Mode::Register));
  REQUIRE(r.Start());
  r.state.chip.BootInto(Phase::Register);
  CHECK_EQ(r.drv.WriteReg(regs::MCC_GDRV_CFG, 1U).error, Error::UnexpectedReset);
  CHECK_EQ(r.state.chip.writes_since_reset, 0);
  SpiFullRig g(Phase::Register, ConfigFor(Mode::Register));
  REQUIRE(g.Start());
  g.state.chip.BootInto(Phase::Register);
  CHECK(!g.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  CHECK_EQ(g.state.chip.energise_events, 0);
}

TEST_CASE("DisableDrive in every mode and under every single bus failure always drops the pin") {
  for (int k = 0; k < 8; ++k) {
    SpiFullRig a(Phase::Parameter, ConfigFor(Mode::Parameter));
    REQUIRE(a.Start());
    REQUIRE(a.drv.EnableDrive(3).ok());
    a.state.fail_transfer = a.state.transfers + k;
    static_cast<void>(a.drv.DisableDrive());
    CHECK(!a.state.chip.DrvEn());
    SpiFullRig b(Phase::Register, ConfigFor(Mode::Register));
    REQUIRE(b.Start());
    REQUIRE(b.drv.GateDriverStart(GateDriverRegConfig{}).ok());
    b.state.fail_transfer = b.state.transfers + k;
    static_cast<void>(b.drv.DisableDrive());
    CHECK(!b.state.chip.DrvEn());
    UartFullRig c(Phase::Parameter, ConfigFor(Mode::Parameter));
    REQUIRE(c.Start());
    REQUIRE(c.drv.EnableDrive(3).ok());
    c.state.chip.drop_uart_reply = true;
    static_cast<void>(c.drv.DisableDrive());
    CHECK(!c.state.chip.DrvEn());
  }
  SpiFullRig d(Phase::Bootloader, ConfigFor(Mode::Bootloader));
  REQUIRE(d.Start());
  d.state.chip.SetDrvEn(true);
  CHECK(d.drv.DisableDrive().ok());
  CHECK(!d.state.chip.DrvEn());
  SpiFullRig e(Phase::Parameter, ConfigFor(Mode::Parameter)); // never started: still attempted
  e.state.chip.SetDrvEn(true);
  static_cast<void>(e.drv.DisableDrive());
  CHECK(!e.state.chip.DrvEn());
}

TEST_CASE("MotorStop is never gated by a probe or a mode flag") {
  SpiRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
  REQUIRE(r.Start());
  const int frames = r.state.chip.frames;
  CHECK(r.drv.MotorStop().ok());
  CHECK(r.state.chip.frames > frames);
  CHECK_EQ(r.state.chip.last_cmd, static_cast<uint8_t>(Tmcl::Mst));
}

TEST_CASE("Nothing is sent when an argument is invalid, on every bus") {
  SpiFullRig s(Phase::Parameter, ConfigFor(Mode::Parameter));
  UartFullRig u(Phase::Parameter, ConfigFor(Mode::Parameter));
  REQUIRE(s.Start());
  REQUIRE(u.Start());
  const int ns = s.state.transfers;
  const int nu = u.state.transfers;
  CHECK_EQ(s.drv.EnableDrive(0).error, Error::InvalidArgument);
  CHECK_EQ(u.drv.EnableDrive(9).error, Error::InvalidArgument);
  CHECK_EQ(s.drv.SetParam(Param::MAX_TORQUE, -1).error, Error::InvalidArgument);
  CHECK_EQ(u.drv.SetParam(Param::MAX_TORQUE, 65536).error, Error::InvalidArgument);
  CHECK_EQ(s.drv.PowerDown(ut::RawEnum<PowerDownTime>(9)).error, Error::InvalidArgument);
  CHECK_EQ(u.drv.SetIo(20, true).error, Error::InvalidArgument);
  CHECK_EQ(s.state.transfers, ns);
  CHECK_EQ(u.state.transfers, nu);
}

TEST_CASE("Boot with the system on never reaches the chip") {
  for (int k = 0; k < 6; ++k) {
    SpiFullRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
    REQUIRE(r.Start());
    REQUIRE(r.drv.EnableDrive(3).ok());
    r.state.fail_transfer = r.state.transfers + k;
    static_cast<void>(r.drv.ReturnToBootloader());
    CHECK_EQ(r.state.chip.boot_with_drive_on, 0);
  }
}

TEST_CASE("The first transfer after a reset is never an all-zero frame") {
  SpiFullRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
  REQUIRE(r.Start());
  for (int i = 0; i < 20; ++i) {
    static_cast<void>(r.drv.GetParam(Param::MAX_TORQUE));
    static_cast<void>(r.drv.SetParam(Param::MAX_TORQUE, i));
  }
  r.state.chip.not_ready_replies = 4;
  static_cast<void>(r.drv.GetParam(Param::MAX_TORQUE));
  CHECK_EQ(r.state.chip.zero_frames, 0);
}

TEST_CASE("Service reads the chip and keeps the stop paths cheap") {
  SpiFullRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
  REQUIRE(r.Start());
  CHECK(r.drv.Service().ok());
  SpiFullRig b(Phase::Bootloader, ConfigFor(Mode::Bootloader));
  REQUIRE(b.Start());
  CHECK(b.drv.Service().ok());
}

TEST_CASE("Deinitialize drops the pin and the driver refuses further calls") {
  UartFullRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
  REQUIRE(r.Start());
  REQUIRE(r.drv.EnableDrive(2).ok());
  CHECK(r.drv.Deinitialize().ok());
  CHECK(!r.state.chip.DrvEn());
  CHECK_EQ(r.drv.EnableDrive(2).error, Error::NotInitialized);
  CHECK(r.Start()); // a second Begin after Deinitialize works
  CHECK_EQ(r.state.chip.energise_with_flag, 0);
}

TEST_CASE("EnableDrive accepts every commutation mode up to the highest") {
  for (uint8_t m = 1U; m <= 8U; ++m) {
    SpiFullRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
    REQUIRE(r.Start());
    CHECK(r.drv.EnableDrive(m).ok());
    CHECK_EQ(r.state.chip.energise_events, 1);
  }
}

TEST_CASE("Boot is not sent when the chip acknowledges the stop but stays on (erratum 3)") {
  SpiFullRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
  REQUIRE(r.Start());
  REQUIRE(r.drv.EnableDrive(3).ok());
  r.state.chip.ignore_stop = true;
  CHECK_EQ(r.drv.ReturnToBootloader().error, Error::Refused);
  CHECK_EQ(r.state.chip.boot_with_drive_on, 0);
  CHECK_EQ(r.drv.GetMode(), Mode::Parameter);
}

TEST_CASE("A chip restart between the command and its reply request is seen") {
  SpiFullRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
  REQUIRE(r.Start());
  REQUIRE(r.drv.SetParam(Param::MAX_TORQUE, 5).ok());
  r.state.chip.reset_on_reply_request = true;
  CHECK_EQ(r.drv.GetParam(Param::MAX_TORQUE).error, Error::UnexpectedReset);
  CHECK(!r.drv.IsInitialized());
  CHECK_EQ(r.state.chip.writes_since_reset, 0);
}

TEST_CASE("The reply request is repeated spi_reply_retries times and no more") {
  Config c = ConfigFor(Mode::Parameter);
  c.spi_reply_retries = 3U;
  for (int n = 0; n <= 5; ++n) {
    SpiFullRig r(Phase::Parameter, c);
    REQUIRE(r.Start());
    r.state.chip.not_ready_replies = n;
    const auto got = r.drv.GetParam(Param::MAX_TORQUE);
    CHECK_EQ(got.ok(), n <= 3);
  }
}
