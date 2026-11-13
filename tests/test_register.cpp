/**
 * @file test_register.cpp
 * @brief Register mode over SPI and UART: access, validation, energising guard, gate-driver start,
 * resets.
 */
#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

namespace {

Config RegCfg() {
  return ConfigFor(Mode::Register);
}

template <typename Rig>
void ExerciseAccess(Rig& r) {
  REQUIRE(r.Start());
  CHECK_EQ(r.drv.GetMode(), Mode::Register);
  CHECK_EQ(r.drv.ReadReg(regs::MCC_INFO_CHIP).value, kChipId);
  CHECK(r.drv.WriteReg(regs::MCC_GDRV_CFG, 0x0003'0001U).ok());
  CHECK_EQ(r.drv.ReadReg(regs::MCC_GDRV_CFG).value, 0x0003'0001U);
  CHECK(r.drv.UpdateReg(regs::MCC_GDRV_CFG, 0x0000'00F0U, 0x0000'0030U).ok());
  CHECK_EQ(r.drv.ReadReg(regs::MCC_GDRV_CFG).value, 0x0003'0031U);
  const int n = r.state.transfers;
  CHECK_EQ(r.drv.WriteReg(regs::MCC_INFO_CHIP, 1U).error, Error::InvalidArgument); // read only
  CHECK_EQ(r.drv.ReadReg(RegAddr{Block::Mcc, 0x7FFU}).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.WriteReg(RegAddr{Block::Mcc, 0x7FFU}, 1U).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.UpdateReg(regs::MCC_GDRV_CFG, 0U, 0U).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.UpdateReg(regs::MCC_GDRV_CFG, 0x0000'0010U, 0x0000'0020U).error,
           Error::InvalidArgument);
  CHECK_EQ(r.drv.UpdateReg(regs::MCC_GDRV_CFG, 0xFF00'0000U, 0U).error, Error::InvalidArgument);
  CHECK_EQ(r.state.transfers, n);
}

} // namespace

TEST_CASE("Register access over SPI") {
  SpiFullRig r(Phase::Register, RegCfg());
  ExerciseAccess(r);
}

TEST_CASE("Register access over UART") {
  UartFullRig r(Phase::Register, RegCfg());
  ExerciseAccess(r);
}

TEST_CASE(
    "Every register of the table can be read: UART and the wide SPI layout reach all of them") {
  UartFullRig u(Phase::Register, RegCfg());
  REQUIRE(u.Start());
  Config wide = RegCfg();
  wide.quirks.reg_spi_wide_offset = true;
  SpiFullRig w(Phase::Register, wide);
  w.state.chip.knobs.reg_wide = true;
  REQUIRE(w.Start());
  SpiFullRig t(Phase::Register, RegCfg());
  REQUIRE(t.Start());
  int n = 0;
  int beyond = 0;
  for (const RegInfo& info : kRegisters) {
    CHECK(u.drv.ReadReg(info.addr).ok());
    CHECK(w.drv.ReadReg(info.addr).ok());
    if (info.addr.offset > 0x1FFU) {
      ++beyond;
      const int before = t.state.transfers;
      CHECK_EQ(t.drv.ReadReg(info.addr).error,
               Error::Unsupported); // the table layout has a 9-bit offset (A3)
      CHECK_EQ(t.state.transfers, before);
    } else {
      CHECK(t.drv.ReadReg(info.addr).ok());
    }
    ++n;
  }
  CHECK_EQ(n, 163);
  CHECK(beyond > 0);
}

TEST_CASE("Writes that energise the gate driver are refused outside GateDriverStart") {
  SpiFullRig r(Phase::Register, RegCfg());
  REQUIRE(r.Start());
  CHECK_EQ(r.drv.WriteReg(regs::MCC_GDRV_HW, 0x0000'0001U).error, Error::Refused);
  CHECK_EQ(r.drv.WriteReg(regs::MCC_GDRV_HW, 0x0100'0000U).error, Error::Refused);
  CHECK_EQ(r.drv.WriteReg(regs::MCC_PWM_CONFIG, 0x0000'0007U).error, Error::Refused);
  CHECK_EQ(r.drv.UpdateReg(regs::MCC_GDRV_HW, 0x0000'0001U, 0x0000'0001U).error, Error::Refused);
  CHECK_EQ(r.state.chip.energise_events, 0);
  CHECK(r.drv.WriteReg(regs::MCC_GDRV_HW, 0U).ok());
  CHECK(r.drv.WriteReg(regs::MCC_PWM_CONFIG, 0U).ok());
  CHECK_EQ(r.state.chip.energise_events, 0);
}

TEST_CASE("GateDriverStart: guards complete before the energising write") {
  SpiFullRig r(Phase::Register, RegCfg());
  REQUIRE(r.Start());
  const GateDriverRegConfig cfg{};
  CHECK(r.drv.GateDriverStart(cfg).ok());
  CHECK_EQ(r.state.chip.energise_events, 1);
  CHECK_EQ(r.state.chip.energise_with_flag, 0);
  CHECK_EQ(r.state.chip.energise_without_drv_en, 0);
  CHECK(r.state.chip.DrvEn());
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW), cfg.gdrv_hw);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_PWM_CONFIG), cfg.pwm_enable | cfg.chop_run);
  CHECK(r.drv.DisableDrive().ok());
  CHECK(!r.state.chip.DrvEn());
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW), 0U);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_PWM_CONFIG), 0U);
  CHECK(r.state.chip.disable_cmds >= 2);
}

TEST_CASE("GateDriverStart: a latched fault or undervoltage forbids the start") {
  const uint32_t faults[] = {0x0000'0001U, 0x0001'0000U, 0x0000'0008U, 0x8000'0000U, 0x2000'0000U};
  for (const uint32_t bits : faults) {
    SpiFullRig r(Phase::Register, RegCfg());
    REQUIRE(r.Start());
    r.state.chip.gdrv_fault_bits = bits;
    const auto e = r.drv.GateDriverStart(GateDriverRegConfig{});
    CHECK(!e.ok());
    CHECK_EQ(r.state.chip.energise_events, 0);
    CHECK(!r.state.chip.DrvEn());
    CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW), 0U);
    CHECK_EQ(r.state.chip.RegValue(regs::MCC_PWM_CONFIG) & 7U, 0U);
  }
  SpiFullRig f(Phase::Register, RegCfg());
  REQUIRE(f.Start());
  f.state.chip.SetRegValue(regs::MCC_GDRV_FAULT, 0x0001'0000U);
  CHECK_EQ(f.drv.GateDriverStart(GateDriverRegConfig{}).error, Error::DeviceFault);
  CHECK_EQ(f.state.chip.energise_events, 0);
}

TEST_CASE("GateDriverStart: invalid configuration sends nothing, a bus failure never leaves the "
          "outputs on") {
  SpiFullRig r(Phase::Register, RegCfg());
  REQUIRE(r.Start());
  const int n = r.state.transfers;
  GateDriverRegConfig bad;
  bad.chop_run = 0;
  CHECK_EQ(r.drv.GateDriverStart(bad).error, Error::InvalidArgument);
  bad.chop_run = 8;
  CHECK_EQ(r.drv.GateDriverStart(bad).error, Error::InvalidArgument);
  bad = GateDriverRegConfig{};
  bad.pwm_enable = 0x0000'0001U;
  CHECK_EQ(r.drv.GateDriverStart(bad).error, Error::InvalidArgument);
  bad = GateDriverRegConfig{};
  bad.gdrv_hw = 0xFFFF'FFFFU;
  CHECK_EQ(r.drv.GateDriverStart(bad).error, Error::InvalidArgument);
  CHECK_EQ(r.state.transfers, n);
  for (int k = 0; k < 40; ++k) {
    SpiFullRig f(Phase::Register, RegCfg());
    REQUIRE(f.Start());
    f.state.fail_transfer = f.state.transfers + k;
    const auto e = f.drv.GateDriverStart(GateDriverRegConfig{});
    if (!e.ok()) {
      CHECK(!f.state.chip.DrvEn());
    }
    CHECK_EQ(f.state.chip.energise_with_flag, 0);
    CHECK_EQ(f.state.chip.energise_without_drv_en, 0);
  }
  SpiRig nopin(Phase::Register,
               RegCfg()); // no GpioSet hook: DRV_EN is strapped high, the guards still run
  REQUIRE(nopin.Start());
  nopin.state.chip.gdrv_fault_bits = 0x1U;
  CHECK(!nopin.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  CHECK_EQ(nopin.state.chip.energise_events, 0);
  nopin.state.chip.gdrv_fault_bits = 0U;
  CHECK(nopin.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  CHECK_EQ(nopin.state.chip.energise_with_flag, 0);
}

TEST_CASE("Register mode over SPI: the boot bit and the datagram counter reveal a reset") {
  SpiFullRig r(Phase::Register, RegCfg());
  REQUIRE(r.Start());
  REQUIRE(r.drv.WriteReg(regs::MCC_GDRV_CFG, 0x0003'0000U).ok());
  r.state.chip.BootInto(Phase::Register);
  CHECK_EQ(r.drv.WriteReg(regs::MCC_GDRV_CFG, 0x0003'0001U).error, Error::UnexpectedReset);
  CHECK_EQ(r.state.chip.writes_since_reset, 0);
  CHECK(!r.drv.IsInitialized());
  CHECK_EQ(r.drv.ReadReg(regs::MCC_INFO_CHIP).error, Error::NotInitialized);
  CHECK(r.Start());
  CHECK(r.drv.WriteReg(regs::MCC_GDRV_CFG, 0x0003'0001U).ok());
}

TEST_CASE("Register mode: DisableDrive after a reset is still attempted") {
  SpiFullRig r(Phase::Register, RegCfg());
  REQUIRE(r.Start());
  REQUIRE(r.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  r.state.chip.BootInto(Phase::Register);
  r.state.chip.SetDrvEn(true);
  const int before = r.state.chip.frames;
  static_cast<void>(r.drv.DisableDrive());
  CHECK(!r.state.chip.DrvEn());
  CHECK(r.state.chip.frames > before);
}

TEST_CASE("RAMDebug through register mode and knob A17") {
  UartFullRig r(Phase::Register, RegCfg());
  REQUIRE(r.Start());
  CHECK_EQ(r.drv.RegRamDebug(16U, 0U, false, 0U).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.RegRamDebug(1U, 32U, false, 0U).error, Error::InvalidArgument); // 5 index bits
  CHECK(r.drv.RegRamDebug(1U, 31U, true, 0xABCD1234U).ok());
  CHECK_EQ(r.drv.RegRamDebug(1U, 31U, false, 0U).value, 0xABCD1234U);
  Config six = RegCfg();
  six.quirks.ram_debug_index_bits = 6;
  UartFullRig s(Phase::Register, six);
  s.state.chip.knobs.ram_index_bits = 6;
  REQUIRE(s.Start());
  CHECK(s.drv.RegRamDebug(2U, 63U, true, 7U).ok());
  CHECK_EQ(s.drv.RegRamDebug(2U, 63U, false, 0U).value, 7U);
  CHECK_EQ(s.drv.RegRamDebug(2U, 64U, false, 0U).error, Error::InvalidArgument);
}

TEST_CASE("Knobs A3 and A4: a matching reading works, the other one does not") {
  Config wide = RegCfg();
  wide.quirks.reg_spi_wide_offset = true;
  SpiFullRig a(Phase::Register, wide);
  a.state.chip.knobs.reg_wide = true;
  REQUIRE(a.Start());
  CHECK_EQ(a.drv.ReadReg(regs::MCC_INFO_CHIP).value, kChipId);
  CHECK(a.drv.WriteReg(regs::MCC_GDRV_CFG, 0x0003'0001U).ok());
  CHECK_EQ(a.drv.ReadReg(regs::MCC_GDRV_CFG).value, 0x0003'0001U);
  SpiFullRig b(Phase::Register, RegCfg()); // driver: table layout, chip: wide layout
  b.state.chip.knobs.reg_wide = true;
  const auto e = b.drv.EnsureInitialized();
  CHECK((!e.ok()) || (b.drv.ReadReg(regs::MCC_GDRV_CFG).value != 0x0003'0001U));
  Config msb = RegCfg();
  msb.quirks.reg_uart_msb_fields = true;
  UartFullRig c(Phase::Register, msb);
  c.state.chip.knobs.reg_uart_msb = true;
  REQUIRE(c.Start());
  CHECK_EQ(c.drv.ReadReg(regs::MCC_INFO_CHIP).value, kChipId);
  CHECK(c.drv.WriteReg(regs::MCC_GDRV_CFG, 0x0003'0001U).ok());
  CHECK_EQ(c.drv.ReadReg(regs::MCC_GDRV_CFG).value, 0x0003'0001U);
}

TEST_CASE("Register-mode bus failures are reported and keep the driver usable") {
  SpiFullRig r(Phase::Register, RegCfg());
  REQUIRE(r.Start());
  r.state.fail_transfer = r.state.transfers;
  CHECK_EQ(r.drv.ReadReg(regs::MCC_INFO_CHIP).error, Error::Bus);
  UartFullRig u(Phase::Register, RegCfg());
  REQUIRE(u.Start());
  u.state.chip.drop_uart_reply = true;
  CHECK_EQ(u.drv.ReadReg(regs::MCC_INFO_CHIP).error, Error::Timeout);
  u.state.chip.drop_uart_reply = false;
  u.state.chip.corrupt_uart_reply = true;
  CHECK_EQ(u.drv.ReadReg(regs::MCC_INFO_CHIP).error, Error::BadReply);
  u.state.chip.corrupt_uart_reply = false;
  CHECK(u.drv.ReadReg(regs::MCC_INFO_CHIP).ok());
}

TEST_CASE("Register SPI: a lost datagram or a stray boot bit is a reset") {
  {
    SpiFullRig r(Phase::Register, RegCfg());
    REQUIRE(r.Start());
    CHECK(r.drv.ReadReg(regs::MCC_INFO_CHIP).ok());
    r.state.chip.skip_reg_counter = true;
    CHECK_EQ(r.drv.WriteReg(regs::MCC_GDRV_CFG, 0x0003'0001U).error, Error::UnexpectedReset);
    CHECK(!r.drv.IsInitialized());
  }
  {
    SpiFullRig r(Phase::Register, RegCfg());
    REQUIRE(r.Start());
    CHECK(r.drv.ReadReg(regs::MCC_INFO_CHIP).ok());
    r.state.chip.stray_boot_bit = true;
    CHECK_EQ(r.drv.WriteReg(regs::MCC_GDRV_CFG, 0x0003'0001U).error, Error::UnexpectedReset);
    CHECK(!r.drv.IsInitialized());
  }
}
