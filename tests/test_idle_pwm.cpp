/**
 * @file test_idle_pwm.cpp
 * @brief The stop and PWM in System off: IDLE_MOTOR_PWM_BEHAVIOR, a stop that is sent before a UART
 * line is resynchronised, a dirty line that survives a restart, and the retry of a failed
 * register-mode stop write.
 *
 * PM "System Off": with IDLE_MOTOR_PWM_BEHAVIOR (parameter 9) at 0 the PWM stays active when
 * COMMUTATION_MODE is set to 0. The parameter acts when System off is entered. The chip model
 * implements exactly that (`PwmActive()`), independent of the driver.
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

/// A drive that runs with a stored IDLE_MOTOR_PWM_BEHAVIOR of 0.
template <typename R>
void RunWithIdleZero(R& r) {
  REQUIRE(r.Start());
  REQUIRE(r.drv.EnableDrive(3).ok());
  r.state.chip.SetParamValue(kIdle, 0);
  CHECK(r.state.chip.PwmActive());
}

template <typename R>
void NoHookStopEstablishesOff() {
  R r(Phase::Parameter, Cfg());
  RunWithIdleZero(r);
  CHECK(r.drv.DisableDrive().ok());
  CHECK(!r.state.chip.PwmActive());
  CHECK_EQ(r.state.chip.ParamValue(kIdle), 1);
  CHECK_EQ(r.state.chip.ParamValue(4U), 0);
}

/// Whichever transfer of the stop is lost, an `Ok` means the PWM is off.
template <typename R>
void NoHookStopNeverOkWithPwm() {
  bool saw_ok = false;
  bool saw_error = false;
  for (int k = 0; k < 14; ++k) {
    R r(Phase::Parameter, Cfg());
    RunWithIdleZero(r);
    r.state.fail_transfer = r.state.transfers + k;
    const auto s = r.drv.DisableDrive();
    if (s.ok()) {
      saw_ok = true;
      CHECK(!r.state.chip.PwmActive());
    } else {
      saw_error = true;
    }
  }
  CHECK(saw_ok);
  CHECK(saw_error);
}

} // namespace

TEST_CASE("No DRV_EN hook: the stop switches the PWM off though IDLE_MOTOR_PWM_BEHAVIOR was 0") {
  NoHookStopEstablishesOff<SpiRig>();
  NoHookStopEstablishesOff<UartRig>();
}

TEST_CASE("No DRV_EN hook: a stop is never Ok while the PWM may still be driven, SPI and UART") {
  NoHookStopNeverOkWithPwm<SpiRig>();
  NoHookStopNeverOkWithPwm<UartRig>();
}

TEST_CASE("No DRV_EN hook: a chip that keeps IDLE_MOTOR_PWM_BEHAVIOR at 0 fails the stop") {
  SpiRig r(Phase::Parameter, Cfg());
  RunWithIdleZero(r);
  r.state.chip.ignored_param = static_cast<int>(kIdle);
  CHECK_EQ(r.drv.DisableDrive().error, Error::VerifyFailed);
  CHECK(r.state.chip.PwmActive()); // reported, not hidden
}

TEST_CASE("No DRV_EN hook: the usual stop with the default parameter is one command") {
  SpiRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  REQUIRE(r.drv.EnableDrive(3).ok());
  const int stops = r.state.chip.disable_cmds;
  CHECK(r.drv.DisableDrive().ok());
  CHECK_EQ(r.state.chip.disable_cmds, stops + 1);
  CHECK(!r.state.chip.PwmActive());
}

TEST_CASE("With a DRV_EN hook the pin switches the stage off and the stored parameter is left") {
  SpiFullRig r(Phase::Parameter, Cfg());
  RunWithIdleZero(r);
  CHECK(r.drv.DisableDrive().ok());
  CHECK(!r.state.chip.DrvEn());
  CHECK_EQ(r.state.chip.ParamValue(kIdle), 0);
}

TEST_CASE("SetParam(IDLE_MOTOR_PWM_BEHAVIOR, 0) is Refused unless the Config allows it") {
  SpiRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  const int n = r.state.transfers;
  CHECK_EQ(r.drv.SetParam(Param::IDLE_MOTOR_PWM_BEHAVIOR, 0).error, Error::Refused);
  CHECK_EQ(r.state.transfers, n);
  CHECK_EQ(r.state.chip.ParamValue(kIdle), 1);
  CHECK(r.drv.SetParam(Param::IDLE_MOTOR_PWM_BEHAVIOR, 1).ok());
  Config c = Cfg();
  c.allow_pwm_in_system_off = true;
  SpiRig q(Phase::Parameter, c);
  REQUIRE(q.Start());
  CHECK(q.drv.SetParam(Param::IDLE_MOTOR_PWM_BEHAVIOR, 0).ok());
  CHECK_EQ(q.state.chip.ParamValue(kIdle), 0);
}

TEST_CASE("The stop forces IDLE_MOTOR_PWM_BEHAVIOR to 1 even when the Config allows 0") {
  Config c = Cfg();
  c.allow_pwm_in_system_off = true;
  SpiRig r(Phase::Parameter, c);
  REQUIRE(r.Start());
  REQUIRE(r.drv.SetParam(Param::IDLE_MOTOR_PWM_BEHAVIOR, 0).ok());
  REQUIRE(r.drv.EnableDrive(3).ok());
  CHECK(r.drv.DisableDrive().ok());
  CHECK(!r.state.chip.PwmActive());
}

// --- m1: a stop is sent before the line is resynchronised ---------------------------------------

TEST_CASE("UART: MotorStop after a failed exchange is the first thing sent, the resync follows") {
  UartRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  r.state.wire.late_next = true;
  CHECK(!r.drv.GetParam(Param::MAX_TORQUE).ok());
  const int t0 = r.state.transfers;
  const uint64_t w0 = r.state.delay_us;
  const int stops = r.state.chip.stop_writes;
  const int drains = r.state.drains;
  CHECK(r.drv.MotorStop().ok()); // the first reply is the stale one; the repeat confirms it
  // The first frame of the call is the stop itself, not a drain: no resync wait came before it.
  CHECK_EQ(r.state.cmd_log[static_cast<std::size_t>(t0)], static_cast<uint8_t>(Tmcl::Mst));
  CHECK(r.state.chip.stop_writes > stops);
  CHECK(r.state.drains > drains); // the repeat resynchronised
  CHECK((r.state.delay_us - w0) >= r.drv.GetConfig().uart_resync_us);
  CHECK_EQ(r.state.wire.Pending(), 0U);
}

TEST_CASE("UART: DisableDrive after a failed exchange sends the stop first, parameter mode") {
  UartFullRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  REQUIRE(r.drv.EnableDrive(3).ok());
  r.state.wire.late_next = true;
  CHECK(!r.drv.GetParam(Param::MAX_TORQUE).ok());
  const int t0 = r.state.transfers;
  CHECK(r.drv.DisableDrive().ok());
  CHECK_EQ(r.state.cmd_log[static_cast<std::size_t>(t0)], static_cast<uint8_t>(Tmcl::Sap));
  CHECK(!r.state.chip.DrvEn());
  CHECK_EQ(r.state.chip.ParamValue(4U), 0);
}

TEST_CASE("UART: DisableDrive after a failed exchange writes first, register mode") {
  UartFullRig r(Phase::Register, Cfg(Mode::Register));
  REQUIRE(r.Start());
  REQUIRE(r.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  r.state.wire.late_next = true;
  CHECK(!r.drv.ReadReg(regs::MCC_INFO_CHIP).ok());
  const int t0 = r.state.transfers;
  static_cast<void>(r.drv.DisableDrive()); // the repeat of a write that read a stale reply waits
  CHECK_EQ(r.state.cmd_log[static_cast<std::size_t>(t0)], detail::kRegUartWrite);
  CHECK(!r.state.chip.DrvEn());
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW) & 0x0300'080FU, 0U);
}

// --- m2: a restart does not forget a dirty line -------------------------------------------------

TEST_CASE("UART: EnsureInitialized keeps the dirty flag and drains before it identifies") {
  UartRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  r.state.wire.late_next = true;
  CHECK(!r.drv.GetParam(Param::MAX_TORQUE).ok());
  r.state.fail_transfer = r.state.transfers; // the stop frame, then the drain before its repeat
  r.state.fail_span = 2;
  CHECK(!r.drv.Deinitialize().ok()); // the line is still dirty, the late reply still on its way
  r.state.fail_transfer = -1;
  CHECK(!r.drv.IsInitialized());
  const int drains = r.state.drains;
  const uint64_t w0 = r.state.delay_us;
  CHECK(r.drv.EnsureInitialized().ok());
  CHECK(r.state.drains > drains);
  CHECK((r.state.delay_us - w0) >= r.drv.GetConfig().uart_resync_us);
  CHECK_EQ(r.state.wire.Pending(), 0U);
}

// --- m12: a single failed write of the register-mode stop is repeated ---------------------------

TEST_CASE("Register-mode stop: two lost writes in a row are an error, the next enable is cleared") {
  SpiFullRig s(Phase::Register, Cfg(Mode::Register));
  REQUIRE(s.Start());
  REQUIRE(s.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  s.state.fail_transfer = s.state.transfers;
  s.state.fail_span = 2; // the write and its repeat
  CHECK_EQ(s.drv.DisableDrive().error, Error::Bus);
  CHECK(!s.state.chip.DrvEn());
  CHECK_EQ(s.state.chip.RegValue(regs::MCC_PWM_CONFIG) & 0x0000'0F07U, 0U);
  UartFullRig u(Phase::Register, Cfg(Mode::Register));
  REQUIRE(u.Start());
  REQUIRE(u.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  u.state.fail_transfer = u.state.transfers;
  u.state.fail_span = 3; // the write, the drain before its repeat, the repeat
  CHECK_EQ(u.drv.DisableDrive().error, Error::Bus);
  CHECK_EQ(u.state.chip.RegValue(regs::MCC_PWM_CONFIG) & 0x0000'0F07U, 0U);
}
