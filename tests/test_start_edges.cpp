/**
 * @file test_start_edges.cpp
 * @brief The edges of the application start and of the bootloader stop: what the start write
 * returns unchanged, a parameter start that fails inside the follow, the UART bank and address
 * before the start write, the bounded boot wait, and the recovery after an unseen restart into an
 * application in bootloader mode without a DRV_EN hook.
 */
#include <cstddef>
#include <cstdint>

#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

namespace {

Config BlCfg() {
  return ConfigFor(Mode::Bootloader);
}

/// Whether an application identity request (TMCL GetInfo over SPI) was sent in `[t0, t1)`.
template <typename R>
bool AppPolled(const R& r, int t0, int t1) {
  for (int i = t0; i < t1; ++i) {
    if (r.state.spi_log[static_cast<std::size_t>(i)] == static_cast<uint8_t>(Tmcl::GetInfo)) {
      return true;
    }
  }
  return false;
}

/// Index (relative to `t0`) of the UART request with command `cmd`, the last one in `[t0, t1)`.
template <typename R>
int LastUartCmd(const R& r, int t0, int t1, uint8_t cmd) {
  int found = -1;
  for (int i = t0; i < t1; ++i) {
    if (r.state.op_log[static_cast<std::size_t>(i)] == cmd) {
      found = i;
    }
  }
  return found;
}

} // namespace

// --- m1: the start write returns a refusal or a seen reset unchanged ----------------------------

TEST_CASE("A start write the bootloader refuses is Nack and nothing is followed") {
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  r.state.chip.refuse_cfg_word = 2; // the boot-mode word
  const int t0 = r.state.transfers;
  CHECK_EQ(r.drv.BlStartApplication(BootMode::Parameter).error, Error::Nack);
  CHECK(!AppPolled(r, t0, r.state.transfers)); // nothing followed
  CHECK(r.drv.GetMode() == Mode::Bootloader);
  CHECK(r.drv.IsInitialized());
  CHECK(r.state.chip.GetPhase() == Phase::Bootloader);
}

TEST_CASE("A reset the probe sees before the start write is returned, nothing is followed") {
  SpiRig dry(Phase::Bootloader, BlCfg());
  REQUIRE(dry.Start());
  const int d0 = dry.state.transfers;
  REQUIRE(dry.drv.BlStartApplication(BootMode::Parameter).ok());
  int w = -1; // the last bootloader Write32 datagram: the start write
  for (int i = d0; i < dry.state.transfers; ++i) {
    if (dry.state.spi_log[static_cast<std::size_t>(i)] == static_cast<uint8_t>(BlCmd::Write32)) {
      w = i;
    }
  }
  REQUIRE(w > d0);
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  const int t0 = r.state.transfers;
  r.state.reset_at_transfer = t0 + (w - 1 - d0); // the probe right before the start write
  CHECK_EQ(r.drv.BlStartApplication(BootMode::Parameter).error, Error::UnexpectedReset);
  CHECK(!AppPolled(r, t0, r.state.transfers)); // nothing followed
  CHECK(!r.drv.IsInitialized());
  CHECK(r.drv.GetMode() == Mode::Bootloader);
  CHECK(r.state.chip.GetPhase() == Phase::Bootloader);
}

// --- m2: a parameter start that fails inside the follow -----------------------------------------

TEST_CASE("The application answers but its layout probe fails: parameter mode, not initialised") {
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  r.state.chip.knobs.tmcl_wire = TmclWire::Lenient; // the probe cannot decide: Refused
  CHECK_EQ(r.drv.BlStartApplication(BootMode::Parameter).error, Error::Refused);
  CHECK(r.drv.GetMode() == Mode::Parameter);
  CHECK(!r.drv.IsInitialized());
  r.state.chip.SetParamValue(4U, 3);
  static_cast<void>(r.drv.DisableDrive());
  CHECK_EQ(r.state.chip.ParamValue(4U), 0);
}

// --- m3: UART bank and address before the start write --------------------------------------------

TEST_CASE("UART start write: the bank is selected again and the address set right before it") {
  UartRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  const int t0 = r.state.transfers;
  REQUIRE(r.drv.BlStartApplication(BootMode::Parameter).ok());
  const int w = LastUartCmd(r, t0, r.state.transfers, static_cast<uint8_t>(BlCmd::Write32));
  REQUIRE(w >= 2);
  CHECK_EQ(r.state.op_log[static_cast<std::size_t>(w - 1)],
           static_cast<uint8_t>(BlCmd::SetAddress));
  CHECK_EQ(r.state.op_log[static_cast<std::size_t>(w - 2)], static_cast<uint8_t>(BlCmd::SetBank));
}

