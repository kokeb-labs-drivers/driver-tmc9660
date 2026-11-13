/**
 * @file fuzz_tmc9660.cpp
 * @brief Fuzz target: the driver over a bus whose behaviour the fuzzer controls.
 *
 * What the driver reads comes from the chip over the bus, so read data is the untrusted input. The
 * fuzzer's bytes choose the bus, the mode the chip starts in and a configuration, then a sequence
 * of driver calls. On every transfer they also choose what the bus does: pass it to the chip model,
 * fail it, flip bits of the reply or replace it. Between calls they can reset the chip, latch error
 * flags, change the knobs of the model and let an application energise by itself.
 *
 * After every call the target checks the driver's promises (docs/safety.md) and aborts, naming the
 * line, when one breaks:
 *
 * - a failed call returns an `Error` the enum defines, and `T{}` as the value;
 * - `NotInitialized`, `InvalidArgument` and `WrongMode` mean nothing was sent;
 * - `UnexpectedReset` leaves the driver uninitialised;
 * - after an unseen reset, in the modes that can detect it, no state-changing command reaches the
 * chip until the driver has noticed and started again (the mode is the one the driver is in at
 * that call: `BlStartApplication()` and `ReturnToBootloader()` change it and adopt the new
 * application as a fresh start, and register-mode and bootloader UART have no reset evidence,
 * A18);
 * - the model never sees a gate driver energised with an error flag latched or with DRV_EN low, a
 * Boot command with the system on, a reserved-mode start, an OTP write or an all-zero TMCL frame;
 * - `Deinitialize()` and `DisableDrive()` always try to drop DRV_EN, initialised or not (a chip
 *   reset does not change the host-driven pin: the model leaves it alone), when the port has the
 *   hook; the fuzzer also picks ports without `GpioSet()`, where DRV_EN is wired high;
 * - a stop that returns `Ok` in parameter mode leaves the PWM off (or DRV_EN low), and an
 *   `EnableDrive()` that fails with at most one lost transfer and no corrupted reply leaves it off
 *   too;
 * - with no corrupted reply, a successful `GetParam()` returns what the chip holds, even when a
 *   reply arrived late and the port does not flush its receive bytes (the UART resync).
 *
 * Built two ways (tests/fuzz/CMakeLists.txt): with libFuzzer (Clang, `-DTMC9660_BUILD_FUZZERS=ON`),
 * and as a plain executable that replays the corpus in tests/fuzz/corpus/ as a CTest on any
 * compiler.
 */
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <type_traits>

#include "mock_tmc9660.hpp"
#include "tmc9660.hpp"

namespace {

using tmc9660::Error;
using tmc9660_test::Phase;
using tmc9660_test::Tmc9660Model;

/// The fuzzer's bytes, read front to back; zeros once they run out.
class Input {
public:
  Input(const uint8_t* data, std::size_t size) noexcept : data_(data), size_(size) {}

  [[nodiscard]] bool Empty() const noexcept {
    return pos_ >= size_;
  }
  uint8_t Byte() noexcept {
    if (pos_ < size_) {
      const uint8_t b = data_[pos_];
      ++pos_;
      return b;
    }
    return 0U;
  }
  uint32_t Word() noexcept {
    uint32_t v = 0U;
    for (int i = 0; i < 4; ++i) {
      v = (v << 8U) | Byte();
    }
    return v;
  }

private:
  const uint8_t* data_;
  std::size_t size_;
  std::size_t pos_ = 0;
};

/// A broken promise: name the line and abort, so libFuzzer saves the input as a crash.
void Check(bool promise, int line = __builtin_LINE()) noexcept {
  if (!promise) {
    static_cast<void>(std::fprintf(stderr, "fuzz_tmc9660: promise broken at line %d\n", line));
    std::abort();
  }
}

constexpr uint8_t kFail = 0xFFU;    ///< The transfer fails and never reaches the chip.
constexpr uint8_t kFlip = 0xFEU;    ///< The reply has bits flipped.
constexpr uint8_t kReplace = 0xFDU; ///< The reply is replaced.

/// The chip model and the bus behaviour shared by both ports.
class Core {
public:
  explicit Core(Input& in) noexcept : in_(in) {}

