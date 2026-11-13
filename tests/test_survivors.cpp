/**
 * @file test_survivors.cpp
 * @brief Guards that no other test pinned: the bootloader SPI command that runs once and the probe
 * before it, every place that marks a UART line dirty, two stale replies, the reset-pin release,
 * the SPI reply retry, the boot-mode read-back, PowerDown after a failed stop and the register
 * baseline.
 */
#include <cstdint>

#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

namespace {

Config Cfg(Mode m) {
  return ConfigFor(m);
}

constexpr std::size_t Cmd(BlCmd c) {
  return static_cast<std::size_t>(c);
}

/// After a failed exchange the next call drains and waits before it sends.
template <typename R, typename Call>
void ExpectResyncBefore(R& r, Call call) {
  const int drains = r.state.drains;
  const uint64_t waited = r.state.delay_us;
  call();
  CHECK(r.state.drains > drains);
  CHECK((r.state.delay_us - waited) >= r.drv.GetConfig().uart_resync_us);
}

} // namespace

// --- R2-2: the bootloader SPI command runs once, after a probe ----------------------------------

TEST_CASE("Bootloader SPI: BUSY never repeats the command, the model counts executions") {
  SpiRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
  REQUIRE(r.Start());
  r.state.chip.busy_replies = 3;
  CHECK(r.drv.BlWrite32(Bank::SpiFlash, 0x100U, 7U).ok());
  CHECK_EQ(r.state.chip.bl_cmds[Cmd(BlCmd::Write32)], 1);
  CHECK_EQ(r.state.chip.FlashWord(0x100U), 7U);
}

TEST_CASE("Bootloader SPI: a command with an effect is not sent first after an unseen reset") {
  SpiRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
  REQUIRE(r.Start());
  r.state.chip.Reset();
  CHECK_EQ(r.drv.BlOtpLoad().error, Error::UnexpectedReset);
  CHECK_EQ(r.state.chip.bl_cmds[Cmd(BlCmd::OtpLoad)], 0); // the probe saw the reset first
  CHECK(!r.drv.IsInitialized());
}

// --- R2-3: every place that marks a UART line dirty ---------------------------------------------

TEST_CASE("Bootloader UART: a bad CRC or address marks the line dirty") {
  UartRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
  REQUIRE(r.Start());
  r.state.chip.corrupt_uart_reply = true;
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ChipType).error, Error::Crc);
  r.state.chip.corrupt_uart_reply = false;
  ExpectResyncBefore(r, [&] { CHECK(r.drv.BlGetInfo(BlInfo::ChipType).ok()); });
  r.state.chip.wrong_uart_address = true;
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ChipType).error, Error::BadReply);
  r.state.chip.wrong_uart_address = false;
  ExpectResyncBefore(r, [&] { CHECK(r.drv.BlGetInfo(BlInfo::ChipType).ok()); });
}

TEST_CASE("Parameter UART: a reply that echoes another command marks the line dirty") {
  UartRig r(Phase::Parameter, Cfg(Mode::Parameter));
  REQUIRE(r.Start());
  r.state.chip.wrong_op_echo = true;
  CHECK_EQ(r.drv.GetParam(Param::MAX_TORQUE).error, Error::BadReply);
  r.state.chip.wrong_op_echo = false;
  ExpectResyncBefore(r, [&] { CHECK(r.drv.GetParam(Param::MAX_TORQUE).ok()); });
}

TEST_CASE("Register UART: a bad frame or another command's reply marks the line dirty") {
  UartRig r(Phase::Register, Cfg(Mode::Register));
  REQUIRE(r.Start());
  r.state.chip.corrupt_uart_reply = true;
  CHECK_EQ(r.drv.ReadReg(regs::MCC_INFO_CHIP).error, Error::BadReply);
  r.state.chip.corrupt_uart_reply = false;
  ExpectResyncBefore(r, [&] { CHECK(r.drv.ReadReg(regs::MCC_INFO_CHIP).ok()); });
  r.state.chip.wrong_op_echo = true; // a valid frame, but the reply of another operation
  CHECK_EQ(r.drv.ReadReg(regs::MCC_INFO_CHIP).error, Error::BadReply);
  r.state.chip.wrong_op_echo = false;
  ExpectResyncBefore(r, [&] { CHECK(r.drv.ReadReg(regs::MCC_INFO_CHIP).ok()); });
}

TEST_CASE("UART: two stale replies in the line are both drained") {
  UartRig r(Phase::Parameter, Cfg(Mode::Parameter));
  REQUIRE(r.Start());
  r.state.chip.SetParamValue(static_cast<unsigned>(Param::MAX_TORQUE), 111);
  r.state.chip.SetParamValue(static_cast<unsigned>(Param::MAX_FLUX), 222);
  r.state.wire.late_next = true;
  CHECK(!r.drv.GetParam(Param::MAX_TORQUE).ok()); // the first stale reply is on its way
  const detail::TmclCommand gap{.op = detail::ToUnderlying(Tmcl::Gap),
                                .type = static_cast<uint16_t>(Param::MAX_TORQUE),
                                .motor = 0U,
                                .value = 0U};
  const auto req =
      detail::TmclUartRequest(r.drv.GetConfig().device_address, gap, TmclLayout::MotorHigh);
  r.state.wire.InjectStale(r.state.chip, req.data(), req.size()); // and a second one
  const auto v = r.drv.GetParam(Param::MAX_FLUX);
  REQUIRE(v.ok());
  CHECK_EQ(v.value, 222);
  CHECK_EQ(r.state.wire.Pending(), 0U);
}

