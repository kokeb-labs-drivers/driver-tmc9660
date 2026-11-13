/**
 * @file test_parameter.cpp
 * @brief Parameter mode over SPI and UART: tables, validation, guards, reset detection, retries.
 */
#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

namespace {

Config Cfg(Mode m = Mode::Parameter) {
  return ConfigFor(m);
}

} // namespace

TEST_CASE("Constructor does no I/O; every call returns NotInitialized before the start") {
  SpiRig r(Phase::Parameter, Cfg());
  CHECK_EQ(r.state.transfers, 0);
  CHECK_EQ(r.drv.GetParam(Param::MOTOR_TYPE).error, Error::NotInitialized);
  CHECK_EQ(r.drv.SetParam(Param::MAX_TORQUE, 100).error, Error::NotInitialized);
  CHECK_EQ(r.drv.EnableDrive(3).error, Error::NotInitialized);
  CHECK_EQ(r.drv.GetGlobal(GlobalBank::System, 1).error, Error::NotInitialized);
  CHECK_EQ(r.drv.StoreParams().error, Error::NotInitialized);
  CHECK_EQ(r.drv.ReadReg(regs::MCC_INFO_CHIP).error, Error::NotInitialized);
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ChipType).error, Error::NotInitialized);
  CHECK_EQ(r.drv.Service().error, Error::NotInitialized);
  CHECK_EQ(r.state.transfers, 0);
}

TEST_CASE("Start in each mode over each bus; the second call does nothing") {
  {
    SpiRig r(Phase::Parameter, Cfg());
    CHECK(r.Start());
    CHECK_EQ(r.drv.GetMode(), Mode::Parameter);
    const int n = r.state.transfers;
    CHECK(r.Start());
    CHECK_EQ(r.state.transfers, n);
    CHECK_EQ(r.state.bus_up_calls, 1);
    const auto id = r.drv.GetChipId();
    CHECK(id.ok());
    CHECK_EQ(id.value, kChipId);
    CHECK(r.drv.GetVersion().ok());
  }
  {
    UartRig r(Phase::Parameter, Cfg());
    CHECK(r.Start());
    CHECK(r.drv.GetChipId().ok());
  }
  {
    SpiRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
    CHECK(r.Start());
    CHECK_EQ(r.drv.GetMode(), Mode::Bootloader);
  }
  {
    UartRig r(Phase::Bootloader, Cfg(Mode::Bootloader));
    CHECK(r.Start());
  }
  {
    SpiRig r(Phase::Register, Cfg(Mode::Register));
    CHECK(r.Start());
    CHECK_EQ(r.drv.ReadReg(regs::MCC_INFO_CHIP).value, kChipId);
  }
  {
    UartRig r(Phase::Register, Cfg(Mode::Register));
    CHECK(r.Start());
    CHECK_EQ(r.drv.ReadReg(regs::MCC_INFO_CHIP).value, kChipId);
  }
}

TEST_CASE("Start failures: bad Config sends nothing, wrong protocol, bus errors") {
  Config bad = Cfg();
  bad.device_address = 0;
  SpiRig a(Phase::Parameter, bad);
  CHECK_EQ(a.drv.EnsureInitialized().error, Error::InvalidArgument);
  CHECK_EQ(a.state.transfers, 0);
  CHECK_EQ(a.state.bus_up_calls, 0);
  Config even = Cfg();
  even.device_address = 2; // the UART carries TMCL: the address must be odd
  UartRig b(Phase::Parameter, even);
  CHECK_EQ(b.drv.EnsureInitialized().error, Error::InvalidArgument);
  CHECK_EQ(b.state.transfers, 0);
  Config ram = Cfg();
  ram.quirks.ram_debug_index_bits = 7;
  CHECK_EQ(Validate(ram), Error::InvalidArgument);
  ram = Cfg();
  ram.canary_variable = 16;
  CHECK_EQ(Validate(ram), Error::InvalidArgument);
  ram = Cfg();
  ram.mode = ut::RawEnum<Mode>(9);
  CHECK_EQ(Validate(ram), Error::InvalidArgument);
  SpiRig c(Phase::Bootloader,
           Cfg()); // the chip is in the bootloader, the driver expects parameter mode
  CHECK(!c.drv.EnsureInitialized().ok());
  CHECK(!c.drv.IsInitialized());
  SpiRig d(Phase::Parameter, Cfg());
  d.state.bus_up_ok = false;
  CHECK_EQ(d.drv.EnsureInitialized().error, Error::Bus);
  for (int k = 0; k < 3; ++k) {
    SpiRig e(Phase::Parameter, Cfg());
    e.state.fail_transfer = k;
    CHECK_EQ(e.drv.EnsureInitialized().error, Error::Bus);
    CHECK(!e.drv.IsInitialized());
  }
  UartRig f(Phase::Parameter, Cfg());
  f.state.uart_fault = BusStatus::Timeout;
  CHECK_EQ(f.drv.EnsureInitialized().error, Error::Timeout);
}

