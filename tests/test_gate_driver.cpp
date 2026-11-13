/**
 * @file test_gate_driver.cpp
 * @brief GateDriverStart: latched evidence is read before it is cleared, the protection words are
 * restored and verified, every failure switches the stage off again.
 */
#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

namespace {
Config Cfg() {
  return ConfigFor(Mode::Register);
}
constexpr uint32_t kEnergise = 0x0300'080FU;
constexpr uint32_t kPwmEn = 0x0000'0F01U;
} // namespace

TEST_CASE("GateDriverStart refuses a latched status bit and leaves the evidence in place") {
  SpiFullRig r(Phase::Register, Cfg());
  REQUIRE(r.Start());
  r.state.chip.SetRegValue(regs::MCC_GDRV_STATUS, 0x1U); // LS_SHORT_U latched by the last run
  const auto s = r.drv.GateDriverStart(GateDriverRegConfig{});
  CHECK_EQ(s.error, Error::DeviceFault);
  CHECK_EQ(r.state.chip.energise_events, 0);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_STATUS), 0x1U); // not cleared by the driver
  CHECK(!r.state.chip.DrvEn());
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW) & kEnergise, 0U);
}

TEST_CASE("GateDriverStart refuses a condition that latches again after the clear") {
  SpiFullRig r(Phase::Register, Cfg());
  REQUIRE(r.Start());
  r.state.chip.relatch_on_clear = 0x1U; // reads clean before the clear, set right after it
  const auto s = r.drv.GateDriverStart(GateDriverRegConfig{});
  CHECK_EQ(s.error, Error::DeviceFault);
  CHECK_EQ(r.state.chip.energise_events, 0);
  CHECK(!r.state.chip.DrvEn());
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW) & kEnergise, 0U);
}

TEST_CASE("GateDriverStart refuses a latched fault word and an undervoltage status") {
  for (const uint32_t bit : {0x1U, 0x8U, 0x0001'0000U, 0x8000'0000U, 0x2000'0000U}) {
    SpiFullRig r(Phase::Register, Cfg());
    REQUIRE(r.Start());
    r.state.chip.SetRegValue(regs::MCC_GDRV_FAULT, bit);
    CHECK_EQ(r.drv.GateDriverStart(GateDriverRegConfig{}).error, Error::DeviceFault);
    CHECK_EQ(r.state.chip.energise_events, 0);
  }
}

TEST_CASE("GateDriverStart ignores the bootstrap flags and starts after the evidence is cleared") {
  SpiFullRig r(Phase::Register, Cfg());
  REQUIRE(r.Start());
  r.state.chip.SetRegValue(regs::MCC_GDRV_STATUS, 0x0000'F000U);
  CHECK(r.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  SpiFullRig q(Phase::Register, Cfg());
  REQUIRE(q.Start());
  q.state.chip.SetRegValue(regs::MCC_GDRV_STATUS, 0x1U);
  CHECK(!q.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  CHECK(q.drv.WriteReg(regs::MCC_GDRV_STATUS, 0x1U).ok()); // the caller looked at it and clears it
  CHECK(q.drv.GateDriverStart(GateDriverRegConfig{}).ok());
}

TEST_CASE(
    "GateDriverStart restores fault reporting and protection and writes the protection words") {
  SpiFullRig r(Phase::Register, Cfg());
  REQUIRE(r.Start());
  GateDriverRegConfig c{};
  c.gdrv_prot = 0x1000'0003U;
  c.gdrv_ocp_uvw = 0x0100'0102U;
  c.gdrv_ocp_y2 = 0x0200'0201U;
  c.status_en = 0x0000'FFFFU;
  c.prot_en = 0x0000'00FFU;
  REQUIRE(r.drv.GateDriverStart(c).ok());
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_PROT), c.gdrv_prot);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_OCP_UVW), c.gdrv_ocp_uvw);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_OCP_Y2), c.gdrv_ocp_y2);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_STATUS_EN), c.status_en);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_PROT_EN), c.prot_en);
  CHECK(r.state.chip.DrvEn());
  SpiFullRig d(Phase::Register, Cfg());
  REQUIRE(d.Start());
  REQUIRE(d.drv.GateDriverStart(GateDriverRegConfig{}).ok()); // the defaults protect
  CHECK_EQ(d.state.chip.RegValue(regs::MCC_GDRV_STATUS_EN), 0xEFFF'FFFFU);
  CHECK_EQ(d.state.chip.RegValue(regs::MCC_GDRV_PROT_EN), 0xAFFF'FFFFU);
}

TEST_CASE("GateDriverStart: the protection words must fit their registers") {
  SpiFullRig r(Phase::Register, Cfg());
  REQUIRE(r.Start());
  const int n = r.state.transfers;
  GateDriverRegConfig c{};
  c.prot_en = 0x5000'0000U;
  CHECK_EQ(r.drv.GateDriverStart(c).error, Error::InvalidArgument);
  c = GateDriverRegConfig{};
  c.status_en = 0x1000'0000U;
  CHECK_EQ(r.drv.GateDriverStart(c).error, Error::InvalidArgument);
  c = GateDriverRegConfig{};
  c.gdrv_prot = 0x8000'0000U;
  CHECK_EQ(r.drv.GateDriverStart(c).error, Error::InvalidArgument);
  c = GateDriverRegConfig{};
  c.gdrv_ocp_uvw = 0x8000'0000U;
  CHECK_EQ(r.drv.GateDriverStart(c).error, Error::InvalidArgument);
  c = GateDriverRegConfig{};
  c.gdrv_ocp_y2 = 0x8000'0000U;
  CHECK_EQ(r.drv.GateDriverStart(c).error, Error::InvalidArgument);
  CHECK_EQ(r.state.transfers, n);
}

TEST_CASE(
    "GateDriverStart: a protection word the chip does not keep is VerifyFailed and switches off") {
  for (const RegAddr a : {regs::MCC_GDRV_PROT_EN, regs::MCC_GDRV_STATUS_EN}) {
    SpiFullRig r(Phase::Register, Cfg());
    REQUIRE(r.Start());
    r.state.chip.ignored_reg_write_offset = static_cast<int>(a.offset);
    GateDriverRegConfig c{};
    c.status_en = 0x0000'FFFFU; // not the reset value, so a dropped write shows
    c.prot_en = 0x0000'00FFU;
    CHECK_EQ(r.drv.GateDriverStart(c).error, Error::VerifyFailed);
    CHECK(!r.state.chip.DrvEn());
    CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW) & kEnergise, 0U);
  }
}

