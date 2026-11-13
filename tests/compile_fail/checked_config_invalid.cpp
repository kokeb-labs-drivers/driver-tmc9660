/**
 * @file checked_config_invalid.cpp
 * @brief Must NOT compile: a CheckedConfig built from a configuration that Validate() rejects fails
 * the build, naming the error (handbook KB-CPP20-013).
 *
 * EXPECT: ConfigIsInvalid
 */
#include "tmc9660_types.hpp"

int main() {
  // Device address 0 is not a valid UART address: Validate() rejects it.
  constexpr tmc9660::CheckedConfig kBoard{tmc9660::Config{.device_address = 0}};
  return static_cast<int>(kBoard.Get().device_address);
}
