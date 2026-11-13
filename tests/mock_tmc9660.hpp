/**
 * @file mock_tmc9660.hpp
 * @brief An independent model of the TMC9660 as seen from its host interface: the bootloader,
 * parameter mode and register mode, over SPI and UART.
 *
 * The model is written from the datasheet and the two reference manuals, not from the driver: its
 * CRC-8, CRC-32, checksum and frame decoding are its own code (a different algorithm for the
 * CRC-8), and it knows nothing about `tmc9660_frames.hpp`. It shares only the register and
 * parameter tables, which are generated from the manuals. Where the documents disagree the model
 * has a knob (`Knobs`), so a test can run the other reading.
 *
 * Faults and observations for the tests: reset injection, BUSY and NOT_READY replies, dropped,
 * corrupted or misaddressed UART replies, and counters that record what a safe driver must never
 * cause (writes after an unseen reset, an energised gate driver with a latched flag, an all-zero
 * TMCL frame, Boot with the system on, an application start with a reserved boot mode).
 */
#ifndef TMC9660_MOCK_TMC9660_HPP
#define TMC9660_MOCK_TMC9660_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "tmc9660_params.hpp"
#include "tmc9660_registers.hpp"

namespace tmc9660_test {

/// How the chip splits the type and motor bits over the two bytes of a TMCL frame (A2). This is the
/// chip's truth in a test; the driver's own `TmclLayout` is a different type and is probed.
enum class TmclWire : uint8_t {
  MotorHigh,   ///< Second byte `motor << 4 | type[11:8]`, first byte `type[7:0]`.
  TypeHigh,    ///< Second byte `type[11:8] << 4 | motor`, first byte `type[7:0]`.
  SpiMsbFirst, ///< First byte `type[11:4]`, second byte `type[3:0] << 4 | motor`.
  Lenient,     ///< A chip that accepts both nibble orders (ORs the nibbles, motor 0): not the real
               ///< chip.
};

/// The readings of the ambiguous places of the documents (design note, section 5).
struct Knobs {
  bool crc_reflect_out{false};             ///< A1.
  TmclWire tmcl_wire{TmclWire::MotorHigh}; ///< A2.
  bool reject_motor{false};   ///< SAP and GAP answer an error status for a motor other than 0.
  bool reg_wide{false};       ///< A3.
  bool reg_uart_msb{false};   ///< A4.
  bool start_on_set{false};   ///< A24: any word-2 write with START set starts (else 0 to 1).
  unsigned ram_index_bits{5}; ///< A17.
};

/// Which protocol the chip speaks.
enum class Phase : uint8_t { Bootloader, Parameter, Register };

/// The model. Fields are grouped by meaning (knobs, observations), not packed: the padding of a
/// test double costs nothing.
// NOLINTNEXTLINE(clang-analyzer-optin.performance.Padding)
class Tmc9660Model {
public:
  /// Fault injection.
  int busy_replies{0};            ///< Bootloader SPI: BUSY for this many NoOp fetches.
  int not_ready_replies{0};       ///< TMCL SPI: NOT_READY for this many reply requests.
  int checksum_error_replies{0};  ///< TMCL SPI: CHECKSUM_ERROR for this many reply requests.
  bool drop_uart_reply{false};    ///< UART: no reply (timeout).
  bool corrupt_uart_reply{false}; ///< UART: the last byte of the reply is wrong.
  bool wrong_uart_address{false}; ///< UART: the reply carries another address.
  bool flash_present{true};       ///< An SPI flash answers.
  bool flash_busy{false};         ///< The flash reports busy.
  bool pll_never_locks{false};    ///< The PLL does not lock after a clock change.
  uint32_t relatch_on_clear{
      0}; ///< Register mode: bits that latch again right after MCC_GDRV_STATUS is cleared.
  uint32_t gdrv_fault_bits{0};   ///< Register mode: bits that stay set in MCC_GDRV_STATUS.
  uint32_t chip_id{0x544D0001U}; ///< What GET_INFO type 0 returns.
  bool corrupt_cfg_write{false}; ///< A write to configuration word `corrupt_cfg_word` flips a bit.
  unsigned corrupt_cfg_word{3U}; ///< The word `corrupt_cfg_write` acts on.
  int ignored_param{-1};   ///< Parameter mode: a SAP of this parameter is acknowledged and dropped.
  int refuse_cfg_word{-1}; ///< Bootloader: a write to this configuration word answers an error.
  bool refuse_chip_type{false}; ///< Bootloader: GET_INFO chip type answers an error status.
  bool ignore_stop{false}; ///< COMMUTATION_MODE = 0 is acknowledged but not executed (erratum 3).
  bool reset_on_reply_request{
      false}; ///< One-shot: the chip restarts before the next TMCL SPI reply request.
  bool skip_reg_counter{
      false}; ///< One-shot: the register-mode datagram counter jumps (a lost datagram).
  bool stray_boot_bit{
      false}; ///< One-shot: a register-mode reply carries the boot bit with a valid counter.
  bool pll_status_low{false};  ///< The clock word reads with PLL_STATUS clear.
  bool wrong_uart_host{false}; ///< UART: the reply carries another host address (byte 0).
  bool wrong_op_echo{false};   ///< TMCL: the reply echoes another command number.
  bool dead_miso{false}; ///< SPI: every reply is all zero (a dead MISO line); nothing is parsed.
  bool reg_unmapped_ok{false};    ///< Register mode: an unmapped offset is acknowledged (status 1).
  bool reg_status_fail{false};    ///< One-shot: the next register access reports a failed status.
  int skip_reg_counter_after{-1}; ///< Register SPI: the datagram counter jumps at this datagram.
  bool corrupt_gdrv_hw{false};    ///< A write to MCC_GDRV_HW stores the value with bit 0 flipped.
  int ignored_reg_write_offset{
      -1}; ///< Writes to this MCC register offset are acknowledged and dropped.
  bool ignore_first_command{
      false};             ///< TMCL SPI: the command frame that carries FIRST_CMD is not run.
  uint32_t reset_bank{5}; ///< The bootloader memory bank selected after a reset.
  Knobs knobs{};          ///< Readings of the ambiguities.

