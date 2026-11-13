#!/usr/bin/env python3
"""One-edit mutants of the driver's safety guards: every one must make the host tests fail.

  TMC9660_GENERATED=<build dir>/tmc9660_generated tools/mutants/mutants.py [--jobs N] [--only ID]

TMC9660_GENERATED names the directory with the generated tmc9660_version.h (the build tree's tmc9660_generated).

Each mutant is one textual edit (file, old text, new text, occurrence) applied to a copy of inc/, src/ and tests/
in a scratch directory; the copy is compiled with the host test flags and run. A mutant is killed when any test fails (a static_assert of the
frame proofs counts); an edit that does not compile is reported BROKEN and fails the run. The script exits 1 when a mutant survives.
Equivalent mutants (an edit that cannot change behaviour because a second guard covers it) are listed in
EQUIVALENT with the reason, and are not run. Needs a C++20 compiler in $CXX (default c++).
"""
import concurrent.futures, os, shutil, subprocess, sys, tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
IPP = 'src/tmc9660.ipp'
HPP = 'inc/tmc9660.hpp'
TYPES = 'inc/tmc9660_types.hpp'

# (id, file, old, new, occurrence of old, what the edit removes)
MUTANTS = [
    ('enable-skips-gdrv-flags', IPP, '(err.value != 0) || (gdrv.value != 0)', '(err.value != 0)', 1,
     'EnableDrive stops reading the gate driver flags'),
    ('enable-skips-general-flags', IPP, '(err.value != 0) || (gdrv.value != 0)', '(gdrv.value != 0)', 1,
     'EnableDrive stops reading the general error flags'),
    ('enable-mode-bound', IPP, 'commutation_mode > static_cast<uint8_t>(kCommutationModeMax)',
     'commutation_mode >= static_cast<uint8_t>(kCommutationModeMax)', 1, 'EnableDrive bound off by one'),
    ('enable-accepts-zero', IPP, '(commutation_mode == 0U) ||', '(false) ||', 1,
     'EnableDrive accepts mode 0'),
    ('setparam-commutation', IPP, '(param == Param::COMMUTATION_MODE) && (value != kSystemOff)', 'false', 1,
     'SetParam may leave System off'),
    ('setparam-motor-type', IPP, 'if (cm.value != kSystemOff) {\n      return fail(Error::Refused, "tmc9660: MOTOR_TYPE',
     'if (false) {\n      return fail(Error::Refused, "tmc9660: MOTOR_TYPE', 1, 'MOTOR_TYPE with the system on'),
    ('setparam-range', IPP, 'valid = valid || ((value >= info.min) && (value <= info.max));', 'valid = true;', 1,
     'SetParam stops checking ranges'),
    ('setparam-range-top', IPP, '(value <= info.max));', '(value <= info.max + 1));', 1, 'range bound off by one'),
    ('setparam-readonly', IPP, '(info->access == Access::ReadOnly) || (motor > 15U)', '(motor > 15U)', 1,
     'SetParam writes read-only parameters'),
    ('readparam-writeonly', IPP, '(info->access == Access::WriteOnly) || (motor > 15U)', '(motor > 15U)', 1,
     'GetParam reads write-only parameters'),
    ('clearflags-class', IPP, '(info->access != Access::ReadWriteClear)', 'false', 1,
     'ClearFlags on a non-flag parameter'),
    ('factory-confirm', IPP, 'if (!confirm) {', 'if (false) {', 1, 'FactoryDefault without confirm'),
    ('factory-system-on', IPP, 'if (cm.value != kSystemOff) {\n    return fail(Error::Refused, "tmc9660: FactoryDefault',
     'if (false) {\n    return fail(Error::Refused, "tmc9660: FactoryDefault', 1, 'FactoryDefault with the system on'),
    ('disable-pin-first', IPP, 'Result<void> first = setDrvEn(GpioSignal::INACTIVE);',
     'Result<void> first = Result<void>::Success();', 1, 'DisableDrive never drops the pin'),
    ('disable-chip-parameter', IPP, 'chip = disableDriveParameter();', 'chip = Result<void>::Success();', 1,
     'DisableDrive never writes COMMUTATION_MODE 0'),
    ('disable-chip-register', IPP, 'chip = disableDriveRegister();', 'chip = Result<void>::Success();', 1,
     'DisableDrive never zeroes the register-mode enables'),
    ('disable-pwm-register', IPP, 'const auto b = clear(regs::MCC_PWM_CONFIG);',
     'const auto b = Result<uint32_t>::Success(0U);', 1, 'disable never zeroes the PWM word'),
    ('reset-probe-spi', IPP, 'if (const Error f = spiFirstCommand(); f != Error::Ok) {', 'if (const Error f = Error::Ok; f != Error::Ok) {', 2,
     'the write probe ignores FIRST_CMD'),
    ('reset-command-spi', IPP, 'if (const Error f = spiFirstCommand(); f != Error::Ok) {', 'if (const Error f = Error::Ok; f != Error::Ok) {', 3,
     'the command frame ignores FIRST_CMD'),
    ('reset-reply-spi', IPP, 'if (const Error f = spiFirstCommand(); f != Error::Ok) {', 'if (const Error f = Error::Ok; f != Error::Ok) {', 1,
     'the reply request ignores FIRST_CMD'),
    ('canary-off', IPP, 'if (write && config_.uart_canary && canary_armed_) {', 'if (false) {', 1,
     'UART writes without the canary check'),
    ('canary-compare', IPP, 'static_cast<int32_t>(r.value.value) != config_.canary_value',
     'static_cast<int32_t>(r.value.value) == 0x7FFFFFFF', 1, 'the canary value is not compared'),
    ('retries-parameter-off-by-one', IPP, 'attempt <= config_.spi_reply_retries;', 'attempt < config_.spi_reply_retries;',
     2, 'one reply-request attempt fewer (TMCL)'),
    ('retries-parameter-extra', IPP, 'attempt <= config_.spi_reply_retries;', 'attempt <= config_.spi_reply_retries + 1U;',
     2, 'one reply-request attempt more (TMCL)'),
    ('retries-bootloader-off-by-one', IPP, 'attempt <= config_.spi_reply_retries;', 'attempt < config_.spi_reply_retries;', 1,
     'one fetch fewer (bootloader)'),
    ('retries-bootloader-extra', IPP, 'attempt <= config_.spi_reply_retries;', 'attempt <= config_.spi_reply_retries + 1U;', 1,
     'one fetch more (bootloader)'),
    ('boot-needs-off', IPP, 'return fail(Error::Refused, "tmc9660: Boot needs the system off (erratum 3)");',
     'static_cast<void>(0);', 1, 'Boot sent with the system on'),
    ('blconfig-checkstart', IPP, 'if (config.CheckStart() != Error::Ok) {', 'if (false) {', 1,
     'BlWriteConfig accepts an unsafe start'),
    ('blconfig-ldo', IPP, '!options.allow_ldo_change) {', 'false) {', 1, 'LDO change without allow_ldo_change'),
    ('blconfig-bootword-last', IPP, 'if (i == kBootModeWordIndex) {\n      continue;', 'if (false) {\n      continue;', 1,
     'the boot-mode word is written in order, not last'),
    ('blconfig-verify', IPP, 'return fail(Error::VerifyFailed, "tmc9660: the configuration read back differently");',
     'static_cast<void>(0);', 1, 'no verify before the boot-mode word'),
    ('blconfig-pll-timeout', IPP, 'return fail(Error::Timeout, "tmc9660: the PLL did not lock after the clock change");',
     'static_cast<void>(0);', 1, 'no PLL lock wait'),
    ('blconfig-pll-before-start', IPP,
     'return fail(Error::Refused, "tmc9660: the PLL is not locked: the application is not started");',
     'static_cast<void>(0);', 1, 'application started with the PLL unlocked'),
    ('blwrite-otp', IPP, 'if ((bank == Bank::Otp) || (bank == Bank::Config)) {', 'if (bank == Bank::Config) {', 1,
     'BlWrite32 reaches the OTP'),
    ('erase-configured', IPP, 'if (!configured.value || !connected.value) {', 'if (false) {', 1,
     'erase without a configured flash'),
    ('erase-busy', IPP, 'if (busy.value) {', 'if (false) {', 1, 'erase while the flash is busy'),
    ('mask-gdrv', 'src/tmc9660.ipp', 'inline constexpr uint32_t kGdrvHwEnergise = 0x0300\'080FU;',
     'inline constexpr uint32_t kGdrvHwEnergise = 0x0300\'0800U;', 1, 'an energising bit leaves the guard'),
    ('mask-pwm', 'src/tmc9660.ipp', 'inline constexpr uint32_t kPwmEnergise = 0x0000\'FF07U;',
     'inline constexpr uint32_t kPwmEnergise = 0x0000\'FF00U;', 1, 'the chop bits leave the guard'),
    ('writereg-guard', IPP, 'if (energisingGuard(addr, value) != Error::Ok) {', 'if (false) {', 1,
     'WriteReg energises'),
    ('updatereg-guard', IPP, 'energisingGuard(addr, next & ~cur.value)', 'energisingGuard(addr, 0U)', 1,
     'UpdateReg energises'),
    ('gds-latched', IPP, '} else if (after.value) {', '} else if (false) {', 1,
     'GateDriverStart ignores the flags read after the clear'),
    ('gds-latched-pre', IPP, 'if (before.value) {', 'if (false) {', 1,
     'GateDriverStart ignores the latched evidence read before the clear (R1-3)'),
    ('gds-clear-before-read', IPP, 'const auto before = gdrvLatched();',
     'static_cast<void>(regPut(regs::MCC_GDRV_STATUS, 0xFFFF\'FFFFU));\n  const auto before = gdrvLatched();', 1,
     'the latched status is cleared before it is read (R1-3)'),
    ('gds-status-en-write', IPP, 'regPut(regs::MCC_GDRV_STATUS_EN, config.status_en)', 'Result<void>::Success()', 1,
     'STATUS_EN is never set after the start (R1-4)'),
    ('gds-prot-en-write', IPP, 'regPut(regs::MCC_GDRV_PROT_EN, config.prot_en)', 'Result<void>::Success()', 1,
     'PROT_EN is never set after the start (R1-4)'),
    ('gds-protection-verify', IPP, '((pe.value & detail::WritableMask(regs::MCC_GDRV_PROT_EN)) != config.prot_en)', 'false', 1,
     'PROT_EN is not read back (R1-4)'),
    ('gds-status-verify', IPP, '(((se.value & detail::WritableMask(regs::MCC_GDRV_STATUS_EN)) != config.status_en) ||', '((false) ||', 1,
     'STATUS_EN is not read back (R1-4)'),
    ('gds-prot-write', IPP, '{.addr = regs::MCC_GDRV_PROT, .value = config.gdrv_prot},', '{.addr = regs::MCC_GDRV_PROT, .value = 0U},', 1,
     'GDRV_PROT is not written'),
    ('gds-hw-verify', IPP, 'if (back.value != config.gdrv_hw) {', 'if (false) {', 1,
     'GDRV_HW is not read back'),
    ('gds-chop', IPP, '(c.chop_run >= 1U)', 'true', 1,
     'GateDriverStart accepts chop 0'),
    ('reset-marks', IPP, 'void TMC9660<Port>::markReset() noexcept {\n  initialized_ = false;',
     'void TMC9660<Port>::markReset() noexcept {\n  initialized_ = initialized_;', 1,
     'a detected reset leaves the driver initialised'),
    ('mode-check', IPP, 'if (mode_ != m) {\n    return Error::WrongMode;', 'if (false) {\n    return Error::WrongMode;', 1,
     'a call of another mode is sent'),
    ('init-check', IPP, 'if (!initialized_) {\n    return Error::NotInitialized;\n  }\n  if (mode_ != m) {',
     'if (false) {\n    return Error::NotInitialized;\n  }\n  if (mode_ != m) {', 1, 'calls before the start are sent'),
    ('reg-probe', IPP, 'if (probe) {\n    if (const Error pe = regSpiDatagram(nop, rep);',
     'if (false) {\n    if (const Error pe = regSpiDatagram(nop, rep);', 1, 'register writes without the probe'),
    ('reg-counter', IPP, 'if (r.boot || (r.counter != reg_counter_)) {', 'if (r.boot) {', 1,
     'the datagram counter is not compared'),
    ('reg-boot-bit', IPP, 'if (r.boot || (r.counter != reg_counter_)) {', 'if (r.counter != reg_counter_) {', 1,
     'the boot bit is not checked'),
    ('service-faultn', IPP, 'if (!high) {\n        markReset();', 'if (high) {\n        markReset();', 1,
     'FAULTN polarity'),
    ('hwreset-needs-gpio', IPP, 'config_.hardware_reset && !detail::HasGpioSet<Port>', 'false', 1,
     'hardware_reset without a GpioSet hook'),
    ('uart-odd-address', IPP, '(config_.device_address & 1U) == 0U', 'false', 1, 'even UART device address'),
    ('validate-canary', TYPES, '(c.canary_variable > 15U)', '(c.canary_variable > 16U)', 1,
     'canary variable above 15'),
    ('validate-ram-bits', TYPES, '(c.quirks.ram_debug_index_bits > 6U)', '(c.quirks.ram_debug_index_bits > 7U)', 1,
     'RAMDebug index of 7 bits'),
    ('setio-range', IPP, 'if (pin > 18U) {', 'if (pin > 19U) {', 1, 'SetIo pin bound'),
    ('global-odd', IPP, '(info->odd_only && ((static_cast<uint64_t>(value) & 1U) == 0U))', 'false', 1,
     'SERIAL_ADDRESS accepts even values'),
    ('crc8-poly', 'inc/tmc9660_frames.hpp', '0x07U', '0x09U', 1, 'CRC-8 polynomial'),
    ('deinit-early-return', IPP, 'const Result<void> r = DisableDrive();\n  initialized_ = false;',
     'if (!initialized_) {\n    return Result<void>::Success();\n  }\n  const Result<void> r = DisableDrive();\n  initialized_ = false;', 1,
     'Deinitialize sends nothing after a reported reset (R1-1)'),
    ('stop-param-retry', IPP, 'auto r = tmcl(detail::ToUnderlying(Tmcl::Mst), 0U, 0U, 0U, false,\n                false); // a stop is never gated by a probe\n  if (!r.ok()) {',
     'auto r = tmcl(detail::ToUnderlying(Tmcl::Mst), 0U, 0U, 0U, false,\n                false); // a stop is never gated by a probe\n  if (false) {', 1,
     'MotorStop is never repeated (R1-8, R3-1)'),
    ('stop-register-retry', IPP, 'if (!w.ok()) {\n      // The first datagram only reported a reset, or the write failed: write once more.\n      w = regExchangeRaw(detail::kRegUartWrite',
     'if (w.error == Error::Bus) {\n      // The first datagram only reported a reset, or the write failed: write once more.\n      w = regExchangeRaw(detail::kRegUartWrite', 1, 'register stop does not write again after a reset (R1-8)'),
    ('stop-register-verify', IPP, 'first = Result<void>::Failure(Error::VerifyFailed);', 'static_cast<void>(0);', 1,
     'register stop does not verify by reading back (R1-7)'),
    ('stop-register-verify-pwm', IPP, '((rb.value & (detail::kPwmEnergise | detail::kPwmEnableBits)) != 0U)', 'false', 1,
     'register stop does not look at the PWM read-back (R1-7)'),
    ('layout-ambiguous', IPP, 'if (answered != 1U) {', 'if (answered == 0U) {', 1,
     'both TMCL layouts answering is accepted (R1-2)'),
    ('layout-verify-skipped', IPP, 'if (const auto v = verifyTmclLayout(); !v.ok()) {\n    return v;\n  }',
     'static_cast<void>(0);', 1, 'the TMCL layout is not proved at the start (R1-2)'),
    ('layout-explicit-unchecked', IPP, 'if (!r.value) {\n      return fail(Error::Refused,\n                  "tmc9660: the chip does not answer the configured',
     'if (false) {\n      return fail(Error::Refused,\n                  "tmc9660: the chip does not answer the configured', 1,
     'an explicit wrong layout is used (R1-2)'),
    ('resync-off', IPP, 'if (uart_dirty_ && !defer_resync_) {\n      if (const Error r = uartResync();', 'if (false) {\n      if (const Error r = uartResync();', 1,
     'no resynchronisation after a UART failure (R1-5)'),
    ('resync-no-drain', IPP, 'if (s != BusStatus::Ok) {\n        break; // quiet', 'if (true) {\n        break; // quiet', 1, 'the line is not drained (R1-5)'),
    ('resync-no-wait', IPP, 'detail::DelayUs(port_, config_.uart_resync_us);\n    for', 'for', 1,
     'no gap before the drain (R1-5)'),
    ('resync-dirty-not-set', IPP, 'if (e != Error::Ok) {\n      uart_dirty_ = true;\n    }', 'static_cast<void>(0);', 1,
     'a failed exchange does not mark the line dirty (R1-5)'),
    ('script-sap-guard', IPP, '((op == Tmcl::Sap) && (value != 0))', 'false', 1, 'SAP COMMUTATION_MODE in a script (R1-6)'),
    ('script-aap-guard', IPP, '|| (op == Tmcl::Aap));', '|| false);', 1, 'AAP COMMUTATION_MODE in a script (R1-6)'),
    ('script-guard-type', IPP, '(type == static_cast<uint16_t>(ToUnderlying(Param::COMMUTATION_MODE)))', 'true', 1,
     'every SAP in a script is refused'),
    ('watchdog-check', IPP, 'if (!watchdog_at_start_ && ((static_cast<uint64_t>(r.value) & detail::kWatchdogEvent) != 0U)) {',
     'if (false) {', 1, 'Service ignores WATCHDOG_EVENT (R1-11)'),
    ('watchdog-baseline', IPP, 'if (!watchdog_at_start_ && ((static_cast<uint64_t>(r.value)', 'if (((static_cast<uint64_t>(r.value)', 1,
     'a flag set before the start counts as a reset (R1-11)'),
    ('faultn-guard-write', IPP, 'if (const Error g = faultnGuard(); g != Error::Ok) {\n      return Result<uint32_t>',
     'if (const Error g = Error::Ok; g != Error::Ok) {\n      return Result<uint32_t>', 1,
     'register UART writes ignore FAULTN (R1-11)'),
    ('bl-setbank-per-word', IPP, 'const auto b = blSetBank(bl_bank_);\n    if (!b.ok()) {\n      return b;\n    }\n  }\n  const auto a = blExchange(BlCmd::SetAddress, address, true);',
     'static_cast<void>(0);\n  }\n  const auto a = blExchange(BlCmd::SetAddress, address, true);', 1,
     'UART bootloader words without a SetBank (R1-11)'),
    ('bl-liveness', IPP, 'if (value == 0U) {\n      const auto id = blExchange(BlCmd::GetInfo', 'if (false) {\n      const auto id = blExchange(BlCmd::GetInfo', 1,
     'a dead MISO line reads as zero (m5)'),
    ('bl-config-zero', IPP, 'if (std::none_of(words.begin(), words.end(), [](uint32_t w) noexcept { return w != 0U; })) {', 'if (false) {', 1, 'an all-zero configuration is accepted (m5)'),
    ('erase-align', IPP, '((address % detail::kFlashSectorBytes) != 0U) ||', '(false) ||', 1, 'erase of an unaligned address (m6)'),
    ('erase-limit', IPP, '(address >= detail::kFlashAddressLimit)', 'false', 1, 'erase above the flash (m6)'),
    ('erase-sector0', IPP, '(address < detail::kFlashSectorBytes) && !allow_partition_table', 'false', 1,
     'erase of sector 0 without the flag (m6)'),
    ('spi-tx-copy', IPP, 'return port_.Transfer(tx_.data(), rx_.data(), len) ? Error::Ok : Error::Bus;',
     'const std::array<uint8_t, 9> copy = tx_;\n    return port_.Transfer(copy.data(), rx_.data(), len) ? Error::Ok : Error::Bus;', 1,
     'the SPI port is given a stack copy, not the driver member (R1-9)'),
    ('uart-tx-copy', IPP, 'port_.Exchange(tx_.data(), tx_len, rx_.data(), rx_len, config_.reply_timeout_us));',
     'port_.Exchange(std::array<uint8_t, 9>(tx_).data(), tx_len, rx_.data(), rx_len, config_.reply_timeout_us));', 1,
     'the UART port is given a temporary copy, not the driver member (R1-9)'),
    # --- round 2 (R2-1 .. R2-3, m1 .. m12) ---
    ('idle-safe-skipped', IPP, 'return idleSafe(stop); // no DRV_EN pin to fall back on: System off must switch the PWM off',
     'return Result<void>::Success();', 1, 'no DRV_EN hook: the stop never looks at IDLE_MOTOR_PWM_BEHAVIOR (R2-1)'),
    ('idle-safe-unconditional', IPP, 'if constexpr (!detail::HasGpioSet<Port>) {\n    return idleSafe(stop);',
     'if constexpr (true) {\n    return idleSafe(stop);', 1, 'the parameter is rewritten even when the DRV_EN pin switches the stage off (R2-1)'),
    ('idle-safe-read-error', IPP, 'if (!idle.ok()) {\n    return Result<void>::Failure(idle.error);',
     'if (!idle.ok()) {\n    return Result<void>::Success();', 1, 'a lost read of the parameter is an Ok stop (R2-1)'),
    ('idle-safe-verify', IPP, 'return (back.value == 1) ? Result<void>::Success() : Result<void>::Failure(Error::VerifyFailed);',
     'return Result<void>::Success();', 1, 'the read-back of IDLE_MOTOR_PWM_BEHAVIOR is not checked (R2-1)'),
    ('idle-safe-no-restop', IPP, 'const auto again = tmclRaw(stop, false, false);',
     'const auto again = Result<detail::TmclReply>::Success(detail::TmclReply{});', 1,
     'System off is not entered again after the parameter is set (R2-1)'),
    ('idle-safe-writes-zero', IPP, '.motor = 0U,\n          .value = 1U},\n      false, false);\n  if (!set.ok())',
     '.motor = 0U,\n          .value = 0U},\n      false, false);\n  if (!set.ok())', 1, 'the stop writes 0 to IDLE_MOTOR_PWM_BEHAVIOR (R2-1)'),
    ('setparam-idle-guard', IPP, '(param == Param::IDLE_MOTOR_PWM_BEHAVIOR) && (value == 0) &&\n      !config_.allow_pwm_in_system_off',
     'false', 1, 'SetParam(IDLE_MOTOR_PWM_BEHAVIOR, 0) is never refused (R2-1)'),
    ('setparam-idle-override', IPP, '(value == 0) &&\n      !config_.allow_pwm_in_system_off', '(value == 0)', 1,
     'the Config override is ignored (R2-1)'),
    ('bl-busy-resend', IPP, 'const auto r = blSpiTransfer(BlCmd::NoOp, 0U); // fetches the reply of `cmd`; only this repeats',
     'const auto r = (attempt == 0U) ? blSpiTransfer(BlCmd::NoOp, 0U) : blSpiTransfer(cmd, value);', 1,
     'the bootloader SPI command is sent again on BUSY (R2-2)'),
    ('bl-spi-probe-skip', IPP, 'if (write) {\n    // A probe first', 'if (false) {\n    // A probe first', 1,
     'no NO_OP probe before a bootloader SPI write (R2-2)'),
    ('dirty-bl-uart', IPP, '    uart_dirty_ = true;\n    return crc_bad ? Error::Crc : Error::BadReply;',
     '    return crc_bad ? Error::Crc : Error::BadReply;', 1, 'a bad bootloader UART reply does not mark the line dirty (R2-3)'),
    ('dirty-tmcl-op', IPP, '      uart_dirty_ = true; // a reply to another request: the stream is out of step',
     '      static_cast<void>(0);', 1, 'a reply that echoes another command does not mark the line dirty (R2-3)'),
    ('dirty-reg-uart', IPP, '    uart_dirty_ = true;\n    return Result<uint32_t>::Failure(Error::BadReply);',
     '    return Result<uint32_t>::Failure(Error::BadReply);', 1, 'a bad register UART reply does not mark the line dirty (R2-3)'),
    ('uart-reg-op-check', IPP, 'config_.tmcl_host_address, config_.device_address, reply) ||\n      (reply.op != command)',
     'config_.tmcl_host_address, config_.device_address, reply) ||\n      false', 1, 'the register UART reply operation is not checked (R2-3)'),
    ('fifo-drain-first', IPP, 'for (uint32_t i = 0U; i < detail::kDrainMax; ++i) {', 'for (uint32_t i = 0U; i < 1U; ++i) {', 1,
     'the resync drains one stale reply only (R2-3)'),
    ('resync-defer-ignored', IPP, 'if (uart_dirty_ && !defer_resync_) {', 'if (uart_dirty_) {', 1,
     'a stop waits for the resync (m1)'),
    ('stop-resync-motorstop', IPP, '  defer_resync_ = true;\n  auto r = tmcl(', '  auto r = tmcl(', 1,
     'MotorStop waits for the resync (m1)'),
    ('stop-resync-parameter', IPP, '  defer_resync_ = true;\n  auto r = tmclRaw(stop, false, false);', '  auto r = tmclRaw(stop, false, false);', 1,
     'the parameter-mode stop waits for the resync (m1)'),
    ('stop-resync-register', IPP, '  defer_resync_ = true;\n  Result<void> first = Result<void>::Success();',
     '  Result<void> first = Result<void>::Success();', 1, 'the register-mode stop waits for the resync (m1)'),
    ('ensure-clears-dirty', IPP, '  mode_ = config_.mode;\n  canary_armed_ = false;', '  mode_ = config_.mode;\n  uart_dirty_ = false;\n  canary_armed_ = false;', 1,
     'EnsureInitialized forgets a dirty line (m2)'),
    ('reg-stop-no-retry', IPP, '    if (!w.ok()) {\n      // The first datagram only reported a reset, or the write failed: write once more.',
     '    if (false) {\n      // The first datagram only reported a reset, or the write failed: write once more.', 1,
     'a failed register-stop write is not repeated (m12)'),
    ('gdrv-mask-bootstrap', IPP, 'inline constexpr uint32_t kBootstrapFlags = 0x0000\'F000U;', 'inline constexpr uint32_t kBootstrapFlags = 0x0000\'FF00U;', 1,
     'the bootstrap mask covers bits 11:8 too (m4)'),
    ('gdrv-latched-swapped', IPP, 'detail::GdrvLatched(status.value, fault.value)', 'detail::GdrvLatched(fault.value, status.value)', 1,
     'GdrvLatched arguments swapped (m4)'),
    ('gdrv-setup-chop7', IPP, '.value = config.pwm_enable | 1U}, // ENABLE_*, CHOP 1', '.value = config.pwm_enable | 7U}, // ENABLE_*, CHOP 1', 1,
     'CHOP is 7 when the gate driver is enabled (m5)'),
    ('gdrv-protect-no-settle', IPP, '  detail::DelayUs(port_, config_.settle_us);\n  if (const auto r = regPut(regs::MCC_GDRV_STATUS_EN',
     '  if (const auto r = regPut(regs::MCC_GDRV_STATUS_EN', 1, 'no bootstrap wait before STATUS_EN and PROT_EN (m5)'),
    ('gdrv-trip-check', IPP, '} else if (tripped.value) {', '} else if (false) {', 1,
     'a protection that trips at once is an Ok start (m6)'),
    ('script-sap-gt1', IPP, '((op == Tmcl::Sap) && (value != 0))', '((op == Tmcl::Sap) && (value > 1))', 1,
     'a script may SAP COMMUTATION_MODE with 1 (m8)'),
    ('hwreset-pin-release', IPP, 'if (!port_.GpioSet(CtrlPin::Reset, GpioSignal::INACTIVE)) {', 'if (false) {', 1,
     'a failed release of RSTN is ignored (m9)'),
    ('spifetch-retry-checksum', IPP, '(st != static_cast<uint8_t>(TmclSpiStatus::ChecksumError))', 'true', 1,
     'CHECKSUM_ERROR replies are not retried (m9, A5)'),
    ('bootmode-written-last-readback', IPP, 'if (back.value != config.Word(kBootModeWordIndex)) {', 'if (false) {', 1,
     'the boot-mode word is not read back (m9)'),
    ('powerdown-disable', IPP, 'if (const auto off = DisableDrive(); !off.ok()) {\n    return off;\n  }\n  if (!initialized_) {',
     'static_cast<void>(DisableDrive());\n  if (!initialized_) {', 1, 'PowerDown goes on after a failed stop (m9)'),
    ('regbaseline-started', IPP, 'reg_started_ =\n        true; // the first datagram', 'reg_started_ =\n        false; // the first datagram', 1,
     'the register baseline does not arm the reset check (m9)'),
    ('powerdown-reset-after-stop', IPP, 'if (!initialized_) {\n    // The stop met a reset', 'if (false) {\n    // The stop met a reset', 1,
     'PowerDown goes on after its stop saw a reset (fuzz finding)'),
    ('boot-reset-after-stop', IPP, 'if (!initialized_) {\n    return fail(Error::UnexpectedReset,\n                "tmc9660: the chip restarted during the stop',
     'if (false) {\n    return fail(Error::UnexpectedReset,\n                "tmc9660: the chip restarted during the stop', 1,
     'ReturnToBootloader sends Boot after its stop saw a reset (fuzz finding)'),
    ('resync-drain-fault', IPP, 'if (s == BusStatus::Fault) {\n        return Error::Bus;', 'if (false) {\n        return Error::Bus;', 1,
     'a drain that fails clears the dirty flag and the request is sent (fuzz finding)'),
    # --- round 3 ---
    ('param-stop-any-retry', IPP, '  if (!r.ok()) {\n    // The frame only reported a reset (SPI FIRST_CMD), was lost',
     '  if (r.error == Error::UnexpectedReset) {\n    // The frame only reported a reset (SPI FIRST_CMD), was lost', 1,
     'the parameter-mode stop is repeated only after a reported reset (R3-1)'),
    ('motorstop-any-retry', IPP, '  if (!r.ok()) {\n    r = tmcl(detail::ToUnderlying(Tmcl::Mst)',
     '  if (r.error == Error::UnexpectedReset) {\n    r = tmcl(detail::ToUnderlying(Tmcl::Mst)', 1,
     'MotorStop is repeated only after a reported reset (R3-1)'),
    ('enable-fail-full-stop', IPP, '    static_cast<void>(DisableDrive());\n    return Result<void>::Failure(w.error);',
     '    static_cast<void>(setDrvEn(GpioSignal::INACTIVE));\n    return Result<void>::Failure(w.error);', 1,
     'a failed EnableDrive only drops the pin (R3-2)'),
    ('reg-stop-faultn', IPP, 'auto w = regExchangeRaw(detail::kRegUartWrite, static_cast<uint8_t>(a.block), a.offset, true,\n                            0U, false);',
     'auto w = regExchangeRaw(detail::kRegUartWrite, static_cast<uint8_t>(a.block), a.offset, true,\n                            0U, true);', 1,
     'the register-mode stop is gated by the FAULTN guard (round 3 m1)'),
    ('bl-first-reset-ignored', IPP, '  if (detail::IsResetStatus(first.value.status)) {', '  if (false) {', 1,
     'the bootloader SPI command datagram ignores SESSION_START (round 3 m2)'),
    ('rtb-identity-fail-mode', IPP, '    mode_ = Mode::Parameter;\n    return fail(b.ok() ? id.error', '    return fail(b.ok() ? id.error', 1,
     'a failed ReturnToBootloader leaves the driver in bootloader mode (round 3 m3)'),
    # --- round 4 ---
    ('r4-mode-kept-identity', IPP, '    mode_ = previous;\n    return fail(v.error, "tmc9660: identity check failed");',
     '    return fail(v.error, "tmc9660: identity check failed");', 1,
     'a failed restart leaves the Config protocol, not the last one proved (R4-1)'),
    ('r4-mode-kept-hwreset', IPP, '      mode_ = previous;\n      return h;', '      return h;', 1,
     'a failed hardware-reset restart leaves the Config protocol (R4-1)'),
    ('r4-nohook-unknown', IPP, '} else if (!initialized_ && !detail::HasGpioSet<Port>) {', '} else if (false) {', 1,
     'no proved protocol and no DRV_EN hook: the stop is a silent Ok (R4-1)'),
    ('pstop-repeat-deferred', IPP, '    // the resynchronisation, and the stop is confirmed by a reply of its own.\n    r = tmclRaw(stop, false, false);',
     '    // the resynchronisation, and the stop is confirmed by a reply of its own.\n    defer_resync_ = true;\n    r = tmclRaw(stop, false, false);', 1,
     'the repeat of the parameter-mode stop skips the resync (round 4 m2)'),
    ('rstop-readback-gdrv-skip', IPP, '    if (!ra.ok()) {\n      first = Result<void>::Failure(ra.error);', '    if (false) {\n      first = Result<void>::Failure(ra.error);', 1,
     'a failed read-back of MCC_GDRV_HW is not reported (round 4 m3)'),
    ('rstop-readback-pwm-skip', IPP, '    } else if (!rb.ok()) {\n      first = Result<void>::Failure(rb.error);', '    } else if (false) {\n      first = Result<void>::Failure(rb.error);', 1,
     'a failed read-back of MCC_PWM_CONFIG is not reported (round 4 m3)'),
    ('enable-pin-fail-no-drop', IPP, '    static_cast<void>(setDrvEn(GpioSignal::INACTIVE));\n    return pin;', '    return pin;', 1,
     'a DRV_EN that cannot be raised is not dropped again (round 4 m4)'),
    ('rtb-stop-err-ignored', IPP, '  if (const auto off = DisableDrive(); !off.ok()) {\n    return fail(off.error, "tmc9660: the drive could not be switched off; no Boot sent");\n  }',
     '  static_cast<void>(DisableDrive());', 1, 'ReturnToBootloader goes on after a failed stop (round 4 m4)'),
    ('pstop-probe', IPP, '  defer_resync_ = true;\n  auto r = tmclRaw(stop, false, false);', '  defer_resync_ = true;\n  auto r = tmclRaw(stop, true, false);', 1,
     'the parameter-mode stop sends a probe first (round 4 m4)'),
    ('motorstop-probe', IPP, 'auto r = tmcl(detail::ToUnderlying(Tmcl::Mst), 0U, 0U, 0U, false,\n                false);', 'auto r = tmcl(detail::ToUnderlying(Tmcl::Mst), 0U, 0U, 0U, true,\n                false);', 1,
     'MotorStop sends a probe first (round 4 m4)'),
    # --- round 5 ---
    ('r5-start-follow', IPP, '  const auto f = blFollowStart(config);',
     '  if (!w.ok()) {\n    return Result<void>::Failure(w.error);\n  }\n  const auto f = blFollowStart(config);', 1,
     'a failed start write is not followed: the application may run unseen (R5-1)'),
    ('r5-start-any-set', IPP, 'const bool start = (config.Get(BootField::StartMotorCtrl) != 0U);',
     'const bool start = (old.Get(BootField::StartMotorCtrl) == 0U) && (config.Get(BootField::StartMotorCtrl) != 0U);', 1,
     'a boot-mode write with START already set is not treated as a start (R5-1, A24)'),
    ('r5-follow-bootloader-check', IPP, '    if (verifyIdentity().ok()) {\n      initialized_ = true;', '    if (false) {\n      initialized_ = true;', 1,
     'after a start that did not happen the driver does not find the bootloader again (R5-1)'),
    ('r5-resync-default', TYPES, "uart_resync_us{150'000}", "uart_resync_us{100'000}", 1,
     'the resync gap default drops to the chip gap itself (round 5 m1)'),
    ('r5-param-start-keep', IPP, '    if (const auto p = startParameterMode(); !p.ok()) {\n      return fail(p.error, "tmc9660: parameter mode did not start");',
     '    if (const auto p = startParameterMode(); !p.ok()) {\n      mode_ = previous;\n      return fail(p.error, "tmc9660: parameter mode did not start");', 1,
     'a parameter start that fails after the identity check forgets parameter mode (round 5 m2)'),
    ('r5-bl-stop-verify', IPP, '    if (!id.ok()) {\n      chip = Result<void>::Failure(id.error);\n    } else if (id.value.value != kChipId) {',
     '    if (false) {\n      chip = Result<void>::Failure(id.error);\n    } else if (false) {', 1,
     'no DRV_EN hook in the bootloader: the stop does not check that the bootloader still runs (round 5 fuzz)'),
    ('r5-bl-stop-swallow', IPP, '    const auto id = blExchange(BlCmd::GetInfo, static_cast<uint32_t>(BlInfo::ChipType), false);\n    if (!id.ok()) {',
     '    const auto vi = verifyIdentity();\n    const auto id = blExchange(BlCmd::GetInfo, static_cast<uint32_t>(BlInfo::ChipType), false);\n    if (!vi.ok() || !id.ok()) {', 1,
     'the stop check consumes a SESSION_START first, so an unseen bootloader reset passes (round 5 fuzz)'),
    # --- round 6 ---
    ('r6-start-nack-followed', IPP, 'if ((w.error == Error::Nack) || (w.error == Error::UnexpectedReset)) {', 'if (w.error == Error::UnexpectedReset) {', 1,
     'a start write the bootloader refused is followed anyway (round 6 m1)'),
    ('r6-start-reset-followed', IPP, 'if ((w.error == Error::Nack) || (w.error == Error::UnexpectedReset)) {', 'if (w.error == Error::Nack) {', 1,
     'a start write not sent after a seen reset is followed anyway (round 6 m1)'),
    ('r6-follow-param-start', IPP, '    if (const auto p = startParameterMode(); !p.ok()) {\n      return p;\n    }',
     '    static_cast<void>(startParameterMode());', 1, 'a parameter start that fails inside the follow is adopted (round 6 m2)'),
    ('r6-start-bank-reselect', IPP, '  if constexpr (!kSpi) {\n    if (const auto b = blSetBank(bl_bank_); !b.ok()) {\n      return b;\n    }\n  }\n  const auto a = blExchange(\n      BlCmd::SetAddress',
     '  const auto a = blExchange(\n      BlCmd::SetAddress', 1, 'the UART bank is not selected again before the start write (round 6 m3)'),
    ('r6-start-setaddress', IPP, '  if (!a.ok()) {\n    return Result<void>::Failure(a.error);\n  }\n  const auto w = blExchange(BlCmd::Write32, config.Word(kBootModeWordIndex), true);',
     '  const auto w = blExchange(BlCmd::Write32, config.Word(kBootModeWordIndex), true);', 1,
     'a failed SetAddress before the start write is not returned (round 6 m3)'),
    ('r6-wait-budget', IPP, '    cost_us += static_cast<uint64_t>(config_.reply_timeout_us) + config_.uart_resync_us +\n               (static_cast<uint64_t>(detail::kDrainMax) * detail::kDrainTimeoutUs);',
     '    cost_us += 0U;', 1, 'the UART identity wait does not count reply waits and resyncs (round 6 m4)'),
    ('r6-bl-recovery', IPP, '      blStopUnknownApplication(); // not the bootloader', '      static_cast<void>(0); // not the bootloader', 1,
     'no recovery after an unseen restart into an application (round 6 m5)'),
    ('r6-bl-recovery-reset', IPP, 'const bool bootloader_answered = (chip.error == Error::UnexpectedReset) ||\n                                     (chip.error == Error::Busy) || (chip.error == Error::Nack);', 'const bool bootloader_answered = (chip.error == Error::Busy) || (chip.error == Error::Nack);', 1,
     'a bootloader that restarted as itself is taken for an application (round 6 m5)'),
    ('r6-bl-app-mode-param', IPP, '    bl_app_mode_ = Mode::Parameter;', '    static_cast<void>(0);', 1,
     'BOOT_MODE parameter is not remembered for the recovery stop (round 6 m5)'),
    ('r6-bl-app-mode-reg', IPP, '    bl_app_mode_ = Mode::Register;', '    static_cast<void>(0);', 1,
     'BOOT_MODE register is not remembered for the recovery stop (round 6 m5)'),
    # --- round 7 ---
    ('r7-answered-busy', IPP, '(chip.error == Error::UnexpectedReset) ||\n                                     (chip.error == Error::Busy) || (chip.error == Error::Nack);',
     '(chip.error == Error::UnexpectedReset) ||\n                                     (chip.error == Error::Nack);', 1,
     'BUSY on the stop check is taken for an application (round 7 m1)'),
    ('r7-answered-nack', IPP, '(chip.error == Error::Busy) || (chip.error == Error::Nack);\n    if (!chip.ok() && !bootloader_answered)',
     '(chip.error == Error::Busy);\n    if (!chip.ok() && !bootloader_answered)', 1,
     'an error status on the stop check is taken for an application (round 7 m1)'),
    ('r7-adopt-read', IPP, '    if (mode_ == Mode::Bootloader) {\n      // No DRV_EN hook: an unseen',
     '    if (false) {\n      // No DRV_EN hook: an unseen', 1,
     'the configuration is not read at adoption: the application guess ignores the power-up BOOT_MODE (round 7 m2)'),
    ('r7-adopt-reset', IPP, '      if (!c.ok() && !initialized_) {\n        return fail(c.error, "tmc9660: the chip restarted during the start");',
     '      if (false) {\n        return fail(c.error, "tmc9660: the chip restarted during the start");', 1,
     'a restart seen during the adoption read still returns Ok (round 7 m2)'),
    ('r7-no-wake', IPP, '    detail::DelayUs(port_, detail::kWakeUs); // T_WAKE', '    static_cast<void>(0); // T_WAKE', 1,
     'no wait for T_WAKE before the first identity poll after the reset (round 7 m3)'),
    ('r7-ctor-mode', HPP, ', bl_app_mode_(config.mode) {}', ', bl_app_mode_(Mode::Bootloader) {}', 1,
     'the application guess does not start from Config::mode (round 7 m5)'),
    ('r7-wait-no-reply', IPP, 'cost_us += static_cast<uint64_t>(config_.reply_timeout_us) + config_.uart_resync_us +', 'cost_us += config_.uart_resync_us +', 1,
     'the identity wait does not count the UART reply wait (round 7 m6)'),
    ('r7-wait-no-drain', IPP, '(static_cast<uint64_t>(detail::kDrainMax) * detail::kDrainTimeoutUs);', '0U;', 1,
     'the identity wait does not count the drains (round 7 m6)'),
    ('ci1-copybytes-last', 'inc/tmc9660_detail.hpp', 'for (std::size_t k = 0U; k < n; ++k) {', 'for (std::size_t k = 0U; (k + 1U) < n; ++k) {', 1,
     'the byte copy into the frame buffer drops the last byte (CI round 1: std::copy replaced)'),
    ('ci1-copybytes-none', 'inc/tmc9660_detail.hpp', 'const std::size_t n = (src.size() < dst.size()) ? src.size() : dst.size();', 'const std::size_t n = 0U;', 1,
     'the byte copy into the frame buffer copies nothing (CI round 1)'),
    ('r7-wait-no-mismatch', IPP, '  return Result<void>::Failure((last.error == Error::DeviceIdMismatch) ? Error::DeviceIdMismatch\n                                                                       : Error::Timeout);',
     '  return Result<void>::Failure(Error::Timeout);', 1,
     'the end of the boot wait reports Timeout for a chip of another type (round 7 m5)'),
]