// --- start-up and the other guards --------------------------------------------------------------

TEST_CASE("Hardware reset: a pin that cannot be driven or released fails the start") {
  for (int k = 0; k < 3; ++k) { // DRV_EN low, RSTN asserted, RSTN released
    Config c = Cfg(Mode::Parameter);
    c.hardware_reset = true;
    SpiFullRig r(Phase::Parameter, c);
    r.state.pin_fail_at = k;
    CHECK_EQ(r.drv.EnsureInitialized().error, Error::Bus);
    CHECK(!r.drv.IsInitialized());
  }
  Config c = Cfg(Mode::Parameter);
  c.hardware_reset = true;
  SpiFullRig ok(Phase::Parameter, c);
  CHECK(ok.drv.EnsureInitialized().ok());
}

TEST_CASE("SPI parameter mode: CHECKSUM_ERROR replies are retried, then Busy") {
  Config c = Cfg(Mode::Parameter);
  c.spi_reply_retries = 2U;
  SpiRig r(Phase::Parameter, c);
  REQUIRE(r.Start());
  r.state.chip.checksum_error_replies = 2;
  CHECK(r.drv.GetParam(Param::MAX_TORQUE).ok());
  r.state.chip.checksum_error_replies = 3;
  CHECK_EQ(r.drv.GetParam(Param::MAX_TORQUE).error, Error::Busy);
}

TEST_CASE("BlWriteConfig: the boot-mode word is read back after it is written") {
  SpiRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
  REQUIRE(r.Start());
  BootConfig next = r.drv.BlReadConfig().value;
  REQUIRE(Error::Ok == next.Set(BootField::BootMode, 2U)); // no start: the word is only stored
  r.state.chip.corrupt_cfg_write = true;
  r.state.chip.corrupt_cfg_word = 2U;
  CHECK_EQ(r.drv.BlWriteConfig(next).error, Error::VerifyFailed);
}

TEST_CASE("PowerDown does not go on after a stop that failed") {
  SpiFullRig r(Phase::Parameter, Cfg(Mode::Parameter));
  REQUIRE(r.Start());
  REQUIRE(r.drv.EnableDrive(3).ok());
  r.state.fail_transfer = r.state.transfers; // the stop's frame and its repeat
  r.state.fail_span = 2;
  CHECK(!r.drv.PowerDown(PowerDownTime::S1).ok());
  CHECK(r.drv.IsInitialized());
  CHECK_EQ(r.state.chip.GlobalValue(0U, global0::GO_TO_TIMEOUT_POWER_DOWN_STATE), 0);
}

TEST_CASE("Register SPI: a reset between the baseline datagram and the next one is seen") {
  SpiFullRig q(Phase::Register, Cfg(Mode::Register));
  q.state.reset_at_transfer = 1; // the baseline is transfer 0, the identity read transfer 1
  CHECK_EQ(q.drv.EnsureInitialized().error, Error::UnexpectedReset);
  CHECK(!q.drv.IsInitialized());
  SpiFullRig r(Phase::Register, Cfg(Mode::Register));
  REQUIRE(r.Start());
  r.state.chip.Reset();
  CHECK_EQ(r.drv.ReadReg(regs::MCC_INFO_CHIP).error, Error::UnexpectedReset);
  CHECK(!r.drv.IsInitialized());
}

// --- fuzz finding: a reset seen by the stop inside PowerDown / ReturnToBootloader ---------------

TEST_CASE(
    "PowerDown and ReturnToBootloader report a reset that their stop saw, not NotInitialized") {
  SpiFullRig r(Phase::Parameter, Cfg(Mode::Parameter));
  REQUIRE(r.Start());
  r.state.chip.Reset(); // unseen: the stop's first frame reports FIRST_CMD, its repeat stops
  CHECK_EQ(r.drv.PowerDown(PowerDownTime::S1).error, Error::UnexpectedReset);
  CHECK(!r.drv.IsInitialized());
  CHECK_EQ(r.state.chip.GlobalValue(0U, global0::GO_TO_TIMEOUT_POWER_DOWN_STATE), 0);
  CHECK_EQ(r.state.chip.ParamValue(4U), 0);
  SpiFullRig b(Phase::Parameter, Cfg(Mode::Parameter));
  REQUIRE(b.Start());
  b.state.chip.Reset();
  CHECK_EQ(b.drv.ReturnToBootloader().error, Error::UnexpectedReset);
  CHECK(!b.drv.IsInitialized());
  CHECK(b.state.chip.GetPhase() == Phase::Parameter); // no Boot was sent
}
