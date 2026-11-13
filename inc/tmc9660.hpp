/**
 * @file tmc9660.hpp
 * @brief TMC9660 driver: the one header to include.
 * @copyright Copyright (c) 2026 The driver-tmc9660 authors. SPDX-License-Identifier: MIT
 *
 * @details
 * `tmc9660::TMC9660<Port>` drives one TMC9660 servo controller and gate driver through a platform
 * port you write (see `tmc9660_spi_interface.hpp` and `tmc9660_uart_interface.hpp`). The driver has
 * no platform code, allocates nothing, throws nothing, takes no lock and is not thread-safe: give
 * each object one owner, or guard it with one mutex.
 */
#ifndef TMC9660_HPP
#define TMC9660_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>

#include "tmc9660_bootconfig.hpp"
#include "tmc9660_detail.hpp"
#include "tmc9660_frames.hpp"
#include "tmc9660_globals.hpp"
#include "tmc9660_params.hpp"
#include "tmc9660_registers.hpp"
#include "tmc9660_spi_interface.hpp"
#include "tmc9660_types.hpp"
#include "tmc9660_uart_interface.hpp"
#include "tmc9660_version.h"

namespace tmc9660 {

namespace detail {

// Optional hooks: a `Declares*` concept only tests the name, the `Has*` concept the exact
// signature, `noexcept` and the result type; the driver rejects a hook that is declared but does
// not match (KB-CPP20-007).

/// The port declares something called `GpioSet`.
template <typename Port>
concept DeclaresGpioSet =
    requires(Port& port) { port.GpioSet(CtrlPin::Reset, GpioSignal::ACTIVE); };

/// The port drives control pins: `bool GpioSet(CtrlPin, GpioSignal) noexcept`.
template <typename Port>
concept HasGpioSet =
    requires { ExactHook<Port, bool(CtrlPin, GpioSignal) noexcept>::Take(&Port::GpioSet); };

/// The port declares something called `GpioGet`.
template <typename Port>
concept DeclaresGpioGet =
    requires(Port& port, bool& level) { port.GpioGet(CtrlPin::Faultn, level); };

/// The port reads control pins: `bool GpioGet(CtrlPin, bool& high) noexcept`.
template <typename Port>
concept HasGpioGet =
    requires { ExactHook<Port, bool(CtrlPin, bool&) noexcept>::Take(&Port::GpioGet); };

/// The port declares something called `DelayUs`.
template <typename Port>
concept DeclaresDelayUs = requires(Port& port) { port.DelayUs(static_cast<uint32_t>(1)); };

/// The port waits: `void DelayUs(uint32_t us) noexcept`. A narrower parameter would truncate long
/// waits.
template <typename Port>
concept HasDelayUs = requires { ExactHook<Port, void(uint32_t) noexcept>::Take(&Port::DelayUs); };

/// The port declares something called `Log`.
template <typename Port>
concept DeclaresLog = requires(Port& port) { port.Log(LogLevel::Info, ""); };

/// The port takes log lines: `void Log(LogLevel, const char* message) noexcept`.
template <typename Port>
concept HasLog =
    requires { ExactHook<Port, void(LogLevel, const char*) noexcept>::Take(&Port::Log); };

/// Wait through the port. A port without `DelayUs()` fails to compile at the first call that needs
/// a wait: the chip's reply gap and boot times are requirements, never skipped.
template <typename Port>
void DelayUs(Port& port, uint32_t us) noexcept {
  if constexpr (HasDelayUs<Port>) {
    port.DelayUs(us);
  } else {
    static_assert(kAlwaysFalse<Port>,
                  "tmc9660: the port must implement `void DelayUs(uint32_t us) noexcept`");
  }
}

/// Log through the port, or nothing when the port has no `Log()`. The driver never formats text.
template <typename Port>
void Log(Port& port, LogLevel level, const char* message) noexcept {
  if constexpr (HasLog<Port>) {
    port.Log(level, message);
  } else {
    // No Log() hook: every parameter is used in this branch too (MISRA C++:2023 Rule 0.2.2).
    static_cast<void>(port);
    static_cast<void>(level);
    static_cast<void>(message);
  }
}

} // namespace detail

/** @brief Options of `TMC9660::BlWriteConfig()`. */
struct BlWriteOptions {
  bool allow_ldo_change{
      false}; ///< Allow a change of the LDO word: it changes VEXT1 and VEXT2 at once.
};

/** @brief Queries about the external memory of the bootloader. */
enum class MemQuery : uint8_t {
  Configured = 0, ///< MEM_IS_CONFIGURED.
  Connected,      ///< MEM_IS_CONNECTED.
  Busy,           ///< MEM_IS_BUSY.
};

/** @brief Power-down durations of `TMC9660::PowerDown()` (global parameter 11). */
enum class PowerDownTime : uint8_t {
  Ms250 = 0, ///< 250 ms.
  Ms500,     ///< 500 ms.
  S1,        ///< 1 s.
  S2,        ///< 2 s.
  S4,        ///< 4 s.
  S8,        ///< 8 s.
  S16,       ///< 16 s.
  S32,       ///< 32 s.
};

/** @brief One instruction of a script download. */
struct TmclInstruction {
  Tmcl op{Tmcl::Stop}; ///< Command number.
  uint16_t type{0};    ///< Type or parameter number (12 bits).
  uint8_t motor{0};    ///< Motor or bank (4 bits).
  int32_t value{0};    ///< Value.
};

/** @brief Gate-driver start-up of register mode: the words of the manual's start sequence. */
struct GateDriverRegConfig {
  uint32_t pwm_enable{
      0x0000'0F00U}; ///< MCC_PWM_CONFIG written first: ENABLE_UX1/VX2/WY1/Y2 and CHOP 1 are added.
  uint32_t gdrv_cfg{0};    ///< MCC_GDRV_CFG: gate current and adaptive mode.
  uint32_t gdrv_timing{0}; ///< MCC_GDRV_TIMING.
  uint32_t gdrv_bbm{0};    ///< MCC_GDRV_BBM: break-before-make times.
  uint32_t gdrv_hw{
      0x0300'0FFFU}; ///< MCC_GDRV_HW: bias, charge pump, comparators and bridge enables.
  uint8_t chop_run{
      7}; ///< MCC_PWM_CONFIG.CHOP that runs the outputs, 1 to 7; 2 holds the high sides on, 5 and 6
          ///< are single-side PWM (RM CHOP table): 7 is the normal run value.
  uint32_t gdrv_prot{0}; ///< MCC_GDRV_PROT, written before the start (the reset value is 0).
  /// MCC_GDRV_OCP_UVW, written before the start. 0 is the reset value: the lowest threshold, no
  /// blanking, no deglitch. Set the board's values: with every protection on, 0 can trip at once
  /// (`GateDriverStart()` then returns `DeviceFault` and leaves the stage off).
  uint32_t gdrv_ocp_uvw{0};
  uint32_t gdrv_ocp_y2{0}; ///< MCC_GDRV_OCP_Y2, as `gdrv_ocp_uvw`.
  uint32_t status_en{
      0xEFFF'FFFFU}; ///< MCC_GDRV_STATUS_EN after the start: fault reporting (reset value: all on).
  uint32_t prot_en{0xAFFF'FFFFU}; ///< MCC_GDRV_PROT_EN after the start: protection handling (all
                                  ///< on). 0 runs the stage unprotected: say so on purpose.
};

/** @brief Satisfied by a port that derives from exactly one of `SpiInterface` and `UartInterface`.
 */
template <typename Port>
concept AnyPort = SpiPort<Port> != UartPort<Port>;

/// The last parameter-mode reply as received (see `TMC9660::GetLastReply()`).
struct LastReply {
  uint8_t status{0}; ///< TMCL status code.
  uint8_t op{0};     ///< Echoed command number.
  uint32_t value{0}; ///< Reply value.
};

/**
 * @brief Driver for one TMC9660 on a platform port.
 *
 * Lifecycle: construct with the configuration (no I/O), then `EnsureInitialized()`, the one way to
 * start (bus up, optional reset, identity check in the mode the chip is expected to be in), use,
 * optionally `Deinitialize()` (the gate driver is switched off). Every call that touches the chip
 * returns `Error::NotInitialized` until `EnsureInitialized()` has succeeded, and again after the
 * driver has seen a reset.
 *
 * The chip speaks one of three protocols (`Mode`); a call that belongs to another mode returns
 * `Error::WrongMode` and sends nothing. Every call validates its arguments against the datasheet
 * tables before it sends a byte (`Error::InvalidArgument`, nothing sent).
 *
 * Optional port hooks, found at compile time:
 *
 * - `void DelayUs(uint32_t us) noexcept`: the SPI reply gap, reset and boot waits. Without it:
 * compile error, the waits are required.
 * - `bool GpioSet(CtrlPin, GpioSignal) noexcept`: RSTN (hardware reset), DRV_EN, WAKE. Without it:
 * no hardware reset, and `EnableDrive()` assumes DRV_EN is wired high.
 * - `bool GpioGet(CtrlPin, bool& high) noexcept`: FAULTN, for boot completion and `Service()`.
 * Without it: boot completion is waited for by time.
 * - `void Log(LogLevel, const char* message) noexcept`: one line per lifecycle event or failure.
 *   Without it: nothing is logged.
 *
 * A hook declared with the wrong signature is a compile error.
 *
 * @tparam Port Your class derived from `SpiInterface<Port>` or `UartInterface<Port>`, not both.
 */
template <AnyPort Port>
class TMC9660 {
  static_assert(!detail::DeclaresGpioSet<Port> || detail::HasGpioSet<Port>,
                "tmc9660: the optional GpioSet hook must be `bool GpioSet(CtrlPin, GpioSignal) "
                "noexcept`, so that a "
                "failed pin write is reported");
  static_assert(
      !detail::DeclaresGpioGet<Port> || detail::HasGpioGet<Port>,
      "tmc9660: the optional GpioGet hook must be `bool GpioGet(CtrlPin, bool& high) noexcept`");
  static_assert(
      !detail::DeclaresDelayUs<Port> || detail::HasDelayUs<Port>,
      "tmc9660: the DelayUs hook must be `void DelayUs(uint32_t us) noexcept`, so that no wait is "
      "truncated");
  static_assert(
      !detail::DeclaresLog<Port> || detail::HasLog<Port>,
      "tmc9660: the optional Log hook must be `void Log(LogLevel, const char* message) noexcept`");

public:
  /**
   * @brief Bind the driver to a port and store the configuration. No I/O, cannot fail.
   * @param port The port; it must outlive the driver.
   * @param config What `EnsureInitialized()` will apply; checked there, before any I/O.
   */
  explicit TMC9660(Port& port, const Config& config = {}) noexcept
      : port_(port), config_(config), mode_(config.mode), bl_app_mode_(config.mode) {}

  /** @brief One driver object per chip. */
  TMC9660(const TMC9660&) = delete;
  /** @brief One driver object per chip. */
  TMC9660& operator=(const TMC9660&) = delete;

  /// @name Lifecycle
  /// @{

  /**
   * @brief Start the driver, once: bus up, optional hardware reset, identity check in
   * `Config::mode`.
   * @return `Ok`; `InvalidArgument` (nothing sent) for a bad `Config` or a device address that is
   * even when the UART carries TMCL or register mode; `Bus`; `Timeout`; `DeviceIdMismatch` when the
   * chip ID is not the TMC9660's.
   */
  Result<void> EnsureInitialized() noexcept;

  /**
   * @brief Switch the gate driver off and mark the driver uninitialised. The chip is left as it is.
   *
   * The stop is sent whether or not the driver is initialised: after a reported reset
   * (`UnexpectedReset`) the DRV_EN pin is still high and the chip may run an application of its
   * own, so this call drops the pin and sends the stop frames then too (`DisableDrive()`).
   * @return The first error of the switch-off.
   */
  Result<void> Deinitialize() noexcept;

  /** @return true after `EnsureInitialized()` succeeded and until a reset is detected or
   * `Deinitialize()` runs. */
  [[nodiscard]] bool IsInitialized() const noexcept {
    return initialized_;
  }
  /**
   * @return The protocol the chip is in, as far as the driver knows: `Config::mode` until a start
   * or a mode change proves another one; a failed start keeps the last one proved (the stop uses
   * it).
   */
  [[nodiscard]] Mode GetMode() const noexcept {
    return mode_;
  }
  /** @return The status byte of the last reply the chip sent: a `BlStatus` or a `TmclStatus` value.
   */
  [[nodiscard]] uint8_t LastStatus() const noexcept {
    return last_status_;
  }
  /** @brief The last parameter-mode reply, as the chip sent it (no bus traffic).
   * @return The status, the echoed command number and the 32-bit value of the last TMCL reply
   * whose frame was valid; all zero before the first one. */
  [[nodiscard]] LastReply GetLastReply() const noexcept {
    return {.status = last_status_tmcl_, .op = last_op_, .value = last_value_};
  }
  /** @return The configuration the driver uses. */
  [[nodiscard]] const Config& GetConfig() const noexcept {
    return config_;
  }

  /**
   * @brief Poll the things that need no request: FAULTN when the port reads it, and in parameter
   * mode a read of GENERAL_ERROR_FLAGS that also keeps the heartbeat alive.
   * @return `Ok`; `UnexpectedReset` when `Config::faultn_is_reset` and FAULTN is low outside the
   * bootloader, or when the flag word shows WATCHDOG_EVENT that was clear at the start (a watchdog
   * reset; a reset that leaves no flag is found by the canary or FIRST_CMD instead).
   */
  Result<void> Service() noexcept;

  /// @}

  /// @name Safety
  /// @{

  /**
   * @brief Switch the gate driver off. Never refused: whatever the mode, state or earlier error,
   * every step is attempted, the DRV_EN pin first.
   *
   * Parameter mode writes COMMUTATION_MODE = 0, once more after any failure (a reported reset, a
   * lost frame, a stale or late UART reply; the repeat resynchronises the line first), and without
   * a DRV_EN hook then makes sure IDLE_MOTOR_PWM_BEHAVIOR is 1; register mode writes zero to
   * MCC_GDRV_HW and MCC_PWM_CONFIG, always both, and reads both back: `VerifyFailed` when an
   * energising bit is still set (a stop the chip did not apply, for example under the wrong
   * register SPI layout, A3). In bootloader mode nothing needs switching off: with a DRV_EN hook
   * the pin drop is the stop; without one the stop is `Ok` only when the bootloader still answers
   * its chip type. BUSY, an error status or a seen reset are the bootloader answering: the error is
   * returned, no application stop is sent and the mode stays. Any other failure may be an unseen
   * restart into an application: the driver sends the stop of the application that BOOT_MODE names
   * (read at adoption, before anything was written, so it is the OTP value; or by the last
   * `BlReadConfig()`), else of `Config::mode`, becomes uninitialised in that protocol and returns
   * the error; with no proved protocol and no hook it is `NotInitialized`.
   * @return The first error met; `Ok` when every step worked.
   */
  Result<void> DisableDrive() noexcept;

  /**
   * @brief Energise the gate driver in parameter mode, after the start-up guards.
   *
   * Guards, in order, all before DRV_EN or COMMUTATION_MODE is touched: mode is `Parameter`,
   * `commutation_mode` is 1 to 8, GENERAL_ERROR_FLAGS and GDRV_ERROR_FLAGS read zero. Then DRV_EN
   * is asserted (when the port drives it) and COMMUTATION_MODE written; if that write fails (the
   * chip may still have run it) the full stop of `DisableDrive()` follows, so a failed call leaves
   * the drive stopped also without a DRV_EN hook.
   * @param commutation_mode COMMUTATION_MODE value, 1 to 8.
   * @return `Ok`; `InvalidArgument`; `NotInitialized`; `WrongMode`; `DeviceFault` (a flag is
   * latched: read and clear it); the bus errors.
   */
  Result<void> EnableDrive(uint8_t commutation_mode) noexcept;

  /**
   * @brief Stop the motor (MST). Parameter mode. Sent before a UART resync, and once more (after
   * the resync) when the first attempt fails.
   * @return `Ok` or the error of the repeat.
   */
  Result<void> MotorStop() noexcept;

  /// @}

  /// @name Bootloader
  /// @{

  /** @brief GET_INFO. @param info The selector. @return The value. */
  Result<uint32_t> BlGetInfo(BlInfo info) noexcept;
  /**
   * @brief Read a 32-bit word of a bootloader memory bank.
   * @param bank The bank. @param address Byte address (a multiple of 4).
   * @return The word; `InvalidArgument` for a misaligned address.
   */
  Result<uint32_t> BlRead32(Bank bank, uint32_t address) noexcept;
  /**
   * @brief Write a 32-bit word of an external memory bank (SPI flash or I2C EEPROM).
   * @param bank `SpiFlash` or `I2cEeprom`: the OTP bank is never written (`Refused`), the
   * configuration memory has its own call (`BlWriteConfig`, `Refused`).
   * @param address Byte address (a multiple of 4). @param value The word.
   * @return `Ok`, `InvalidArgument`, `Refused`, `WrongMode`, the bus errors.
   */
  Result<void> BlWrite32(Bank bank, uint32_t address, uint32_t value) noexcept;
  /** @brief Read the 64-byte configuration memory. @return The configuration. */
  Result<BootConfig> BlReadConfig() noexcept;
  /**
   * @brief Write a configuration and verify it.
   *
   * Refused before anything is written when `config.CheckStart()` fails (the application would
   * start with a reserved BOOT_MODE or a clock other than 40 MHz) or when the LDO word changes
   * without `options.allow_ldo_change`. Only the words that differ are written; the clock word with
   * one 32-bit write; the BOOT_MODE word last. A write of the BOOT_MODE word with START_MOTOR_CTRL
   * set starts the application (A24): the driver switches to the new mode and waits up to
   * `Config::boot_timeout_ms` for its chip ID. If the start write fails other than by a bootloader
   * error status, the start is followed all the same. End states on failure: the application is
   * adopted (`Ok`); the bootloader still answers, so the driver stays in it, initialised, and
   * returns the error; or the driver is left uninitialised in the application's mode, so a stop
   * reaches it.
   * @param config The configuration. @param options Permissions for changes that affect the board.
   * @return `Ok`, `Refused`, `VerifyFailed`, `Timeout`, `DeviceIdMismatch`, the bus errors.
   */
  Result<void> BlWriteConfig(const BootConfig& config, const BlWriteOptions& options = {}) noexcept;
  /**
   * @brief Read the configuration, set BOOT_MODE and START_MOTOR_CTRL and write it
   * (`BlWriteConfig`).
   * @param target `Register` or `Parameter`; the reserved values are `Refused`.
   * @return As `BlWriteConfig`.
   */
  Result<void> BlStartApplication(BootMode target) noexcept;
  /**
   * @brief Ask about an external memory.
   * @param bank `SpiFlash` or `I2cEeprom`. @param query What to ask.
   * @return The answer.
   */
  Result<bool> BlMemoryStatus(Bank bank, MemQuery query) noexcept;
  /**
   * @brief Erase one 4 KiB sector of the SPI flash (SECTOR_ERASE 0x20). Refused unless the flash is
   * configured and connected, and refused for sector 0, which holds the partition descriptor,
   * unless `allow_partition_table`.
   * @param address The first address of the sector: a multiple of 4096 below 2^24 (the flash takes
   * a 24-bit address); anything else is `InvalidArgument` and nothing is sent.
   * @param allow_partition_table Allow sector 0 (the partition descriptor).
   * @return `Ok`, `Refused`, `InvalidArgument`, `Busy`, bus errors.
   */
  Result<void> BlEraseFlashSector(uint32_t address, bool allow_partition_table = false) noexcept;
  /**
   * @brief OTP_LOAD: load the OTP configuration. (OTP_BURN is not exposed: it is permanent.)
   * @return `Ok`, `WrongMode`, the transport errors.
   */
  Result<void> BlOtpLoad() noexcept;

  /// @}

  /// @name Parameter mode
  /// @{

  /**
   * @brief GetInfo type 0: the chip ID.
   * @return The chip ID, `WrongMode` outside parameter mode, the transport errors.
   */
  Result<uint32_t> GetChipId() noexcept;
  /**
   * @brief GetInfo type 1: the version word.
   * @return The version word, `WrongMode` outside parameter mode, the transport errors.
   */
  Result<uint32_t> GetVersion() noexcept;
  /**
   * @brief GAP: read an axis parameter.
   * @param param The parameter. @param motor The axis, 0.
   * @return The value, sign-extended when the parameter's range is signed. `InvalidArgument` for a
   * write-only parameter or a motor above 15.
   */
  Result<int64_t> GetParam(Param param, uint8_t motor = 0) noexcept;
  /**
   * @brief Single-shot raw read: one GAP with a parameter number taken as given (not looked up in
   * the table, so a number the table does not know can be read). Nothing is written to the chip,
   * so this cannot energise anything.
   * @param type The 12-bit parameter number. @param motor The axis, 0.
   * @return The 32-bit reply value unsigned and uninterpreted, `InvalidArgument` (nothing sent)
   * for a number above 0xFFF or a motor above 15, `WrongMode` outside parameter mode.
   */
  Result<uint32_t> ReadRawParam(uint16_t type, uint8_t motor = 0) noexcept;
  /**
   * @brief SAP: write an axis parameter, after checking access, range and coded set against
   * Table 57.
   *
   * Refused: COMMUTATION_MODE other than 0 (use `EnableDrive()`), and MOTOR_TYPE while
   * COMMUTATION_MODE is not 0.
   * @param param The parameter. @param value The value, checked against the table. @param motor The
   * axis, 0.
   * @return `Ok`, `InvalidArgument` (nothing sent), `Refused`, `Nack`, the transport errors.
   */
  Result<void> SetParam(Param param, int64_t value, uint8_t motor = 0) noexcept;
  /**
   * @brief SAP of a flag word: write 1 to the bits to clear.
   * @param flags One of the flag parameters.
   * @param mask The bits to clear.
   * @return `Ok`, `InvalidArgument` (not a flag parameter, nothing sent), the transport errors.
   */
  Result<void> ClearFlags(Param flags, uint32_t mask) noexcept;
  /**
   * @brief STAP: store all storable parameters to the external memory.
   * @return `Ok`, `WrongMode`, `Nack`, the transport errors.
   */
  Result<void> StoreParams() noexcept;
  /**
   * @brief FactoryDefault: clears the settings stored in the external memory.
   * @param confirm Must be true: the call is `Refused` otherwise. COMMUTATION_MODE must be 0, else
   * `Refused`.
   * @return `Ok`, `Refused` (nothing changed), the transport errors.
   */
  Result<void> FactoryDefault(bool confirm) noexcept;
  /** @brief GGP. @param bank 0, 2 or 3. @param number The parameter number. @return The value. */
  Result<int64_t> GetGlobal(GlobalBank bank, uint16_t number) noexcept;
  /**
   * @brief SGP, checked against Tables 61 to 63.
   * @param bank The bank.
   * @param number The parameter number in the bank.
   * @param value The value, checked against the table.
   * @return `Ok`, `InvalidArgument` (nothing sent), `Nack`, the transport errors.
   */
  Result<void> SetGlobal(GlobalBank bank, uint16_t number, int64_t value) noexcept;
  /**
   * @brief SetParam of TARGET_TORQUE.
   * @param value The target.
   * @return As `SetParam()`.
   */
  Result<void> SetTargetTorque(int32_t value) noexcept {
    return SetParam(Param::TARGET_TORQUE, value);
  }
  /**
   * @brief SetParam of TARGET_VELOCITY.
   * @param value The target.
   * @return As `SetParam()`.
   */
  Result<void> SetTargetVelocity(int32_t value) noexcept {
    return SetParam(Param::TARGET_VELOCITY, value);
  }
  /**
   * @brief SetParam of TARGET_POSITION.
   * @param value The target.
   * @return As `SetParam()`.
   */
  Result<void> SetTargetPosition(int32_t value) noexcept {
    return SetParam(Param::TARGET_POSITION, value);
  }
  /** @brief RFS. @param action Start, stop or status. @return The reply value (the status for
   * `Status`). */
  Result<int32_t> ReferenceSearchCommand(ReferenceSearch action) noexcept;
  /**
   * @brief SIO: set a digital output.
   * @param pin Port number 0 to 18.
   * @param high The level.
   * @return `Ok`, `InvalidArgument` (nothing sent), the transport errors.
   */
  Result<void> SetIo(uint8_t pin, bool high) noexcept;
  /** @brief GIO. @param pin Port number 0 to 18. @param analog true: the analog value (bank 1).
   * @return The value. */
  Result<int32_t> GetIo(uint8_t pin, bool analog) noexcept;
  /**
   * @brief RAMDebug command.
   * @param type Table 16 type.
   * @param motor_bank The motor/bank field.
   * @param value The value.
   * @return The reply value, `InvalidArgument` (nothing sent), the transport errors.
   */
  Result<int32_t> RamDebug(RamDebugType type, uint8_t motor_bank, int32_t value) noexcept;
  /**
   * @brief ApplRun: run the script. The chip executes what is in its script memory, a script stored
   * earlier or one that was not downloaded through this driver included, and the driver cannot see
   * it: such a script may set COMMUTATION_MODE without `EnableDrive()`'s guards (see
   * docs/safety.md). Only `ScriptDownload()` screens what it loads.
   * @param from_address true: start at `address`.
   * @param address The start address.
   * @return `Ok`, `WrongMode`, `UnexpectedReset`, the transport errors.
   */
  Result<void> ScriptRun(bool from_address, uint32_t address) noexcept;
  /**
   * @brief ApplStop.
   * @return `Ok`, `WrongMode`, the transport errors.
   */
  Result<void> ScriptStop() noexcept;
  /**
   * @brief ApplStep.
   * @return `Ok`, `WrongMode`, the transport errors.
   */
  Result<void> ScriptStep() noexcept;
  /**
   * @brief ApplReset.
   * @return `Ok`, `WrongMode`, the transport errors.
   */
  Result<void> ScriptReset() noexcept;
  /**
   * @brief GetStatus of the script.
   * @return The state number, `WrongMode`, the transport errors.
   */
  Result<int32_t> ScriptStatus() noexcept;
  /**
   * @brief ReadMem: the script instruction word at `address`.
   * @param address The word address.
   * @return The word, `WrongMode`, the transport errors.
   */
  Result<uint32_t> ScriptReadMem(uint32_t address) noexcept;
  /**
   * @brief Download a script: DownloadStart, every instruction (each acknowledged with CMD_LOADED),
   * DownloadEnd. DownloadEnd is sent even when an instruction failed. The chip executes scripts on
   * its own. The one way a script of this driver could energise the stage, setting
   * COMMUTATION_MODE (SAP of parameter 4 with a non-zero value, or AAP of it, whose value is the
   * accumulator), is `Refused` before anything is sent; nothing else a script does is analysed.
   * @param script The instructions; at most 255, each one a script-capable command.
   * @return `Ok`, `InvalidArgument` or `Refused` (nothing sent), the error of the first failing
   * instruction.
   */
  Result<void> ScriptDownload(std::span<const TmclInstruction> script) noexcept;
  /**
   * @brief Put the chip into the power-down state for a time (global parameter 11). The gate driver
   * is switched off first; the chip restarts afterwards, so the driver becomes uninitialised.
   * @param time Duration.
   * @return `Ok`, `InvalidArgument` (nothing sent), the transport errors.
   */
  Result<void> PowerDown(PowerDownTime time) noexcept;
  /**
   * @brief Return to the bootloader (Boot command). The gate driver is switched off and
   * COMMUTATION_MODE is read back as 0 first (erratum 3); then Boot is sent; then the bootloader's
   * chip ID is checked.
   * @return `Ok`, `Refused` (the system is not off), the transport errors.
   */
  Result<void> ReturnToBootloader() noexcept;
  /**
   * @brief Erratum 5 workaround: disable the SPI1 slave (op 146, type 4, bank 9, value 0). Only
   * call it when the host is not connected to SPI1.
   * @return `Ok`, `WrongMode`, the transport errors.
   */
  Result<void> DisableSpi1Slave() noexcept;

  /// @}

  /// @name Register mode
  /// @{

  /**
   * @brief Read a register of the register map.
   * @param addr The register. @return The value; `InvalidArgument` for an address the map does not
   * list; `Unsupported` when the offset does not fit the SPI layout (ambiguity A3).
   */
  Result<uint32_t> ReadReg(RegAddr addr) noexcept;
  /**
   * @brief Write a register. Refused (nothing sent): read-only registers (`InvalidArgument`) and
   * any write that sets a bit that energises the gate driver (MCC_GDRV_HW and MCC_PWM_CONFIG enable
   * bits): use `GateDriverStart()`.
   * @param addr The register.
   * @param value The new value.
   * @return `Ok`, `InvalidArgument`, `Refused`, `UnexpectedReset`, the transport errors.
   */
  Result<void> WriteReg(RegAddr addr, uint32_t value) noexcept;
  /**
   * @brief Read-modify-write of the bits in `mask`; the mask must lie within the register's
   * writable bits.
   * @param addr The register.
   * @param mask The bits to change.
   * @param value The new value of those bits.
   * @return As `WriteReg()`; `InvalidArgument` for a mask outside the writable bits.
   */
  Result<void> UpdateReg(RegAddr addr, uint32_t mask, uint32_t value) noexcept;
  /**
   * @brief The value of one field.
   * @tparam F A `fields::<REGISTER>::<FIELD>` type.
   * @param addr The register that holds the field.
   * @return The field value, or the error of `ReadReg()`.
   */
  template <typename F>
  Result<uint32_t> ReadField(RegAddr addr) noexcept {
    const Result<uint32_t> r = ReadReg(addr);
    return r.ok() ? Result<uint32_t>::Success(F::Extract(r.value)) : r;
  }
  /**
   * @brief Set one field.
   * @tparam F A `fields::<REGISTER>::<FIELD>` type.
   * @param addr The register that holds the field.
   * @param value The new field value; `InvalidArgument` when it does not fit.
   * @return As `UpdateReg()`.
   */
  template <typename F>
  Result<void> WriteField(RegAddr addr, uint32_t value) noexcept {
    if (value > F::kMax) {
      return Result<void>::Failure(Error::InvalidArgument);
    }
    return UpdateReg(addr, F::kMask, F::Insert(0U, value));
  }
  /**
   * @brief RAMDebug access in register mode.
   * @param subcommand RAMDebug subcommand 0 to 15. @param index Index (5 or 6 bits,
   * `Quirks::ram_debug_index_bits`).
   * @param write true to write.
   * @param value The data.
   * @return The data read back, `InvalidArgument` (nothing sent), the transport errors.
   */
  Result<uint32_t> RegRamDebug(uint8_t subcommand, uint8_t index, bool write,
                               uint32_t value) noexcept;
  /**
   * @brief Energise the gate driver in register mode, in the order of the register manual, with the
   * guards complete before the first energising write: the sequence sets up PWM, gate-driver
   * configuration, timing and break-before- make, clears the status, reads MCC_GDRV_STATUS and
   * MCC_GDRV_FAULT and stops on any latched bit, asserts DRV_EN and only then writes MCC_GDRV_HW
   * and the run value of CHOP. The result is read back; a mismatch switches the gate driver off
   * again.
   * @param config The words.
   * @return `Ok`, `InvalidArgument` (bits outside the registers' writable bits), `DeviceFault`,
   * `VerifyFailed`, `WrongMode`, the bus errors.
   */
  Result<void> GateDriverStart(const GateDriverRegConfig& config) noexcept;

  /// @}

private:
  static constexpr bool kSpi = SpiPort<Port>;

  // Transports.
  Result<detail::BlReply> blExchange(BlCmd cmd, uint32_t value, bool write) noexcept;
  Result<detail::BlReply> blSpiTransfer(BlCmd cmd, uint32_t value) noexcept;
  Error blTransport(BlCmd cmd, uint32_t value, bool write, detail::BlReply& reply) noexcept;
  Error tmclTransport(const detail::TmclCommand& c, bool probe, detail::TmclReply& reply) noexcept;
  Error blExchangeSpi(BlCmd cmd, uint32_t value, bool write, detail::BlReply& reply) noexcept;
  Error blExchangeUart(BlCmd cmd, uint32_t value, detail::BlReply& reply) noexcept;
  Error tmclRawSpi(const detail::TmclCommand& c, bool probe, detail::TmclReply& reply) noexcept;
  Error tmclRawUart(const detail::TmclCommand& c, detail::TmclReply& reply) noexcept;
  Error tmclSpiFetch(detail::TmclReply& reply) noexcept;
  Error spiFirstCommand() noexcept;
  Result<uint32_t> regSpiExchange(uint8_t block, uint16_t offset, bool write, uint32_t data,
                                  bool probe) noexcept;
  Result<uint32_t> regUartExchange(uint8_t command, uint8_t block, uint16_t offset, uint32_t data,
                                   bool probe) noexcept;
  Error regSpiDatagram(const std::array<uint8_t, detail::kRegSpiLen>& req,
                       detail::RegSpiReply& out) noexcept;
  Result<detail::TmclReply> tmcl(uint8_t op, uint16_t type, uint8_t motor, uint32_t value,
                                 bool write, bool accept_loaded) noexcept;
  Result<detail::TmclReply> tmclRaw(const detail::TmclCommand& c, bool probe,
                                    bool accept_loaded) noexcept;
  Result<uint32_t> regExchange(bool write, RegAddr addr, uint32_t data) noexcept;
  Result<uint32_t> regExchangeRaw(uint8_t command, uint8_t block, uint16_t offset, bool write,
                                  uint32_t data, bool probe) noexcept;
  Error checkRegSpiReply(const detail::RegSpiReply& r) noexcept;
  Error spiTransfer(std::size_t len) noexcept;
  Error uartExchange(std::size_t tx_len, std::size_t rx_len) noexcept;
  Error mapBusStatus(BusStatus s) const noexcept;

  // Helpers.
  Result<void> fail(Error e, const char* message) noexcept;
  Error need(Mode m) const noexcept;
  void markReset() noexcept;
  Result<void> verifyIdentity() noexcept;
  Result<void> waitForIdentity() noexcept;
  Result<void> regBaseline() noexcept;
  Result<void> setDrvEn(GpioSignal s) noexcept;
  Result<void> blSetup(Bank bank, uint32_t address) noexcept;
  Result<uint32_t> blReadWord(uint32_t address) noexcept;
  Result<void> blWriteWord(uint32_t address, uint32_t value) noexcept;
  Result<void> disableDriveParameter() noexcept;
  Result<void> startParameterMode() noexcept;
  Result<void> verifyTmclLayout() noexcept;
  Result<bool> probeTmclLayout(TmclLayout layout) noexcept;
  Error uartResync() noexcept;
  Error faultnGuard() noexcept;
  Result<void> blSetBank(Bank bank) noexcept;
  Result<void> blLiveness(uint32_t value) noexcept;
  Result<void> disableDriveRegister() noexcept;
  Result<void> idleSafe(const detail::TmclCommand& stop) noexcept;
  Result<void> startPreconditions() noexcept;
  Result<void> hardwareReset() noexcept;
  Result<void> blWaitPll(const BootConfig& config, std::size_t index) noexcept;
  Result<void> blWriteChanged(const BootConfig& old, const BootConfig& config) noexcept;
  Result<void> blVerifyConfig(const BootConfig& config, bool start) noexcept;
  Result<void> blFollowStart(const BootConfig& config) noexcept;
  Result<void> blStartWrite(const BootConfig& config) noexcept;
  void blStopUnknownApplication() noexcept;
  Result<void> blWriteBootMode(const BootConfig& config, bool start) noexcept;
  Result<void> regPut(RegAddr addr, uint32_t value) noexcept;
  Result<bool> gdrvLatched() noexcept;
  Result<void> gdrvSetup(const GateDriverRegConfig& config) noexcept;
  Result<void> gdrvEnergise(const GateDriverRegConfig& config) noexcept;
  Result<void> gdrvProtect(const GateDriverRegConfig& config) noexcept;
  Result<void> armCanary() noexcept;
  Result<int64_t> readParamRaw(Param p, uint8_t motor) noexcept;
  Error energisingGuard(RegAddr addr, uint32_t value) const noexcept;

  Port& port_;
  Config config_;
  Mode mode_{Mode::Bootloader};
  bool initialized_{false};
  bool canary_armed_{false};
  bool reg_started_{false};
  bool defer_resync_{false}; ///< The next UART exchange is a stop: send it before any resync.
  bool uart_dirty_{false};   ///< A UART exchange failed: drain the receive bytes before the next.
  bool watchdog_at_start_{false}; ///< WATCHDOG_EVENT was already set when parameter mode started.
  TmclLayout layout_{TmclLayout::MotorHigh}; ///< The TMCL layout the start-up probe verified.
  Mode bl_app_mode_{
      Mode::Bootloader};       ///< The application the last BOOT_MODE read names, or
                               ///< `Config::mode`: the stop's guess after an unseen restart.
  Bank bl_bank_{Bank::Config}; ///< The bootloader bank the last SetBank selected.
  uint8_t reg_counter_{0};
  uint8_t last_status_{0};
  uint8_t last_status_tmcl_{0};
  uint8_t last_op_{0};
  uint32_t last_value_{0};
  std::array<uint8_t, 9> tx_{};
  std::array<uint8_t, 9> rx_{};
};

} // namespace tmc9660

// codeql::misra_deviation_next_line(MD-CPP-009)
#include "../src/tmc9660.ipp" // IWYU pragma: export

#endif // TMC9660_HPP