  Tmc9660Model chip;
  tmc9660_test::UartWire wire;     ///< The UART receive FIFO: late and stale replies.
  int drv_en_inactive_attempts{0}; ///< GpioSet(DrvEn, INACTIVE) calls, whatever they returned.
  bool tampered_any{false};        ///< A reply was flipped or replaced at some point.
  bool tampered_since_reset{
      false}; ///< Noise may have hidden the reset evidence (a documented limit).
  int transfers{0};
  int failures{0}; ///< Transfers and pin writes the fuzzer failed.
  int tampers{0};  ///< Replies the fuzzer flipped or replaced.
  bool faultn_high{true};

  uint8_t NextAction() noexcept {
    return in_.Byte();
  }

  void Tamper(uint8_t action, uint8_t* rx, std::size_t len) noexcept {
    if ((action == kFlip) || (action == kReplace)) {
      tampered_since_reset = true;
      tampered_any = true;
      ++tampers;
    }
    if (action == kFlip) {
      const uint8_t mask = in_.Byte();
      for (std::size_t i = 0U; i < len; ++i) {
        rx[i] = static_cast<uint8_t>(rx[i] ^ (((i % 2U) == 0U) ? mask : 0U));
      }
    } else if (action == kReplace) {
      for (std::size_t i = 0U; i < len; ++i) {
        rx[i] = in_.Byte();
      }
    } else {
      // passed through
    }
  }

  bool Pin(tmc9660::CtrlPin pin, tmc9660::GpioSignal signal) noexcept {
    if ((pin == tmc9660::CtrlPin::DrvEn) && (signal == tmc9660::GpioSignal::INACTIVE)) {
      ++drv_en_inactive_attempts;
    }
    if (NextAction() == kFail) {
      ++failures;
      return false;
    }
    const bool active = (signal == tmc9660::GpioSignal::ACTIVE);
    if (pin == tmc9660::CtrlPin::DrvEn) {
      chip.SetDrvEn(active);
    } else if (pin == tmc9660::CtrlPin::Reset) {
      chip.ResetPin(active);
    } else {
      // Wake is not modelled.
    }
    return true;
  }

private:
  Input& in_;
};

/// `Pins`: the port has `GpioSet()`; without it DRV_EN is wired high and RSTN is not driven.
template <bool Pins>
class FuzzSpi : public tmc9660::SpiInterface<FuzzSpi<Pins>> {
public:
  explicit FuzzSpi(Core& core) noexcept : core_(core) {}
  bool EnsureInitialized() noexcept {
    return true;
  }
  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    Check((tx != nullptr) && (rx != nullptr) && (len >= 5U) && (len <= 8U));
    ++core_.transfers;
    const uint8_t action = core_.NextAction();
    if (action == kFail) {
      ++core_.failures;
      return false;
    }
    core_.chip.SpiTransfer(tx, rx, len);
    core_.Tamper(action, rx, len);
    return true;
  }
  void DelayUs(uint32_t us) noexcept {
    static_cast<void>(us);
  }
  bool GpioSet(tmc9660::CtrlPin pin, tmc9660::GpioSignal signal) noexcept
    requires Pins
  {
    return core_.Pin(pin, signal);
  }
  bool GpioGet(tmc9660::CtrlPin pin, bool& high) const noexcept {
    static_cast<void>(pin);
    high = core_.faultn_high;
    return true;
  }
  void Log(tmc9660::LogLevel level, const char* message) noexcept {
    static_cast<void>(level);
    Check((message != nullptr) && (static_cast<unsigned char>(message[0]) != 0U));
  }

private:
  Core& core_;
};

