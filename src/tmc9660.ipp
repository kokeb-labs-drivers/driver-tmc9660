/**
 * @file tmc9660.ipp
 * @brief TMC9660 driver implementation, included by `tmc9660.hpp`.
 * @copyright Copyright (c) 2026 The driver-tmc9660 authors. SPDX-License-Identifier: MIT
 *
 * References: DS = TMC9660 datasheet, PM = Parameter Mode Reference Manual, RM = Register Mode
 * Reference Manual.
 */
#ifndef TMC9660_IPP
#define TMC9660_IPP

namespace tmc9660 {

namespace detail {

/// Bits of MCC_GDRV_HW that energise the gate driver: BIAS_EN, CHARGEPUMP_EN, BST_SW_CP_EN and the
/// bridge enables.
inline constexpr uint32_t kGdrvHwEnergise = 0x0300'080FU;
/// Bits of MCC_PWM_CONFIG that enable outputs: CHOP, ENABLE_* and EXT_ENABLE_*.
inline constexpr uint32_t kPwmEnergise = 0x0000'FF07U;
/// ENABLE_UX1, ENABLE_VX2, ENABLE_WY1 and ENABLE_Y2 of MCC_PWM_CONFIG.
inline constexpr uint32_t kPwmEnableBits = 0x0000'0F00U;
/// Bits of the LDO word that hold the LDO configuration (word 0, bits 0 to 8).
inline constexpr uint32_t kLdoMask = 0x0000'01FFU;
/// PLL_STATUS, bit 30 of the clock word: read only.
inline constexpr uint32_t kPllStatus = 0x4000'0000U;
/// Bootstrap-undervoltage flags: they are set until the bootstrap capacitors charge and are no
/// start-up fault.
inline constexpr uint32_t kBootstrapFlags = 0x0000'F000U;
/// Latched fault bits of MCC_GDRV_FAULT.
inline constexpr uint32_t kGdrvFaultActive = 0x000F'000FU;
/// VS_UVLO_STS and VDRV_UVLO_STS of MCC_GDRV_FAULT.
inline constexpr uint32_t kGdrvUvloSts = 0xA000'0000U;
/// Delay in the polls of boot and PLL waits.
inline constexpr uint32_t kPollUs = 10'000;
/// Wait after the reset pin is released, before the first identity poll: T_WAKE is given as a
/// typical value only, 44 ms (A9), so the first poll finds the bootloader ready.
inline constexpr uint32_t kWakeUs = 50'000;
/// WATCHDOG_EVENT, bit 9 of GENERAL_ERROR_FLAGS: a watchdog reset happened (PM Table 59).
inline constexpr uint32_t kWatchdogEvent = 0x0000'0200U;
/// How long the drain after a failed UART exchange waits for a stale byte, in microseconds.
inline constexpr uint32_t kDrainTimeoutUs = 5'000;
/// The most stale 9-byte replies the drain removes after one failure.
inline constexpr uint32_t kDrainMax = 3;
/// SPI flash addresses are 24 bits (DS "SPI Flash"); command 0x20 erases a 4 KiB sector.
inline constexpr uint32_t kFlashAddressLimit = 0x0100'0000U;
/// The flash sector size of FLASH_ERASE_SECTOR (SECTOR_ERASE byte 0x20).
inline constexpr uint32_t kFlashSectorBytes = 4096U;

/// A bootloader reply that says the chip restarted.
[[nodiscard]] constexpr bool IsResetStatus(uint8_t status) noexcept {
  return (status == static_cast<uint8_t>(BlStatus::SessionStart)) ||
         (status == static_cast<uint8_t>(BlStatus::BootloaderResumed));
}

/// A RAMDebug type that Table 16 lists.
[[nodiscard]] constexpr bool IsRamDebugType(RamDebugType t) noexcept {
  const uint8_t v = ToUnderlying(t);
  return (v <= 14U) && (v != 2U);
}

/// A command a script download may carry: not a download, boot, run-control or version command.
[[nodiscard]] constexpr bool IsScriptOp(Tmcl op) noexcept {
  const uint8_t v = ToUnderlying(op);
  const bool in_script_range = (v >= 19U) && (v <= 57U);
  const bool in_control_set = (v == 3U) || (v == 5U) || (v == 6U) || (v == 9U) || (v == 10U) ||
                              (v == 13U) || (v == 14U) || (v == 15U);
  return in_script_range || in_control_set;
}

/// A latched gate-driver condition: any MCC_GDRV_STATUS bit but the bootstrap flags, or a latched
/// or undervoltage bit of MCC_GDRV_FAULT.
[[nodiscard]] constexpr bool GdrvLatched(uint32_t status, uint32_t fault) noexcept {
  return ((status & ~kBootstrapFlags) != 0U) || ((fault & (kGdrvFaultActive | kGdrvUvloSts)) != 0U);
}

/// A script instruction that would leave System off without `EnableDrive()`: SAP of
/// COMMUTATION_MODE with a non-zero value, or AAP of it (the accumulator is not known).
[[nodiscard]] constexpr bool IsEnergisingScriptOp(Tmcl op, uint16_t type, int32_t value) noexcept {
  const bool commutation = (type == static_cast<uint16_t>(ToUnderlying(Param::COMMUTATION_MODE)));
  return commutation && (((op == Tmcl::Sap) && (value != 0)) || (op == Tmcl::Aap));
}

/// Whether `value` is the default, one of the coded values or inside the range of the parameter.
[[nodiscard]] inline bool ParamValueAllowed(const ParamInfo& info, int64_t value) noexcept {
  bool valid = (value == info.def);
  if (info.coded) {
    for (uint32_t i = 0U; (i < info.allowed_count) && !valid; ++i) {
      valid = (static_cast<int64_t>(
                   kAllowedValues[static_cast<std::size_t>(info.allowed_first) + i]) == value);
    }
  } else {
    valid = valid || ((value >= info.min) && (value <= info.max));
  }
  return valid;
}

/// Writable bits of a register of the table (0 when it is not in it).
[[nodiscard]] inline uint32_t WritableMask(RegAddr addr) noexcept {
  const RegInfo* info = FindRegister(addr);
  return (info == nullptr) ? 0U : info->writable_mask;
}

/// Whether every word of a `GateDriverRegConfig` is inside the writable bits of its register.
[[nodiscard]] inline bool GdrvConfigValid(const GateDriverRegConfig& c) noexcept {
  struct Word {
    RegAddr addr;
    uint32_t value;
  };
  const std::array<Word, 9> words{{{.addr = regs::MCC_GDRV_CFG, .value = c.gdrv_cfg},
                                   {.addr = regs::MCC_GDRV_TIMING, .value = c.gdrv_timing},
                                   {.addr = regs::MCC_GDRV_BBM, .value = c.gdrv_bbm},
                                   {.addr = regs::MCC_GDRV_HW, .value = c.gdrv_hw},
                                   {.addr = regs::MCC_GDRV_PROT, .value = c.gdrv_prot},
                                   {.addr = regs::MCC_GDRV_OCP_UVW, .value = c.gdrv_ocp_uvw},
                                   {.addr = regs::MCC_GDRV_OCP_Y2, .value = c.gdrv_ocp_y2},
                                   {.addr = regs::MCC_GDRV_STATUS_EN, .value = c.status_en},
                                   {.addr = regs::MCC_GDRV_PROT_EN, .value = c.prot_en}}};
  for (const Word& w : words) {
    const RegInfo* info = FindRegister(w.addr);
    if ((info == nullptr) || ((w.value & ~info->writable_mask) != 0U)) {
      return false;
    }
  }
  return ((c.pwm_enable & ~kPwmEnableBits) == 0U) && (c.chop_run >= 1U) && (c.chop_run <= 7U);
}

} // namespace detail

// -------------------------------------------------------------------------------------------------
// Helpers
// -------------------------------------------------------------------------------------------------

template <AnyPort Port>
Result<void> TMC9660<Port>::fail(Error e, const char* message) noexcept {
  detail::Log(port_, LogLevel::Error, message);
  return Result<void>::Failure(e);
}

template <AnyPort Port>
Error TMC9660<Port>::need(Mode m) const noexcept {
  if (!initialized_) {
    return Error::NotInitialized;
  }
  if (mode_ != m) {
    return Error::WrongMode;
  }
  return Error::Ok;
}

template <AnyPort Port>
void TMC9660<Port>::markReset() noexcept {
  initialized_ = false;
  canary_armed_ = false;
  reg_started_ = false;
  detail::Log(port_, LogLevel::Warning, "tmc9660: the chip restarted; the driver is uninitialised");
}

template <AnyPort Port>
Error TMC9660<Port>::mapBusStatus(BusStatus s) const noexcept {
  switch (s) {
  case BusStatus::Ok:
    return Error::Ok;
  case BusStatus::Timeout:
    return Error::Timeout;
  case BusStatus::Fault:
    return Error::Bus;
  default:
    break;
  }
  return Error::Bus;
}

template <AnyPort Port>
Error TMC9660<Port>::spiTransfer(std::size_t len) noexcept {
  if constexpr (kSpi) {
    return port_.Transfer(tx_.data(), rx_.data(), len) ? Error::Ok : Error::Bus;
  } else {
    static_cast<void>(len);
    return Error::Unsupported;
  }
}

template <AnyPort Port>
Error TMC9660<Port>::uartResync() noexcept {
  if constexpr (!kSpi) {
    // The chip discards a partial request after its gap and a reply may still be on its way: wait
    // it out, then read until the line is quiet, so that no stale reply answers the next request.
    detail::DelayUs(port_, config_.uart_resync_us);
    for (uint32_t i = 0U; i < detail::kDrainMax; ++i) {
      const BusStatus s =
          port_.Exchange(tx_.data(), 0U, rx_.data(), rx_.size(), detail::kDrainTimeoutUs);
      if (s == BusStatus::Fault) {
        return Error::Bus; // the drain itself failed: the line stays dirty, nothing is sent
      }
      if (s != BusStatus::Ok) {
        break; // quiet
      }
    }
  }
  uart_dirty_ = false;
  return Error::Ok;
}

template <AnyPort Port>
Error TMC9660<Port>::uartExchange(std::size_t tx_len, std::size_t rx_len) noexcept {
  if constexpr (kSpi) {
    static_cast<void>(tx_len);
    static_cast<void>(rx_len);
    return Error::Unsupported;
  } else {
    if (uart_dirty_ && !defer_resync_) {
      if (const Error r = uartResync(); r != Error::Ok) {
        return r;
      }
    }
    defer_resync_ = false; // a stop is sent first; the line is resynchronised for what follows
    const Error e = mapBusStatus(
        port_.Exchange(tx_.data(), tx_len, rx_.data(), rx_len, config_.reply_timeout_us));
    if (e != Error::Ok) {
      uart_dirty_ = true;
    }
    return e;
  }
}

template <AnyPort Port>
Result<void> TMC9660<Port>::setDrvEn(GpioSignal s) noexcept {
  if constexpr (detail::HasGpioSet<Port>) {
    return port_.GpioSet(CtrlPin::DrvEn, s) ? Result<void>::Success()
                                            : Result<void>::Failure(Error::Bus);
  } else {
    static_cast<void>(s);
    return Result<void>::Success();
  }
}

// -------------------------------------------------------------------------------------------------
// Bootloader transport (DS "Bootloader")
// -------------------------------------------------------------------------------------------------

template <AnyPort Port>
Result<detail::BlReply> TMC9660<Port>::blSpiTransfer(BlCmd cmd, uint32_t value) noexcept {
  const auto req = detail::BlSpiRequest(detail::ToUnderlying(cmd), value);
  std::copy(req.begin(), req.end(), tx_.begin());
  const Error e = spiTransfer(detail::kBlSpiLen);
  if (e != Error::Ok) {
    return Result<detail::BlReply>::Failure(e);
  }
  return Result<detail::BlReply>::Success(
      detail::BlSpiDecode(std::span<const uint8_t>{rx_}.first<detail::kBlSpiLen>()));
}

template <AnyPort Port>
Error TMC9660<Port>::blExchangeSpi(BlCmd cmd, uint32_t value, bool write,
                                   detail::BlReply& reply) noexcept {
  if (write) {
    // A probe first: a reset since the last reply must not be followed by a write.
    const auto probe = blSpiTransfer(BlCmd::NoOp, 0U);
    if (!probe.ok()) {
      return probe.error;
    }
    if (detail::IsResetStatus(probe.value.status)) {
      markReset();
      return Error::UnexpectedReset;
    }
  }
  const auto first = blSpiTransfer(cmd, value);
  if (!first.ok()) {
    return first.error;
  }
  if (detail::IsResetStatus(first.value.status)) {
    markReset();
    return Error::UnexpectedReset;
  }
  for (uint32_t attempt = 0U; attempt <= config_.spi_reply_retries; ++attempt) {
    const auto r = blSpiTransfer(BlCmd::NoOp, 0U); // fetches the reply of `cmd`; only this repeats
    if (!r.ok()) {
      return r.error;
    }
    reply = r.value;
    if (reply.status != static_cast<uint8_t>(BlStatus::Busy)) {
      return Error::Ok;
    }
    detail::DelayUs(port_, config_.spi_gap_us);
  }
  last_status_ = reply.status;
  return Error::Busy;
}

template <AnyPort Port>
Error TMC9660<Port>::blExchangeUart(BlCmd cmd, uint32_t value, detail::BlReply& reply) noexcept {
  const bool reflect = config_.quirks.crc8_reflect_output;
  const auto req =
      detail::BlUartRequest(config_.device_address, detail::ToUnderlying(cmd), value, reflect);
  std::copy(req.begin(), req.end(), tx_.begin());
  const Error e = uartExchange(detail::kBlUartLen, detail::kBlUartLen);
  if (e != Error::Ok) {
    return e;
  }
  const auto frame = std::span<const uint8_t>{rx_}.first<detail::kBlUartLen>();
  if (!detail::BlUartDecode(frame, config_.host_address, config_.device_address, reflect, reply)) {
    const bool crc_bad = detail::Crc8(frame.first(7U), reflect) != frame[7];
    uart_dirty_ = true;
    return crc_bad ? Error::Crc : Error::BadReply;
  }
  return Error::Ok;
}

template <AnyPort Port>
Error TMC9660<Port>::blTransport(BlCmd cmd, uint32_t value, bool write,
                                 detail::BlReply& reply) noexcept {
  if constexpr (kSpi) {
    return blExchangeSpi(cmd, value, write, reply);
  } else {
    static_cast<void>(write);
    return blExchangeUart(cmd, value, reply);
  }
}

template <AnyPort Port>
Result<detail::BlReply> TMC9660<Port>::blExchange(BlCmd cmd, uint32_t value, bool write) noexcept {
  detail::BlReply reply{};
  const Error e = blTransport(cmd, value, write, reply);
  if (e != Error::Ok) {
    return Result<detail::BlReply>::Failure(e);
  }
  last_status_ = reply.status;
  if (reply.status == static_cast<uint8_t>(BlStatus::Ok)) {
    return Result<detail::BlReply>::Success(reply);
  }
  if (detail::IsResetStatus(reply.status)) {
    markReset();
    return Result<detail::BlReply>::Failure(Error::UnexpectedReset);
  }
  return Result<detail::BlReply>::Failure(
      reply.status == static_cast<uint8_t>(BlStatus::Busy) ? Error::Busy : Error::Nack);
}

// -------------------------------------------------------------------------------------------------
// Parameter-mode transport (PM "Communication")
// -------------------------------------------------------------------------------------------------

template <AnyPort Port>
Error TMC9660<Port>::spiFirstCommand() noexcept {
  if (rx_[0] == static_cast<uint8_t>(TmclSpiStatus::FirstCommand)) {
    markReset();
    return Error::UnexpectedReset;
  }
  return Error::Ok;
}

template <AnyPort Port>
Error TMC9660<Port>::tmclSpiFetch(detail::TmclReply& reply) noexcept {
  for (uint32_t attempt = 0U; attempt <= config_.spi_reply_retries; ++attempt) {
    const auto rr =
        detail::TmclSpiReplyRequest(); // only the reply-request is repeated, never the command
    std::copy(rr.begin(), rr.end(), tx_.begin());
    const Error re = spiTransfer(detail::kTmclSpiLen);
    if (re != Error::Ok) {
      return re;
    }
    if (const Error f = spiFirstCommand(); f != Error::Ok) {
      return f;
    }
    const uint8_t st = rx_[0];
    if (st == static_cast<uint8_t>(TmclSpiStatus::Ok)) {
      return detail::TmclSpiDecode(std::span<const uint8_t>{rx_}.first<detail::kTmclSpiLen>(),
                                   reply)
                 ? Error::Ok
                 : Error::BadReply;
    }
    if ((st != static_cast<uint8_t>(TmclSpiStatus::NotReady)) &&
        (st != static_cast<uint8_t>(TmclSpiStatus::ChecksumError))) {
      return Error::BadReply;
    }
    detail::DelayUs(port_, config_.spi_gap_us);
  }
  return Error::Busy;
}

template <AnyPort Port>
Error TMC9660<Port>::tmclRawSpi(const detail::TmclCommand& c, bool probe,
                                detail::TmclReply& reply) noexcept {
  if (probe) {
    const auto rr = detail::TmclSpiReplyRequest();
    std::copy(rr.begin(), rr.end(), tx_.begin());
    const Error pe = spiTransfer(detail::kTmclSpiLen);
    if (pe != Error::Ok) {
      return pe;
    }
    if (const Error f = spiFirstCommand(); f != Error::Ok) {
      return f;
    }
  }
  const auto req = detail::TmclSpiRequest(c, layout_);
  std::copy(req.begin(), req.end(), tx_.begin());
  const Error ce = spiTransfer(detail::kTmclSpiLen);
  if (ce != Error::Ok) {
    return ce;
  }
  if (const Error f = spiFirstCommand(); f != Error::Ok) {
    return f;
  }
  detail::DelayUs(port_, config_.spi_gap_us);
  return tmclSpiFetch(reply);
}

template <AnyPort Port>
Error TMC9660<Port>::tmclRawUart(const detail::TmclCommand& c, detail::TmclReply& reply) noexcept {
  const auto req = detail::TmclUartRequest(config_.device_address, c, layout_);
  std::copy(req.begin(), req.end(), tx_.begin());
  const Error e = uartExchange(detail::kTmclUartLen, detail::kTmclUartLen);
  if (e != Error::Ok) {
    return e;
  }
  if (!detail::TmclUartDecode(std::span<const uint8_t>{rx_}.first<detail::kTmclUartLen>(),
                              config_.tmcl_host_address, config_.device_address, reply)) {
    uart_dirty_ = true;
    return Error::BadReply;
  }
  return Error::Ok;
}

template <AnyPort Port>
Error TMC9660<Port>::tmclTransport(const detail::TmclCommand& c, bool probe,
                                   detail::TmclReply& reply) noexcept {
  if constexpr (kSpi) {
    return tmclRawSpi(c, probe, reply);
  } else {
    static_cast<void>(probe);
    return tmclRawUart(c, reply);
  }
}

template <AnyPort Port>
Result<detail::TmclReply> TMC9660<Port>::tmclRaw(const detail::TmclCommand& c, bool probe,
                                                 bool accept_loaded) noexcept {
  detail::TmclReply reply{};
  const Error e = tmclTransport(c, probe, reply);
  if (e != Error::Ok) {
    return Result<detail::TmclReply>::Failure(e);
  }
  if (reply.op != c.op) {
    if constexpr (!kSpi) {
      uart_dirty_ = true; // a reply to another request: the stream is out of step
    }
    return Result<detail::TmclReply>::Failure(Error::BadReply);
  }
  last_status_ = reply.status;
  last_status_tmcl_ = reply.status;
  last_op_ = reply.op;
  last_value_ = reply.value;
  const bool ok = (reply.status == static_cast<uint8_t>(TmclStatus::Ok)) ||
                  (accept_loaded && (reply.status == static_cast<uint8_t>(TmclStatus::CmdLoaded)));
  return ok ? Result<detail::TmclReply>::Success(reply)
            : Result<detail::TmclReply>::Failure(Error::Nack);
}

template <AnyPort Port>
Result<detail::TmclReply> TMC9660<Port>::tmcl(uint8_t op, uint16_t type, uint8_t motor,
                                              uint32_t value, bool write,
                                              bool accept_loaded) noexcept {
  const detail::TmclCommand c{.op = op, .type = type, .motor = motor, .value = value};
  if constexpr (!kSpi) {
    // The chip has no reset flag in parameter mode: the canary says whether it still holds what we
    // wrote.
    if (write && config_.uart_canary && canary_armed_) {
      const detail::TmclCommand probe{.op = detail::ToUnderlying(Tmcl::Ggp),
                                      .type = config_.canary_variable,
                                      .motor = detail::ToUnderlying(GlobalBank::UserVars),
                                      .value = 0U};
      const auto r = tmclRaw(probe, false, false);
      if (!r.ok()) {
        return r;
      }
      if (static_cast<int32_t>(r.value.value) != config_.canary_value) {
        markReset();
        return Result<detail::TmclReply>::Failure(Error::UnexpectedReset);
      }
    }
  }
  return tmclRaw(c, write, accept_loaded);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::armCanary() noexcept {
  if constexpr (!kSpi) {
    if (config_.uart_canary) {
      const auto r =
          tmclRaw(detail::TmclCommand{.op = detail::ToUnderlying(Tmcl::Sgp),
                                      .type = config_.canary_variable,
                                      .motor = detail::ToUnderlying(GlobalBank::UserVars),
                                      .value = static_cast<uint32_t>(config_.canary_value)},
                  false, false);
      if (!r.ok()) {
        return Result<void>::Failure(r.error);
      }
      canary_armed_ = true;
    }
  }
  return Result<void>::Success();
}

// -------------------------------------------------------------------------------------------------
// Register-mode transport (RM "Communication")
// -------------------------------------------------------------------------------------------------

template <AnyPort Port>
Error TMC9660<Port>::checkRegSpiReply(const detail::RegSpiReply& r) noexcept {
  if (reg_started_) {
    if (r.boot || (r.counter != reg_counter_)) {
      markReset();
      return Error::UnexpectedReset;
    }
  }
  reg_started_ = true;
  reg_counter_ = static_cast<uint8_t>((static_cast<uint32_t>(r.counter) + 1U) & 7U);
  return Error::Ok;
}

template <AnyPort Port>
Error TMC9660<Port>::regSpiDatagram(const std::array<uint8_t, detail::kRegSpiLen>& req,
                                    detail::RegSpiReply& out) noexcept {
  std::copy(req.begin(), req.end(), tx_.begin());
  const Error e = spiTransfer(detail::kRegSpiLen);
  if (e != Error::Ok) {
    return e;
  }
  out = detail::RegSpiDecode(std::span<const uint8_t>{rx_}.first<detail::kRegSpiLen>());
  return checkRegSpiReply(out);
}

template <AnyPort Port>
Result<uint32_t> TMC9660<Port>::regSpiExchange(uint8_t block, uint16_t offset, bool write,
                                               uint32_t data, bool probe) noexcept {
  const bool wide = config_.quirks.reg_spi_wide_offset;
  std::array<uint8_t, detail::kRegSpiLen> req{};
  if (!detail::RegSpiRequest(block, offset, write, data, wide, req)) {
    return Result<uint32_t>::Failure(Error::Unsupported);
  }
  std::array<uint8_t, detail::kRegSpiLen> nop{};
  static_cast<void>(
      detail::RegSpiRequest(0U, 0U, false, 0U, wide, nop)); // MCC_INFO_CHIP read: harmless
  detail::RegSpiReply rep{};
  if (probe) {
    if (const Error pe = regSpiDatagram(nop, rep); pe != Error::Ok) {
      return Result<uint32_t>::Failure(pe);
    }
  }
  if (const Error e1 = regSpiDatagram(req, rep); e1 != Error::Ok) {
    return Result<uint32_t>::Failure(e1);
  }
  if (const Error e2 = regSpiDatagram(nop, rep); e2 != Error::Ok) {
    return Result<uint32_t>::Failure(e2);
  }
  last_status_ = rep.ok ? 1U : 0U;
  return rep.ok ? Result<uint32_t>::Success(rep.value) : Result<uint32_t>::Failure(Error::Nack);
}

template <AnyPort Port>
Result<uint32_t> TMC9660<Port>::regUartExchange(uint8_t command, uint8_t block, uint16_t offset,
                                                uint32_t data, bool probe) noexcept {
  if (probe) {
    // A write needs evidence that no reset went unseen; the UART reply has none, FAULTN is the
    // only source (A18).
    if (const Error g = faultnGuard(); g != Error::Ok) {
      return Result<uint32_t>::Failure(g);
    }
  }
  std::array<uint8_t, detail::kRegUartLen> req{};
  if (!detail::RegUartRequest({.module_address = config_.device_address,
                               .command = command,
                               .block = block,
                               .offset = offset,
                               .data = data,
                               .msb_fields = config_.quirks.reg_uart_msb_fields},
                              req)) {
    return Result<uint32_t>::Failure(Error::InvalidArgument);
  }
  std::copy(req.begin(), req.end(), tx_.begin());
  const Error e = uartExchange(detail::kRegUartLen, detail::kRegUartLen);
  if (e != Error::Ok) {
    return Result<uint32_t>::Failure(e);
  }
  detail::TmclReply reply{};
  if (!detail::TmclUartDecode(std::span<const uint8_t>{rx_}.first<detail::kRegUartLen>(),
                              config_.tmcl_host_address, config_.device_address, reply) ||
      (reply.op != command)) {
    uart_dirty_ = true;
    return Result<uint32_t>::Failure(Error::BadReply);
  }
  last_status_ = reply.status;
  if (reply.status != static_cast<uint8_t>(TmclStatus::Ok)) {
    return Result<uint32_t>::Failure(Error::Nack);
  }
  return Result<uint32_t>::Success(reply.value);
}

template <AnyPort Port>
Result<uint32_t> TMC9660<Port>::regExchangeRaw(uint8_t command, uint8_t block, uint16_t offset,
                                               bool write, uint32_t data, bool probe) noexcept {
  if constexpr (kSpi) {
    static_cast<void>(command);
    return regSpiExchange(block, offset, write, data, probe);
  } else {
    static_cast<void>(write);
    return regUartExchange(command, block, offset, data, probe);
  }
}

template <AnyPort Port>
Result<uint32_t> TMC9660<Port>::regExchange(bool write, RegAddr addr, uint32_t data) noexcept {
  return regExchangeRaw(write ? detail::kRegUartWrite : detail::kRegUartRead,
                        static_cast<uint8_t>(addr.block), addr.offset, write, data, write);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::regBaseline() noexcept {
  if constexpr (kSpi) {
    std::array<uint8_t, detail::kRegSpiLen> nop{};
    static_cast<void>(
        detail::RegSpiRequest(0U, 0U, false, 0U, config_.quirks.reg_spi_wide_offset, nop));
    std::copy(nop.begin(), nop.end(), tx_.begin());
    const Error e = spiTransfer(detail::kRegSpiLen);
    if (e != Error::Ok) {
      return Result<void>::Failure(e);
    }
    const detail::RegSpiReply r =
        detail::RegSpiDecode(std::span<const uint8_t>{rx_}.first<detail::kRegSpiLen>());
    reg_started_ =
        true; // the first datagram of a session carries the boot bit: that is the baseline
    reg_counter_ = static_cast<uint8_t>((static_cast<uint32_t>(r.counter) + 1U) & 7U);
  }
  return Result<void>::Success();
}

template <AnyPort Port>
Error TMC9660<Port>::faultnGuard() noexcept {
  if constexpr (detail::HasGpioGet<Port>) {
    if (config_.faultn_is_reset) {
      bool high = true;
      if (!port_.GpioGet(CtrlPin::Faultn, high)) {
        return Error::Bus;
      }
      if (!high) {
        markReset();
        return Error::UnexpectedReset;
      }
    }
  }
  return Error::Ok;
}

// -------------------------------------------------------------------------------------------------
// Lifecycle
// -------------------------------------------------------------------------------------------------

template <AnyPort Port>
Result<void> TMC9660<Port>::verifyIdentity() noexcept {
  uint32_t id = 0U;
  switch (mode_) {
  case Mode::Bootloader: {
    if constexpr (kSpi) {
      // The first datagram of a session returns SESSION_START (or the reply of an earlier one):
      // consume it.
      const auto first = blSpiTransfer(BlCmd::NoOp, 0U);
      if (!first.ok()) {
        return Result<void>::Failure(first.error);
      }
    }
    const auto r = blExchange(BlCmd::GetInfo, static_cast<uint32_t>(BlInfo::ChipType), false);
    if (!r.ok()) {
      return Result<void>::Failure(r.error);
    }
    id = r.value.value;
    break;
  }
  case Mode::Parameter: {
    if constexpr (kSpi) {
      // The reply-request that consumes FIRST_CMD, so that a later FIRST_CMD means a reset.
      const auto rr = detail::TmclSpiReplyRequest();
      std::copy(rr.begin(), rr.end(), tx_.begin());
      const Error e = spiTransfer(detail::kTmclSpiLen);
      if (e != Error::Ok) {
        return Result<void>::Failure(e);
      }
    }
    const auto r = tmcl(detail::ToUnderlying(Tmcl::GetInfo), 0U, 0U, 0U, false, false);
    if (!r.ok()) {
      return Result<void>::Failure(r.error);
    }
    id = r.value.value;
    break;
  }
  case Mode::Register: {
    const auto b = regBaseline();
    if (!b.ok()) {
      return b;
    }
    const auto r = regExchange(false, regs::MCC_INFO_CHIP, 0U);
    if (!r.ok()) {
      return Result<void>::Failure(r.error);
    }
    id = r.value;
    break;
  }
  default:
    return Result<void>::Failure(Error::InvalidArgument);
  }
  return (id == kChipId) ? Result<void>::Success() : Result<void>::Failure(Error::DeviceIdMismatch);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::waitForIdentity() noexcept {
  // `boot_timeout_ms` bounds the whole wait: every failed poll is charged the pause between polls
  // and, on UART, the longest it can have taken (a full reply wait, then the resync gap and the
  // drains before the next request). The driver has no clock; these are the waits it asked for.
  uint64_t budget_us = static_cast<uint64_t>(config_.boot_timeout_ms) * 1000U;
  uint64_t cost_us = detail::kPollUs;
  if constexpr (!kSpi) {
    cost_us += static_cast<uint64_t>(config_.reply_timeout_us) + config_.uart_resync_us +
               (static_cast<uint64_t>(detail::kDrainMax) * detail::kDrainTimeoutUs);
  }
  Result<void> last = Result<void>::Failure(Error::Timeout);
  // Validate() keeps `boot_timeout_ms` at 1 or more, so the budget is positive and one poll is
  // made.
  while (budget_us > 0U) {
    last = verifyIdentity();
    if (last.ok()) {
      return last;
    }
    detail::DelayUs(port_, detail::kPollUs);
    budget_us = (budget_us > cost_us) ? (budget_us - cost_us) : 0U;
  }
  return Result<void>::Failure((last.error == Error::DeviceIdMismatch) ? Error::DeviceIdMismatch
                                                                       : Error::Timeout);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::startPreconditions() noexcept {
  if (Validate(config_) != Error::Ok) {
    return fail(Error::InvalidArgument, "tmc9660: invalid Config");
  }
  if constexpr (!kSpi) {
    if ((config_.mode != Mode::Bootloader) && ((config_.device_address & 1U) == 0U)) {
      return fail(Error::InvalidArgument,
                  "tmc9660: the device address must be odd for TMCL and register mode");
    }
  }
  if (config_.hardware_reset && !detail::HasGpioSet<Port>) {
    return fail(Error::Unsupported, "tmc9660: hardware_reset needs the GpioSet hook");
  }
  if (!port_.EnsureInitialized()) {
    return fail(Error::Bus, "tmc9660: the port could not bring the bus up");
  }
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::hardwareReset() noexcept {
  if constexpr (detail::HasGpioSet<Port>) {
    // Outputs off first, then the reset pulse; RSTN is released inactive.
    if (!port_.GpioSet(CtrlPin::DrvEn, GpioSignal::INACTIVE) ||
        !port_.GpioSet(CtrlPin::Reset, GpioSignal::ACTIVE)) {
      return fail(Error::Bus, "tmc9660: the reset pin could not be driven");
    }
    detail::DelayUs(port_, 1'000U);
    if (!port_.GpioSet(CtrlPin::Reset, GpioSignal::INACTIVE)) {
      return fail(Error::Bus, "tmc9660: the reset pin could not be released");
    }
    detail::DelayUs(port_, detail::kWakeUs); // T_WAKE: the first poll must not be a doomed one
  }
  if (const auto w = waitForIdentity(); !w.ok()) {
    return fail(w.error, "tmc9660: the chip did not answer after the reset");
  }
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::EnsureInitialized() noexcept {
  if (initialized_) {
    return Result<void>::Success();
  }
  if (const auto pre = startPreconditions(); !pre.ok()) {
    return pre;
  }
  // The protocol of the Config is tried; it is kept only once the chip answers in it. A failed
  // start leaves the last protocol proved (a started application, say), so the stop still speaks
  // the one the chip may be running.
  const Mode previous = mode_;
  mode_ = config_.mode;
  canary_armed_ = false;
  reg_started_ = false;
  if (config_.hardware_reset) {
    if (const auto h = hardwareReset(); !h.ok()) {
      mode_ = previous;
      return h;
    }
  } else if (const auto v = verifyIdentity(); !v.ok()) {
    mode_ = previous;
    return fail(v.error, "tmc9660: identity check failed");
  } else {
    // identity confirmed
  }
  if (mode_ == Mode::Parameter) {
    if (const auto p = startParameterMode(); !p.ok()) {
      return fail(p.error, "tmc9660: parameter mode did not start");
    }
  }
  initialized_ = true;
  if constexpr (!detail::HasGpioSet<Port>) {
    if (mode_ == Mode::Bootloader) {
      // No DRV_EN hook: an unseen restart is stopped by guessing the application, and after a
      // restart the OTP decides. Nothing was written since the reset, so the configuration read
      // here is what the OTP loaded; the read is best effort (the guess stays `Config::mode`).
      const auto c = BlReadConfig();
      if (!c.ok() && !initialized_) {
        return fail(c.error, "tmc9660: the chip restarted during the start");
      }
    }
  }
  detail::Log(port_, LogLevel::Info, "tmc9660: initialised");
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::Deinitialize() noexcept {
  // Always the full stop: after a reported reset DRV_EN is still high and the chip may run an
  // application that energises by itself, so "not initialised" is no reason to send nothing.
  const Result<void> r = DisableDrive();
  initialized_ = false;
  canary_armed_ = false;
  return r;
}

template <AnyPort Port>
Result<bool> TMC9660<Port>::probeTmclLayout(TmclLayout layout) noexcept {
  layout_ = layout;
  // Two reads of flag words whose numbers (299 and 300) need type[11:8] = 1. Under the other
  // layout the chip sees type 43 or 44, which no parameter has: it answers with an error status.
  // A read cannot change anything, whatever the chip makes of the frame.
  const std::array<Param, 2> probes{Param::GENERAL_ERROR_FLAGS, Param::GDRV_ERROR_FLAGS};
  for (const Param p : probes) {
    const auto r =
        tmclRaw(detail::TmclCommand{.op = detail::ToUnderlying(Tmcl::Gap),
                                    .type = static_cast<uint16_t>(detail::ToUnderlying(p)),
                                    .motor = 0U,
                                    .value = 0U},
                false, false);
    if (r.error == Error::Nack) {
      return Result<bool>::Success(false);
    }
    if (!r.ok()) {
      return Result<bool>::Failure(r.error);
    }
  }
  return Result<bool>::Success(true);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::verifyTmclLayout() noexcept {
  const TmclLayout want = config_.quirks.tmcl_layout;
  if (want != TmclLayout::Auto) {
    const auto r = probeTmclLayout(want);
    if (!r.ok()) {
      return Result<void>::Failure(r.error);
    }
    if (!r.value) {
      return fail(Error::Refused,
                  "tmc9660: the chip does not answer the configured TMCL layout (A2)");
    }
    layout_ = want;
    return Result<void>::Success();
  }
  const std::array<TmclLayout, 2> candidates{TmclLayout::MotorHigh, TmclLayout::TypeHigh};
  uint32_t answered = 0U;
  TmclLayout found = TmclLayout::MotorHigh;
  for (const TmclLayout candidate : candidates) {
    const auto r = probeTmclLayout(candidate);
    if (!r.ok()) {
      return Result<void>::Failure(r.error);
    }
    if (r.value) {
      ++answered;
      found = candidate;
    }
  }
  if (answered != 1U) {
    return fail(Error::Refused,
                "tmc9660: no single TMCL layout answered the probe: set Quirks::tmcl_layout (A2)");
  }
  layout_ = found;
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::startParameterMode() noexcept {
  // No write leaves the driver before the layout is proved: a wrong one would put a protection
  // parameter on the wire as another parameter.
  if (const auto v = verifyTmclLayout(); !v.ok()) {
    return v;
  }
  const auto flags = readParamRaw(Param::GENERAL_ERROR_FLAGS, 0U);
  if (!flags.ok()) {
    return Result<void>::Failure(flags.error);
  }
  watchdog_at_start_ = ((static_cast<uint64_t>(flags.value) & detail::kWatchdogEvent) != 0U);
  return armCanary();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::Service() noexcept {
  if (!initialized_) {
    return Result<void>::Failure(Error::NotInitialized);
  }
  if (mode_ != Mode::Bootloader) {
    if (const Error g = faultnGuard(); g != Error::Ok) {
      return Result<void>::Failure(g);
    }
  }
  if (mode_ == Mode::Parameter) {
    const auto r = readParamRaw(Param::GENERAL_ERROR_FLAGS, 0U);
    if (!r.ok()) {
      return Result<void>::Failure(r.error);
    }
    if (!watchdog_at_start_ && ((static_cast<uint64_t>(r.value) & detail::kWatchdogEvent) != 0U)) {
      markReset(); // WATCHDOG_EVENT appeared after the start: the chip restarted
      return Result<void>::Failure(Error::UnexpectedReset);
    }
  }
  return Result<void>::Success();
}

// -------------------------------------------------------------------------------------------------
// Safety
// -------------------------------------------------------------------------------------------------

template <AnyPort Port>
Result<void> TMC9660<Port>::disableDriveParameter() noexcept {
  const detail::TmclCommand stop{
      .op = detail::ToUnderlying(Tmcl::Sap),
      .type = static_cast<uint16_t>(detail::ToUnderlying(Param::COMMUTATION_MODE)),
      .motor = 0U,
      .value = static_cast<uint32_t>(kSystemOff)};
  defer_resync_ = true;
  auto r = tmclRaw(stop, false, false);
  if (!r.ok()) {
    // The frame only reported a reset (SPI FIRST_CMD), was lost, met a stale reply or was merged
    // with a partial request: send it once more. The line is dirty now, so the repeat waits for
    // the resynchronisation, and the stop is confirmed by a reply of its own.
    r = tmclRaw(stop, false, false);
  }
  if (!r.ok()) {
    return Result<void>::Failure(r.error);
  }
  if constexpr (!detail::HasGpioSet<Port>) {
    return idleSafe(stop); // no DRV_EN pin to fall back on: System off must switch the PWM off
  }
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::idleSafe(const detail::TmclCommand& stop) noexcept {
  // PM "System Off": with IDLE_MOTOR_PWM_BEHAVIOR 0 the PWM stays active in System off. The
  // parameter acts when System off is entered, so it is set to 1 and System off is entered again.
  const auto idle = readParamRaw(Param::IDLE_MOTOR_PWM_BEHAVIOR, 0U);
  if (!idle.ok()) {
    return Result<void>::Failure(idle.error);
  }
  if (idle.value == 1) {
    return Result<void>::Success();
  }
  const auto set = tmclRaw(
      detail::TmclCommand{
          .op = detail::ToUnderlying(Tmcl::Sap),
          .type = static_cast<uint16_t>(detail::ToUnderlying(Param::IDLE_MOTOR_PWM_BEHAVIOR)),
          .motor = 0U,
          .value = 1U},
      false, false);
  if (!set.ok()) {
    return Result<void>::Failure(set.error);
  }
  const auto again = tmclRaw(stop, false, false);
  if (!again.ok()) {
    return Result<void>::Failure(again.error);
  }
  const auto back = readParamRaw(Param::IDLE_MOTOR_PWM_BEHAVIOR, 0U);
  if (!back.ok()) {
    return Result<void>::Failure(back.error);
  }
  return (back.value == 1) ? Result<void>::Success() : Result<void>::Failure(Error::VerifyFailed);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::disableDriveRegister() noexcept {
  const auto clear = [this](RegAddr a) noexcept -> Result<uint32_t> {
    auto w = regExchangeRaw(detail::kRegUartWrite, static_cast<uint8_t>(a.block), a.offset, true,
                            0U, false);
    if (!w.ok()) {
      // The first datagram only reported a reset, or the write failed: write once more.
      w = regExchangeRaw(detail::kRegUartWrite, static_cast<uint8_t>(a.block), a.offset, true, 0U,
                         false);
    }
    return w;
  };
  defer_resync_ = true;
  Result<void> first = Result<void>::Success();
  const auto a = clear(regs::MCC_GDRV_HW);
  if (!a.ok()) {
    first = Result<void>::Failure(a.error);
  }
  const auto b = clear(regs::MCC_PWM_CONFIG);
  if (!b.ok() && first.ok()) {
    first = Result<void>::Failure(b.error);
  }
  // Both writes are always attempted; then both are read back, because a write that the chip took
  // for something else (a wrong SPI layout, A3) is acknowledged all the same.
  const auto ra =
      regExchangeRaw(detail::kRegUartRead, static_cast<uint8_t>(regs::MCC_GDRV_HW.block),
                     regs::MCC_GDRV_HW.offset, false, 0U, false);
  const auto rb =
      regExchangeRaw(detail::kRegUartRead, static_cast<uint8_t>(regs::MCC_PWM_CONFIG.block),
                     regs::MCC_PWM_CONFIG.offset, false, 0U, false);
  if (first.ok()) {
    if (!ra.ok()) {
      first = Result<void>::Failure(ra.error);
    } else if (!rb.ok()) {
      first = Result<void>::Failure(rb.error);
    } else if (((ra.value & detail::kGdrvHwEnergise) != 0U) ||
               ((rb.value & (detail::kPwmEnergise | detail::kPwmEnableBits)) != 0U)) {
      first = Result<void>::Failure(Error::VerifyFailed);
    } else {
      // both energising masks read back clear
    }
  }
  return first;
}

template <AnyPort Port>
void TMC9660<Port>::blStopUnknownApplication() noexcept {
  // The bootloader did not answer (SESSION_START would have said it restarted as itself): an
  // unseen restart may have started the application the last BOOT_MODE read names, or the one
  // `Config::mode` names. The driver is no longer initialised and speaks that protocol from now
  // on, so this stop and the next ones reach it; the caller still gets the error.
  initialized_ = false;
  canary_armed_ = false;
  reg_started_ = false;
  if (bl_app_mode_ == Mode::Parameter) {
    mode_ = Mode::Parameter;
    static_cast<void>(disableDriveParameter());
  } else if (bl_app_mode_ == Mode::Register) {
    mode_ = Mode::Register;
    static_cast<void>(disableDriveRegister());
  } else {
    // no application named: nothing to address; DisableDrive() now reports NotInitialized
  }
}

template <AnyPort Port>
Result<void> TMC9660<Port>::DisableDrive() noexcept {
  // Never refused: every step is attempted, in the order pin, chip, whatever happened before.
  Result<void> first = setDrvEn(GpioSignal::INACTIVE);
  Result<void> chip = Result<void>::Success();
  if (mode_ == Mode::Parameter) {
    chip = disableDriveParameter();
  } else if (mode_ == Mode::Register) {
    chip = disableDriveRegister();
  } else if (!initialized_ && !detail::HasGpioSet<Port>) {
    // No proved protocol and no DRV_EN pin: the chip may run an application the driver cannot
    // address, so the stop is not reported as done.
    chip = Result<void>::Failure(Error::NotInitialized);
  } else if constexpr (!detail::HasGpioSet<Port>) {
    // No DRV_EN pin: the stop is done only if the bootloader still answers, with its reset evidence
    // checked (a SESSION_START on SPI is UnexpectedReset). An unseen restart may have started an
    // application (bootloader UART has no reset evidence, A18).
    const auto id = blExchange(BlCmd::GetInfo, static_cast<uint32_t>(BlInfo::ChipType), false);
    if (!id.ok()) {
      chip = Result<void>::Failure(id.error);
    } else if (id.value.value != kChipId) {
      chip = Result<void>::Failure(Error::DeviceIdMismatch);
    } else {
      // the bootloader still runs: its gate driver is off
    }
    // A reset seen (UnexpectedReset), BUSY or an error status (Nack) are all answers of the
    // bootloader itself: it runs, nothing is guessed and the mode stays.
    const bool bootloader_answered = (chip.error == Error::UnexpectedReset) ||
                                     (chip.error == Error::Busy) || (chip.error == Error::Nack);
    if (!chip.ok() && !bootloader_answered) {
      blStopUnknownApplication(); // not the bootloader: maybe an application, best effort
    }
  } else {
    // The bootloader leaves the gate driver off (proved by the start), or DRV_EN was dropped.
  }
  if (first.ok() && !chip.ok()) {
    first = chip;
  }
  return first;
}

template <AnyPort Port>
Result<void> TMC9660<Port>::MotorStop() noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  defer_resync_ = true;
  auto r = tmcl(detail::ToUnderlying(Tmcl::Mst), 0U, 0U, 0U, false,
                false); // a stop is never gated by a probe
  if (!r.ok()) {
    r = tmcl(detail::ToUnderlying(Tmcl::Mst), 0U, 0U, 0U, false, false); // once more, resynced
  }
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::EnableDrive(uint8_t commutation_mode) noexcept {
  if ((commutation_mode == 0U) || (commutation_mode > static_cast<uint8_t>(kCommutationModeMax))) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  // Guards first: nothing is energised until both flag words read back zero.
  const auto err = readParamRaw(Param::GENERAL_ERROR_FLAGS, 0U);
  if (!err.ok()) {
    return Result<void>::Failure(err.error);
  }
  const auto gdrv = readParamRaw(Param::GDRV_ERROR_FLAGS, 0U);
  if (!gdrv.ok()) {
    return Result<void>::Failure(gdrv.error);
  }
  if ((err.value != 0) || (gdrv.value != 0)) {
    return fail(Error::DeviceFault, "tmc9660: a latched error flag forbids enabling the drive");
  }
  if (const auto pin = setDrvEn(GpioSignal::ACTIVE); !pin.ok()) {
    static_cast<void>(setDrvEn(GpioSignal::INACTIVE));
    return pin;
  }
  const auto w = tmcl(detail::ToUnderlying(Tmcl::Sap),
                      static_cast<uint16_t>(detail::ToUnderlying(Param::COMMUTATION_MODE)), 0U,
                      commutation_mode, true, false);
  if (!w.ok()) {
    // The chip may have run the SAP although its reply was lost: the full stop, which also works
    // without a DRV_EN hook.
    static_cast<void>(DisableDrive());
    return Result<void>::Failure(w.error);
  }
  return Result<void>::Success();
}

// -------------------------------------------------------------------------------------------------
// Bootloader API
// -------------------------------------------------------------------------------------------------

template <AnyPort Port>
Result<uint32_t> TMC9660<Port>::BlGetInfo(BlInfo info) noexcept {
  if (const Error e = need(Mode::Bootloader); e != Error::Ok) {
    return Result<uint32_t>::Failure(e);
  }
  const auto r = blExchange(BlCmd::GetInfo, static_cast<uint32_t>(info), false);
  if (!r.ok()) {
    return Result<uint32_t>::Failure(r.error);
  }
  if (const auto live = blLiveness(r.value.value); !live.ok()) {
    return Result<uint32_t>::Failure(live.error);
  }
  return Result<uint32_t>::Success(r.value.value);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::blSetBank(Bank bank) noexcept {
  const auto b = blExchange(BlCmd::SetBank, static_cast<uint32_t>(bank), true);
  if (!b.ok()) {
    return Result<void>::Failure(b.error);
  }
  bl_bank_ = bank;
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::blSetup(Bank bank, uint32_t address) noexcept {
  const auto b = blSetBank(bank);
  if (!b.ok()) {
    return b;
  }
  const auto a = blExchange(BlCmd::SetAddress, address, true);
  return a.ok() ? Result<void>::Success() : Result<void>::Failure(a.error);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::blLiveness(uint32_t value) noexcept {
  if constexpr (kSpi) {
    // Bootloader SPI replies have no checksum, and `status 0, value 0` is a valid reply: a dead
    // MISO line looks like a read that returned zero. A zero is therefore believed only when the
    // chip ID, read right after, is still the chip's.
    if (value == 0U) {
      const auto id = blExchange(BlCmd::GetInfo, static_cast<uint32_t>(BlInfo::ChipType), false);
      if (!id.ok()) {
        return Result<void>::Failure(id.error);
      }
      if (id.value.value != kChipId) {
        return Result<void>::Failure(Error::BadReply);
      }
    }
  } else {
    static_cast<void>(value); // the UART reply is protected by a CRC-8
  }
  return Result<void>::Success();
}

template <AnyPort Port>
Result<uint32_t> TMC9660<Port>::blReadWord(uint32_t address) noexcept {
  const auto a = blExchange(BlCmd::SetAddress, address, true);
  if (!a.ok()) {
    return Result<uint32_t>::Failure(a.error);
  }
  const auto r = blExchange(BlCmd::Read32, 0U, false);
  return r.ok() ? Result<uint32_t>::Success(r.value.value) : Result<uint32_t>::Failure(r.error);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::blWriteWord(uint32_t address, uint32_t value) noexcept {
  if constexpr (!kSpi) {
    // A UART reply carries no reset evidence: select the bank again before every word, so that a
    // restart between two words cannot send the next one to the chip's default bank.
    const auto b = blSetBank(bl_bank_);
    if (!b.ok()) {
      return b;
    }
  }
  const auto a = blExchange(BlCmd::SetAddress, address, true);
  if (!a.ok()) {
    return Result<void>::Failure(a.error);
  }
  const auto w = blExchange(BlCmd::Write32, value, true);
  return w.ok() ? Result<void>::Success() : Result<void>::Failure(w.error);
}

template <AnyPort Port>
Result<uint32_t> TMC9660<Port>::BlRead32(Bank bank, uint32_t address) noexcept {
  if (const Error e = need(Mode::Bootloader); e != Error::Ok) {
    return Result<uint32_t>::Failure(e);
  }
  const bool known_bank = (bank == Bank::SpiFlash) || (bank == Bank::I2cEeprom) ||
                          (bank == Bank::Otp) || (bank == Bank::Config);
  if (((address & 3U) != 0U) || (!known_bank)) {
    return Result<uint32_t>::Failure(Error::InvalidArgument);
  }
  const auto s = blSetBank(bank);
  if (!s.ok()) {
    return Result<uint32_t>::Failure(s.error);
  }
  const auto w = blReadWord(address);
  if (!w.ok()) {
    return w;
  }
  if (const auto live = blLiveness(w.value); !live.ok()) {
    return Result<uint32_t>::Failure(live.error);
  }
  return w;
}

template <AnyPort Port>
Result<void> TMC9660<Port>::BlWrite32(Bank bank, uint32_t address, uint32_t value) noexcept {
  if (const Error e = need(Mode::Bootloader); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  const bool known_bank = (bank == Bank::SpiFlash) || (bank == Bank::I2cEeprom) ||
                          (bank == Bank::Otp) || (bank == Bank::Config);
  if (((address & 3U) != 0U) || (!known_bank)) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if ((bank == Bank::Otp) || (bank == Bank::Config)) {
    return fail(Error::Refused,
                "tmc9660: OTP is never written; the configuration has BlWriteConfig()");
  }
  const auto s = blSetBank(bank);
  if (!s.ok()) {
    return s;
  }
  return blWriteWord(address, value);
}

template <AnyPort Port>
Result<BootConfig> TMC9660<Port>::BlReadConfig() noexcept {
  if (const Error e = need(Mode::Bootloader); e != Error::Ok) {
    return Result<BootConfig>::Failure(e);
  }
  if (const auto s = blSetup(Bank::Config, kConfigMemAddress); !s.ok()) {
    return Result<BootConfig>::Failure(s.error);
  }
  std::array<uint32_t, BootConfig::kWords> words{};
  for (uint32_t i = 0U; i < BootConfig::kWords; ++i) {
    const auto r = blExchange(BlCmd::Read32Inc, 0U, false);
    if (!r.ok()) {
      return Result<BootConfig>::Failure(r.error);
    }
    words[i] = r.value.value;
  }
  if constexpr (kSpi) {
    // The configuration is never all zero (the host and device addresses and the clock word are
    // set after every reset): sixteen zero words are a dead MISO line, not a configuration.
    if (std::none_of(words.begin(), words.end(), [](uint32_t w) noexcept { return w != 0U; })) {
      return Result<BootConfig>::Failure(Error::BadReply);
    }
  }
  const BootConfig read{words};
  const uint32_t boot_mode = read.Get(BootField::BootMode);
  if (boot_mode == static_cast<uint32_t>(BootMode::Parameter)) {
    bl_app_mode_ = Mode::Parameter;
  } else if (boot_mode == static_cast<uint32_t>(BootMode::Register)) {
    bl_app_mode_ = Mode::Register;
  } else {
    // no application named: keep the previous guess
  }
  return Result<BootConfig>::Success(read);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::blWaitPll(const BootConfig& config, std::size_t index) noexcept {
  detail::DelayUs(port_, config_.settle_us);
  bool locked = (config.Get(BootField::ClockPllOutSel) == 0U);
  for (uint32_t k = 0U; (k <= (config_.boot_timeout_ms / 10U)) && !locked; ++k) {
    const auto c = blReadWord(kConfigMemAddress + static_cast<uint32_t>(4U * index));
    if (!c.ok()) {
      return Result<void>::Failure(c.error);
    }
    locked = (c.value & detail::kPllStatus) != 0U;
    if (!locked) {
      detail::DelayUs(port_, detail::kPollUs);
    }
  }
  if (!locked) {
    return fail(Error::Timeout, "tmc9660: the PLL did not lock after the clock change");
  }
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::blWriteChanged(const BootConfig& old,
                                           const BootConfig& config) noexcept {
  for (std::size_t i = 0U; i < BootConfig::kWords; ++i) {
    if (i == kBootModeWordIndex) {
      continue; // written last
    }
    const uint32_t mask = (i == kClockWordIndex) ? ~detail::kPllStatus : 0xFFFF'FFFFU;
    if (((old.Word(i) ^ config.Word(i)) & mask) == 0U) {
      continue;
    }
    const auto w = blWriteWord(kConfigMemAddress + static_cast<uint32_t>(4U * i), config.Word(i));
    if (!w.ok()) {
      return w;
    }
    if (i == 0U) {
      // The addresses take effect after the reply: follow them.
      config_.device_address = static_cast<uint8_t>(config.Get(BootField::DeviceAddress));
      config_.host_address = static_cast<uint8_t>(config.Get(BootField::HostAddress));
    }
    if (i == kClockWordIndex) {
      if (const auto pll = blWaitPll(config, i); !pll.ok()) {
        return pll;
      }
    }
  }
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::blVerifyConfig(const BootConfig& config, bool start) noexcept {
  // Verify everything but the boot-mode word before it is written.
  if (const auto s = blSetup(Bank::Config, kConfigMemAddress); !s.ok()) {
    return s;
  }
  for (std::size_t i = 0U; i < BootConfig::kWords; ++i) {
    const auto r = blExchange(BlCmd::Read32Inc, 0U, false);
    if (!r.ok()) {
      return Result<void>::Failure(r.error);
    }
    if (i == kBootModeWordIndex) {
      continue;
    }
    const uint32_t mask = (i == kClockWordIndex) ? ~detail::kPllStatus : 0xFFFF'FFFFU;
    if (((r.value.value ^ config.Word(i)) & mask) != 0U) {
      return fail(Error::VerifyFailed, "tmc9660: the configuration read back differently");
    }
  }
  if (start) {
    const auto clk = blReadWord(kConfigMemAddress + static_cast<uint32_t>(4U * kClockWordIndex));
    if (!clk.ok()) {
      return Result<void>::Failure(clk.error);
    }
    if ((clk.value & detail::kPllStatus) == 0U) {
      return fail(Error::Refused, "tmc9660: the PLL is not locked: the application is not started");
    }
  }
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::blFollowStart(const BootConfig& config) noexcept {
  detail::DelayUs(port_, config_.settle_us);
  mode_ = (config.Get(BootField::BootMode) == static_cast<uint32_t>(BootMode::Register))
              ? Mode::Register
              : Mode::Parameter;
  initialized_ = false;
  canary_armed_ = false;
  reg_started_ = false;
  if (const auto id = waitForIdentity(); !id.ok()) {
    // Not the application: if the bootloader still answers, nothing started and the driver stays
    // in it; otherwise the application may run unseen and the stop keeps its protocol.
    const Mode target = mode_;
    mode_ = Mode::Bootloader;
    if (verifyIdentity().ok()) {
      initialized_ = true;
      return fail(id.error, "tmc9660: the application did not start; the bootloader still runs");
    }
    mode_ = target;
    return fail(id.error, "tmc9660: the application did not answer after the start");
  }
  if (mode_ == Mode::Parameter) {
    if (const auto p = startParameterMode(); !p.ok()) {
      return p;
    }
  }
  initialized_ = true;
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::blStartWrite(const BootConfig& config) noexcept {
  // Split from blWriteWord(): a failure before the Write32 means the start was never sent; a
  // failure of the Write32 itself (a lost, late, corrupt or BUSY reply) may hide a start the chip
  // already acted on. Only a bootloader error status says it did not.
  if constexpr (!kSpi) {
    if (const auto b = blSetBank(bl_bank_); !b.ok()) {
      return b;
    }
  }
  const auto a = blExchange(
      BlCmd::SetAddress, kConfigMemAddress + static_cast<uint32_t>(4U * kBootModeWordIndex), true);
  if (!a.ok()) {
    return Result<void>::Failure(a.error);
  }
  const auto w = blExchange(BlCmd::Write32, config.Word(kBootModeWordIndex), true);
  if ((w.error == Error::Nack) || (w.error == Error::UnexpectedReset)) {
    return Result<void>::Failure(w.error); // refused, or not sent after a reset the probe saw
  }
  const auto f = blFollowStart(config);
  if (!f.ok() && !w.ok()) {
    return Result<void>::Failure(w.error);
  }
  return f;
}

template <AnyPort Port>
Result<void> TMC9660<Port>::blWriteBootMode(const BootConfig& config, bool start) noexcept {
  if (start) {
    return blStartWrite(config);
  }
  const auto w = blWriteWord(kConfigMemAddress + static_cast<uint32_t>(4U * kBootModeWordIndex),
                             config.Word(kBootModeWordIndex));
  if (!w.ok()) {
    return w;
  }
  const auto back = blReadWord(kConfigMemAddress + static_cast<uint32_t>(4U * kBootModeWordIndex));
  if (!back.ok()) {
    return Result<void>::Failure(back.error);
  }
  if (back.value != config.Word(kBootModeWordIndex)) {
    return fail(Error::VerifyFailed, "tmc9660: the boot-mode word read back differently");
  }
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::BlWriteConfig(const BootConfig& config,
                                          const BlWriteOptions& options) noexcept {
  if (const Error e = need(Mode::Bootloader); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  if (config.CheckStart() != Error::Ok) {
    return fail(
        Error::Refused,
        "tmc9660: the application must not start with a reserved boot mode or without 40 MHz");
  }
  const auto cur = BlReadConfig();
  if (!cur.ok()) {
    return Result<void>::Failure(cur.error);
  }
  const BootConfig& old = cur.value;
  if ((((old.Word(kLdoWordIndex) ^ config.Word(kLdoWordIndex)) & detail::kLdoMask) != 0U) &&
      !options.allow_ldo_change) {
    return fail(Error::Refused, "tmc9660: changing the LDO configuration needs allow_ldo_change");
  }
  // Any write of the boot-mode word with START_MOTOR_CTRL set is treated as a start, also when the
  // bootloader already shows the bit set (A24: whether the bootloader acts on the edge or on the
  // value is not documented).
  const bool start = (config.Get(BootField::StartMotorCtrl) != 0U);
  if (const auto w = blWriteChanged(old, config); !w.ok()) {
    return w;
  }
  if (const auto v = blVerifyConfig(config, start); !v.ok()) {
    return v;
  }
  if (((old.Word(kBootModeWordIndex) ^ config.Word(kBootModeWordIndex)) != 0U)) {
    return blWriteBootMode(config, start);
  }
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::BlStartApplication(BootMode target) noexcept {
  if ((target != BootMode::Register) && (target != BootMode::Parameter)) {
    return fail(Error::Refused,
                "tmc9660: the application is never started with a reserved boot mode");
  }
  const auto cur = BlReadConfig();
  if (!cur.ok()) {
    return Result<void>::Failure(cur.error);
  }
  BootConfig next = cur.value;
  if ((next.Set(BootField::BootMode, static_cast<uint32_t>(target)) != Error::Ok) ||
      (next.Set(BootField::StartMotorCtrl, 1U) != Error::Ok)) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  return BlWriteConfig(next, BlWriteOptions{});
}

template <AnyPort Port>
Result<bool> TMC9660<Port>::BlMemoryStatus(Bank bank, MemQuery query) noexcept {
  if (const Error e = need(Mode::Bootloader); e != Error::Ok) {
    return Result<bool>::Failure(e);
  }
  if (((bank != Bank::SpiFlash) && (bank != Bank::I2cEeprom))) {
    return Result<bool>::Failure(Error::InvalidArgument);
  }
  BlCmd cmd = BlCmd::MemIsConfigured;
  switch (query) {
  case MemQuery::Configured:
    cmd = BlCmd::MemIsConfigured;
    break;
  case MemQuery::Connected:
    cmd = BlCmd::MemIsConnected;
    break;
  case MemQuery::Busy:
    cmd = BlCmd::MemIsBusy;
    break;
  default:
    return Result<bool>::Failure(Error::InvalidArgument);
  }
  const auto s = blSetBank(bank);
  if (!s.ok()) {
    return Result<bool>::Failure(s.error);
  }
  const auto r = blExchange(cmd, 0U, false);
  if (!r.ok()) {
    return Result<bool>::Failure(r.error);
  }
  if (const auto live = blLiveness(r.value.value); !live.ok()) {
    return Result<bool>::Failure(live.error);
  }
  return Result<bool>::Success(r.value.value != 0U);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::BlEraseFlashSector(uint32_t address,
                                               bool allow_partition_table) noexcept {
  if (((address % detail::kFlashSectorBytes) != 0U) || (address >= detail::kFlashAddressLimit)) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if ((address < detail::kFlashSectorBytes) && !allow_partition_table) {
    return fail(Error::Refused,
                "tmc9660: sector 0 holds the partition descriptor: pass allow_partition_table");
  }
  const auto configured = BlMemoryStatus(Bank::SpiFlash, MemQuery::Configured);
  if (!configured.ok()) {
    return Result<void>::Failure(configured.error);
  }
  const auto connected = BlMemoryStatus(Bank::SpiFlash, MemQuery::Connected);
  if (!connected.ok()) {
    return Result<void>::Failure(connected.error);
  }
  if (!configured.value || !connected.value) {
    return fail(Error::Refused, "tmc9660: the SPI flash is not configured and connected");
  }
  const auto busy = BlMemoryStatus(Bank::SpiFlash, MemQuery::Busy);
  if (!busy.ok()) {
    return Result<void>::Failure(busy.error);
  }
  if (busy.value) {
    return Result<void>::Failure(Error::Busy);
  }
  const auto s = blSetBank(Bank::SpiFlash);
  if (!s.ok()) {
    return s;
  }
  const auto e = blExchange(BlCmd::FlashEraseSector, address, true);
  return e.ok() ? Result<void>::Success() : Result<void>::Failure(e.error);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::BlOtpLoad() noexcept {
  if (const Error e = need(Mode::Bootloader); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  const auto r = blExchange(BlCmd::OtpLoad, 0U, true);
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

// -------------------------------------------------------------------------------------------------
// Parameter-mode API
// -------------------------------------------------------------------------------------------------

template <AnyPort Port>
Result<uint32_t> TMC9660<Port>::GetChipId() noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<uint32_t>::Failure(e);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::GetInfo), 0U, 0U, 0U, false, false);
  return r.ok() ? Result<uint32_t>::Success(r.value.value) : Result<uint32_t>::Failure(r.error);
}

template <AnyPort Port>
Result<uint32_t> TMC9660<Port>::GetVersion() noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<uint32_t>::Failure(e);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::GetInfo), 1U, 0U, 0U, false, false);
  return r.ok() ? Result<uint32_t>::Success(r.value.value) : Result<uint32_t>::Failure(r.error);
}

template <AnyPort Port>
Result<int64_t> TMC9660<Port>::readParamRaw(Param p, uint8_t motor) noexcept {
  const ParamInfo* info = FindParam(detail::ToUnderlying(p));
  if ((info == nullptr) || (info->access == Access::WriteOnly) || (motor > 15U)) {
    return Result<int64_t>::Failure(Error::InvalidArgument);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::Gap), info->id, motor, 0U, false, false);
  if (!r.ok()) {
    return Result<int64_t>::Failure(r.error);
  }
  const int64_t v = (info->min < 0) ? static_cast<int64_t>(static_cast<int32_t>(r.value.value))
                                    : static_cast<int64_t>(r.value.value);
  return Result<int64_t>::Success(v);
}

template <AnyPort Port>
Result<int64_t> TMC9660<Port>::GetParam(Param param, uint8_t motor) noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<int64_t>::Failure(e);
  }
  return readParamRaw(param, motor);
}

template <AnyPort Port>
Result<uint32_t> TMC9660<Port>::ReadRawParam(uint16_t type, uint8_t motor) noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<uint32_t>::Failure(e);
  }
  if ((type > 0x0FFFU) || (motor > 15U)) {
    return Result<uint32_t>::Failure(Error::InvalidArgument);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::Gap), type, motor, 0U, false, false);
  if (!r.ok()) {
    return Result<uint32_t>::Failure(r.error);
  }
  return Result<uint32_t>::Success(r.value.value);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::SetParam(Param param, int64_t value, uint8_t motor) noexcept {
  const ParamInfo* info = FindParam(detail::ToUnderlying(param));
  if ((info == nullptr) || (info->access == Access::ReadOnly) || (motor > 15U)) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if (!detail::ParamValueAllowed(*info, value)) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  if ((param == Param::IDLE_MOTOR_PWM_BEHAVIOR) && (value == 0) &&
      !config_.allow_pwm_in_system_off) {
    return fail(Error::Refused,
                "tmc9660: IDLE_MOTOR_PWM_BEHAVIOR 0 keeps the PWM active in System off: set "
                "Config::allow_pwm_in_system_off");
  }
  if ((param == Param::COMMUTATION_MODE) && (value != kSystemOff)) {
    return fail(Error::Refused, "tmc9660: use EnableDrive() to leave System off");
  }
  if (param == Param::MOTOR_TYPE) {
    const auto cm = readParamRaw(Param::COMMUTATION_MODE, 0U);
    if (!cm.ok()) {
      return Result<void>::Failure(cm.error);
    }
    if (cm.value != kSystemOff) {
      return fail(Error::Refused, "tmc9660: MOTOR_TYPE changes only with the system off");
    }
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::Sap), info->id, motor,
                      static_cast<uint32_t>(static_cast<uint64_t>(value)), true, false);
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::ClearFlags(Param flags, uint32_t mask) noexcept {
  const ParamInfo* info = FindParam(detail::ToUnderlying(flags));
  if ((info == nullptr) || (info->access != Access::ReadWriteClear)) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  return SetParam(flags, static_cast<int64_t>(mask), 0U);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::StoreParams() noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::Stap), 0x0FFFU, 0x0FU, 0xFFFF'FFFFU, true, false);
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::FactoryDefault(bool confirm) noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  if (!confirm) {
    return fail(Error::Refused,
                "tmc9660: FactoryDefault clears the stored settings: pass confirm = true");
  }
  const auto cm = readParamRaw(Param::COMMUTATION_MODE, 0U);
  if (!cm.ok()) {
    return Result<void>::Failure(cm.error);
  }
  if (cm.value != kSystemOff) {
    return fail(Error::Refused, "tmc9660: FactoryDefault needs the system off");
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::FactoryDefault), 0U, 0U, 0U, true, false);
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

template <AnyPort Port>
Result<int64_t> TMC9660<Port>::GetGlobal(GlobalBank bank, uint16_t number) noexcept {
  const GlobalInfo* info = FindGlobal(detail::ToUnderlying(bank), number);
  if ((info == nullptr) || (info->access == Access::WriteOnly)) {
    return Result<int64_t>::Failure(Error::InvalidArgument);
  }
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<int64_t>::Failure(e);
  }
  const auto r =
      tmcl(detail::ToUnderlying(Tmcl::Ggp), number, detail::ToUnderlying(bank), 0U, false, false);
  if (!r.ok()) {
    return Result<int64_t>::Failure(r.error);
  }
  const int64_t v = (info->min < 0) ? static_cast<int64_t>(static_cast<int32_t>(r.value.value))
                                    : static_cast<int64_t>(r.value.value);
  return Result<int64_t>::Success(v);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::SetGlobal(GlobalBank bank, uint16_t number, int64_t value) noexcept {
  const GlobalInfo* info = FindGlobal(detail::ToUnderlying(bank), number);
  if ((info == nullptr) || (info->access == Access::ReadOnly) || (value < info->min) ||
      (value > info->max) || (info->odd_only && ((static_cast<uint64_t>(value) & 1U) == 0U))) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::Sgp), number, detail::ToUnderlying(bank),
                      static_cast<uint32_t>(static_cast<uint64_t>(value)), true, false);
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

template <AnyPort Port>
Result<int32_t> TMC9660<Port>::ReferenceSearchCommand(ReferenceSearch action) noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<int32_t>::Failure(e);
  }
  if (detail::ToUnderlying(action) > 2U) {
    return Result<int32_t>::Failure(Error::InvalidArgument);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::Rfs), detail::ToUnderlying(action), 0U, 0U,
                      action != ReferenceSearch::Status, false);
  return r.ok() ? Result<int32_t>::Success(static_cast<int32_t>(r.value.value))
                : Result<int32_t>::Failure(r.error);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::SetIo(uint8_t pin, bool high) noexcept {
  if (pin > 18U) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::Sio), pin, 0U, high ? 1U : 0U, true, false);
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

template <AnyPort Port>
Result<int32_t> TMC9660<Port>::GetIo(uint8_t pin, bool analog) noexcept {
  if (pin > 18U) {
    return Result<int32_t>::Failure(Error::InvalidArgument);
  }
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<int32_t>::Failure(e);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::Gio), pin, analog ? 1U : 0U, 0U, false, false);
  return r.ok() ? Result<int32_t>::Success(static_cast<int32_t>(r.value.value))
                : Result<int32_t>::Failure(r.error);
}

template <AnyPort Port>
Result<int32_t> TMC9660<Port>::RamDebug(RamDebugType type, uint8_t motor_bank,
                                        int32_t value) noexcept {
  if (!detail::IsRamDebugType(type)) {
    return Result<int32_t>::Failure(Error::InvalidArgument);
  }
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<int32_t>::Failure(e);
  }
  const bool read_only =
      (type == RamDebugType::GetState) || (type == RamDebugType::ReadSample) ||
      (type == RamDebugType::GetInfo) || (type == RamDebugType::GetChannelType) ||
      (type == RamDebugType::GetChannelAddress) || (type == RamDebugType::GetPretrigger);
  const auto r = tmcl(detail::ToUnderlying(Tmcl::RamDebug), detail::ToUnderlying(type), motor_bank,
                      static_cast<uint32_t>(value), !read_only, false);
  return r.ok() ? Result<int32_t>::Success(static_cast<int32_t>(r.value.value))
                : Result<int32_t>::Failure(r.error);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::ScriptRun(bool from_address, uint32_t address) noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::ApplRun), from_address ? 1U : 0U, 0U,
                      from_address ? address : 0U, true, false);
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::ScriptStop() noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::ApplStop), 0U, 0U, 0U, false, false);
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::ScriptStep() noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::ApplStep), 0U, 0U, 0U, true, false);
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::ScriptReset() noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::ApplReset), 0U, 0U, 0U, true, false);
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

template <AnyPort Port>
Result<int32_t> TMC9660<Port>::ScriptStatus() noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<int32_t>::Failure(e);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::GetStatus), 0U, 0U, 0U, false, false);
  return r.ok() ? Result<int32_t>::Success(static_cast<int32_t>(r.value.value))
                : Result<int32_t>::Failure(r.error);
}

template <AnyPort Port>
Result<uint32_t> TMC9660<Port>::ScriptReadMem(uint32_t address) noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<uint32_t>::Failure(e);
  }
  const auto r = tmcl(detail::ToUnderlying(Tmcl::ReadMem), 0U, 0U, address, false, false);
  return r.ok() ? Result<uint32_t>::Success(r.value.value) : Result<uint32_t>::Failure(r.error);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::ScriptDownload(std::span<const TmclInstruction> script) noexcept {
  if (script.empty() || (script.size() > 255U)) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  for (const TmclInstruction& ins : script) {
    if (!detail::IsScriptOp(ins.op) || (ins.type > 0x0FFFU) || (ins.motor > 15U)) {
      return Result<void>::Failure(Error::InvalidArgument);
    }
  }
  for (const TmclInstruction& ins : script) {
    if (detail::IsEnergisingScriptOp(ins.op, ins.type, ins.value)) {
      return fail(Error::Refused,
                  "tmc9660: a script must not leave System off: only EnableDrive() does (SAP or "
                  "AAP of COMMUTATION_MODE)");
    }
  }
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  const auto start = tmcl(detail::ToUnderlying(Tmcl::DownloadStart), 0U, 0U, 0U, true, false);
  if (!start.ok()) {
    return Result<void>::Failure(start.error);
  }
  Result<void> first = Result<void>::Success();
  for (const TmclInstruction& ins : script) {
    const auto r = tmcl(detail::ToUnderlying(ins.op), ins.type, ins.motor,
                        static_cast<uint32_t>(ins.value), true, true);
    if (!r.ok()) {
      first = Result<void>::Failure(r.error);
      break;
    }
  }
  const auto end = tmclRaw(
      detail::TmclCommand{
          .op = detail::ToUnderlying(Tmcl::DownloadEnd), .type = 0U, .motor = 0U, .value = 0U},
      false, false);
  if (first.ok() && !end.ok()) {
    first = Result<void>::Failure(end.error);
  }
  return first;
}

template <AnyPort Port>
Result<void> TMC9660<Port>::PowerDown(PowerDownTime time) noexcept {
  if (detail::ToUnderlying(time) > 7U) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  if (const auto off = DisableDrive(); !off.ok()) {
    return off;
  }
  if (!initialized_) {
    // The stop met a reset it then confirmed: nothing else is sent until the driver starts again.
    return Result<void>::Failure(Error::UnexpectedReset);
  }
  const auto r = SetGlobal(GlobalBank::System, global0::GO_TO_TIMEOUT_POWER_DOWN_STATE,
                           detail::ToUnderlying(time));
  if (!r.ok()) {
    return r;
  }
  initialized_ = false; // the chip restarts when the time is over
  canary_armed_ = false;
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::ReturnToBootloader() noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  if (const auto off = DisableDrive(); !off.ok()) {
    return fail(off.error, "tmc9660: the drive could not be switched off; no Boot sent");
  }
  if (!initialized_) {
    return fail(Error::UnexpectedReset,
                "tmc9660: the chip restarted during the stop; no Boot sent");
  }
  const auto cm = readParamRaw(Param::COMMUTATION_MODE, 0U);
  if (!cm.ok()) {
    return Result<void>::Failure(cm.error);
  }
  if (cm.value != kSystemOff) {
    return fail(Error::Refused, "tmc9660: Boot needs the system off (erratum 3)");
  }
  const auto b = tmcl(detail::ToUnderlying(Tmcl::Boot), 0x0081U, 0x92U, kBootMagic, true, false);
  initialized_ = false; // the chip restarts: every earlier assumption is void
  canary_armed_ = false;
  if ((b.error == Error::Nack) || (b.error == Error::UnexpectedReset) ||
      (b.error == Error::NotInitialized)) {
    return Result<void>::Failure(b.error); // refused or not sent: still in parameter mode
  }
  // Any other outcome, including a lost reply, may still be a restart: the bootloader's identity
  // decides.
  mode_ = Mode::Bootloader;
  detail::DelayUs(port_, config_.settle_us);
  if (const auto id = waitForIdentity(); !id.ok()) {
    mode_ = Mode::Parameter;
    return fail(b.ok() ? id.error : b.error, "tmc9660: the bootloader did not answer after Boot");
  }
  initialized_ = true;
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::DisableSpi1Slave() noexcept {
  if (const Error e = need(Mode::Parameter); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  const auto r = tmcl(detail::kRegUartWrite, 4U, 9U, 0U, true, false);
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

// -------------------------------------------------------------------------------------------------
// Register-mode API
// -------------------------------------------------------------------------------------------------

template <AnyPort Port>
Error TMC9660<Port>::energisingGuard(RegAddr addr, uint32_t value) const noexcept {
  if ((addr.block == regs::MCC_GDRV_HW.block) && (addr.offset == regs::MCC_GDRV_HW.offset) &&
      ((value & detail::kGdrvHwEnergise) != 0U)) {
    return Error::Refused;
  }
  if ((addr.block == regs::MCC_PWM_CONFIG.block) && (addr.offset == regs::MCC_PWM_CONFIG.offset) &&
      ((value & detail::kPwmEnergise) != 0U)) {
    return Error::Refused;
  }
  return Error::Ok;
}

template <AnyPort Port>
Result<uint32_t> TMC9660<Port>::ReadReg(RegAddr addr) noexcept {
  if (FindRegister(addr) == nullptr) {
    return Result<uint32_t>::Failure(Error::InvalidArgument);
  }
  if (const Error e = need(Mode::Register); e != Error::Ok) {
    return Result<uint32_t>::Failure(e);
  }
  return regExchange(false, addr, 0U);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::WriteReg(RegAddr addr, uint32_t value) noexcept {
  const RegInfo* info = FindRegister(addr);
  if ((info == nullptr) || (info->writable_mask == 0U)) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if (const Error e = need(Mode::Register); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  if (energisingGuard(addr, value) != Error::Ok) {
    return fail(Error::Refused,
                "tmc9660: this write energises the gate driver: use GateDriverStart()");
  }
  const auto r = regExchange(true, addr, value);
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::UpdateReg(RegAddr addr, uint32_t mask, uint32_t value) noexcept {
  const RegInfo* info = FindRegister(addr);
  if ((info == nullptr) || ((mask & ~info->writable_mask) != 0U) || ((value & ~mask) != 0U) ||
      (mask == 0U)) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if (const Error e = need(Mode::Register); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  const auto cur = regExchange(false, addr, 0U);
  if (!cur.ok()) {
    return Result<void>::Failure(cur.error);
  }
  const uint32_t next = (cur.value & ~mask) | value;
  if (energisingGuard(addr, next & ~cur.value) != Error::Ok) {
    return fail(Error::Refused,
                "tmc9660: this update energises the gate driver: use GateDriverStart()");
  }
  const auto r = regExchange(true, addr, next);
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

template <AnyPort Port>
Result<uint32_t> TMC9660<Port>::RegRamDebug(uint8_t subcommand, uint8_t index, bool write,
                                            uint32_t value) noexcept {
  const uint32_t bits = config_.quirks.ram_debug_index_bits;
  if ((subcommand > 15U) || (static_cast<uint32_t>(index) >= (1U << bits))) {
    return Result<uint32_t>::Failure(Error::InvalidArgument);
  }
  if (const Error e = need(Mode::Register); e != Error::Ok) {
    return Result<uint32_t>::Failure(e);
  }
  const auto offset = static_cast<uint16_t>((static_cast<uint32_t>(subcommand) << bits) | index);
  return regExchangeRaw(detail::kRegUartRamDebug, static_cast<uint8_t>(Block::RamDebug), offset,
                        write, value, write);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::regPut(RegAddr addr, uint32_t value) noexcept {
  const auto r = regExchange(true, addr, value);
  return r.ok() ? Result<void>::Success() : Result<void>::Failure(r.error);
}

template <AnyPort Port>
Result<bool> TMC9660<Port>::gdrvLatched() noexcept {
  const auto status = regExchange(false, regs::MCC_GDRV_STATUS, 0U);
  if (!status.ok()) {
    return Result<bool>::Failure(status.error);
  }
  const auto fault = regExchange(false, regs::MCC_GDRV_FAULT, 0U);
  if (!fault.ok()) {
    return Result<bool>::Failure(fault.error);
  }
  return Result<bool>::Success(detail::GdrvLatched(status.value, fault.value));
}

template <AnyPort Port>
Result<void> TMC9660<Port>::gdrvSetup(const GateDriverRegConfig& config) noexcept {
  // Steps 1 to 4 of the manual, then the protection words: configuration only, nothing is
  // energised (the ENABLE bits and CHOP = 1 are set up but the gate driver itself stays off).
  struct Word {
    RegAddr addr;
    uint32_t value;
  };
  const std::array<Word, 9> words{
      {{.addr = regs::MCC_PWM_CONFIG, .value = config.pwm_enable | 1U}, // ENABLE_*, CHOP 1
       {.addr = regs::MCC_GDRV_CFG, .value = config.gdrv_cfg},
       {.addr = regs::MCC_GDRV_TIMING, .value = config.gdrv_timing},
       {.addr = regs::MCC_GDRV_BBM, .value = config.gdrv_bbm},
       {.addr = regs::MCC_GDRV_PROT, .value = config.gdrv_prot},
       {.addr = regs::MCC_GDRV_OCP_UVW, .value = config.gdrv_ocp_uvw},
       {.addr = regs::MCC_GDRV_OCP_Y2, .value = config.gdrv_ocp_y2},
       {.addr = regs::MCC_GDRV_STATUS_EN, .value = 0U}, // "disables any faults at start"
       {.addr = regs::MCC_GDRV_STATUS, .value = 0xFFFF'FFFFU}}};
  for (const Word& w : words) {
    if (const auto r = regPut(w.addr, w.value); !r.ok()) {
      return r;
    }
  }
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::gdrvEnergise(const GateDriverRegConfig& config) noexcept {
  if (const auto r = regPut(regs::MCC_GDRV_HW, config.gdrv_hw); !r.ok()) {
    return r;
  }
  const auto back = regExchange(false, regs::MCC_GDRV_HW, 0U);
  if (!back.ok()) {
    return Result<void>::Failure(back.error);
  }
  if (back.value != config.gdrv_hw) {
    return Result<void>::Failure(Error::VerifyFailed);
  }
  return regPut(regs::MCC_PWM_CONFIG, config.pwm_enable | config.chop_run);
}

template <AnyPort Port>
Result<void> TMC9660<Port>::gdrvProtect(const GateDriverRegConfig& config) noexcept {
  // The manual's "Protection Setup": the bootstrap capacitors charge first, then fault reporting
  // (STATUS_EN), then the protection handling (PROT_EN). Without these two writes a shorted
  // bridge is neither reported nor shut off by the chip.
  detail::DelayUs(port_, config_.settle_us);
  if (const auto r = regPut(regs::MCC_GDRV_STATUS_EN, config.status_en); !r.ok()) {
    return r;
  }
  if (const auto r = regPut(regs::MCC_GDRV_PROT_EN, config.prot_en); !r.ok()) {
    return r;
  }
  const auto se = regExchange(false, regs::MCC_GDRV_STATUS_EN, 0U);
  const auto pe = regExchange(false, regs::MCC_GDRV_PROT_EN, 0U);
  if (!se.ok()) {
    return Result<void>::Failure(se.error);
  }
  if (!pe.ok()) {
    return Result<void>::Failure(pe.error);
  }
  if (((se.value & detail::WritableMask(regs::MCC_GDRV_STATUS_EN)) != config.status_en) ||
      ((pe.value & detail::WritableMask(regs::MCC_GDRV_PROT_EN)) != config.prot_en)) {
    return Result<void>::Failure(Error::VerifyFailed);
  }
  return Result<void>::Success();
}

template <AnyPort Port>
Result<void> TMC9660<Port>::GateDriverStart(const GateDriverRegConfig& config) noexcept {
  if (!detail::GdrvConfigValid(config)) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if (const Error e = need(Mode::Register); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  // The latched evidence is read first, before anything is written or cleared: the clear below
  // (a step of the manual) would destroy it, and with MCC_GDRV_STATUS_EN at 0 the later read sees
  // nothing at all. A latched bit is cleared by the caller (write 1 to it) after looking at it.
  const auto before = gdrvLatched();
  if (!before.ok()) {
    return Result<void>::Failure(before.error);
  }
  if (before.value) {
    static_cast<void>(DisableDrive());
    return fail(Error::DeviceFault,
                "tmc9660: a latched gate-driver fault forbids the start: read and clear it");
  }
  Result<void> r = gdrvSetup(config);
  if (r.ok()) {
    // A second look after the clear: a condition that is still there latches again.
    const auto after = gdrvLatched();
    if (!after.ok()) {
      r = Result<void>::Failure(after.error);
    } else if (after.value) {
      r = fail(Error::DeviceFault,
               "tmc9660: a gate-driver fault or undervoltage forbids the start");
    } else {
      r = setDrvEn(GpioSignal::ACTIVE);
    }
  }
  if (r.ok()) {
    r = gdrvEnergise(config);
  }
  if (r.ok()) {
    r = gdrvProtect(config);
  }
  if (r.ok()) {
    // A protection that trips at once (for example the reset OCP words with every protection on)
    // latches now: the start is not Ok with the stage already shut off by the chip.
    const auto tripped = gdrvLatched();
    if (!tripped.ok()) {
      r = Result<void>::Failure(tripped.error);
    } else if (tripped.value) {
      r = fail(Error::DeviceFault, "tmc9660: the gate driver tripped right after the start");
    } else {
      // running
    }
  }
  if (!r.ok()) {
    static_cast<void>(DisableDrive());
  }
  return r;
}

} // namespace tmc9660

#endif // TMC9660_IPP
