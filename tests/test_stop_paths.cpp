/**
 * @file test_stop_paths.cpp
 * @brief The stop paths: Deinitialize and DisableDrive after a reset, SPI and UART, parameter and
 * register mode; the read-back of the register-mode stop; the transfer buffers.
 *
 * The chip model does not touch the DRV_EN pin when it resets (the host drives the pin), so a stop
 * that is skipped leaves the pin high here, as it would on the board.
 */
#include <bit>
#include <cstdint>

#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

namespace {

constexpr uint32_t kGdrvHwOn = 0x0300'080FU;
constexpr uint32_t kPwmOn = 0x0000'0F07U;

Config Cfg(Mode m = Mode::Parameter) {
  return ConfigFor(m);
}

/// The chip restarts into an application that energises by itself (a stored script): the pin the
/// host drives is still high and the chip holds COMMUTATION_MODE = 3.
template <typename R>
void RestartIntoEnergisedParameter(R& r) {
  r.state.chip.Reset();
  r.state.chip.SetParamValue(4U, 3);
}

/// The same for register mode.
template <typename R>
void RestartIntoEnergisedRegister(R& r) {
  r.state.chip.Reset();
  r.state.chip.SetRegValue(regs::MCC_GDRV_HW, kGdrvHwOn);
  r.state.chip.SetRegValue(regs::MCC_PWM_CONFIG, kPwmOn);
}

} // namespace

// --- R1-1: Deinitialize after a reset report must still stop -------------------------------------

TEST_CASE("Deinitialize after UnexpectedReset drops DRV_EN and stops, SPI parameter mode") {
  SpiFullRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  REQUIRE(r.drv.EnableDrive(3).ok());
  CHECK(r.state.chip.DrvEn());
  RestartIntoEnergisedParameter(r);
  CHECK(r.state.chip.DrvEn()); // the model leaves the pin alone: the reset does not clear it
  CHECK_EQ(r.drv.SetParam(Param::MAX_TORQUE, 100).error, Error::UnexpectedReset);
  CHECK(!r.drv.IsInitialized());
  const int pin_writes = r.state.drv_en_writes;
  const int stops = r.state.chip.disable_cmds;
  CHECK(r.drv.Deinitialize().ok());
  CHECK(r.state.drv_en_writes > pin_writes);
  CHECK(!r.state.chip.DrvEn());
  CHECK(r.state.chip.disable_cmds > stops);
  CHECK_EQ(r.state.chip.ParamValue(4U), 0);
}

TEST_CASE("Deinitialize after UnexpectedReset drops DRV_EN and stops, UART parameter mode") {
  UartFullRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  REQUIRE(r.drv.EnableDrive(3).ok());
  RestartIntoEnergisedParameter(r);
  CHECK(r.state.chip.DrvEn());
  CHECK_EQ(r.drv.SetParam(Param::MAX_TORQUE, 100).error, Error::UnexpectedReset); // the canary
  CHECK(!r.drv.IsInitialized());
  const int stops = r.state.chip.disable_cmds;
  CHECK(r.drv.Deinitialize().ok());
  CHECK(!r.state.chip.DrvEn());
  CHECK(r.state.chip.disable_cmds > stops);
  CHECK_EQ(r.state.chip.ParamValue(4U), 0);
}

TEST_CASE("Deinitialize after UnexpectedReset drops DRV_EN and zeroes the enables, register SPI") {
  SpiFullRig r(Phase::Register, Cfg(Mode::Register));
  REQUIRE(r.Start());
  REQUIRE(r.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  CHECK(r.state.chip.DrvEn());
  RestartIntoEnergisedRegister(r);
  CHECK_EQ(r.drv.ReadReg(regs::MCC_INFO_CHIP).error, Error::UnexpectedReset); // the boot bit
  CHECK(!r.drv.IsInitialized());
  CHECK(r.drv.Deinitialize().ok());
  CHECK(!r.state.chip.DrvEn());
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW) & kGdrvHwOn, 0U);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_PWM_CONFIG) & kPwmOn, 0U);
}

TEST_CASE("Deinitialize after a reset nobody noticed stops the chip, register UART") {
  UartFullRig r(Phase::Register, Cfg(Mode::Register));
  REQUIRE(r.Start());
  REQUIRE(r.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  RestartIntoEnergisedRegister(r);
  r.state.faultn_high = false;
  r.state.chip.SetDrvEn(true);
  CHECK(r.drv.Deinitialize().ok());
  CHECK(!r.state.chip.DrvEn());
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW) & kGdrvHwOn, 0U);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_PWM_CONFIG) & kPwmOn, 0U);
}

