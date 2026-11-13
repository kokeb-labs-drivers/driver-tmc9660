/**
 * @file fixture.hpp
 * @brief A rig: bus state, port and driver, started in a chosen phase.
 */
#ifndef TMC9660_TEST_FIXTURE_HPP
#define TMC9660_TEST_FIXTURE_HPP

#include "mock_tmc9660_bus.hpp"
#include "test_harness.hpp"
#include "tmc9660.hpp"

namespace tmc9660_test {

/// Bus state, a port of type `P`, and a driver for it.
template <typename P>
struct Rig {
  BusState state;
  P port;
  tmc9660::TMC9660<P> drv;

  Rig(Phase phase, const tmc9660::Config& config) : state(), port(state), drv(port, config) {
    state.chip.BootInto(phase);
    state.chip.SetDrvEn(
        !tmc9660::detail::HasGpioSet<P>); // a port without the hook wires DRV_EN high
  }

  /// Start the driver; the test fails when it does not start.
  bool Start() {
    return drv.EnsureInitialized().ok();
  }
};

/// A `Config` for `mode` with the defaults.
inline tmc9660::Config ConfigFor(tmc9660::Mode mode) {
  tmc9660::Config c{};
  c.mode = mode;
  return c;
}

using SpiRig = Rig<SpiMin>;
using SpiFullRig = Rig<SpiFull>;
using UartRig = Rig<UartMin>;
using UartFullRig = Rig<UartFull>;

} // namespace tmc9660_test

#endif // TMC9660_TEST_FIXTURE_HPP
