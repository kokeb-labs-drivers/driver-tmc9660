/**
 * @file configuration.cpp
 * @brief The Configuration page's examples, compiled by the host build (never run).
 */
#include "my_port.hpp"
#include "tmc9660.hpp"
#include "tmc9660_kconfig.hpp"

// --8<-- [start:custom]
tmc9660::Config UartConfig() {
  tmc9660::Config c{}; // the defaults
  c.mode = tmc9660::Mode::Parameter;
  c.device_address = 3;    // UART: odd, as set in the boot configuration
  c.boot_timeout_ms = 750; // how long to wait for the chip after a reset
  c.uart_canary = true;    // detect an unseen reset through a user variable
  return c;                // tmc9660::Validate(c) checks it without the chip
}

tmc9660::Error StartOnUart(MyUartPort& port) {
  static tmc9660::TMC9660<MyUartPort> chip(port, UartConfig()); // stored, no I/O
  return chip.EnsureInitialized().error; // InvalidArgument if the config is out of range
}
// --8<-- [end:custom]

// --8<-- [start:checked]
// A configuration proven at build time: an invalid one does not compile.
constexpr tmc9660::CheckedConfig kBoardConfig{tmc9660::Config{.mode = tmc9660::Mode::Register}};

tmc9660::Mode BoardMode() {
  return kBoardConfig.Get().mode;
}
// --8<-- [end:checked]

// --8<-- [start:quirks]
tmc9660::Config RegisterModeWithOtherReadings() {
  tmc9660::Config c{};
  c.mode = tmc9660::Mode::Register;
  c.quirks.reg_spi_wide_offset =
      true; // ambiguity A3: the other reading of the register datagram layout
  c.quirks.ram_debug_index_bits = 6; // ambiguity A17: a 6-bit RAMDebug index
  return c;
}
// --8<-- [end:quirks]

// --8<-- [start:kconfig]
tmc9660::Error StartFromMenuconfig(MyPort& port) {
  static tmc9660::TMC9660<MyPort> chip(port, tmc9660::KconfigDefaults());
  return chip.EnsureInitialized().error;
}
// --8<-- [end:kconfig]
