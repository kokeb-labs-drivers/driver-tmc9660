/**
 * @file tmc9660_types.hpp
 * @brief Public types of the TMC9660 driver: errors, `Result<T>`, the protocol enumerations,
 * `Config` and its model knobs.
 * @copyright Copyright (c) 2026 The driver-tmc9660 authors. SPDX-License-Identifier: MIT
 *
 * @details
 * Everything here is plain data with no platform dependency, so handlers, tests and tools can
 * include it without the driver template. The configuration defaults are the values the datasheet
 * gives (device address 1, host address 255 for the bootloader, 2 for TMCL).
 */
#ifndef TMC9660_TYPES_HPP
#define TMC9660_TYPES_HPP

#include <cstdint>

#include "tmc9660_detail.hpp"
#include "tmc9660_version.h"

/** @brief The TMC9660 driver: everything it declares lives here; internals in `detail`. */
namespace tmc9660 {

/**
 * @defgroup tmc9660_types Driver types
 * @brief Errors, results, protocol enumerations and configuration.
 * @{
 */

/**
 * @brief Why a call failed. Every call of the driver returns one of these in its `Result`.
 *
 * Values 0 to 9 are the shared base set; chip-specific values start at 16. `[[nodiscard]]` on the
 * type: a function that returns a bare `Error` cannot have it ignored either (KB-CPP20-037).
 */
enum class [[nodiscard]] Error : uint8_t {
  // Base set: the same names and values in every driver of the org (standards/driver.md).
  Ok = 0,              ///< The call succeeded.
  NotInitialized = 1,  ///< `EnsureInitialized()` has not succeeded, or `Deinitialize()` or a
                       ///< detected reset ran since.
  InvalidArgument = 2, ///< An argument or a `Config` field is out of range; nothing was sent.
  Bus = 3,             ///< A port hook returned `false` or `BusStatus::Fault`.
  Timeout = 4,         ///< No reply, or the chip did not become ready in the allowed time.
  DeviceIdMismatch =
      5,           ///< The chip ID is not the TMC9660's: wrong chip, no chip or a wiring fault.
  Crc = 6,         ///< The bootloader UART CRC-8 of a reply did not match.
  Busy = 7,        ///< The chip answered BUSY and the retries ran out.
  DeviceFault = 8, ///< A latched error flag forbids the call (gate driver or general error flags).
  Unsupported = 9, ///< The hook or silicon feature this call needs is absent.
  // Chip-specific values start at 16; each one is returned by some call (tests/test_errors.cpp).
  UnexpectedReset =
      16,         ///< The chip reset since the last validated reply; the driver is uninitialised.
  Nack = 17,      ///< The chip answered with an error status; see `LastStatus()`.
  WrongMode = 18, ///< The call belongs to another mode than the chip is in; nothing was sent.
  Refused = 19,   ///< A safety guard said no; nothing was sent or changed.
  VerifyFailed = 20, ///< A read back after a write did not match.
  BadReply = 21, ///< A reply failed its checksum, address or opcode check; the value is discarded.
};

/**
 * @brief The name of an error, for logs.
 * @param error The error.
 * @return A static string such as `"Timeout"`; `"Unknown"` for a value that is not an enumerator.
 */
constexpr const char* ToString(Error error) noexcept {
  switch (error) {
  case Error::Ok:
    return "Ok";
  case Error::NotInitialized:
    return "NotInitialized";
  case Error::InvalidArgument:
    return "InvalidArgument";
  case Error::Bus:
    return "Bus";
  case Error::Timeout:
    return "Timeout";
  case Error::DeviceIdMismatch:
    return "DeviceIdMismatch";
  case Error::Crc:
    return "Crc";
  case Error::Busy:
    return "Busy";
  case Error::DeviceFault:
    return "DeviceFault";
  case Error::Unsupported:
    return "Unsupported";
  case Error::UnexpectedReset:
    return "UnexpectedReset";
  case Error::Nack:
    return "Nack";
  case Error::WrongMode:
    return "WrongMode";
  case Error::Refused:
    return "Refused";
  case Error::VerifyFailed:
    return "VerifyFailed";
  case Error::BadReply:
    return "BadReply";
  default:
    break;
  }
  return "Unknown";
}

/**
 * @brief The value of a call, or why there is none.
 *
 * Check `ok()` (or the `bool` conversion) before reading `value`; on failure `value` is `T{}`.
 *
 * @tparam T The value type. `Result<void>` carries only the error.
 */
template <typename T>
struct [[nodiscard]] Result {
  T value{};              ///< The value; `T{}` when the call failed.
  Error error{Error::Ok}; ///< `Error::Ok` on success.