  /// Observations.
  int writes_since_reset{0};      ///< State-changing commands executed since the last `Reset()`.
  int energise_events{0};         ///< Gate driver outputs switched on (any mode).
  int energise_with_flag{0};      ///< Of those, with an error flag latched.
  int energise_without_drv_en{0}; ///< Of those, with DRV_EN low.
  int zero_frames{0};             ///< All-zero TMCL SPI frames received.
  int boot_with_drive_on{0};      ///< Boot commands received with COMMUTATION_MODE not 0.
  int reserved_start_events{0};   ///< START_MOTOR_CTRL written with a reserved BOOT_MODE.
  int otp_writes{0};              ///< Writes to the OTP bank.
  int config_word_writes{0};      ///< Writes to the configuration memory.
  int erases{0};                  ///< Flash sectors erased.
  int frames{0};                  ///< Frames received (all buses).
  int ldo_writes{0};              ///< Writes that changed the LDO bits.
  int stop_writes{0}; ///< Of `writes_since_reset`, the ones that only stop: MST, COMMUTATION_MODE
                      ///< 0, zero enables.
  int disable_cmds{
      0}; ///< COMMUTATION_MODE = 0 writes and zero writes of the register-mode enables.
  std::array<int, 64> bl_cmds{}; ///< Bootloader commands executed, by command number.
  uint64_t now_us{0};            ///< Time, as the port reports it (set by the bus mock).
  uint64_t hw_energise_us{0};    ///< `now_us` when MCC_GDRV_HW last switched the gate driver on.
  uint64_t status_en_us{0};      ///< `now_us` of the last write to MCC_GDRV_STATUS_EN.
  uint64_t prot_en_us{0};        ///< `now_us` of the last write to MCC_GDRV_PROT_EN.
  uint32_t chop_at_energise{0};  ///< CHOP field of MCC_PWM_CONFIG when the gate driver was enabled.
  int energise_chop_events{0};   ///< Times the gate driver was enabled.
  int trip_on_prot_en{0};        ///< Register mode: bits that latch in MCC_GDRV_STATUS on PROT_EN.
  uint8_t last_cmd{0};           ///< Last bootloader command or TMCL op.
  uint8_t last_cmd_type{0};      ///< Last TMCL type byte (low).
  uint8_t last_cmd_motor{0};     ///< Last TMCL motor byte (raw byte 3 of the frame).

  /// Start state: bootloader with the default configuration.
  Tmc9660Model() noexcept {
    ResetConfig();
    Reset();
  }

  /// A watchdog reset: a reset that also sets WATCHDOG_EVENT in GENERAL_ERROR_FLAGS (parameter
  /// mode).
  void WatchdogReset() noexcept {
    Reset();
    params_[299] = static_cast<int64_t>(static_cast<uint64_t>(params_[299]) | 0x200U);
  }

  /// A chip reset (power cycle, watchdog, brown-out): everything volatile restarts. The DRV_EN pin
  /// is driven by the host and is not touched.
  void Reset() noexcept {
    writes_since_reset = 0;
    stop_writes = 0;
    pwm_started_ = false;
    session_started_ = false;
    spi_first_ = true;
    reg_first_ = true;
    reg_counter_ = 0;
    pending_bl_ = {0, 0};
    pending_reg_ = {0, 0, false};
    ready_reply_valid_ = false;
    idle_image_ = true;
    params_.fill(0);
    for (const tmc9660::ParamInfo& p : tmc9660::kParams) {
      params_[p.id] = p.def;
    }
    for (auto& bank : globals_) {
      for (auto& g : bank) {
        g = 0;
      }
    }
    globals_[0][2] = 2; // SERIAL_HOST_ADDRESS
    globals_[0][1] = 1; // SERIAL_ADDRESS
    globals_[0][4] = 100;
    regs_.fill(0U);
    for (const tmc9660::RegInfo& r : tmc9660::kRegisters) {
      regs_[regIndex(static_cast<uint8_t>(r.addr.block), r.addr.offset)] = r.reset;
    }
    cfg_ = reset_cfg_;
    bank_ = reset_bank;
    address_ = 0;
    reg_datagrams_ = 0;
    script_state_ = 0;
    download_ = false;
    download_count_ = 0;
    pll_delay_ = 0;
    phase_ = startPhase();
    applyAddresses();
  }

  /// The configuration that `Reset()` restores (what the OTP holds).
  void ResetConfig() noexcept {
    for (auto& w : reset_cfg_) {
      w = 0;
    }
    reset_cfg_[0] = (255U << 24) | (1U << 16); // host 255, device 1
    reset_cfg_[6] =
        (1U << 30) | (1U << 16) | (14U << 18) | 99U; // PLL locked, PLL out 1, RDIV 14, reserved 99
  }

  /// Make the chip come up in `phase` after resets: set BOOT_MODE and START_MOTOR_CTRL in the reset
  /// configuration.
  void BootInto(Phase phase) noexcept {
    reset_cfg_[2] &= ~((3U) | (1U << 12));
    if (phase == Phase::Parameter) {
      reset_cfg_[2] |= 2U | (1U << 12);
    } else if (phase == Phase::Register) {
      reset_cfg_[2] |= 1U | (1U << 12);
    } else {
      // bootloader
    }
    Reset();
  }

