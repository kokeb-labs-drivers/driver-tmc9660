/**
 * @file test_restart_mode.cpp
 * @brief The protocol the driver speaks survives a failed restart (R4-1), and a stop that cannot
 * know what the chip runs says so instead of returning Ok.
 */
#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

namespace {

/// Config in bootloader mode, the application started from it, then a restart that fails because
/// the chip is no longer in the bootloader: the stop must still reach the application.
template <typename R>
void StopAfterFailedRestart() {
  R r(Phase::Bootloader, ConfigFor(Mode::Bootloader));
  REQUIRE(r.Start());
  REQUIRE(r.drv.BlStartApplication(BootMode::Parameter).ok());
  REQUIRE(r.drv.GetMode() == Mode::Parameter);
  static_cast<void>(r.drv.Deinitialize());
  r.state.chip.SetParamValue(4U, 3);      // the application energises by itself (a stored script)
  CHECK(!r.drv.EnsureInitialized().ok()); // the bootloader does not answer: it is not running
  CHECK(r.drv.GetMode() == Mode::Parameter); // the last proved protocol is kept
  CHECK(r.drv.DisableDrive().ok());
  CHECK_EQ(r.state.chip.ParamValue(4U), 0);
  CHECK(!r.state.chip.PwmActive());
}

/// Never started, Config in bootloader mode, the chip runs an application, no DRV_EN hook: the
/// driver cannot know what to send and has no pin, so the stop is not Ok.
template <typename R>
void StopUnknownProtocolNoHook() {
  R r(Phase::Parameter, ConfigFor(Mode::Bootloader));
  CHECK(!r.drv.EnsureInitialized().ok());
  r.state.chip.SetParamValue(4U, 3);
  CHECK_EQ(r.drv.DisableDrive().error, Error::NotInitialized);
  CHECK_EQ(r.drv.Deinitialize().error, Error::NotInitialized);
}

} // namespace

TEST_CASE("A failed restart keeps the protocol: the stop reaches the application, SPI and UART") {
  StopAfterFailedRestart<SpiRig>();
  StopAfterFailedRestart<UartRig>();
  StopAfterFailedRestart<SpiFullRig>();
}

TEST_CASE("No DRV_EN hook and no proved protocol: DisableDrive() is not a silent Ok") {
  StopUnknownProtocolNoHook<SpiRig>();
  StopUnknownProtocolNoHook<UartRig>();
}

TEST_CASE("With a DRV_EN hook and no proved protocol the pin drop makes the stop Ok") {
  SpiFullRig r(Phase::Parameter, ConfigFor(Mode::Bootloader));
  CHECK(!r.drv.EnsureInitialized().ok());
  CHECK(r.drv.DisableDrive().ok());
  CHECK(!r.state.chip.DrvEn());
}

TEST_CASE("Initialised in the bootloader, the stop without a hook is Ok: the gate driver is off") {
  SpiRig r(Phase::Bootloader, ConfigFor(Mode::Bootloader));
  REQUIRE(r.Start());
  CHECK(r.drv.DisableDrive().ok());
}

// --- round 4 minors: guards no other test pinned ------------------------------------------------

TEST_CASE("Register-mode stop: a failed read-back is reported, whichever word") {
  for (int k = 2; k < 4; ++k) { // UART: write GDRV_HW, write PWM_CONFIG, read GDRV_HW, read PWM
    UartFullRig r(Phase::Register, ConfigFor(Mode::Register));
    REQUIRE(r.Start());
    REQUIRE(r.drv.GateDriverStart(GateDriverRegConfig{}).ok());
    r.state.fail_transfer = r.state.transfers + k;
    CHECK_EQ(r.drv.DisableDrive().error, Error::Bus);
  }
}

TEST_CASE("EnableDrive: a DRV_EN that cannot be raised is dropped again") {
  SpiFullRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
  REQUIRE(r.Start());
  r.state.chip.SetDrvEn(true);             // high from before
  r.state.pin_fail_at = r.state.pin_calls; // the ACTIVE write fails, the drop that follows works
  CHECK_EQ(r.drv.EnableDrive(3).error, Error::Bus);
  CHECK(!r.state.chip.DrvEn());
  CHECK_EQ(r.state.chip.energise_events, 0);
}

TEST_CASE("ReturnToBootloader: a stop whose pin write fails sends no Boot") {
  SpiFullRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
  REQUIRE(r.Start());
  r.state.pin_fail = true; // the chip part of the stop works, DRV_EN cannot be dropped
  CHECK_EQ(r.drv.ReturnToBootloader().error, Error::Bus);
  CHECK(r.state.chip.GetPhase() == Phase::Parameter);
  CHECK(r.drv.GetMode() == Mode::Parameter);
}

TEST_CASE("SPI: the stops send their command first, never a probe in front of it") {
  SpiFullRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
  REQUIRE(r.Start());
  REQUIRE(r.drv.EnableDrive(3).ok());
  int t0 = r.state.transfers;
  CHECK(r.drv.DisableDrive().ok());
  CHECK_EQ(r.state.spi_log[static_cast<std::size_t>(t0)], static_cast<uint8_t>(Tmcl::Sap));
  t0 = r.state.transfers;
  CHECK(r.drv.MotorStop().ok());
  CHECK_EQ(r.state.spi_log[static_cast<std::size_t>(t0)], static_cast<uint8_t>(Tmcl::Mst));
}

TEST_CASE("A failed hardware-reset restart keeps the protocol too") {
  Config c = ConfigFor(Mode::Bootloader);
  c.hardware_reset = true;
  SpiFullRig r(Phase::Bootloader, c);
  REQUIRE(r.Start());
  REQUIRE(r.drv.BlStartApplication(BootMode::Parameter).ok());
  static_cast<void>(r.drv.Deinitialize());
  r.state.chip.SetParamValue(4U, 3);
  r.state.pin_fail = true; // RSTN cannot be pulsed
  CHECK_EQ(r.drv.EnsureInitialized().error, Error::Bus);
  CHECK(r.drv.GetMode() == Mode::Parameter);
  r.state.pin_fail = false;
  static_cast<void>(r.drv.DisableDrive());
  CHECK_EQ(r.state.chip.ParamValue(4U), 0);
}

// --- round 5 minors
// -------------------------------------------------------------------------------

TEST_CASE("The UART resync waits at least 150 ms by default: 50 % over the chip's ~100 ms gap") {
  CHECK(Config{}.uart_resync_us >= 150'000U);
}

TEST_CASE("A parameter-mode start that fails after the identity check keeps parameter mode") {
  SpiRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
  REQUIRE(r.Start());
  REQUIRE(r.drv.ReturnToBootloader().ok());
  REQUIRE(r.drv.GetMode() == Mode::Bootloader);
  static_cast<void>(r.drv.Deinitialize());
  r.state.chip.BootInto(Phase::Parameter);          // the application runs again
  r.state.chip.knobs.tmcl_wire = TmclWire::Lenient; // the layout probe cannot decide: Refused
  CHECK_EQ(r.drv.EnsureInitialized().error, Error::Refused);
  CHECK(r.drv.GetMode() == Mode::Parameter); // the chip answered in parameter mode
  r.state.chip.SetParamValue(4U, 3);
  static_cast<void>(r.drv.DisableDrive());
  CHECK_EQ(r.state.chip.ParamValue(4U), 0);
}