# Edits that cannot change behaviour because another guard covers the same case.
EQUIVALENT = {
    'blstart-reserved': 'BlStartApplication: with the reserved boot mode removed from the first guard, '
                        'BootConfig::CheckStart refuses the same start before any write',
    # Probe-on-stop mutants of round 2 that stay equivalent (the parameter-mode and MotorStop ones are killed since
    # round 4 by the SPI frame log): a probe in front of a stop only changes how the stop is delivered; a reported
    # reset is followed by the repeat, so the chip receives the same stop either way.
    'disable-reg-uart-probe': 'disableDriveRegister with probe = true on the register UART: a failed or '
                              'reset-reporting first write is repeated by the clear lambda, so both words '
                              'are written either way',
    # Round 3 reviewer survivors judged equivalent:
    'idle-set-err-ignored': 'idleSafe ignoring a failed SAP of parameter 9: the read-back of parameter 9 still decides the result',
    'idle-read-err-ignored': 'idleSafe ignoring a failed read of parameter 9: the SAP and the read-back that follow decide the result',
    'regstop-readback-enable-bits': 'the register-stop read-back without kPwmEnableBits: kPwmEnergise (0xFF07) already contains 0x0F00',
    'rtb-boot-nack-map': 'ReturnToBootloader mapping a Nack of Boot like a lost reply: the bootloader identity wait fails and '
                         'the mode is restored, so the outcome is the same',
    'canary-probe-on-stop': 'the canary read in front of a stop: a reported reset is followed by the retry, '
                            'and the stop reaches the chip as before',
}

