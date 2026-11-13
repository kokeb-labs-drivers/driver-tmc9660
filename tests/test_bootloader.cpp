/**
 * @file test_bootloader.cpp
 * @brief Bootloader mode over SPI and UART: info, configuration writes and their guards, start,
 * flash.
 */
#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

namespace {

Config BlCfg() {
  return ConfigFor(Mode::Bootloader);
}

template <typename Rig>
void InfoAndMemory(Rig& r) {
  REQUIRE(r.Start());
  CHECK_EQ(r.drv.GetMode(), Mode::Bootloader);
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ChipType).value, kChipId);
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ChipFrequency).value, 40'000'000U);
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ConfigMemSize).value, 64U);
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ConfigMemStart).value, kConfigMemAddress);
  CHECK_EQ(r.drv.BlGetInfo(ut::RawEnum<BlInfo>(99)).error,
           Error::Nack); // the chip refuses an unknown selector
  CHECK(r.drv.BlRead32(Bank::Config, kConfigMemAddress).ok());
  CHECK_EQ(r.drv.BlRead32(Bank::Config, kConfigMemAddress + 1U).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.BlRead32(ut::RawEnum<Bank>(7), 0U).error, Error::InvalidArgument);
  r.state.chip.SetFlashWord(0x40U, 0xCAFE0001U);
  CHECK_EQ(r.drv.BlRead32(Bank::SpiFlash, 0x40U).value, 0xCAFE0001U);
  CHECK(r.drv.BlWrite32(Bank::SpiFlash, 0x44U, 0x12345678U).ok());
  CHECK_EQ(r.state.chip.FlashWord(0x44U), 0x12345678U);
  CHECK_EQ(r.drv.BlWrite32(Bank::SpiFlash, 0x46U, 1U).error, Error::InvalidArgument);
}

} // namespace

TEST_CASE("Bootloader info, reads and flash writes over SPI") {
  SpiRig r(Phase::Bootloader, BlCfg());
  InfoAndMemory(r);
}

TEST_CASE("Bootloader info, reads and flash writes over UART") {
  UartRig r(Phase::Bootloader, BlCfg());
  InfoAndMemory(r);
}

TEST_CASE("OTP and the configuration bank are never written through BlWrite32") {
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  const int n = r.state.transfers;
  CHECK_EQ(r.drv.BlWrite32(Bank::Otp, 0U, 1U).error, Error::Refused);
  CHECK_EQ(r.drv.BlWrite32(Bank::Config, kConfigMemAddress, 1U).error, Error::Refused);
  CHECK_EQ(r.state.transfers, n);
  CHECK_EQ(r.state.chip.otp_writes, 0);
  CHECK_EQ(r.state.chip.config_word_writes, 0);
}

TEST_CASE("Calls of another mode return WrongMode and send nothing") {
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  const int n = r.state.transfers;
  CHECK_EQ(r.drv.GetParam(Param::MAX_TORQUE).error, Error::WrongMode);
  CHECK_EQ(r.drv.SetParam(Param::MAX_TORQUE, 1).error, Error::WrongMode);
  CHECK_EQ(r.drv.EnableDrive(3).error, Error::WrongMode);
  CHECK_EQ(r.drv.ReadReg(regs::MCC_INFO_CHIP).error, Error::WrongMode);
  CHECK_EQ(r.drv.GetChipId().error, Error::WrongMode);
  CHECK_EQ(r.drv.ReturnToBootloader().error, Error::WrongMode);
  CHECK_EQ(r.state.transfers, n);
  SpiRig p(Phase::Parameter, ConfigFor(Mode::Parameter));
  REQUIRE(p.Start());
  const int m = p.state.transfers;
  CHECK_EQ(p.drv.BlGetInfo(BlInfo::ChipType).error, Error::WrongMode);
  CHECK_EQ(p.drv.BlWriteConfig(BootConfig{}).error, Error::WrongMode);
  CHECK_EQ(p.drv.BlOtpLoad().error, Error::WrongMode);
  CHECK_EQ(p.state.transfers, m);
}

TEST_CASE("BlWriteConfig writes only the words that differ") {
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  const auto cur = r.drv.BlReadConfig();
  REQUIRE(cur.ok());
  CHECK(r.drv.BlWriteConfig(cur.value).ok()); // nothing to do
  CHECK_EQ(r.state.chip.config_word_writes, 0);
  BootConfig next = cur.value;
  REQUIRE(Error::Ok == next.Set(BootField::DeviceAddress, 5U));
  CHECK(r.drv.BlWriteConfig(next).ok());
  CHECK_EQ(r.state.chip.config_word_writes, 1);
  CHECK(r.drv.BlReadConfig().value == next);
  CHECK_EQ(r.state.chip.reserved_start_events, 0);
}

