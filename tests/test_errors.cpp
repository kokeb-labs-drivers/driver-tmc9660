/**
 * @file test_errors.cpp
 * @brief Every Error value is returned by some call; names, Result and Config validation.
 */
#include <string_view>

#include "fixture.hpp"

using namespace tmc9660;
using namespace tmc9660_test;

TEST_CASE("Every Error value comes out of a real call") {
  bool seen[32] = {};
  const auto note = [&seen](Error e) { seen[static_cast<unsigned>(e)] = true; };
  {
    SpiRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
    note(r.drv.GetParam(Param::MAX_TORQUE).error); // NotInitialized
    REQUIRE(r.Start());
    note(r.drv.SetParam(Param::MAX_TORQUE, 70000).error);   // InvalidArgument
    note(r.drv.SetParam(Param::COMMUTATION_MODE, 3).error); // Refused
    note(r.drv.ReadReg(regs::MCC_INFO_CHIP).error);         // WrongMode
    r.state.fail_transfer = r.state.transfers;
    note(r.drv.GetParam(Param::MAX_TORQUE).error); // Bus
    r.state.chip.Reset();
    note(r.drv.SetParam(Param::MAX_TORQUE, 1).error); // UnexpectedReset
  }
  {
    SpiRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
    REQUIRE(r.Start());
    r.state.chip.not_ready_replies = 1000;
    note(r.drv.GetParam(Param::MAX_TORQUE).error); // Busy
  }
  {
    UartRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
    REQUIRE(r.Start());
    r.state.chip.drop_uart_reply = true;
    note(r.drv.GetParam(Param::MAX_TORQUE).error); // Timeout
    r.state.chip.drop_uart_reply = false;
    r.state.chip.corrupt_uart_reply = true;
    note(r.drv.GetParam(Param::MAX_TORQUE).error); // BadReply
  }
  {
    UartRig r(Phase::Bootloader, ConfigFor(Mode::Bootloader));
    REQUIRE(r.Start());
    r.state.chip.corrupt_uart_reply = true;
    note(r.drv.BlGetInfo(BlInfo::ChipType).error); // Crc
    r.state.chip.corrupt_uart_reply = false;
    note(r.drv.BlGetInfo(ut::RawEnum<BlInfo>(99)).error); // Nack
  }
  {
    SpiRig r(Phase::Bootloader, ConfigFor(Mode::Bootloader));
    r.state.chip.chip_id = 1U;
    note(r.drv.EnsureInitialized().error); // DeviceIdMismatch
  }
  {
    SpiRig r(Phase::Parameter, ConfigFor(Mode::Parameter));
    REQUIRE(r.Start());
    r.state.chip.SetParamValue(299, 0x20);
    note(r.drv.EnableDrive(3).error); // DeviceFault
  }
  {
    SpiRig r(Phase::Register, ConfigFor(Mode::Register));
    REQUIRE(r.Start());
    note(r.drv.ReadReg(RegAddr{Block::Mcc, 0x200U}).error); // Unsupported
  }
  {
    SpiRig r(Phase::Bootloader, ConfigFor(Mode::Bootloader));
    REQUIRE(r.Start());
    BootConfig next = r.drv.BlReadConfig().value;
    REQUIRE(Error::Ok == next.SetWord(3U, 0xF0U));
    r.state.chip.corrupt_cfg_write = true;
    note(r.drv.BlWriteConfig(next).error); // VerifyFailed
  }
  note(Error::Ok);
  const Error expected[] = {Error::Ok,
                            Error::NotInitialized,
                            Error::InvalidArgument,
                            Error::Bus,
                            Error::Timeout,
                            Error::DeviceIdMismatch,
                            Error::Crc,
                            Error::Busy,
                            Error::DeviceFault,
                            Error::Unsupported,
                            Error::UnexpectedReset,
                            Error::Nack,
                            Error::WrongMode,
                            Error::Refused,
                            Error::VerifyFailed,
                            Error::BadReply};
  for (const Error e : expected) {
    CHECK(seen[static_cast<unsigned>(e)]);
  }
}

TEST_CASE("ToString names every value; an unknown value is named Unknown") {
  CHECK(std::string_view(ToString(Error::Ok)) == std::string_view("Ok"));
  CHECK(std::string_view(ToString(Error::UnexpectedReset)) == std::string_view("UnexpectedReset"));
  CHECK(std::string_view(ToString(ut::RawEnum<Error>(77))) == std::string_view("Unknown"));
  const Error all[] = {Error::Ok,
                       Error::NotInitialized,
                       Error::InvalidArgument,
                       Error::Bus,
                       Error::Timeout,
                       Error::DeviceIdMismatch,
                       Error::Crc,
                       Error::Busy,
                       Error::DeviceFault,
                       Error::Unsupported,
                       Error::UnexpectedReset,
                       Error::Nack,
                       Error::WrongMode,
                       Error::Refused,
                       Error::VerifyFailed,
                       Error::BadReply};
  for (const Error e : all) {
    CHECK(std::string_view(ToString(e)) != std::string_view("Unknown"));
  }
}

TEST_CASE("Result carries a value only on success") {
  const auto ok = Result<int>::Success(5);
  CHECK(ok.ok());
  CHECK(static_cast<bool>(ok));
  CHECK_EQ(ok.value, 5);
  const auto bad = Result<int>::Failure(Error::Bus);
  CHECK(!bad.ok());
  CHECK_EQ(bad.value, 0);
  CHECK_EQ(bad.error, Error::Bus);
  CHECK(Result<void>::Success().ok());
  CHECK_EQ(Result<void>::Failure(Error::Refused).error, Error::Refused);
}

TEST_CASE("Config validation rejects every out-of-range field") {
  Config c{};
  CHECK_EQ(Validate(c), Error::Ok);
  Config x = c;
  x.mode = ut::RawEnum<Mode>(7);
  CHECK_EQ(Validate(x), Error::InvalidArgument);
  x = c;
  x.device_address = 0;
  CHECK_EQ(Validate(x), Error::InvalidArgument);
  x = c;
  x.host_address = 0;
  CHECK_EQ(Validate(x), Error::InvalidArgument);
  x = c;
  x.tmcl_host_address = 0;
  CHECK_EQ(Validate(x), Error::InvalidArgument);
  x = c;
  x.quirks.ram_debug_index_bits = 4;
  CHECK_EQ(Validate(x), Error::InvalidArgument);
  x = c;
  x.canary_variable = 16;
  CHECK_EQ(Validate(x), Error::InvalidArgument);
  x = c;
  x.boot_timeout_ms = 0;
  CHECK_EQ(Validate(x), Error::InvalidArgument);
  x = c;
  x.reply_timeout_us = 0;
  CHECK_EQ(Validate(x), Error::InvalidArgument);
  x = c;
  x.spi_reply_retries = 51;
  CHECK_EQ(Validate(x), Error::InvalidArgument);
}

TEST_CASE("CheckedConfig proves a configuration at build time") {
  constexpr CheckedConfig kBoard{Config{}};
  CHECK_EQ(kBoard.Get().device_address, 1U);
  CHECK_EQ(Validate(kBoard.Get()), Error::Ok);
}
