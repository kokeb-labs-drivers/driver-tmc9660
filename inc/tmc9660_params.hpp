// Generated from the TMC9660 Parameter Mode Reference Manual (Rev. 0, 02/25), Table 57. Do not edit
// by hand.
/**
 * @file tmc9660_params.hpp
 * @brief The axis parameters of parameter mode (SAP, GAP, STAP, AAP): number, range, default and
 * access of every entry of Table 57, and the value constants of the coded ones.
 * @copyright Copyright (c) 2026 The driver-tmc9660 authors. SPDX-License-Identifier: MIT
 */
#ifndef TMC9660_PARAMS_HPP
#define TMC9660_PARAMS_HPP

#include <array>
#include <cstddef>
#include <cstdint>

namespace tmc9660 {

/// Access class of a parameter (the R/W column of Table 57).
enum class Access : uint8_t {
  ReadOnly = 0,        ///< R: GAP only.
  ReadWrite = 1,       ///< RW: SAP and GAP, not stored by STAP.
  ReadWriteStored = 2, ///< RWE: SAP and GAP, STAP stores it.
  WriteOnly = 3,       ///< W: SAP only.
  ReadWriteClear = 4,  ///< Flags: GAP; SAP with 1 clears the bit (RWC).
};

/// One row of Table 57.
struct ParamInfo {
  uint16_t id;   ///< The parameter number (the TMCL type field).
  Access access; ///< Access class.
  bool coded;    ///< true: only the values in `kAllowedValues[allowed_first ...]` are valid.
  uint8_t allowed_count;  ///< Number of allowed values when `coded`.
  uint16_t allowed_first; ///< Index into `kAllowedValues`.
  int64_t min;            ///< Smallest value (a flag word: 0).
  int64_t max;            ///< Largest value (a flag word: 0xFFFFFFFF).
  int64_t def;            ///< Reset default.
};

/// The axis parameters, by name; the value is the number sent as the TMCL type.
enum class Param : uint16_t {
  /// Selected motor type.
  MOTOR_TYPE = 0,
  /// Pole pair count of the motor.
  MOTOR_POLE_PAIRS = 1,
  /// Motor direction bit. Inverts the motor direction.
  MOTOR_DIRECTION = 2,
  /// Set the frequency of the motor PWM.
  MOTOR_PWM_FREQUENCY = 3,
  /// Selected FOC operation mode depending on feedback used for commutation.
  COMMUTATION_MODE = 4,
  /// PID UQ/UD output limit for circular limiter.
  OUTPUT_VOLTAGE_LIMIT = 5,
  /// Maximum motor torque.
  MAX_TORQUE = 6,
  /// Max. motor flux.
  MAX_FLUX = 7,
  /// PWM switching scheme.
  PWM_SWITCHING_SCHEME = 8,
  /// Configure if the PWM should be off (high-z) or on (all motor phases same voltage) in
  /// commutation.
  IDLE_MOTOR_PWM_BEHAVIOR = 9,
  /// Shunt type used for ADC measurements.
  ADC_SHUNT_TYPE = 12,
  /// Raw ADC measurement of the ADC I0 shunt.
  ADC_I0_RAW = 13,
  /// Raw ADC measurement of the ADC I1 shunt.
  ADC_I1_RAW = 14,
  /// Raw ADC measurement of the ADC I2 shunt.
  ADC_I2_RAW = 15,
  /// Raw ADC measurement of the ADC I3 shunt.
  ADC_I3_RAW = 16,
  /// Current sense amplifier gain for ADC I0, I1 and I2.
  CSA_GAIN_ADC_I0_TO_ADC_I2 = 17,
  /// Current sense amplifier gain for ADC I3.
  CSA_GAIN_ADC_I3 = 18,
  /// Current sense amplifier filter for ADC I0, I1 and I2.
  CSA_FILTER_ADC_I0_TO_ADC_I2 = 19,
  /// Current sense amplifier filter for ADC I3.
  CSA_FILTER_ADC_I3 = 20,
  /// Current scaling factor converting internal units to real- world units.
  CURRENT_SCALING_FACTOR = 21,
  /// Mapping ADC to UX1.
  PHASE_UX1_ADC_MAPPING = 22,
  /// Mapping ADC to VX2.
  PHASE_VX2_ADC_MAPPING = 23,
  /// Mapping ADC to WY1.
  PHASE_WY1_ADC_MAPPING = 24,
  /// Mapping ADC to Y2.
  PHASE_Y2_ADC_MAPPING = 25,
  /// Scaling applied to ADC I0.
  ADC_I0_SCALE = 26,
  /// Scaling applied to ADC I1.
  ADC_I1_SCALE = 27,
  /// Scaling applied to ADC I2.
  ADC_I2_SCALE = 28,
  /// Scaling applied to ADC I3.
  ADC_I3_SCALE = 29,
  /// Invert the reading of ADC I0.
  ADC_I0_INVERTED = 30,
  /// Invert the reading of ADC I1.
  ADC_I1_INVERTED = 31,
  /// Invert the reading of ADC I2.
  ADC_I2_INVERTED = 32,
  /// Invert the reading of ADC I3.
  ADC_I3_INVERTED = 33,
  /// Offset applied to ADC I0 measurement.
  ADC_I0_OFFSET = 34,
  /// Offset applied to ADC I1 measurement.
  ADC_I1_OFFSET = 35,
  /// Offset applied to ADC I2 measurement.
  ADC_I2_OFFSET = 36,
  /// Offset applied to ADC I3 measurement.
  ADC_I3_OFFSET = 37,
  /// Scaled and offset compensated ADC I0 measurement.
  ADC_I0 = 38,
  /// Scaled and offset compensated ADC I1 measurement.
  ADC_I1 = 39,
  /// Scaled and offset compensated ADC I2 measurement.
  ADC_I2 = 40,
  /// Scaled and offset compensated ADC I3 measurement.
  ADC_I3 = 41,
  /// Phi_e calculated by the ramper hardware.
  OPENLOOP_ANGLE = 45,
  /// Openloop current applied in openloop, current mode.
  OPENLOOP_CURRENT = 46,
  /// Openloop voltage applied in openloop, voltage mode.
  OPENLOOP_VOLTAGE = 47,
  /// Gain applied to acceleration feedforward.
  ACCELERATION_FF_GAIN = 50,
  /// Shift applied to acceleration feedforward.
  ACCELERATION_FF_SHIFT = 51,
  /// Enable the application of acceleration and deceleration ramps.
  RAMP_ENABLE = 52,
  /// Specify the control loop structure for velocity mode.
  DIRECT_VELOCITY_MODE = 53,
  /// Acceleration in top part of eight-point ramp.
  RAMP_AMAX = 54,
  /// First acceleration in eight-point ramp.
  RAMP_A1 = 55,
  /// Second acceleration in eight-point ramp.
  RAMP_A2 = 56,
  /// Deceleration in top part of eight-point ramp.
  RAMP_DMAX = 57,
  /// Second deceleration in eight-point ramp.
  RAMP_D1 = 58,
  /// First deceleration in eight-point ramp.
  RAMP_D2 = 59,
  /// Maximum velocity of eight-point ramp.
  RAMP_VMAX = 60,
  /// Velocity threshold to switch from A1/D1 to A2/D2.
  RAMP_V1 = 61,
  /// Velocity threshold to switch from A2/D2 to AMAX/DMAX.
  RAMP_V2 = 62,
  /// Start velocity of ramp.
  RAMP_VSTART = 63,
  /// Stop velocity of ramp.
  RAMP_VSTOP = 64,
  /// Minimum time at VMAX to start deceleration.
  RAMP_TVMAX = 65,
  /// Wait time at end of ramp to signal stop.
  RAMP_TZEROWAIT = 66,
  /// Enable the acceleration feedforward feature.
  ACCELERATION_FEEDFORWARD_ENABLE = 67,
  /// Enable the velocity feedforward feature.
  VELOCITY_FEEDFORWARD_ENABLE = 68,
  /// Target velocity calculated by ramp controller.
  RAMP_VELOCITY = 69,
  /// Target position calculated by ramp controller.
  RAMP_POSITION = 70,
  /// Phi_e calculated from hall feedback.
  HALL_PHI_E = 74,
  /// Hall sensor 60-degree/sector offset composed of 120 offset (order) and 180 degree offset
  /// (polari.
  HALL_SECTOR_OFFSET = 75,
  /// Filter length of the hall sensor input signal filters.
  HALL_FILTER_LENGTH = 76,
  /// Hall offset compensation for 0 degree hall position.
  HALL_POSITION_0_OFFSET = 77,
  /// Hall offset compensation for 60 degree hall position.
  HALL_POSITION_60_OFFSET = 78,
  /// Hall offset compensation for 120 degree hall position.
  HALL_POSITION_120_OFFSET = 79,
  /// Hall offset compensation for 180 degree hall position.
  HALL_POSITION_180_OFFSET = 80,
  /// Hall offset compensation for 240 degree hall position.
  HALL_POSITION_240_OFFSET = 81,
  /// Hall offset compensation for 300 degree hall position.
  HALL_POSITION_300_OFFSET = 82,
  /// Invert the hall angle direction.
  HALL_INVERT_DIRECTION = 83,
  /// Enable the hall extrapolation to generate a higher resolution position signal.
  HALL_EXTRAPOLATION_ENABLE = 84,
  /// Use this parameter to compensate hall sensor mounting tolerances.
  HALL_PHI_E_OFFSET = 85,
  /// Phi_e calculated from abn feedback.
  ABN_1_PHI_E = 89,
  /// ABN 1 encoder steps per rotation (CPR).
  ABN_1_STEPS = 90,
  /// ABN 1 encoder rotation direction.
  ABN_1_DIRECTION = 91,
  /// Select an ABN encoder initialization method that fits best to your motor's sensors.
  ABN_1_INIT_METHOD = 92,
  /// Actual state of ABN encoder initialization.
  ABN_1_INIT_STATE = 93,
  /// When one of the "Forced phi_e" initialization methods is used, this value defines the wait
  /// time.
  ABN_1_INIT_DELAY = 94,
  /// Init velocity for ABN encoder initialization with N- channel offset.
  ABN_1_INIT_VELOCITY = 95,
  /// Offset between phi_e zero and the ABN encoders index pulse position.
  ABN_1_N_CHANNEL_PHI_E_OFFSET = 96,
  /// ABN 1 encoder N-channel is inverted.
  ABN_1_N_CHANNEL_INVERTED = 97,
  /// ABN 1 encoder N-channel filtering.
  ABN_1_N_CHANNEL_FILTERING = 98,
  /// Clear the actual position on the next ABN 1 encoder N- channel event.
  ABN_1_CLEAR_ON_NEXT_NULL = 99,
  /// Raw ABN encoder internal counter value.
  ABN_1_VALUE = 100,
  /// Target torque value.
  TARGET_TORQUE = 104,
  /// Actual motor torque value.
  ACTUAL_TORQUE = 105,
  /// Target flux value.
  TARGET_FLUX = 106,
  /// Actual motor flux value.
  ACTUAL_FLUX = 107,
  /// Offset applied to torque value.
  TORQUE_OFFSET = 108,
  /// P parameter for torque PI regulator.
  TORQUE_P = 109,
  /// I parameter for torque PI regulator.
  TORQUE_I = 110,
  /// P parameter for flux PI regulator.
  FLUX_P = 111,
  /// I parameter for flux PI regulator.
  FLUX_I = 112,
  /// Enable to configure separate PI values for the torque and flux current control loops.
  SEPARATE_TORQUE_FLUX_PI_PARAMTERS = 113,
  /// P parameter normalization format for current PI regulator.
  CURRENT_NORM_P = 114,
  /// I parameter normalization format for current PI regulator.
  CURRENT_NORM_I = 115,
  /// Torque PI regulator error.
  TORQUE_PI_ERROR = 116,
  /// Flux PI regulator error.
  FLUX_PI_ERROR = 117,
  /// Integrated error of torque PI regulator.
  TORQUE_PI_INTEGRATOR = 118,
  /// Integrated error of flux PI regulator.
  FLUX_PI_INTEGRATOR = 119,
  /// Offset applied to flux value.
  FLUX_OFFSET = 120,
  /// Feedback source for the velocity PI regulator.
  VELOCITY_SENSOR_SELECTION = 123,
  /// Target velocity value.
  TARGET_VELOCITY = 124,
  /// Actual velocity value.
  ACTUAL_VELOCITY = 125,
  /// Offset applied to velocity value.
  VELOCITY_OFFSET = 126,
  /// P parameter for velocity PI regulator.
  VELOCITY_P = 127,
  /// I parameter for velocity PI regulator.
  VELOCITY_I = 128,
  /// P parameter normalization format for velocity PI regulator.
  VELOCITY_NORM_P = 129,
  /// I parameter normalization format for velocity PI regulator.
  VELOCITY_NORM_I = 130,
  /// Integrated error of velocity PI regulator.
  VELOCITY_PI_INTEGRATOR = 131,
  /// Velocity PI regulator error.
  VELOCITY_PI_ERROR = 132,
  /// Scaling factor to convert internal velocity to real-world units.
  VELOCITY_SCALING_FACTOR = 133,
  /// Maximum of velocity deviation tolerated before stop event is triggered (if activated).
  STOP_ON_VELOCITY_DEVIATION = 134,
  /// Downsampling factor for velocity controller.
  VELOCITY_LOOP_DOWNSAMPLING = 135,
  /// Deviation between target and actual velocity below which the velocity reached flag goes active
  /// a.
  VELOCITY_REACHED_THRESHOLD = 136,
  /// Velocity threshold switching from period to frequency velocity meter.
  VELOCITY_METER_SWITCH_THRESHOLD = 137,
  /// Velocity hysteresis for switching back from frequency to period velocity meter.
  VELOCITY_METER_SWITCH_HYSTERESIS = 138,
  /// Currently used velocity meter mode.
  VELOCITY_METER_MODE = 139,
  /// Feedback source for the position PI regulator.
  POSITION_SENSOR_SELECTION = 142,
  /// Target position value.
  TARGET_POSITION = 143,
  /// Actual position value.
  ACTUAL_POSITION = 144,
  /// Scaling factor to convert internal position to real-world units.
  POSITION_SCALING_FACTOR = 145,
  /// P parameter for position PI regulator.
  POSITION_P = 146,
  /// I parameter for position PI regulator.
  POSITION_I = 147,
  /// P parameter normalization format for position PI regulator.
  POSITION_NORM_P = 148,
  /// I parameter normalization format for position PI regulator.
  POSITION_NORM_I = 149,
  /// Integrated error of position PI regulator.
  POSITION_PI_INTEGRATOR = 150,
  /// Error of position PI regulator.
  POSITION_PI_ERROR = 151,
  /// Maximum of position deviation tolerated before stop event is triggered (if activated).
  STOP_ON_POSITION_DEVIATION = 152,
  /// Downsampling factor for position controller.
  POSITION_LOOP_DOWNSAMPLING = 153,
  /// Position switch latched.
  LATCH_POSITION = 154,
  /// Position limit low.
  POSITION_LIMIT_LOW = 155,
  /// Position limit high.
  POSITION_LIMIT_HIGH = 156,
  /// Deviation between target and actual position below which the position reached flag goes active
  /// a.
  POSITION_REACHED_THRESHOLD = 157,
  /// Bitwise enable for stopping when reference switch input is triggered.
  REFERENCE_SWITCH_ENABLE = 161,
  /// Bitwise configuration of reference switch configuration.
  REFERENCE_SWITCH_POLARITY_AND_SWAP = 162,
  /// Bitwise configuration of reference switch latch configuration.
  REFERENCE_SWITCH_LATCH_SETTINGS = 163,
  /// Bitwise configuration of stop configuration.
  EVENT_STOP_SETTINGS = 164,
  /// Determine the reference switch search sequence.
  REFERENCE_SWITCH_SEARCH_MODE = 165,
  /// Speed used for the reference switch search sequence.
  REFERENCE_SWITCH_SEARCH_SPEED = 166,
  /// Lower speed used e.g. for positioning the motor at a reference switch position.
  REFERENCE_SWITCH_SPEED = 167,
  /// Right limit switch position.
  RIGHT_LIMIT_SWITCH_POSITION = 168,
  /// Home switch position.
  HOME_SWITCH_POSITION = 169,
  /// Last reference position.
  LAST_REFERENCE_POSITION = 170,
  /// ABN 2 encoder steps per rotation (CPR).
  ABN_2_STEPS = 174,
  /// ABN 2 encoder rotation direction.
  ABN_2_DIRECTION = 175,
  /// ABN 2 encoder gear ratio.
  ABN_2_GEAR_RATIO = 176,
  /// Enable the ABN 2 encoder.
  ABN_2_ENABLE = 177,
  /// Raw ABN2 encoder internal counter value.
  ABN_2_VALUE = 178,
  /// Add a delay from CS going low to first SCLK edge.
  SPI_ENCODE_CS_SETTLE_DELAY_TIME = 181,
  /// Extend CS idle time between SPI message frames.
  SPI_ENCODER_CS_IDLE_DELAY_TIME = 182,
  /// Size of the first SPI transfer frame.
  SPI_ENCODER_MAIN_TRANSFER_CMD_SIZE = 183,
  /// Size of the optional secondary SPI transfer frame.
  SPI_ENCODER_SECONDARY_TRANSFER_CMD_SIZE = 184,
  /// Used to set the transmit data and read out the received data.
  SPI_ENCODER_TRANSFER_DATA_3_0 = 185,
  /// Used to set the transmit data and read out the received data.
  SPI_ENCODER_TRANSFER_DATA_7_4 = 186,
  /// Used to set the transmit data and read out the received data.
  SPI_ENCODER_TRANSFER_DATA_11_8 = 187,
  /// Used to set the transmit data and read out the received data.
  SPI_ENCODER_TRANSFER_DATA_15_12 = 188,
  /// SPI interface setting, polarity and phase.
  SPI_ENCODER_TRANSFER = 189,
  /// Mask to be used to collect the position counter value from the continuous received data.
  SPI_ENCODER_POSITION_COUNTER_MASK = 190,
  /// Right bit shift for the position counter value before mask is applied.
  SPI_ENCODER_POSITION_COUNTER_SHIFT = 191,
  /// Actual SPI encoder position value.
  SPI_ENCODER_POSITION_COUNTER_VALUE = 192,
  /// Actual absolute encoder angle value.
  SPI_ENCODER_COMMUTATION_ANGLE = 193,
  /// Select the used absolute encoder initialization mode Forces the rotor into PHI_E zero using
  /// the.
  SPI_ENCODER_INITIALIZATION_METHOD = 194,
  /// SPI encoder direction.
  SPI_ENCODER_DIRECTION = 195,
  /// This value represents the internal commutation offset.
  SPI_ENCODER_OFFSET = 196,
  /// Enable the lookup table based encoder correction.
  SPI_LUT_CORRECTION_ENABLE = 197,
  /// Address to read or write the lookup table.
  SPI_LUT_ADDRESS_SELECT = 198,
  /// Data to read or write to a lookup table address.
  SPI_LUT_DATA = 199,
  /// All LUT table entries are multiplied with 2^SHIFT_FACTOR to compensate for larger erros if
  /// neede.
  SPI_LUT_COMMON_SHIFT_FACTOR = 201,
  /// Configure step/dir to use between 1 and 1024 microsteps per full step.
  STEP_DIR_STEP_DIVIDER_SHIFT = 205,
  /// Enable the Step/Dir input functionality.
  STEP_DIR_ENABLE = 206,
  /// Enable the Step/Dir extrapolation feature.
  STEP_DIR_EXTRAPOLATION_ENABLE = 207,
  /// Step signal timeout limit.
  STEP_DIR_STEP_SIGNAL_TIMEOUT_LIMIT = 208,
  /// Maximum velocity up to which extrapolation is used.
  STEP_DIR_MAXIMUM_EXTRAPOLATION_VELOCITY = 209,
  /// Enable the brake chopper functionality.
  BRAKE_CHOPPER_ENABLE = 212,
  /// If the brake chopper is enabled and supply voltage exceeds this value, the brake chopper
  /// output.
  BRAKE_CHOPPER_VOLTAGE_LIMIT = 213,
  /// An activated brake chopper is deactivated if the actual supply voltage is lower than
  /// BRAKE_CHOPP.
  BRAKE_CHOPPER_HYSTERESIS = 214,
  /// Release the external brake by applying a PWM signal.
  RELEASE_BRAKE = 216,
  /// Set the duty cycle of the first PWM phase for releasing the brake.
  BRAKE_RELEASING_DUTY_CYCLE = 217,
  /// Set the duty cycle of the second PWM phase to hold the brake.
  BRAKE_HOLDING_DUTY_CYCLE = 218,
  /// Set the duration the brake PWM uses the first duty cycle.
  BRAKE_RELEASING_DURATION = 219,
  /// Invert the brake output.
  INVERT_BRAKE_OUTPUT = 221,
  /// Thermal winding time constant for the used motor.
  THERMAL_WINDING_TIME_CONSTANT_1 = 224,
  /// An actual IIT sum that exceeds this limit leads to trigger the IIT_1_EXCEEDED.
  IIT_LIMIT_1 = 225,
  /// Actual sum of the IIT monitor 1.
  IIT_SUM_1 = 226,
  /// Thermal winding time constant for the used motor.
  THERMAL_WINDING_TIME_CONSTANT_2 = 227,
  /// An actual IIT sum that exceeds this limit leads to trigger the IIT_2_EXCEEDED.
  IIT_LIMIT_2 = 228,
  /// Actual sum of the IIT monitor 2.
  IIT_SUM_2 = 229,
  /// Reset both IIT sums.
  RESET_IIT_SUMS = 230,
  /// Total current through the motor windings.
  ACTUAL_TOTAL_MOTOR_CURRENT = 231,
  /// PWM_L output polarity.
  PWM_L_OUTPUT_POLARITY = 233,
  /// PWM_H output polarity.
  PWM_H_OUTPUT_POLARITY = 234,
  /// Break before make time for the low side gates of the UVW phases.
  BREAK_BEFORE_MAKE_TIME_LOW_UVW = 235,
  /// Break before make time for the high side gates of the UVW phases.
  BREAK_BEFORE_MAKE_TIME_HIGH_UVW = 236,
  /// Break before make time for the low side gate of the Y2 phase.
  BREAK_BEFORE_MAKE_TIME_LOW_Y2 = 237,
  /// Break before make time for the high side gate of the Y2 phase.
  BREAK_BEFORE_MAKE_TIME_HIGH_Y2 = 238,
  /// If enabled, the discharge cycle of the low- and high-side gates for the UVW phases is
  /// shortened.
  USE_ADAPTIVE_DRIVE_TIME_UVW = 239,
  /// If enabled, the discharge cycle of the low- and high-side gates for the Y2 phase is shortened
  /// by.
  USE_ADAPTIVE_DRIVE_TIME_Y2 = 240,
  /// Discharge time for the low and high side gates of the UVW phases.
  DRIVE_TIME_SINK_UVW = 241,
  /// Charge time for the low and high side gates of the UVW phases.
  DRIVE_TIME_SOURCE_UVW = 242,
  /// Discharge time for the low and high side gates of the Y2 phase.
  DRIVE_TIME_SINK_Y2 = 243,
  /// Charge time for the low and high side gates of the Y2 phase.
  DRIVE_TIME_SOURCE_Y2 = 244,
  /// Limit the maximum sink current for the low and high side gates of the UVW phases.
  UVW_SINK_CURRENT = 245,
  /// Limit the maximum source current for the low and high side gates of the UVW phases.
  UVW_SOURCE_CURRENT = 246,
  /// Limit the maximum sink current for the low and high side gates of the Y2 phase.
  Y2_SINK_CURRENT = 247,
  /// Limit the maximum source current for the low and high side gates of the Y2 phase.
  Y2_SOURCE_CURRENT = 248,
  /// Bootstrap current limit.
  BOOTSTRAP_CURRENT_LIMIT = 249,
  /// Undervoltage protection level for VS (Supply voltage). 0 disables the comparator.
  UNDERVOLTAGE_PROTECTION_SUPPLY_LEVEL = 250,
  /// Enable the undervoltage protection for VDRV (Driver voltage).
  UNDERVOLTAGE_PROTECTION_VDRV_ENABLE = 251,
  /// Enable the undervoltage protection on the bootstrap capacitor of the UVW phases.
  UNDERVOLTAGE_PROTECTION_BST_UVW_ENABLE = 252,
  /// Enable the undervoltage protection on the bootstrap capacitor of the Y2 phase.
  UNDERVOLTAGE_PROTECTION_BST_Y2_ENABLE = 253,
  /// Enable the overcurrent protection on the low side of the UVW phases.
  OVERCURRENT_PROTECTION_UVW_LOW_SIDE_ENABLE = 254,
  /// Enable the overcurrent protection on the high side of the UVW phases.
  OVERCURRENT_PROTECTION_UVW_HIGH_SIDE_ENABLE = 255,
  /// Enable the overcurrent protection on the low side of the Y2 phase.
  OVERCURRENT_PROTECTION_Y2_LOW_SIDE_ENABLE = 256,
  /// Enable the overcurrent protection on the high side of the Y2 phase.
  OVERCURRENT_PROTECTION_Y2_HIGH_SIDE_ENABLE = 257,
  /// Overcurrent protection threshold for the low side of the UVW phases (uses second list if
  /// OVERCUR.
  OVERCURRENT_PROTECTION_UVW_LOW_SIDE_THRESHOLD = 258,
  /// Overcurrent protection threshold for the high side of the UVW phases.
  OVERCURRENT_PROTECTION_UVW_HIGH_SIDE_THRESHOLD = 259,
  /// Overcurrent protection threshold for the low side of the Y2 phase (uses second list if
  /// OVERCURRE.
  OVERCURRENT_PROTECTION_Y2_LOW_SIDE_THRESHOLD = 260,
  /// Overcurrent protection threshold for the high side of the Y2 phase.
  OVERCURRENT_PROTECTION_Y2_HIGH_SIDE_THRESHOLD = 261,
  /// Overcurrent protection blanking time for the low side of the UVW phases.
  OVERCURRENT_PROTECTION_UVW_LOW_SIDE_BLANKING = 262,
  /// Overcurrent protection blanking time for the high side of the UVW phases.
  OVERCURRENT_PROTECTION_UVW_HIGH_SIDE_BLANKING = 263,
  /// Overcurrent protection blanking time for the low side of the Y2 phase.
  OVERCURRENT_PROTECTION_Y2_LOW_SIDE_BLANKING = 264,
  /// Overcurrent protection blanking time for the high side of the Y2 phase.
  OVERCURRENT_PROTECTION_Y2_HIGH_SIDE_BLANKING = 265,
  /// Overcurrent protection deglitch time for the low side of the UVW phases.
  OVERCURRENT_PROTECTION_UVW_LOW_SIDE_DEGLITCH = 266,
  /// Overcurrent protection deglitch time for the high side of the UVW phases.
  OVERCURRENT_PROTECTION_UVW_HIGH_SIDE_DEGLITCH = 267,
  /// Overcurrent protection deglitch time for the low side of the Y2 phase.
  OVERCURRENT_PROTECTION_Y2_LOW_SIDE_DEGLITCH = 268,
  /// Overcurrent protection deglitch time for the high side of the Y2 phase.
  OVERCURRENT_PROTECTION_Y2_HIGH_SIDE_DEGLITCH = 269,
  /// Use the VDS measurement for the overcurrent protection on the low side of the UVW phases.
  OVERCURRENT_PROTECTION_UVW_LOW_SIDE_USE_VDS = 270,
  /// Use the VDS measurement for the overcurrent protection on the low side of the Y2 phase.
  OVERCURRENT_PROTECTION_Y2_LOW_SIDE_USE_VDS = 271,
  /// Enable the gate-source short protection for the ON transition of the low side of the UVW
  /// phases.
  VGS_SHORT_ON_PROTECTION_UVW_LOW_SIDE_ENABLE = 272,
  /// Enable the gate-source short protection for the OFF transition of the low side of the UVW
  /// phases.
  VGS_SHORT_OFF_PROTECTION_UVW_LOW_SIDE_ENABLE = 273,
  /// Enable the gate-source short protection for the ON transition of the high side of the UVW
  /// phases.
  VGS_SHORT_ON_PROTECTION_UVW_HIGH_SIDE_ENABLE = 274,
  /// Enable the gate-source short protection for the OFF transition of the high side of the UVW
  /// phase.
  VGS_SHORT_OFF_PROTECTION_UVW_HIGH_SIDE_ENABLE = 275,
  /// Enable the gate-source short protection for the ON transition of the low side of the Y2 phase.
  VGS_SHORT_ON_PROTECTION_Y2_LOW_SIDE_ENABLE = 276,
  /// Enable the gate-source short protection for the OFF transition of the low side of the Y2
  /// phase.
  VGS_SHORT_OFF_PROTECTION_Y2_LOW_SIDE_ENABLE = 277,
  /// Enable the gate-source short protection for the ON transition of the high side of the Y2
  /// phase.
  VGS_SHORT_ON_PROTECTION_Y2_HIGH_SIDE_ENABLE = 278,
  /// Enable the gate-source short protection for the OFF transition of the high side of the Y2
  /// phase.
  VGS_SHORT_OFF_PROTECTION_Y2_HIGH_SIDE_ENABLE = 279,
  /// Gate-source short protection blanking time for the low and high sides of the UVW phases.
  VGS_SHORT_PROTECTION_UVW_BLANKING = 280,
  /// Gate-source short protection blanking time for the low and high sides of the Y2 phase.
  VGS_SHORT_PROTECTION_Y2_BLANKING = 281,
  /// Gate-source short protection deglitch time for the low and high sides of the UVW phases.
  VGS_SHORT_PROTECTION_UVW_DEGLITCH = 282,
  /// Gate-source short protection deglitch time for low and high sides of the Y2 phase.
  VGS_SHORT_PROTECTION_Y2_DEGLITCH = 283,
  /// This value defines the state the system goes to after a fault condition on a motor phase
  /// occurs.
  GDRV_RETRY_BEHAVIOUR = 286,
  /// This value defines the state the system goes to after a fault condition on a motor phase
  /// occurs.
  DRIVE_FAULT_BEHAVIOUR = 287,
  /// Maximum number of retries that are performed for every fault that is detected.
  FAULT_HANDLER_NUMBER_OF_RETRIES = 288,
  /// Actual status flags.
  GENERAL_STATUS_FLAGS = 289,
  /// The actual supply voltage.
  SUPPLY_VOLTAGE = 290,
  /// The supply overvoltage warning threshold.
  SUPPLY_OVERVOLTAGE_WARNING_THRESHOLD = 291,
  /// The supply undervoltage warning threshold.
  SUPPLY_UNDERVOLTAGE_WARNING_THRESHOLD = 292,
  /// The actual temperature at the external temperature sensor.
  EXTERNAL_TEMPERATURE = 293,
  /// The temperature threshold at which the motor driver is shut down.
  EXTERNAL_TEMPERATURE_SHUTDOWN_THRESHOLD = 294,
  /// The temperature threshold above which the warning flag is set.
  EXTERNAL_TEMPERATURE_WARNING_THRESHOLD = 295,
  /// The actual temperature of the chip.
  CHIP_TEMPERATURE = 296,
  /// The temperature threshold at which the motor driver is shut down.
  CHIP_TEMPERATURE_SHUTDOWN_THRESHOLD = 297,
  /// The temperature threshold above which the warning flag is set.
  CHIP_TEMPERATURE_WARNING_THRESHOLD = 298,
  /// Actual error flags.
  GENERAL_ERROR_FLAGS = 299,
  /// Gate driver error flags.
  GDRV_ERROR_FLAGS = 300,
  /// ADC channel clipped.
  ADC_STATUS_FLAGS = 301,
  /// Raw inputs for ABN, hall, reference switches, driver enabled, hall filtered and ABN2 or
  /// Step/Dir.
  MCC_INPUTS_RAW = 304,
  /// Interim result of the FOC for phase U (X in case of motor type is a stepper motor).
  FOC_VOLTAGE_UX = 305,
  /// Interim result of the FOC for phase W (Y in case of motor type is a stepper motor).
  FOC_VOLTAGE_WY = 306,
  /// Interim result of the FOC for phase V (BLDC motor only).
  FOC_VOLTAGE_V = 307,
  /// I parameter for field weakening controller.
  FIELDWEAKENING_I = 308,
  /// Maximum motor voltage allowed for field weakening.
  FIELDWEAKENING_VOLTAGE_THRESHOLD = 310,
  /// Interim measurement of the FOC for phase UX.
  FOC_CURRENT_UX = 311,
  /// Interim measurement of the FOC for phase V.
  FOC_CURRENT_V = 312,
  /// Interim measurement of the FOC for phase WY.
  FOC_CURRENT_WY = 313,
  /// Interim measurement of the FOC for Uq.
  FOC_VOLTAGE_UQ = 314,
  /// Interim measurement of the FOC for Iq.
  FOC_CURRENT_IQ = 315,
  /// Enable the target torque biquad filter.
  TARGET_TORQUE_BIQUAD_FILTER_ENABLE = 318,
  /// Target torque biquad filter aCoeff_1.
  TARGET_TORQUE_BIQUAD_FILTER_ACOEFF_1 = 319,
  /// Target torque biquad filter aCoeff_2.
  TARGET_TORQUE_BIQUAD_FILTER_ACOEFF_2 = 320,
  /// Target torque biquad filter bCoeff_0.
  TARGET_TORQUE_BIQUAD_FILTER_BCOEFF_0 = 321,
  /// Target torque biquad filter bCoeff_1.
  TARGET_TORQUE_BIQUAD_FILTER_BCOEFF_1 = 322,
  /// Target torque biquad filter bCoeff_2.
  TARGET_TORQUE_BIQUAD_FILTER_BCOEFF_2 = 323,
  /// Enable the actual velocity biquad filter.
  ACTUAL_VELOCITY_BIQUAD_FILTER_ENABLE = 324,
  /// Actual velocity biquad filter aCoeff_1.
  ACTUAL_VELOCITY_BIQUAD_FILTER_ACOEFF_1 = 325,
  /// Actual velocity biquad filter aCoeff_2.
  ACTUAL_VELOCITY_BIQUAD_FILTER_ACOEFF_2 = 326,
  /// Actual velocity biquad filter bCoeff_0.
  ACTUAL_VELOCITY_BIQUAD_FILTER_BCOEFF_0 = 327,
  /// Actual velocity biquad filter bCoeff_1.
  ACTUAL_VELOCITY_BIQUAD_FILTER_BCOEFF_1 = 328,
  /// Actual velocity biquad filter bCoeff_2.
  ACTUAL_VELOCITY_BIQUAD_FILTER_BCOEFF_2 = 329,
  /// Raw (unscaled) torque and flux target values combined into one 32 bit value.
  TORQUE_FLUX_COMBINED_TARGET_VALUES = 330,
  /// Raw (unscaled) torque and flux actual values combined into one 32 bit value.
  TORQUE_FLUX_COMBINED_ACTUAL_VALUES = 331,
  /// Raw (unscaled) voltage actual values combined into one 32 bit value.
  VOLTAGE_D_Q_COMBINED_ACTUAL_VALUES = 332,
  /// Periodically summed up actual torque value.
  INTEGRATED_ACTUAL_TORQUE_VALUE = 333,
  /// Periodically summed up actual velocity value.
  INTEGRATED_ACTUAL_VELOCITY_VALUE = 334,
};

/// Every value a coded parameter accepts, concatenated.
inline constexpr std::array<int32_t, 513> kAllowedValues{
    0,  1,  2,  3,  0,  1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  1,  2,  0,  1,  0,  1,  2,  3,
    4,  0,  1,  2,  3,  4,  0,  1,  2,  3,  4,  0,  1,  2,  3,  0,  1,  2,  3,  0,  1,  2,  3,  0,
    1,  2,  3,  0,  1,  2,  3,  0,  1,  2,  3,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  2,  3,  4,
    5,  6,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  2,  3,  4,  5,  0,  1,  0,  1,  0,  1,  0,  1,
    2,  3,  0,  1,  2,  3,  0,  1,  0,  1,  2,  3,  4,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  2,
    3,  4,  0,  1,  2,  3,  0,  1,  2,  3,  0,  1,  2,  0,  1,  2,  3,  4,  0,  1,  2,  3,  0,  1,
    2,  3,  0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
    14, 15, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  5,
    6,  7,  1,  2,  3,  4,  5,  6,  7,  8,  0,  1,  0,  1,  0,  1,  2,  0,  1,  2,  0,  1,  0,  1,
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,
    1,  0,  1,  0,  1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,
    3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
    11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,
    3,  4,  5,  6,  7,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  2,  3,  4,
    5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
    13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  4,
    5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,
    5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,
    5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,
    5,  6,  7,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,
    1,  2,  3,  0,  1,  2,  3,  0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,  0,
    1,  0,  1,  2,  3,  0,  1,  0,  1,
};

/// Table 57, sorted by number.
inline constexpr std::array<ParamInfo, 289> kParams{{
    {.id = 0,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 0,
     .min = 0LL,
     .max = 3LL,
     .def = 0LL},
    {.id = 1,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 4,
     .min = 0LL,
     .max = 127LL,
     .def = 1LL},
    {.id = 2,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 4,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 3,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 6,
     .min = 10000LL,
     .max = 100000LL,
     .def = 25000LL},
    {.id = 4,
     .access = Access::ReadWrite,
     .coded = true,
     .allowed_count = 9,
     .allowed_first = 6,
     .min = 0LL,
     .max = 8LL,
     .def = 0LL},
    {.id = 5,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 15,
     .min = 0LL,
     .max = 32767LL,
     .def = 8000LL},
    {.id = 6,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 15,
     .min = 0LL,
     .max = 65535LL,
     .def = 2000LL},
    {.id = 7,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 15,
     .min = 0LL,
     .max = 65535LL,
     .def = 2000LL},
    {.id = 8,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 3,
     .allowed_first = 15,
     .min = 0LL,
     .max = 2LL,
     .def = 1LL},
    {.id = 9,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 18,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 12,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 5,
     .allowed_first = 20,
     .min = 0LL,
     .max = 4LL,
     .def = 4LL},
    {.id = 13,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 25,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 14,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 25,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 15,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 25,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 16,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 25,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 17,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 5,
     .allowed_first = 25,
     .min = 0LL,
     .max = 4LL,
     .def = 1LL},
    {.id = 18,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 5,
     .allowed_first = 30,
     .min = 0LL,
     .max = 4LL,
     .def = 1LL},
    {.id = 19,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 35,
     .min = 0LL,
     .max = 3LL,
     .def = 0LL},
    {.id = 20,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 39,
     .min = 0LL,
     .max = 3LL,
     .def = 0LL},
    {.id = 21,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 43,
     .min = 1LL,
     .max = 65535LL,
     .def = 520LL},
    {.id = 22,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 43,
     .min = 0LL,
     .max = 3LL,
     .def = 0LL},
    {.id = 23,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 47,
     .min = 0LL,
     .max = 3LL,
     .def = 1LL},
    {.id = 24,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 51,
     .min = 0LL,
     .max = 3LL,
     .def = 2LL},
    {.id = 25,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 55,
     .min = 0LL,
     .max = 3LL,
     .def = 3LL},
    {.id = 26,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 59,
     .min = 1LL,
     .max = 32767LL,
     .def = 1024LL},
    {.id = 27,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 59,
     .min = 1LL,
     .max = 32767LL,
     .def = 1024LL},
    {.id = 28,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 59,
     .min = 1LL,
     .max = 32767LL,
     .def = 1024LL},
    {.id = 29,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 59,
     .min = 1LL,
     .max = 32767LL,
     .def = 1024LL},
    {.id = 30,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 59,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 31,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 61,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 32,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 63,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 33,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 65,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 34,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 67,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 35,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 67,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 36,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 67,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 37,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 67,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 38,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 67,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 39,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 67,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 40,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 67,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 41,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 67,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 45,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 67,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 46,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 67,
     .min = 0LL,
     .max = 65535LL,
     .def = 1000LL},
    {.id = 47,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 67,
     .min = 0LL,
     .max = 16383LL,
     .def = 0LL},
    {.id = 50,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 67,
     .min = 0LL,
     .max = 65535LL,
     .def = 8LL},
    {.id = 51,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 7,
     .allowed_first = 67,
     .min = 0LL,
     .max = 6LL,
     .def = 4LL},
    {.id = 52,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 74,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 53,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 76,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 54,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 78,
     .min = 1LL,
     .max = 8388607LL,
     .def = 1000LL},
    {.id = 55,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 78,
     .min = 1LL,
     .max = 8388607LL,
     .def = 8000LL},
    {.id = 56,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 78,
     .min = 1LL,
     .max = 8388607LL,
     .def = 4000LL},
    {.id = 57,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 78,
     .min = 1LL,
     .max = 8388607LL,
     .def = 1000LL},
    {.id = 58,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 78,
     .min = 1LL,
     .max = 8388607LL,
     .def = 8000LL},
    {.id = 59,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 78,
     .min = 1LL,
     .max = 8388607LL,
     .def = 8000LL},
    {.id = 60,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 78,
     .min = 0LL,
     .max = 134217727LL,
     .def = 134217727LL},
    {.id = 61,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 78,
     .min = 0LL,
     .max = 134217727LL,
     .def = 0LL},
    {.id = 62,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 78,
     .min = 0LL,
     .max = 134217727LL,
     .def = 0LL},
    {.id = 63,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 78,
     .min = 0LL,
     .max = 8388607LL,
     .def = 0LL},
    {.id = 64,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 78,
     .min = 1LL,
     .max = 8388607LL,
     .def = 1LL},
    {.id = 65,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 78,
     .min = 0LL,
     .max = 65535LL,
     .def = 0LL},
    {.id = 66,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 78,
     .min = 0LL,
     .max = 65535LL,
     .def = 0LL},
    {.id = 67,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 78,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 68,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 80,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 69,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 82,
     .min = -134217727LL,
     .max = 134217727LL,
     .def = 0LL},
    {.id = 70,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 82,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 74,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 82,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 75,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 6,
     .allowed_first = 82,
     .min = 0LL,
     .max = 5LL,
     .def = 0LL},
    {.id = 76,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 88,
     .min = 0LL,
     .max = 255LL,
     .def = 0LL},
    {.id = 77,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 88,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 78,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 88,
     .min = -32768LL,
     .max = 32767LL,
     .def = 10922LL},
    {.id = 79,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 88,
     .min = -32768LL,
     .max = 32767LL,
     .def = 21845LL},
    {.id = 80,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 88,
     .min = -32768LL,
     .max = 32767LL,
     .def = -32768LL},
    {.id = 81,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 88,
     .min = -32768LL,
     .max = 32767LL,
     .def = -21846LL},
    {.id = 82,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 88,
     .min = -32768LL,
     .max = 32767LL,
     .def = -10923LL},
    {.id = 83,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 88,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 84,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 90,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 85,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 92,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 89,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 92,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 90,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 92,
     .min = 0LL,
     .max = 16777215LL,
     .def = 65536LL},
    {.id = 91,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 92,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 92,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 94,
     .min = 0LL,
     .max = 3LL,
     .def = 0LL},
    {.id = 93,
     .access = Access::ReadOnly,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 98,
     .min = 0LL,
     .max = 3LL,
     .def = 0LL},
    {.id = 94,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 102,
     .min = 1000LL,
     .max = 10000LL,
     .def = 1000LL},
    {.id = 95,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 102,
     .min = -200000LL,
     .max = 200000LL,
     .def = 5LL},
    {.id = 96,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 102,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 97,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 102,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 98,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 5,
     .allowed_first = 104,
     .min = 0LL,
     .max = 4LL,
     .def = 0LL},
    {.id = 99,
     .access = Access::ReadWrite,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 109,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 100,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 111,
     .min = 0LL,
     .max = 16777215LL,
     .def = 0LL},
    {.id = 104,
     .access = Access::ReadWrite,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 111,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 105,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 111,
     .min = -32767LL,
     .max = 32768LL,
     .def = 0LL},
    {.id = 106,
     .access = Access::ReadWrite,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 111,
     .min = -10000LL,
     .max = 10000LL,
     .def = 0LL},
    {.id = 107,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 111,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 108,
     .access = Access::ReadWrite,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 111,
     .min = -4700LL,
     .max = 4700LL,
     .def = 0LL},
    {.id = 109,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 111,
     .min = 0LL,
     .max = 32767LL,
     .def = 50LL},
    {.id = 110,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 111,
     .min = 0LL,
     .max = 32767LL,
     .def = 100LL},
    {.id = 111,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 111,
     .min = 0LL,
     .max = 32767LL,
     .def = 50LL},
    {.id = 112,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 111,
     .min = 0LL,
     .max = 32767LL,
     .def = 100LL},
    {.id = 113,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 111,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 114,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 113,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 115,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 115,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 116,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 117,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 117,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 117,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 118,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 117,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 119,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 117,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 120,
     .access = Access::ReadWrite,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 117,
     .min = -4700LL,
     .max = 4700LL,
     .def = 0LL},
    {.id = 123,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 5,
     .allowed_first = 117,
     .min = 0LL,
     .max = 4LL,
     .def = 0LL},
    {.id = 124,
     .access = Access::ReadWrite,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 122,
     .min = -134217728LL,
     .max = 134217727LL,
     .def = 0LL},
    {.id = 125,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 122,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 126,
     .access = Access::ReadWrite,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 122,
     .min = -200000LL,
     .max = 200000LL,
     .def = 0LL},
    {.id = 127,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 122,
     .min = 0LL,
     .max = 32767LL,
     .def = 800LL},
    {.id = 128,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 122,
     .min = 0LL,
     .max = 32767LL,
     .def = 1LL},
    {.id = 129,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 122,
     .min = 0LL,
     .max = 3LL,
     .def = 2LL},
    {.id = 130,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 126,
     .min = 0LL,
     .max = 3LL,
     .def = 2LL},
    {.id = 131,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 130,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 132,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 130,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 133,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 130,
     .min = 1LL,
     .max = 2047LL,
     .def = 1LL},
    {.id = 134,
     .access = Access::ReadWrite,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 130,
     .min = 0LL,
     .max = 200000LL,
     .def = 0LL},
    {.id = 135,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 130,
     .min = 0LL,
     .max = 127LL,
     .def = 5LL},
    {.id = 136,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 130,
     .min = 0LL,
     .max = 2000000000LL,
     .def = 0LL},
    {.id = 137,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 130,
     .min = 0LL,
     .max = 134217727LL,
     .def = 2000LL},
    {.id = 138,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 130,
     .min = 0LL,
     .max = 65535LL,
     .def = 500LL},
    {.id = 139,
     .access = Access::ReadOnly,
     .coded = true,
     .allowed_count = 3,
     .allowed_first = 130,
     .min = 0LL,
     .max = 2LL,
     .def = 0LL},
    {.id = 142,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 5,
     .allowed_first = 133,
     .min = 0LL,
     .max = 4LL,
     .def = 0LL},
    {.id = 143,
     .access = Access::ReadWrite,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 138,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 144,
     .access = Access::ReadWrite,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 138,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 145,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 138,
     .min = 1024LL,
     .max = 65535LL,
     .def = 1024LL},
    {.id = 146,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 138,
     .min = 0LL,
     .max = 32767LL,
     .def = 5LL},
    {.id = 147,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 138,
     .min = 0LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 148,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 138,
     .min = 0LL,
     .max = 3LL,
     .def = 1LL},
    {.id = 149,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 142,
     .min = 0LL,
     .max = 3LL,
     .def = 1LL},
    {.id = 150,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 146,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 151,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 146,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 152,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 146,
     .min = 0LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 153,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 146,
     .min = 0LL,
     .max = 127LL,
     .def = 0LL},
    {.id = 154,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 146,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 155,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 146,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = -2147483648LL},
    {.id = 156,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 146,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 2147483647LL},
    {.id = 157,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 146,
     .min = 0LL,
     .max = 2000000000LL,
     .def = 0LL},
    {.id = 161,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 8,
     .allowed_first = 146,
     .min = 0LL,
     .max = 7LL,
     .def = 0LL},
    {.id = 162,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 16,
     .allowed_first = 154,
     .min = 0LL,
     .max = 15LL,
     .def = 0LL},
    {.id = 163,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 16,
     .allowed_first = 170,
     .min = 0LL,
     .max = 15LL,
     .def = 0LL},
    {.id = 164,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 8,
     .allowed_first = 186,
     .min = 0LL,
     .max = 7LL,
     .def = 0LL},
    {.id = 165,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 8,
     .allowed_first = 194,
     .min = 1LL,
     .max = 8LL,
     .def = 0LL},
    {.id = 166,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 202,
     .min = -134217728LL,
     .max = 134217727LL,
     .def = 0LL},
    {.id = 167,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 202,
     .min = -134217728LL,
     .max = 134217727LL,
     .def = 0LL},
    {.id = 168,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 202,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 169,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 202,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 170,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 202,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 174,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 202,
     .min = 0LL,
     .max = 16777215LL,
     .def = 1024LL},
    {.id = 175,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 202,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 176,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 204,
     .min = 1LL,
     .max = 255LL,
     .def = 1LL},
    {.id = 177,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 204,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 178,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 206,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 181,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 206,
     .min = 0LL,
     .max = 6375LL,
     .def = 0LL},
    {.id = 182,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 206,
     .min = 0LL,
     .max = 102LL,
     .def = 0LL},
    {.id = 183,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 206,
     .min = 1LL,
     .max = 16LL,
     .def = 1LL},
    {.id = 184,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 206,
     .min = 0LL,
     .max = 15LL,
     .def = 0LL},
    {.id = 185,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 206,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 186,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 206,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 187,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 206,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 188,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 206,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 189,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 3,
     .allowed_first = 206,
     .min = 0LL,
     .max = 2LL,
     .def = 0LL},
    {.id = 190,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 209,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 191,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 209,
     .min = 0LL,
     .max = 127LL,
     .def = 0LL},
    {.id = 192,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 209,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 193,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 209,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 194,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 3,
     .allowed_first = 209,
     .min = 0LL,
     .max = 2LL,
     .def = 0LL},
    {.id = 195,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 212,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 196,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 214,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 197,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 214,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 198,
     .access = Access::ReadWrite,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 216,
     .min = 0LL,
     .max = 255LL,
     .def = 0LL},
    {.id = 199,
     .access = Access::ReadWrite,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 216,
     .min = -128LL,
     .max = 127LL,
     .def = 0LL},
    {.id = 201,
     .access = Access::ReadWrite,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 216,
     .min = 0LL,
     .max = 4LL,
     .def = 0LL},
    {.id = 205,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 11,
     .allowed_first = 216,
     .min = 0LL,
     .max = 10LL,
     .def = 0LL},
    {.id = 206,
     .access = Access::ReadWrite,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 227,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 207,
     .access = Access::ReadWrite,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 229,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 208,
     .access = Access::ReadWrite,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 231,
     .min = 1LL,
     .max = 2000LL,
     .def = 1000LL},
    {.id = 209,
     .access = Access::ReadWrite,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 231,
     .min = 0LL,
     .max = 2147483647LL,
     .def = 2147483647LL},
    {.id = 212,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 231,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 213,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 233,
     .min = 50LL,
     .max = 1000LL,
     .def = 260LL},
    {.id = 214,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 233,
     .min = 0LL,
     .max = 50LL,
     .def = 5LL},
    {.id = 216,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 233,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 217,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 235,
     .min = 0LL,
     .max = 99LL,
     .def = 75LL},
    {.id = 218,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 235,
     .min = 0LL,
     .max = 99LL,
     .def = 11LL},
    {.id = 219,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 235,
     .min = 0LL,
     .max = 65535LL,
     .def = 80LL},
    {.id = 221,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 235,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 224,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 237,
     .min = 1000LL,
     .max = 60000LL,
     .def = 3000LL},
    {.id = 225,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 237,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 4294967295LL},
    {.id = 226,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 237,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 227,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 237,
     .min = 1000LL,
     .max = 60000LL,
     .def = 6000LL},
    {.id = 228,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 237,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 4294967295LL},
    {.id = 229,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 237,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 230,
     .access = Access::WriteOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 237,
     .min = 0LL,
     .max = 0LL,
     .def = 0LL},
    {.id = 231,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 237,
     .min = 0LL,
     .max = 65535LL,
     .def = 0LL},
    {.id = 233,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 237,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 234,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 239,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 235,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 241,
     .min = 0LL,
     .max = 255LL,
     .def = 0LL},
    {.id = 236,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 241,
     .min = 0LL,
     .max = 255LL,
     .def = 0LL},
    {.id = 237,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 241,
     .min = 0LL,
     .max = 255LL,
     .def = 0LL},
    {.id = 238,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 241,
     .min = 0LL,
     .max = 255LL,
     .def = 0LL},
    {.id = 239,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 241,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 240,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 243,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 241,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 245,
     .min = 0LL,
     .max = 255LL,
     .def = 255LL},
    {.id = 242,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 245,
     .min = 0LL,
     .max = 255LL,
     .def = 255LL},
    {.id = 243,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 245,
     .min = 0LL,
     .max = 255LL,
     .def = 255LL},
    {.id = 244,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 245,
     .min = 0LL,
     .max = 255LL,
     .def = 255LL},
    {.id = 245,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 16,
     .allowed_first = 245,
     .min = 0LL,
     .max = 15LL,
     .def = 4LL},
    {.id = 246,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 16,
     .allowed_first = 261,
     .min = 0LL,
     .max = 15LL,
     .def = 4LL},
    {.id = 247,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 16,
     .allowed_first = 277,
     .min = 0LL,
     .max = 15LL,
     .def = 4LL},
    {.id = 248,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 16,
     .allowed_first = 293,
     .min = 0LL,
     .max = 15LL,
     .def = 4LL},
    {.id = 249,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 8,
     .allowed_first = 309,
     .min = 0LL,
     .max = 7LL,
     .def = 7LL},
    {.id = 250,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 317,
     .min = 0LL,
     .max = 16LL,
     .def = 0LL},
    {.id = 251,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 317,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 252,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 319,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 253,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 321,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 254,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 323,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 255,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 325,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 256,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 327,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 257,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 329,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 258,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 16,
     .allowed_first = 331,
     .min = 0LL,
     .max = 15LL,
     .def = 0LL},
    {.id = 259,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 16,
     .allowed_first = 347,
     .min = 0LL,
     .max = 15LL,
     .def = 0LL},
    {.id = 260,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 16,
     .allowed_first = 363,
     .min = 0LL,
     .max = 15LL,
     .def = 0LL},
    {.id = 261,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 16,
     .allowed_first = 379,
     .min = 0LL,
     .max = 15LL,
     .def = 0LL},
    {.id = 262,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 8,
     .allowed_first = 395,
     .min = 0LL,
     .max = 7LL,
     .def = 2LL},
    {.id = 263,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 8,
     .allowed_first = 403,
     .min = 0LL,
     .max = 7LL,
     .def = 2LL},
    {.id = 264,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 8,
     .allowed_first = 411,
     .min = 0LL,
     .max = 7LL,
     .def = 2LL},
    {.id = 265,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 8,
     .allowed_first = 419,
     .min = 0LL,
     .max = 7LL,
     .def = 2LL},
    {.id = 266,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 8,
     .allowed_first = 427,
     .min = 0LL,
     .max = 7LL,
     .def = 6LL},
    {.id = 267,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 8,
     .allowed_first = 435,
     .min = 0LL,
     .max = 7LL,
     .def = 6LL},
    {.id = 268,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 8,
     .allowed_first = 443,
     .min = 0LL,
     .max = 7LL,
     .def = 6LL},
    {.id = 269,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 8,
     .allowed_first = 451,
     .min = 0LL,
     .max = 7LL,
     .def = 6LL},
    {.id = 270,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 459,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 271,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 461,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 272,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 463,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 273,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 465,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 274,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 467,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 275,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 469,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 276,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 471,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 277,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 473,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 278,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 475,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 279,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 477,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 280,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 479,
     .min = 0LL,
     .max = 3LL,
     .def = 1LL},
    {.id = 281,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 483,
     .min = 0LL,
     .max = 3LL,
     .def = 1LL},
    {.id = 282,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 8,
     .allowed_first = 487,
     .min = 0LL,
     .max = 7LL,
     .def = 1LL},
    {.id = 283,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 8,
     .allowed_first = 495,
     .min = 0LL,
     .max = 7LL,
     .def = 1LL},
    {.id = 286,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 503,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 287,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 4,
     .allowed_first = 505,
     .min = 0LL,
     .max = 3LL,
     .def = 0LL},
    {.id = 288,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 255LL,
     .def = 5LL},
    {.id = 289,
     .access = Access::ReadWriteClear,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 290,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 1000LL,
     .def = 0LL},
    {.id = 291,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 1000LL,
     .def = 480LL},
    {.id = 292,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 1000LL,
     .def = 50LL},
    {.id = 293,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 65535LL,
     .def = 0LL},
    {.id = 294,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 65535LL,
     .def = 65535LL},
    {.id = 295,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 65535LL,
     .def = 65535LL},
    {.id = 296,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 65535LL,
     .def = 0LL},
    {.id = 297,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 65535LL,
     .def = 65535LL},
    {.id = 298,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 65535LL,
     .def = 65535LL},
    {.id = 299,
     .access = Access::ReadWriteClear,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 300,
     .access = Access::ReadWriteClear,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 301,
     .access = Access::ReadWriteClear,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 304,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 305,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 306,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 307,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 308,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 310,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = 0LL,
     .max = 32767LL,
     .def = 32767LL},
    {.id = 311,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 312,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 313,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 314,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 315,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 509,
     .min = -32768LL,
     .max = 32767LL,
     .def = 0LL},
    {.id = 318,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 509,
     .min = 0LL,
     .max = 1LL,
     .def = 0LL},
    {.id = 319,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 511,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 320,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 511,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 321,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 511,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 1048576LL},
    {.id = 322,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 511,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 323,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 511,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 0LL},
    {.id = 324,
     .access = Access::ReadWriteStored,
     .coded = true,
     .allowed_count = 2,
     .allowed_first = 511,
     .min = 0LL,
     .max = 1LL,
     .def = 1LL},
    {.id = 325,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 513,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 1849195LL},
    {.id = 326,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 513,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 15961938LL},
    {.id = 327,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 513,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 3665LL},
    {.id = 328,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 513,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 7329LL},
    {.id = 329,
     .access = Access::ReadWriteStored,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 513,
     .min = -2147483648LL,
     .max = 2147483647LL,
     .def = 3665LL},
    {.id = 330,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 513,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 331,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 513,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 332,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 513,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 333,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 513,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
    {.id = 334,
     .access = Access::ReadOnly,
     .coded = false,
     .allowed_count = 0,
     .allowed_first = 513,
     .min = 0LL,
     .max = 4294967295LL,
     .def = 0LL},
}};

/**
 * @brief The table row of an axis parameter.
 * @param id The parameter number.
 * @return The row, or `nullptr` when the manual lists no such parameter.
 */
[[nodiscard]] constexpr const ParamInfo* FindParam(uint32_t id) noexcept {
  std::size_t lo = 0U;
  std::size_t hi = kParams.size();
  while (lo < hi) {
    const std::size_t mid = lo + ((hi - lo) / 2U);
    if (kParams[mid].id < id) {
      lo = mid + 1U;
    } else {
      hi = mid;
    }
  }
  if ((lo < kParams.size()) && (kParams[lo].id == id)) {
    return &kParams[lo];
  }
  return nullptr;
}

/// Value constants of the coded parameters (the enumerations of Table 57), one namespace per
/// parameter.
namespace value {
/// Values of MOTOR_TYPE.
namespace motor_type {
/// NO_MOTOR.
inline constexpr int32_t NO_MOTOR = 0;
/// DC_MOTOR.
inline constexpr int32_t DC_MOTOR = 1;
/// STEPPER_MOTOR.
inline constexpr int32_t STEPPER_MOTOR = 2;
/// BLDC_MOTOR.
inline constexpr int32_t BLDC_MOTOR = 3;
} // namespace motor_type
/// Values of COMMUTATION_MODE.
namespace commutation_mode {
/// SYSTEM_OFF.
inline constexpr int32_t SYSTEM_OFF = 0;
/// SYSTEM_OFF_LOW_SIDE_FETS_ON.
inline constexpr int32_t SYSTEM_OFF_LOW_SIDE_FETS_ON = 1;
/// SYSTEM_OFF_HIGH_SIDE_FETS_ON.
inline constexpr int32_t SYSTEM_OFF_HIGH_SIDE_FETS_ON = 2;
/// FOC_OPENLOOP_VOLTAGE_MODE.
inline constexpr int32_t FOC_OPENLOOP_VOLTAGE_MODE = 3;
/// FOC_OPENLOOP_CURRENT_MODE.
inline constexpr int32_t FOC_OPENLOOP_CURRENT_MODE = 4;
/// FOC_ABN.
inline constexpr int32_t FOC_ABN = 5;
/// FOC_HALL_SENSOR.
inline constexpr int32_t FOC_HALL_SENSOR = 6;
/// RESERVED.
inline constexpr int32_t RESERVED = 7;
/// FOC_SPI_ENC.
inline constexpr int32_t FOC_SPI_ENC = 8;
} // namespace commutation_mode
/// Values of PWM_SWITCHING_SCHEME.
namespace pwm_switching_scheme {
/// STANDARD.
inline constexpr int32_t STANDARD = 0;
/// SVPWM.
inline constexpr int32_t SVPWM = 1;
/// FLAT_BOTTOM.
inline constexpr int32_t FLAT_BOTTOM = 2;
} // namespace pwm_switching_scheme
/// Values of ADC_SHUNT_TYPE.
namespace adc_shunt_type {
/// INLINE_UVW.
inline constexpr int32_t INLINE_UVW = 0;
/// INLINE_VW.
inline constexpr int32_t INLINE_VW = 1;
/// INLINE_UW.
inline constexpr int32_t INLINE_UW = 2;
/// INLINE_UV.
inline constexpr int32_t INLINE_UV = 3;
/// BOTTOM_SHUNTS.
inline constexpr int32_t BOTTOM_SHUNTS = 4;
} // namespace adc_shunt_type
/// Values of CSA_GAIN_ADC_I0_TO_ADC_I2.
namespace csa_gain_adc_i0_to_adc_i2 {
/// GAIN_5X.
inline constexpr int32_t GAIN_5X = 0;
/// GAIN_10X.
inline constexpr int32_t GAIN_10X = 1;
/// GAIN_20X.
inline constexpr int32_t GAIN_20X = 2;
/// GAIN_40X.
inline constexpr int32_t GAIN_40X = 3;
/// GAIN_1X_BYPASS_CSA.
inline constexpr int32_t GAIN_1X_BYPASS_CSA = 4;
} // namespace csa_gain_adc_i0_to_adc_i2
/// Values of CSA_GAIN_ADC_I3.
namespace csa_gain_adc_i3 {
/// GAIN_5X.
inline constexpr int32_t GAIN_5X = 0;
/// GAIN_10X.
inline constexpr int32_t GAIN_10X = 1;
/// GAIN_20X.
inline constexpr int32_t GAIN_20X = 2;
/// GAIN_40X.
inline constexpr int32_t GAIN_40X = 3;
/// GAIN_1X_BYPASS_CSA.
inline constexpr int32_t GAIN_1X_BYPASS_CSA = 4;
} // namespace csa_gain_adc_i3
/// Values of CSA_FILTER_ADC_I0_TO_ADC_I2.
namespace csa_filter_adc_i0_to_adc_i2 {
/// T_0_55_MICROSEC.
inline constexpr int32_t T_0_55_MICROSEC = 0;
/// T_0_75_MICROSEC.
inline constexpr int32_t T_0_75_MICROSEC = 1;
/// T_1_0_MICROSEC.
inline constexpr int32_t T_1_0_MICROSEC = 2;
/// T_1_35_MICROSEC.
inline constexpr int32_t T_1_35_MICROSEC = 3;
} // namespace csa_filter_adc_i0_to_adc_i2
/// Values of CSA_FILTER_ADC_I3.
namespace csa_filter_adc_i3 {
/// T_0_55_MICROSEC.
inline constexpr int32_t T_0_55_MICROSEC = 0;
/// T_0_75_MICROSEC.
inline constexpr int32_t T_0_75_MICROSEC = 1;
/// T_1_0_MICROSEC.
inline constexpr int32_t T_1_0_MICROSEC = 2;
/// T_1_35_MICROSEC.
inline constexpr int32_t T_1_35_MICROSEC = 3;
} // namespace csa_filter_adc_i3
/// Values of PHASE_UX1_ADC_MAPPING.
namespace phase_ux1_adc_mapping {
/// ADC_I0.
inline constexpr int32_t ADC_I0 = 0;
/// ADC_I1.
inline constexpr int32_t ADC_I1 = 1;
/// ADC_I2.
inline constexpr int32_t ADC_I2 = 2;
/// ADC_I3.
inline constexpr int32_t ADC_I3 = 3;
} // namespace phase_ux1_adc_mapping
/// Values of PHASE_VX2_ADC_MAPPING.
namespace phase_vx2_adc_mapping {
/// ADC_I0.
inline constexpr int32_t ADC_I0 = 0;
/// ADC_I1.
inline constexpr int32_t ADC_I1 = 1;
/// ADC_I2.
inline constexpr int32_t ADC_I2 = 2;
/// ADC_I3.
inline constexpr int32_t ADC_I3 = 3;
} // namespace phase_vx2_adc_mapping
/// Values of PHASE_WY1_ADC_MAPPING.
namespace phase_wy1_adc_mapping {
/// ADC_I0.
inline constexpr int32_t ADC_I0 = 0;
/// ADC_I1.
inline constexpr int32_t ADC_I1 = 1;
/// ADC_I2.
inline constexpr int32_t ADC_I2 = 2;
/// ADC_I3.
inline constexpr int32_t ADC_I3 = 3;
} // namespace phase_wy1_adc_mapping
/// Values of PHASE_Y2_ADC_MAPPING.
namespace phase_y2_adc_mapping {
/// ADC_I0.
inline constexpr int32_t ADC_I0 = 0;
/// ADC_I1.
inline constexpr int32_t ADC_I1 = 1;
/// ADC_I2.
inline constexpr int32_t ADC_I2 = 2;
/// ADC_I3.
inline constexpr int32_t ADC_I3 = 3;
} // namespace phase_y2_adc_mapping
/// Values of ACCELERATION_FF_SHIFT.
namespace acceleration_ff_shift {
/// NO_SHIFT.
inline constexpr int32_t NO_SHIFT = 0;
/// SHIFT_4_BIT.
inline constexpr int32_t SHIFT_4_BIT = 1;
/// SHIFT_8_BIT.
inline constexpr int32_t SHIFT_8_BIT = 2;
/// SHIFT_12_BIT.
inline constexpr int32_t SHIFT_12_BIT = 3;
/// SHIFT_16_BIT.
inline constexpr int32_t SHIFT_16_BIT = 4;
/// SHIFT_20_BIT.
inline constexpr int32_t SHIFT_20_BIT = 5;
/// SHIFT_24_BIT.
inline constexpr int32_t SHIFT_24_BIT = 6;
} // namespace acceleration_ff_shift
/// Values of HALL_SECTOR_OFFSET.
namespace hall_sector_offset {
/// DEG_0.
inline constexpr int32_t DEG_0 = 0;
/// DEG_60.
inline constexpr int32_t DEG_60 = 1;
/// DEG_120.
inline constexpr int32_t DEG_120 = 2;
/// DEG_180.
inline constexpr int32_t DEG_180 = 3;
/// DEG_240.
inline constexpr int32_t DEG_240 = 4;
/// DEG_300.
inline constexpr int32_t DEG_300 = 5;
} // namespace hall_sector_offset
/// Values of ABN_1_INIT_METHOD.
namespace abn_1_init_method {
/// FORCED_PHI_E_ZERO_WITH_ACTIVE_SWING.
inline constexpr int32_t FORCED_PHI_E_ZERO_WITH_ACTIVE_SWING = 0;
/// FORCED_PHI_E_90_ZERO.
inline constexpr int32_t FORCED_PHI_E_90_ZERO = 1;
/// USE_HALL.
inline constexpr int32_t USE_HALL = 2;
/// USE_N_CHANNEL_OFFSET.
inline constexpr int32_t USE_N_CHANNEL_OFFSET = 3;
} // namespace abn_1_init_method
/// Values of ABN_1_INIT_STATE.
namespace abn_1_init_state {
/// IDLE.
inline constexpr int32_t IDLE = 0;
/// BUSY.
inline constexpr int32_t BUSY = 1;
/// WAIT.
inline constexpr int32_t WAIT = 2;
/// DONE.
inline constexpr int32_t DONE = 3;
} // namespace abn_1_init_state
/// Values of ABN_1_N_CHANNEL_FILTERING.
namespace abn_1_n_channel_filtering {
/// FILTERING_OFF.
inline constexpr int32_t FILTERING_OFF = 0;
/// N_EVENT_ON_A_HIGH_B_HIGH.
inline constexpr int32_t N_EVENT_ON_A_HIGH_B_HIGH = 1;
/// N_EVENT_ON_A_HIGH_B_LOW.
inline constexpr int32_t N_EVENT_ON_A_HIGH_B_LOW = 2;
/// N_EVENT_ON_A_LOW_B_HIGH.
inline constexpr int32_t N_EVENT_ON_A_LOW_B_HIGH = 3;
/// N_EVENT_ON_A_LOW_B_LOW.
inline constexpr int32_t N_EVENT_ON_A_LOW_B_LOW = 4;
} // namespace abn_1_n_channel_filtering
/// Values of CURRENT_NORM_P.
namespace current_norm_p {
/// SHIFT_8_BIT.
inline constexpr int32_t SHIFT_8_BIT = 0;
/// SHIFT_16_BIT.
inline constexpr int32_t SHIFT_16_BIT = 1;
} // namespace current_norm_p
/// Values of CURRENT_NORM_I.
namespace current_norm_i {
/// SHIFT_8_BIT.
inline constexpr int32_t SHIFT_8_BIT = 0;
/// SHIFT_16_BIT.
inline constexpr int32_t SHIFT_16_BIT = 1;
} // namespace current_norm_i
/// Values of VELOCITY_SENSOR_SELECTION.
namespace velocity_sensor_selection {
/// SAME_AS_COMMUTATION.
inline constexpr int32_t SAME_AS_COMMUTATION = 0;
/// DIGITAL_HALL.
inline constexpr int32_t DIGITAL_HALL = 1;
/// ABN1_ENCODER.
inline constexpr int32_t ABN1_ENCODER = 2;
/// ABN2_ENCODER.
inline constexpr int32_t ABN2_ENCODER = 3;
/// SPI_ENCODER.
inline constexpr int32_t SPI_ENCODER = 4;
} // namespace velocity_sensor_selection
/// Values of VELOCITY_NORM_P.
namespace velocity_norm_p {
/// NO_SHIFT.
inline constexpr int32_t NO_SHIFT = 0;
/// SHIFT_8_BIT.
inline constexpr int32_t SHIFT_8_BIT = 1;
/// SHIFT_16_BIT.
inline constexpr int32_t SHIFT_16_BIT = 2;
/// SHIFT_24_BIT.
inline constexpr int32_t SHIFT_24_BIT = 3;
} // namespace velocity_norm_p
/// Values of VELOCITY_NORM_I.
namespace velocity_norm_i {
/// SHIFT_8_BIT.
inline constexpr int32_t SHIFT_8_BIT = 0;
/// SHIFT_16_BIT.
inline constexpr int32_t SHIFT_16_BIT = 1;
/// SHIFT_24_BIT.
inline constexpr int32_t SHIFT_24_BIT = 2;
/// SHIFT_32_BIT.
inline constexpr int32_t SHIFT_32_BIT = 3;
} // namespace velocity_norm_i
/// Values of VELOCITY_METER_MODE.
namespace velocity_meter_mode {
/// PERIOD_METER.
inline constexpr int32_t PERIOD_METER = 0;
/// FREQUENCY_METER.
inline constexpr int32_t FREQUENCY_METER = 1;
/// SOFTWARE_METER.
inline constexpr int32_t SOFTWARE_METER = 2;
} // namespace velocity_meter_mode
/// Values of POSITION_SENSOR_SELECTION.
namespace position_sensor_selection {
/// SAME_AS_COMMUTATION.
inline constexpr int32_t SAME_AS_COMMUTATION = 0;
/// DIGITAL_HALL.
inline constexpr int32_t DIGITAL_HALL = 1;
/// ABN1_ENCODER.
inline constexpr int32_t ABN1_ENCODER = 2;
/// ABN2_ENCODER.
inline constexpr int32_t ABN2_ENCODER = 3;
/// SPI_ENCODER.
inline constexpr int32_t SPI_ENCODER = 4;
} // namespace position_sensor_selection
/// Values of POSITION_NORM_P.
namespace position_norm_p {
/// NO_SHIFT.
inline constexpr int32_t NO_SHIFT = 0;
/// SHIFT_8_BIT.
inline constexpr int32_t SHIFT_8_BIT = 1;
/// SHIFT_16_BIT.
inline constexpr int32_t SHIFT_16_BIT = 2;
/// SHIFT_24_BIT.
inline constexpr int32_t SHIFT_24_BIT = 3;
} // namespace position_norm_p
/// Values of POSITION_NORM_I.
namespace position_norm_i {
/// SHIFT_8_BIT.
inline constexpr int32_t SHIFT_8_BIT = 0;
/// SHIFT_16_BIT.
inline constexpr int32_t SHIFT_16_BIT = 1;
/// SHIFT_24_BIT.
inline constexpr int32_t SHIFT_24_BIT = 2;
/// SHIFT_32_BIT.
inline constexpr int32_t SHIFT_32_BIT = 3;
} // namespace position_norm_i
/// Values of REFERENCE_SWITCH_ENABLE.
namespace reference_switch_enable {
/// NO_STOP_ON_SWITCH_TRIGGERED.
inline constexpr int32_t NO_STOP_ON_SWITCH_TRIGGERED = 0;
/// STOP_ON_L.
inline constexpr int32_t STOP_ON_L = 1;
/// STOP_ON_R.
inline constexpr int32_t STOP_ON_R = 2;
/// STOP_ON_R_AND_L.
inline constexpr int32_t STOP_ON_R_AND_L = 3;
/// STOP_ON_H.
inline constexpr int32_t STOP_ON_H = 4;
/// STOP_ON_H_AND_L.
inline constexpr int32_t STOP_ON_H_AND_L = 5;
/// STOP_ON_H_AND_R.
inline constexpr int32_t STOP_ON_H_AND_R = 6;
/// STOP_ON_H_R_AND_L.
inline constexpr int32_t STOP_ON_H_R_AND_L = 7;
} // namespace reference_switch_enable
/// Values of REFERENCE_SWITCH_POLARITY_AND_SWAP.
namespace reference_switch_polarity_and_swap {
/// NOT_SWAPPED_NOT_INVERTED.
inline constexpr int32_t NOT_SWAPPED_NOT_INVERTED = 0;
/// L_INVERTED.
inline constexpr int32_t L_INVERTED = 1;
/// R_INVERTED.
inline constexpr int32_t R_INVERTED = 2;
/// R_AND_L_INVERTED.
inline constexpr int32_t R_AND_L_INVERTED = 3;
/// H_INVERTED.
inline constexpr int32_t H_INVERTED = 4;
/// H_AND_L_INVERTED.
inline constexpr int32_t H_AND_L_INVERTED = 5;
/// H_AND_R_INVERTED.
inline constexpr int32_t H_AND_R_INVERTED = 6;
/// H_R_AND_L_INVERTED.
inline constexpr int32_t H_R_AND_L_INVERTED = 7;
/// L_R_SWAPPED_L_INVERTED.
inline constexpr int32_t L_R_SWAPPED_L_INVERTED = 8;
/// L_R_SWAPPED_R_INVERTED.
inline constexpr int32_t L_R_SWAPPED_R_INVERTED = 9;
/// L_R_SWAPPED_R_AND_L_INVERTED.
inline constexpr int32_t L_R_SWAPPED_R_AND_L_INVERTED = 10;
/// L_R_SWAPPED_H_INVERTED.
inline constexpr int32_t L_R_SWAPPED_H_INVERTED = 11;
/// L_R_SWAPPED_H_AND_L_INVERTED.
inline constexpr int32_t L_R_SWAPPED_H_AND_L_INVERTED = 12;
/// L_R_SWAPPED.
inline constexpr int32_t L_R_SWAPPED = 13;
/// L_R_SWAPPED_H_AND_R_INVERTED.
inline constexpr int32_t L_R_SWAPPED_H_AND_R_INVERTED = 14;
/// L_R_SWAPPED_H_R_AND_L_INVERTED.
inline constexpr int32_t L_R_SWAPPED_H_R_AND_L_INVERTED = 15;
} // namespace reference_switch_polarity_and_swap
/// Values of REFERENCE_SWITCH_LATCH_SETTINGS.
namespace reference_switch_latch_settings {
/// NO_TRIGGER.
inline constexpr int32_t NO_TRIGGER = 0;
/// L_R_RISING_EDGE.
inline constexpr int32_t L_R_RISING_EDGE = 1;
/// L_R_FALLING_EDGE.
inline constexpr int32_t L_R_FALLING_EDGE = 2;
/// L_R_BOTH_EDGES.
inline constexpr int32_t L_R_BOTH_EDGES = 3;
/// H_RISING_EDGE.
inline constexpr int32_t H_RISING_EDGE = 4;
/// H_L_R_RISING_EDGE.
inline constexpr int32_t H_L_R_RISING_EDGE = 5;
/// H_RISING_L_R_FALLING_EDGE.
inline constexpr int32_t H_RISING_L_R_FALLING_EDGE = 6;
/// H_RISING_L_R_BOTH_EDGES.
inline constexpr int32_t H_RISING_L_R_BOTH_EDGES = 7;
/// H_FALLING_EDGE.
inline constexpr int32_t H_FALLING_EDGE = 8;
/// H_FALLING_L_R_RISING_EDGE.
inline constexpr int32_t H_FALLING_L_R_RISING_EDGE = 9;
/// H_L_R_FALLING_EDGE.
inline constexpr int32_t H_L_R_FALLING_EDGE = 10;
/// H_FALLING_L_R_BOTH_EDGES.
inline constexpr int32_t H_FALLING_L_R_BOTH_EDGES = 11;
/// H_BOTH_EDGES.
inline constexpr int32_t H_BOTH_EDGES = 12;
/// H_BOTH_L_R_RISING_EDGE.
inline constexpr int32_t H_BOTH_L_R_RISING_EDGE = 13;
/// H_BOTH_L_R_FALLING_EDGE.
inline constexpr int32_t H_BOTH_L_R_FALLING_EDGE = 14;
/// H_L_R_BOTH_EDGES.
inline constexpr int32_t H_L_R_BOTH_EDGES = 15;
} // namespace reference_switch_latch_settings
/// Values of EVENT_STOP_SETTINGS.
namespace event_stop_settings {
/// DO_HARD_STOP.
inline constexpr int32_t DO_HARD_STOP = 0;
/// DO_SOFT_STOP.
inline constexpr int32_t DO_SOFT_STOP = 1;
/// STOP_ON_POS_DEVIATION.
inline constexpr int32_t STOP_ON_POS_DEVIATION = 2;
/// STOP_ON_POS_DEVIATION_SOFT_STOP.
inline constexpr int32_t STOP_ON_POS_DEVIATION_SOFT_STOP = 3;
/// STOP_ON_VEL_DEVIATION.
inline constexpr int32_t STOP_ON_VEL_DEVIATION = 4;
/// STOP_ON_VEL_DEVIATION_SOFT_STOP.
inline constexpr int32_t STOP_ON_VEL_DEVIATION_SOFT_STOP = 5;
/// STOP_ON_POS_VEL_DEVIATION.
inline constexpr int32_t STOP_ON_POS_VEL_DEVIATION = 6;
/// STOP_ON_POS_VEL_DEVIATION_SOFT_STOP.
inline constexpr int32_t STOP_ON_POS_VEL_DEVIATION_SOFT_STOP = 7;
} // namespace event_stop_settings
/// Values of REFERENCE_SWITCH_SEARCH_MODE.
namespace reference_switch_search_mode {
/// LEFT_SWITCH.
inline constexpr int32_t LEFT_SWITCH = 1;
/// RIGHT_SWITCH_LEFT_SWITCH.
inline constexpr int32_t RIGHT_SWITCH_LEFT_SWITCH = 2;
/// RIGHT_SWITCH_LEFT_SWITCH_BOTH_SIDES.
inline constexpr int32_t RIGHT_SWITCH_LEFT_SWITCH_BOTH_SIDES = 3;
/// LEFT_SWITCH_BOTH_SIDES.
inline constexpr int32_t LEFT_SWITCH_BOTH_SIDES = 4;
/// HOME_SWITCH_NEG_DIR_LEFT_END_SWITCH.
inline constexpr int32_t HOME_SWITCH_NEG_DIR_LEFT_END_SWITCH = 5;
/// HOME_SWITCH_POS_DIR_RIGHT_END_SWITCH.
inline constexpr int32_t HOME_SWITCH_POS_DIR_RIGHT_END_SWITCH = 6;
/// HOME_SWITCH_NEG_DIR_IGNORE_END_SWITCH.
inline constexpr int32_t HOME_SWITCH_NEG_DIR_IGNORE_END_SWITCH = 7;
/// HOME_SWITCH_POS_DIR_IGNORE_END_SWITCH.
inline constexpr int32_t HOME_SWITCH_POS_DIR_IGNORE_END_SWITCH = 8;
} // namespace reference_switch_search_mode
/// Values of SPI_ENCODER_TRANSFER.
namespace spi_encoder_transfer {
/// OFF.
inline constexpr int32_t OFF = 0;
/// TRIGGER_SINGLE_TRANSFER.
inline constexpr int32_t TRIGGER_SINGLE_TRANSFER = 1;
/// CONTINUOUS_POSITION_COUNTER_READ.
inline constexpr int32_t CONTINUOUS_POSITION_COUNTER_READ = 2;
} // namespace spi_encoder_transfer
/// Values of SPI_ENCODER_INITIALIZATION_METHOD.
namespace spi_encoder_initialization_method {
/// FORCED_PHI_E_ZERO_WITH_ACTIVE_SWING.
inline constexpr int32_t FORCED_PHI_E_ZERO_WITH_ACTIVE_SWING = 0;
/// FORCED_PHI_E_90_ZERO.
inline constexpr int32_t FORCED_PHI_E_90_ZERO = 1;
/// USE_OFFSET.
inline constexpr int32_t USE_OFFSET = 2;
} // namespace spi_encoder_initialization_method
/// Values of STEP_DIR_STEP_DIVIDER_SHIFT.
namespace step_dir_step_divider_shift {
/// STEP_MODE_FULL.
inline constexpr int32_t STEP_MODE_FULL = 0;
/// STEP_MODE_HALF.
inline constexpr int32_t STEP_MODE_HALF = 1;
/// STEP_MODE_QUARTER.
inline constexpr int32_t STEP_MODE_QUARTER = 2;
/// STEP_MODE_1_8TH.
inline constexpr int32_t STEP_MODE_1_8TH = 3;
/// STEP_MODE_1_16TH.
inline constexpr int32_t STEP_MODE_1_16TH = 4;
/// STEP_MODE_1_32ND.
inline constexpr int32_t STEP_MODE_1_32ND = 5;
/// STEP_MODE_1_64TH.
inline constexpr int32_t STEP_MODE_1_64TH = 6;
/// STEP_MODE_1_128TH.
inline constexpr int32_t STEP_MODE_1_128TH = 7;
/// STEP_MODE_1_256TH.
inline constexpr int32_t STEP_MODE_1_256TH = 8;
/// STEP_MODE_1_512TH.
inline constexpr int32_t STEP_MODE_1_512TH = 9;
/// STEP_MODE_1_1024TH.
inline constexpr int32_t STEP_MODE_1_1024TH = 10;
} // namespace step_dir_step_divider_shift
/// Values of UVW_SINK_CURRENT.
namespace uvw_sink_current {
/// CUR_50_MILLIAMP.
inline constexpr int32_t CUR_50_MILLIAMP = 0;
/// CUR_100_MILLIAMP.
inline constexpr int32_t CUR_100_MILLIAMP = 1;
/// CUR_160_MILLIAMP.
inline constexpr int32_t CUR_160_MILLIAMP = 2;
/// CUR_210_MILLIAMP.
inline constexpr int32_t CUR_210_MILLIAMP = 3;
/// CUR_270_MILLIAMP.
inline constexpr int32_t CUR_270_MILLIAMP = 4;
/// CUR_320_MILLIAMP.
inline constexpr int32_t CUR_320_MILLIAMP = 5;
/// CUR_380_MILLIAMP.
inline constexpr int32_t CUR_380_MILLIAMP = 6;
/// CUR_430_MILLIAMP.
inline constexpr int32_t CUR_430_MILLIAMP = 7;
/// CUR_580_MILLIAMP.
inline constexpr int32_t CUR_580_MILLIAMP = 8;
/// CUR_720_MILLIAMP.
inline constexpr int32_t CUR_720_MILLIAMP = 9;
/// CUR_860_MILLIAMP.
inline constexpr int32_t CUR_860_MILLIAMP = 10;
/// CUR_1000_MILLIAMP.
inline constexpr int32_t CUR_1000_MILLIAMP = 11;
/// CUR_1250_MILLIAMP.
inline constexpr int32_t CUR_1250_MILLIAMP = 12;
/// CUR_1510_MILLIAMP.
inline constexpr int32_t CUR_1510_MILLIAMP = 13;
/// CUR_1770_MILLIAMP.
inline constexpr int32_t CUR_1770_MILLIAMP = 14;
/// CUR_2000_MILLIAMP.
inline constexpr int32_t CUR_2000_MILLIAMP = 15;
} // namespace uvw_sink_current
/// Values of UVW_SOURCE_CURRENT.
namespace uvw_source_current {
/// CUR_25_MILLIAMP.
inline constexpr int32_t CUR_25_MILLIAMP = 0;
/// CUR_50_MILLIAMP.
inline constexpr int32_t CUR_50_MILLIAMP = 1;
/// CUR_80_MILLIAMP.
inline constexpr int32_t CUR_80_MILLIAMP = 2;
/// CUR_105_MILLIAMP.
inline constexpr int32_t CUR_105_MILLIAMP = 3;
/// CUR_135_MILLIAMP.
inline constexpr int32_t CUR_135_MILLIAMP = 4;
/// CUR_160_MILLIAMP.
inline constexpr int32_t CUR_160_MILLIAMP = 5;
/// CUR_190_MILLIAMP.
inline constexpr int32_t CUR_190_MILLIAMP = 6;
/// CUR_215_MILLIAMP.
inline constexpr int32_t CUR_215_MILLIAMP = 7;
/// CUR_290_MILLIAMP.
inline constexpr int32_t CUR_290_MILLIAMP = 8;
/// CUR_360_MILLIAMP.
inline constexpr int32_t CUR_360_MILLIAMP = 9;
/// CUR_430_MILLIAMP.
inline constexpr int32_t CUR_430_MILLIAMP = 10;
/// CUR_500_MILLIAMP.
inline constexpr int32_t CUR_500_MILLIAMP = 11;
/// CUR_625_MILLIAMP.
inline constexpr int32_t CUR_625_MILLIAMP = 12;
/// CUR_755_MILLIAMP.
inline constexpr int32_t CUR_755_MILLIAMP = 13;
/// CUR_855_MILLIAMP.
inline constexpr int32_t CUR_855_MILLIAMP = 14;
/// CUR_1000_MILLIAMP.
inline constexpr int32_t CUR_1000_MILLIAMP = 15;
} // namespace uvw_source_current
/// Values of Y2_SINK_CURRENT.
namespace y2_sink_current {
/// CUR_50_MILLIAMP.
inline constexpr int32_t CUR_50_MILLIAMP = 0;
/// CUR_100_MILLIAMP.
inline constexpr int32_t CUR_100_MILLIAMP = 1;
/// CUR_160_MILLIAMP.
inline constexpr int32_t CUR_160_MILLIAMP = 2;
/// CUR_210_MILLIAMP.
inline constexpr int32_t CUR_210_MILLIAMP = 3;
/// CUR_270_MILLIAMP.
inline constexpr int32_t CUR_270_MILLIAMP = 4;
/// CUR_320_MILLIAMP.
inline constexpr int32_t CUR_320_MILLIAMP = 5;
/// CUR_380_MILLIAMP.
inline constexpr int32_t CUR_380_MILLIAMP = 6;
/// CUR_430_MILLIAMP.
inline constexpr int32_t CUR_430_MILLIAMP = 7;
/// CUR_580_MILLIAMP.
inline constexpr int32_t CUR_580_MILLIAMP = 8;
/// CUR_720_MILLIAMP.
inline constexpr int32_t CUR_720_MILLIAMP = 9;
/// CUR_860_MILLIAMP.
inline constexpr int32_t CUR_860_MILLIAMP = 10;
/// CUR_1000_MILLIAMP.
inline constexpr int32_t CUR_1000_MILLIAMP = 11;
/// CUR_1250_MILLIAMP.
inline constexpr int32_t CUR_1250_MILLIAMP = 12;
/// CUR_1510_MILLIAMP.
inline constexpr int32_t CUR_1510_MILLIAMP = 13;
/// CUR_1770_MILLIAMP.
inline constexpr int32_t CUR_1770_MILLIAMP = 14;
/// CUR_2000_MILLIAMP.
inline constexpr int32_t CUR_2000_MILLIAMP = 15;
} // namespace y2_sink_current
/// Values of Y2_SOURCE_CURRENT.
namespace y2_source_current {
/// CUR_25_MILLIAMP.
inline constexpr int32_t CUR_25_MILLIAMP = 0;
/// CUR_50_MILLIAMP.
inline constexpr int32_t CUR_50_MILLIAMP = 1;
/// CUR_80_MILLIAMP.
inline constexpr int32_t CUR_80_MILLIAMP = 2;
/// CUR_105_MILLIAMP.
inline constexpr int32_t CUR_105_MILLIAMP = 3;
/// CUR_135_MILLIAMP.
inline constexpr int32_t CUR_135_MILLIAMP = 4;
/// CUR_160_MILLIAMP.
inline constexpr int32_t CUR_160_MILLIAMP = 5;
/// CUR_190_MILLIAMP.
inline constexpr int32_t CUR_190_MILLIAMP = 6;
/// CUR_215_MILLIAMP.
inline constexpr int32_t CUR_215_MILLIAMP = 7;
/// CUR_290_MILLIAMP.
inline constexpr int32_t CUR_290_MILLIAMP = 8;
/// CUR_360_MILLIAMP.
inline constexpr int32_t CUR_360_MILLIAMP = 9;
/// CUR_430_MILLIAMP.
inline constexpr int32_t CUR_430_MILLIAMP = 10;
/// CUR_500_MILLIAMP.
inline constexpr int32_t CUR_500_MILLIAMP = 11;
/// CUR_625_MILLIAMP.
inline constexpr int32_t CUR_625_MILLIAMP = 12;
/// CUR_755_MILLIAMP.
inline constexpr int32_t CUR_755_MILLIAMP = 13;
/// CUR_855_MILLIAMP.
inline constexpr int32_t CUR_855_MILLIAMP = 14;
/// CUR_1000_MILLIAMP.
inline constexpr int32_t CUR_1000_MILLIAMP = 15;
} // namespace y2_source_current
/// Values of BOOTSTRAP_CURRENT_LIMIT.
namespace bootstrap_current_limit {
/// CUR_45_MILLIAMP.
inline constexpr int32_t CUR_45_MILLIAMP = 0;
/// CUR_91_MILLIAMP.
inline constexpr int32_t CUR_91_MILLIAMP = 1;
/// CUR_141_MILLIAMP.
inline constexpr int32_t CUR_141_MILLIAMP = 2;
/// CUR_191_MILLIAMP.
inline constexpr int32_t CUR_191_MILLIAMP = 3;
/// CUR_267_MILLIAMP.
inline constexpr int32_t CUR_267_MILLIAMP = 4;
/// CUR_292_MILLIAMP.
inline constexpr int32_t CUR_292_MILLIAMP = 5;
/// CUR_341_MILLIAMP.
inline constexpr int32_t CUR_341_MILLIAMP = 6;
/// CUR_391_MILLIAMP.
inline constexpr int32_t CUR_391_MILLIAMP = 7;
} // namespace bootstrap_current_limit
/// Values of OVERCURRENT_PROTECTION_UVW_LOW_SIDE_THRESHOLD.
namespace overcurrent_protection_uvw_low_side_threshold {
/// V_80_OR_63_MILLIVOLT.
inline constexpr int32_t V_80_OR_63_MILLIVOLT = 0;
/// V_165_OR_125_MILLIVOLT.
inline constexpr int32_t V_165_OR_125_MILLIVOLT = 1;
/// V_250_OR_187_MILLIVOLT.
inline constexpr int32_t V_250_OR_187_MILLIVOLT = 2;
/// V_330_OR_248_MILLIVOLT.
inline constexpr int32_t V_330_OR_248_MILLIVOLT = 3;
/// V_415_OR_312_MILLIVOLT.
inline constexpr int32_t V_415_OR_312_MILLIVOLT = 4;
/// V_500_OR_374_MILLIVOLT.
inline constexpr int32_t V_500_OR_374_MILLIVOLT = 5;
/// V_582_OR_434_MILLIVOLT.
inline constexpr int32_t V_582_OR_434_MILLIVOLT = 6;
/// V_660_OR_504_MILLIVOLT.
inline constexpr int32_t V_660_OR_504_MILLIVOLT = 7;
/// V_125_OR_705_MILLIVOLT.
inline constexpr int32_t V_125_OR_705_MILLIVOLT = 8;
/// V_250_OR_940_MILLIVOLT.
inline constexpr int32_t V_250_OR_940_MILLIVOLT = 9;
/// V_375_OR_1180_MILLIVOLT.
inline constexpr int32_t V_375_OR_1180_MILLIVOLT = 10;
/// V_500_OR_1410_MILLIVOLT.
inline constexpr int32_t V_500_OR_1410_MILLIVOLT = 11;
/// V_625_OR_1650_MILLIVOLT.
inline constexpr int32_t V_625_OR_1650_MILLIVOLT = 12;
/// V_750_OR_1880_MILLIVOLT.
inline constexpr int32_t V_750_OR_1880_MILLIVOLT = 13;
/// V_875_OR_2110_MILLIVOLT.
inline constexpr int32_t V_875_OR_2110_MILLIVOLT = 14;
/// V_1000_OR_2350_MILLIVOLT.
inline constexpr int32_t V_1000_OR_2350_MILLIVOLT = 15;
} // namespace overcurrent_protection_uvw_low_side_threshold
/// Values of OVERCURRENT_PROTECTION_UVW_HIGH_SIDE_THRESHOLD.
namespace overcurrent_protection_uvw_high_side_threshold {
/// V_63_MILLIVOLT.
inline constexpr int32_t V_63_MILLIVOLT = 0;
/// V_125_MILLIVOLT.
inline constexpr int32_t V_125_MILLIVOLT = 1;
/// V_187_MILLIVOLT.
inline constexpr int32_t V_187_MILLIVOLT = 2;
/// V_248_MILLIVOLT.
inline constexpr int32_t V_248_MILLIVOLT = 3;
/// V_312_MILLIVOLT.
inline constexpr int32_t V_312_MILLIVOLT = 4;
/// V_374_MILLIVOLT.
inline constexpr int32_t V_374_MILLIVOLT = 5;
/// V_434_MILLIVOLT.
inline constexpr int32_t V_434_MILLIVOLT = 6;
/// V_504_MILLIVOLT.
inline constexpr int32_t V_504_MILLIVOLT = 7;
/// V_705_MILLIVOLT.
inline constexpr int32_t V_705_MILLIVOLT = 8;
/// V_940_MILLIVOLT.
inline constexpr int32_t V_940_MILLIVOLT = 9;
/// V_1180_MILLIVOLT.
inline constexpr int32_t V_1180_MILLIVOLT = 10;
/// V_1410_MILLIVOLT.
inline constexpr int32_t V_1410_MILLIVOLT = 11;
/// V_1650_MILLIVOLT.
inline constexpr int32_t V_1650_MILLIVOLT = 12;
/// V_1880_MILLIVOLT.
inline constexpr int32_t V_1880_MILLIVOLT = 13;
/// V_2110_MILLIVOLT.
inline constexpr int32_t V_2110_MILLIVOLT = 14;
/// V_2350_MILLIVOLT.
inline constexpr int32_t V_2350_MILLIVOLT = 15;
} // namespace overcurrent_protection_uvw_high_side_threshold
/// Values of OVERCURRENT_PROTECTION_Y2_LOW_SIDE_THRESHOLD.
namespace overcurrent_protection_y2_low_side_threshold {
/// V_80_OR_63_MILLIVOLT.
inline constexpr int32_t V_80_OR_63_MILLIVOLT = 0;
/// V_165_OR_125_MILLIVOLT.
inline constexpr int32_t V_165_OR_125_MILLIVOLT = 1;
/// V_250_OR_187_MILLIVOLT.
inline constexpr int32_t V_250_OR_187_MILLIVOLT = 2;
/// V_330_OR_248_MILLIVOLT.
inline constexpr int32_t V_330_OR_248_MILLIVOLT = 3;
/// V_415_OR_312_MILLIVOLT.
inline constexpr int32_t V_415_OR_312_MILLIVOLT = 4;
/// V_500_OR_374_MILLIVOLT.
inline constexpr int32_t V_500_OR_374_MILLIVOLT = 5;
/// V_582_OR_434_MILLIVOLT.
inline constexpr int32_t V_582_OR_434_MILLIVOLT = 6;
/// V_660_OR_504_MILLIVOLT.
inline constexpr int32_t V_660_OR_504_MILLIVOLT = 7;
/// V_125_OR_705_MILLIVOLT.
inline constexpr int32_t V_125_OR_705_MILLIVOLT = 8;
/// V_250_OR_940_MILLIVOLT.
inline constexpr int32_t V_250_OR_940_MILLIVOLT = 9;
/// V_375_OR_1180_MILLIVOLT.
inline constexpr int32_t V_375_OR_1180_MILLIVOLT = 10;
/// V_500_OR_1410_MILLIVOLT.
inline constexpr int32_t V_500_OR_1410_MILLIVOLT = 11;
/// V_625_OR_1650_MILLIVOLT.
inline constexpr int32_t V_625_OR_1650_MILLIVOLT = 12;
/// V_750_OR_1880_MILLIVOLT.
inline constexpr int32_t V_750_OR_1880_MILLIVOLT = 13;
/// V_875_OR_2110_MILLIVOLT.
inline constexpr int32_t V_875_OR_2110_MILLIVOLT = 14;
/// V_1000_OR_2350_MILLIVOLT.
inline constexpr int32_t V_1000_OR_2350_MILLIVOLT = 15;
} // namespace overcurrent_protection_y2_low_side_threshold
/// Values of OVERCURRENT_PROTECTION_Y2_HIGH_SIDE_THRESHOLD.
namespace overcurrent_protection_y2_high_side_threshold {
/// V_63_MILLIVOLT.
inline constexpr int32_t V_63_MILLIVOLT = 0;
/// V_125_MILLIVOLT.
inline constexpr int32_t V_125_MILLIVOLT = 1;
/// V_187_MILLIVOLT.
inline constexpr int32_t V_187_MILLIVOLT = 2;
/// V_248_MILLIVOLT.
inline constexpr int32_t V_248_MILLIVOLT = 3;
/// V_312_MILLIVOLT.
inline constexpr int32_t V_312_MILLIVOLT = 4;
/// V_374_MILLIVOLT.
inline constexpr int32_t V_374_MILLIVOLT = 5;
/// V_434_MILLIVOLT.
inline constexpr int32_t V_434_MILLIVOLT = 6;
/// V_504_MILLIVOLT.
inline constexpr int32_t V_504_MILLIVOLT = 7;
/// V_705_MILLIVOLT.
inline constexpr int32_t V_705_MILLIVOLT = 8;
/// V_940_MILLIVOLT.
inline constexpr int32_t V_940_MILLIVOLT = 9;
/// V_1180_MILLIVOLT.
inline constexpr int32_t V_1180_MILLIVOLT = 10;
/// V_1410_MILLIVOLT.
inline constexpr int32_t V_1410_MILLIVOLT = 11;
/// V_1650_MILLIVOLT.
inline constexpr int32_t V_1650_MILLIVOLT = 12;
/// V_1880_MILLIVOLT.
inline constexpr int32_t V_1880_MILLIVOLT = 13;
/// V_2110_MILLIVOLT.
inline constexpr int32_t V_2110_MILLIVOLT = 14;
/// V_2350_MILLIVOLT.
inline constexpr int32_t V_2350_MILLIVOLT = 15;
} // namespace overcurrent_protection_y2_high_side_threshold
/// Values of OVERCURRENT_PROTECTION_UVW_LOW_SIDE_BLANKING.
namespace overcurrent_protection_uvw_low_side_blanking {
/// OFF.
inline constexpr int32_t OFF = 0;
/// T_0_25_MICROSEC.
inline constexpr int32_t T_0_25_MICROSEC = 1;
/// T_0_5_MICROSEC.
inline constexpr int32_t T_0_5_MICROSEC = 2;
/// T_1_MICROSEC.
inline constexpr int32_t T_1_MICROSEC = 3;
/// T_2_MICROSEC.
inline constexpr int32_t T_2_MICROSEC = 4;
/// T_4_MICROSEC.
inline constexpr int32_t T_4_MICROSEC = 5;
/// T_6_MICROSEC.
inline constexpr int32_t T_6_MICROSEC = 6;
/// T_8_MICROSEC.
inline constexpr int32_t T_8_MICROSEC = 7;
} // namespace overcurrent_protection_uvw_low_side_blanking
/// Values of OVERCURRENT_PROTECTION_UVW_HIGH_SIDE_BLANKING.
namespace overcurrent_protection_uvw_high_side_blanking {
/// OFF.
inline constexpr int32_t OFF = 0;
/// T_0_25_MICROSEC.
inline constexpr int32_t T_0_25_MICROSEC = 1;
/// T_0_5_MICROSEC.
inline constexpr int32_t T_0_5_MICROSEC = 2;
/// T_1_MICROSEC.
inline constexpr int32_t T_1_MICROSEC = 3;
/// T_2_MICROSEC.
inline constexpr int32_t T_2_MICROSEC = 4;
/// T_4_MICROSEC.
inline constexpr int32_t T_4_MICROSEC = 5;
/// T_6_MICROSEC.
inline constexpr int32_t T_6_MICROSEC = 6;
/// T_8_MICROSEC.
inline constexpr int32_t T_8_MICROSEC = 7;
} // namespace overcurrent_protection_uvw_high_side_blanking
/// Values of OVERCURRENT_PROTECTION_Y2_LOW_SIDE_BLANKING.
namespace overcurrent_protection_y2_low_side_blanking {
/// OFF.
inline constexpr int32_t OFF = 0;
/// T_0_25_MICROSEC.
inline constexpr int32_t T_0_25_MICROSEC = 1;
/// T_0_5_MICROSEC.
inline constexpr int32_t T_0_5_MICROSEC = 2;
/// T_1_MICROSEC.
inline constexpr int32_t T_1_MICROSEC = 3;
/// T_2_MICROSEC.
inline constexpr int32_t T_2_MICROSEC = 4;
/// T_4_MICROSEC.
inline constexpr int32_t T_4_MICROSEC = 5;
/// T_6_MICROSEC.
inline constexpr int32_t T_6_MICROSEC = 6;
/// T_8_MICROSEC.
inline constexpr int32_t T_8_MICROSEC = 7;
} // namespace overcurrent_protection_y2_low_side_blanking
/// Values of OVERCURRENT_PROTECTION_Y2_HIGH_SIDE_BLANKING.
namespace overcurrent_protection_y2_high_side_blanking {
/// OFF.
inline constexpr int32_t OFF = 0;
/// T_0_25_MICROSEC.
inline constexpr int32_t T_0_25_MICROSEC = 1;
/// T_0_5_MICROSEC.
inline constexpr int32_t T_0_5_MICROSEC = 2;
/// T_1_MICROSEC.
inline constexpr int32_t T_1_MICROSEC = 3;
/// T_2_MICROSEC.
inline constexpr int32_t T_2_MICROSEC = 4;
/// T_4_MICROSEC.
inline constexpr int32_t T_4_MICROSEC = 5;
/// T_6_MICROSEC.
inline constexpr int32_t T_6_MICROSEC = 6;
/// T_8_MICROSEC.
inline constexpr int32_t T_8_MICROSEC = 7;
} // namespace overcurrent_protection_y2_high_side_blanking
/// Values of OVERCURRENT_PROTECTION_UVW_LOW_SIDE_DEGLITCH.
namespace overcurrent_protection_uvw_low_side_deglitch {
/// OFF.
inline constexpr int32_t OFF = 0;
/// T_0_25_MICROSEC.
inline constexpr int32_t T_0_25_MICROSEC = 1;
/// T_0_5_MICROSEC.
inline constexpr int32_t T_0_5_MICROSEC = 2;
/// T_1_MICROSEC.
inline constexpr int32_t T_1_MICROSEC = 3;
/// T_2_MICROSEC.
inline constexpr int32_t T_2_MICROSEC = 4;
/// T_4_MICROSEC.
inline constexpr int32_t T_4_MICROSEC = 5;
/// T_6_MICROSEC.
inline constexpr int32_t T_6_MICROSEC = 6;
/// T_8_MICROSEC.
inline constexpr int32_t T_8_MICROSEC = 7;
} // namespace overcurrent_protection_uvw_low_side_deglitch
/// Values of OVERCURRENT_PROTECTION_UVW_HIGH_SIDE_DEGLITCH.
namespace overcurrent_protection_uvw_high_side_deglitch {
/// OFF.
inline constexpr int32_t OFF = 0;
/// T_0_25_MICROSEC.
inline constexpr int32_t T_0_25_MICROSEC = 1;
/// T_0_5_MICROSEC.
inline constexpr int32_t T_0_5_MICROSEC = 2;
/// T_1_MICROSEC.
inline constexpr int32_t T_1_MICROSEC = 3;
/// T_2_MICROSEC.
inline constexpr int32_t T_2_MICROSEC = 4;
/// T_4_MICROSEC.
inline constexpr int32_t T_4_MICROSEC = 5;
/// T_6_MICROSEC.
inline constexpr int32_t T_6_MICROSEC = 6;
/// T_8_MICROSEC.
inline constexpr int32_t T_8_MICROSEC = 7;
} // namespace overcurrent_protection_uvw_high_side_deglitch
/// Values of OVERCURRENT_PROTECTION_Y2_LOW_SIDE_DEGLITCH.
namespace overcurrent_protection_y2_low_side_deglitch {
/// OFF.
inline constexpr int32_t OFF = 0;
/// T_0_25_MICROSEC.
inline constexpr int32_t T_0_25_MICROSEC = 1;
/// T_0_5_MICROSEC.
inline constexpr int32_t T_0_5_MICROSEC = 2;
/// T_1_MICROSEC.
inline constexpr int32_t T_1_MICROSEC = 3;
/// T_2_MICROSEC.
inline constexpr int32_t T_2_MICROSEC = 4;
/// T_4_MICROSEC.
inline constexpr int32_t T_4_MICROSEC = 5;
/// T_6_MICROSEC.
inline constexpr int32_t T_6_MICROSEC = 6;
/// T_8_MICROSEC.
inline constexpr int32_t T_8_MICROSEC = 7;
} // namespace overcurrent_protection_y2_low_side_deglitch
/// Values of OVERCURRENT_PROTECTION_Y2_HIGH_SIDE_DEGLITCH.
namespace overcurrent_protection_y2_high_side_deglitch {
/// OFF.
inline constexpr int32_t OFF = 0;
/// T_0_25_MICROSEC.
inline constexpr int32_t T_0_25_MICROSEC = 1;
/// T_0_5_MICROSEC.
inline constexpr int32_t T_0_5_MICROSEC = 2;
/// T_1_MICROSEC.
inline constexpr int32_t T_1_MICROSEC = 3;
/// T_2_MICROSEC.
inline constexpr int32_t T_2_MICROSEC = 4;
/// T_4_MICROSEC.
inline constexpr int32_t T_4_MICROSEC = 5;
/// T_6_MICROSEC.
inline constexpr int32_t T_6_MICROSEC = 6;
/// T_8_MICROSEC.
inline constexpr int32_t T_8_MICROSEC = 7;
} // namespace overcurrent_protection_y2_high_side_deglitch
/// Values of VGS_SHORT_PROTECTION_UVW_BLANKING.
namespace vgs_short_protection_uvw_blanking {
/// OFF.
inline constexpr int32_t OFF = 0;
/// T_0_25_MICROSEC.
inline constexpr int32_t T_0_25_MICROSEC = 1;
/// T_0_5_MICROSEC.
inline constexpr int32_t T_0_5_MICROSEC = 2;
/// T_1_MICROSEC.
inline constexpr int32_t T_1_MICROSEC = 3;
} // namespace vgs_short_protection_uvw_blanking
/// Values of VGS_SHORT_PROTECTION_Y2_BLANKING.
namespace vgs_short_protection_y2_blanking {
/// OFF.
inline constexpr int32_t OFF = 0;
/// T_0_25_MICROSEC.
inline constexpr int32_t T_0_25_MICROSEC = 1;
/// T_0_5_MICROSEC.
inline constexpr int32_t T_0_5_MICROSEC = 2;
/// T_1_MICROSEC.
inline constexpr int32_t T_1_MICROSEC = 3;
} // namespace vgs_short_protection_y2_blanking
/// Values of VGS_SHORT_PROTECTION_UVW_DEGLITCH.
namespace vgs_short_protection_uvw_deglitch {
/// OFF.
inline constexpr int32_t OFF = 0;
/// T_0_25_MICROSEC.
inline constexpr int32_t T_0_25_MICROSEC = 1;
/// T_0_5_MICROSEC.
inline constexpr int32_t T_0_5_MICROSEC = 2;
/// T_1_MICROSEC.
inline constexpr int32_t T_1_MICROSEC = 3;
/// T_2_MICROSEC.
inline constexpr int32_t T_2_MICROSEC = 4;
/// T_4_MICROSEC.
inline constexpr int32_t T_4_MICROSEC = 5;
/// T_6_MICROSEC.
inline constexpr int32_t T_6_MICROSEC = 6;
/// T_8_MICROSEC.
inline constexpr int32_t T_8_MICROSEC = 7;
} // namespace vgs_short_protection_uvw_deglitch
/// Values of VGS_SHORT_PROTECTION_Y2_DEGLITCH.
namespace vgs_short_protection_y2_deglitch {
/// OFF.
inline constexpr int32_t OFF = 0;
/// T_0_25_MICROSEC.
inline constexpr int32_t T_0_25_MICROSEC = 1;
/// T_0_5_MICROSEC.
inline constexpr int32_t T_0_5_MICROSEC = 2;
/// T_1_MICROSEC.
inline constexpr int32_t T_1_MICROSEC = 3;
/// T_2_MICROSEC.
inline constexpr int32_t T_2_MICROSEC = 4;
/// T_4_MICROSEC.
inline constexpr int32_t T_4_MICROSEC = 5;
/// T_6_MICROSEC.
inline constexpr int32_t T_6_MICROSEC = 6;
/// T_8_MICROSEC.
inline constexpr int32_t T_8_MICROSEC = 7;
} // namespace vgs_short_protection_y2_deglitch
/// Values of GDRV_RETRY_BEHAVIOUR.
namespace gdrv_retry_behaviour {
/// OPEN_CIRCUIT.
inline constexpr int32_t OPEN_CIRCUIT = 0;
/// ELECTRICAL_BRAKING.
inline constexpr int32_t ELECTRICAL_BRAKING = 1;
} // namespace gdrv_retry_behaviour
/// Values of DRIVE_FAULT_BEHAVIOUR.
namespace drive_fault_behaviour {
/// OPEN_CIRCUIT.
inline constexpr int32_t OPEN_CIRCUIT = 0;
/// ELECTRICAL_BRAKING.
inline constexpr int32_t ELECTRICAL_BRAKING = 1;
/// MECHANICAL_BRAKING_AND_OPEN_CIRCUIT.
inline constexpr int32_t MECHANICAL_BRAKING_AND_OPEN_CIRCUIT = 2;
/// MECHANICAL_AND_ELECTRICAL_BRAKING.
inline constexpr int32_t MECHANICAL_AND_ELECTRICAL_BRAKING = 3;
} // namespace drive_fault_behaviour
} // namespace value

} // namespace tmc9660

#endif // TMC9660_PARAMS_HPP
