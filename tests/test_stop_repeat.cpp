/**
 * @file test_stop_repeat.cpp
 * @brief The parameter-mode stop is repeated once after any failure (a late reply, a lost frame),
 * then IDLE_MOTOR_PWM_BEHAVIOR is made safe; a failed EnableDrive() leaves the drive stopped also
 * without a DRV_EN hook; register-mode and bootloader details that no other test pinned.
 */
#include <cstdint>

#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

namespace {

constexpr unsigned kIdle = static_cast<unsigned>(Param::IDLE_MOTOR_PWM_BEHAVIOR);

Config Cfg(Mode m = Mode::Parameter) {
  return ConfigFor(m);
}

template <typename R>
void RunWithIdleZero(R& r) {
  REQUIRE(r.Start());
  REQUIRE(r.drv.EnableDrive(3).ok());
  r.state.chip.SetParamValue(kIdle, 0);
}

template <typename R>
void StopAfterLostFrame() {
  R r(Phase::Parameter, Cfg());
  RunWithIdleZero(r);
  r.state.fail_transfer = r.state.transfers; // the stop's frame never reaches the chip
  CHECK(r.drv.DisableDrive().ok());
  CHECK_EQ(r.state.chip.ParamValue(4U), 0);
  if constexpr (detail::HasGpioSet<decltype(r.port)>) {
    CHECK(!r.state.chip.DrvEn()); // the pin switches the stage off; the stored 0 is left alone
  } else {
    CHECK(!r.state.chip.PwmActive());
  }
}

/// Whichever transfer of EnableDrive() is lost, a failure leaves the drive stopped.
template <typename R, bool ExpectLostReply>
void EnableFailureStops() {
  bool saw_failure_after_write = false;
  for (int k = 0; k < 16; ++k) {
    R r(Phase::Parameter, Cfg());
    REQUIRE(r.Start());
    r.state.fail_transfer = r.state.transfers + k;
    const int energised = r.state.chip.energise_events;
    const auto e = r.drv.EnableDrive(3);
    if (!e.ok()) {
      CHECK_EQ(r.state.chip.ParamValue(4U), 0);
      CHECK(!r.state.chip.PwmActive());
      saw_failure_after_write =
          saw_failure_after_write || (r.state.chip.energise_events > energised);
    }
  }
  if constexpr (ExpectLostReply) {
    CHECK(saw_failure_after_write); // the case of R3-2: the chip ran the SAP, the reply was lost
  }
}

} // namespace

// --- R3-1: the parameter-mode stop is repeated after any failure -------------------------------

TEST_CASE("No DRV_EN hook, UART: a stop after a late reply ends Ok with the PWM off") {
  UartRig r(Phase::Parameter, Cfg());
  RunWithIdleZero(r);
  r.state.wire.late_next = true;
  CHECK(!r.drv.GetParam(Param::MAX_TORQUE).ok()); // a stale reply is now in the line
  const int drains = r.state.drains;
  const uint64_t waited = r.state.delay_us;
  const auto t0 = static_cast<std::size_t>(r.state.transfers);
  CHECK(r.drv.DisableDrive().ok());
  CHECK_EQ(r.state.cmd_log[t0], static_cast<uint8_t>(Tmcl::Sap)); // the stop, at once
  CHECK_EQ(r.state.cmd_log[t0 + 1U], 0U);                         // a drain before its repeat
  CHECK_EQ(r.state.cmd_log[t0 + 2U], 0U);
  CHECK(!r.state.chip.PwmActive());
  CHECK_EQ(r.state.chip.ParamValue(kIdle), 1);
  CHECK(r.state.drains > drains); // the repeat waited for the resync and drained the line
  CHECK((r.state.delay_us - waited) >= r.drv.GetConfig().uart_resync_us);
  CHECK_EQ(r.state.wire.Pending(), 0U);
}

TEST_CASE("No DRV_EN hook: a stop whose frame is lost is sent again, SPI and UART") {
  StopAfterLostFrame<SpiRig>();
  StopAfterLostFrame<UartRig>();
}