TEST_CASE("SetParam and GetParam: round trip, signs, validation sends nothing") {
  SpiRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  CHECK(r.drv.SetParam(Param::MAX_TORQUE, 1500).ok());
  CHECK_EQ(r.drv.GetParam(Param::MAX_TORQUE).value, 1500);
  CHECK_EQ(r.state.chip.ParamValue(6), 1500);
  CHECK(r.drv.SetTargetTorque(-200).ok());
  CHECK_EQ(r.drv.GetParam(Param::TARGET_TORQUE).value, -200);
  CHECK(r.drv.SetTargetVelocity(-5000).ok());
  CHECK_EQ(r.drv.GetParam(Param::TARGET_VELOCITY).value, -5000);
  CHECK(r.drv.SetTargetPosition(-2147483647 - 1).ok());
  CHECK_EQ(r.drv.GetParam(Param::TARGET_POSITION).value, -2147483648LL);
  const int n = r.state.transfers;
  CHECK_EQ(r.drv.SetParam(Param::MAX_TORQUE, 70000).error, Error::InvalidArgument); // above 65535
  CHECK_EQ(r.drv.SetParam(Param::MOTOR_TYPE, 9).error,
           Error::InvalidArgument); // not in the coded set
  CHECK_EQ(r.drv.SetParam(Param::ACTUAL_TORQUE, 1).error, Error::InvalidArgument); // read only
  CHECK_EQ(r.drv.SetParam(Param::MAX_TORQUE, 1, 16).error,
           Error::InvalidArgument); // motor above 15
  CHECK_EQ(r.drv.GetParam(Param::MAX_TORQUE, 16).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.SetParam(ut::RawEnum<Param>(9999), 1).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.GetParam(ut::RawEnum<Param>(9999)).error, Error::InvalidArgument);
  CHECK_EQ(r.state.transfers, n);
  CHECK_EQ(r.state.chip.zero_frames, 0);
}

TEST_CASE("ReadRawParam and GetLastReply: one GAP, nothing written, validation sends nothing") {
  SpiRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  CHECK(r.drv.SetParam(Param::MAX_TORQUE, 1500).ok());
  const auto raw = r.drv.ReadRawParam(static_cast<uint16_t>(Param::MAX_TORQUE));
  REQUIRE(raw.ok());
  CHECK_EQ(raw.value, 1500U);
  const LastReply last = r.drv.GetLastReply();
  CHECK_EQ(last.value, 1500U);
  CHECK_EQ(last.status, static_cast<uint8_t>(TmclStatus::Ok));
  CHECK_EQ(last.op, static_cast<uint8_t>(Tmcl::Gap));
  const int n = r.state.transfers;
  CHECK_EQ(r.drv.ReadRawParam(0x1000U).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.ReadRawParam(1U, 16U).error, Error::InvalidArgument);
  CHECK_EQ(r.state.transfers, n);
}

