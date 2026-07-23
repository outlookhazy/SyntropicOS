/**
 * @file syn_cia402.h
 * @brief CANopen CiA 402 Servo Drive & Motion Control Profile Engine.
 *
 * Implements the standard CiA 402 power drive state machine, Controlword
 * (0x6040) processing, Statusword (0x6041) status reporting, and motion
 * modes of operation (Profile Position, Profile Velocity, Profile Torque,
 * Homing, and Cyclic Synchronous Position/Velocity).
 *
 * Designed to bind directly into syn_canopen Object Dictionary tables.
 * @ingroup syn_protocol
 */

#ifndef SYN_CIA402_H
#define SYN_CIA402_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../common/syn_defs.h"
#include "../util/syn_scurve.h"
#include "syn_canopen.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief CiA 402 Drive State Machine States */
typedef enum {
    SYN_CIA402_STATE_NOT_READY           = 0, /**< Not Ready to Switch On */
    SYN_CIA402_STATE_SWITCH_ON_DISABLED = 1, /**< Switch On Disabled */
    SYN_CIA402_STATE_READY_TO_SWITCH_ON = 2, /**< Ready to Switch On */
    SYN_CIA402_STATE_SWITCHED_ON        = 3, /**< Switched On */
    SYN_CIA402_STATE_OPERATION_ENABLED   = 4, /**< Operation Enabled */
    SYN_CIA402_STATE_QUICK_STOP_ACTIVE   = 5, /**< Quick Stop Active */
    SYN_CIA402_STATE_FAULT_REACTION      = 6, /**< Fault Reaction Active */
    SYN_CIA402_STATE_FAULT               = 7  /**< Fault */
} SYN_CiA402State;

/** @brief CiA 402 Modes of Operation (0x6060 / 0x6061) */
typedef enum {
    SYN_CIA402_MODE_NO_MODE = 0,  /**< No mode change / unassigned */
    SYN_CIA402_MODE_PP      = 1,  /**< Profile Position Mode */
    SYN_CIA402_MODE_PV      = 3,  /**< Profile Velocity Mode */
    SYN_CIA402_MODE_PT      = 4,  /**< Profile Torque Mode */
    SYN_CIA402_MODE_HM      = 6,  /**< Homing Mode */
    SYN_CIA402_MODE_CSP     = 8,  /**< Cyclic Synchronous Position Mode */
    SYN_CIA402_MODE_CSV     = 9,  /**< Cyclic Synchronous Velocity Mode */
    SYN_CIA402_MODE_CST     = 10  /**< Cyclic Synchronous Torque Mode */
} SYN_CiA402Mode;

/** @name Controlword Bits (0x6040)
 * @{
 */
#define SYN_CIA402_CW_SWITCH_ON       (1U << 0) /**< Bit 0: Switch On */
#define SYN_CIA402_CW_ENABLE_VOLTAGE  (1U << 1) /**< Bit 1: Enable Voltage */
#define SYN_CIA402_CW_QUICK_STOP      (1U << 2) /**< Bit 2: Quick Stop (active low) */
#define SYN_CIA402_CW_ENABLE_OP       (1U << 3) /**< Bit 3: Enable Operation */
#define SYN_CIA402_CW_NEW_SETPOINT    (1U << 4) /**< Bit 4: PP New Setpoint */
#define SYN_CIA402_CW_CHANGE_IMMED    (1U << 5) /**< Bit 5: PP Change Immediately */
#define SYN_CIA402_CW_ABS_REL         (1U << 6) /**< Bit 6: PP Absolute(0) / Relative(1) */
#define SYN_CIA402_CW_FAULT_RESET     (1U << 7) /**< Bit 7: Fault Reset (0->1 transition) */
#define SYN_CIA402_CW_HALT            (1U << 8) /**< Bit 8: Halt motion */
/** @} */

/** @name Statusword Bits (0x6041)
 * @{
 */