TEST_CASE("Deinitialize before any start still drops the pin") {
  SpiFullRig r(Phase::Parameter, Cfg());
  r.state.chip.SetDrvEn(true);
  CHECK(r.drv.Deinitialize().ok());
  CHECK(!r.state.chip.DrvEn());
  // Before any start the driver speaks the Config's protocol: the parameter stop is sent too.
  CHECK(r.state.transfers > 0);
  CHECK_EQ(r.state.chip.ParamValue(4U), 0);
}

// --- R1-8: stops on SPI with a reset pending ----------------------------------------------------

TEST_CASE("DisableDrive on SPI with an unseen reset: the stop is sent and confirmed") {
  // The chip ignores the command that carries FIRST_CMD (the pessimistic reading of "the initial
  // datagram"): the stop must be sent again and answered.
  SpiFullRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  REQUIRE(r.drv.EnableDrive(3).ok());
  r.state.chip.ignore_first_command = true;
  RestartIntoEnergisedParameter(r);
  const int before = r.state.transfers;
  CHECK(r.drv.DisableDrive().ok());
  CHECK_EQ(r.state.chip.ParamValue(4U), 0);
  CHECK(!r.state.chip.DrvEn());
  // the command frame (FIRST_CMD), the command frame again, the reply request: no probe in front
  CHECK_EQ(r.state.transfers - before, 3);
}

TEST_CASE("MotorStop on SPI with an unseen reset: MST reaches the chip") {
  SpiRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  r.state.chip.Reset();
  const int stops = r.state.chip.stop_writes;
  CHECK(r.drv.MotorStop().ok());
  CHECK(r.state.chip.stop_writes > stops);
  CHECK_EQ(r.state.chip.last_cmd, static_cast<uint8_t>(Tmcl::Mst));
}

TEST_CASE("MotorStop on SPI with an unseen reset and a chip that ignores the first command") {
  SpiRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  r.state.chip.ignore_first_command = true;
  r.state.chip.Reset();
  const int stops = r.state.chip.stop_writes;
  CHECK(r.drv.MotorStop().ok());
  CHECK(r.state.chip.stop_writes > stops); // sent again after the frame that reported the reset
}

TEST_CASE("Register-mode stop on SPI with an unseen reset: both enables are written") {
  SpiFullRig r(Phase::Register, Cfg(Mode::Register));
  REQUIRE(r.Start());
  REQUIRE(r.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  RestartIntoEnergisedRegister(r);
  const int stops = r.state.chip.disable_cmds;
  CHECK(r.drv.DisableDrive().ok()); // the first datagram only reports the boot bit: write again
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW), 0U);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_PWM_CONFIG), 0U);
  CHECK(r.state.chip.disable_cmds >= (stops + 2));
}