TEST_CASE("Every parameter of Table 57 is reachable and validated") {
  Config sweep = Cfg();
  sweep.allow_pwm_in_system_off = true; // the sweep also writes IDLE_MOTOR_PWM_BEHAVIOR = 0
  SpiRig r(Phase::Parameter, sweep);
  REQUIRE(r.Start());
  int checked = 0;
  for (const ParamInfo& p : kParams) {
    const auto id = static_cast<Param>(p.id);
    if (p.access != Access::WriteOnly) {
      const auto g = r.drv.GetParam(id);
      CHECK(g.ok());
      CHECK_EQ(g.value, p.def);
    } else {
      CHECK_EQ(r.drv.GetParam(id).error, Error::InvalidArgument);
    }
    if (p.access == Access::ReadOnly) {
      CHECK_EQ(r.drv.SetParam(id, p.def).error, Error::InvalidArgument);
    } else if (id != Param::COMMUTATION_MODE) {
      const int before = r.state.transfers;
      CHECK(r.drv.SetParam(id, p.def).ok()); // the default is always accepted
      CHECK(r.state.transfers > before);
      if (!p.coded && (p.access != Access::ReadWriteClear)) {
        CHECK(r.drv.SetParam(id, p.max).ok());
        const int n = r.state.transfers;
        CHECK_EQ(r.drv.SetParam(id, p.max + 1).error, Error::InvalidArgument);
        CHECK_EQ(r.drv.SetParam(id, p.min - 1).error, Error::InvalidArgument);
        CHECK_EQ(r.state.transfers, n);
        CHECK(r.drv.SetParam(id, p.def).ok());
      }
      if (p.coded) {
        for (uint32_t i = 0; i < p.allowed_count; ++i) {
          CHECK(r.drv.SetParam(id, kAllowedValues[static_cast<std::size_t>(p.allowed_first) + i])
                    .ok());
        }
        CHECK(r.drv.SetParam(id, p.def).ok());
      }
    } else {
      // COMMUTATION_MODE is refused by SetParam; the other tests cover it.
    }
    ++checked;
  }
  CHECK_EQ(checked, 289);
  CHECK_EQ(r.state.chip.zero_frames, 0);
}

TEST_CASE("Flags: read, clear with a mask") {
  SpiRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  r.state.chip.SetParamValue(299, 0x30);
  CHECK_EQ(r.drv.GetParam(Param::GENERAL_ERROR_FLAGS).value, 0x30);
  CHECK(r.drv.ClearFlags(Param::GENERAL_ERROR_FLAGS, 0x10).ok());
  CHECK_EQ(r.drv.GetParam(Param::GENERAL_ERROR_FLAGS).value, 0x20);
  CHECK_EQ(r.drv.ClearFlags(Param::MAX_TORQUE, 1).error, Error::InvalidArgument);
}

TEST_CASE("COMMUTATION_MODE: only EnableDrive leaves System off; guards run before anything is "
          "energised") {
  SpiFullRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  r.state.chip.SetDrvEn(false);
  CHECK_EQ(r.drv.SetParam(Param::COMMUTATION_MODE, 3).error, Error::Refused);
  CHECK_EQ(r.drv.EnableDrive(0).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.EnableDrive(9).error, Error::InvalidArgument);
  r.state.chip.SetParamValue(300, 0x1); // a latched gate-driver flag
  CHECK_EQ(r.drv.EnableDrive(3).error, Error::DeviceFault);
  r.state.chip.SetParamValue(300, 0);
  r.state.chip.SetParamValue(299, 0x20); // a latched general error flag
  CHECK_EQ(r.drv.EnableDrive(3).error, Error::DeviceFault);
  CHECK_EQ(r.state.chip.energise_events, 0);
  CHECK(!r.state.chip.DrvEn());
  r.state.chip.SetParamValue(299, 0);
  CHECK(r.drv.EnableDrive(3).ok());
  CHECK_EQ(r.state.chip.energise_events, 1);
  CHECK_EQ(r.state.chip.energise_with_flag, 0);
  CHECK_EQ(r.state.chip.energise_without_drv_en, 0);
  CHECK(r.state.chip.DrvEn());
  CHECK_EQ(r.drv.SetParam(Param::MOTOR_TYPE, 3).error, Error::Refused); // not with the system on
  CHECK(r.drv.DisableDrive().ok());
  CHECK(!r.state.chip.DrvEn());
  CHECK_EQ(r.state.chip.ParamValue(4), 0);
  CHECK(r.drv.SetParam(Param::MOTOR_TYPE, 3).ok());
}

