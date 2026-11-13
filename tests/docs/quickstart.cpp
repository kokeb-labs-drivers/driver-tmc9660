/**
 * @file quickstart.cpp
 * @brief The Quickstart page's example, compiled by the host build (never run).
 */
#include "my_port.hpp"

// --8<-- [start:quickstart]
#include "tmc9660.hpp"

int read_actual_velocity(MyPort& port, int32_t& velocity) {
  tmc9660::Config config{};
  config.mode =
      tmc9660::Mode::Parameter; // the chip boots into the mode its boot configuration says
  tmc9660::TMC9660<MyPort> chip(port, config); // no I/O yet

  if (auto r = chip.EnsureInitialized(); !r) { // 1. bus up, identity check, first handshake
    return static_cast<int>(r.error); //    Bus, Timeout or DeviceIdMismatch: check wiring and mode
  }
  const auto v =
      chip.GetParam(tmc9660::Param::ACTUAL_VELOCITY); // 2. a checked read of one parameter
  if (!v) {
    return static_cast<int>(v.error); //    UnexpectedReset: the chip restarted, start again
  }
  velocity = static_cast<int32_t>(v.value); // 3. in the unit of the parameter
  return 0;
}
// --8<-- [end:quickstart]