TEST_CASE("With a DRV_EN hook a lost stop frame is sent again too") {
  StopAfterLostFrame<SpiFullRig>();
  StopAfterLostFrame<UartFullRig>();
}

TEST_CASE("UART: MotorStop after a late reply is confirmed by its repeat") {
  UartRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  r.state.wire.late_next = true;
  CHECK(!r.drv.GetParam(Param::MAX_TORQUE).ok());
  CHECK(r.drv.MotorStop().ok());
  r.state.fail_transfer = r.state.transfers; // and a lost MST frame is sent again
  CHECK(r.drv.MotorStop().ok());
}

// --- R3-2: a failed EnableDrive() stops the drive ------------------------------------------------

TEST_CASE("EnableDrive: a failure after the COMMUTATION_MODE write stops the drive, no hook") {
  EnableFailureStops<SpiRig, true>();
  EnableFailureStops<UartRig, false>(); // a lost UART frame never reaches the chip: see below
  UartRig u(Phase::Parameter, Cfg());
  REQUIRE(u.Start());
  u.state.wire.late_op = static_cast<int>(Tmcl::Sap); // the chip runs it, the reply is late
  const int energised = u.state.chip.energise_events;
  CHECK(!u.drv.EnableDrive(3).ok());
  CHECK(u.state.chip.energise_events > energised); // the SAP did run
  CHECK_EQ(u.state.chip.ParamValue(4U), 0);
  CHECK(!u.state.chip.PwmActive());
}

TEST_CASE("EnableDrive: a failure after the COMMUTATION_MODE write stops the drive, with hook") {
  EnableFailureStops<SpiFullRig, true>();
}

// --- m1: the register-mode stop is not gated by FAULTN -------------------------------------------

TEST_CASE("Register UART with faultn_is_reset and FAULTN low: the stop still zeroes both words") {
  Config c = Cfg(Mode::Register);
  c.faultn_is_reset = true;
  UartFullRig r(Phase::Register, c);
  REQUIRE(r.Start());
  REQUIRE(r.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  r.state.faultn_high = false;
  const int frames = r.state.transfers;
  static_cast<void>(r.drv.DisableDrive());
  CHECK(r.drv.IsInitialized()); // the stop did not consult FAULTN (which would read as a reset)
  CHECK_EQ(r.state.cmd_log[static_cast<std::size_t>(frames)], detail::kRegUartWrite);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW) & 0x0300'080FU, 0U);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_PWM_CONFIG) & 0x0000'0F07U, 0U);
}

// --- m2: a bootloader SPI read after an unseen reset ---------------------------------------------

TEST_CASE("Bootloader SPI: a read after an unseen reset is UnexpectedReset, not data") {
  SpiRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
  REQUIRE(r.Start());
  r.state.chip.Reset();
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ChipType).error, Error::UnexpectedReset);
  CHECK(!r.drv.IsInitialized());
}

// --- m3: a failed ReturnToBootloader() keeps parameter mode --------------------------------------

TEST_CASE(
    "ReturnToBootloader that fails leaves parameter mode, so the stop still reaches the chip") {
  bool saw = false;
  for (int k = 0; k < 20; ++k) {
    SpiFullRig r(Phase::Parameter, Cfg());
    REQUIRE(r.Start());
    r.state.fail_transfer = r.state.transfers + k;
    const auto b = r.drv.ReturnToBootloader();
    r.state.fail_transfer = -1;
    if (b.ok() || (r.state.chip.GetPhase() != Phase::Parameter)) {
      continue;
    }
    saw = true;
    CHECK(r.drv.GetMode() == Mode::Parameter);
    r.state.chip.SetParamValue(4U, 3); // the chip runs again (a script)
    static_cast<void>(r.drv.DisableDrive());
    CHECK_EQ(r.state.chip.ParamValue(4U), 0);
  }
  CHECK(saw);
}