TEST_CASE(
    "GateDriverStart: a GDRV_HW the chip stores differently is VerifyFailed and switches off") {
  SpiFullRig r(Phase::Register, Cfg());
  REQUIRE(r.Start());
  r.state.chip.corrupt_gdrv_hw = true;
  CHECK_EQ(r.drv.GateDriverStart(GateDriverRegConfig{}).error, Error::VerifyFailed);
  CHECK(!r.state.chip.DrvEn());
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_PWM_CONFIG) & kPwmEn, 0U);
}

TEST_CASE("GateDriverStart: whichever transfer fails, the stage ends switched off") {
  bool saw_ok = false;
  for (int k = 0; k < 120; ++k) {
    SpiFullRig r(Phase::Register, Cfg());
    REQUIRE(r.Start());
    r.state.fail_transfer = r.state.transfers + k;
    const auto s = r.drv.GateDriverStart(GateDriverRegConfig{});
    if (s.ok()) {
      saw_ok = true;
      continue;
    }
    r.state.fail_transfer = -1;
    if (s.error == Error::Bus) {
      // a lost transfer inside the sequence: the failure path wrote the stop (its own transfers
      // were not the failing one), except when the failing transfer was the stop itself
      CHECK_EQ(r.state.chip.energise_with_flag, 0);
      CHECK_EQ(r.state.chip.energise_without_drv_en, 0);
      const bool off = ((r.state.chip.RegValue(regs::MCC_GDRV_HW) & kEnergise) == 0U) &&
                       ((r.state.chip.RegValue(regs::MCC_PWM_CONFIG) & kPwmEn) == 0U);
      CHECK(off);
      CHECK(!r.state.chip.DrvEn());
    }
  }
  CHECK(saw_ok);
}