template <bool Pins>
class FuzzUart : public tmc9660::UartInterface<FuzzUart<Pins>> {
public:
  explicit FuzzUart(Core& core) noexcept : core_(core) {}
  bool EnsureInitialized() noexcept {
    return true;
  }
  tmc9660::BusStatus Exchange(const uint8_t* tx, std::size_t tx_len, uint8_t* rx,
                              std::size_t rx_len, uint32_t timeout_us) noexcept {
    Check((tx != nullptr) && (rx != nullptr) &&
          ((tx_len == 0U) || ((tx_len >= 8U) && (tx_len <= 9U))) && (timeout_us > 0U));
    ++core_.transfers;
    const uint8_t action = core_.NextAction();
    if (action == kFail) {
      ++core_.failures;
      return tmc9660::BusStatus::Fault;
    }
    if (!core_.wire.Exchange(core_.chip, tx, tx_len, rx, rx_len)) {
      return tmc9660::BusStatus::Timeout;
    }
    core_.Tamper(action, rx, rx_len);
    return tmc9660::BusStatus::Ok;
  }
  void DelayUs(uint32_t us) noexcept {
    static_cast<void>(us);
  }
  bool GpioSet(tmc9660::CtrlPin pin, tmc9660::GpioSignal signal) noexcept
    requires Pins
  {
    return core_.Pin(pin, signal);
  }
  bool GpioGet(tmc9660::CtrlPin pin, bool& high) const noexcept {
    static_cast<void>(pin);
    high = core_.faultn_high;
    return true;
  }
  void Log(tmc9660::LogLevel level, const char* message) noexcept {
    static_cast<void>(level);
    Check((message != nullptr) && (static_cast<unsigned char>(message[0]) != 0U));
  }

private:
  Core& core_;
};

/// Tracks the promises across calls.
template <typename Drv>
class Watch {
public:
  Watch(Drv& drv, Core& core, bool uart, bool canary) noexcept
      : drv_(drv), core_(core), uart_(uart), canary_(canary) {}

  /// Run one call; check what it returned.
  template <typename T>
  void After(const tmc9660::Result<T>& r, int before) noexcept {
    const bool was_init = was_init_;
    Check(std::strcmp(tmc9660::ToString(r.error), "Unknown") != 0);
    if constexpr (!std::is_void_v<T>) {
      if (!r.ok()) {
        Check(r.value == T{});
      }
    }
    const bool sent = (core_.transfers != before);
    if ((r.error == Error::NotInitialized) || (r.error == Error::InvalidArgument) ||
        (r.error == Error::WrongMode)) {
      Check(!sent);
    }
    if (r.error == Error::UnexpectedReset) {
      Check(!drv_.IsInitialized());
      noticed_ = true;
    }
    if (drv_.GetMode() != mode_before_) {
      noticed_ = true; // a new application was started and adopted: a fresh baseline
    }
    if (was_init && reset_pending_ && detectsReset() && !noticed_ && !core_.tampered_since_reset) {
      Check(core_.chip.writes_since_reset == core_.chip.stop_writes); // only stop commands may pass
    }
    safetyInvariants();
  }

  /// Called before each call: does the driver believe it is initialised?
  void Before() noexcept {
    was_init_ = drv_.IsInitialized();
    mode_before_ = drv_.GetMode();
  }
  void Reset() noexcept {
    core_.chip.BootInto(phase_);
    core_.tampered_since_reset = false;
    reset_pending_ = true;
    noticed_ = false;
  }
  void Started() noexcept {
    noticed_ = true;
  }
  void SetPhase(Phase p) noexcept {
    phase_ = p;
  }

private:
  /// Whether the transport of the driver's current mode can show an unseen reset (docs/safety.md).
  [[nodiscard]] bool detectsReset() const noexcept {
    const tmc9660::Mode m = drv_.GetMode();
    if (uart_) {
      return (m == tmc9660::Mode::Parameter) && canary_; // only the canary
    }
    static_cast<void>(m);
    return true; // SESSION_START, FIRST_CMD, the boot bit and the counter
  }

  void safetyInvariants() const noexcept {
    const Tmc9660Model& c = core_.chip;
    Check(c.energise_with_flag == 0);
    Check(c.energise_without_drv_en == 0);
    Check(c.boot_with_drive_on == 0);
    Check(c.reserved_start_events == 0);
    Check(c.otp_writes == 0);
    Check(c.zero_frames == 0);
  }

  Drv& drv_;
  Core& core_;
  bool uart_;
  bool canary_;
  bool reset_pending_{false};
  bool noticed_{false};
  bool was_init_{false};
  tmc9660::Mode mode_before_{tmc9660::Mode::Parameter};
  Phase phase_{Phase::Parameter};
};

/// The port type of a driver type.
template <typename Drv>
struct PortOf;
template <typename P>
struct PortOf<tmc9660::TMC9660<P>> {
  using Type = P;
};