  /// The current protocol.
  [[nodiscard]] Phase GetPhase() const noexcept {
    return phase_;
  }
  /// The level of DRV_EN as the port drives it.
  void SetDrvEn(bool v) noexcept {
    drv_en_ = v;
  }
  /// The level of DRV_EN.
  [[nodiscard]] bool DrvEn() const noexcept {
    return drv_en_;
  }
  /// Hardware RSTN level changes call this.
  void ResetPin(bool asserted) noexcept {
    if (asserted) {
      Reset();
    }
  }
  /// Parameter mode: the PWM logic is running. PM "System Off": with IDLE_MOTOR_PWM_BEHAVIOR 0
  /// (parameter 9) the PWM stays active when COMMUTATION_MODE is set to 0; the parameter acts at
  /// the moment System off is entered, so setting it to 1 afterwards does not stop a PWM that is
  /// already running until System off is entered again.
  [[nodiscard]] bool PwmActive() const noexcept {
    return (params_[4] != 0) || pwm_started_;
  }
  /// A parameter value as the chip holds it.
  [[nodiscard]] int64_t ParamValue(unsigned id) const noexcept {
    return params_[id];
  }
  /// Set a parameter value directly (a flag, a sensor).
  void SetParamValue(unsigned id, int64_t v) noexcept {
    params_[id] = v;
    if ((id == 4U) && (v != 0)) {
      pwm_started_ = true;
    }
  }
  /// A global parameter.
  [[nodiscard]] int64_t GlobalValue(unsigned bank, unsigned n) const noexcept {
    return globals_[bank][n];
  }
  /// Set a global parameter directly.
  void SetGlobalValue(unsigned bank, unsigned n, int64_t v) noexcept {
    globals_[bank][n] = v;
  }
  /// A register value.
  [[nodiscard]] uint32_t RegValue(tmc9660::RegAddr a) const noexcept {
    return regs_[regIndex(static_cast<uint8_t>(a.block), a.offset)];
  }
  /// Set a register value directly.
  void SetRegValue(tmc9660::RegAddr a, uint32_t v) noexcept {
    regs_[regIndex(static_cast<uint8_t>(a.block), a.offset)] = v;
  }
  /// A configuration word.
  [[nodiscard]] uint32_t CfgWord(unsigned i) const noexcept {
    return cfg_[i];
  }
  /// Set a configuration word directly (also the reset configuration).
  void SetCfgWord(unsigned i, uint32_t v) noexcept {
    cfg_[i] = v;
    reset_cfg_[i] = v;
  }
  /// Number of script instructions stored by the last download.
  [[nodiscard]] int DownloadCount() const noexcept {
    return download_count_;
  }
  /// Whether the model is inside a download.
  [[nodiscard]] bool InDownload() const noexcept {
    return download_;
  }
  /// The SPI flash word at a byte address.
  [[nodiscard]] uint32_t FlashWord(uint32_t address) const noexcept {
    return flash_[(address / 4U) % flash_.size()];
  }
  /// Preload a flash word.
  void SetFlashWord(uint32_t address, uint32_t v) noexcept {
    flash_[(address / 4U) % flash_.size()] = v;
  }

  // --- the model's own checks, independent of the driver
  // ---------------------------------------------------

  /// CRC-8 of the bootloader UART frames: the reflected algorithm (polynomial 0xE0), then the
  /// result reflected again unless the knob says the datasheet prose is right.
  uint8_t Crc8(const uint8_t* d, std::size_t n) const noexcept {
    uint32_t crc = 0U;
    for (std::size_t i = 0; i < n; ++i) {
      crc ^= static_cast<uint32_t>(d[i]);
      for (uint32_t b = 0U; b < 8U; ++b) {
        crc = ((crc & 1U) != 0U) ? ((crc >> 1U) ^ 0xE0U) : (crc >> 1U);
      }
    }
    // `crc` is the result of the algorithm with reflected input and output; the datasheet's
    // examples do not reflect the output, so reverse it.
    uint32_t rev = 0U;
    for (uint32_t b = 0U; b < 8U; ++b) {
      rev = (rev << 1U) | ((crc >> b) & 1U);
    }
    return static_cast<uint8_t>(knobs.crc_reflect_out ? crc : rev);
  }
  /// 8-bit sum.
  static uint8_t Sum(const uint8_t* d, std::size_t n) noexcept {
    unsigned s = 0;
    for (std::size_t i = 0; i < n; ++i) {
      s += d[i];
    }
    return static_cast<uint8_t>(s & 0xFFU);
  }

  // --- SPI
  // -----------------------------------------------------------------------------------------------

  /// One SPI transfer of `len` bytes.
  void SpiTransfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    ++frames;
    if (phase_ == Phase::Bootloader) {
      blSpi(tx, rx, len);
    } else if (phase_ == Phase::Parameter) {
      paramSpi(tx, rx, len);
    } else {
      regSpi(tx, rx, len);
    }
    if (dead_miso) {
      for (std::size_t i = 0; i < len; ++i) {
        rx[i] = 0; // the chip took the datagram; nothing comes back
      }
    }
  }

  /// Register SPI datagrams answered since the last reset.
  [[nodiscard]] int RegDatagrams() const noexcept {
    return reg_datagrams_;
  }

  // --- UART
  // ----------------------------------------------------------------------------------------------

  /// One UART request: the number of reply bytes the chip puts on the line (0: none). The reply is
  /// complete here; when it arrives is the port's business (`UartWire`).
  std::size_t UartRequest(const uint8_t* tx, std::size_t tx_len, uint8_t* out) noexcept {
    ++frames;
    std::array<uint8_t, 9> reply{};
    std::size_t n = 0;
    if (phase_ == Phase::Bootloader) {
      n = blUart(tx, tx_len, reply.data());
    } else if (phase_ == Phase::Parameter) {
      n = tmclUart(tx, tx_len, reply.data(), false);
    } else {
      n = tmclUart(tx, tx_len, reply.data(), true);
    }
    applyPendingSwitch();
    if ((n == 0U) || drop_uart_reply) {
      return 0;
    }
    if (corrupt_uart_reply) {
      reply[n - 1U] = static_cast<uint8_t>(reply[n - 1U] ^ 0x5AU);
    }
    if (wrong_uart_address) {
      reply[1] = static_cast<uint8_t>(reply[1] ^ 0x08U);
      if (phase_ == Phase::Bootloader) {
        reply[7] = Crc8(reply.data(), 7);
      } else {
        reply[8] = Sum(reply.data(), 8);
      }
    }
    if (wrong_uart_host) {
      reply[0] = static_cast<uint8_t>(reply[0] ^ 0x10U);
      if (phase_ == Phase::Bootloader) {
        reply[7] = Crc8(reply.data(), 7);
      } else {
        reply[8] = Sum(reply.data(), 8);
      }
    }
    for (std::size_t i = 0; i < n; ++i) {
      out[i] = reply[i];
    }
    return n;
  }

private:
  struct BlPending {
    uint8_t status;
    uint32_t value;
  };
  struct RegPending {
    uint32_t value;
    uint32_t unused;
    bool ok;
  };

  static std::size_t regIndex(uint8_t block, unsigned offset) noexcept {
    return (static_cast<std::size_t>(block) * 1024U) + (offset & 0x3FFU);
  }