TEST_CASE("UART start write: a failed SetAddress is returned without waiting for an application") {
  UartRig dry(Phase::Bootloader, BlCfg());
  REQUIRE(dry.Start());
  const int d0 = dry.state.transfers;
  REQUIRE(dry.drv.BlStartApplication(BootMode::Parameter).ok());
  const int w = LastUartCmd(dry, d0, dry.state.transfers, static_cast<uint8_t>(BlCmd::Write32));
  REQUIRE(w >= 1);
  UartRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  r.state.fail_transfer = r.state.transfers + (w - 1 - d0); // the SetAddress of the start write
  const int before = r.state.transfers;
  CHECK_EQ(r.drv.BlStartApplication(BootMode::Parameter).error, Error::Bus);
  CHECK(r.drv.GetMode() == Mode::Bootloader);
  CHECK((r.state.transfers - before) <= (w - d0)); // nothing after: no identity polling
  CHECK(r.state.chip.GetPhase() == Phase::Bootloader);
}

// --- m4: the boot wait is bounded by boot_timeout_ms ---------------------------------------------

TEST_CASE("UART: the identity wait counts each failed poll's reply wait, resync and drains") {
  Config c = ConfigFor(Mode::Parameter);
  c.hardware_reset = true;
  c.boot_timeout_ms = 450;
  UartFullRig r(Phase::Parameter, c);
  r.state.chip.drop_uart_reply = true; // the chip never answers
  CHECK(!r.drv.EnsureInitialized().ok());
  const int requests = r.state.transfers - r.state.drains;
  // One failed poll costs 10 + 50 + 150 + 3 * 5 = 225 ms with the defaults: 450 ms allow exactly
  // two. Without the pause 215 ms, the reply wait 175, the resync 75 or the drains 210 ms leave a
  // third poll.
  CHECK_EQ(requests, 2);
}

TEST_CASE("UART: a budget below one poll still polls once") {
  Config c = ConfigFor(Mode::Parameter);
  c.hardware_reset = true;
  c.boot_timeout_ms = 1;
  UartFullRig r(Phase::Parameter, c);
  r.state.chip.drop_uart_reply = true;
  CHECK_EQ(r.drv.EnsureInitialized().error, Error::Timeout);
  CHECK_EQ(r.state.transfers - r.state.drains, 1);
}

TEST_CASE("A chip of another type is DeviceIdMismatch at the end of the boot wait") {
  Config c = ConfigFor(Mode::Parameter);
  c.hardware_reset = true;
  c.boot_timeout_ms = 30;
  SpiFullRig r(Phase::Parameter, c);
  r.state.chip.chip_id = 0x1234U;
  CHECK_EQ(r.drv.EnsureInitialized().error, Error::DeviceIdMismatch);
}

TEST_CASE("A slow UART reply timeout still leaves the first poll after T_WAKE") {
  Config c = ConfigFor(Mode::Parameter);
  c.hardware_reset = true;
  c.reply_timeout_us = 400'000; // one failed poll is charged more than the whole budget
  UartFullRig r(Phase::Parameter, c);
  r.state.wake_us = 44'000; // T_WAKE, typical
  CHECK(r.Start());
  CHECK(r.drv.IsInitialized());
}

// --- m5: bootloader mode without a DRV_EN hook recovers from an unseen restart ------------------

TEST_CASE("No hook, bootloader adopted, unseen restart into an application: stop, then recover") {
  SpiRig r(Phase::Bootloader, BlCfg());
  r.state.chip.SetCfgWord(2U, 2U); // the power-up configuration names parameter mode, no start
  REQUIRE(r.Start());              // adoption reads it before anything is written
  r.state.chip.BootInto(Phase::Parameter); // the restart the OTP decides
  r.state.chip.SetParamValue(4U, 3);
  CHECK(!r.drv.DisableDrive().ok()); // the bootloader did not answer: an error, as before
  CHECK(!r.drv.IsInitialized());
  CHECK_EQ(r.state.chip.ParamValue(4U), 0); // but the parameter stop was sent all the same
  CHECK(r.drv.GetMode() == Mode::Parameter);
  CHECK(r.drv.DisableDrive().ok()); // and the next stop speaks to the application
}

TEST_CASE("No hook, bootloader adopted: the last BOOT_MODE read picks the stop (register mode)") {
  UartRig r(Phase::Bootloader, BlCfg());
  r.state.chip.SetCfgWord(2U, 1U); // the power-up configuration names register mode, no start
  REQUIRE(r.Start());
  r.state.chip.BootInto(Phase::Register);
  r.state.chip.SetRegValue(regs::MCC_GDRV_HW, 0x0300'080FU);
  r.state.chip.SetRegValue(regs::MCC_PWM_CONFIG, 0x0000'0F07U);
  CHECK(!r.drv.DisableDrive().ok());
  CHECK(!r.drv.IsInitialized());
  CHECK(r.drv.GetMode() == Mode::Register);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW) & 0x0300'080FU, 0U);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_PWM_CONFIG) & 0x0000'0F07U, 0U);
}

