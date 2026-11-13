/**
 * @file test_start_follow.cpp
 * @brief Starting the application from the bootloader (R5-1): a start write whose reply is lost,
 * late, corrupt or BUSY may still have started the application, so the driver follows the start
 * and never reports a stop as done while the application runs; a write of the boot-mode word with
 * START_MOTOR_CTRL already set is a start too (A24).
 */
#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

namespace {

/// Whichever transfer of BlStartApplication() goes wrong (a late UART reply or a lost transfer),
/// a stop that says Ok afterwards leaves the application's PWM off.
template <typename R, bool Late>
void StartSweep() {
  bool saw_started = false;
  for (int k = 0; k < 80; ++k) {
    R r(Phase::Bootloader, ConfigFor(Mode::Bootloader));
    REQUIRE(r.Start());
    if constexpr (Late) {
      r.state.late_at = r.state.transfers + k;
    } else {
      r.state.fail_transfer = r.state.transfers + k;
    }
    const auto s = r.drv.BlStartApplication(BootMode::Parameter);
    r.state.late_at = -1;
    r.state.fail_transfer = -1;
    if (s.ok() || (r.state.chip.GetPhase() == Phase::Bootloader)) {
      CHECK(!s.ok() || (r.drv.GetMode() == Mode::Parameter));
      continue;
    }
    saw_started = true; // the application runs although the call failed
    CHECK((r.drv.GetMode() != Mode::Bootloader) || !r.drv.IsInitialized());
    r.state.chip.SetParamValue(4U, 3); // the application energises by itself
    const auto d = r.drv.DisableDrive();
    CHECK(!d.ok() || !r.state.chip.PwmActive());
  }
  static_cast<void>(saw_started);
}

} // namespace

TEST_CASE("A start whose reply is late on UART: the driver follows it, no silent stop") {
  StartSweep<UartRig, true>();
  int k = 0;
  bool found = false;
  for (; (k < 80) && !found; ++k) { // find the k whose late reply leaves the application running
    UartRig q(Phase::Bootloader, ConfigFor(Mode::Bootloader));
    REQUIRE(q.Start());
    q.state.late_at = q.state.transfers + k;
    const auto s = q.drv.BlStartApplication(BootMode::Parameter);
    if (q.state.chip.GetPhase() == Phase::Parameter) {
      found = true;
      CHECK(q.drv.GetMode() == Mode::Parameter);
      CHECK(!s.ok() || q.drv.IsInitialized());
      q.state.chip.SetParamValue(4U, 3);
      static_cast<void>(q.drv.DisableDrive());
      CHECK_EQ(q.state.chip.ParamValue(4U), 0);
    }
  }
  CHECK(found);
}

TEST_CASE("A start whose transfer fails on SPI or UART: the driver follows it, no silent stop") {
  StartSweep<SpiRig, false>();
  StartSweep<UartRig, false>();
}

TEST_CASE("Boot-mode word written with START already set: a start, under either reading (A24)") {
  for (const bool on_set : {false, true}) {
    SpiRig r(Phase::Bootloader, ConfigFor(Mode::Bootloader));
    r.state.chip.knobs.start_on_set = on_set;
    REQUIRE(r.Start());
    const auto cur = r.drv.BlReadConfig();
    REQUIRE(cur.ok());
    BootConfig first = cur.value;
    REQUIRE(Error::Ok == first.Set(BootField::StartMotorCtrl, 1U)); // START set, boot mode reserved
    r.state.chip.SetCfgWord(2U, first.Word(2U));                    // as the bootloader holds it
    BootConfig next = first;
    REQUIRE(Error::Ok == next.Set(BootField::BootMode, 2U));
    const auto s = r.drv.BlWriteConfig(next);
    if (r.state.chip.GetPhase() == Phase::Parameter) {
      CHECK(s.ok()); // the application started and was adopted
      CHECK(r.drv.GetMode() == Mode::Parameter);
      CHECK(r.drv.IsInitialized());
    } else {
      // Not started: the driver finds the bootloader still answering and stays in it.
      CHECK(!s.ok());
      CHECK(r.drv.GetMode() == Mode::Bootloader);
      CHECK(r.drv.IsInitialized());
    }
    CHECK_EQ(r.state.chip.GetPhase() == Phase::Parameter, on_set);
  }
}

TEST_CASE("No DRV_EN hook, bootloader adopted: the stop checks the bootloader still runs") {
  SpiRig s(Phase::Bootloader, ConfigFor(Mode::Bootloader));
  REQUIRE(s.Start());
  CHECK(s.drv.DisableDrive().ok());        // the bootloader answers: its gate driver is off
  s.state.chip.BootInto(Phase::Parameter); // an unseen restart into an application
  s.state.chip.SetParamValue(4U, 3);
  CHECK(!s.drv.DisableDrive().ok()); // not a silent Ok
  UartRig u(Phase::Bootloader, ConfigFor(Mode::Bootloader));
  REQUIRE(u.Start());
  CHECK(u.drv.DisableDrive().ok());
  u.state.chip.BootInto(Phase::Parameter);
  u.state.chip.SetParamValue(4U, 3);
  CHECK(!u.drv.DisableDrive().ok());
}

TEST_CASE("No DRV_EN hook, bootloader SPI: the stop's check does not swallow the reset evidence") {
  SpiRig r(Phase::Bootloader, ConfigFor(Mode::Bootloader));
  REQUIRE(r.Start());
  r.state.chip.Reset(); // the bootloader restarted unseen
  CHECK_EQ(r.drv.DisableDrive().error, Error::UnexpectedReset);
  CHECK(!r.drv.IsInitialized());
  const int writes = r.state.chip.writes_since_reset;
  CHECK_EQ(r.drv.BlWrite32(Bank::SpiFlash, 0x100U, 1U).error, Error::NotInitialized);
  CHECK_EQ(r.state.chip.writes_since_reset, writes);
}
