/**
 * @file test_hardening.cpp
 * @brief Reply validation, reset evidence, scripts, flash erase and the other guards that the
 * independent review found unpinned.
 */
#include <array>

#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

namespace {
Config Cfg(Mode m) {
  return ConfigFor(m);
}
} // namespace

// --- R1-6: scripts cannot set COMMUTATION_MODE --------------------------------------------------

TEST_CASE("ScriptDownload refuses SAP and AAP of COMMUTATION_MODE and sends nothing") {
  SpiRig r(Phase::Parameter, Cfg(Mode::Parameter));
  REQUIRE(r.Start());
  const int n = r.state.transfers;
  const std::array<TmclInstruction, 2> sap{{{Tmcl::Wait, 0, 0, 10}, {Tmcl::Sap, 4, 0, 5}}};
  CHECK_EQ(r.drv.ScriptDownload(sap).error, Error::Refused);
  const std::array<TmclInstruction, 1> one{{{Tmcl::Sap, 4, 0, 1}}}; // the smallest non-zero value
  CHECK_EQ(r.drv.ScriptDownload(one).error, Error::Refused);
  const std::array<TmclInstruction, 1> aap{{{Tmcl::Aap, 4, 0, 0}}};
  CHECK_EQ(r.drv.ScriptDownload(aap).error, Error::Refused);
  CHECK_EQ(r.state.transfers, n);
  CHECK_EQ(r.state.chip.DownloadCount(), 0);
}

TEST_CASE("ScriptDownload still allows a stop and other parameters") {
  SpiRig r(Phase::Parameter, Cfg(Mode::Parameter));
  REQUIRE(r.Start());
  const std::array<TmclInstruction, 3> ok{
      {{Tmcl::Sap, 4, 0, 0}, {Tmcl::Sap, 6, 0, 100}, {Tmcl::Aap, 6, 0, 0}}};
  CHECK(r.drv.ScriptDownload(ok).ok());
}

// --- R1-11: WATCHDOG_EVENT, FAULTN before register UART writes -----------------------------------

TEST_CASE("Service sees a watchdog reset through WATCHDOG_EVENT when the canary is off") {
  Config c = Cfg(Mode::Parameter);
  c.uart_canary = false;
  UartRig r(Phase::Parameter, c);
  REQUIRE(r.Start());
  CHECK(r.drv.Service().ok());
  r.state.chip.WatchdogReset();
  CHECK_EQ(r.drv.Service().error, Error::UnexpectedReset);
  CHECK(!r.drv.IsInitialized());
}

TEST_CASE("A WATCHDOG_EVENT that was already set at the start is not a new reset") {
  Config c = Cfg(Mode::Parameter);
  c.uart_canary = false;
  UartRig r(Phase::Parameter, c);
  r.state.chip.SetParamValue(299U, 0x200);
  REQUIRE(r.Start());
  CHECK(r.drv.Service().ok());
  CHECK(r.drv.IsInitialized());
}

TEST_CASE("Register UART writes consult FAULTN when asked to; the stop does not") {
  Config c = Cfg(Mode::Register);
  c.faultn_is_reset = true;
  UartFullRig r(Phase::Register, c);
  REQUIRE(r.Start());
  r.state.faultn_high = false;
  const int n = r.state.transfers;
  CHECK_EQ(r.drv.WriteReg(regs::MCC_GDRV_CFG, 1U).error, Error::UnexpectedReset);
  CHECK_EQ(r.state.transfers, n);
  CHECK(!r.drv.IsInitialized());
  r.state.chip.SetRegValue(regs::MCC_PWM_CONFIG, 0x0F07U);
  CHECK(r.drv.DisableDrive().ok()); // the stop is sent although FAULTN is low
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_PWM_CONFIG), 0U);
  UartFullRig q(Phase::Register, c);
  REQUIRE(q.Start());
  CHECK(q.drv.WriteReg(regs::MCC_GDRV_CFG, 1U).ok()); // FAULTN high
}

// --- R1-11: the bank is selected again before every UART bootloader word -------------------------