  [[nodiscard]] Phase startPhase() const noexcept {
    const bool start = (reset_cfg_[2] & (1U << 12)) != 0U;
    const unsigned mode = reset_cfg_[2] & 3U;
    if (start && (mode == 2U)) {
      return Phase::Parameter;
    }
    if (start && (mode == 1U)) {
      return Phase::Register;
    }
    return Phase::Bootloader;
  }

  void applyAddresses() noexcept {
    device_ = static_cast<uint8_t>((cfg_[0] >> 16) & 0xFFU);
    host_ = static_cast<uint8_t>((cfg_[0] >> 24) & 0xFFU);
  }

  void applyPendingSwitch() noexcept {
    if (switch_pending_) {
      switch_pending_ = false;
      applyAddresses();
      if (phase_switch_ != phase_) {
        phase_ = phase_switch_;
        spi_first_ = true;
        reg_first_ = true;
        reg_counter_ = 0;
        ready_reply_valid_ = false;
        idle_image_ = true;
      }
    }
  }

  // ----- bootloader
  // ---------------------------------------------------------------------------------------

  /// Execute a bootloader command: returns status and value.
  BlPending blExec(uint8_t cmd, uint32_t value) noexcept {
    last_cmd = cmd;
    ++bl_cmds[cmd % bl_cmds.size()];
    BlPending r{0, 0};
    switch (cmd) {
    case 0: // GET_INFO
      switch (value) {
      case 0:
        if (refuse_chip_type) {
          r.status = 4; // INVALID_VALUE
        } else {
          r.value = chip_id;
        }
        break;
      case 1:
        r.value = 0x00010002U;
        break;
      case 2:
        r.value = 0x0000'00FFU;
        break;
      case 13:
        r.value = 1U;
        break;
      case 14:
        r.value = ((cfg_[6] >> 16) & 3U) == 1U ? 40'000'000U : 15'000'000U;
        break;
      case 17:
        r.value = 0x20000U;
        break;
      case 18:
        r.value = 64U;
        break;
      case 28:
        r.value = 2U;
        break;
      default:
        r.status = 4; // INVALID_VALUE
        break;
      }
      break;
    case 8:
      r.value = bank_;
      break;
    case 9:
      if ((value == 1U) || (value == 2U) || (value == 3U) || (value == 5U)) {
        bank_ = value;
      } else {
        r.status = 14; // INVALID_BANK
      }
      break;
    case 10:
      r.value = address_;
      break;
    case 11:
      address_ = value;
      break;
    case 12:
    case 13:
      r = blReadMem(cmd == 13);
      break;
    case 18:
    case 19:
      r.status = blWriteMem(value, cmd == 19);
      break;
    case 29:
      break;
    case 30: // OTP_LOAD
      ++writes_since_reset;
      break;
    case 32:
      r.value = (bank_ == 1U) && ((cfg_[2] & (1U << 16)) != 0U) ? 1U : 0U;
      if ((bank_ != 1U) && (bank_ != 2U)) {
        r.status = 14;
      }
      break;
    case 33:
      r.value = (bank_ == 1U) && flash_present ? 1U : 0U;
      if ((bank_ != 1U) && (bank_ != 2U)) {
        r.status = 14;
      }
      break;
    case 40:
      r.value = flash_busy ? 1U : 0U;
      break;
    case 37: // FLASH_ERASE_SECTOR
      if (bank_ != 1U) {
        r.status = 14;
      } else {
        ++erases;
        ++writes_since_reset;
        const std::size_t first = static_cast<std::size_t>(value / 4096U) * 1024U;
        for (std::size_t i = 0; i < 1024U; ++i) {
          flash_[(first + i) % flash_.size()] = 0xFFFF'FFFFU;
        }
      }
      break;
    default:
      r.status = 1; // CMD_NOT_FOUND
      break;
    }
    return r;
  }

  BlPending blReadMem(bool inc) noexcept {
    BlPending r{0, 0};
    if (bank_ == 5U) {
      const std::size_t i = (address_ - 0x20000U) / 4U;
      if ((address_ < 0x20000U) || (i >= cfg_.size())) {
        r.status = 3; // INVALID_ADDR
      } else {
        if (i == 6U) {
          if (pll_delay_ > 0) {
            --pll_delay_;
            if ((pll_delay_ == 0) && !pll_never_locks && (((cfg_[6] >> 16) & 3U) == 1U)) {
              cfg_[6] |= (1U << 30);
            }
          }
        }
        r.value = cfg_[i];
        if ((i == 6U) && pll_status_low) {
          r.value &= ~(1U << 30);
        }
      }
    } else if (bank_ == 1U) {
      r.value = flash_[(address_ / 4U) % flash_.size()];
    } else if (bank_ == 3U) {
      r.value = 0;
    } else {
      r.value = 0;
    }
    if (inc && (r.status == 0U)) {
      address_ += 4U;
    }
    return r;
  }

  uint8_t blWriteMem(uint32_t value, bool inc) noexcept {
    uint8_t status = 0;
    ++writes_since_reset;
    if (bank_ == 5U) {
      const std::size_t i = (address_ - 0x20000U) / 4U;
      if ((address_ < 0x20000U) || (i >= cfg_.size())) {
        return 3;
      }
      if (std::cmp_equal(i, refuse_cfg_word)) {
        return 3; // an error status: the word is not written, nothing starts
      }
      ++config_word_writes;
      uint32_t v = value;
      if (i == 6U) {
        v = (v & ~(1U << 30)) | (cfg_[6] & (1U << 30)); // read only
        if (((v ^ cfg_[6]) & ~(1U << 30)) != 0U) {
          cfg_[6] &= ~(1U << 30);
          v &= ~(1U << 30);
          pll_delay_ = 2;
        }
      }
      if (i == 0U) {
        if (((v ^ cfg_[0]) & 0x1FFU) != 0U) {
          ++ldo_writes;
        }
      }
      const uint32_t old = cfg_[i];
      if ((i == corrupt_cfg_word) && corrupt_cfg_write) {
        v ^= (1U << 5);
      }
      cfg_[i] = v;
      if (i == 0U) {
        switch_pending_ = true; // address change takes effect after the reply
        phase_switch_ = phase_;
      }
      if (i == 2U) {
        const bool start =
            ((v & (1U << 12)) != 0U) && (knobs.start_on_set || ((old & (1U << 12)) == 0U));
        const unsigned mode = v & 3U;
        if (start) {
          if (mode == 1U) {
            switch_pending_ = true;
            phase_switch_ = Phase::Register;
          } else if (mode == 2U) {
            switch_pending_ = true;
            phase_switch_ = Phase::Parameter;
          } else {
            ++reserved_start_events;
          }
        }
      }
    } else if (bank_ == 3U) {
      ++otp_writes;
      status = 18;
    } else if (bank_ == 1U) {
      flash_[(address_ / 4U) % flash_.size()] = value;
    } else {
      // I2C EEPROM: accepted, not modelled
    }
    if (inc && (status == 0U)) {
      address_ += 4U;
    }
    return status;
  }

