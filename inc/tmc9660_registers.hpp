// Generated from the TMC9660 Register Mode Reference Manual (Rev. 0, 04/25). Do not edit by hand.
/**
 * @file tmc9660_registers.hpp
 * @brief The registers of register mode: block, offset, reset value and access masks of every
 * register of the manual's register map, and the bit fields of each.
 * @copyright Copyright (c) 2026 The driver-tmc9660 authors. SPDX-License-Identifier: MIT
 */
#ifndef TMC9660_REGISTERS_HPP
#define TMC9660_REGISTERS_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "tmc9660_detail.hpp"

namespace tmc9660 {

/// Register blocks (Table 5 of the register manual).
enum class Block : uint8_t {
  Mcc = 0,       ///< Motion control core.
  Adc = 1,       ///< ADC configuration.
  SysCtrl = 2,   ///< System control.
  RamDebug = 31, ///< RAMDebug; offsets are subcommand and index, not register numbers.
};

/// Address of a register: its block and its offset in the block.
struct RegAddr {
  Block block;     ///< The register block.
  uint16_t offset; ///< The register number in the block.
};

/// One row of the register map.
struct RegInfo {
  RegAddr addr;           ///< Where the register is.
  uint32_t reset;         ///< Reset value (the fields' reset values combined).
  uint32_t readable_mask; ///< Bits the manual defines as fields.
  uint32_t writable_mask; ///< Bits of fields whose type is RW (including W1C).
  uint32_t w1c_mask;      ///< Bits that clear when 1 is written.
};

/// The registers by name.
namespace regs {
/// Chip ID.
inline constexpr RegAddr MCC_INFO_CHIP{.block = Block::Mcc, .offset = 0x000};
/// Raw phase currents I1, I0.
inline constexpr RegAddr MCC_ADC_I1_I0_RAW{.block = Block::Mcc, .offset = 0x020};
/// Raw phase currents I3, I2.
inline constexpr RegAddr MCC_ADC_I3_I2_RAW{.block = Block::Mcc, .offset = 0x021};
/// Measured phase voltages U1 and U0.
inline constexpr RegAddr MCC_ADC_U1_U0_RAW{.block = Block::Mcc, .offset = 0x022};
/// Measured phase voltage.
inline constexpr RegAddr MCC_ADC_U3_U2_RAW{.block = Block::Mcc, .offset = 0x023};
/// Raw Die temperature voltage and supply voltage monitoring value.
inline constexpr RegAddr MCC_ADC_TEMP_VM_RAW{.block = Block::Mcc, .offset = 0x024};
/// Raw analog values AIN1 and AIN0.
inline constexpr RegAddr MCC_ADC_AIN1_AIN0_RAW{.block = Block::Mcc, .offset = 0x025};
/// Raw analog values AIN3 and AIN2.
inline constexpr RegAddr MCC_ADC_AIN3_AIN2_RAW{.block = Block::Mcc, .offset = 0x026};
/// General configuration setup of the current ADCs.
inline constexpr RegAddr MCC_ADC_I_GEN_CONFIG{.block = Block::Mcc, .offset = 0x040};
/// Current ADC channel 0 offset and scaling values.
inline constexpr RegAddr MCC_ADC_I0_CONFIG{.block = Block::Mcc, .offset = 0x041};
/// Current ADC channel 1 offset and scaling values.
inline constexpr RegAddr MCC_ADC_I1_CONFIG{.block = Block::Mcc, .offset = 0x042};
/// Current ADC channel 2 offset and scaling values.
inline constexpr RegAddr MCC_ADC_I2_CONFIG{.block = Block::Mcc, .offset = 0x043};
/// Current ADC channel 3 offset and scaling values.
inline constexpr RegAddr MCC_ADC_I3_CONFIG{.block = Block::Mcc, .offset = 0x044};
/// Phase current I1, I0; after applying scaling and offset.
inline constexpr RegAddr MCC_ADC_I1_I0_SCALED{.block = Block::Mcc, .offset = 0x045};
/// Phase current I3, I2; after applying scaling and offset.
inline constexpr RegAddr MCC_ADC_I3_I2_SCALED{.block = Block::Mcc, .offset = 0x046};
/// Scaled current ADC value including signed added offset as input for the FOC.
inline constexpr RegAddr MCC_ADC_IWY_IUX{.block = Block::Mcc, .offset = 0x047};
/// Scaled current ADC value including signed added offset as input for the FOC.
inline constexpr RegAddr MCC_ADC_IV{.block = Block::Mcc, .offset = 0x048};
/// ADC status flags.
inline constexpr RegAddr MCC_ADC_STATUS{.block = Block::Mcc, .offset = 0x049};
/// Motor type configuration and number of pole pairs.
inline constexpr RegAddr MCC_MOTOR_CONFIG{.block = Block::Mcc, .offset = 0x060};
/// Register for selection of ramp mode, motion mode and controller feedforward.
inline constexpr RegAddr MCC_MOTION_CONFIG{.block = Block::Mcc, .offset = 0x061};
/// Selection of phi resp.
inline constexpr RegAddr MCC_PHI_E_SELECTION{.block = Block::Mcc, .offset = 0x062};
/// Angle used for the inner FOC loop.
inline constexpr RegAddr MCC_PHI_E{.block = Block::Mcc, .offset = 0x063};
/// PWM Configuration: Chopper Mode, Enable Space Vector, Enable Bridge, Y2 Source, Offset.
inline constexpr RegAddr MCC_PWM_CONFIG{.block = Block::Mcc, .offset = 0x080};
/// PWM counter maximum length register PWM_MAXCNT controls the PWM frequency.
inline constexpr RegAddr MCC_PWM_MAXCNT{.block = Block::Mcc, .offset = 0x081};
/// Determines a threshold for switching from three phase current measurement to two phase in Auto
/// K.
inline constexpr RegAddr MCC_PWM_SWITCH_LIMIT{.block = Block::Mcc, .offset = 0x083};
/// ABN Decoder electrical angle PHI_E and mechanical angle PHI_M.
inline constexpr RegAddr MCC_ABN_PHI_E_PHI_M{.block = Block::Mcc, .offset = 0x0a0};
/// ABN decoder configuration.
inline constexpr RegAddr MCC_ABN_MODE{.block = Block::Mcc, .offset = 0x0a1};
/// Decoder counts per revolution (CPR). ABN_CPR_INV needs to be set as well (ABN_CPR_INV = 2^32 /
/// A.
inline constexpr RegAddr MCC_ABN_CPR{.block = Block::Mcc, .offset = 0x0a2};
/// 2^32 divided by decoder counts per revolution.
inline constexpr RegAddr MCC_ABN_CPR_INV{.block = Block::Mcc, .offset = 0x0a3};
/// Raw decoder count.
inline constexpr RegAddr MCC_ABN_COUNT{.block = Block::Mcc, .offset = 0x0a4};
/// ABN_COUNT latched on N pulse, when N pulse clears ABN_COUNT then ABN_COUNT_N is also 0.
inline constexpr RegAddr MCC_ABN_COUNT_N{.block = Block::Mcc, .offset = 0x0a5};
/// Offset for ABN_PHI_E.
inline constexpr RegAddr MCC_ABN_PHI_E_OFFSET{.block = Block::Mcc, .offset = 0x0a6};
/// Hall decoder configuration.
inline constexpr RegAddr MCC_HALL_MODE{.block = Block::Mcc, .offset = 0x0c0};
/// Maximum dx for extrapolation (default for digital hall: (2^16)/6).
inline constexpr RegAddr MCC_HALL_DPHI_MAX{.block = Block::Mcc, .offset = 0x0c1};
/// Offset for electrical angle hall_phi_e of hall decoder.
inline constexpr RegAddr MCC_HALL_PHI_E_OFFSET{.block = Block::Mcc, .offset = 0x0c2};
/// Count of passed hall states.
inline constexpr RegAddr MCC_HALL_COUNT{.block = Block::Mcc, .offset = 0x0c3};
/// Hall decoder electrical angle PHI_E.
inline constexpr RegAddr MCC_HALL_PHI_E_EXTRAPOLATED_PHI_E{.block = Block::Mcc, .offset = 0x0c4};
/// Position of the hall sensor.
inline constexpr RegAddr MCC_HALL_POSITION_060_POSITION_000{.block = Block::Mcc, .offset = 0x0c5};
/// Position of the hall sensor.
inline constexpr RegAddr MCC_HALL_POSITION_180_POSITION_120{.block = Block::Mcc, .offset = 0x0c6};
/// Position of the hall sensor.
inline constexpr RegAddr MCC_HALL_POSITION_300_POSITION_240{.block = Block::Mcc, .offset = 0x0c7};
/// Biquad velocity filter coefficient A_1.
inline constexpr RegAddr MCC_BIQUAD_V_A_1{.block = Block::Mcc, .offset = 0x0e0};
/// Biquad velocity filter coefficient A_2.
inline constexpr RegAddr MCC_BIQUAD_V_A_2{.block = Block::Mcc, .offset = 0x0e1};
/// Biquad velocity filter coefficient B_0.
inline constexpr RegAddr MCC_BIQUAD_V_B_0{.block = Block::Mcc, .offset = 0x0e2};
/// Biquad velocity filter coefficient B_1.
inline constexpr RegAddr MCC_BIQUAD_V_B_1{.block = Block::Mcc, .offset = 0x0e3};
/// Biquad velocity filter coefficient B_2.
inline constexpr RegAddr MCC_BIQUAD_V_B_2{.block = Block::Mcc, .offset = 0x0e4};
/// Biquad velocity filter enable.
inline constexpr RegAddr MCC_BIQUAD_V_ENABLE{.block = Block::Mcc, .offset = 0x0e5};
/// Biquad torque filter coefficient A_1.
inline constexpr RegAddr MCC_BIQUAD_T_A_1{.block = Block::Mcc, .offset = 0x0e6};
/// Biquad torque filter coefficient A_2.
inline constexpr RegAddr MCC_BIQUAD_T_A_2{.block = Block::Mcc, .offset = 0x0e7};
/// Biquad torque filter coefficient B_0.
inline constexpr RegAddr MCC_BIQUAD_T_B_0{.block = Block::Mcc, .offset = 0x0e8};
/// Biquad torque filter coefficient B_1.
inline constexpr RegAddr MCC_BIQUAD_T_B_1{.block = Block::Mcc, .offset = 0x0e9};
/// Biquad torque filter coefficient B_2.
inline constexpr RegAddr MCC_BIQUAD_T_B_2{.block = Block::Mcc, .offset = 0x0ea};
/// Biquad torque filter enable.
inline constexpr RegAddr MCC_BIQUAD_T_ENABLE{.block = Block::Mcc, .offset = 0x0eb};
/// Velocity meter configuration.
inline constexpr RegAddr MCC_VELOCITY_CONFIG{.block = Block::Mcc, .offset = 0x100};
/// Scaling factor for velocity meter output.
inline constexpr RegAddr MCC_VELOCITY_SCALING{.block = Block::Mcc, .offset = 0x101};
/// Velocity meter configuration.
inline constexpr RegAddr MCC_V_MIN_POS_DEV_TIME_COUNTER_LIMIT{.block = Block::Mcc, .offset = 0x102};
/// Velocity deviation to generate tracking error flag.
inline constexpr RegAddr MCC_MAX_VEL_DEVIATION{.block = Block::Mcc, .offset = 0x103};
/// Position configuration.
inline constexpr RegAddr MCC_POSITION_CONFIG{.block = Block::Mcc, .offset = 0x120};
/// Absolute position deviation to generate tracking error flag.
inline constexpr RegAddr MCC_MAX_POS_DEVIATION{.block = Block::Mcc, .offset = 0x121};
/// Ramp status and switch event status flags.
inline constexpr RegAddr MCC_RAMPER_STATUS{.block = Block::Mcc, .offset = 0x140};
/// First acceleration value during EigthPoint ramp mode.
inline constexpr RegAddr MCC_RAMPER_A1{.block = Block::Mcc, .offset = 0x141};
/// Second acceleration value during EigthPoint ramp mode.
inline constexpr RegAddr MCC_RAMPER_A2{.block = Block::Mcc, .offset = 0x142};
/// Maximum acceleration value in top part of EigthPoint ramp.
inline constexpr RegAddr MCC_RAMPER_A_MAX{.block = Block::Mcc, .offset = 0x143};
/// Lower deceleration value during EigthPoint ramp mode.
inline constexpr RegAddr MCC_RAMPER_D1{.block = Block::Mcc, .offset = 0x144};
/// Higher deceleration value in EigthPoint ramp.
inline constexpr RegAddr MCC_RAMPER_D2{.block = Block::Mcc, .offset = 0x145};
/// Deceleration in top part of EightPoint ramp.
inline constexpr RegAddr MCC_RAMPER_D_MAX{.block = Block::Mcc, .offset = 0x146};
/// First velocity value during EigthPoint ramp mode.
inline constexpr RegAddr MCC_RAMPER_V_START{.block = Block::Mcc, .offset = 0x147};
/// First velocity value for ac-/deceleration value switching during EigthPoint ramp mode.
inline constexpr RegAddr MCC_RAMPER_V1{.block = Block::Mcc, .offset = 0x148};
/// Second velocity value for ac-/deceleration value switching during EigthPoint ramp mode.
inline constexpr RegAddr MCC_RAMPER_V2{.block = Block::Mcc, .offset = 0x149};
/// Stop Velocity in EightPoint Ramp.
inline constexpr RegAddr MCC_RAMPER_V_STOP{.block = Block::Mcc, .offset = 0x14a};
/// Maximum velocity value for positioning in EightPoint ramp.
inline constexpr RegAddr MCC_RAMPER_V_MAX{.block = Block::Mcc, .offset = 0x14b};
/// Target velocity value in EightPoint ramp.
inline constexpr RegAddr MCC_RAMPER_V_TARGET{.block = Block::Mcc, .offset = 0x14c};
/// Ramper mode configuration bits.
inline constexpr RegAddr MCC_RAMPER_SWITCH_MODE{.block = Block::Mcc, .offset = 0x14d};
/// Ramper timing configuration.
inline constexpr RegAddr MCC_RAMPER_TIME_CONFIG{.block = Block::Mcc, .offset = 0x14e};
/// Actual ramp acceleration value.
inline constexpr RegAddr MCC_RAMPER_A_ACTUAL{.block = Block::Mcc, .offset = 0x14f};
/// Actual multi-turn position of ramp controller.
inline constexpr RegAddr MCC_RAMPER_X_ACTUAL{.block = Block::Mcc, .offset = 0x150};
/// Actual velocity of Ramp controller output.
inline constexpr RegAddr MCC_RAMPER_V_ACTUAL{.block = Block::Mcc, .offset = 0x151};
/// Multi-turn target position of Ramp controller.
inline constexpr RegAddr MCC_RAMPER_X_TARGET{.block = Block::Mcc, .offset = 0x152};
/// PHI_E of ramp controller.
inline constexpr RegAddr MCC_RAMPER_PHI_E{.block = Block::Mcc, .offset = 0x153};
/// Offset value for calculation of RAMPER_PHI_E.
inline constexpr RegAddr MCC_RAMPER_PHI_E_OFFSET{.block = Block::Mcc, .offset = 0x154};
/// Gain and shift factor for Acc. feedforward.
inline constexpr RegAddr MCC_RAMPER_ACC_FF{.block = Block::Mcc, .offset = 0x155};
/// Latches RAMPER_X_ACTUAL on left or right switch or Encoder trigger.
inline constexpr RegAddr MCC_RAMPER_X_ACTUAL_LATCH{.block = Block::Mcc, .offset = 0x156};
/// Latches PID_POSITION_ACTUAL on left or right switch or Encoder trigger.
inline constexpr RegAddr MCC_POSITION_ACTUAL_LATCH{.block = Block::Mcc, .offset = 0x157};
/// Set the amplitude of the PRBS signal used by some settings of MCC_MOTION_CONFIG -> MOTION_MODE.
inline constexpr RegAddr MCC_PRBS_AMPLITUDE{.block = Block::Mcc, .offset = 0x160};
/// Set downsampling rate of PWM frequency to trigger new PRBS value generation.
inline constexpr RegAddr MCC_PRBS_DOWN_SAMPLING_RATIO{.block = Block::Mcc, .offset = 0x161};
/// PI controller configuration.
inline constexpr RegAddr MCC_PID_CONFIG{.block = Block::Mcc, .offset = 0x180};
/// Configuration of the PI Flux controller gains.
inline constexpr RegAddr MCC_PID_FLUX_COEFF{.block = Block::Mcc, .offset = 0x181};
/// Configuration of the PI Torque controller gains.
inline constexpr RegAddr MCC_PID_TORQUE_COEFF{.block = Block::Mcc, .offset = 0x182};
/// Configuration of the PI Fieldweakening controller gains.
inline constexpr RegAddr MCC_PID_FIELDWEAK_COEFF{.block = Block::Mcc, .offset = 0x183};
/// Maximum voltage allowed for fieldweakening.
inline constexpr RegAddr MCC_PID_U_S_MAX{.block = Block::Mcc, .offset = 0x184};
/// Configuration of the PI Velocity controller gains.
inline constexpr RegAddr MCC_PID_VELOCITY_COEFF{.block = Block::Mcc, .offset = 0x185};
/// Configuration of the PI Position controller gains.
inline constexpr RegAddr MCC_PID_POSITION_COEFF{.block = Block::Mcc, .offset = 0x186};
/// Position controller ignores position errors smaller than PID_POSITION_TOLERANCE if
/// EVENT_POS_REA.
inline constexpr RegAddr MCC_PID_POSITION_TOLERANCE{.block = Block::Mcc, .offset = 0x187};
/// Number of PWM periods the abs(PID_POSITION_ERROR) must stay within PID_POSITION_TOLERANCE after.
inline constexpr RegAddr MCC_PID_POSITION_TOLERANCE_DELAY{.block = Block::Mcc, .offset = 0x188};
/// Set maximum output voltage limit value.
inline constexpr RegAddr MCC_PID_UQ_UD_LIMITS{.block = Block::Mcc, .offset = 0x189};
/// Set maximum target absolute current for torque and flux PI controller.
inline constexpr RegAddr MCC_PID_TORQUE_FLUX_LIMITS{.block = Block::Mcc, .offset = 0x18a};
/// Set maximum absolute velocity for velocity PI controller.
inline constexpr RegAddr MCC_PID_VELOCITY_LIMIT{.block = Block::Mcc, .offset = 0x18b};
/// Set minimum target position for position PI controller.
inline constexpr RegAddr MCC_PID_POSITION_LIMIT_LOW{.block = Block::Mcc, .offset = 0x18c};
/// Set maximum target position for position PI controller.
inline constexpr RegAddr MCC_PID_POSITION_LIMIT_HIGH{.block = Block::Mcc, .offset = 0x18d};
/// PID target torque and target flux (for torque mode).
inline constexpr RegAddr MCC_PID_TORQUE_FLUX_TARGET{.block = Block::Mcc, .offset = 0x18e};
/// PID torque and flux offset.
inline constexpr RegAddr MCC_PID_TORQUE_FLUX_OFFSET{.block = Block::Mcc, .offset = 0x18f};
/// PID Target velocity (for velocity mode).
inline constexpr RegAddr MCC_PID_VELOCITY_TARGET{.block = Block::Mcc, .offset = 0x190};
/// PID velocity offset for feed forward control.
inline constexpr RegAddr MCC_PID_VELOCITY_OFFSET{.block = Block::Mcc, .offset = 0x191};
/// Target position register (for position mode).
inline constexpr RegAddr MCC_PID_POSITION_TARGET{.block = Block::Mcc, .offset = 0x192};
/// PID actual torque and flux.
inline constexpr RegAddr MCC_PID_TORQUE_FLUX_ACTUAL{.block = Block::Mcc, .offset = 0x193};
/// PID actual velocity.
inline constexpr RegAddr MCC_PID_VELOCITY_ACTUAL{.block = Block::Mcc, .offset = 0x194};
/// PID actual position.
inline constexpr RegAddr MCC_PID_POSITION_ACTUAL{.block = Block::Mcc, .offset = 0x195};
/// Offset for actual position.
inline constexpr RegAddr MCC_PID_POSITION_ACTUAL_OFFSET{.block = Block::Mcc, .offset = 0x196};
/// PID torque error.
inline constexpr RegAddr MCC_PID_TORQUE_ERROR{.block = Block::Mcc, .offset = 0x197};
/// PID flux error.
inline constexpr RegAddr MCC_PID_FLUX_ERROR{.block = Block::Mcc, .offset = 0x198};
/// PID velocity error.
inline constexpr RegAddr MCC_PID_VELOCITY_ERROR{.block = Block::Mcc, .offset = 0x199};
/// PID position error.
inline constexpr RegAddr MCC_PID_POSITION_ERROR{.block = Block::Mcc, .offset = 0x19a};
/// PID torque Integrator.
inline constexpr RegAddr MCC_PID_TORQUE_INTEGRATOR{.block = Block::Mcc, .offset = 0x19b};
/// PID flux Integrator.
inline constexpr RegAddr MCC_PID_FLUX_INTEGRATOR{.block = Block::Mcc, .offset = 0x19c};
/// PID velocity Integrator.
inline constexpr RegAddr MCC_PID_VELOCITY_INTEGRATOR{.block = Block::Mcc, .offset = 0x19d};
/// PID position Integrator.
inline constexpr RegAddr MCC_PID_POSITION_INTEGRATOR{.block = Block::Mcc, .offset = 0x19e};
/// PID target torque and target flux for readback.
inline constexpr RegAddr MCC_PIDIN_TORQUE_FLUX_TARGET{.block = Block::Mcc, .offset = 0x1a0};
/// PID target velocity for readback.
inline constexpr RegAddr MCC_PIDIN_VELOCITY_TARGET{.block = Block::Mcc, .offset = 0x1a1};
/// PID target position for readback.
inline constexpr RegAddr MCC_PIDIN_POSITION_TARGET{.block = Block::Mcc, .offset = 0x1a2};
/// PID target torque and target flux after PID_TORQUE_FLUX_LIMITS applied.
inline constexpr RegAddr MCC_PIDIN_TORQUE_FLUX_TARGET_LIMITED{.block = Block::Mcc, .offset = 0x1a3};
/// PID target velocity after PID_VELOCITY_LIMIT applied.
inline constexpr RegAddr MCC_PIDIN_VELOCITY_TARGET_LIMITED{.block = Block::Mcc, .offset = 0x1a4};
/// PID target position after PID_POSITION_LIMIT_LOW and PID_POSITION_LIMIT_HIGH applied.
inline constexpr RegAddr MCC_PIDIN_POSITION_TARGET_LIMITED{.block = Block::Mcc, .offset = 0x1a5};
/// Interim result of the FOC, IALPHA, and IBETA term.
inline constexpr RegAddr MCC_FOC_IBETA_IALPHA{.block = Block::Mcc, .offset = 0x1a6};
/// Interim result of the FOC, ID, and IQ term.
inline constexpr RegAddr MCC_FOC_IQ_ID{.block = Block::Mcc, .offset = 0x1a7};
/// Interim result of the FOC, UD, and UQ term.
inline constexpr RegAddr MCC_FOC_UQ_UD{.block = Block::Mcc, .offset = 0x1a8};
/// Interim result of the FOC, UD, and UQ term.
inline constexpr RegAddr MCC_FOC_UQ_UD_LIMITED{.block = Block::Mcc, .offset = 0x1a9};
/// Interim result of the FOC, UALPHA, and UBETA term.
inline constexpr RegAddr MCC_FOC_UBETA_UALPHA{.block = Block::Mcc, .offset = 0x1aa};
/// Interim result of the FOC, UUX, and UWY term.
inline constexpr RegAddr MCC_FOC_UWY_UUX{.block = Block::Mcc, .offset = 0x1ab};
/// Interim result of the FOC, UV term.
inline constexpr RegAddr MCC_FOC_UV{.block = Block::Mcc, .offset = 0x1ac};
/// Interim result PWM duty cycle UX1 and VX2.
inline constexpr RegAddr MCC_PWM_VX2_UX1{.block = Block::Mcc, .offset = 0x1ad};
/// Interim result PWM duty cycle WY1 and Y2.
inline constexpr RegAddr MCC_PWM_Y2_WY1{.block = Block::Mcc, .offset = 0x1ae};
/// Actual velocity measured by fixed frequency sampling.
inline constexpr RegAddr MCC_VELOCITY_FRQ{.block = Block::Mcc, .offset = 0x1af};
/// Actual velocity measured by period measurement.
inline constexpr RegAddr MCC_VELOCITY_PER{.block = Block::Mcc, .offset = 0x1b0};
/// Actual motor voltage and current.
inline constexpr RegAddr MCC_U_S_ACTUAL_I_S_ACTUAL{.block = Block::Mcc, .offset = 0x1c0};
/// Actual Power applied to motor (P = U×I).
inline constexpr RegAddr MCC_P_MOTOR{.block = Block::Mcc, .offset = 0x1c1};
/// Raw input signals PWM_IN, DIR, STP, Digital Hall Inputs, and digital ABN encoder inputs as raw
/// s.
inline constexpr RegAddr MCC_INPUTS_RAW{.block = Block::Mcc, .offset = 0x1c2};
/// Raw output signals for each PWM channel high- and low-side.
inline constexpr RegAddr MCC_OUTPUTS_RAW{.block = Block::Mcc, .offset = 0x1c3};
/// Status Flag BitVector Vector, individual status bits are set to '1' on error condition pulse
/// '1'.
inline constexpr RegAddr MCC_STATUS_FLAGS{.block = Block::Mcc, .offset = 0x1c4};
/// Controls the general gate driver setup.
inline constexpr RegAddr MCC_GDRV_HW{.block = Block::Mcc, .offset = 0x1e3};
/// Controls the gate driver behavior and current settings.
inline constexpr RegAddr MCC_GDRV_CFG{.block = Block::Mcc, .offset = 0x1e4};
/// Sets the T_DRIVE sink and source times for all channels.
inline constexpr RegAddr MCC_GDRV_TIMING{.block = Block::Mcc, .offset = 0x1e9};
/// Controls the BBM_L and BBM_H times for all channels.
inline constexpr RegAddr MCC_GDRV_BBM{.block = Block::Mcc, .offset = 0x1ea};
/// Contains the general and VGS related protection settings.
inline constexpr RegAddr MCC_GDRV_PROT{.block = Block::Mcc, .offset = 0x1eb};
/// Configures the overcurrent protection for phases U, V and W.
inline constexpr RegAddr MCC_GDRV_OCP_UVW{.block = Block::Mcc, .offset = 0x1ec};
/// Configures the overcurrent protection for phase Y2.
inline constexpr RegAddr MCC_GDRV_OCP_Y2{.block = Block::Mcc, .offset = 0x1ed};
/// Enables the protection mechanism for the different fault events, note that the fault needs to
/// be.
inline constexpr RegAddr MCC_GDRV_PROT_EN{.block = Block::Mcc, .offset = 0x1ee};
/// Enables the reporting of the different fault events.
inline constexpr RegAddr MCC_GDRV_STATUS_EN{.block = Block::Mcc, .offset = 0x1ef};
/// Contains the status of the different fault events.
inline constexpr RegAddr MCC_GDRV_STATUS{.block = Block::Mcc, .offset = 0x1f0};
/// Contains the status of the different fault events.
inline constexpr RegAddr MCC_GDRV_FAULT{.block = Block::Mcc, .offset = 0x1f1};
/// External writable ADC value for phase I1, I0.
inline constexpr RegAddr MCC_ADC_I1_I0_EXT{.block = Block::Mcc, .offset = 0x200};
/// External writable ADC value for phase I2. Note that depending on the selection in register
/// MCC_A.
inline constexpr RegAddr MCC_ADC_I2_EXT{.block = Block::Mcc, .offset = 0x201};
/// External writable PWM value.
inline constexpr RegAddr MCC_PWM_VX2_UX1_EXT{.block = Block::Mcc, .offset = 0x202};
/// External writable PWM value.
inline constexpr RegAddr MCC_PWM_Y2_WY1_EXT{.block = Block::Mcc, .offset = 0x203};
/// External writable PWM compare value for phase Y2_ALT.
inline constexpr RegAddr MCC_PWM_EXT_Y2_ALT{.block = Block::Mcc, .offset = 0x204};
/// External writable parameter for open-loop voltage control mode, useful during system setup.
inline constexpr RegAddr MCC_VOLTAGE_EXT{.block = Block::Mcc, .offset = 0x205};
/// Angle phi_e_ext and phi_m_ext for external writing into this register.
inline constexpr RegAddr MCC_PHI_EXT{.block = Block::Mcc, .offset = 0x206};
/// Actual velocity for external override.
inline constexpr RegAddr MCC_VELOCITY_EXT{.block = Block::Mcc, .offset = 0x208};
/// General system status flags.
inline constexpr RegAddr FAULT_STS{.block = Block::SysCtrl, .offset = 0x008};
/// General system status flags.
inline constexpr RegAddr FAULT_R_INT{.block = Block::SysCtrl, .offset = 0x009};
/// Mask for register FAULT_R_INT.
inline constexpr RegAddr FAULT_R_ENA_F{.block = Block::SysCtrl, .offset = 0x00a};
/// ADC sources config.
inline constexpr RegAddr ADC_SRC_CONFIG{.block = Block::Adc, .offset = 0x001};
/// ADC setup.
inline constexpr RegAddr ADC_SETUP{.block = Block::Adc, .offset = 0x002};
/// ADC status bits.
inline constexpr RegAddr ADC_STATUS{.block = Block::Adc, .offset = 0x005};
/// CSA setup.
inline constexpr RegAddr CSA_SETUP{.block = Block::Adc, .offset = 0x007};
} // namespace regs

/// The register map, sorted by block and offset.
inline constexpr std::array<RegInfo, 163> kRegisters{{
    {.addr = regs::MCC_INFO_CHIP,
     .reset = 0x544d0001U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_I1_I0_RAW,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_I3_I2_RAW,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_U1_U0_RAW,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_U3_U2_RAW,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_TEMP_VM_RAW,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_AIN1_AIN0_RAW,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_AIN3_AIN2_RAW,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_I_GEN_CONFIG,
     .reset = 0x000010e4U,
     .readable_mask = 0xffff1effU,
     .writable_mask = 0xffff1effU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_I0_CONFIG,
     .reset = 0xfc000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_I1_CONFIG,
     .reset = 0xfc000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_I2_CONFIG,
     .reset = 0xfc000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_I3_CONFIG,
     .reset = 0xfc000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_I1_I0_SCALED,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_I3_I2_SCALED,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_IWY_IUX,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_IV,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_STATUS,
     .reset = 0x00000000U,
     .readable_mask = 0x3fff3fffU,
     .writable_mask = 0x3fff3fffU,
     .w1c_mask = 0x3fff3fffU},
    {.addr = regs::MCC_MOTOR_CONFIG,
     .reset = 0x00000001U,
     .readable_mask = 0x0003007fU,
     .writable_mask = 0x0003007fU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_MOTION_CONFIG,
     .reset = 0x00000000U,
     .readable_mask = 0x000000ffU,
     .writable_mask = 0x000000ffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PHI_E_SELECTION,
     .reset = 0x00000000U,
     .readable_mask = 0x0000000fU,
     .writable_mask = 0x0000000fU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PHI_E,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PWM_CONFIG,
     .reset = 0x00000000U,
     .readable_mask = 0xfffffff7U,
     .writable_mask = 0xfffffff7U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PWM_MAXCNT,
     .reset = 0x000012bfU,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x0000ffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PWM_SWITCH_LIMIT,
     .reset = 0x0000ffffU,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x0000ffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ABN_PHI_E_PHI_M,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ABN_MODE,
     .reset = 0x00000000U,
     .readable_mask = 0x0000113fU,
     .writable_mask = 0x0000113fU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ABN_CPR,
     .reset = 0x00010000U,
     .readable_mask = 0x00ffffffU,
     .writable_mask = 0x00ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ABN_CPR_INV,
     .reset = 0x00010000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ABN_COUNT,
     .reset = 0x00000000U,
     .readable_mask = 0x00ffffffU,
     .writable_mask = 0x00ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ABN_COUNT_N,
     .reset = 0x00000000U,
     .readable_mask = 0x00ffffffU,
     .writable_mask = 0x00ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ABN_PHI_E_OFFSET,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x0000ffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_HALL_MODE,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ff73U,
     .writable_mask = 0x0000ff73U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_HALL_DPHI_MAX,
     .reset = 0x00002aaaU,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x0000ffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_HALL_PHI_E_OFFSET,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x0000ffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_HALL_COUNT,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_HALL_PHI_E_EXTRAPOLATED_PHI_E,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_HALL_POSITION_060_POSITION_000,
     .reset = 0x2aaa0000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_HALL_POSITION_180_POSITION_120,
     .reset = 0x80005555U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_HALL_POSITION_300_POSITION_240,
     .reset = 0xd555aaaaU,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_BIQUAD_V_A_1,
     .reset = 0x001c376bU,
     .readable_mask = 0x00ffffffU,
     .writable_mask = 0x00ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_BIQUAD_V_A_2,
     .reset = 0x00f38f52U,
     .readable_mask = 0x00ffffffU,
     .writable_mask = 0x00ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_BIQUAD_V_B_0,
     .reset = 0x00000e51U,
     .readable_mask = 0x00ffffffU,
     .writable_mask = 0x00ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_BIQUAD_V_B_1,
     .reset = 0x00001ca1U,
     .readable_mask = 0x00ffffffU,
     .writable_mask = 0x00ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_BIQUAD_V_B_2,
     .reset = 0x00000e51U,
     .readable_mask = 0x00ffffffU,
     .writable_mask = 0x00ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_BIQUAD_V_ENABLE,
     .reset = 0x00000001U,
     .readable_mask = 0x00000001U,
     .writable_mask = 0x00000001U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_BIQUAD_T_A_1,
     .reset = 0x00000000U,
     .readable_mask = 0x00ffffffU,
     .writable_mask = 0x00ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_BIQUAD_T_A_2,
     .reset = 0x00000000U,
     .readable_mask = 0x00ffffffU,
     .writable_mask = 0x00ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_BIQUAD_T_B_0,
     .reset = 0x00100000U,
     .readable_mask = 0x00ffffffU,
     .writable_mask = 0x00ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_BIQUAD_T_B_1,
     .reset = 0x00000000U,
     .readable_mask = 0x00ffffffU,
     .writable_mask = 0x00ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_BIQUAD_T_B_2,
     .reset = 0x00000000U,
     .readable_mask = 0x00ffffffU,
     .writable_mask = 0x00ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_BIQUAD_T_ENABLE,
     .reset = 0x00000000U,
     .readable_mask = 0x00000001U,
     .writable_mask = 0x00000001U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_VELOCITY_CONFIG,
     .reset = 0x00000000U,
     .readable_mask = 0x000077ffU,
     .writable_mask = 0x000077ffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_VELOCITY_SCALING,
     .reset = 0x000028f6U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x0000ffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_V_MIN_POS_DEV_TIME_COUNTER_LIMIT,
     .reset = 0x0001fff0U,
     .readable_mask = 0x7fffffffU,
     .writable_mask = 0x7fffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_MAX_VEL_DEVIATION,
     .reset = 0x00010000U,
     .readable_mask = 0x7fffffffU,
     .writable_mask = 0x7fffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_POSITION_CONFIG,
     .reset = 0x00000000U,
     .readable_mask = 0x000000ffU,
     .writable_mask = 0x000000ffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_MAX_POS_DEVIATION,
     .reset = 0x00008000U,
     .readable_mask = 0x7fffffffU,
     .writable_mask = 0x7fffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_STATUS,
     .reset = 0x00000000U,
     .readable_mask = 0x0003ffffU,
     .writable_mask = 0x00008638U,
     .w1c_mask = 0x00008638U},
    {.addr = regs::MCC_RAMPER_A1,
     .reset = 0x00010000U,
     .readable_mask = 0x007fffffU,
     .writable_mask = 0x007fffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_A2,
     .reset = 0x00010000U,
     .readable_mask = 0x007fffffU,
     .writable_mask = 0x007fffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_A_MAX,
     .reset = 0x00010000U,
     .readable_mask = 0x007fffffU,
     .writable_mask = 0x007fffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_D1,
     .reset = 0x00010000U,
     .readable_mask = 0x007fffffU,
     .writable_mask = 0x007fffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_D2,
     .reset = 0x00010000U,
     .readable_mask = 0x007fffffU,
     .writable_mask = 0x007fffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_D_MAX,
     .reset = 0x00010000U,
     .readable_mask = 0x007fffffU,
     .writable_mask = 0x007fffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_V_START,
     .reset = 0x00000000U,
     .readable_mask = 0x007fffffU,
     .writable_mask = 0x007fffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_V1,
     .reset = 0x00000000U,
     .readable_mask = 0x07ffffffU,
     .writable_mask = 0x07ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_V2,
     .reset = 0x00000000U,
     .readable_mask = 0x07ffffffU,
     .writable_mask = 0x07ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_V_STOP,
     .reset = 0x00000100U,
     .readable_mask = 0x007fffffU,
     .writable_mask = 0x007fffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_V_MAX,
     .reset = 0x07ffffffU,
     .readable_mask = 0x07ffffffU,
     .writable_mask = 0x07ffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_V_TARGET,
     .reset = 0x00000000U,
     .readable_mask = 0x0fffffffU,
     .writable_mask = 0x0fffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_SWITCH_MODE,
     .reset = 0x00000000U,
     .readable_mask = 0x000fdfffU,
     .writable_mask = 0x000fdfffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_TIME_CONFIG,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_A_ACTUAL,
     .reset = 0x00000000U,
     .readable_mask = 0x00ffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_X_ACTUAL,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_V_ACTUAL,
     .reset = 0x00000000U,
     .readable_mask = 0x0fffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_X_TARGET,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_PHI_E,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_PHI_E_OFFSET,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x0000ffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_ACC_FF,
     .reset = 0x00060000U,
     .readable_mask = 0x0007ffffU,
     .writable_mask = 0x0007ffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_RAMPER_X_ACTUAL_LATCH,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_POSITION_ACTUAL_LATCH,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PRBS_AMPLITUDE,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PRBS_DOWN_SAMPLING_RATIO,
     .reset = 0x00000000U,
     .readable_mask = 0x000000ffU,
     .writable_mask = 0x000000ffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_CONFIG,
     .reset = 0x00008550U,
     .readable_mask = 0x7f7ffffdU,
     .writable_mask = 0x7f7ffffdU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_FLUX_COEFF,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_TORQUE_COEFF,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_FIELDWEAK_COEFF,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_U_S_MAX,
     .reset = 0x00007fffU,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x0000ffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_VELOCITY_COEFF,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_POSITION_COEFF,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_POSITION_TOLERANCE,
     .reset = 0x00000000U,
     .readable_mask = 0x7fffffffU,
     .writable_mask = 0x7fffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_POSITION_TOLERANCE_DELAY,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x0000ffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_UQ_UD_LIMITS,
     .reset = 0x00005a81U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x0000ffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_TORQUE_FLUX_LIMITS,
     .reset = 0x7fff7fffU,
     .readable_mask = 0x7fff7fffU,
     .writable_mask = 0x7fff7fffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_VELOCITY_LIMIT,
     .reset = 0x7fffffffU,
     .readable_mask = 0x7fffffffU,
     .writable_mask = 0x7fffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_POSITION_LIMIT_LOW,
     .reset = 0x80000001U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_POSITION_LIMIT_HIGH,
     .reset = 0x7fffffffU,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_TORQUE_FLUX_TARGET,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_TORQUE_FLUX_OFFSET,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_VELOCITY_TARGET,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_VELOCITY_OFFSET,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_POSITION_TARGET,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_TORQUE_FLUX_ACTUAL,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_VELOCITY_ACTUAL,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_POSITION_ACTUAL,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_POSITION_ACTUAL_OFFSET,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_TORQUE_ERROR,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_FLUX_ERROR,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_VELOCITY_ERROR,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_POSITION_ERROR,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_TORQUE_INTEGRATOR,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_FLUX_INTEGRATOR,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_VELOCITY_INTEGRATOR,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PID_POSITION_INTEGRATOR,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PIDIN_TORQUE_FLUX_TARGET,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PIDIN_VELOCITY_TARGET,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PIDIN_POSITION_TARGET,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PIDIN_TORQUE_FLUX_TARGET_LIMITED,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PIDIN_VELOCITY_TARGET_LIMITED,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PIDIN_POSITION_TARGET_LIMITED,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_FOC_IBETA_IALPHA,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_FOC_IQ_ID,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_FOC_UQ_UD,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_FOC_UQ_UD_LIMITED,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_FOC_UBETA_UALPHA,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_FOC_UWY_UUX,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_FOC_UV,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PWM_VX2_UX1,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PWM_Y2_WY1,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_VELOCITY_FRQ,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_VELOCITY_PER,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_U_S_ACTUAL_I_S_ACTUAL,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_P_MOTOR,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_INPUTS_RAW,
     .reset = 0x00000000U,
     .readable_mask = 0x0070f707U,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_OUTPUTS_RAW,
     .reset = 0x00000000U,
     .readable_mask = 0x000000ffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_STATUS_FLAGS,
     .reset = 0x00000000U,
     .readable_mask = 0x94f27bffU,
     .writable_mask = 0x94f27bffU,
     .w1c_mask = 0x94f27bffU},
    {.addr = regs::MCC_GDRV_HW,
     .reset = 0x00000000U,
     .readable_mask = 0x13000fffU,
     .writable_mask = 0x13000fffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_GDRV_CFG,
     .reset = 0x00030000U,
     .readable_mask = 0x00f3ffffU,
     .writable_mask = 0x00f3ffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_GDRV_TIMING,
     .reset = 0xffffffffU,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_GDRV_BBM,
     .reset = 0x14141414U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_GDRV_PROT,
     .reset = 0x00000000U,
     .readable_mask = 0x10ff3737U,
     .writable_mask = 0x10ff3737U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_GDRV_OCP_UVW,
     .reset = 0x00000000U,
     .readable_mask = 0x0f778f77U,
     .writable_mask = 0x0f778f77U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_GDRV_OCP_Y2,
     .reset = 0x00000000U,
     .readable_mask = 0x0f778f77U,
     .writable_mask = 0x0f778f77U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_GDRV_PROT_EN,
     .reset = 0x00000000U,
     .readable_mask = 0xafffffffU,
     .writable_mask = 0xafffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_GDRV_STATUS_EN,
     .reset = 0xefffffffU,
     .readable_mask = 0xefffffffU,
     .writable_mask = 0xefffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_GDRV_STATUS,
     .reset = 0x00000000U,
     .readable_mask = 0xefffffffU,
     .writable_mask = 0xefffffffU,
     .w1c_mask = 0xefffffffU},
    {.addr = regs::MCC_GDRV_FAULT,
     .reset = 0x00000000U,
     .readable_mask = 0xe00ff00fU,
     .writable_mask = 0x000f000fU,
     .w1c_mask = 0x000f000fU},
    {.addr = regs::MCC_ADC_I1_I0_EXT,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_ADC_I2_EXT,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x0000ffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PWM_VX2_UX1_EXT,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PWM_Y2_WY1_EXT,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PWM_EXT_Y2_ALT,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ffffU,
     .writable_mask = 0x0000ffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_VOLTAGE_EXT,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_PHI_EXT,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::MCC_VELOCITY_EXT,
     .reset = 0x00000000U,
     .readable_mask = 0xffffffffU,
     .writable_mask = 0xffffffffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::ADC_SRC_CONFIG,
     .reset = 0xb9b9b9b9U,
     .readable_mask = 0xbfffbfffU,
     .writable_mask = 0xbfffbfffU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::ADC_SETUP,
     .reset = 0x00000000U,
     .readable_mask = 0x000f0000U,
     .writable_mask = 0x000f0000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::ADC_STATUS,
     .reset = 0x00000000U,
     .readable_mask = 0x0000ff0fU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::CSA_SETUP,
     .reset = 0x00000000U,
     .readable_mask = 0x000ff77fU,
     .writable_mask = 0x000ff77fU,
     .w1c_mask = 0x00000000U},
    {.addr = regs::FAULT_STS,
     .reset = 0x00000000U,
     .readable_mask = 0x00001fffU,
     .writable_mask = 0x00000000U,
     .w1c_mask = 0x00000000U},
    {.addr = regs::FAULT_R_INT,
     .reset = 0x00000000U,
     .readable_mask = 0x00009fffU,
     .writable_mask = 0x00009fffU,
     .w1c_mask = 0x00001fffU},
    {.addr = regs::FAULT_R_ENA_F,
     .reset = 0x00000783U,
     .readable_mask = 0x00001fffU,
     .writable_mask = 0x00001fffU,
     .w1c_mask = 0x00000000U},
}};

/**
 * @brief The map row of a register.
 * @param addr The register.
 * @return The row, or `nullptr` when the manual lists none at that address.
 */
[[nodiscard]] constexpr const RegInfo* FindRegister(RegAddr addr) noexcept {
  for (const RegInfo& info : kRegisters) {
    if ((info.addr.block == addr.block) && (info.addr.offset == addr.offset)) {
      return &info;
    }
  }
  return nullptr;
}

/// Bit fields, one lower-case namespace per register: `fields::mcc_pwm_config::EnableX`.
namespace fields {
/// Fields of MCC_INFO_CHIP.
namespace mcc_info_chip {}
/// Fields of MCC_ADC_I1_I0_RAW.
namespace mcc_adc_i1_i0_raw {
/// Bits 31:16, R, signed: the manual's I1.
using I1 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's I0.
using I0 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_i1_i0_raw
/// Fields of MCC_ADC_I3_I2_RAW.
namespace mcc_adc_i3_i2_raw {
/// Bits 31:16, R, signed: the manual's I3.
using I3 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's I2.
using I2 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_i3_i2_raw
/// Fields of MCC_ADC_U1_U0_RAW.
namespace mcc_adc_u1_u0_raw {
/// Bits 31:16, R, signed: the manual's U1.
using U1 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's U0.
using U0 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_u1_u0_raw
/// Fields of MCC_ADC_U3_U2_RAW.
namespace mcc_adc_u3_u2_raw {
/// Bits 31:16, R, signed: the manual's U3.
using U3 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's U2.
using U2 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_u3_u2_raw
/// Fields of MCC_ADC_TEMP_VM_RAW.
namespace mcc_adc_temp_vm_raw {
/// Bits 31:16, R, signed: the manual's TEMP.
using Temp = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's VM.
using Vm = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_temp_vm_raw
/// Fields of MCC_ADC_AIN1_AIN0_RAW.
namespace mcc_adc_ain1_ain0_raw {
/// Bits 31:16, R, signed: the manual's AIN1.
using Ain1 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's AIN0.
using Ain0 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_ain1_ain0_raw
/// Fields of MCC_ADC_AIN3_AIN2_RAW.
namespace mcc_adc_ain3_ain2_raw {
/// Bits 31:16, R, signed: the manual's AIN3.
using Ain3 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's AIN2.
using Ain2 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_ain3_ain2_raw
/// Fields of MCC_ADC_I_GEN_CONFIG.
namespace mcc_adc_i_gen_config {
/// Bits 31:16, RW, unsigned: the manual's TRIGGER_POS.
using TriggerPos = detail::Field<uint32_t, 16U, 16U>;
/// Bits 12:12, RW: the manual's TRIGGER_SELECT.
using TriggerSelect = detail::Field<uint32_t, 12U, 1U>;
/// Bits 11:9, RW: the manual's MEASUREMENT_MODE.
using MeasurementMode = detail::Field<uint32_t, 9U, 3U>;
/// Bits 7:6, RW: the manual's Y2_SELECT.
using Y2Select = detail::Field<uint32_t, 6U, 2U>;
/// Bits 5:4, RW: the manual's WY1_SELECT.
using Wy1Select = detail::Field<uint32_t, 4U, 2U>;
/// Bits 3:2, RW: the manual's VX2_SELECT.
using Vx2Select = detail::Field<uint32_t, 2U, 2U>;
/// Bits 1:0, RW: the manual's UX1_SELECT.
using Ux1Select = detail::Field<uint32_t, 0U, 2U>;
} // namespace mcc_adc_i_gen_config
/// Fields of MCC_ADC_I0_CONFIG.
namespace mcc_adc_i0_config {
/// Bits 31:16, RW, signed: the manual's SCALE.
using Scale = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's OFFSET.
using Offset = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_i0_config
/// Fields of MCC_ADC_I1_CONFIG.
namespace mcc_adc_i1_config {
/// Bits 31:16, RW, signed: the manual's SCALE.
using Scale = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's OFFSET.
using Offset = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_i1_config
/// Fields of MCC_ADC_I2_CONFIG.
namespace mcc_adc_i2_config {
/// Bits 31:16, RW, signed: the manual's SCALE.
using Scale = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's OFFSET.
using Offset = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_i2_config
/// Fields of MCC_ADC_I3_CONFIG.
namespace mcc_adc_i3_config {
/// Bits 31:16, RW, signed: the manual's SCALE.
using Scale = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's OFFSET.
using Offset = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_i3_config
/// Fields of MCC_ADC_I1_I0_SCALED.
namespace mcc_adc_i1_i0_scaled {
/// Bits 31:16, R, signed: the manual's I1.
using I1 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's I0.
using I0 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_i1_i0_scaled
/// Fields of MCC_ADC_I3_I2_SCALED.
namespace mcc_adc_i3_i2_scaled {
/// Bits 31:16, R, signed: the manual's I3.
using I3 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's I2.
using I2 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_i3_i2_scaled
/// Fields of MCC_ADC_IWY_IUX.
namespace mcc_adc_iwy_iux {
/// Bits 31:16, R, signed: the manual's IWY.
using Iwy = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's IUX.
using Iux = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_iwy_iux
/// Fields of MCC_ADC_IV.
namespace mcc_adc_iv {
/// Bits 15:0, R, signed: the manual's IV.
using Iv = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_iv
/// Fields of MCC_ADC_STATUS.
namespace mcc_adc_status {
/// Bits 29:29, RW, W1C: the manual's TEMP_DONE.
using TempDone = detail::Field<uint32_t, 29U, 1U>;
/// Bits 28:28, RW, W1C: the manual's VM_DONE.
using VmDone = detail::Field<uint32_t, 28U, 1U>;
/// Bits 27:27, RW, W1C: the manual's AIN3_DONE.
using Ain3Done = detail::Field<uint32_t, 27U, 1U>;
/// Bits 26:26, RW, W1C: the manual's AIN2_DONE.
using Ain2Done = detail::Field<uint32_t, 26U, 1U>;
/// Bits 25:25, RW, W1C: the manual's AIN1_DONE.
using Ain1Done = detail::Field<uint32_t, 25U, 1U>;
/// Bits 24:24, RW, W1C: the manual's AIN0_DONE.
using Ain0Done = detail::Field<uint32_t, 24U, 1U>;
/// Bits 23:23, RW, W1C: the manual's U3_DONE.
using U3Done = detail::Field<uint32_t, 23U, 1U>;
/// Bits 22:22, RW, W1C: the manual's U2_DONE.
using U2Done = detail::Field<uint32_t, 22U, 1U>;
/// Bits 21:21, RW, W1C: the manual's U1_DONE.
using U1Done = detail::Field<uint32_t, 21U, 1U>;
/// Bits 20:20, RW, W1C: the manual's U0_DONE.
using U0Done = detail::Field<uint32_t, 20U, 1U>;
/// Bits 19:19, RW, W1C: the manual's I3_DONE.
using I3Done = detail::Field<uint32_t, 19U, 1U>;
/// Bits 18:18, RW, W1C: the manual's I2_DONE.
using I2Done = detail::Field<uint32_t, 18U, 1U>;
/// Bits 17:17, RW, W1C: the manual's I1_DONE.
using I1Done = detail::Field<uint32_t, 17U, 1U>;
/// Bits 16:16, RW, W1C: the manual's I0_DONE.
using I0Done = detail::Field<uint32_t, 16U, 1U>;
/// Bits 13:13, RW, W1C: the manual's TEMP_CLIPPED.
using TempClipped = detail::Field<uint32_t, 13U, 1U>;
/// Bits 12:12, RW, W1C: the manual's VM_CLIPPED.
using VmClipped = detail::Field<uint32_t, 12U, 1U>;
/// Bits 11:11, RW, W1C: the manual's AIN3_CLIPPED.
using Ain3Clipped = detail::Field<uint32_t, 11U, 1U>;
/// Bits 10:10, RW, W1C: the manual's AIN2_CLIPPED.
using Ain2Clipped = detail::Field<uint32_t, 10U, 1U>;
/// Bits 9:9, RW, W1C: the manual's AIN1_CLIPPED.
using Ain1Clipped = detail::Field<uint32_t, 9U, 1U>;
/// Bits 8:8, RW, W1C: the manual's AIN0_CLIPPED.
using Ain0Clipped = detail::Field<uint32_t, 8U, 1U>;
/// Bits 7:7, RW, W1C: the manual's U3_CLIPPED.
using U3Clipped = detail::Field<uint32_t, 7U, 1U>;
/// Bits 6:6, RW, W1C: the manual's U2_CLIPPED.
using U2Clipped = detail::Field<uint32_t, 6U, 1U>;
/// Bits 5:5, RW, W1C: the manual's U1_CLIPPED.
using U1Clipped = detail::Field<uint32_t, 5U, 1U>;
/// Bits 4:4, RW, W1C: the manual's U0_CLIPPED.
using U0Clipped = detail::Field<uint32_t, 4U, 1U>;
/// Bits 3:3, RW, W1C: the manual's I3_CLIPPED.
using I3Clipped = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, RW, W1C: the manual's I2_CLIPPED.
using I2Clipped = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, RW, W1C: the manual's I1_CLIPPED.
using I1Clipped = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, RW, W1C: the manual's I0_CLIPPED.
using I0Clipped = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_adc_status
/// Fields of MCC_MOTOR_CONFIG.
namespace mcc_motor_config {
/// Bits 17:16, RW: the manual's TYPE.
using Type = detail::Field<uint32_t, 16U, 2U>;
/// Bits 6:0, RW, unsigned: the manual's N_POLE_PAIRS.
using NPolePairs = detail::Field<uint32_t, 0U, 7U>;
} // namespace mcc_motor_config
/// Fields of MCC_MOTION_CONFIG.
namespace mcc_motion_config {
/// Bits 7:6, RW: the manual's FEEDFORWARD.
using Feedforward = detail::Field<uint32_t, 6U, 2U>;
/// Bits 5:5, RW: the manual's RAMP_MODE.
using RampMode = detail::Field<uint32_t, 5U, 1U>;
/// Bits 4:4, RW: the manual's RAMP_ENABLE.
using RampEnable = detail::Field<uint32_t, 4U, 1U>;
/// Bits 3:0, RW: the manual's MOTION_MODE.
using MotionMode = detail::Field<uint32_t, 0U, 4U>;
} // namespace mcc_motion_config
/// Fields of MCC_PHI_E_SELECTION.
namespace mcc_phi_e_selection {
/// Bits 3:0, RW: the manual's PHI_E_SELECTION.
using PhiESelection = detail::Field<uint32_t, 0U, 4U>;
} // namespace mcc_phi_e_selection
/// Fields of MCC_PHI_E.
namespace mcc_phi_e {
/// Bits 15:0, R, signed: the manual's PHI_E.
using PhiE = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_phi_e
/// Fields of MCC_PWM_CONFIG.
namespace mcc_pwm_config {
/// Bits 31:16, RW, unsigned: the manual's DUTY_CYCLE_OFFSET.
using DutyCycleOffset = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:15, RW: the manual's EXT_ENABLE_Y2.
using ExtEnableY2 = detail::Field<uint32_t, 15U, 1U>;
/// Bits 14:14, RW: the manual's EXT_ENABLE_WY1.
using ExtEnableWy1 = detail::Field<uint32_t, 14U, 1U>;
/// Bits 13:13, RW: the manual's EXT_ENABLE_VX2.
using ExtEnableVx2 = detail::Field<uint32_t, 13U, 1U>;
/// Bits 12:12, RW: the manual's EXT_ENABLE_UX1.
using ExtEnableUx1 = detail::Field<uint32_t, 12U, 1U>;
/// Bits 11:11, RW: the manual's ENABLE_Y2.
using EnableY2 = detail::Field<uint32_t, 11U, 1U>;
/// Bits 10:10, RW: the manual's ENABLE_WY1.
using EnableWy1 = detail::Field<uint32_t, 10U, 1U>;
/// Bits 9:9, RW: the manual's ENABLE_VX2.
using EnableVx2 = detail::Field<uint32_t, 9U, 1U>;
/// Bits 8:8, RW: the manual's ENABLE_UX1.
using EnableUx1 = detail::Field<uint32_t, 8U, 1U>;
/// Bits 7:6, RW: the manual's Y2_HS_SRC.
using Y2HsSrc = detail::Field<uint32_t, 6U, 2U>;
/// Bits 5:4, RW: the manual's SV_MODE.
using SvMode = detail::Field<uint32_t, 4U, 2U>;
/// Bits 2:0, RW: the manual's CHOP.
using Chop = detail::Field<uint32_t, 0U, 3U>;
} // namespace mcc_pwm_config
/// Fields of MCC_PWM_MAXCNT.
namespace mcc_pwm_maxcnt {
/// Bits 15:0, RW, unsigned: the manual's PWM_MAXCNT.
using PwmMaxcnt = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pwm_maxcnt
/// Fields of MCC_PWM_SWITCH_LIMIT.
namespace mcc_pwm_switch_limit {
/// Bits 15:0, RW, unsigned: the manual's PWM_SWITCH_LIMIT.
using PwmSwitchLimit = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pwm_switch_limit
/// Fields of MCC_ABN_PHI_E_PHI_M.
namespace mcc_abn_phi_e_phi_m {
/// Bits 31:16, R, signed: the manual's PHI_E.
using PhiE = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's PHI_M.
using PhiM = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_abn_phi_e_phi_m
/// Fields of MCC_ABN_MODE.
namespace mcc_abn_mode {
/// Bits 12:12, RW: the manual's DIRECTION.
using Direction = detail::Field<uint32_t, 12U, 1U>;
/// Bits 8:8, RW: the manual's CLN.
using Cln = detail::Field<uint32_t, 8U, 1U>;
/// Bits 5:5, RW: the manual's DISABLE_FILTER.
using DisableFilter = detail::Field<uint32_t, 5U, 1U>;
/// Bits 4:4, RW: the manual's CLEAR_COUNT_ON_N.
using ClearCountOnN = detail::Field<uint32_t, 4U, 1U>;
/// Bits 3:3, RW: the manual's COMBINED_N.
using CombinedN = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, RW: the manual's N_POL.
using NPol = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, RW: the manual's B_POL.
using BPol = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, RW: the manual's A_POL.
using APol = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_abn_mode
/// Fields of MCC_ABN_CPR.
namespace mcc_abn_cpr {
/// Bits 23:0, RW, unsigned: the manual's ABN_CPR.
using AbnCpr = detail::Field<uint32_t, 0U, 24U>;
} // namespace mcc_abn_cpr
/// Fields of MCC_ABN_CPR_INV.
namespace mcc_abn_cpr_inv {}
/// Fields of MCC_ABN_COUNT.
namespace mcc_abn_count {
/// Bits 23:0, RW, unsigned: the manual's ABN_COUNT.
using AbnCount = detail::Field<uint32_t, 0U, 24U>;
} // namespace mcc_abn_count
/// Fields of MCC_ABN_COUNT_N.
namespace mcc_abn_count_n {
/// Bits 23:0, RW, unsigned: the manual's ABN_COUNT_N.
using AbnCountN = detail::Field<uint32_t, 0U, 24U>;
} // namespace mcc_abn_count_n
/// Fields of MCC_ABN_PHI_E_OFFSET.
namespace mcc_abn_phi_e_offset {
/// Bits 15:0, RW, signed: the manual's ABN_PHI_E_OFFSET.
using AbnPhiEOffset = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_abn_phi_e_offset
/// Fields of MCC_HALL_MODE.
namespace mcc_hall_mode {
/// Bits 15:8, RW, unsigned: the manual's FILTER.
using Filter = detail::Field<uint32_t, 8U, 8U>;
/// Bits 6:4, RW: the manual's ORDER.
using Order = detail::Field<uint32_t, 4U, 3U>;
/// Bits 1:1, RW: the manual's EXTRAPOLATION.
using Extrapolation = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, RW: the manual's POLARITY.
using Polarity = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_hall_mode
/// Fields of MCC_HALL_DPHI_MAX.
namespace mcc_hall_dphi_max {
/// Bits 15:0, RW, unsigned: the manual's HALL_DPHI_MAX.
using HallDphiMax = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_hall_dphi_max
/// Fields of MCC_HALL_PHI_E_OFFSET.
namespace mcc_hall_phi_e_offset {
/// Bits 15:0, RW, signed: the manual's HALL_PHI_E_OFFSET.
using HallPhiEOffset = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_hall_phi_e_offset
/// Fields of MCC_HALL_COUNT.
namespace mcc_hall_count {
/// Bits 15:0, R, signed: the manual's HALL_COUNT.
using HallCount = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_hall_count
/// Fields of MCC_HALL_PHI_E_EXTRAPOLATED_PHI_E.
namespace mcc_hall_phi_e_extrapolated_phi_e {
/// Bits 31:16, R, signed: the manual's PHI_E_EXTRAPOLATED.
using PhiEExtrapolated = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's PHI_E.
using PhiE = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_hall_phi_e_extrapolated_phi_e
/// Fields of MCC_HALL_POSITION_060_POSITION_000.
namespace mcc_hall_position_060_position_000 {
/// Bits 31:16, RW, signed: the manual's POSITION_060.
using Position060 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's POSITION_000.
using Position000 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_hall_position_060_position_000
/// Fields of MCC_HALL_POSITION_180_POSITION_120.
namespace mcc_hall_position_180_position_120 {
/// Bits 31:16, RW, signed: the manual's POSITION_180.
using Position180 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's POSITION_120.
using Position120 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_hall_position_180_position_120
/// Fields of MCC_HALL_POSITION_300_POSITION_240.
namespace mcc_hall_position_300_position_240 {
/// Bits 31:16, RW, signed: the manual's POSITION_300.
using Position300 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's POSITION_240.
using Position240 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_hall_position_300_position_240
/// Fields of MCC_BIQUAD_V_A_1.
namespace mcc_biquad_v_a_1 {
/// Bits 23:0, RW, signed: the manual's BIQUAD_V_A_1.
using BiquadVA1 = detail::Field<uint32_t, 0U, 24U>;
} // namespace mcc_biquad_v_a_1
/// Fields of MCC_BIQUAD_V_A_2.
namespace mcc_biquad_v_a_2 {
/// Bits 23:0, RW, signed: the manual's BIQUAD_V_A_2.
using BiquadVA2 = detail::Field<uint32_t, 0U, 24U>;
} // namespace mcc_biquad_v_a_2
/// Fields of MCC_BIQUAD_V_B_0.
namespace mcc_biquad_v_b_0 {
/// Bits 23:0, RW, signed: the manual's BIQUAD_V_B_0.
using BiquadVB0 = detail::Field<uint32_t, 0U, 24U>;
} // namespace mcc_biquad_v_b_0
/// Fields of MCC_BIQUAD_V_B_1.
namespace mcc_biquad_v_b_1 {
/// Bits 23:0, RW, signed: the manual's BIQUAD_V_B_1.
using BiquadVB1 = detail::Field<uint32_t, 0U, 24U>;
} // namespace mcc_biquad_v_b_1
/// Fields of MCC_BIQUAD_V_B_2.
namespace mcc_biquad_v_b_2 {
/// Bits 23:0, RW, signed: the manual's BIQUAD_V_B_2.
using BiquadVB2 = detail::Field<uint32_t, 0U, 24U>;
} // namespace mcc_biquad_v_b_2
/// Fields of MCC_BIQUAD_V_ENABLE.
namespace mcc_biquad_v_enable {
/// Bits 0:0, RW: the manual's BIQUAD_V_ENABLE.
using BiquadVEnable = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_biquad_v_enable
/// Fields of MCC_BIQUAD_T_A_1.
namespace mcc_biquad_t_a_1 {
/// Bits 23:0, RW, signed: the manual's BIQUAD_T_A_1.
using BiquadTA1 = detail::Field<uint32_t, 0U, 24U>;
} // namespace mcc_biquad_t_a_1
/// Fields of MCC_BIQUAD_T_A_2.
namespace mcc_biquad_t_a_2 {
/// Bits 23:0, RW, signed: the manual's BIQUAD_T_A_2.
using BiquadTA2 = detail::Field<uint32_t, 0U, 24U>;
} // namespace mcc_biquad_t_a_2
/// Fields of MCC_BIQUAD_T_B_0.
namespace mcc_biquad_t_b_0 {
/// Bits 23:0, RW, signed: the manual's BIQUAD_T_B_0.
using BiquadTB0 = detail::Field<uint32_t, 0U, 24U>;
} // namespace mcc_biquad_t_b_0
/// Fields of MCC_BIQUAD_T_B_1.
namespace mcc_biquad_t_b_1 {
/// Bits 23:0, RW, signed: the manual's BIQUAD_T_B_1.
using BiquadTB1 = detail::Field<uint32_t, 0U, 24U>;
} // namespace mcc_biquad_t_b_1
/// Fields of MCC_BIQUAD_T_B_2.
namespace mcc_biquad_t_b_2 {
/// Bits 23:0, RW, signed: the manual's BIQUAD_T_B_2.
using BiquadTB2 = detail::Field<uint32_t, 0U, 24U>;
} // namespace mcc_biquad_t_b_2
/// Fields of MCC_BIQUAD_T_ENABLE.
namespace mcc_biquad_t_enable {
/// Bits 0:0, RW: the manual's BIQUAD_T_ENABLE.
using BiquadTEnable = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_biquad_t_enable
/// Fields of MCC_VELOCITY_CONFIG.
namespace mcc_velocity_config {
/// Bits 14:12, RW: the manual's MOVING_AVRG_FILTER_SAMPLES.
using MovingAvrgFilterSamples = detail::Field<uint32_t, 12U, 3U>;
/// Bits 10:9, RW: the manual's METER_TYPE.
using MeterType = detail::Field<uint32_t, 9U, 2U>;
/// Bits 8:8, RW: the manual's METER_SYNC_PULSE.
using MeterSyncPulse = detail::Field<uint32_t, 8U, 1U>;
/// Bits 7:0, RW: the manual's SELECTION.
using Selection = detail::Field<uint32_t, 0U, 8U>;
} // namespace mcc_velocity_config
/// Fields of MCC_VELOCITY_SCALING.
namespace mcc_velocity_scaling {
/// Bits 15:0, RW, signed: the manual's VELOCITY_SCALING.
using VelocityScaling = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_velocity_scaling
/// Fields of MCC_V_MIN_POS_DEV_TIME_COUNTER_LIMIT.
namespace mcc_v_min_pos_dev_time_counter_limit {
/// Bits 30:16, RW, unsigned: the manual's V_MIN_POS_DEV.
using VMinPosDev = detail::Field<uint32_t, 16U, 15U>;
/// Bits 15:0, RW, unsigned: the manual's TIME_COUNTER_LIMIT.
using TimeCounterLimit = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_v_min_pos_dev_time_counter_limit
/// Fields of MCC_MAX_VEL_DEVIATION.
namespace mcc_max_vel_deviation {
/// Bits 30:0, RW, unsigned: the manual's MAX_VEL_DEVIATION.
using MaxVelDeviation = detail::Field<uint32_t, 0U, 31U>;
} // namespace mcc_max_vel_deviation
/// Fields of MCC_POSITION_CONFIG.
namespace mcc_position_config {
/// Bits 7:0, RW: the manual's SELECTION.
using Selection = detail::Field<uint32_t, 0U, 8U>;
} // namespace mcc_position_config
/// Fields of MCC_MAX_POS_DEVIATION.
namespace mcc_max_pos_deviation {
/// Bits 30:0, RW, unsigned: the manual's MAX_POS_DEVIATION.
using MaxPosDeviation = detail::Field<uint32_t, 0U, 31U>;
} // namespace mcc_max_pos_deviation
/// Fields of MCC_RAMPER_STATUS.
namespace mcc_ramper_status {
/// Bits 17:17, R: the manual's STALL_IN_POS_ERR.
using StallInPosErr = detail::Field<uint32_t, 17U, 1U>;
/// Bits 16:16, R: the manual's STALL_IN_VEL_ERR.
using StallInVelErr = detail::Field<uint32_t, 16U, 1U>;
/// Bits 15:15, RW, W1C: the manual's SECOND_MOVE.
using SecondMove = detail::Field<uint32_t, 15U, 1U>;
/// Bits 14:14, R: the manual's T_ZEROWAIT_ACTIVE.
using TZerowaitActive = detail::Field<uint32_t, 14U, 1U>;
/// Bits 13:13, R: the manual's V_ZERO.
using VZero = detail::Field<uint32_t, 13U, 1U>;
/// Bits 12:12, R: the manual's POSITION_REACHED.
using PositionReached = detail::Field<uint32_t, 12U, 1U>;
/// Bits 11:11, R: the manual's VELOCITY_REACHED.
using VelocityReached = detail::Field<uint32_t, 11U, 1U>;
/// Bits 10:10, RW, W1C: the manual's EVENT_POS_REACHED.
using EventPosReached = detail::Field<uint32_t, 10U, 1U>;
/// Bits 9:9, RW, W1C: the manual's EVENT_STOP_SG.
using EventStopSg = detail::Field<uint32_t, 9U, 1U>;
/// Bits 8:8, R: the manual's EVENT_STOP_H.
using EventStopH = detail::Field<uint32_t, 8U, 1U>;
/// Bits 7:7, R: the manual's EVENT_STOP_R.
using EventStopR = detail::Field<uint32_t, 7U, 1U>;
/// Bits 6:6, R: the manual's EVENT_STOP_L.
using EventStopL = detail::Field<uint32_t, 6U, 1U>;
/// Bits 5:5, RW, W1C: the manual's STATUS_LATCH_H.
using StatusLatchH = detail::Field<uint32_t, 5U, 1U>;
/// Bits 4:4, RW, W1C: the manual's STATUS_LATCH_R.
using StatusLatchR = detail::Field<uint32_t, 4U, 1U>;
/// Bits 3:3, RW, W1C: the manual's STATUS_LATCH_L.
using StatusLatchL = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, R: the manual's STATUS_STOP_H.
using StatusStopH = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, R: the manual's STATUS_STOP_R.
using StatusStopR = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, R: the manual's STATUS_STOP_L.
using StatusStopL = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_ramper_status
/// Fields of MCC_RAMPER_A1.
namespace mcc_ramper_a1 {
/// Bits 22:0, RW, unsigned: the manual's RAMPER_A1.
using RamperA1 = detail::Field<uint32_t, 0U, 23U>;
} // namespace mcc_ramper_a1
/// Fields of MCC_RAMPER_A2.
namespace mcc_ramper_a2 {
/// Bits 22:0, RW, unsigned: the manual's RAMPER_A2.
using RamperA2 = detail::Field<uint32_t, 0U, 23U>;
} // namespace mcc_ramper_a2
/// Fields of MCC_RAMPER_A_MAX.
namespace mcc_ramper_a_max {
/// Bits 22:0, RW, unsigned: the manual's RAMPER_A_MAX.
using RamperAMax = detail::Field<uint32_t, 0U, 23U>;
} // namespace mcc_ramper_a_max
/// Fields of MCC_RAMPER_D1.
namespace mcc_ramper_d1 {
/// Bits 22:0, RW, unsigned: the manual's RAMPER_D1.
using RamperD1 = detail::Field<uint32_t, 0U, 23U>;
} // namespace mcc_ramper_d1
/// Fields of MCC_RAMPER_D2.
namespace mcc_ramper_d2 {
/// Bits 22:0, RW, unsigned: the manual's RAMPER_D2.
using RamperD2 = detail::Field<uint32_t, 0U, 23U>;
} // namespace mcc_ramper_d2
/// Fields of MCC_RAMPER_D_MAX.
namespace mcc_ramper_d_max {
/// Bits 22:0, RW, unsigned: the manual's RAMPER_D_MAX.
using RamperDMax = detail::Field<uint32_t, 0U, 23U>;
} // namespace mcc_ramper_d_max
/// Fields of MCC_RAMPER_V_START.
namespace mcc_ramper_v_start {
/// Bits 22:0, RW, unsigned: the manual's RAMPER_V_START.
using RamperVStart = detail::Field<uint32_t, 0U, 23U>;
} // namespace mcc_ramper_v_start
/// Fields of MCC_RAMPER_V1.
namespace mcc_ramper_v1 {
/// Bits 26:0, RW, unsigned: the manual's RAMPER_V1.
using RamperV1 = detail::Field<uint32_t, 0U, 27U>;
} // namespace mcc_ramper_v1
/// Fields of MCC_RAMPER_V2.
namespace mcc_ramper_v2 {
/// Bits 26:0, RW, unsigned: the manual's RAMPER_V2.
using RamperV2 = detail::Field<uint32_t, 0U, 27U>;
} // namespace mcc_ramper_v2
/// Fields of MCC_RAMPER_V_STOP.
namespace mcc_ramper_v_stop {
/// Bits 22:0, RW, unsigned: the manual's RAMPER_V_STOP.
using RamperVStop = detail::Field<uint32_t, 0U, 23U>;
} // namespace mcc_ramper_v_stop
/// Fields of MCC_RAMPER_V_MAX.
namespace mcc_ramper_v_max {
/// Bits 26:0, RW, unsigned: the manual's RAMPER_V_MAX.
using RamperVMax = detail::Field<uint32_t, 0U, 27U>;
} // namespace mcc_ramper_v_max
/// Fields of MCC_RAMPER_V_TARGET.
namespace mcc_ramper_v_target {
/// Bits 27:0, RW, signed: the manual's RAMPER_V_TARGET.
using RamperVTarget = detail::Field<uint32_t, 0U, 28U>;
} // namespace mcc_ramper_v_target
/// Fields of MCC_RAMPER_SWITCH_MODE.
namespace mcc_ramper_switch_mode {
/// Bits 19:19, RW: the manual's VELOCITY_OVERWRITE.
using VelocityOverwrite = detail::Field<uint32_t, 19U, 1U>;
/// Bits 18:18, RW: the manual's STOP_ON_VEL_DEVIATION.
using StopOnVelDeviation = detail::Field<uint32_t, 18U, 1U>;
/// Bits 17:17, RW: the manual's STOP_ON_POS_DEVIATION.
using StopOnPosDeviation = detail::Field<uint32_t, 17U, 1U>;
/// Bits 16:16, RW: the manual's SW_HARD_STOP.
using SwHardStop = detail::Field<uint32_t, 16U, 1U>;
/// Bits 15:15, RW: the manual's SOFTSTOP_ENABLE.
using SoftstopEnable = detail::Field<uint32_t, 15U, 1U>;
/// Bits 14:14, RW: the manual's SG_STOP_ENABLE.
using SgStopEnable = detail::Field<uint32_t, 14U, 1U>;
/// Bits 12:12, RW: the manual's LATCH_H_INACTIVE.
using LatchHInactive = detail::Field<uint32_t, 12U, 1U>;
/// Bits 11:11, RW: the manual's LATCH_H_ACTIVE.
using LatchHActive = detail::Field<uint32_t, 11U, 1U>;
/// Bits 10:10, RW: the manual's LATCH_R_INACTIVE.
using LatchRInactive = detail::Field<uint32_t, 10U, 1U>;
/// Bits 9:9, RW: the manual's LATCH_R_ACTIVE.
using LatchRActive = detail::Field<uint32_t, 9U, 1U>;
/// Bits 8:8, RW: the manual's LATCH_L_INACTIVE.
using LatchLInactive = detail::Field<uint32_t, 8U, 1U>;
/// Bits 7:7, RW: the manual's LATCH_L_ACTIVE.
using LatchLActive = detail::Field<uint32_t, 7U, 1U>;
/// Bits 6:6, RW: the manual's SWAP_LR.
using SwapLr = detail::Field<uint32_t, 6U, 1U>;
/// Bits 5:5, RW: the manual's STOP_H_POL.
using StopHPol = detail::Field<uint32_t, 5U, 1U>;
/// Bits 4:4, RW: the manual's STOP_R_POL.
using StopRPol = detail::Field<uint32_t, 4U, 1U>;
/// Bits 3:3, RW: the manual's STOP_L_POL.
using StopLPol = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, RW: the manual's STOP_H_ENABLE.
using StopHEnable = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, RW: the manual's STOP_R_ENABLE.
using StopREnable = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, RW: the manual's STOP_L_ENABLE.
using StopLEnable = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_ramper_switch_mode
/// Fields of MCC_RAMPER_TIME_CONFIG.
namespace mcc_ramper_time_config {
/// Bits 31:16, RW, unsigned: the manual's T_VMAX.
using TVmax = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, unsigned: the manual's T_ZEROWAIT.
using TZerowait = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_ramper_time_config
/// Fields of MCC_RAMPER_A_ACTUAL.
namespace mcc_ramper_a_actual {
/// Bits 23:0, R, signed: the manual's RAMPER_A_ACTUAL.
using RamperAActual = detail::Field<uint32_t, 0U, 24U>;
} // namespace mcc_ramper_a_actual
/// Fields of MCC_RAMPER_X_ACTUAL.
namespace mcc_ramper_x_actual {}
/// Fields of MCC_RAMPER_V_ACTUAL.
namespace mcc_ramper_v_actual {
/// Bits 27:0, R, signed: the manual's RAMPER_V_ACTUAL.
using RamperVActual = detail::Field<uint32_t, 0U, 28U>;
} // namespace mcc_ramper_v_actual
/// Fields of MCC_RAMPER_X_TARGET.
namespace mcc_ramper_x_target {}
/// Fields of MCC_RAMPER_PHI_E.
namespace mcc_ramper_phi_e {
/// Bits 15:0, R, signed: the manual's RAMPER_PHI_E.
using RamperPhiE = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_ramper_phi_e
/// Fields of MCC_RAMPER_PHI_E_OFFSET.
namespace mcc_ramper_phi_e_offset {
/// Bits 15:0, RW, signed: the manual's RAMPER_PHI_E_OFFSET.
using RamperPhiEOffset = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_ramper_phi_e_offset
/// Fields of MCC_RAMPER_ACC_FF.
namespace mcc_ramper_acc_ff {
/// Bits 18:16, RW: the manual's SHIFT.
using Shift = detail::Field<uint32_t, 16U, 3U>;
/// Bits 15:0, RW, unsigned: the manual's GAIN.
using Gain = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_ramper_acc_ff
/// Fields of MCC_RAMPER_X_ACTUAL_LATCH.
namespace mcc_ramper_x_actual_latch {}
/// Fields of MCC_POSITION_ACTUAL_LATCH.
namespace mcc_position_actual_latch {}
/// Fields of MCC_PRBS_AMPLITUDE.
namespace mcc_prbs_amplitude {}
/// Fields of MCC_PRBS_DOWN_SAMPLING_RATIO.
namespace mcc_prbs_down_sampling_ratio {
/// Bits 7:0, RW, unsigned: the manual's PRBS_DOWN_SAMPLING_RATIO.
using PrbsDownSamplingRatio = detail::Field<uint32_t, 0U, 8U>;
} // namespace mcc_prbs_down_sampling_ratio
/// Fields of MCC_PID_CONFIG.
namespace mcc_pid_config {
/// Bits 30:24, RW, unsigned: the manual's VEL_SMPL.
using VelSmpl = detail::Field<uint32_t, 24U, 7U>;
/// Bits 22:16, RW, unsigned: the manual's POS_SMPL.
using PosSmpl = detail::Field<uint32_t, 16U, 7U>;
/// Bits 15:12, RW, unsigned: the manual's VEL_SCALE.
using VelScale = detail::Field<uint32_t, 12U, 4U>;
/// Bits 11:10, RW: the manual's POSITION_NORM_I.
using PositionNormI = detail::Field<uint32_t, 10U, 2U>;
/// Bits 9:8, RW: the manual's POSITION_NORM_P.
using PositionNormP = detail::Field<uint32_t, 8U, 2U>;
/// Bits 7:6, RW: the manual's VELOCITY_NORM_I.
using VelocityNormI = detail::Field<uint32_t, 6U, 2U>;
/// Bits 5:4, RW: the manual's VELOCITY_NORM_P.
using VelocityNormP = detail::Field<uint32_t, 4U, 2U>;
/// Bits 3:3, RW: the manual's CURRENT_NORM_I.
using CurrentNormI = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, RW: the manual's CURRENT_NORM_P.
using CurrentNormP = detail::Field<uint32_t, 2U, 1U>;
/// Bits 0:0, RW: the manual's KEEP_POS_TARGET.
using KeepPosTarget = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_pid_config
/// Fields of MCC_PID_FLUX_COEFF.
namespace mcc_pid_flux_coeff {
/// Bits 31:16, RW, signed: the manual's P.
using P = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's I.
using I = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pid_flux_coeff
/// Fields of MCC_PID_TORQUE_COEFF.
namespace mcc_pid_torque_coeff {
/// Bits 31:16, RW, signed: the manual's P.
using P = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's I.
using I = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pid_torque_coeff
/// Fields of MCC_PID_FIELDWEAK_COEFF.
namespace mcc_pid_fieldweak_coeff {
/// Bits 31:16, RW, signed: the manual's P.
using P = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's I.
using I = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pid_fieldweak_coeff
/// Fields of MCC_PID_U_S_MAX.
namespace mcc_pid_u_s_max {
/// Bits 15:0, RW, unsigned: the manual's U_S_MAX.
using USMax = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pid_u_s_max
/// Fields of MCC_PID_VELOCITY_COEFF.
namespace mcc_pid_velocity_coeff {
/// Bits 31:16, RW, signed: the manual's P.
using P = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's I.
using I = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pid_velocity_coeff
/// Fields of MCC_PID_POSITION_COEFF.
namespace mcc_pid_position_coeff {
/// Bits 31:16, RW, signed: the manual's P.
using P = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's I.
using I = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pid_position_coeff
/// Fields of MCC_PID_POSITION_TOLERANCE.
namespace mcc_pid_position_tolerance {
/// Bits 30:0, RW, unsigned: the manual's PID_POSITION_TOLERANCE.
using PidPositionTolerance = detail::Field<uint32_t, 0U, 31U>;
} // namespace mcc_pid_position_tolerance
/// Fields of MCC_PID_POSITION_TOLERANCE_DELAY.
namespace mcc_pid_position_tolerance_delay {
/// Bits 15:0, RW, unsigned: the manual's PID_POSITION_TOLERANCE_DELAY.
using PidPositionToleranceDelay = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pid_position_tolerance_delay
/// Fields of MCC_PID_UQ_UD_LIMITS.
namespace mcc_pid_uq_ud_limits {
/// Bits 15:0, RW, unsigned: the manual's PID_UQ_UD_LIMITS.
using PidUqUdLimits = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pid_uq_ud_limits
/// Fields of MCC_PID_TORQUE_FLUX_LIMITS.
namespace mcc_pid_torque_flux_limits {
/// Bits 30:16, RW, unsigned: the manual's PID_TORQUE_LIMIT.
using PidTorqueLimit = detail::Field<uint32_t, 16U, 15U>;
/// Bits 14:0, RW, unsigned: the manual's PID_FLUX_LIMIT.
using PidFluxLimit = detail::Field<uint32_t, 0U, 15U>;
} // namespace mcc_pid_torque_flux_limits
/// Fields of MCC_PID_VELOCITY_LIMIT.
namespace mcc_pid_velocity_limit {
/// Bits 30:0, RW, unsigned: the manual's PID_VELOCITY_LIMIT.
using PidVelocityLimit = detail::Field<uint32_t, 0U, 31U>;
} // namespace mcc_pid_velocity_limit
/// Fields of MCC_PID_POSITION_LIMIT_LOW.
namespace mcc_pid_position_limit_low {}
/// Fields of MCC_PID_POSITION_LIMIT_HIGH.
namespace mcc_pid_position_limit_high {}
/// Fields of MCC_PID_TORQUE_FLUX_TARGET.
namespace mcc_pid_torque_flux_target {
/// Bits 31:16, RW, signed: the manual's PID_TORQUE_TARGET.
using PidTorqueTarget = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's PID_FLUX_TARGET.
using PidFluxTarget = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pid_torque_flux_target
/// Fields of MCC_PID_TORQUE_FLUX_OFFSET.
namespace mcc_pid_torque_flux_offset {
/// Bits 31:16, RW, signed: the manual's PID_TORQUE_OFFSET.
using PidTorqueOffset = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's PID_FLUX_OFFSET.
using PidFluxOffset = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pid_torque_flux_offset
/// Fields of MCC_PID_VELOCITY_TARGET.
namespace mcc_pid_velocity_target {}
/// Fields of MCC_PID_VELOCITY_OFFSET.
namespace mcc_pid_velocity_offset {}
/// Fields of MCC_PID_POSITION_TARGET.
namespace mcc_pid_position_target {}
/// Fields of MCC_PID_TORQUE_FLUX_ACTUAL.
namespace mcc_pid_torque_flux_actual {
/// Bits 31:16, R, signed: the manual's PID_TORQUE_ACTUAL.
using PidTorqueActual = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's PID_FLUX_ACTUAL.
using PidFluxActual = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pid_torque_flux_actual
/// Fields of MCC_PID_VELOCITY_ACTUAL.
namespace mcc_pid_velocity_actual {}
/// Fields of MCC_PID_POSITION_ACTUAL.
namespace mcc_pid_position_actual {}
/// Fields of MCC_PID_POSITION_ACTUAL_OFFSET.
namespace mcc_pid_position_actual_offset {}
/// Fields of MCC_PID_TORQUE_ERROR.
namespace mcc_pid_torque_error {
/// Bits 15:0, R, signed: the manual's PID_TORQUE_ERROR.
using PidTorqueError = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pid_torque_error
/// Fields of MCC_PID_FLUX_ERROR.
namespace mcc_pid_flux_error {
/// Bits 15:0, R, signed: the manual's PID_FLUX_ERROR.
using PidFluxError = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pid_flux_error
/// Fields of MCC_PID_VELOCITY_ERROR.
namespace mcc_pid_velocity_error {}
/// Fields of MCC_PID_POSITION_ERROR.
namespace mcc_pid_position_error {}
/// Fields of MCC_PID_TORQUE_INTEGRATOR.
namespace mcc_pid_torque_integrator {}
/// Fields of MCC_PID_FLUX_INTEGRATOR.
namespace mcc_pid_flux_integrator {}
/// Fields of MCC_PID_VELOCITY_INTEGRATOR.
namespace mcc_pid_velocity_integrator {}
/// Fields of MCC_PID_POSITION_INTEGRATOR.
namespace mcc_pid_position_integrator {}
/// Fields of MCC_PIDIN_TORQUE_FLUX_TARGET.
namespace mcc_pidin_torque_flux_target {
/// Bits 31:16, R, signed: the manual's PIDIN_TORQUE_TARGET.
using PidinTorqueTarget = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's PIDIN_FLUX_TARGET.
using PidinFluxTarget = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pidin_torque_flux_target
/// Fields of MCC_PIDIN_VELOCITY_TARGET.
namespace mcc_pidin_velocity_target {}
/// Fields of MCC_PIDIN_POSITION_TARGET.
namespace mcc_pidin_position_target {}
/// Fields of MCC_PIDIN_TORQUE_FLUX_TARGET_LIMITED.
namespace mcc_pidin_torque_flux_target_limited {
/// Bits 31:16, R, signed: the manual's PIDIN_TORQUE_TARGET_LIMITED.
using PidinTorqueTargetLimited = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's PIDIN_FLUX_TARGET_LIMITED.
using PidinFluxTargetLimited = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pidin_torque_flux_target_limited
/// Fields of MCC_PIDIN_VELOCITY_TARGET_LIMITED.
namespace mcc_pidin_velocity_target_limited {}
/// Fields of MCC_PIDIN_POSITION_TARGET_LIMITED.
namespace mcc_pidin_position_target_limited {}
/// Fields of MCC_FOC_IBETA_IALPHA.
namespace mcc_foc_ibeta_ialpha {
/// Bits 31:16, R, signed: the manual's IBETA.
using Ibeta = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's IALPHA.
using Ialpha = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_foc_ibeta_ialpha
/// Fields of MCC_FOC_IQ_ID.
namespace mcc_foc_iq_id {
/// Bits 31:16, R, signed: the manual's IQ.
using Iq = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's ID.
using Id = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_foc_iq_id
/// Fields of MCC_FOC_UQ_UD.
namespace mcc_foc_uq_ud {
/// Bits 31:16, R, signed: the manual's UQ.
using Uq = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's UD.
using Ud = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_foc_uq_ud
/// Fields of MCC_FOC_UQ_UD_LIMITED.
namespace mcc_foc_uq_ud_limited {
/// Bits 31:16, R, signed: the manual's UQ.
using Uq = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's UD.
using Ud = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_foc_uq_ud_limited
/// Fields of MCC_FOC_UBETA_UALPHA.
namespace mcc_foc_ubeta_ualpha {
/// Bits 31:16, R, signed: the manual's UBETA.
using Ubeta = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's UALPHA.
using Ualpha = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_foc_ubeta_ualpha
/// Fields of MCC_FOC_UWY_UUX.
namespace mcc_foc_uwy_uux {
/// Bits 31:16, R, signed: the manual's UWY.
using Uwy = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, signed: the manual's UUX.
using Uux = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_foc_uwy_uux
/// Fields of MCC_FOC_UV.
namespace mcc_foc_uv {
/// Bits 15:0, R, signed: the manual's UV.
using Uv = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_foc_uv
/// Fields of MCC_PWM_VX2_UX1.
namespace mcc_pwm_vx2_ux1 {
/// Bits 31:16, R, unsigned: the manual's VX2.
using Vx2 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, unsigned: the manual's UX1.
using Ux1 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pwm_vx2_ux1
/// Fields of MCC_PWM_Y2_WY1.
namespace mcc_pwm_y2_wy1 {
/// Bits 31:16, R, unsigned: the manual's Y2.
using Y2 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, unsigned: the manual's WY1.
using Wy1 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pwm_y2_wy1
/// Fields of MCC_VELOCITY_FRQ.
namespace mcc_velocity_frq {}
/// Fields of MCC_VELOCITY_PER.
namespace mcc_velocity_per {}
/// Fields of MCC_U_S_ACTUAL_I_S_ACTUAL.
namespace mcc_u_s_actual_i_s_actual {
/// Bits 31:16, R, unsigned: the manual's U_S_ACTUAL.
using USActual = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, R, unsigned: the manual's I_S_ACTUAL.
using ISActual = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_u_s_actual_i_s_actual
/// Fields of MCC_P_MOTOR.
namespace mcc_p_motor {}
/// Fields of MCC_INPUTS_RAW.
namespace mcc_inputs_raw {
/// Bits 22:22, R: the manual's HALL_W_FILT.
using HallWFilt = detail::Field<uint32_t, 22U, 1U>;
/// Bits 21:21, R: the manual's HALL_V_FILT.
using HallVFilt = detail::Field<uint32_t, 21U, 1U>;
/// Bits 20:20, R: the manual's HALL_U_FILT.
using HallUFilt = detail::Field<uint32_t, 20U, 1U>;
/// Bits 15:15, R: the manual's ENI.
using Eni = detail::Field<uint32_t, 15U, 1U>;
/// Bits 14:14, R: the manual's REF_SW_H.
using RefSwH = detail::Field<uint32_t, 14U, 1U>;
/// Bits 13:13, R: the manual's REF_SW_L.
using RefSwL = detail::Field<uint32_t, 13U, 1U>;
/// Bits 12:12, R: the manual's REF_SW_R.
using RefSwR = detail::Field<uint32_t, 12U, 1U>;
/// Bits 10:10, R: the manual's HALL_W.
using HallW = detail::Field<uint32_t, 10U, 1U>;
/// Bits 9:9, R: the manual's HALL_V.
using HallV = detail::Field<uint32_t, 9U, 1U>;
/// Bits 8:8, R: the manual's HALL_U.
using HallU = detail::Field<uint32_t, 8U, 1U>;
/// Bits 2:2, R: the manual's ENC_N.
using EncN = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, R: the manual's ENC_B.
using EncB = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, R: the manual's ENC_A.
using EncA = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_inputs_raw
/// Fields of MCC_OUTPUTS_RAW.
namespace mcc_outputs_raw {
/// Bits 7:7, R: the manual's PWM_Y2_H.
using PwmY2H = detail::Field<uint32_t, 7U, 1U>;
/// Bits 6:6, R: the manual's PWM_Y2_L.
using PwmY2L = detail::Field<uint32_t, 6U, 1U>;
/// Bits 5:5, R: the manual's PWM_WY1_H.
using PwmWy1H = detail::Field<uint32_t, 5U, 1U>;
/// Bits 4:4, R: the manual's PWM_WY1_L.
using PwmWy1L = detail::Field<uint32_t, 4U, 1U>;
/// Bits 3:3, R: the manual's PWM_VX2_H.
using PwmVx2H = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, R: the manual's PWM_VX2_L.
using PwmVx2L = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, R: the manual's PWM_UX1_H.
using PwmUx1H = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, R: the manual's PWM_UX1_L.
using PwmUx1L = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_outputs_raw
/// Fields of MCC_STATUS_FLAGS.
namespace mcc_status_flags {
/// Bits 31:31, RW, W1C: the manual's ENI.
using Eni = detail::Field<uint32_t, 31U, 1U>;
/// Bits 28:28, RW, W1C: the manual's ENC_N.
using EncN = detail::Field<uint32_t, 28U, 1U>;
/// Bits 26:26, RW, W1C: the manual's ADC_I_CLIPPED.
using AdcIClipped = detail::Field<uint32_t, 26U, 1U>;
/// Bits 23:23, RW, W1C: the manual's POSITION_REACHED.
using PositionReached = detail::Field<uint32_t, 23U, 1U>;
/// Bits 22:22, RW, W1C: the manual's REF_SW_H.
using RefSwH = detail::Field<uint32_t, 22U, 1U>;
/// Bits 21:21, RW, W1C: the manual's REF_SW_R.
using RefSwR = detail::Field<uint32_t, 21U, 1U>;
/// Bits 20:20, RW, W1C: the manual's REF_SW_L.
using RefSwL = detail::Field<uint32_t, 20U, 1U>;
/// Bits 17:17, RW, W1C: the manual's SHORT.
using Short = detail::Field<uint32_t, 17U, 1U>;
/// Bits 14:14, RW, W1C: the manual's PID_FW_OUTPUT_LIMIT.
using PidFwOutputLimit = detail::Field<uint32_t, 14U, 1U>;
/// Bits 13:13, RW, W1C: the manual's VELOCITY_TRACKING_ERROR.
using VelocityTrackingError = detail::Field<uint32_t, 13U, 1U>;
/// Bits 12:12, RW, W1C: the manual's POSITION_TRACKING_ERROR.
using PositionTrackingError = detail::Field<uint32_t, 12U, 1U>;
/// Bits 11:11, RW, W1C: the manual's HALL_ERROR.
using HallError = detail::Field<uint32_t, 11U, 1U>;
/// Bits 9:9, RW, W1C: the manual's PWM_SWITCH_LIMIT_ACTIVE.
using PwmSwitchLimitActive = detail::Field<uint32_t, 9U, 1U>;
/// Bits 8:8, RW, W1C: the manual's IPARK_VOLTLIM_LIMIT_U.
using IparkVoltlimLimitU = detail::Field<uint32_t, 8U, 1U>;
/// Bits 7:7, RW, W1C: the manual's PID_IQ_OUTPUT_LIMIT.
using PidIqOutputLimit = detail::Field<uint32_t, 7U, 1U>;
/// Bits 6:6, RW, W1C: the manual's PID_IQ_TARGET_LIMIT.
using PidIqTargetLimit = detail::Field<uint32_t, 6U, 1U>;
/// Bits 5:5, RW, W1C: the manual's PID_ID_OUTPUT_LIMIT.
using PidIdOutputLimit = detail::Field<uint32_t, 5U, 1U>;
/// Bits 4:4, RW, W1C: the manual's PID_ID_TARGET_LIMIT.
using PidIdTargetLimit = detail::Field<uint32_t, 4U, 1U>;
/// Bits 3:3, RW, W1C: the manual's PID_V_OUTPUT_LIMIT.
using PidVOutputLimit = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, RW, W1C: the manual's PID_V_TARGET_LIMIT.
using PidVTargetLimit = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, RW, W1C: the manual's PID_X_OUTPUT_LIMIT.
using PidXOutputLimit = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, RW, W1C: the manual's PID_X_TARGET_LIMIT.
using PidXTargetLimit = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_status_flags
/// Fields of MCC_GDRV_HW.
namespace mcc_gdrv_hw {
/// Bits 28:28, RW: the manual's HS_AS_LS_Y2.
using HsAsLsY2 = detail::Field<uint32_t, 28U, 1U>;
/// Bits 25:25, RW: the manual's BIAS_EN.
using BiasEn = detail::Field<uint32_t, 25U, 1U>;
/// Bits 24:24, RW: the manual's CHARGEPUMP_EN.
using ChargepumpEn = detail::Field<uint32_t, 24U, 1U>;
/// Bits 11:11, RW: the manual's BST_SW_CP_EN.
using BstSwCpEn = detail::Field<uint32_t, 11U, 1U>;
/// Bits 10:8, RW: the manual's BST_ILIM_MAX.
using BstIlimMax = detail::Field<uint32_t, 8U, 3U>;
/// Bits 7:7, RW: the manual's VS_UVLO_CMP_EN.
using VsUvloCmpEn = detail::Field<uint32_t, 7U, 1U>;
/// Bits 6:6, RW: the manual's VDRV_UVLO_CMP_EN.
using VdrvUvloCmpEn = detail::Field<uint32_t, 6U, 1U>;
/// Bits 5:5, RW: the manual's HS_OCP_CMP_EN.
using HsOcpCmpEn = detail::Field<uint32_t, 5U, 1U>;
/// Bits 4:4, RW: the manual's LS_OCP_CMP_EN.
using LsOcpCmpEn = detail::Field<uint32_t, 4U, 1U>;
/// Bits 3:3, RW: the manual's BRIDGE_ENABLE_Y2.
using BridgeEnableY2 = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, RW: the manual's BRIDGE_ENABLE_W.
using BridgeEnableW = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, RW: the manual's BRIDGE_ENABLE_V.
using BridgeEnableV = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, RW: the manual's BRIDGE_ENABLE_U.
using BridgeEnableU = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_gdrv_hw
/// Fields of MCC_GDRV_CFG.
namespace mcc_gdrv_cfg {
/// Bits 23:20, RW: the manual's VS_UVLO_LVL.
using VsUvloLvl = detail::Field<uint32_t, 20U, 4U>;
/// Bits 17:17, RW: the manual's ADAPTIVE_MODE_Y2.
using AdaptiveModeY2 = detail::Field<uint32_t, 17U, 1U>;
/// Bits 16:16, RW: the manual's ADAPTIVE_MODE_UVW.
using AdaptiveModeUvw = detail::Field<uint32_t, 16U, 1U>;
/// Bits 15:12, RW: the manual's IGATE_SOURCE_Y2.
using IgateSourceY2 = detail::Field<uint32_t, 12U, 4U>;
/// Bits 11:8, RW: the manual's IGATE_SINK_Y2.
using IgateSinkY2 = detail::Field<uint32_t, 8U, 4U>;
/// Bits 7:4, RW: the manual's IGATE_SOURCE_UVW.
using IgateSourceUvw = detail::Field<uint32_t, 4U, 4U>;
/// Bits 3:0, RW: the manual's IGATE_SINK_UVW.
using IgateSinkUvw = detail::Field<uint32_t, 0U, 4U>;
} // namespace mcc_gdrv_cfg
/// Fields of MCC_GDRV_TIMING.
namespace mcc_gdrv_timing {
/// Bits 31:24, RW, unsigned: the manual's T_DRIVE_SOURCE_Y2.
using TDriveSourceY2 = detail::Field<uint32_t, 24U, 8U>;
/// Bits 23:16, RW, unsigned: the manual's T_DRIVE_SINK_Y2.
using TDriveSinkY2 = detail::Field<uint32_t, 16U, 8U>;
/// Bits 15:8, RW, unsigned: the manual's T_DRIVE_SOURCE_UVW.
using TDriveSourceUvw = detail::Field<uint32_t, 8U, 8U>;
/// Bits 7:0, RW, unsigned: the manual's T_DRIVE_SINK_UVW.
using TDriveSinkUvw = detail::Field<uint32_t, 0U, 8U>;
} // namespace mcc_gdrv_timing
/// Fields of MCC_GDRV_BBM.
namespace mcc_gdrv_bbm {
/// Bits 31:24, RW, unsigned: the manual's BBM_H_Y2.
using BbmHY2 = detail::Field<uint32_t, 24U, 8U>;
/// Bits 23:16, RW, unsigned: the manual's BBM_L_Y2.
using BbmLY2 = detail::Field<uint32_t, 16U, 8U>;
/// Bits 15:8, RW, unsigned: the manual's BBM_H_UVW.
using BbmHUvw = detail::Field<uint32_t, 8U, 8U>;
/// Bits 7:0, RW, unsigned: the manual's BBM_L_UVW.
using BbmLUvw = detail::Field<uint32_t, 0U, 8U>;
} // namespace mcc_gdrv_bbm
/// Fields of MCC_GDRV_PROT.
namespace mcc_gdrv_prot {
/// Bits 28:28, RW: the manual's TERM_PWM_ON_SHORT.
using TermPwmOnShort = detail::Field<uint32_t, 28U, 1U>;
/// Bits 23:22, RW: the manual's HS_RETRIES_Y2.
using HsRetriesY2 = detail::Field<uint32_t, 22U, 2U>;
/// Bits 21:20, RW: the manual's LS_RETRIES_Y2.
using LsRetriesY2 = detail::Field<uint32_t, 20U, 2U>;
/// Bits 19:18, RW: the manual's HS_RETRIES_UVW.
using HsRetriesUvw = detail::Field<uint32_t, 18U, 2U>;
/// Bits 17:16, RW: the manual's LS_RETRIES_UVW.
using LsRetriesUvw = detail::Field<uint32_t, 16U, 2U>;
/// Bits 13:12, RW: the manual's VGS_BLANKING_Y2.
using VgsBlankingY2 = detail::Field<uint32_t, 12U, 2U>;
/// Bits 10:8, RW: the manual's VGS_DEGLITCH_Y2.
using VgsDeglitchY2 = detail::Field<uint32_t, 8U, 3U>;
/// Bits 5:4, RW: the manual's VGS_BLANKING_UVW.
using VgsBlankingUvw = detail::Field<uint32_t, 4U, 2U>;
/// Bits 2:0, RW: the manual's VGS_DEGLITCH_UVW.
using VgsDeglitchUvw = detail::Field<uint32_t, 0U, 3U>;
} // namespace mcc_gdrv_prot
/// Fields of MCC_GDRV_OCP_UVW.
namespace mcc_gdrv_ocp_uvw {
/// Bits 27:24, RW: the manual's HS_OCP_THRES_UVW.
using HsOcpThresUvw = detail::Field<uint32_t, 24U, 4U>;
/// Bits 22:20, RW: the manual's HS_OCP_BLANKING_UVW.
using HsOcpBlankingUvw = detail::Field<uint32_t, 20U, 3U>;
/// Bits 18:16, RW: the manual's HS_OCP_DEGLITCH_UVW.
using HsOcpDeglitchUvw = detail::Field<uint32_t, 16U, 3U>;
/// Bits 15:15, RW: the manual's LS_OCP_USE_VDS_UVW.
using LsOcpUseVdsUvw = detail::Field<uint32_t, 15U, 1U>;
/// Bits 11:8, RW: the manual's LS_OCP_THRES_UVW.
using LsOcpThresUvw = detail::Field<uint32_t, 8U, 4U>;
/// Bits 6:4, RW: the manual's LS_OCP_BLANKING_UVW.
using LsOcpBlankingUvw = detail::Field<uint32_t, 4U, 3U>;
/// Bits 2:0, RW: the manual's LS_OCP_DEGLITCH_UVW.
using LsOcpDeglitchUvw = detail::Field<uint32_t, 0U, 3U>;
} // namespace mcc_gdrv_ocp_uvw
/// Fields of MCC_GDRV_OCP_Y2.
namespace mcc_gdrv_ocp_y2 {
/// Bits 27:24, RW: the manual's HS_OCP_THRES_Y2.
using HsOcpThresY2 = detail::Field<uint32_t, 24U, 4U>;
/// Bits 22:20, RW: the manual's HS_OCP_BLANKING_Y2.
using HsOcpBlankingY2 = detail::Field<uint32_t, 20U, 3U>;
/// Bits 18:16, RW: the manual's HS_OCP_DEGLITCH_Y2.
using HsOcpDeglitchY2 = detail::Field<uint32_t, 16U, 3U>;
/// Bits 15:15, RW: the manual's LS_OCP_USE_VDS_Y2.
using LsOcpUseVdsY2 = detail::Field<uint32_t, 15U, 1U>;
/// Bits 11:8, RW: the manual's LS_OCP_THRES_Y2.
using LsOcpThresY2 = detail::Field<uint32_t, 8U, 4U>;
/// Bits 6:4, RW: the manual's LS_OCP_BLANKING_Y2.
using LsOcpBlankingY2 = detail::Field<uint32_t, 4U, 3U>;
/// Bits 2:0, RW: the manual's LS_OCP_DEGLITCH_Y2.
using LsOcpDeglitchY2 = detail::Field<uint32_t, 0U, 3U>;
} // namespace mcc_gdrv_ocp_y2
/// Fields of MCC_GDRV_PROT_EN.
namespace mcc_gdrv_prot_en {
/// Bits 31:31, RW: the manual's VS_UVLO_PROT.
using VsUvloProt = detail::Field<uint32_t, 31U, 1U>;
/// Bits 29:29, RW: the manual's VDRV_UVLO_PROT.
using VdrvUvloProt = detail::Field<uint32_t, 29U, 1U>;
/// Bits 27:27, RW: the manual's HS_VGS_ON_SHORT_PROT_Y2.
using HsVgsOnShortProtY2 = detail::Field<uint32_t, 27U, 1U>;
/// Bits 26:26, RW: the manual's HS_VGS_ON_SHORT_PROT_W.
using HsVgsOnShortProtW = detail::Field<uint32_t, 26U, 1U>;
/// Bits 25:25, RW: the manual's HS_VGS_ON_SHORT_PROT_V.
using HsVgsOnShortProtV = detail::Field<uint32_t, 25U, 1U>;
/// Bits 24:24, RW: the manual's HS_VGS_ON_SHORT_PROT_U.
using HsVgsOnShortProtU = detail::Field<uint32_t, 24U, 1U>;
/// Bits 23:23, RW: the manual's HS_VGS_OFF_SHORT_PROT_Y2.
using HsVgsOffShortProtY2 = detail::Field<uint32_t, 23U, 1U>;
/// Bits 22:22, RW: the manual's HS_VGS_OFF_SHORT_PROT_W.
using HsVgsOffShortProtW = detail::Field<uint32_t, 22U, 1U>;
/// Bits 21:21, RW: the manual's HS_VGS_OFF_SHORT_PROT_V.
using HsVgsOffShortProtV = detail::Field<uint32_t, 21U, 1U>;
/// Bits 20:20, RW: the manual's HS_VGS_OFF_SHORT_PROT_U.
using HsVgsOffShortProtU = detail::Field<uint32_t, 20U, 1U>;
/// Bits 19:19, RW: the manual's HS_SHORT_PROT_Y2.
using HsShortProtY2 = detail::Field<uint32_t, 19U, 1U>;
/// Bits 18:18, RW: the manual's HS_SHORT_PROT_W.
using HsShortProtW = detail::Field<uint32_t, 18U, 1U>;
/// Bits 17:17, RW: the manual's HS_SHORT_PROT_V.
using HsShortProtV = detail::Field<uint32_t, 17U, 1U>;
/// Bits 16:16, RW: the manual's HS_SHORT_PROT_U.
using HsShortProtU = detail::Field<uint32_t, 16U, 1U>;
/// Bits 15:15, RW: the manual's BST_UVLO_PROT_Y2.
using BstUvloProtY2 = detail::Field<uint32_t, 15U, 1U>;
/// Bits 14:14, RW: the manual's BST_UVLO_PROT_W.
using BstUvloProtW = detail::Field<uint32_t, 14U, 1U>;
/// Bits 13:13, RW: the manual's BST_UVLO_PROT_V.
using BstUvloProtV = detail::Field<uint32_t, 13U, 1U>;
/// Bits 12:12, RW: the manual's BST_UVLO_PROT_U.
using BstUvloProtU = detail::Field<uint32_t, 12U, 1U>;
/// Bits 11:11, RW: the manual's LS_VGS_ON_SHORT_PROT_Y2.
using LsVgsOnShortProtY2 = detail::Field<uint32_t, 11U, 1U>;
/// Bits 10:10, RW: the manual's LS_VGS_ON_SHORT_PROT_W.
using LsVgsOnShortProtW = detail::Field<uint32_t, 10U, 1U>;
/// Bits 9:9, RW: the manual's LS_VGS_ON_SHORT_PROT_V.
using LsVgsOnShortProtV = detail::Field<uint32_t, 9U, 1U>;
/// Bits 8:8, RW: the manual's LS_VGS_ON_SHORT_PROT_U.
using LsVgsOnShortProtU = detail::Field<uint32_t, 8U, 1U>;
/// Bits 7:7, RW: the manual's LS_VGS_OFF_SHORT_PROT_Y2.
using LsVgsOffShortProtY2 = detail::Field<uint32_t, 7U, 1U>;
/// Bits 6:6, RW: the manual's LS_VGS_OFF_SHORT_PROT_W.
using LsVgsOffShortProtW = detail::Field<uint32_t, 6U, 1U>;
/// Bits 5:5, RW: the manual's LS_VGS_OFF_SHORT_PROT_V.
using LsVgsOffShortProtV = detail::Field<uint32_t, 5U, 1U>;
/// Bits 4:4, RW: the manual's LS_VGS_OFF_SHORT_PROT_U.
using LsVgsOffShortProtU = detail::Field<uint32_t, 4U, 1U>;
/// Bits 3:3, RW: the manual's LS_SHORT_PROT_Y2.
using LsShortProtY2 = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, RW: the manual's LS_SHORT_PROT_W.
using LsShortProtW = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, RW: the manual's LS_SHORT_PROT_V.
using LsShortProtV = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, RW: the manual's LS_SHORT_PROT_U.
using LsShortProtU = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_gdrv_prot_en
/// Fields of MCC_GDRV_STATUS_EN.
namespace mcc_gdrv_status_en {
/// Bits 31:31, RW: the manual's VS_UVLO_EN.
using VsUvloEn = detail::Field<uint32_t, 31U, 1U>;
/// Bits 30:30, RW: the manual's VDRV_UVLWRN_EN.
using VdrvUvlwrnEn = detail::Field<uint32_t, 30U, 1U>;
/// Bits 29:29, RW: the manual's VDRV_UVLO_EN.
using VdrvUvloEn = detail::Field<uint32_t, 29U, 1U>;
/// Bits 27:27, RW: the manual's HS_VGS_ON_SHORT_EN_Y2.
using HsVgsOnShortEnY2 = detail::Field<uint32_t, 27U, 1U>;
/// Bits 26:26, RW: the manual's HS_VGS_ON_SHORT_EN_W.
using HsVgsOnShortEnW = detail::Field<uint32_t, 26U, 1U>;
/// Bits 25:25, RW: the manual's HS_VGS_ON_SHORT_EN_V.
using HsVgsOnShortEnV = detail::Field<uint32_t, 25U, 1U>;
/// Bits 24:24, RW: the manual's HS_VGS_ON_SHORT_EN_U.
using HsVgsOnShortEnU = detail::Field<uint32_t, 24U, 1U>;
/// Bits 23:23, RW: the manual's HS_VGS_OFF_SHORT_EN_Y2.
using HsVgsOffShortEnY2 = detail::Field<uint32_t, 23U, 1U>;
/// Bits 22:22, RW: the manual's HS_VGS_OFF_SHORT_EN_W.
using HsVgsOffShortEnW = detail::Field<uint32_t, 22U, 1U>;
/// Bits 21:21, RW: the manual's HS_VGS_OFF_SHORT_EN_V.
using HsVgsOffShortEnV = detail::Field<uint32_t, 21U, 1U>;
/// Bits 20:20, RW: the manual's HS_VGS_OFF_SHORT_EN_U.
using HsVgsOffShortEnU = detail::Field<uint32_t, 20U, 1U>;
/// Bits 19:19, RW: the manual's HS_SHORT_EN_Y2.
using HsShortEnY2 = detail::Field<uint32_t, 19U, 1U>;
/// Bits 18:18, RW: the manual's HS_SHORT_EN_W.
using HsShortEnW = detail::Field<uint32_t, 18U, 1U>;
/// Bits 17:17, RW: the manual's HS_SHORT_EN_V.
using HsShortEnV = detail::Field<uint32_t, 17U, 1U>;
/// Bits 16:16, RW: the manual's HS_SHORT_EN_U.
using HsShortEnU = detail::Field<uint32_t, 16U, 1U>;
/// Bits 15:15, RW: the manual's BST_UVLO_EN_Y2.
using BstUvloEnY2 = detail::Field<uint32_t, 15U, 1U>;
/// Bits 14:14, RW: the manual's BST_UVLO_EN_W.
using BstUvloEnW = detail::Field<uint32_t, 14U, 1U>;
/// Bits 13:13, RW: the manual's BST_UVLO_EN_V.
using BstUvloEnV = detail::Field<uint32_t, 13U, 1U>;
/// Bits 12:12, RW: the manual's BST_UVLO_EN_U.
using BstUvloEnU = detail::Field<uint32_t, 12U, 1U>;
/// Bits 11:11, RW: the manual's LS_VGS_ON_SHORT_EN_Y2.
using LsVgsOnShortEnY2 = detail::Field<uint32_t, 11U, 1U>;
/// Bits 10:10, RW: the manual's LS_VGS_ON_SHORT_EN_W.
using LsVgsOnShortEnW = detail::Field<uint32_t, 10U, 1U>;
/// Bits 9:9, RW: the manual's LS_VGS_ON_SHORT_EN_V.
using LsVgsOnShortEnV = detail::Field<uint32_t, 9U, 1U>;
/// Bits 8:8, RW: the manual's LS_VGS_ON_SHORT_EN_U.
using LsVgsOnShortEnU = detail::Field<uint32_t, 8U, 1U>;
/// Bits 7:7, RW: the manual's LS_VGS_OFF_SHORT_EN_Y2.
using LsVgsOffShortEnY2 = detail::Field<uint32_t, 7U, 1U>;
/// Bits 6:6, RW: the manual's LS_VGS_OFF_SHORT_EN_W.
using LsVgsOffShortEnW = detail::Field<uint32_t, 6U, 1U>;
/// Bits 5:5, RW: the manual's LS_VGS_OFF_SHORT_EN_V.
using LsVgsOffShortEnV = detail::Field<uint32_t, 5U, 1U>;
/// Bits 4:4, RW: the manual's LS_VGS_OFF_SHORT_EN_U.
using LsVgsOffShortEnU = detail::Field<uint32_t, 4U, 1U>;
/// Bits 3:3, RW: the manual's LS_SHORT_EN_Y2.
using LsShortEnY2 = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, RW: the manual's LS_SHORT_EN_W.
using LsShortEnW = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, RW: the manual's LS_SHORT_EN_V.
using LsShortEnV = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, RW: the manual's LS_SHORT_EN_U.
using LsShortEnU = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_gdrv_status_en
/// Fields of MCC_GDRV_STATUS.
namespace mcc_gdrv_status {
/// Bits 31:31, RW, W1C: the manual's VS_UVLO.
using VsUvlo = detail::Field<uint32_t, 31U, 1U>;
/// Bits 30:30, RW, W1C: the manual's VDRV_UVLWRN.
using VdrvUvlwrn = detail::Field<uint32_t, 30U, 1U>;
/// Bits 29:29, RW, W1C: the manual's VDRV_UVLO.
using VdrvUvlo = detail::Field<uint32_t, 29U, 1U>;
/// Bits 27:27, RW, W1C: the manual's HS_VGS_ON_SHORT_Y2.
using HsVgsOnShortY2 = detail::Field<uint32_t, 27U, 1U>;
/// Bits 26:26, RW, W1C: the manual's HS_VGS_ON_SHORT_W.
using HsVgsOnShortW = detail::Field<uint32_t, 26U, 1U>;
/// Bits 25:25, RW, W1C: the manual's HS_VGS_ON_SHORT_V.
using HsVgsOnShortV = detail::Field<uint32_t, 25U, 1U>;
/// Bits 24:24, RW, W1C: the manual's HS_VGS_ON_SHORT_U.
using HsVgsOnShortU = detail::Field<uint32_t, 24U, 1U>;
/// Bits 23:23, RW, W1C: the manual's HS_VGS_OFF_SHORT_Y2.
using HsVgsOffShortY2 = detail::Field<uint32_t, 23U, 1U>;
/// Bits 22:22, RW, W1C: the manual's HS_VGS_OFF_SHORT_W.
using HsVgsOffShortW = detail::Field<uint32_t, 22U, 1U>;
/// Bits 21:21, RW, W1C: the manual's HS_VGS_OFF_SHORT_V.
using HsVgsOffShortV = detail::Field<uint32_t, 21U, 1U>;
/// Bits 20:20, RW, W1C: the manual's HS_VGS_OFF_SHORT_U.
using HsVgsOffShortU = detail::Field<uint32_t, 20U, 1U>;
/// Bits 19:19, RW, W1C: the manual's HS_SHORT_Y2.
using HsShortY2 = detail::Field<uint32_t, 19U, 1U>;
/// Bits 18:18, RW, W1C: the manual's HS_SHORT_W.
using HsShortW = detail::Field<uint32_t, 18U, 1U>;
/// Bits 17:17, RW, W1C: the manual's HS_SHORT_V.
using HsShortV = detail::Field<uint32_t, 17U, 1U>;
/// Bits 16:16, RW, W1C: the manual's HS_SHORT_U.
using HsShortU = detail::Field<uint32_t, 16U, 1U>;
/// Bits 15:15, RW, W1C: the manual's BST_UVLO_Y2.
using BstUvloY2 = detail::Field<uint32_t, 15U, 1U>;
/// Bits 14:14, RW, W1C: the manual's BST_UVLO_W.
using BstUvloW = detail::Field<uint32_t, 14U, 1U>;
/// Bits 13:13, RW, W1C: the manual's BST_UVLO_V.
using BstUvloV = detail::Field<uint32_t, 13U, 1U>;
/// Bits 12:12, RW, W1C: the manual's BST_UVLO_U.
using BstUvloU = detail::Field<uint32_t, 12U, 1U>;
/// Bits 11:11, RW, W1C: the manual's LS_VGS_ON_SHORT_Y2.
using LsVgsOnShortY2 = detail::Field<uint32_t, 11U, 1U>;
/// Bits 10:10, RW, W1C: the manual's LS_VGS_ON_SHORT_W.
using LsVgsOnShortW = detail::Field<uint32_t, 10U, 1U>;
/// Bits 9:9, RW, W1C: the manual's LS_VGS_ON_SHORT_V.
using LsVgsOnShortV = detail::Field<uint32_t, 9U, 1U>;
/// Bits 8:8, RW, W1C: the manual's LS_VGS_ON_SHORT_U.
using LsVgsOnShortU = detail::Field<uint32_t, 8U, 1U>;
/// Bits 7:7, RW, W1C: the manual's LS_VGS_OFF_SHORT_Y2.
using LsVgsOffShortY2 = detail::Field<uint32_t, 7U, 1U>;
/// Bits 6:6, RW, W1C: the manual's LS_VGS_OFF_SHORT_W.
using LsVgsOffShortW = detail::Field<uint32_t, 6U, 1U>;
/// Bits 5:5, RW, W1C: the manual's LS_VGS_OFF_SHORT_V.
using LsVgsOffShortV = detail::Field<uint32_t, 5U, 1U>;
/// Bits 4:4, RW, W1C: the manual's LS_VGS_OFF_SHORT_U.
using LsVgsOffShortU = detail::Field<uint32_t, 4U, 1U>;
/// Bits 3:3, RW, W1C: the manual's LS_SHORT_Y2.
using LsShortY2 = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, RW, W1C: the manual's LS_SHORT_W.
using LsShortW = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, RW, W1C: the manual's LS_SHORT_V.
using LsShortV = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, RW, W1C: the manual's LS_SHORT_U.
using LsShortU = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_gdrv_status
/// Fields of MCC_GDRV_FAULT.
namespace mcc_gdrv_fault {
/// Bits 31:31, R: the manual's VS_UVLO_STS.
using VsUvloSts = detail::Field<uint32_t, 31U, 1U>;
/// Bits 30:30, R: the manual's VDRV_UVLWRN_STS.
using VdrvUvlwrnSts = detail::Field<uint32_t, 30U, 1U>;
/// Bits 29:29, R: the manual's VDRV_UVLO_STS.
using VdrvUvloSts = detail::Field<uint32_t, 29U, 1U>;
/// Bits 19:19, RW, W1C: the manual's HS_FAULT_ACTIVE_Y2.
using HsFaultActiveY2 = detail::Field<uint32_t, 19U, 1U>;
/// Bits 18:18, RW, W1C: the manual's HS_FAULT_ACTIVE_W.
using HsFaultActiveW = detail::Field<uint32_t, 18U, 1U>;
/// Bits 17:17, RW, W1C: the manual's HS_FAULT_ACTIVE_V.
using HsFaultActiveV = detail::Field<uint32_t, 17U, 1U>;
/// Bits 16:16, RW, W1C: the manual's HS_FAULT_ACTIVE_U.
using HsFaultActiveU = detail::Field<uint32_t, 16U, 1U>;
/// Bits 15:15, R: the manual's BST_UVLO_STS_Y2.
using BstUvloStsY2 = detail::Field<uint32_t, 15U, 1U>;
/// Bits 14:14, R: the manual's BST_UVLO_STS_W.
using BstUvloStsW = detail::Field<uint32_t, 14U, 1U>;
/// Bits 13:13, R: the manual's BST_UVLO_STS_V.
using BstUvloStsV = detail::Field<uint32_t, 13U, 1U>;
/// Bits 12:12, R: the manual's BST_UVLO_STS_U.
using BstUvloStsU = detail::Field<uint32_t, 12U, 1U>;
/// Bits 3:3, RW, W1C: the manual's LS_FAULT_ACTIVE_Y2.
using LsFaultActiveY2 = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, RW, W1C: the manual's LS_FAULT_ACTIVE_W.
using LsFaultActiveW = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, RW, W1C: the manual's LS_FAULT_ACTIVE_V.
using LsFaultActiveV = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, RW, W1C: the manual's LS_FAULT_ACTIVE_U.
using LsFaultActiveU = detail::Field<uint32_t, 0U, 1U>;
} // namespace mcc_gdrv_fault
/// Fields of MCC_ADC_I1_I0_EXT.
namespace mcc_adc_i1_i0_ext {
/// Bits 31:16, RW, signed: the manual's I1.
using I1 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's I0.
using I0 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_i1_i0_ext
/// Fields of MCC_ADC_I2_EXT.
namespace mcc_adc_i2_ext {
/// Bits 15:0, RW, signed: the manual's I2.
using I2 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_adc_i2_ext
/// Fields of MCC_PWM_VX2_UX1_EXT.
namespace mcc_pwm_vx2_ux1_ext {
/// Bits 31:16, RW, unsigned: the manual's VX2.
using Vx2 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, unsigned: the manual's UX1.
using Ux1 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pwm_vx2_ux1_ext
/// Fields of MCC_PWM_Y2_WY1_EXT.
namespace mcc_pwm_y2_wy1_ext {
/// Bits 31:16, RW, unsigned: the manual's Y2.
using Y2 = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, unsigned: the manual's WY1.
using Wy1 = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pwm_y2_wy1_ext
/// Fields of MCC_PWM_EXT_Y2_ALT.
namespace mcc_pwm_ext_y2_alt {
/// Bits 15:0, RW, unsigned: the manual's PWM_EXT_Y2_ALT.
using PwmExtY2Alt = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_pwm_ext_y2_alt
/// Fields of MCC_VOLTAGE_EXT.
namespace mcc_voltage_ext {
/// Bits 31:16, RW, signed: the manual's UQ.
using Uq = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's UD.
using Ud = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_voltage_ext
/// Fields of MCC_PHI_EXT.
namespace mcc_phi_ext {
/// Bits 31:16, RW, signed: the manual's PHI_M_EXT.
using PhiMExt = detail::Field<uint32_t, 16U, 16U>;
/// Bits 15:0, RW, signed: the manual's PHI_E_EXT.
using PhiEExt = detail::Field<uint32_t, 0U, 16U>;
} // namespace mcc_phi_ext
/// Fields of MCC_VELOCITY_EXT.
namespace mcc_velocity_ext {}
/// Fields of ADC_SRC_CONFIG.
namespace adc_src_config {
/// Bits 31:31, RW: the manual's ADC3_MUX2_DETOUR.
using Adc3Mux2Detour = detail::Field<uint32_t, 31U, 1U>;
/// Bits 29:28, RW: the manual's ADC3_MUX2_CFG.
using Adc3Mux2Cfg = detail::Field<uint32_t, 28U, 2U>;
/// Bits 27:26, RW: the manual's ADC3_MUX1_CFG.
using Adc3Mux1Cfg = detail::Field<uint32_t, 26U, 2U>;
/// Bits 25:24, RW: the manual's ADC3_MUX0_CFG.
using Adc3Mux0Cfg = detail::Field<uint32_t, 24U, 2U>;
/// Bits 23:23, RW: the manual's ADC2_MUX2_DETOUR.
using Adc2Mux2Detour = detail::Field<uint32_t, 23U, 1U>;
/// Bits 22:22, RW: the manual's ADC2_MUX3_DIS.
using Adc2Mux3Dis = detail::Field<uint32_t, 22U, 1U>;
/// Bits 21:20, RW: the manual's ADC2_MUX2_CFG.
using Adc2Mux2Cfg = detail::Field<uint32_t, 20U, 2U>;
/// Bits 19:18, RW: the manual's ADC2_MUX1_CFG.
using Adc2Mux1Cfg = detail::Field<uint32_t, 18U, 2U>;
/// Bits 17:16, RW: the manual's ADC2_MUX0_CFG.
using Adc2Mux0Cfg = detail::Field<uint32_t, 16U, 2U>;
/// Bits 15:15, RW: the manual's ADC1_MUX2_DETOUR.
using Adc1Mux2Detour = detail::Field<uint32_t, 15U, 1U>;
/// Bits 13:12, RW: the manual's ADC1_MUX2_CFG.
using Adc1Mux2Cfg = detail::Field<uint32_t, 12U, 2U>;
/// Bits 11:10, RW: the manual's ADC1_MUX1_CFG.
using Adc1Mux1Cfg = detail::Field<uint32_t, 10U, 2U>;
/// Bits 9:8, RW: the manual's ADC1_MUX0_CFG.
using Adc1Mux0Cfg = detail::Field<uint32_t, 8U, 2U>;
/// Bits 7:7, RW: the manual's ADC0_MUX2_DETOUR.
using Adc0Mux2Detour = detail::Field<uint32_t, 7U, 1U>;
/// Bits 6:6, RW: the manual's ADC0_MUX3_DIS.
using Adc0Mux3Dis = detail::Field<uint32_t, 6U, 1U>;
/// Bits 5:4, RW: the manual's ADC0_MUX2_CFG.
using Adc0Mux2Cfg = detail::Field<uint32_t, 4U, 2U>;
/// Bits 3:2, RW: the manual's ADC0_MUX1_CFG.
using Adc0Mux1Cfg = detail::Field<uint32_t, 2U, 2U>;
/// Bits 1:0, RW: the manual's ADC0_MUX0_CFG.
using Adc0Mux0Cfg = detail::Field<uint32_t, 0U, 2U>;
} // namespace adc_src_config
/// Fields of ADC_SETUP.
namespace adc_setup {
/// Bits 19:16, RW: the manual's ADC_SHIFT_SAMPLE.
using AdcShiftSample = detail::Field<uint32_t, 16U, 4U>;
} // namespace adc_setup
/// Fields of ADC_STATUS.
namespace adc_status {
/// Bits 15:15, R: the manual's ADC3_MUXSEQ_FAIL.
using Adc3MuxseqFail = detail::Field<uint32_t, 15U, 1U>;
/// Bits 14:14, R: the manual's ADC2_MUXSEQ_FAIL.
using Adc2MuxseqFail = detail::Field<uint32_t, 14U, 1U>;
/// Bits 13:13, R: the manual's ADC1_MUXSEQ_FAIL.
using Adc1MuxseqFail = detail::Field<uint32_t, 13U, 1U>;
/// Bits 12:12, R: the manual's ADC0_MUXSEQ_FAIL.
using Adc0MuxseqFail = detail::Field<uint32_t, 12U, 1U>;
/// Bits 11:11, R: the manual's ADC3_WTCHDG_FAIL.
using Adc3WtchdgFail = detail::Field<uint32_t, 11U, 1U>;
/// Bits 10:10, R: the manual's ADC2_WTCHDG_FAIL.
using Adc2WtchdgFail = detail::Field<uint32_t, 10U, 1U>;
/// Bits 9:9, R: the manual's ADC1_WTCHDG_FAIL.
using Adc1WtchdgFail = detail::Field<uint32_t, 9U, 1U>;
/// Bits 8:8, R: the manual's ADC0_WTCHDG_FAIL.
using Adc0WtchdgFail = detail::Field<uint32_t, 8U, 1U>;
/// Bits 3:3, R: the manual's RDY_ADC_3.
using RdyAdc3 = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, R: the manual's RDY_ADC_2.
using RdyAdc2 = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, R: the manual's RDY_ADC_1.
using RdyAdc1 = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, R: the manual's RDY_ADC_0.
using RdyAdc0 = detail::Field<uint32_t, 0U, 1U>;
} // namespace adc_status
/// Fields of CSA_SETUP.
namespace csa_setup {
/// Bits 19:16, RW: the manual's CSA_AZ_FLTLNGTH_EXP.
using CsaAzFltlngthExp = detail::Field<uint32_t, 16U, 4U>;
/// Bits 15:14, RW: the manual's CSA3_FILT.
using Csa3Filt = detail::Field<uint32_t, 14U, 2U>;
/// Bits 13:12, RW: the manual's CSA012_FILT.
using Csa012Filt = detail::Field<uint32_t, 12U, 2U>;
/// Bits 10:10, RW: the manual's CSA3_BYPASS.
using Csa3Bypass = detail::Field<uint32_t, 10U, 1U>;
/// Bits 9:8, RW: the manual's CSA3_GAIN.
using Csa3Gain = detail::Field<uint32_t, 8U, 2U>;
/// Bits 6:6, RW: the manual's CSA012_BYPASS.
using Csa012Bypass = detail::Field<uint32_t, 6U, 1U>;
/// Bits 5:4, RW: the manual's CSA012_GAIN.
using Csa012Gain = detail::Field<uint32_t, 4U, 2U>;
/// Bits 3:3, RW: the manual's CSA3_EN.
using Csa3En = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, RW: the manual's CSA2_EN.
using Csa2En = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, RW: the manual's CSA1_EN.
using Csa1En = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, RW: the manual's CSA0_EN.
using Csa0En = detail::Field<uint32_t, 0U, 1U>;
} // namespace csa_setup
/// Fields of FAULT_STS.
namespace fault_sts {
/// Bits 12:12, R: the manual's LDO2_READY.
using Ldo2Ready = detail::Field<uint32_t, 12U, 1U>;
/// Bits 11:11, R: the manual's LDO1_READY.
using Ldo1Ready = detail::Field<uint32_t, 11U, 1U>;
/// Bits 10:10, R: the manual's VCCIO_UVLO.
using VccioUvlo = detail::Field<uint32_t, 10U, 1U>;
/// Bits 9:9, R: the manual's VDDA_UVLO.
using VddaUvlo = detail::Field<uint32_t, 9U, 1U>;
/// Bits 8:8, R: the manual's VDD_UVLO.
using VddUvlo = detail::Field<uint32_t, 8U, 1U>;
/// Bits 7:7, R: the manual's VSA_UVLO.
using VsaUvlo = detail::Field<uint32_t, 7U, 1U>;
/// Bits 6:6, R: the manual's CHGP_SHORT.
using ChgpShort = detail::Field<uint32_t, 6U, 1U>;
/// Bits 5:5, R: the manual's CHGP_OK.
using ChgpOk = detail::Field<uint32_t, 5U, 1U>;
/// Bits 4:4, R: the manual's LDOEXT2_SHORT.
using Ldoext2Short = detail::Field<uint32_t, 4U, 1U>;
/// Bits 3:3, R: the manual's LDOEXT1_SHORT.
using Ldoext1Short = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, R: the manual's LDOEXT_TSD.
using LdoextTsd = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, R: the manual's BCK_SHORT.
using BckShort = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, R: the manual's BCK_UVLO.
using BckUvlo = detail::Field<uint32_t, 0U, 1U>;
} // namespace fault_sts
/// Fields of FAULT_R_INT.
namespace fault_r_int {
/// Bits 15:15, RW: the manual's UC_FAULT.
using UcFault = detail::Field<uint32_t, 15U, 1U>;
/// Bits 12:12, RW, W1C: the manual's LDO2_READY_RE_LTC.
using Ldo2ReadyReLtc = detail::Field<uint32_t, 12U, 1U>;
/// Bits 11:11, RW, W1C: the manual's LDO1_READY_RE_LTC.
using Ldo1ReadyReLtc = detail::Field<uint32_t, 11U, 1U>;
/// Bits 10:10, RW, W1C: the manual's VCCIO_UVLO_LTC.
using VccioUvloLtc = detail::Field<uint32_t, 10U, 1U>;
/// Bits 9:9, RW, W1C: the manual's VDDA_UVLO_LTC.
using VddaUvloLtc = detail::Field<uint32_t, 9U, 1U>;
/// Bits 8:8, RW, W1C: the manual's VDD_UVLO_LTC.
using VddUvloLtc = detail::Field<uint32_t, 8U, 1U>;
/// Bits 7:7, RW, W1C: the manual's VSA_UVLO_LTC.
using VsaUvloLtc = detail::Field<uint32_t, 7U, 1U>;
/// Bits 6:6, RW, W1C: the manual's CHGP_SHORT_LTC.
using ChgpShortLtc = detail::Field<uint32_t, 6U, 1U>;
/// Bits 5:5, RW, W1C: the manual's CHGP_OK_LTC.
using ChgpOkLtc = detail::Field<uint32_t, 5U, 1U>;
/// Bits 4:4, RW, W1C: the manual's LDOEXT2_SHORT_LTC.
using Ldoext2ShortLtc = detail::Field<uint32_t, 4U, 1U>;
/// Bits 3:3, RW, W1C: the manual's LDOEXT1_SHORT_LTC.
using Ldoext1ShortLtc = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, RW, W1C: the manual's LDOEXT_TSD_LTC.
using LdoextTsdLtc = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, RW, W1C: the manual's BCK_SHORT_RE_LTC.
using BckShortReLtc = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, RW, W1C: the manual's BCK_UVLO_LTC.
using BckUvloLtc = detail::Field<uint32_t, 0U, 1U>;
} // namespace fault_r_int
/// Fields of FAULT_R_ENA_F.
namespace fault_r_ena_f {
/// Bits 12:12, RW: the manual's LDO2_READY_RE_ENA_F.
using Ldo2ReadyReEnaF = detail::Field<uint32_t, 12U, 1U>;
/// Bits 11:11, RW: the manual's LDO1_READY_RE_ENA_F.
using Ldo1ReadyReEnaF = detail::Field<uint32_t, 11U, 1U>;
/// Bits 10:10, RW: the manual's VCCIO_UVLO_ENA_F.
using VccioUvloEnaF = detail::Field<uint32_t, 10U, 1U>;
/// Bits 9:9, RW: the manual's VDDA_UVLO_ENA_F.
using VddaUvloEnaF = detail::Field<uint32_t, 9U, 1U>;
/// Bits 8:8, RW: the manual's VDD_UVLO_ENA_F.
using VddUvloEnaF = detail::Field<uint32_t, 8U, 1U>;
/// Bits 7:7, RW: the manual's VSA_UVLO_ENA_F.
using VsaUvloEnaF = detail::Field<uint32_t, 7U, 1U>;
/// Bits 6:6, RW: the manual's CHGP_SHORT_ENA_F.
using ChgpShortEnaF = detail::Field<uint32_t, 6U, 1U>;
/// Bits 5:5, RW: the manual's CHGP_OK_ENA_F.
using ChgpOkEnaF = detail::Field<uint32_t, 5U, 1U>;
/// Bits 4:4, RW: the manual's LDOEXT2_SHORT_ENA_F.
using Ldoext2ShortEnaF = detail::Field<uint32_t, 4U, 1U>;
/// Bits 3:3, RW: the manual's LDOEXT1_SHORT_ENA_F.
using Ldoext1ShortEnaF = detail::Field<uint32_t, 3U, 1U>;
/// Bits 2:2, RW: the manual's LDOEXT_TSD_ENA_F.
using LdoextTsdEnaF = detail::Field<uint32_t, 2U, 1U>;
/// Bits 1:1, RW: the manual's BCK_SHORT_RE_ENA_F.
using BckShortReEnaF = detail::Field<uint32_t, 1U, 1U>;
/// Bits 0:0, RW: the manual's BCK_UVLO_ENA_F.
using BckUvloEnaF = detail::Field<uint32_t, 0U, 1U>;
} // namespace fault_r_ena_f
} // namespace fields

} // namespace tmc9660

#endif // TMC9660_REGISTERS_HPP