TEST_CASE("UART BlWriteConfig: a restart between words cannot send a word to another bank") {
  const auto next_config = [](auto& r) -> BootConfig {
    BootConfig next = r.drv.BlReadConfig().value;
    CHECK(Error::Ok == next.SetWord(3U, 0xF0U));
    CHECK(Error::Ok == next.SetWord(4U, 0x0FU));
    return next;
  };
  // A reference run records the commands; the transfers that begin a word write (SetBank, then
  // SetAddress, then Write32) are the ones before which the restart is injected.
  std::array<uint8_t, 512> ref{};
  int total = 0;
  {
    UartRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
    REQUIRE(r.Start());
    const BootConfig next = next_config(r);
    REQUIRE(r.drv.BlWriteConfig(next).ok());
    ref = r.state.op_log;
    total = r.state.transfers;
  }
  const auto set_bank = static_cast<uint8_t>(BlCmd::SetBank);
  const auto set_addr = static_cast<uint8_t>(BlCmd::SetAddress);
  const auto write32 = static_cast<uint8_t>(BlCmd::Write32);
  int word_starts = 0;
  for (int k = 0; (k + 2) < total; ++k) {
    const auto at = [&](int i) { return ref[static_cast<std::size_t>(i)]; };
    if ((at(k) != set_bank) || (at(k + 1) != set_addr) || (at(k + 2) != write32)) {
      continue;
    }
    ++word_starts;
    UartRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
    r.state.chip.reset_bank = 1U; // after a restart the chip is on the SPI flash bank
    REQUIRE(r.Start());
    const BootConfig next = next_config(r);
    r.state.reset_at_transfer = k;
    static_cast<void>(r.drv.BlWriteConfig(next));
    for (uint32_t a = 0U; a < 0x1000U; a += 4U) {
      CHECK_EQ(r.state.chip.FlashWord(a), 0U); // no configuration word reached the flash
    }
  }
  CHECK(word_starts >= 2);
}

// --- R1-10: reply validation ---------------------------------------------------------------------

TEST_CASE("A reply that echoes another command is BadReply, on SPI and UART") {
  SpiRig s(Phase::Parameter, Cfg(Mode::Parameter));
  REQUIRE(s.Start());
  s.state.chip.wrong_op_echo = true;
  CHECK_EQ(s.drv.GetParam(Param::MAX_TORQUE).error, Error::BadReply);
  UartRig u(Phase::Parameter, Cfg(Mode::Parameter));
  REQUIRE(u.Start());
  u.state.chip.wrong_op_echo = true;
  CHECK_EQ(u.drv.GetParam(Param::MAX_TORQUE).error, Error::BadReply);
}

TEST_CASE("A UART reply from another host or another address is rejected") {
  UartRig u(Phase::Parameter, Cfg(Mode::Parameter));
  REQUIRE(u.Start());
  u.state.chip.wrong_uart_host = true;
  CHECK_EQ(u.drv.GetParam(Param::MAX_TORQUE).error, Error::BadReply);
  u.state.chip.wrong_uart_host = false;
  u.state.chip.wrong_uart_address = true;
  CHECK_EQ(u.drv.GetParam(Param::MAX_TORQUE).error, Error::BadReply);
  UartRig b(Phase::Bootloader, Cfg(Mode::Bootloader));
  REQUIRE(b.Start());
  b.state.chip.wrong_uart_host = true;
  CHECK(!b.drv.BlGetInfo(BlInfo::ChipType).ok());
  b.state.chip.wrong_uart_host = false;
  b.state.chip.wrong_uart_address = true;
  CHECK(!b.drv.BlGetInfo(BlInfo::ChipType).ok());
  UartRig g(Phase::Register, Cfg(Mode::Register));
  REQUIRE(g.Start());
  g.state.chip.wrong_uart_host = true;
  CHECK_EQ(g.drv.ReadReg(regs::MCC_INFO_CHIP).error, Error::BadReply);
}

TEST_CASE("Register SPI: a jump of the datagram counter is seen on every datagram of a call") {
  for (int k = 0; k < 2; ++k) { // the request datagram, then the datagram that fetches the reply
    SpiRig r(Phase::Register, Cfg(Mode::Register));
    REQUIRE(r.Start());
    r.state.chip.skip_reg_counter_after = r.state.chip.RegDatagrams() + k;
    CHECK_EQ(r.drv.ReadReg(regs::MCC_INFO_CHIP).error, Error::UnexpectedReset);
    CHECK(!r.drv.IsInitialized());
  }
  for (int k = 0; k < 3; ++k) { // a write: probe, request, fetch
    SpiRig r(Phase::Register, Cfg(Mode::Register));
    REQUIRE(r.Start());
    r.state.chip.skip_reg_counter_after = r.state.chip.RegDatagrams() + k;
    CHECK_EQ(r.drv.WriteReg(regs::MCC_GDRV_CFG, 1U).error, Error::UnexpectedReset);
  }
}

TEST_CASE("Register SPI: a failed request status is Nack") {
  SpiRig r(Phase::Register, Cfg(Mode::Register));
  REQUIRE(r.Start());
  r.state.chip.reg_status_fail = true;
  CHECK_EQ(r.drv.ReadReg(regs::MCC_INFO_CHIP).error, Error::Nack);
}

TEST_CASE("UART canary: a failed canary read is its own error, not a reset") {
  UartRig r(Phase::Parameter, Cfg(Mode::Parameter));
  REQUIRE(r.Start());
  r.state.fail_transfer = r.state.transfers; // the canary read that precedes the write
  CHECK_EQ(r.drv.SetParam(Param::MAX_TORQUE, 5).error, Error::Bus);
  CHECK(r.drv.IsInitialized());
  r.state.fail_transfer = -1;
  CHECK(r.drv.SetParam(Param::MAX_TORQUE, 5).ok());
}