TEST_CASE("EnableDrive drops DRV_EN again when any step fails") {
  for (int k = 0; k < 12; ++k) {
    SpiFullRig r(Phase::Parameter, Cfg());
    REQUIRE(r.Start());
    r.state.chip.SetDrvEn(false);
    r.state.fail_transfer = r.state.transfers + k;
    const auto e = r.drv.EnableDrive(3);
    if (!e.ok()) {
      CHECK(!r.state.chip.DrvEn());
    }
    CHECK_EQ(r.state.chip.energise_without_drv_en, 0);
    CHECK_EQ(r.state.chip.energise_with_flag, 0);
  }
  SpiFullRig p(Phase::Parameter, Cfg());
  REQUIRE(p.Start());
  p.state.pin_fail = true;
  CHECK_EQ(p.drv.EnableDrive(3).error, Error::Bus);
  CHECK_EQ(p.state.chip.energise_events, 0);
}

TEST_CASE("DisableDrive is never refused") {
  SpiFullRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  REQUIRE(r.drv.EnableDrive(3).ok());
  r.state.fail_transfer = r.state.transfers; // the chip write and its repeat fail: the pin drops
  r.state.fail_span = 2;
  const auto e = r.drv.DisableDrive();
  CHECK_EQ(e.error, Error::Bus);
  CHECK(!r.state.chip.DrvEn());
  // uninitialised after a reset: still attempted
  SpiFullRig u(Phase::Parameter, Cfg());
  REQUIRE(u.Start());
  REQUIRE(u.drv.EnableDrive(3).ok());
  u.state.chip.Reset();
  u.state.chip.SetDrvEn(true);
  CHECK_EQ(u.drv.SetParam(Param::MAX_TORQUE, 5).error, Error::UnexpectedReset);
  CHECK(!u.drv.IsInitialized());
  CHECK_EQ(u.state.chip.writes_since_reset, 0);
  const int disables = u.state.chip.disable_cmds;
  static_cast<void>(u.drv.DisableDrive()); // attempted whatever the state
  CHECK(!u.state.chip.DrvEn());
  CHECK(u.state.chip.disable_cmds > disables);
  // a pin failure does not stop the chip write
  SpiFullRig p(Phase::Parameter, Cfg());
  REQUIRE(p.Start());
  REQUIRE(p.drv.EnableDrive(3).ok());
  p.state.pin_fail = true;
  CHECK_EQ(p.drv.DisableDrive().error, Error::Bus);
  CHECK_EQ(p.state.chip.ParamValue(4), 0);
  // Deinitialize disables too
  SpiFullRig q(Phase::Parameter, Cfg());
  REQUIRE(q.Start());
  REQUIRE(q.drv.EnableDrive(3).ok());
  CHECK(q.drv.Deinitialize().ok());
  CHECK(!q.drv.IsInitialized());
  CHECK_EQ(q.state.chip.ParamValue(4), 0);
  CHECK(q.drv.Deinitialize().ok());
}

TEST_CASE("Global parameters") {
  SpiRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  CHECK(r.drv.SetGlobal(GlobalBank::System, global0::SERIAL_ADDRESS, 3).ok());
  const int n = r.state.transfers;
  CHECK_EQ(r.drv.SetGlobal(GlobalBank::System, global0::SERIAL_ADDRESS, 4).error,
           Error::InvalidArgument);
  CHECK_EQ(r.drv.SetGlobal(GlobalBank::System, global0::SERIAL_ADDRESS, 0).error,
           Error::InvalidArgument);
  CHECK_EQ(r.drv.SetGlobal(GlobalBank::System, global0::MAIN_LOOPS, 1).error,
           Error::InvalidArgument);
  CHECK_EQ(r.drv.GetGlobal(GlobalBank::System, global0::GO_TO_TIMEOUT_POWER_DOWN_STATE).error,
           Error::InvalidArgument);
  CHECK_EQ(r.drv.SetGlobal(GlobalBank::System, 9, 1).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.SetGlobal(GlobalBank::UserVars, 16, 1).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.SetGlobal(GlobalBank::Interrupts, 10, 4).error, Error::InvalidArgument);
  CHECK_EQ(r.state.transfers, n);
  CHECK(r.drv.SetGlobal(GlobalBank::UserVars, 3, -77).ok());
  CHECK_EQ(r.drv.GetGlobal(GlobalBank::UserVars, 3).value, -77);
  CHECK(r.drv.SetGlobal(GlobalBank::Interrupts, 0, 1000).ok());
  CHECK(r.drv.SetGlobal(GlobalBank::Interrupts, 31, 3).ok());
  CHECK(r.drv.SetGlobal(GlobalBank::System, global0::HEARTBEAT_MONITORING_CONFIG, 2).ok());
  CHECK(r.drv.SetGlobal(GlobalBank::System, global0::HEARTBEAT_MONITORING_TIMEOUT, 4294967295LL)
            .ok());
  CHECK_EQ(r.drv.GetGlobal(GlobalBank::System, global0::HEARTBEAT_MONITORING_TIMEOUT).value,
           4294967295LL);
  CHECK(r.drv.GetGlobal(GlobalBank::System, global0::MAIN_LOOPS).ok());
  for (const GlobalInfo& g : kGlobals) { // every row is reachable
    const auto bank = static_cast<GlobalBank>(g.bank);
    if (g.access != Access::WriteOnly) {
      CHECK(r.drv.GetGlobal(bank, g.number).ok());
    }
    if (g.access != Access::ReadOnly) {
      CHECK(r.drv.SetGlobal(bank, g.number, g.min + (g.odd_only ? 0 : 0)).ok());
    }
  }
}

