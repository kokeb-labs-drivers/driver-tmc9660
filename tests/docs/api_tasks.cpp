/**
 * @file api_tasks.cpp
 * @brief The API guide's examples, compiled by the host build (never run).
 */
#include <array>
#include <cstdio>

#include "my_port.hpp"
#include "tmc9660.hpp"

using Chip = tmc9660::TMC9660<MyPort>;

// --8<-- [start:lifecycle]
tmc9660::Error BringUp(Chip& chip) {
  // The one entry point: starts on the first call, then returns Ok with no I/O. After a failure the
  // next call starts again from the beginning.
  const auto r = chip.EnsureInitialized();
  if (!r) {
    std::printf("TMC9660 init: %s\n", tmc9660::ToString(r.error));
  }
  return r.error;
}
// --8<-- [end:lifecycle]

// --8<-- [start:bootloader]
tmc9660::Error StartApplication(Chip& chip) {
  // The chip comes up in its bootloader. Read the configuration, change one field, write it back:
  // only the words that differ are written, the boot-mode word last, and the driver refuses a start
  // that would hang the chip.
  auto cfg = chip.BlReadConfig();
  if (!cfg) {
    return cfg.error;
  }
  tmc9660::BootConfig next = cfg.value;
  if (const auto e = next.Set(tmc9660::BootField::BootMode, 2U); e != tmc9660::Error::Ok) {
    return e; // 2 is parameter mode
  }
  if (const auto e = next.Set(tmc9660::BootField::StartMotorCtrl, 1U); e != tmc9660::Error::Ok) {
    return e;
  }
  return chip.BlWriteConfig(next).error; // follows the chip into parameter mode
}
// --8<-- [end:bootloader]

// --8<-- [start:parameters]
tmc9660::Error SetLimits(Chip& chip) {
  // Every parameter is checked against the datasheet's range and coded set before a byte is sent.
  if (auto r = chip.SetParam(tmc9660::Param::MOTOR_TYPE, 3); !r) { // 3: BLDC; needs the system off
    return r.error;
  }
  if (auto r = chip.SetParam(tmc9660::Param::MAX_TORQUE, 1500); !r) {
    return r.error; // InvalidArgument above 65535: nothing was sent
  }
  return chip.StoreParams().error;
}
// --8<-- [end:parameters]

// --8<-- [start:drive]
tmc9660::Error RunMotor(Chip& chip) {
  // The only way to a non-zero commutation mode. The guards read the flag words first; DRV_EN rises
  // only after they are clean, then the mode is set.
  if (auto r = chip.EnableDrive(3); !r) {
    return r.error; // DeviceFault: an error flag is latched; Refused is never returned here
  }
  const auto r = chip.SetTargetVelocity(500);
  if (!r) {
    static_cast<void>(
        chip.DisableDrive()); // never refused: DRV_EN first, then the commutation mode
    return r.error;
  }
  return tmc9660::Error::Ok;
}
// --8<-- [end:drive]

// --8<-- [start:register-mode]
tmc9660::Error StartGateDriver(Chip& chip) {
  // Register mode: the gate driver starts only through GateDriverStart(), which runs the manual's
  // sequence with the fault and undervoltage guards before the energising write. WriteReg() refuses
  // those bits.
  tmc9660::GateDriverRegConfig cfg{};
  cfg.gdrv_cfg = 0x0003'0000U; // gate currents and adaptive mode: see the register reference
  return chip.GateDriverStart(cfg).error;
}
// --8<-- [end:register-mode]

// --8<-- [start:reset]
tmc9660::Error KeepAlive(Chip& chip) {
  const auto r = chip.Service(); // FAULTN and a heartbeat read: the way to notice a reset
  if (r.error == tmc9660::Error::UnexpectedReset) {
    static_cast<void>(chip.DisableDrive()); // the pin first
    return chip.EnsureInitialized().error;  // the chip lost its configuration: start it again
  }
  return r.error;
}
// --8<-- [end:reset]

// --8<-- [start:errors]
const char* Describe(tmc9660::Error e) {
  switch (e) {
  case tmc9660::Error::Bus:
  case tmc9660::Error::DeviceIdMismatch:
    return "check the bus wiring, the mode and the device address";
  case tmc9660::Error::WrongMode:
    return "the chip is in another mode: nothing was sent";
  case tmc9660::Error::Refused:
    return "a safety guard said no: read the documentation of the call";
  case tmc9660::Error::UnexpectedReset:
    return "the chip restarted: call EnsureInitialized() again";
  default:
    return tmc9660::ToString(e);
  }
}
// --8<-- [end:errors]

// --8<-- [start:deinit]
void PowerDown(Chip& chip) {
  if (auto r = chip.Deinitialize(); !r) { // drops DRV_EN and sets the system off first
    std::printf("TMC9660 shutdown write failed: %s\n", tmc9660::ToString(r.error));
  }
}
// --8<-- [end:deinit]