TEST_CASE("Register-mode stop: a failed first write does not skip the second") {
  for (int k = 0; k < 2; ++k) { // the two datagrams of the first write
    SpiFullRig r(Phase::Register, Cfg(Mode::Register));
    REQUIRE(r.Start());
    REQUIRE(r.drv.GateDriverStart(GateDriverRegConfig{}).ok());
    r.state.fail_transfer = r.state.transfers + k; // inside the write of MCC_GDRV_HW
    const auto stop = r.drv.DisableDrive();        // a single failed write is retried once
    CHECK(stop.ok());
    CHECK(!r.state.chip.DrvEn());
    CHECK_EQ(r.state.chip.RegValue(regs::MCC_PWM_CONFIG) & kPwmOn, 0U); // the second write ran
  }
  UartFullRig u(Phase::Register, Cfg(Mode::Register));
  REQUIRE(u.Start());
  REQUIRE(u.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  u.state.fail_transfer = u.state.transfers; // the first stop write
  CHECK(u.drv.DisableDrive().ok());
  CHECK_EQ(u.state.chip.RegValue(regs::MCC_PWM_CONFIG) & kPwmOn, 0U);
}

// --- R1-7: the register-mode stop is read back --------------------------------------------------

TEST_CASE(
    "Register-mode stop reports VerifyFailed when the chip took the writes for something else") {
  // The chip uses the 10-bit offset layout (A3) and acknowledges unmapped offsets: with the
  // default layout the stop writes land on offsets that do not exist and read as success.
  SpiFullRig r(Phase::Register, Cfg(Mode::Register));
  r.state.chip.knobs.reg_wide = true;
  r.state.chip.reg_unmapped_ok = true;
  REQUIRE(r.Start()); // MCC_INFO_CHIP is block 0, offset 0: the same bits in both layouts
  r.state.chip.SetRegValue(regs::MCC_GDRV_HW, kGdrvHwOn);
  r.state.chip.SetRegValue(regs::MCC_PWM_CONFIG, kPwmOn);
  r.state.chip.SetDrvEn(true);
  const auto stop = r.drv.DisableDrive();
  CHECK_EQ(stop.error, Error::VerifyFailed);
  CHECK(!r.state.chip.DrvEn()); // the pin is dropped whatever the registers say
  CHECK((r.state.chip.RegValue(regs::MCC_GDRV_HW) & kGdrvHwOn) != 0U);
}

TEST_CASE("Register-mode stop: a clear PWM_CONFIG alone is not enough, nor a clear GDRV_HW alone") {
  {
    SpiRig r(Phase::Register, Cfg(Mode::Register));
    REQUIRE(r.Start());
    r.state.chip.ignored_reg_write_offset = static_cast<int>(regs::MCC_PWM_CONFIG.offset);
    r.state.chip.SetRegValue(regs::MCC_PWM_CONFIG, kPwmOn);
    CHECK_EQ(r.drv.DisableDrive().error, Error::VerifyFailed);
  }
  {
    SpiRig r(Phase::Register, Cfg(Mode::Register));
    REQUIRE(r.Start());
    r.state.chip.ignored_reg_write_offset = static_cast<int>(regs::MCC_GDRV_HW.offset);
    r.state.chip.SetRegValue(regs::MCC_GDRV_HW, kGdrvHwOn);
    CHECK_EQ(r.drv.DisableDrive().error, Error::VerifyFailed);
  }
  {
    SpiRig r(Phase::Register, Cfg(Mode::Register));
    REQUIRE(r.Start());
    r.state.chip.SetRegValue(regs::MCC_GDRV_HW, kGdrvHwOn);
    r.state.chip.SetRegValue(regs::MCC_PWM_CONFIG, kPwmOn);
    CHECK(r.drv.DisableDrive().ok()); // a working chip: the read-back is clear
  }
}

// --- R1-9: the transfer buffers belong to the driver object -------------------------------------

namespace {

template <typename R>
bool BuffersInsideDriver(const R& r) {
  const auto lo = std::bit_cast<std::uintptr_t>(&r.drv);
  const auto hi = lo + sizeof(r.drv);
  const auto tx = std::bit_cast<std::uintptr_t>(r.state.last_tx_ptr);
  const auto rx = std::bit_cast<std::uintptr_t>(r.state.last_rx_ptr);
  return (tx >= lo) && (tx < hi) && (rx >= lo) && (rx < hi);
}

} // namespace

TEST_CASE("The port is always handed buffers that live inside the driver object") {
  {
    SpiRig r(Phase::Parameter, Cfg());
    REQUIRE(r.Start());
    CHECK(r.drv.SetParam(Param::MAX_TORQUE, 100).ok());
    CHECK(BuffersInsideDriver(r));
    CHECK(r.drv.GetParam(Param::MAX_TORQUE).ok());
    CHECK(BuffersInsideDriver(r));
  }
  {
    UartRig r(Phase::Parameter, Cfg());
    REQUIRE(r.Start());
    CHECK(r.drv.SetParam(Param::MAX_TORQUE, 100).ok());
    CHECK(BuffersInsideDriver(r));
  }
  {
    SpiRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
    REQUIRE(r.Start());
    CHECK(r.drv.BlGetInfo(BlInfo::ChipType).ok());
    CHECK(BuffersInsideDriver(r));
  }
  {
    UartRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
    REQUIRE(r.Start());
    CHECK(r.drv.BlGetInfo(BlInfo::ChipType).ok());
    CHECK(BuffersInsideDriver(r));
  }
  {
    SpiRig r(Phase::Register, Cfg(Mode::Register));
    REQUIRE(r.Start());
    CHECK(r.drv.ReadReg(regs::MCC_INFO_CHIP).ok());
    CHECK(BuffersInsideDriver(r));
  }
  {
    UartRig r(Phase::Register, Cfg(Mode::Register));
    REQUIRE(r.Start());
    CHECK(r.drv.ReadReg(regs::MCC_INFO_CHIP).ok());
    CHECK(BuffersInsideDriver(r));
  }
}