TEST_CASE(
    "No hook, nothing read: Config::mode Bootloader sends no stop but leaves the driver uninit") {
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  r.state.chip.BootInto(Phase::Parameter);
  CHECK(!r.drv.DisableDrive().ok());
  CHECK(!r.drv.IsInitialized());
  CHECK_EQ(r.drv.DisableDrive().error, Error::NotInitialized); // no protocol known: not Ok
}

TEST_CASE("No hook, bootloader adopted, its own reset seen: it stays the bootloader") {
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  BootConfig next = r.drv.BlReadConfig().value;
  REQUIRE(Error::Ok == next.Set(BootField::BootMode, 2U)); // an application is named, not started
  REQUIRE(r.drv.BlWriteConfig(next).ok());
  REQUIRE(r.drv.BlReadConfig().ok());
  r.state.chip.Reset(); // SESSION_START: the bootloader itself restarted
  CHECK_EQ(r.drv.DisableDrive().error, Error::UnexpectedReset);
  CHECK(r.drv.GetMode() == Mode::Bootloader);
}

// --- round 7: a bootloader that answers is never taken for an application ----------------------

TEST_CASE("No hook: BUSY on the stop's chip-type check is the bootloader, no application stop") {
  SpiRig r(Phase::Bootloader, BlCfg());
  r.state.chip.SetCfgWord(2U, 2U);
  REQUIRE(r.Start());
  r.state.chip.SetParamValue(4U, 3);
  r.state.chip.busy_replies = 1000;
  const int t0 = r.state.transfers;
  CHECK_EQ(r.drv.DisableDrive().error, Error::Busy);
  CHECK(!AppPolled(r, t0, r.state.transfers));
  CHECK_EQ(r.state.chip.ParamValue(4U), 3); // no application stop was sent
  CHECK(r.drv.GetMode() == Mode::Bootloader);
  CHECK(r.drv.IsInitialized());
}

TEST_CASE("No hook: an error status on the stop's chip-type check is the bootloader (SPI)") {
  SpiRig r(Phase::Bootloader, BlCfg());
  r.state.chip.SetCfgWord(2U, 2U);
  REQUIRE(r.Start());
  r.state.chip.SetParamValue(4U, 3);
  r.state.chip.refuse_chip_type = true;
  CHECK_EQ(r.drv.DisableDrive().error, Error::Nack);
  CHECK_EQ(r.state.chip.ParamValue(4U), 3);
  CHECK(r.drv.GetMode() == Mode::Bootloader);
  CHECK(r.drv.IsInitialized());
}

TEST_CASE("No hook: an error status on the stop's chip-type check is the bootloader (UART)") {
  UartRig r(Phase::Bootloader, BlCfg());
  r.state.chip.SetCfgWord(2U, 1U);
  REQUIRE(r.Start());
  r.state.chip.SetRegValue(regs::MCC_GDRV_HW, 0x0300'080FU);
  r.state.chip.refuse_chip_type = true;
  CHECK_EQ(r.drv.DisableDrive().error, Error::Nack);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW), 0x0300'080FU); // no register stop
  CHECK(r.drv.GetMode() == Mode::Bootloader);
  CHECK(r.drv.IsInitialized());
}

TEST_CASE("No hook, parameter mode left by ReturnToBootloader: the guess is Config::mode") {
  SpiRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
  REQUIRE(r.Start());
  REQUIRE(r.drv.ReturnToBootloader().ok());
  REQUIRE(r.drv.GetMode() == Mode::Bootloader);
  r.state.chip.BootInto(Phase::Parameter); // an unseen restart into the application
  r.state.chip.SetParamValue(4U, 3);
  CHECK(!r.drv.DisableDrive().ok());
  CHECK_EQ(r.state.chip.ParamValue(4U), 0); // the stop speaks the protocol of Config::mode
  CHECK(r.drv.GetMode() == Mode::Parameter);
}

TEST_CASE("No hook: a restart during the adoption read fails the start, nothing is adopted") {
  int total = 0;
  {
    SpiRig dry(Phase::Bootloader, BlCfg());
    REQUIRE(dry.Start());
    total = dry.state.transfers;
  }
  SpiRig r(Phase::Bootloader, BlCfg());
  r.state.reset_at_transfer = total - 3; // inside the configuration read
  CHECK(!r.drv.EnsureInitialized().ok());
  CHECK(!r.drv.IsInitialized());
}