  /** @return true when the call succeeded. */
  [[nodiscard]] constexpr bool ok() const noexcept {
    return error == Error::Ok;
  }
  /** @return true when the call succeeded. */
  constexpr explicit operator bool() const noexcept {
    return ok();
  }

  /**
   * @brief A successful result.
   * @param v The value.
   * @return `{v, Error::Ok}`.
   */
  static constexpr Result Success(T v) noexcept {
    return {v, Error::Ok};
  }
  /**
   * @brief A failed result.
   * @param e Why it failed; never `Error::Ok`.
   * @return `{T{}, e}`.
   */
  static constexpr Result Failure(Error e) noexcept {
    return {T{}, e};
  }
};

/** @brief The result of a call that returns no value. */
template <>
struct [[nodiscard]] Result<void> {
  Error error{Error::Ok}; ///< `Error::Ok` on success.

  /** @return true when the call succeeded. */
  [[nodiscard]] constexpr bool ok() const noexcept {
    return error == Error::Ok;
  }
  /** @return true when the call succeeded. */
  constexpr explicit operator bool() const noexcept {
    return ok();
  }

  /** @return A successful result. */
  static constexpr Result Success() noexcept {
    return {Error::Ok};
  }
  /**
   * @brief A failed result.
   * @param e Why it failed; never `Error::Ok`.
   * @return `{e}`.
   */
  static constexpr Result Failure(Error e) noexcept {
    return {e};
  }
};

/** @brief Severity passed to the optional `Log()` hook. */
enum class LogLevel : uint8_t {
  Error = 0, ///< A call failed.
  Warning,   ///< Something unexpected that the driver recovered from.
  Info,      ///< Lifecycle events: initialised, mode changes.
  Debug,     ///< Detail for bring-up.
};

/** @brief What a UART port reports for one exchange. */
enum class BusStatus : uint8_t {
  Ok = 0,  ///< The bytes were sent and the expected reply was received.
  Fault,   ///< The UART failed (framing, overrun, driver error).
  Timeout, ///< The reply did not arrive in time.
};

/** @brief The control pins of the chip that the optional `GpioSet()` and `GpioGet()` hooks may
 * drive. */
enum class CtrlPin : uint8_t {
  Reset = 0, ///< RSTN, active low on the package pin; ACTIVE holds the chip in reset.
  DrvEn,     ///< DRV_EN, active high on the package pin; ACTIVE allows the gate-driver outputs.
  Wake,      ///< WAKE, the wake-up pin (ACTIVE: the application's wake condition is asserted).
  Faultn,    ///< FAULTN, an input; `GpioGet` reports the pin level (low: asserted).
};

/** @brief Logical level of a control pin; the port maps it to the physical polarity. */
enum class GpioSignal : uint8_t {
  INACTIVE = 0, ///< The pin function is deasserted.
  ACTIVE = 1,   ///< The pin function is asserted.
};

/** @brief The protocol the chip speaks on its host interface. */
enum class Mode : uint8_t {
  Bootloader = 0, ///< The bootloader: GET_INFO, memory access, configuration.
  Parameter = 1,  ///< The application in parameter mode: TMCL commands.
  Register = 2,   ///< The application in register mode: direct register access.
};

namespace detail {
/// `Mode` is contiguous from 0: the last enumerator for `IsEnumerator()`.
template <>
struct EnumLast<Mode> {
  /// The last enumerator.
  static constexpr Mode kValue = Mode::Register;
};
} // namespace detail

/** @brief BOOT_MODE of the boot configuration (offset 8, bits 0-1). */
enum class BootMode : uint8_t {
  Reserved =
      0, ///< The hardware default: no application. The driver never starts the application with it.
  Register = 1,  ///< Register mode.
  Parameter = 2, ///< Parameter mode.
  Reserved3 = 3, ///< Reserved.
};

/** @brief External memory banks of the bootloader (SET_BANK). */
enum class Bank : uint8_t {
  SpiFlash = 1,  ///< External SPI flash.
  I2cEeprom = 2, ///< External I2C EEPROM.
  Otp = 3,       ///< One-time programmable memory.
  Config = 5,    ///< The 64-byte configuration memory at 0x00020000.
};

/** @brief GET_INFO selectors of the bootloader. */
enum class BlInfo : uint8_t {
  ChipType = 0,          ///< 0x544D0001.
  BlVersion = 1,         ///< Bootloader version.
  Features = 2,          ///< Feature bits.
  GitInfo = 12,          ///< Git revision of the bootloader.
  ChipVersion = 13,      ///< Silicon version.
  ChipFrequency = 14,    ///< System clock in Hz.
  ConfigMemStart = 17,   ///< Start address of the configuration memory.
  ConfigMemSize = 18,    ///< Size of the configuration memory in bytes.
  OtpMemSize = 19,       ///< Size of the OTP memory.
  I2cMemSize = 20,       ///< Size of the I2C memory.
  SpiMemSize = 21,       ///< Size of the SPI flash.
  PartitionVersion = 22, ///< Partition table version.
  SpiMemPartitions = 25, ///< Partitions in the SPI flash.
  I2cMemPartitions = 26, ///< Partitions in the I2C memory.
  ChipVariant = 28,      ///< Chip variant (2).
};

/** @brief Bootloader reply status codes (datasheet bootloader chapter). */
enum class BlStatus : uint8_t {
  Ok = 0,                 ///< Success.
  CmdNotFound = 1,        ///< Unknown command.
  InvalidAddr = 3,        ///< Address not valid.
  InvalidValue = 4,       ///< Value not valid.
  InvalidBank = 14,       ///< Bank not valid.
  Busy = 15,              ///< SPI only: the reply is not ready.
  MemUnconfigured = 17,   ///< External memory not configured.
  OtpError = 18,          ///< SPI only: OTP error.
  SessionStart = 19,      ///< First reply of a session; the value is the version.
  CmdNotAvailable = 20,   ///< Command not available.
  BootloaderResumed = 21, ///< SPI only: the bootloader restarted.
};

/** @brief Bootloader command codes. */
enum class BlCmd : uint8_t {
  GetInfo = 0,           ///< Read an information value.
  GetBank = 8,           ///< Read the current memory bank.
  SetBank = 9,           ///< Select a memory bank.
  GetAddress = 10,       ///< Read the current memory address.
  SetAddress = 11,       ///< Set the memory address.
  Read32 = 12,           ///< Read 32 bits.
  Read32Inc = 13,        ///< Read 32 bits, then increment the address.
  Read16 = 14,           ///< Read 16 bits.
  Read16Inc = 15,        ///< Read 16 bits, then increment.
  Read8 = 16,            ///< Read 8 bits.
  Read8Inc = 17,         ///< Read 8 bits, then increment.
  Write32 = 18,          ///< Write 32 bits.
  Write32Inc = 19,       ///< Write 32 bits, then increment.
  Write16 = 20,          ///< Write 16 bits.
  Write16Inc = 21,       ///< Write 16 bits, then increment.
  Write8 = 22,           ///< Write 8 bits.
  Write8Inc = 23,        ///< Write 8 bits, then increment.
  NoOp = 29,             ///< Retrieve a reply (SPI).
  OtpLoad = 30,          ///< Load the OTP configuration.
  OtpBurn = 31,          ///< Permanent: not exposed by the driver.
  MemIsConfigured = 32,  ///< Is the selected external memory configured.
  MemIsConnected = 33,   ///< Is the selected external memory connected.
  FlashSendCmd = 36,     ///< Raw flash command: not exposed by the driver.
  FlashEraseSector = 37, ///< Erase one flash sector.
  MemIsBusy = 40,        ///< Is the external memory busy.
  BootstrapRs485 = 255,  ///< RS485 bootstrap: must be the first command.
};

/** @brief TMCL operation codes (Table 18 of the parameter mode manual). */
enum class Tmcl : uint8_t {
  Mst = 3,              ///< Motor stop.
  Sap = 5,              ///< Set axis parameter.
  Gap = 6,              ///< Get axis parameter.
  Stap = 7,             ///< Store axis parameters.
  Sgp = 9,              ///< Set global parameter.
  Ggp = 10,             ///< Get global parameter.
  Rfs = 13,             ///< Reference search.
  Sio = 14,             ///< Set I/O.
  Gio = 15,             ///< Get I/O.
  Calc = 19,            ///< Calculate with the accumulator.
  Comp = 20,            ///< Compare.
  Jc = 21,              ///< Jump conditional.
  Ja = 22,              ///< Jump absolute.
  Csub = 23,            ///< Call subroutine.
  Rsub = 24,            ///< Return from subroutine.
  Ei = 25,              ///< Enable interrupt.
  Di = 26,              ///< Disable interrupt.
  Wait = 27,            ///< Wait.
  Stop = 28,            ///< Stop the script.
  Calcx = 33,           ///< Calculate with the X register.
  Aap = 34,             ///< Accumulator to axis parameter.
  Agp = 35,             ///< Accumulator to global parameter.
  Cle = 36,             ///< Clear error flags.
  Vect = 37,            ///< Set interrupt vector.
  Reti = 38,            ///< Return from interrupt.
  Calcvv = 40,          ///< Calculate variable with variable.
  Calcva = 41,          ///< Calculate variable with accumulator.
  Calcav = 42,          ///< Calculate accumulator with variable.
  Calcvx = 43,          ///< Calculate variable with X.
  Calcxv = 44,          ///< Calculate X with variable.
  Calcv = 45,           ///< Calculate variable.
  Rst = 48,             ///< Restart.
  Djnz = 49,            ///< Decrement and jump if not zero.
  Siv = 55,             ///< Set indexed variable.
  Giv = 56,             ///< Get indexed variable.
  Aiv = 57,             ///< Accumulator to indexed variable.
  ApplStop = 128,       ///< Stop the script.
  ApplRun = 129,        ///< Run the script.
  ApplStep = 130,       ///< Single-step the script.
  ApplReset = 131,      ///< Reset the script.
  DownloadStart = 132,  ///< Start a script download.
  DownloadEnd = 133,    ///< End a script download.
  ReadMem = 134,        ///< Read script memory.
  GetStatus = 135,      ///< Script status.
  GetVersion = 136,     ///< Legacy version string: not used by the driver (ambiguity A11).
  FactoryDefault = 137, ///< Restore the factory defaults.
  Breakpoint = 141,     ///< Set a breakpoint.
  RamDebug = 142,       ///< RAMDebug access.
  GetInfo = 157,        ///< ID and version.
  Boot = 242,           ///< Return to the bootloader.
};

/** @brief TMCL reply status codes (Table 19). */
enum class TmclStatus : uint8_t {
  Ok = 100,                 ///< Command executed.
  CmdLoaded = 101,          ///< Command stored by a download.
  ChecksumError = 1,        ///< Checksum error in the request.
  InvalidCommand = 2,       ///< Unknown command.
  WrongType = 3,            ///< Type not valid.
  InvalidValue = 4,         ///< Value not valid.
  CmdNotAvailable = 6,      ///< Command not available.
  CmdLoadError = 7,         ///< Download error.
  MaxExceeded = 9,          ///< Limit exceeded.
  DownloadNotPossible = 10, ///< Download not possible.
};

/** @brief SPI status byte of a TMCL reply. */
enum class TmclSpiStatus : uint8_t {
  Ok = 0xFF,            ///< The reply is valid.
  ChecksumError = 0x00, ///< The chip received a bad checksum (also seen under load).
  FirstCommand = 0x0C,  ///< The first command since a reset.
  NotReady = 0xF0,      ///< The reply is not ready.
};

/** @brief RAMDebug types (Table 16). */
enum class RamDebugType : uint8_t {
  Init = 0,               ///< Initialise and reset.
  SampleCount = 1,        ///< Total number of samples.
  Prescaler = 3,          ///< Sampling divider minus one.
  SetChannel = 4,         ///< Set a channel.
  SetTriggerChannel = 5,  ///< Set the trigger source.
  TriggerMaskShift = 6,   ///< Mask and shift of the trigger value.
  EnableTrigger = 7,      ///< Start the measurement.
  GetState = 8,           ///< State: idle, trigger, capture, complete, pretrigger.
  ReadSample = 9,         ///< Read a sample by index.
  GetInfo = 10,           ///< General information.
  GetChannelType = 11,    ///< Channel type by index.
  GetChannelAddress = 12, ///< Channel address by index.
  SetPretrigger = 13,     ///< Total pretrigger samples.
  GetPretrigger = 14,     ///< Read the pretrigger sample count.
};

/** @brief `RFS` sub-commands. */
enum class ReferenceSearch : uint8_t {
  Start = 0,  ///< Start the search.
  Stop = 1,   ///< Stop the search.
  Status = 2, ///< Read the search status.
};

/** @brief Value of COMMUTATION_MODE (parameter 4) that leaves the gate driver off. */
inline constexpr int32_t kSystemOff = 0;
/** @brief The largest valid COMMUTATION_MODE value. */
inline constexpr int32_t kCommutationModeMax = 8;

/** @brief The chip ID every TMC9660 reports (bootloader CHIP_TYPE, register MCC_INFO_CHIP). */
inline constexpr uint32_t kChipId = 0x544D0001U;
/** @brief The system clock motor control needs, in Hz. */
inline constexpr uint32_t kMotorControlClockHz = 40'000'000U;
/** @brief The magic value of the `Boot` command. */
inline constexpr uint32_t kBootMagic = 0xA3B4C5D6U;

/**
 * @brief Where the type and the motor or bank bytes of a TMCL frame put their bits (ambiguity A2).
 *
 * The parameter manual numbers the UART bits least significant bit first (the sync bit is bit 0):
 * type bits 16 to 27, motor or bank bits 28 to 31, so the fourth byte reads `motor << 4 |
 * type[11:8]`
 * (`MotorHigh`). A vendor example is reported to pack it the other way (`TypeHigh`). The SPI table
 * (Table 3) numbers its bits most significant bit first, which read literally gives a third layout
 * (type[11:4] in byte 1). It is not offered: a chip that uses it decodes the probe frames as types
 * that do not exist (299 reads as 0x2B0), answers an error to both probes, and the driver refuses
 * to start (fail closed, nothing written); supporting it needs a code change (bench item A2). The
 * documents are not settled and nothing has run on a chip, so the driver never trusts a layout: at
 * the start of parameter mode it probes the layouts with two read-only commands and starts only
 * when exactly the configured one (or, for `Auto`, exactly one of the two) is answered. A wrong
 * layout would put a protection parameter (256 to 288) on the wire as another parameter, for
 * example 260 as COMMUTATION_MODE.
 */
enum class TmclLayout : uint8_t {
  Auto = 0,  ///< Probe both layouts; start only when exactly one answers.
  MotorHigh, ///< Byte 3 (SPI byte 2): `motor << 4 | type[11:8]`; the byte before holds `type[7:0]`.
  TypeHigh,  ///< Byte 3 (SPI byte 2): `type[11:8] << 4 | motor`; the byte before holds `type[7:0]`.
};

namespace detail {
/// `TmclLayout` is contiguous from 0: the last enumerator for `IsEnumerator()`.
template <>
struct EnumLast<TmclLayout> {
  /// The last enumerator.
  static constexpr TmclLayout kValue = TmclLayout::TypeHigh;
};
} // namespace detail

/**
 * @brief Model knobs: where the documents disagree, the driver reads one way by default and a flag
 * selects the other (docs/ambiguities.md).
 */
struct Quirks {
  bool crc8_reflect_output{
      false}; ///< A1: reflect the CRC-8 result, as the prose says. The examples do not.
  TmclLayout tmcl_layout{
      TmclLayout::Auto}; ///< A2: the TMCL type and motor bytes; probed at the start, never trusted.
  bool reg_spi_wide_offset{false}; ///< A3: 5-bit block, write bit 42, 10-bit offset.
  bool reg_uart_msb_fields{false}; ///< A4: `offset[10:8] << 5 | block` in byte 3.
  uint8_t ram_debug_index_bits{
      5}; ///< A17: width of the RAMDebug index in a register offset: 5 or 6.
};

/**
 * @brief Configuration of the driver. The defaults are the chip's.
 */
struct Config {
  Mode mode{Mode::Parameter}; ///< The protocol the chip is in at `EnsureInitialized()`.
  uint8_t device_address{1};  ///< UART device address (odd: its upper 7 bits are the TMCL address).
  uint8_t host_address{255};  ///< Host address in bootloader UART replies.
  uint8_t tmcl_host_address{2};      ///< Host address in TMCL and register-mode UART replies.
  uint32_t reply_timeout_us{50'000}; ///< How long the UART port waits for a reply.
  /// Leave PWM active in System off; without it `SetParam(IDLE_MOTOR_PWM_BEHAVIOR, 0)` is
  /// `Refused`. The stop of a port with no DRV_EN hook sets the parameter to 1 whatever this says.
  bool allow_pwm_in_system_off{false}; ///< See the lines above.
  uint32_t uart_resync_us{150'000};    ///< Quiet time after a failed UART exchange, before the
                                    ///< receive bytes are drained: the chip's discard gap of about
                                    ///< 100 ms (A6) with a 50 % margin.
  uint32_t boot_timeout_ms{500}; ///< Bound of every identity wait (start-up with `hardware_reset`,
                                 ///< a start, `Boot`): each failed poll is charged 10 ms and, on
                                 ///< UART, its reply wait, resync gap and drains (A9).
  uint32_t settle_us{20'000};    ///< Wait after a write that reconfigures the chip (A13).
  uint32_t spi_gap_us{1'000};    ///< Gap between the command and the reply-request on SPI.
  uint8_t spi_reply_retries{
      5};                 ///< Repeats of the reply-request on NOT_READY or CHECKSUM_ERROR (A5).
  bool uart_canary{true}; ///< Parameter mode over UART: detect an unseen reset (A7).
  uint8_t canary_variable{15};      ///< User variable that holds the canary.
  int32_t canary_value{0x5AC3A55A}; ///< The canary.
  bool hardware_reset{false}; ///< `EnsureInitialized()` pulses RSTN (needs the `GpioSet` hook)
                              ///< instead of adopting the chip.
  bool faultn_is_reset{
      false};      ///< `Service()` treats FAULTN low outside the bootloader as a reset (A18).
  Quirks quirks{}; ///< Readings of ambiguous documentation.
};

/**
 * @brief Check a `Config` without touching the bus.
 * @param c The configuration.
 * @return `Error::Ok` or `Error::InvalidArgument`.
 */
[[nodiscard]] constexpr Error Validate(const Config& c) noexcept {
  if ((c.device_address == 0U) || (c.host_address == 0U) || (c.tmcl_host_address == 0U)) {
    return Error::InvalidArgument;
  }
  if ((c.reply_timeout_us == 0U) || (c.boot_timeout_ms == 0U) || (c.spi_reply_retries > 50U)) {
    return Error::InvalidArgument;
  }
  if ((c.canary_variable > 15U) || (c.quirks.ram_debug_index_bits < 5U) ||
      (c.quirks.ram_debug_index_bits > 6U)) {
    return Error::InvalidArgument;
  }
  if (!detail::IsEnumerator(c.mode) || !detail::IsEnumerator(c.quirks.tmcl_layout)) {
    return Error::InvalidArgument;
  }
  return Error::Ok;
}

/**
 * @brief Reached only when a `CheckedConfig` is built from an invalid configuration: it is never
 * defined, so the build fails and names it (KB-CPP20-013).
 */
void ConfigIsInvalid() noexcept;

/**
 * @brief A configuration proven valid at build time.
 *
 * `constexpr CheckedConfig kBoard{Config{.device_address = 0}};` fails to compile, naming
 * `ConfigIsInvalid`, when `Validate()` rejects the configuration. Pass `kBoard.Get()` to the
 * driver; the driver still validates at start-up, for configurations built at run time.
 */
class CheckedConfig {
public:
  /**
   * @brief Check `config` at build time.
   * @param config The configuration; it must pass `Validate()`.
   */
  consteval explicit CheckedConfig(const Config& config) noexcept : config_(config) {
    if (Validate(config) != Error::Ok) {
      ConfigIsInvalid();
    }
  }
  /** @return The checked configuration. */
  [[nodiscard]] constexpr const Config& Get() const& noexcept {
    return config_;
  }
  /** @brief Not on a temporary: the reference would dangle (KB-CPP20-044). */
  void Get() const&& = delete;

private:
  Config config_;
};

/** @return The driver version, e.g. `"0.1.0"`, from the generated `tmc9660_version.h`. No I/O. */
constexpr const char* GetDriverVersion() noexcept {
  return TMC9660_VERSION_STRING;
}
/** @return The major version. No I/O. */
constexpr uint32_t GetDriverVersionMajor() noexcept {
  return TMC9660_VERSION_MAJOR;
}
/** @return The minor version. No I/O. */
constexpr uint32_t GetDriverVersionMinor() noexcept {
  return TMC9660_VERSION_MINOR;
}
/** @return The patch version. No I/O. */
constexpr uint32_t GetDriverVersionPatch() noexcept {
  return TMC9660_VERSION_PATCH;
}

/** @} */

} // namespace tmc9660

#endif // TMC9660_TYPES_HPP