  void blSpi(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    if (len != 5U) {
      for (std::size_t i = 0; i < len; ++i) {
        rx[i] = 0;
      }
      return;
    }
    // The reply to the previous datagram is what this one clocks out.
    BlPending out = pending_bl_;
    if (!session_started_) {
      out = {19, 0x00010002U}; // SESSION_START with the version
      session_started_ = true;
    } else if ((tx[0] == 29U) && (busy_replies > 0)) {
      // A NoOp that fetches: the reply is not ready yet; it stays pending.
      --busy_replies;
      out = {15, 0};
      rx[0] = out.status;
      putBe(rx + 1, out.value);
      last_cmd = 29;
      return;
    } else {
      // A command datagram: its reply is produced below.
    }
    rx[0] = out.status;
    putBe(rx + 1, out.value);
    const uint32_t value = getBe(tx + 1);
    if (tx[0] == 29U) {
      // NoOp: its own reply is OK; the real reply was delivered above only if it was pending.
      pending_bl_ = BlPending{0, 0};
      last_cmd = 29;
    } else {
      pending_bl_ = blExec(tx[0], value);
    }
    applyPendingSwitch();
  }

  size_t blUart(const uint8_t* tx, std::size_t len, uint8_t* reply) noexcept {
    if ((len != 8U) || (tx[0] != 0x55U) || (tx[1] != device_) || (Crc8(tx, 7) != tx[7])) {
      return 0;
    }
    const BlPending r = blExec(tx[2], getBe(tx + 3));
    reply[0] = host_;
    reply[1] = device_;
    reply[2] = r.status;
    putBe(reply + 3, r.value);
    reply[7] = Crc8(reply, 7);
    return 8;
  }

  // ----- parameter mode
  // -----------------------------------------------------------------------------------

  struct TmclOut {
    uint8_t status;
    uint32_t value;
  };

  /// Decode type and motor from the two bytes, with the chip's wire layout.
  void decodeTypeMotor(uint8_t byte1, uint8_t byte2, unsigned& type,
                       unsigned& motor) const noexcept {
    const unsigned b1 = byte1;
    const unsigned b2 = byte2;
    if ((b2 > 0x0FU) && (b1 == 0x81U) && (b2 == 0x92U)) {
      type = b1;
      motor = b2; // Boot: the whole byte
      return;
    }
    switch (knobs.tmcl_wire) {
    case TmclWire::TypeHigh:
      type = b1 | ((b2 >> 4U) << 8U);
      motor = b2 & 0x0FU;
      break;
    case TmclWire::SpiMsbFirst:
      type = (b1 << 4U) | (b2 >> 4U);
      motor = b2 & 0x0FU;
      break;
    case TmclWire::Lenient:
      type = b1 | (((b2 & 0x0FU) | (b2 >> 4U)) << 8U);
      motor = 0U;
      break;
    case TmclWire::MotorHigh:
    default:
      type = b1 | ((b2 & 0x0FU) << 8U);
      motor = b2 >> 4U;
      break;
    }
  }

  static bool odd(int64_t v) noexcept {
    return (static_cast<uint64_t>(v) & 1U) != 0U;
  }