#define SYN_CIA402_SW_READY_TO_SWITCH_ON (1U << 0)  /**< Bit 0: Ready to Switch On */
#define SYN_CIA402_SW_SWITCHED_ON        (1U << 1)  /**< Bit 1: Switched On */
#define SYN_CIA402_SW_OPERATION_ENABLED  (1U << 2)  /**< Bit 2: Operation Enabled */
#define SYN_CIA402_SW_FAULT              (1U << 3)  /**< Bit 3: Fault */
#define SYN_CIA402_SW_VOLTAGE_ENABLED    (1U << 4)  /**< Bit 4: Voltage Enabled */
#define SYN_CIA402_SW_QUICK_STOP         (1U << 5)  /**< Bit 5: Quick Stop (1 = not active) */
#define SYN_CIA402_SW_SWITCH_ON_DISABLED (1U << 6)  /**< Bit 6: Switch On Disabled */
#define SYN_CIA402_SW_WARNING            (1U << 7)  /**< Bit 7: Warning */
#define SYN_CIA402_SW_TARGET_REACHED     (1U << 10) /**< Bit 10: Target Reached */
#define SYN_CIA402_SW_INTERNAL_LIMIT     (1U << 11) /**< Bit 11: Internal Limit Active */
#define SYN_CIA402_SW_SETPOINT_ACK       (1U << 12) /**< Bit 12: Setpoint Acknowledge */
/** @} */

/**
 * @brief Configuration parameters for CiA 402 Drive.
 */
typedef struct {
    int32_t max_profile_velocity;   /**< Max profile velocity in units/s (0x607F) */
    int32_t profile_acceleration;   /**< Profile acceleration in units/s^2 (0x6083) */
    int32_t profile_deceleration;   /**< Profile deceleration in units/s^2 (0x6084) */
    int32_t quick_stop_deceleration;/**< Quick stop deceleration in units/s^2 (0x6085) */
    int16_t max_torque;             /**< Maximum allowable torque/current setpoint (0x6072) */
} SYN_CiA402Config;

/**
 * @brief CiA 402 Drive Handle.
 */
typedef struct {
    SYN_CiA402State state;          /**< Current drive state machine state */
    SYN_CiA402Config cfg;           /**< Drive configuration parameters */

    /* 0x6000-series Object Dictionary Variables */
    uint16_t controlword;           /**< Index 0x6040 Controlword */
    uint16_t statusword;            /**< Index 0x6041 Statusword */
    int8_t   mode_of_operation;     /**< Index 0x6060 Modes of Operation */
    int8_t   mode_display;          /**< Index 0x6061 Modes of Operation Display */
    int32_t  target_position;       /**< Index 0x607A Target Position */
    int32_t  actual_position;       /**< Index 0x6064 Position Actual Value */
    int32_t  target_velocity;       /**< Index 0x60FF Target Velocity */
    int32_t  actual_velocity;       /**< Index 0x606C Velocity Actual Value */
    int16_t  target_torque;         /**< Index 0x6071 Target Torque */
    int16_t  actual_torque;         /**< Index 0x6077 Torque Actual Value */
    uint16_t error_code;            /**< Index 0x603F Error Code */

    /* Motion Trajectory & State Flags */
    SYN_SCurve scurve;              /**< Internal S-Curve trajectory generator for PP mode */
    float   current_speed;          /**< Current internal continuous speed */
    float   current_pos;            /**< Current internal continuous position */
    bool    setpoint_ack;           /**< Setpoint acknowledge bit indicator */
    bool    target_reached;         /**< Target reached flag */
    uint16_t prev_controlword;      /**< Previous controlword for edge detection */
} SYN_CiA402Drive;

/**
 * @brief Initialize a CiA 402 Drive instance.
 *
 * @param drive Pointer to drive handle.
 * @param cfg Pointer to drive configuration.
 * @return SYN_OK on success, SYN_INVALID_PARAM if pointers are NULL.
 */
SYN_Status syn_cia402_init(SYN_CiA402Drive *drive, const SYN_CiA402Config *cfg);