FLAGS = ['-std=c++20', '-fno-exceptions', '-fno-rtti', '-w', '-O0']


def run_one(item, work_root, cxx):
    mid, rel, old, new, nth, what = item
    work = Path(tempfile.mkdtemp(prefix='mut-' + mid + '-', dir=work_root))
    for d in ('inc', 'src', 'tests'):
        shutil.copytree(ROOT / d, work / d, ignore=shutil.ignore_patterns('compile_fail', 'fuzz', 'docs', 'install_consumer'))
    path = work / rel
    text = path.read_text()
    if text.count(old) < nth:
        return mid, 'BROKEN', 'pattern not found: ' + old[:60]
    parts = text.split(old)
    path.write_text(old.join(parts[:nth]) + new + old.join(parts[nth:]))
    tests = sorted(str(p) for p in (work / 'tests').glob('test_*.cpp')
                   if p.name not in ('test_kconfig.cpp', 'test_version.cpp'))
    version_dir = os.environ.get('TMC9660_GENERATED', str(ROOT / 'build' / 'generated'))
    exe = work / 'tests_exe'
    build = subprocess.run([cxx, *FLAGS, '-I' + str(work / 'inc'), '-I' + str(work / 'tests'), '-I' + version_dir,
                            *tests, '-o', str(exe)], capture_output=True, text=True)
    if build.returncode != 0:
        if 'static assertion failed' in build.stderr or 'static_assert' in build.stderr:
            verdict = ('KILLED', 'a static_assert failed')
        else:
            verdict = ('BROKEN', 'build failed: ' + build.stderr.strip().splitlines()[-1][:80])
    else:
        run = subprocess.run([str(exe)], capture_output=True, text=True)
        verdict = ('KILLED', 'a test failed') if run.returncode != 0 else ('SURVIVED', what)
    shutil.rmtree(work, ignore_errors=True)
    return mid, verdict[0], verdict[1]


def main():
    args = sys.argv[1:]
    jobs = int(args[args.index('--jobs') + 1]) if '--jobs' in args else 4
    only = args[args.index('--only') + 1] if '--only' in args else None
    cxx = os.environ.get('CXX', 'c++')
    items = [m for m in MUTANTS if only in (None, m[0])]
    with tempfile.TemporaryDirectory(prefix='tmc9660-mutants-') as work_root:
        with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
            results = list(pool.map(lambda m: run_one(m, work_root, cxx), items))
    bad = 0
    for mid, verdict, note in results:
        print('%-28s %-9s %s' % (mid, verdict, note))
        bad += verdict != 'KILLED'
    print('%d mutants, %d killed, %d not killed; %d equivalent not run' %
          (len(results), len(results) - bad, bad, len(EQUIVALENT)))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