/// The chip runs the protocol the driver speaks (an unseen reset may have changed it) at the
/// addresses it uses.
template <typename Drv>
bool SameMode(const Drv& drv, const Core& core) noexcept {
  const Phase p = core.chip.GetPhase();
  const tmc9660::Mode m = drv.GetMode();
  if (m == tmc9660::Mode::Parameter) {
    // SetGlobal() may have moved the chip's serial addresses away from the Config's: no reply
    // then passes the address check, which is the caller's doing.
    const tmc9660::Config& cfg = drv.GetConfig();
    return (p == Phase::Parameter) &&
           (core.chip.GlobalValue(0U, 1U) == static_cast<int64_t>(cfg.device_address)) &&
           (core.chip.GlobalValue(0U, 2U) == static_cast<int64_t>(cfg.tmcl_host_address));
  }
  if (m == tmc9660::Mode::Register) {
    return p == Phase::Register;
  }
  return p == Phase::Bootloader;
}

/// In parameter mode, the PWM is off or the stage is cut off by DRV_EN (port with the hook).
template <typename Drv>
void CheckStopped(const Core& core) noexcept {
  const Tmc9660Model& c = core.chip;
  if (c.GetPhase() != Phase::Parameter) {
    return; // the model's PWM is a parameter-mode notion
  }
  if constexpr (tmc9660::detail::HasGpioSet<typename PortOf<Drv>::Type>) {
    Check(!c.PwmActive() || !c.DrvEn());
  } else {
    Check(!c.PwmActive());
  }
}