/**
 * @brief Process new Controlword command and update drive state machine.
 *
 * @param drive Pointer to drive handle.
 * @param controlword Received 16-bit Controlword (0x6040).
 * @return SYN_OK on success, SYN_INVALID_PARAM if drive is NULL.
 */
SYN_Status syn_cia402_set_controlword(SYN_CiA402Drive *drive, uint16_t controlword);

/**
 * @brief Retrieve current 16-bit Statusword value.
 *
 * @param drive Pointer to drive handle.
 * @return 16-bit Statusword (0x6041), or 0 if drive is NULL.
 */
uint16_t syn_cia402_get_statusword(const SYN_CiA402Drive *drive);

/**
 * @brief Set requested Mode of Operation.
 *
 * @param drive Pointer to drive handle.
 * @param mode Target mode (SYN_CIA402_MODE_*).
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid inputs.
 */
SYN_Status syn_cia402_set_mode(SYN_CiA402Drive *drive, int8_t mode);

/**
 * @brief Set target position for PP or CSP mode.
 *
 * @param drive Pointer to drive handle.
 * @param target_pos Target position value (0x607A).
 * @return SYN_OK on success, SYN_INVALID_PARAM if drive is NULL.
 */
SYN_Status syn_cia402_set_target_position(SYN_CiA402Drive *drive, int32_t target_pos);

/**
 * @brief Set target velocity for PV or CSV mode.
 *
 * @param drive Pointer to drive handle.
 * @param target_vel Target velocity value (0x60FF).
 * @return SYN_OK on success, SYN_INVALID_PARAM if drive is NULL.
 */
SYN_Status syn_cia402_set_target_velocity(SYN_CiA402Drive *drive, int32_t target_vel);

/**
 * @brief Set target torque for PT or CST mode.
 *
 * @param drive Pointer to drive handle.
 * @param target_torque Target torque value (0x6071).
 * @return SYN_OK on success, SYN_INVALID_PARAM if drive is NULL.
 */
SYN_Status syn_cia402_set_target_torque(SYN_CiA402Drive *drive, int16_t target_torque);

/**
 * @brief Periodic motion control update step.
 *
 * Executes trajectory planning (S-Curve PP, PV ramps) and state machine updates.
 *
 * @param drive Pointer to drive handle.
 * @param dt_ms Elapsed time step in milliseconds.
 * @return SYN_OK on success, SYN_INVALID_PARAM if drive is NULL.
 */
SYN_Status syn_cia402_update(SYN_CiA402Drive *drive, uint32_t dt_ms);

/**
 * @brief Report actual motor feedback (position, velocity, torque).
 *
 * @param drive Pointer to drive handle.
 * @param actual_pos Actual measured position (0x6064).
 * @param actual_vel Actual measured velocity (0x606C).
 * @param actual_torque Actual measured torque (0x6077).
 * @return SYN_OK on success, SYN_INVALID_PARAM if drive is NULL.
 */
SYN_Status syn_cia402_report_actuals(SYN_CiA402Drive *drive, int32_t actual_pos, int32_t actual_vel, int16_t actual_torque);

/**
 * @brief Trigger a fault state in the drive.
 *
 * @param drive Pointer to drive handle.
 * @param error_code 16-bit error code (0x603F).
 * @return SYN_OK on success, SYN_INVALID_PARAM if drive is NULL.
 */
SYN_Status syn_cia402_trigger_fault(SYN_CiA402Drive *drive, uint16_t error_code);

/**
 * @brief Populate a standard CANopen Object Dictionary array with CiA 402 drive objects.
 *
 * Creates entries for 0x6040, 0x6041, 0x6060, 0x6061, 0x6064, 0x606C, 0x6071, 0x6077, 0x607A, 0x60FF.
 *
 * @param entries Output array of SYN_CANOpenODEntry elements.
 * @param max_entries Maximum capacity of entries array (must be >= 10).
 * @param drive Pointer to drive handle.
 * @return Number of entries populated, or 0 on error.
 */
size_t syn_cia402_populate_od(SYN_CANOpenODEntry *entries, size_t max_entries, SYN_CiA402Drive *drive);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CIA402_H */