  TmclOut exec(uint8_t op, unsigned type, unsigned motor, uint32_t raw) noexcept {
    last_cmd = op;
    last_cmd_type = static_cast<uint8_t>(type & 0xFFU);
    last_cmd_motor = static_cast<uint8_t>(motor);
    TmclOut r{100, 0};
    const auto sv = static_cast<int32_t>(raw);
    if (op == 0U) {
      r.status = 2;
      return r;
    }
    if (download_ && (op != 133U)) {
      ++download_count_;
      r.status = 101;
      return r;
    }
    switch (op) {
    case 157:
      if (type == 0U) {
        r.value = chip_id;
      } else if (type == 1U) {
        r.value = 0x00010000U;
      } else {
        r.status = 3;
      }
      break;
    case 3:
      ++writes_since_reset;
      ++stop_writes;
      break;
    case 5: { // SAP
      const tmc9660::ParamInfo* p = tmc9660::FindParam(type);
      if ((p == nullptr) || (p->access == tmc9660::Access::ReadOnly) ||
          (knobs.reject_motor && (motor != 0U))) {
        r.status = 3;
        break;
      }
      ++writes_since_reset;
      int64_t v = (p->min < 0) ? static_cast<int64_t>(sv) : static_cast<int64_t>(raw);
      if (p->access == tmc9660::Access::ReadWriteClear) {
        params_[type] = static_cast<int64_t>(static_cast<uint64_t>(params_[type]) &
                                             ~static_cast<uint64_t>(raw));
        break;
      }
      bool valid = (v == p->def);
      if (p->coded) {
        for (unsigned i = 0; i < p->allowed_count; ++i) {
          valid = valid || (tmc9660::kAllowedValues[p->allowed_first + i] == v);
        }
      } else {
        valid = valid || ((v >= p->min) && (v <= p->max));
      }
      if (!valid) {
        r.status = 4;
        break;
      }
      if ((type == 4U) && (v != 0)) {
        pwm_started_ = true;
        if (params_[4] == 0) {
          ++energise_events;
          if (!drv_en_) {
            ++energise_without_drv_en;
          }
          if ((params_[299] != 0) || (params_[300] != 0)) {
            ++energise_with_flag;
          }
        }
      }
      if ((type == 4U) && (v == 0)) {
        ++disable_cmds;
        ++stop_writes;
        if (ignore_stop) {
          break;
        }
      }
      if (std::cmp_equal(type, ignored_param)) {
        break; // acknowledged and dropped
      }
      params_[type] = v;
      if ((type == 4U) && (v == 0) && (params_[9] == 1)) {
        pwm_started_ = false; // IDLE_MOTOR_PWM_BEHAVIOR 1: System off switches the PWM off
      }
      break;
    }
    case 6: { // GAP
      const tmc9660::ParamInfo* p = tmc9660::FindParam(type);
      if ((p == nullptr) || (p->access == tmc9660::Access::WriteOnly) ||
          (knobs.reject_motor && (motor != 0U))) {
        r.status = 3;
        break;
      }
      r.value = static_cast<uint32_t>(static_cast<int64_t>(params_[type]));
      break;
    }
    case 7:
      ++writes_since_reset;
      break;
    case 9: { // SGP
      if (motor > 3U) {
        r.status = 4;
        break;
      }
      ++writes_since_reset;
      if ((motor == 0U) && (type == 1U) && !odd(static_cast<int64_t>(raw))) {
        r.status = 4;
        break;
      }
      globals_[motor][type & 63U] =
          (motor == 2U) ? static_cast<int64_t>(sv) : static_cast<int64_t>(raw);
      break;
    }
    case 10:
      r.value = static_cast<uint32_t>(globals_[motor & 3U][type & 63U]);
      break;
    case 13:
      ++writes_since_reset;
      r.value = (type == 2U) ? 0U : 1U;
      break;
    case 14:
      ++writes_since_reset;
      break;
    case 15:
      r.value = (motor == 1U) ? 1234U : 1U;
      break;
    case 128:
      script_state_ = 0;
      break;
    case 129:
      script_state_ = 1;
      break;
    case 130:
      break;
    case 131:
      script_state_ = 0;
      break;
    case 132:
      download_ = true;
      download_count_ = 0;
      break;
    case 133:
      download_ = false;
      break;
    case 134:
      r.value = 0x1234'0000U | raw;
      break;
    case 135:
      r.value = static_cast<uint32_t>(script_state_);
      break;
    case 137:
      ++writes_since_reset;
      for (const tmc9660::ParamInfo& p : tmc9660::kParams) {
        params_[p.id] = p.def;
      }
      break;
    case 142:
      r.value = ramdebug_[type & 15U];
      if ((type != 8U) && (type != 9U) && (type != 10U)) {
        ramdebug_[type & 15U] = raw;
        ++writes_since_reset;
      }
      break;
    case 146:
      ++writes_since_reset;
      break;
    case 242:
      if ((type == 0x81U) && (motor == 0x92U) && (raw == 0xA3B4C5D6U)) {
        if (params_[4] != 0) {
          ++boot_with_drive_on;
        }
        phase_ = Phase::Bootloader;
        session_started_ = false;
        spi_first_ = true;
        ready_reply_valid_ = false;
        idle_image_ = true;
        cfg_[2] &= ~(1U << 12);
      } else {
        r.status = 4;
      }
      break;
    default:
      r.status = 2;
      break;
    }
    return r;
  }

  void buildSpiImage(uint8_t* out, uint8_t spi_status, const TmclOut& r,
                     uint8_t op) const noexcept {
    out[0] = spi_status;
    out[1] = r.status;
    out[2] = op;
    putBe(out + 3, r.value);
    out[7] = Sum(out, 7);
  }

  void paramSpi(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    if (len != 8U) {
      for (std::size_t i = 0; i < len; ++i) {
        rx[i] = 0;
      }
      return;
    }
    const bool reply_request = (tx[0] == 0xFFU) && (tx[7] == 0xFFU);
    const bool first_frame = spi_first_;
    bool deliver_cmd_reply = false;
    uint8_t spi_status = 0xFF;
    if (reply_request && reset_on_reply_request) {
      reset_on_reply_request = false;
      Reset();
    }
    if (reply_request) {
      if (ready_reply_valid_) {
        if (not_ready_replies > 0) {
          --not_ready_replies;
          spi_status = 0xF0;
        } else if (checksum_error_replies > 0) {
          --checksum_error_replies;
          spi_status = 0x00;
        } else {
          deliver_cmd_reply = true;
        }
      }
    }
    // What this frame clocks out.
    if (spi_first_) {
      rx[0] = 0x0C;
      for (std::size_t i = 1; i < 8U; ++i) {
        rx[i] = 0;
      }
      spi_first_ = false;
    } else if (reply_request && ready_reply_valid_) {
      if (deliver_cmd_reply) {
        for (std::size_t i = 0; i < 8U; ++i) {
          rx[i] = ready_reply_[i];
        }
        ready_reply_valid_ = false;
      } else {
        for (std::size_t i = 0; i < 8U; ++i) {
          rx[i] = ready_reply_[i];
        }
        rx[0] = spi_status;
        if (spi_status == 0x00U) {
          rx[7] = static_cast<uint8_t>(rx[7] ^ 0x11U);
        }
      }
    } else {
      // The reply to a non-command frame: idle image (OK status, nothing to say).
      rx[0] = 0xFF;
      for (std::size_t i = 1; i < 8U; ++i) {
        rx[i] = 0;
      }
    }
    if (reply_request) {
      return;
    }
    // A command frame.
    if (Sum(tx, 7) != tx[7]) {
      TmclOut bad{1, 0};
      buildSpiImage(ready_reply_.data(), 0xFF, bad, tx[0]);
      ready_reply_valid_ = true;
      return;
    }
    if (tx[0] == 0U) {
      ++zero_frames;
    }
    if (first_frame && ignore_first_command) {
      return; // the chip answers FIRST_CMD and runs nothing
    }
    unsigned type = 0;
    unsigned motor = 0;
    decodeTypeMotor(tx[1], tx[2], type, motor);
    const TmclOut r = exec(tx[0], type, motor, getBe(tx + 3));
    buildSpiImage(ready_reply_.data(), 0xFF, r,
                  wrong_op_echo ? static_cast<uint8_t>(tx[0] ^ 0x01U) : tx[0]);
    ready_reply_valid_ = true;
  }