/// One driver call chosen by the fuzzer.
template <typename Drv, typename W>
void Step(Drv& drv, Core& core, W& watch, Input& in) noexcept {
  const auto op = static_cast<uint8_t>(in.Byte() % 34U);
  const uint8_t a = in.Byte();
  const uint32_t v = in.Word();
  const int before = core.transfers;
  watch.Before();
  const auto param = [&a]() noexcept {
    return static_cast<tmc9660::Param>(
        tmc9660::kParams[static_cast<std::size_t>(a) % tmc9660::kParams.size()].id);
  };
  const auto reg = [&a]() noexcept {
    return tmc9660::kRegisters[static_cast<std::size_t>(a) % tmc9660::kRegisters.size()].addr;
  };
  switch (op) {
  case 0: {
    const auto r = drv.EnsureInitialized();
    if (drv.IsInitialized()) {
      watch.Started(); // the driver has noticed and started again
    }
    watch.After(r, before);
    break;
  }
  case 1: {
    const int attempts = core.drv_en_inactive_attempts;
    const int tampers = core.tampers;
    const auto r = drv.Deinitialize();
    watch.After(r, before);
    if constexpr (tmc9660::detail::HasGpioSet<typename PortOf<Drv>::Type>) {
      Check(core.drv_en_inactive_attempts > attempts); // dropped even when not initialised
    }
    if (r.ok() && (core.tampers == tampers)) {
      CheckStopped<Drv>(core); // a stop that says Ok
    }
    break;
  }
  case 2: {
    const int attempts = core.drv_en_inactive_attempts;
    const int tampers = core.tampers;
    const int failures = core.failures;
    const bool clean =
        (core.chip.not_ready_replies == 0) && (core.chip.checksum_error_replies == 0);
    const int stops = core.chip.disable_cmds;
    // Judged on the protocol the driver spoke when the call began: a stop that finds another one
    // running (the bootloader gone, an application there) sends its stop but reports the finding.
    const bool same = SameMode(drv, core);
    const auto r = drv.DisableDrive();
    watch.After(r, before);
    if constexpr (tmc9660::detail::HasGpioSet<typename PortOf<Drv>::Type>) {
      Check(core.drv_en_inactive_attempts > attempts);
    }
    if (r.ok() && (core.tampers == tampers)) {
      CheckStopped<Drv>(core);
    }
    if (clean && (core.failures == failures) && (core.tampers == tampers) && same &&
        (core.chip.disable_cmds > stops)) {
      // The chip got the stop and nothing was lost or corrupted: a late or stale reply alone never
      // makes the stop fail (the repeat after the resync confirms it).
      Check(r.ok());
    }
    break;
  }
  case 3: {
    const bool was_running = core.chip.PwmActive();
    const int failures = core.failures;
    const int tampers = core.tampers;
    const auto r = drv.EnableDrive(a);
    watch.After(r, before);
    // One lost transfer (the reply of the COMMUTATION_MODE write, say) is what makes an enable fail
    // after the chip ran it; the stop that follows has its own repeat. Two can defeat both.
    if (!r.ok() && !was_running && ((core.failures - failures) <= 1) && (core.tampers == tampers)) {
      CheckStopped<Drv>(core); // a failed enable leaves nothing running, also without the hook
    }
    break;
  }
  case 4:
    watch.After(drv.SetParam(param(), static_cast<int32_t>(v), static_cast<uint8_t>(a >> 4U)),
                before);
    break;
  case 5: {
    const auto p = param();
    const auto r = drv.GetParam(p, 0U);
    watch.After(r, before);
    if (r.ok() && !core.tampered_any) {
      Check(r.value == core.chip.ParamValue(static_cast<unsigned>(p))); // not another reply's value
    }
    break;
  }
  case 6:
    watch.After(drv.SetGlobal(static_cast<tmc9660::GlobalBank>(a & 3U),
                              static_cast<uint16_t>(v & 0x3FU), static_cast<int32_t>(v >> 8U)),
                before);
    break;
  case 7:
    watch.After(
        drv.GetGlobal(static_cast<tmc9660::GlobalBank>(a & 3U), static_cast<uint16_t>(v & 0x3FU)),
        before);
    break;
  case 8:
    watch.After(drv.ClearFlags(param(), v), before);
    break;
  case 9:
    watch.After(drv.StoreParams(), before);
    break;
  case 10:
    watch.After(drv.FactoryDefault((a & 1U) != 0U), before);
    break;
  case 11:
    watch.After(drv.ReturnToBootloader(), before);
    break;
  case 12:
    watch.After(drv.BlGetInfo(static_cast<tmc9660::BlInfo>(a)), before);
    break;
  case 13:
    watch.After(drv.BlRead32(std::bit_cast<tmc9660::Bank>(static_cast<uint8_t>(a & 7U)), v & ~3U),
                before);
    break;
  case 14:
    watch.After(
        drv.BlWrite32(std::bit_cast<tmc9660::Bank>(static_cast<uint8_t>(a & 7U)), v & ~3U, v),
        before);
    break;
  case 15: {
    tmc9660::BootConfig cfg;
    static_cast<void>(cfg.SetWord(static_cast<std::size_t>(a % 16U), v));
    static_cast<void>(cfg.SetWord(2U, (v >> 3U) | (static_cast<uint32_t>(a) << 28U)));
    tmc9660::BlWriteOptions options;
    options.allow_ldo_change = ((a & 0x80U) != 0U);
    watch.After(drv.BlWriteConfig(cfg, options), before);
    break;
  }
  case 16:
    watch.After(drv.BlStartApplication(static_cast<tmc9660::BootMode>(a & 3U)), before);
    break;
  case 17:
    watch.After(drv.BlEraseFlashSector(v), before);
    break;
  case 18:
    watch.After(drv.ReadReg(reg()), before);
    break;
  case 19:
    watch.After(drv.WriteReg(reg(), v), before);
    break;
  case 20:
    watch.After(drv.UpdateReg(reg(), v, v & (v >> 3U)), before);
    break;
  case 21: {
    tmc9660::GateDriverRegConfig cfg;
    cfg.gdrv_cfg = v & 0x00F3'FFFFU;
    cfg.gdrv_hw = (v >> 4U) & 0x1300'0FFFU;
    cfg.chop_run = static_cast<uint8_t>(a & 7U);
    watch.After(drv.GateDriverStart(cfg), before);
    break;
  }
  case 22:
    watch.After(drv.RegRamDebug(static_cast<uint8_t>(a & 15U), static_cast<uint8_t>(v & 63U),
                                (a & 0x80U) != 0U, v),
                before);
    break;
  case 23:
    watch.After(drv.Service(), before);
    break;
  case 24:
    watch.After(drv.MotorStop(), before);
    break;
  case 25:
    watch.After(drv.SetIo(static_cast<uint8_t>(a & 31U), (v & 1U) != 0U), before);
    break;
  case 26:
    watch.After(drv.PowerDown(static_cast<tmc9660::PowerDownTime>(a & 15U)), before);
    break;
  case 27:
    watch.Reset(); // an unseen reset of the chip
    break;
  case 28:
    core.chip.SetParamValue(299U, v & 0x30U); // latch or clear general error flags
    core.chip.SetParamValue(300U, (v >> 8U) & 1U);
    core.chip.gdrv_fault_bits = (v >> 16U) & 0x0F0FU;
    if (((v & 0x8000'0000U) != 0U) && (core.chip.GetPhase() == Phase::Parameter)) {
      // An application that energises by itself (a stored script): the stops must still work.
      core.chip.SetParamValue(4U, static_cast<int64_t>(a & 7U) + 1);
    }
    break;
  case 29:
    core.faultn_high = ((a & 1U) != 0U);
    core.chip.flash_busy = ((a & 2U) != 0U);
    core.chip.pll_never_locks = ((a & 4U) != 0U);
    core.chip.corrupt_cfg_write = ((a & 8U) != 0U);
    break;
  case 30:
    core.chip.not_ready_replies = static_cast<int>(a & 7U);
    core.chip.busy_replies = static_cast<int>((static_cast<uint32_t>(a) >> 3U) & 7U);
    core.chip.checksum_error_replies = static_cast<int>((v & 3U));
    break;
  case 31:
    watch.After(drv.ScriptRun((a & 1U) != 0U, v & 0xFFFU), before);
    break;
  case 32:
    core.wire.late_next = ((a & 1U) != 0U); // the next UART reply arrives after the timeout
    core.wire.port_flushes = ((a & 2U) != 0U);
    break;
  case 33:
    watch.Reset();
    core.chip.WatchdogReset(); // an unseen reset that also sets WATCHDOG_EVENT
    break;
  default:
    break;
  }
}

template <typename Port>
void Run(Input& in, Core& core, Port& port, const tmc9660::Config& config, Phase phase, bool uart) {
  tmc9660::TMC9660<Port> drv(port, config);
  Watch<tmc9660::TMC9660<Port>> watch(drv, core, uart, config.uart_canary);
  watch.SetPhase(phase);
  for (int i = 0; (i < 200) && !in.Empty(); ++i) {
    Step(drv, core, watch, in);
  }
  static_cast<void>(drv.DisableDrive());
}

} // namespace

static_assert(tmc9660::detail::HasGpioSet<FuzzSpi<true>> &&
              !tmc9660::detail::HasGpioSet<FuzzSpi<false>>);
static_assert(tmc9660::detail::HasGpioSet<FuzzUart<true>> &&
              !tmc9660::detail::HasGpioSet<FuzzUart<false>>);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, std::size_t size) {
  Input in(data, size);
  const uint8_t head = in.Byte();
  const bool uart = ((head & 1U) != 0U);
  const auto phase_bits = static_cast<uint8_t>((static_cast<uint32_t>(head) >> 1U) % 3U);
  Phase phase = Phase::Register;
  if (phase_bits == 0U) {
    phase = Phase::Bootloader;
  } else if (phase_bits == 1U) {
    phase = Phase::Parameter;
  } else {
    // Phase::Register
  }
  tmc9660::Config config{};
  config.mode = tmc9660::Mode::Register;
  if (phase == Phase::Bootloader) {
    config.mode = tmc9660::Mode::Bootloader;
  } else if (phase == Phase::Parameter) {
    config.mode = tmc9660::Mode::Parameter;
  } else {
    // Mode::Register
  }
  config.hardware_reset = false;
  config.uart_canary = ((head & 0x10U) == 0U);
  config.faultn_is_reset = ((head & 0x20U) != 0U);
  config.boot_timeout_ms = 30U;
  config.spi_reply_retries = static_cast<uint8_t>((static_cast<uint32_t>(head) >> 6U) & 3U);
  Core core(in);
  core.chip.BootInto(phase);
  core.chip.SetDrvEn((head & 0x08U) != 0U);
  const bool pins = ((head & 0x04U) == 0U);
  if (!pins) {
    core.chip.SetDrvEn(true); // no hook: DRV_EN is wired high
  }
  if (uart && pins) {
    FuzzUart<true> port(core);
    Run(in, core, port, config, phase, true);
  } else if (uart) {
    FuzzUart<false> port(core);
    Run(in, core, port, config, phase, true);
  } else if (pins) {
    FuzzSpi<true> port(core);
    Run(in, core, port, config, phase, false);
  } else {
    FuzzSpi<false> port(core);
    Run(in, core, port, config, phase, false);
  }
  return 0;
}