TEST_CASE(
    "ReturnToBootloader: a bootloader that does not identify itself leaves the driver stopped") {
  SpiRig r(Phase::Parameter, Cfg(Mode::Parameter));
  REQUIRE(r.Start());
  r.state.chip.chip_id = 0x1234U;
  CHECK(!r.drv.ReturnToBootloader().ok());
  CHECK(!r.drv.IsInitialized());
  const int n = r.state.transfers;
  CHECK_EQ(r.drv.MotorStop().error, Error::NotInitialized);
  CHECK_EQ(r.state.transfers, n);
}

TEST_CASE("BlStartApplication: an application that does not identify itself is not adopted") {
  SpiRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
  REQUIRE(r.Start());
  r.state.chip.chip_id = 0x1234U;
  CHECK(!r.drv.BlStartApplication(BootMode::Parameter).ok());
  CHECK(!r.drv.IsInitialized());
  CHECK_EQ(r.drv.GetChipId().error, Error::NotInitialized);
}

// --- bootloader SPI: reset before a write, BUSY, dead line ---------------------------------------

TEST_CASE("Bootloader SPI: a write after an unseen reset is not executed") {
  SpiRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
  REQUIRE(r.Start());
  r.state.chip.Reset();
  CHECK_EQ(r.drv.BlWrite32(Bank::SpiFlash, 0x100U, 7U).error, Error::UnexpectedReset);
  CHECK_EQ(r.state.chip.writes_since_reset, 0);
  CHECK(!r.drv.IsInitialized());
}

TEST_CASE("Bootloader SPI: BUSY repeats the fetch, never the command") {
  SpiRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
  REQUIRE(r.Start());
  r.state.chip.busy_replies = 2;
  CHECK(r.drv.BlWrite32(Bank::SpiFlash, 0x100U, 7U).ok());
  CHECK_EQ(r.state.chip.writes_since_reset, 1);
  CHECK_EQ(r.state.chip.FlashWord(0x100U), 7U);
}

TEST_CASE("Bootloader SPI: exactly spi_reply_retries + 1 fetches are made, then Busy") {
  Config c = Cfg(Mode::Bootloader);
  c.spi_reply_retries = 2U;
  {
    SpiRig r(Phase::Bootloader, c);
    REQUIRE(r.Start());
    r.state.chip.busy_replies = 2; // the third fetch (the last one allowed) answers
    CHECK(r.drv.BlGetInfo(BlInfo::BlVersion).ok());
  }
  {
    SpiRig r(Phase::Bootloader, c);
    REQUIRE(r.Start());
    r.state.chip.busy_replies = 3; // one more than the fetches allowed
    CHECK_EQ(r.drv.BlGetInfo(BlInfo::BlVersion).error, Error::Busy);
  }
}

TEST_CASE("Bootloader SPI: a dead MISO line (all zero) is not a read") {
  {
    SpiRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
    REQUIRE(r.Start());
    r.state.chip.dead_miso = true;
    CHECK_EQ(r.drv.BlRead32(Bank::SpiFlash, 0U).error, Error::BadReply);
    CHECK_EQ(r.drv.BlGetInfo(BlInfo::BlVersion).error, Error::BadReply);
    CHECK_EQ(r.drv.BlMemoryStatus(Bank::SpiFlash, MemQuery::Busy).error, Error::BadReply);
    CHECK_EQ(r.drv.BlReadConfig().error, Error::BadReply);
  }
  {
    SpiRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
    REQUIRE(r.Start());
    CHECK_EQ(r.drv.BlRead32(Bank::SpiFlash, 0x40U).value, 0U); // a real zero is believed
    CHECK(r.drv.BlRead32(Bank::SpiFlash, 0x40U).ok());
    CHECK(r.drv.BlMemoryStatus(Bank::SpiFlash, MemQuery::Busy).ok());
    CHECK(r.drv.BlReadConfig().ok());
  }
}

// --- flash erase address checks ------------------------------------------------------------------

TEST_CASE("BlEraseFlashSector validates the address before anything is sent") {
  SpiRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
  REQUIRE(r.Start());
  r.state.chip.SetCfgWord(2U, r.state.chip.CfgWord(2U) | (1U << 16)); // flash configured
  const int n = r.state.transfers;
  CHECK_EQ(r.drv.BlEraseFlashSector(0x1004U).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.BlEraseFlashSector(0x1800U).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.BlEraseFlashSector(0x0100'0000U).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.BlEraseFlashSector(0x0000U).error, Error::Refused);
  CHECK_EQ(r.drv.BlEraseFlashSector(0x0800U).error, Error::InvalidArgument);
  CHECK_EQ(r.state.transfers, n);
  CHECK_EQ(r.state.chip.erases, 0);
  CHECK(r.drv.BlEraseFlashSector(0x1000U).ok());
  CHECK_EQ(r.state.chip.erases, 1);
  CHECK(r.drv.BlEraseFlashSector(0x00FF'F000U).ok());
  CHECK(r.drv.BlEraseFlashSector(0U, true).ok());
  CHECK_EQ(r.state.chip.erases, 3);
}