  size_t tmclUart(const uint8_t* tx, std::size_t len, uint8_t* reply, bool reg_mode) noexcept {
    if ((len != 9U) || ((tx[0] & 1U) == 0U) || ((tx[0] & 0xFEU) != (device_ & 0xFEU)) ||
        (Sum(tx, 8) != tx[8])) {
      return 0;
    }
    TmclOut r{100, 0};
    uint8_t echo = tx[1];
    if (wrong_op_echo) {
      echo = static_cast<uint8_t>(echo ^ 0x01U);
    }
    if (reg_mode) {
      r = regUartExec(tx);
    } else {
      unsigned type = 0;
      unsigned motor = 0;
      decodeTypeMotor(tx[2], tx[3], type, motor);
      r = exec(tx[1], type, motor, getBe(tx + 4));
    }
    reply[0] = static_cast<uint8_t>(globals_[0][2]);
    reply[1] = static_cast<uint8_t>(device_ | 1U);
    reply[2] = r.status;
    reply[3] = echo;
    putBe(reply + 4, r.value);
    reply[8] = Sum(reply, 8);
    return 9;
  }

  // ----- register mode
  // ------------------------------------------------------------------------------------

  /// Execute a register access: returns false when the address is not a register.
  bool regAccess(uint8_t block, unsigned offset, bool write, uint32_t data,
                 uint32_t& out) noexcept {
    if (block == 31U) {
      out = ramdebug_[(offset >> knobs.ram_index_bits) & 15U];
      if (write) {
        ramdebug_[(offset >> knobs.ram_index_bits) & 15U] = data;
        ++writes_since_reset;
      }
      return true;
    }
    const tmc9660::RegAddr a{.block = static_cast<tmc9660::Block>(block),
                             .offset = static_cast<uint16_t>(offset)};
    const tmc9660::RegInfo* info = tmc9660::FindRegister(a);
    if (reg_status_fail) {
      reg_status_fail = false;
      out = 0;
      return false; // the access is reported as failed and not executed
    }
    if (info == nullptr) {
      out = 0;
      return reg_unmapped_ok;
    }
    if (write && (block == 0U) && (std::cmp_equal(offset, ignored_reg_write_offset))) {
      out = 0;
      return true; // acknowledged, not stored
    }
    if (write && corrupt_gdrv_hw && (block == 0U) &&
        (offset == tmc9660::regs::MCC_GDRV_HW.offset)) {
      data ^= 1U;
    }
    uint32_t& cell = regs_[regIndex(block, offset)];
    if (!write) {
      out = cell;
      if ((block == 0U) && (offset == tmc9660::regs::MCC_GDRV_STATUS.offset)) {
        out |= gdrv_fault_bits;
      }
      return true;
    }
    ++writes_since_reset;
    const uint32_t before = cell;
    uint32_t next = cell;
    const uint32_t w1c = info->w1c_mask;
    const uint32_t rw = info->writable_mask & ~w1c;
    next = (next & ~rw) | (data & rw);
    next &= ~(data & w1c);
    if ((block == 0U) && (offset == tmc9660::regs::MCC_GDRV_STATUS.offset) && (data != 0U)) {
      next |= relatch_on_clear; // the condition is still there: it latches again
    }
    cell = next;
    out = 0;
    if ((block == 0U) && (offset == tmc9660::regs::MCC_GDRV_HW.offset)) {
      const uint32_t en = 0x0300'080FU;
      if (((next & en) != 0U) && ((before & en) == 0U)) {
        ++energise_events;
        ++energise_chop_events;
        hw_energise_us = now_us;
        chop_at_energise = regs_[regIndex(0, tmc9660::regs::MCC_PWM_CONFIG.offset)] & 7U;
        if (!drv_en_) {
          ++energise_without_drv_en;
        }
        if (((regs_[regIndex(0, tmc9660::regs::MCC_GDRV_STATUS.offset)] | gdrv_fault_bits) != 0U)) {
          ++energise_with_flag;
        }
      }
      if (next == 0U) {
        ++disable_cmds;
        ++stop_writes;
      }
    }
    if ((block == 0U) && (offset == tmc9660::regs::MCC_GDRV_STATUS_EN.offset)) {
      status_en_us = now_us;
    }
    if ((block == 0U) && (offset == tmc9660::regs::MCC_GDRV_PROT_EN.offset)) {
      prot_en_us = now_us;
      regs_[regIndex(0, tmc9660::regs::MCC_GDRV_STATUS.offset)] |=
          static_cast<uint32_t>(trip_on_prot_en);
    }
    if ((block == 0U) && (offset == tmc9660::regs::MCC_PWM_CONFIG.offset) && (next == 0U)) {
      ++disable_cmds;
      ++stop_writes;
    }
    return true;
  }

  TmclOut regUartExec(const uint8_t* tx) noexcept {
    unsigned offset = 0;
    uint8_t block = 0;
    if (knobs.reg_uart_msb) {
      offset = tx[2] | (static_cast<unsigned>(tx[3] >> 5) << 8);
      block = static_cast<uint8_t>(tx[3] & 0x1FU);
    } else {
      offset = tx[2] | (static_cast<unsigned>(tx[3] & 7U) << 8);
      block = static_cast<uint8_t>(tx[3] >> 3);
    }
    TmclOut r{100, 0};
    uint32_t out = 0;
    if (tx[1] == 146U) {
      r.status = regAccess(block, offset, true, getBe(tx + 4), out) ? 100 : 3;
    } else if (tx[1] == 148U) {
      r.status = regAccess(block, offset, false, 0, out) ? 100 : 3;
      r.value = out;
    } else if (tx[1] == 142U) {
      r.status = regAccess(31, offset, true, getBe(tx + 4), out) ? 100 : 3;
      r.value = out;
    } else {
      r.status = 2;
    }
    last_cmd = tx[1];
    return r;
  }