TEST_CASE("BlWriteConfig follows an address change on the UART") {
  UartRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  BootConfig next = r.drv.BlReadConfig().value;
  REQUIRE(Error::Ok == next.Set(BootField::DeviceAddress, 7U));
  CHECK(r.drv.BlWriteConfig(next).ok());
  CHECK_EQ(r.drv.GetConfig().device_address, 7U);
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ChipType).value, kChipId); // the chip answers at the new address
}

TEST_CASE("BlWriteConfig: a read-back mismatch stops before the boot-mode word") {
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  BootConfig next = r.drv.BlReadConfig().value;
  REQUIRE(Error::Ok == next.Set(BootField::BootMode, 2U));
  REQUIRE(Error::Ok == next.SetWord(3U, 0x0000'00F0U));
  r.state.chip.corrupt_cfg_write = true;
  const uint32_t boot_word = r.state.chip.CfgWord(2U);
  CHECK_EQ(r.drv.BlWriteConfig(next).error, Error::VerifyFailed);
  CHECK_EQ(r.state.chip.CfgWord(2U),
           boot_word); // the boot-mode word is written last, after the verify
  CHECK(r.state.chip.GetPhase() == Phase::Bootloader);
}

TEST_CASE("BlWriteConfig refuses an unsafe start and writes nothing") {
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  BootConfig base = r.drv.BlReadConfig().value;
  BootConfig reserved = base; // start with the reserved BOOT_MODE
  REQUIRE(Error::Ok == reserved.Set(BootField::StartMotorCtrl, 1U));
  CHECK_EQ(r.drv.BlWriteConfig(reserved).error, Error::Refused);
  BootConfig noclock = base; // start without the PLL
  REQUIRE(Error::Ok == noclock.Set(BootField::BootMode, 2U));
  REQUIRE(Error::Ok == noclock.Set(BootField::StartMotorCtrl, 1U));
  REQUIRE(Error::Ok == noclock.Set(BootField::ClockPllOutSel, 0U));
  CHECK_EQ(r.drv.BlWriteConfig(noclock).error, Error::Refused);
  CHECK_EQ(r.drv.BlStartApplication(BootMode::Reserved).error, Error::Refused);
  CHECK_EQ(r.drv.BlStartApplication(BootMode::Reserved3).error, Error::Refused);
  CHECK_EQ(r.state.chip.config_word_writes, 0);
  CHECK_EQ(r.state.chip.reserved_start_events, 0);
  CHECK(r.state.chip.GetPhase() == Phase::Bootloader);
}

TEST_CASE("BlWriteConfig: LDO changes need allow_ldo_change") {
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  BootConfig next = r.drv.BlReadConfig().value;
  REQUIRE(Error::Ok == next.Set(BootField::LdoVext1, 2U));
  CHECK_EQ(r.drv.BlWriteConfig(next).error, Error::Refused);
  CHECK_EQ(r.state.chip.ldo_writes, 0);
  CHECK_EQ(r.state.chip.config_word_writes, 0);
  BlWriteOptions allow;
  allow.allow_ldo_change = true;
  CHECK(r.drv.BlWriteConfig(next, allow).ok());
  CHECK_EQ(r.state.chip.ldo_writes, 1);
}

TEST_CASE("BlWriteConfig: the PLL must lock after a clock change") {
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  BootConfig next = r.drv.BlReadConfig().value;
  REQUIRE(Error::Ok == next.Set(BootField::ClockRdiv, 13U));
  r.state.chip.pll_never_locks = true;
  CHECK_EQ(r.drv.BlWriteConfig(next).error, Error::Timeout);
  SpiRig ok(Phase::Bootloader, BlCfg());
  REQUIRE(ok.Start());
  CHECK(ok.drv.BlWriteConfig(next).ok());
  CHECK(ok.state.delay_us > 0U);
}

TEST_CASE("BlStartApplication hands over to each application mode") {
  {
    SpiRig r(Phase::Bootloader, BlCfg());
    REQUIRE(r.Start());
    CHECK(r.drv.BlStartApplication(BootMode::Parameter).ok());
    CHECK_EQ(r.drv.GetMode(), Mode::Parameter);
    CHECK(r.state.chip.GetPhase() == Phase::Parameter);
    CHECK_EQ(r.drv.GetChipId().value, kChipId);
    CHECK_EQ(r.state.chip.reserved_start_events, 0);
  }
  {
    UartRig r(Phase::Bootloader, BlCfg());
    REQUIRE(r.Start());
    CHECK(r.drv.BlStartApplication(BootMode::Register).ok());
    CHECK_EQ(r.drv.GetMode(), Mode::Register);
    CHECK_EQ(r.drv.ReadReg(regs::MCC_INFO_CHIP).value, kChipId);
  }
  {
    UartRig r(Phase::Bootloader, BlCfg());
    REQUIRE(r.Start());
    CHECK(r.drv.BlStartApplication(BootMode::Parameter).ok());
    CHECK(r.drv.SetParam(Param::MAX_TORQUE, 5).ok()); // the canary was armed by the start
    r.state.chip.BootInto(Phase::Parameter); // a reset that comes back up in parameter mode
    CHECK_EQ(r.drv.SetParam(Param::MAX_TORQUE, 6).error, Error::UnexpectedReset);
    CHECK_EQ(r.state.chip.writes_since_reset, 0);
  }
}

TEST_CASE("Flash erase needs a configured, connected, idle flash") {
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  r.state.chip.SetCfgWord(2U, r.state.chip.CfgWord(2U) | (1U << 16)); // flash configured
  r.state.chip.flash_present = false;
  CHECK_EQ(r.drv.BlEraseFlashSector(0x1000U).error, Error::Refused);
  CHECK_EQ(r.state.chip.erases, 0);
  r.state.chip.flash_present = true;
  r.state.chip.flash_busy = true;
  CHECK_EQ(r.drv.BlEraseFlashSector(0x1000U).error, Error::Busy);
  CHECK_EQ(r.state.chip.erases, 0);
  r.state.chip.flash_busy = false;
  CHECK(r.drv.BlEraseFlashSector(0x1000U).ok());
  CHECK_EQ(r.state.chip.erases, 1);
  CHECK_EQ(r.drv.BlMemoryStatus(Bank::Otp, MemQuery::Busy).error, Error::InvalidArgument);
  CHECK(r.drv.BlMemoryStatus(Bank::SpiFlash, MemQuery::Connected).value);
  CHECK(r.drv.BlOtpLoad().ok());
  CHECK_EQ(r.state.chip.otp_writes, 0);
}

TEST_CASE("Bootloader SPI: BUSY retries only the fetch, a session start means a reset") {
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  r.state.chip.busy_replies = 3;
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ChipType).value, kChipId);
  r.state.chip.busy_replies = 100000;
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ChipType).error, Error::Busy);
  r.state.chip.busy_replies = 0;
  const int w = r.state.chip.writes_since_reset;
  r.state.chip.Reset(); // the bootloader restarts
  CHECK_EQ(r.drv.BlWrite32(Bank::SpiFlash, 0x10U, 1U).error, Error::UnexpectedReset);
  CHECK_EQ(r.state.chip.writes_since_reset, 0);
  CHECK(w >= 0);
  CHECK(!r.drv.IsInitialized());
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ChipType).error, Error::NotInitialized);
  CHECK(r.Start());
  CHECK(r.drv.BlWrite32(Bank::SpiFlash, 0x10U, 1U).ok());
}

