/**
 * @file test_kconfig.cpp
 * @brief KconfigDefaults(): the defaults without Kconfig, and menuconfig values when they are set.
 *
 * Built twice: without CONFIG_ macros (KCONFIG_SET undefined) and with the values an sdkconfig.h
 * would define (KCONFIG_SET=1, see tests/CMakeLists.txt).
 */
#include "test_harness.hpp"
#include "tmc9660_kconfig.hpp"

#ifdef KCONFIG_SET
TEST_CASE("Kconfig values reach the Config") {
  constexpr tmc9660::Config kC = tmc9660::KconfigDefaults();
  CHECK_EQ(kC.mode, tmc9660::Mode::Register);
  CHECK_EQ(kC.device_address, 3U);
  CHECK_EQ(kC.boot_timeout_ms, 750U);
  CHECK(kC.hardware_reset);
  CHECK(!kC.uart_canary);
  CHECK_EQ(tmc9660::Validate(kC), tmc9660::Error::Ok);
}
#else
TEST_CASE("Without Kconfig, KconfigDefaults() equals Config{}") {
  constexpr tmc9660::Config kC = tmc9660::KconfigDefaults();
  constexpr tmc9660::Config kD{};
  CHECK_EQ(kC.mode, kD.mode);
  CHECK_EQ(kC.device_address, kD.device_address);
  CHECK_EQ(kC.boot_timeout_ms, kD.boot_timeout_ms);
  CHECK_EQ(kC.hardware_reset, kD.hardware_reset);
  CHECK_EQ(kC.uart_canary, kD.uart_canary);
}
#endif