  void regSpi(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    if (len != 6U) {
      for (std::size_t i = 0; i < len; ++i) {
        rx[i] = 0;
      }
      return;
    }
    // The reply to the previous datagram, with the boot bit on the first one and the datagram
    // counter.
    if (skip_reg_counter) {
      skip_reg_counter = false;
      reg_counter_ = static_cast<uint8_t>((reg_counter_ + 1U) & 7U);
    }
    if ((skip_reg_counter_after >= 0) && (reg_datagrams_ == skip_reg_counter_after)) {
      skip_reg_counter_after = -1;
      reg_counter_ = static_cast<uint8_t>((reg_counter_ + 1U) & 7U);
    }
    ++reg_datagrams_;
    const bool boot = reg_first_ || stray_boot_bit;
    stray_boot_bit = false;
    reg_first_ = false;
    const uint32_t value = boot ? 0U : pending_reg_.value;
    const bool ok = boot ? false : pending_reg_.ok;
    rx[0] = static_cast<uint8_t>((boot ? 0x80U : 0U) | ((reg_counter_ & 7U) << 4));
    rx[1] = ok ? 1U : 0U;
    putBe(rx + 2, value);
    reg_counter_ = static_cast<uint8_t>((reg_counter_ + 1U) & 7U);
    // Decode this request.
    const unsigned hi = (static_cast<unsigned>(tx[0]) << 8) | tx[1];
    bool write = false;
    uint8_t block = 0;
    unsigned offset = 0;
    if (knobs.reg_wide) {
      block = static_cast<uint8_t>((hi >> 11) & 0x1FU);
      write = ((hi >> 10) & 1U) != 0U;
      offset = hi & 0x3FFU;
    } else {
      block = static_cast<uint8_t>((hi >> 10) & 0x3FU);
      write = ((hi >> 9) & 1U) != 0U;
      offset = hi & 0x1FFU;
    }
    uint32_t out = 0;
    const bool ok2 = regAccess(block, offset, write, getBe(tx + 2), out);
    pending_reg_ = RegPending{out, 0, ok2};
    last_cmd = write ? 146 : 148;
  }

  static void putBe(uint8_t* p, uint32_t v) noexcept {
    p[0] = static_cast<uint8_t>(v >> 24);
    p[1] = static_cast<uint8_t>(v >> 16);
    p[2] = static_cast<uint8_t>(v >> 8);
    p[3] = static_cast<uint8_t>(v);
  }
  static uint32_t getBe(const uint8_t* p) noexcept {
    return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8) | p[3];
  }

  Phase phase_{Phase::Bootloader};
  Phase phase_switch_{Phase::Bootloader};
  bool switch_pending_{false};
  bool session_started_{false};
  bool spi_first_{true};
  bool reg_first_{true};
  uint8_t reg_counter_{0};
  int reg_datagrams_{0};
  uint8_t device_{1};
  uint8_t host_{255};
  bool drv_en_{false};
  uint32_t bank_{5};
  uint32_t address_{0};
  int pll_delay_{0};
  BlPending pending_bl_{0, 0};
  bool pwm_started_{false};
  RegPending pending_reg_{0, 0, false};
  std::array<uint8_t, 8> ready_reply_{};
  bool ready_reply_valid_{false};
  bool idle_image_{true};
  std::array<uint32_t, 16> cfg_{};
  std::array<uint32_t, 16> reset_cfg_{};
  std::array<int64_t, 512> params_{};
  std::array<std::array<int64_t, 64>, 4> globals_{};
  std::array<uint32_t, 4096> regs_{};
  std::array<uint32_t, 16> ramdebug_{};
  std::array<uint32_t, 1024> flash_{};
  int script_state_{0};
  bool download_{false};
  int download_count_{0};
};

/// The UART as a port sees it: a receive FIFO between the chip and the port. A reply can arrive
/// after the port gave up (a late reply) and then sits in the FIFO; a port that does not discard
/// pending bytes before it sends hands that stale reply to the next request, and every reply after
/// it is shifted by one (the TMCL reply carries no request identity).
class UartWire {
public:
  bool port_flushes{false}; ///< The port discards pending receive bytes before it sends.
  bool late_next{false};    ///< One-shot: the chip's next reply arrives after the port timed out.
  int late_op{-1}; ///< One-shot: the reply to the next TMCL request with this command arrives late.
  int late_replies{0};     ///< Replies that arrived late.
  int stale_bytes_seen{0}; ///< Stale bytes that were in the FIFO when a request was sent.

  /// One exchange: send `tx_len` bytes (0: only receive, a drain), then receive `rx_len` bytes.
  /// Returns false on a timeout; the bytes received before the timeout are consumed.
  bool Exchange(Tmc9660Model& chip, const uint8_t* tx, std::size_t tx_len, uint8_t* rx,
                std::size_t rx_len) noexcept {
    for (std::size_t i = 0; i < late_len_; ++i) { // arrived while the host was waiting
      push(late_[i]);
    }
    late_len_ = 0;
    if (tx_len > 0U) {
      if (port_flushes) {
        len_ = 0;
      }
      stale_bytes_seen += static_cast<int>(len_);
      std::array<uint8_t, 9> reply{};
      const std::size_t n = chip.UartRequest(tx, tx_len, reply.data());
      const bool op_late = (late_op >= 0) && (tx_len > 1U) && std::cmp_equal(tx[1], late_op);
      if ((n > 0U) && (late_next || op_late)) {
        late_next = false;
        if (op_late) {
          late_op = -1;
        }
        ++late_replies;
        for (std::size_t i = 0; i < n; ++i) {
          late_[i] = reply[i];
        }
        late_len_ = n;
      } else {
        for (std::size_t i = 0; i < n; ++i) {
          push(reply[i]);
        }
      }
    }
    if (len_ < rx_len) {
      len_ = 0;
      return false;
    }
    for (std::size_t i = 0; i < rx_len; ++i) {
      rx[i] = fifo_[i];
    }
    for (std::size_t i = rx_len; i < len_; ++i) {
      fifo_[i - rx_len] = fifo_[i];
    }
    len_ -= rx_len;
    return true;
  }
  /// A stale reply: what the chip answers to `req` is left in the FIFO, as if the request had been
  /// sent earlier and its reply never read.
  void InjectStale(Tmc9660Model& chip, const uint8_t* req, std::size_t req_len) noexcept {
    std::array<uint8_t, 9> reply{};
    const std::size_t n = chip.UartRequest(req, req_len, reply.data());
    for (std::size_t i = 0; i < n; ++i) {
      push(reply[i]);
    }
  }
  /// Bytes waiting in the FIFO.
  [[nodiscard]] std::size_t Pending() const noexcept {
    return len_;
  }

private:
  void push(uint8_t b) noexcept {
    if (len_ < fifo_.size()) {
      fifo_[len_++] = b;
    }
  }
  std::array<uint8_t, 64> fifo_{};
  std::size_t len_{0};
  std::array<uint8_t, 9> late_{};
  std::size_t late_len_{0};
};

} // namespace tmc9660_test

#endif // TMC9660_MOCK_TMC9660_HPP
