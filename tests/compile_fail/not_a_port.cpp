/**
 * @file not_a_port.cpp
 * @brief Must NOT compile: the driver is constrained by the port concept.
 *
 * EXPECT: AnyPort|constraints not satisfied
 */
#include "tmc9660.hpp"

struct NotAPort {
  bool EnsureInitialized() noexcept {
    return true;
  }
};

int main() {
  NotAPort port;
  tmc9660::TMC9660<NotAPort> chip(port);
  return static_cast<int>(chip.EnsureInitialized().error);
}
