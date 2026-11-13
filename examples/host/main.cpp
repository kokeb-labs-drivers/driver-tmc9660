/**
 * @file main.cpp
 * @brief The TMC9660 driver on your PC: from the bootloader to a running drive, against a simulated
 * chip.
 *
 * No hardware and no SDK: the host port (host_tmc9660_port.hpp) connects the real driver to the
 * chip model the host tests use. The steps are the ones a board goes through: check the bootloader,
 * start the application in parameter mode, set limits, switch the drive on through the guarded
 * call, read it back and switch it off. The same calls run on an MCU with that MCU's port
 * (examples/esp32/). Exit code 0 when every call succeeded.
 */
#include <cstdint>
#include <cstdio>

#include "host_tmc9660_port.hpp"
#include "tmc9660.hpp"

namespace {

/// Print the failing call and return false.
bool Step(const char* what, tmc9660::Error error) {
  if (error != tmc9660::Error::Ok) {
    static_cast<void>(std::printf("%s failed: %s\n", what, tmc9660::ToString(error)));
    return false;
  }
  return true;
}

} // namespace

int main() {
  HostTmc9660Port port;
  port.chip.SetDrvEn(false);
  tmc9660::Config config{};
  config.mode = tmc9660::Mode::Bootloader;              // the chip comes up in its bootloader
  tmc9660::TMC9660<HostTmc9660Port> chip(port, config); // no I/O yet

  static_cast<void>(std::printf("TMC9660 driver %s on the host\n", tmc9660::GetDriverVersion()));
  if (!Step("EnsureInitialized", chip.EnsureInitialized().error)) {
    return 1;
  }
  const auto id = chip.BlGetInfo(tmc9660::BlInfo::ChipType);
  if (!Step("BlGetInfo", id.error)) {
    return 1;
  }
  static_cast<void>(std::printf("bootloader: chip type %08X\n", static_cast<unsigned>(id.value)));

  // Start the motion-control application in parameter mode. The driver refuses a reserved boot mode
  // or a missing 40 MHz clock, and follows the chip into the new mode.
  if (!Step("BlStartApplication", chip.BlStartApplication(tmc9660::BootMode::Parameter).error)) {
    return 1;
  }
  static_cast<void>(std::printf("parameter mode: chip type %08X\n",
                                static_cast<unsigned>(chip.GetChipId().value)));

  // Limits and a target, validated against the datasheet's ranges before anything is sent.
  if (!Step("SetParam MAX_TORQUE", chip.SetParam(tmc9660::Param::MAX_TORQUE, 1500).error) ||
      !Step("SetParam MOTOR_POLE_PAIRS",
            chip.SetParam(tmc9660::Param::MOTOR_POLE_PAIRS, 7).error)) {
    return 1;
  }
  const auto bad =
      chip.SetParam(tmc9660::Param::MAX_TORQUE, 70000); // above the range: nothing is sent
  static_cast<void>(std::printf("MAX_TORQUE 70000: %s\n", tmc9660::ToString(bad.error)));

  // The only way to a non-zero commutation mode: the guards read the flags, DRV_EN rises, then the
  // mode is set.
  if (!Step("EnableDrive", chip.EnableDrive(3).error)) {
    return 1;
  }
  static_cast<void>(std::printf("drive on, DRV_EN %s\n", port.chip.DrvEn() ? "high" : "low"));
  const auto torque = chip.GetParam(tmc9660::Param::MAX_TORQUE);
  static_cast<void>(std::printf("MAX_TORQUE %d\n", static_cast<int>(torque.value)));

  // A stop is never refused: DRV_EN first, then the commutation mode.
  if (!Step("DisableDrive", chip.DisableDrive().error)) {
    return 1;
  }
  static_cast<void>(std::printf("drive off, DRV_EN %s\n", port.chip.DrvEn() ? "high" : "low"));
  static_cast<void>(std::puts("done: 0 errors"));
  return 0;
}