TEST_CASE("Bootloader UART: CRC error, timeout, bus fault, wrong address") {
  UartRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  r.state.chip.corrupt_uart_reply = true;
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ChipType).error, Error::Crc);
  r.state.chip.corrupt_uart_reply = false;
  r.state.chip.drop_uart_reply = true;
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ChipType).error, Error::Timeout);
  r.state.chip.drop_uart_reply = false;
  r.state.uart_fault = BusStatus::Fault;
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ChipType).error, Error::Bus);
  r.state.uart_fault = BusStatus::Ok;
  CHECK(r.drv.BlGetInfo(BlInfo::ChipType).ok());
  // knob A1: the output reflection reading
  Config c = BlCfg();
  c.quirks.crc8_reflect_output = true;
  UartRig k(Phase::Bootloader, c);
  k.state.chip.knobs.crc_reflect_out = true;
  CHECK(k.Start());
  UartRig bad(Phase::Bootloader, c); // the chip uses the other reading
  CHECK(!bad.drv.EnsureInitialized().ok());
}

TEST_CASE("Identity mismatch in the bootloader") {
  SpiRig r(Phase::Bootloader, BlCfg());
  r.state.chip.chip_id = 0x12345678U;
  CHECK_EQ(r.drv.EnsureInitialized().error, Error::DeviceIdMismatch);
  CHECK(!r.drv.IsInitialized());
}

TEST_CASE("BlWriteConfig does not start the application while the PLL reads as unlocked") {
  SpiRig r(Phase::Bootloader, BlCfg());
  REQUIRE(r.Start());
  BootConfig next = r.drv.BlReadConfig().value;
  REQUIRE(Error::Ok == next.Set(BootField::BootMode, 2U));
  REQUIRE(Error::Ok == next.Set(BootField::StartMotorCtrl, 1U));
  r.state.chip.pll_status_low = true;
  CHECK_EQ(r.drv.BlWriteConfig(next).error, Error::Refused);
  CHECK(r.state.chip.GetPhase() == Phase::Bootloader);
}