TEST_CASE("Unseen reset over SPI: no write follows, the driver becomes uninitialised") {
  SpiRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  REQUIRE(r.drv.SetParam(Param::MAX_TORQUE, 10).ok());
  r.state.chip.Reset();
  CHECK_EQ(r.drv.SetParam(Param::MAX_TORQUE, 20).error, Error::UnexpectedReset);
  CHECK_EQ(r.state.chip.writes_since_reset, 0);
  CHECK(!r.drv.IsInitialized());
  CHECK_EQ(r.drv.GetParam(Param::MAX_TORQUE).error, Error::NotInitialized);
  CHECK(r.Start());
  CHECK(r.drv.SetParam(Param::MAX_TORQUE, 20).ok());
  r.state.chip.Reset();
  CHECK_EQ(r.drv.GetParam(Param::MAX_TORQUE).error,
           Error::UnexpectedReset); // a read after a reset is not trusted
  CHECK(!r.drv.IsInitialized());
}

TEST_CASE("Unseen reset over UART: the canary stops the write") {
  UartRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  CHECK_EQ(r.state.chip.GlobalValue(2, 15), 0x5AC3A55A);
  REQUIRE(r.drv.SetParam(Param::MAX_TORQUE, 10).ok());
  r.state.chip.Reset();
  CHECK_EQ(r.drv.SetParam(Param::MAX_TORQUE, 20).error, Error::UnexpectedReset);
  CHECK_EQ(r.state.chip.writes_since_reset, 0);
  CHECK(!r.drv.IsInitialized());
  Config off = Cfg();
  off.uart_canary = false;
  UartRig o(Phase::Parameter, off);
  REQUIRE(o.Start());
  o.state.chip.Reset();
  CHECK(o.drv.SetParam(Param::MAX_TORQUE, 20).ok()); // documented limit: no canary, no detection
  Config var = Cfg();
  var.canary_variable = 4;
  UartRig v(Phase::Parameter, var);
  REQUIRE(v.Start());
  CHECK_EQ(v.state.chip.GlobalValue(2, 4), 0x5AC3A55A);
}

TEST_CASE("SPI replies: NOT_READY and CHECKSUM_ERROR repeat only the reply request") {
  SpiRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  r.state.chip.not_ready_replies = 3;
  CHECK(r.drv.SetParam(Param::MAX_TORQUE, 55).ok());
  CHECK_EQ(r.state.chip.ParamValue(6), 55);
  r.state.chip.checksum_error_replies = 2;
  const int w = r.state.chip.writes_since_reset;
  CHECK(r.drv.SetParam(Param::MAX_TORQUE, 56).ok());
  CHECK_EQ(r.state.chip.writes_since_reset, w + 1); // the command ran exactly once
  r.state.chip.not_ready_replies = 100;
  const int w2 = r.state.chip.writes_since_reset;
  CHECK_EQ(r.drv.SetParam(Param::MAX_TORQUE, 57).error, Error::Busy);
  CHECK_EQ(r.state.chip.writes_since_reset, w2 + 1);
  r.state.chip.not_ready_replies = 0;
  CHECK_EQ(r.state.chip.zero_frames, 0);
  CHECK(r.state.delay_us >= 1000U);
}