TEST_CASE("GateDriverStart: a pin that cannot be driven stops the start and switches off") {
  SpiFullRig r(Phase::Register, Cfg());
  REQUIRE(r.Start());
  r.state.pin_fail = true;
  CHECK_EQ(r.drv.GateDriverStart(GateDriverRegConfig{}).error, Error::Bus);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_PWM_CONFIG) & kPwmEn, 0U);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW) & kEnergise, 0U);
  CHECK_EQ(r.state.chip.energise_events, 0);
}

// --- m4: the masks of the latched-evidence check, bit by bit ------------------------------------

TEST_CASE("GateDriverStart: every MCC_GDRV_STATUS bit but the bootstrap flags refuses the start") {
  constexpr uint32_t kBootstrap = 0x0000'F000U; // RM p.140: charge state, not a fault
  for (unsigned bit = 0U; bit < 32U; ++bit) {
    if (bit == 28U) {
      continue; // not a register bit
    }
    SpiFullRig r(Phase::Register, Cfg());
    REQUIRE(r.Start());
    const uint32_t mask = 1U << bit;
    r.state.chip.SetRegValue(regs::MCC_GDRV_STATUS, mask);
    const auto s = r.drv.GateDriverStart(GateDriverRegConfig{});
    if ((mask & kBootstrap) != 0U) {
      CHECK(s.ok());
    } else {
      CHECK_EQ(s.error, Error::DeviceFault);
      CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_STATUS), mask); // the evidence is untouched
      CHECK_EQ(r.state.chip.energise_events, 0);
    }
  }
}

TEST_CASE("GateDriverStart: MCC_GDRV_FAULT bits 3:0, 19:16, 29 and 31 refuse, the others do not") {
  constexpr uint32_t kRefuse = 0xA00F'000FU; // active faults and the UVLO status bits
  for (unsigned bit = 0U; bit < 32U; ++bit) {
    SpiFullRig r(Phase::Register, Cfg());
    REQUIRE(r.Start());
    const uint32_t mask = 1U << bit;
    r.state.chip.SetRegValue(regs::MCC_GDRV_FAULT, mask);
    const auto s = r.drv.GateDriverStart(GateDriverRegConfig{});
    if ((mask & kRefuse) != 0U) {
      CHECK_EQ(s.error, Error::DeviceFault);
      CHECK_EQ(r.state.chip.energise_events, 0);
    } else {
      CHECK(s.ok());
    }
  }
}

// --- m5: CHOP is 1 before the gate driver is enabled; the bootstrap capacitors charge first -----

TEST_CASE("GateDriverStart: CHOP is 1 when the gate driver is enabled, then the configured one") {
  SpiFullRig r(Phase::Register, Cfg());
  REQUIRE(r.Start());
  GateDriverRegConfig c{};
  c.chop_run = 5U;
  REQUIRE(r.drv.GateDriverStart(c).ok());
  CHECK_EQ(r.state.chip.energise_chop_events, 1);
  CHECK_EQ(r.state.chip.chop_at_energise, 1U);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_PWM_CONFIG) & 7U, 5U);
}

TEST_CASE("GateDriverStart: STATUS_EN and PROT_EN are written after the bootstrap settle time") {
  Config cfg = Cfg();
  cfg.settle_us = 123'456U;
  SpiFullRig r(Phase::Register, cfg);
  REQUIRE(r.Start());
  REQUIRE(r.drv.GateDriverStart(GateDriverRegConfig{}).ok());
  CHECK(r.state.chip.status_en_us >= (r.state.chip.hw_energise_us + 123'456U));
  CHECK(r.state.chip.prot_en_us >= r.state.chip.status_en_us);
}

// --- m6: a protection that trips at once is not an Ok start -------------------------------------

TEST_CASE("GateDriverStart: a trip right after the protections are on is DeviceFault and off") {
  SpiFullRig r(Phase::Register, Cfg());
  REQUIRE(r.Start());
  r.state.chip.trip_on_prot_en = 0x0000'0100; // an overcurrent, latched when PROT_EN is written
  CHECK_EQ(r.drv.GateDriverStart(GateDriverRegConfig{}).error, Error::DeviceFault);
  CHECK(!r.state.chip.DrvEn());
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_GDRV_HW) & kEnergise, 0U);
  CHECK_EQ(r.state.chip.RegValue(regs::MCC_PWM_CONFIG) & kPwmEn, 0U);
}