TEST_CASE("UART replies: timeout, fault, bad checksum, wrong address") {
  UartRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  r.state.chip.corrupt_uart_reply = true;
  const auto a = r.drv.GetParam(Param::MAX_TORQUE);
  CHECK_EQ(a.error, Error::BadReply);
  CHECK_EQ(a.value, 0);
  r.state.chip.corrupt_uart_reply = false;
  r.state.chip.wrong_uart_address = true;
  CHECK_EQ(r.drv.GetParam(Param::MAX_TORQUE).error, Error::BadReply);
  r.state.chip.wrong_uart_address = false;
  r.state.chip.drop_uart_reply = true;
  CHECK_EQ(r.drv.GetParam(Param::MAX_TORQUE).error, Error::Timeout);
  r.state.chip.drop_uart_reply = false;
  r.state.uart_fault = BusStatus::Fault;
  CHECK_EQ(r.drv.GetParam(Param::MAX_TORQUE).error, Error::Bus);
  r.state.uart_fault = BusStatus::Ok;
  CHECK(r.drv.GetParam(Param::MAX_TORQUE).ok());
}

TEST_CASE("Nack: the chip's error status is reported and kept") {
  SpiRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  CHECK_EQ(r.drv.RamDebug(RamDebugType::Init, 0, 0).error, Error::Ok);
  const auto x = r.drv.ReferenceSearchCommand(ut::RawEnum<ReferenceSearch>(7));
  CHECK_EQ(x.error, Error::InvalidArgument);
  // a model refusal: the Boot magic is wrong
  CHECK_EQ(r.drv.DisableSpi1Slave().error, Error::Ok);
  CHECK_EQ(r.state.chip.last_cmd, 146);
  CHECK_EQ(r.state.chip.last_cmd_type, 4);
  CHECK_EQ(r.state.chip.last_cmd_motor, 9);
}

TEST_CASE("Scripts: run control, download, validation") {
  SpiRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  const std::array<TmclInstruction, 3> script{
      {{Tmcl::Sap, 6, 0, 100}, {Tmcl::Wait, 0, 0, 10}, {Tmcl::Stop, 0, 0, 0}}};
  CHECK(r.drv.ScriptDownload(script).ok());
  CHECK_EQ(r.state.chip.DownloadCount(), 3);
  CHECK(!r.state.chip.InDownload());
  const int n = r.state.transfers;
  const std::array<TmclInstruction, 1> boot{{{Tmcl::Boot, 0x81, 0x9, 0}}};
  CHECK_EQ(r.drv.ScriptDownload(boot).error, Error::InvalidArgument);
  const std::array<TmclInstruction, 1> big{{{Tmcl::Sap, 0x1000, 0, 0}}};
  CHECK_EQ(r.drv.ScriptDownload(big).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.ScriptDownload(std::span<const TmclInstruction>{}).error, Error::InvalidArgument);
  CHECK_EQ(r.state.transfers, n);
  CHECK(r.drv.ScriptRun(false, 0).ok());
  CHECK_EQ(r.drv.ScriptStatus().value, 1);
  CHECK(r.drv.ScriptStep().ok());
  CHECK(r.drv.ScriptStop().ok());
  CHECK_EQ(r.drv.ScriptStatus().value, 0);
  CHECK(r.drv.ScriptRun(true, 12).ok());
  CHECK(r.drv.ScriptReset().ok());
  CHECK_EQ(r.drv.ScriptReadMem(5).value, 0x12340005U);
}

TEST_CASE("RamDebug, I/O, reference search, stored settings") {
  UartRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  CHECK(r.drv.RamDebug(RamDebugType::SampleCount, 0, 100).ok());
  CHECK_EQ(r.drv.RamDebug(RamDebugType::SampleCount, 0, 0).value, 100);
  CHECK_EQ(r.drv.RamDebug(ut::RawEnum<RamDebugType>(2), 0, 0).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.RamDebug(ut::RawEnum<RamDebugType>(15), 0, 0).error, Error::InvalidArgument);
  CHECK(r.drv.SetIo(3, true).ok());
  CHECK_EQ(r.drv.SetIo(19, true).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.GetIo(19, false).error, Error::InvalidArgument);
  CHECK_EQ(r.drv.GetIo(2, true).value, 1234);
  CHECK(r.drv.ReferenceSearchCommand(ReferenceSearch::Start).ok());
  CHECK(r.drv.ReferenceSearchCommand(ReferenceSearch::Status).ok());
  CHECK(r.drv.ReferenceSearchCommand(ReferenceSearch::Stop).ok());
  CHECK(r.drv.StoreParams().ok());
  CHECK(r.drv.MotorStop().ok());
  const int n = r.state.transfers;
  CHECK_EQ(r.drv.FactoryDefault(false).error, Error::Refused);
  CHECK_EQ(r.state.transfers, n);
  CHECK(r.drv.FactoryDefault(true).ok());
  REQUIRE(r.drv.EnableDrive(3).ok());
  CHECK_EQ(r.drv.FactoryDefault(true).error, Error::Refused); // the system is on
}

TEST_CASE("PowerDown switches off first and leaves the driver uninitialised") {
  UartFullRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  REQUIRE(r.drv.EnableDrive(3).ok());
  CHECK_EQ(r.drv.PowerDown(ut::RawEnum<PowerDownTime>(8)).error, Error::InvalidArgument);
  CHECK(r.drv.PowerDown(PowerDownTime::S2).ok());
  CHECK_EQ(r.state.chip.ParamValue(4), 0);
  CHECK_EQ(r.state.chip.GlobalValue(0, 11), 3);
  CHECK(!r.drv.IsInitialized());
}

TEST_CASE("ReturnToBootloader: system off first, then Boot, then the bootloader answers") {
  SpiFullRig r(Phase::Parameter, Cfg());
  REQUIRE(r.Start());
  REQUIRE(r.drv.EnableDrive(3).ok());
  CHECK(r.drv.ReturnToBootloader().ok());
  CHECK_EQ(r.state.chip.boot_with_drive_on, 0);
  CHECK_EQ(r.drv.GetMode(), Mode::Bootloader);
  CHECK(r.state.chip.GetPhase() == Phase::Bootloader);
  CHECK_EQ(r.drv.BlGetInfo(BlInfo::ChipType).value, kChipId);
  // when the switch-off fails, no Boot is sent
  SpiFullRig f(Phase::Parameter, Cfg());
  REQUIRE(f.Start());
  REQUIRE(f.drv.EnableDrive(3).ok());
  f.state.fail_transfer = f.state.transfers;
  f.state.fail_span = 2; // the stop and its repeat
  CHECK(!f.drv.ReturnToBootloader().ok());
  CHECK(f.state.chip.GetPhase() == Phase::Parameter);
  UartRig u(Phase::Parameter, Cfg());
  REQUIRE(u.Start());
  CHECK(u.drv.ReturnToBootloader().ok());
  CHECK_EQ(u.drv.BlGetInfo(BlInfo::ChipFrequency).value, 40'000'000U);
}

TEST_CASE("Service: FAULTN and the heartbeat read") {
  Config c = Cfg();
  c.faultn_is_reset = true;
  SpiFullRig r(Phase::Parameter, c);
  REQUIRE(r.Start());
  const int n = r.state.transfers;
  CHECK(r.drv.Service().ok());
  CHECK(r.state.transfers > n);
  r.state.faultn_high = false;
  CHECK_EQ(r.drv.Service().error, Error::UnexpectedReset);
  CHECK(!r.drv.IsInitialized());
  SpiFullRig q(Phase::Parameter, Cfg()); // not configured as a reset signal
  REQUIRE(q.Start());
  q.state.faultn_high = false;
  CHECK(q.drv.Service().ok());
}

TEST_CASE("Hardware reset at start-up, with DRV_EN low first") {
  Config c = Cfg();
  c.hardware_reset = true;
  SpiFullRig r(Phase::Parameter, c);
  r.state.chip.SetDrvEn(true);
  CHECK(r.Start());
  CHECK(!r.state.chip.DrvEn());
  CHECK(r.state.logs > 0);
  SpiRig n(Phase::Parameter, c); // no GpioSet hook
  CHECK_EQ(n.drv.EnsureInitialized().error, Error::Unsupported);
  CHECK_EQ(n.state.transfers, 0);
  Config t = c;
  t.boot_timeout_ms = 20;
  SpiFullRig w(Phase::Bootloader, t); // expects parameter mode, the chip stays in the bootloader
  CHECK(!w.drv.EnsureInitialized().ok());
}
