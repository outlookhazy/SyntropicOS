/**
 * @file syn_bldc_6step.h
 * @brief Zero-Heap 6-Step (Trapezoidal) BLDC Motor Commutation Driver.
 *
 * Provides 6-step trapezoidal block commutation for 3-phase Brushless DC (BLDC) motors:
 * - 3-channel Hall sensor state decoding (Hall states 1..6)
 * - 6-step phase commutation lookup table for 3-phase half-bridge outputs (U, V, W)
 * - Clockwise (CW) & Counter-Clockwise (CCW) rotation support
 * - Fault detection for invalid Hall states (0b000 & 0b111)
 * - PWM duty cycle scaling (0..1000)
 * - Hall transition edge timestamping for RPM speed calculation
 */

#ifndef SYN_BLDC_6STEP_H
#define SYN_BLDC_6STEP_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/control/syn_pid.h"
#include "syntropic/util/syn_qmath.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Phase Output State (High-Side / Low-Side Gate Switches). */
typedef enum {
    SYN_BLDC_GATE_OFF = 0, /**< Both High-Side and Low-Side gates OFF (Hi-Z). */
    SYN_BLDC_GATE_PWM = 1, /**< High-Side PWM modulated, Low-Side OFF. */
    SYN_BLDC_GATE_LOW = 2, /**< High-Side OFF, Low-Side ON (GND return). */
} SYN_BLDC_GateState;

/** 3-Phase Gate Output Command. */
typedef struct {
    SYN_BLDC_GateState u; /**< Phase U gate state. */
    SYN_BLDC_GateState v; /**< Phase V gate state. */
    SYN_BLDC_GateState w; /**< Phase W gate state. */
    uint16_t duty;        /**< Active PWM duty cycle (0..1000). */
} SYN_BLDC_PhaseOutputs;

/** Motor Rotation Direction. */
typedef enum {
    SYN_BLDC_DIR_CW = 1, /**< Clockwise rotation. */
    SYN_BLDC_DIR_CCW = 2 /**< Counter-clockwise rotation. */
} SYN_BLDC_Direction;

/** Driver State. */
typedef enum {
    SYN_BLDC_STATE_STOPPED = 0, /**< Motor stopped (all gates OFF). */
    SYN_BLDC_STATE_RUNNING = 1, /**< Normal 6-step commutation running. */
    SYN_BLDC_STATE_FAULT = 2    /**< Fault state (e.g. invalid Hall sensor input). */
} SYN_BLDC_State;

/** Configuration parameters for 6-Step BLDC driver. */
typedef struct {
    uint8_t pole_pairs;     /**< Number of motor rotor pole pairs (default: 4). */
    uint16_t pwm_frequency; /**< PWM carrier frequency in Hz (informative/diagnostic). */
} SYN_BLDC_Config;

/** 6-Step BLDC Motor Instance. */
typedef struct {
    SYN_BLDC_Config config;       /**< Configuration snapshot. */
    SYN_BLDC_State state;         /**< Current driver state. */
    SYN_BLDC_Direction direction; /**< Requested rotation direction. */
    uint8_t current_step;         /**< Active 6-step commutation step (1..6). */
    uint8_t hall_state;           /**< Last sampled 3-bit Hall sensor state (1..6). */
    uint16_t duty;                /**< Target PWM duty cycle (0..1000). */
    uint32_t hall_transitions;    /**< Total accumulated Hall edge transitions. */
    uint32_t last_hall_tick_ms;   /**< Timestamp of last Hall transition. */
    uint32_t rpm;                 /**< Calculated motor speed in RPM. */
    SYN_PID speed_pid;            /**< Integrated speed PID controller instance. */
    bool speed_pid_active;        /**< True if PID closed-loop speed control is enabled. */
    bool speed_tick_valid;        /**< True once initial speed window timestamp is latched. */
} SYN_BLDC_6Step;

/**
 * @brief Initialize 6-Step BLDC driver instance.
 *
 * @param bldc Pointer to driver struct.
 * @param cfg  Configuration parameters (or NULL for defaults: 4 pole pairs).
 * @return SYN_OK on success, SYN_ERR_INVALID_PARAM if bldc is NULL.
 */
SYN_Status syn_bldc_6step_init(SYN_BLDC_6Step *bldc, const SYN_BLDC_Config *cfg);

/**
 * @brief Process 3-bit Hall sensor reading and update 6-step phase commutation gates.
 *
 * Call this from Hall sensor EXTI GPIO interrupts or input sampling loops.
 *
 * @param bldc       Pointer to driver struct.
 * @param hall_state 3-bit Hall sensor state (bits [2:0] = H3, H2, H1). Valid: 1..6.
 * @param out        Pointer to output struct to receive new 3-phase gate states.
 * @return SYN_OK on valid commutation step, SYN_ERR_FAULT if hall_state is 0b000 or 0b111.
 */
SYN_Status syn_bldc_6step_set_hall(SYN_BLDC_6Step *bldc, uint8_t hall_state,
                                   SYN_BLDC_PhaseOutputs *out);

/**
 * @brief Set PWM duty cycle for motor speed control.
 *
 * @param bldc             Pointer to driver struct.
 * @param duty_0_to_1000   PWM duty cycle clamped to range 0..1000.
 * @return SYN_OK on success.
 */
SYN_Status syn_bldc_6step_set_duty(SYN_BLDC_6Step *bldc, uint16_t duty_0_to_1000);

/**
 * @brief Set motor rotation direction.
 *
 * @param bldc Pointer to driver struct.
 * @param dir  SYN_BLDC_DIR_CW or SYN_BLDC_DIR_CCW.
 * @return SYN_OK on success.
 */
SYN_Status syn_bldc_6step_set_direction(SYN_BLDC_6Step *bldc, SYN_BLDC_Direction dir);

/**
 * @brief Start motor driver (enable gate commutation).
 *
 * @param bldc Pointer to driver struct.
 * @return SYN_OK on success.
 */
SYN_Status syn_bldc_6step_start(SYN_BLDC_6Step *bldc);

/**
 * @brief Stop motor driver (turn OFF all 6 gates).
 *
 * @param bldc Pointer to driver struct.
 * @param out  Optional pointer to receive updated OFF gate states.
 * @return SYN_OK on success.
 */
SYN_Status syn_bldc_6step_stop(SYN_BLDC_6Step *bldc, SYN_BLDC_PhaseOutputs *out);

/**
 * @brief Update Hall-based speed calculation (RPM) and speed PID loop.
 *
 * Call periodically (e.g. every 10ms or 100ms).
 *
 * @param bldc       Pointer to driver struct.
 * @param now_ms     Current system tick in milliseconds.
 * @param target_rpm Target motor speed in RPM (used if speed PID is active).
 * @return Calculated current RPM.
 */
uint32_t syn_bldc_6step_update_speed(SYN_BLDC_6Step *bldc, uint32_t now_ms, uint32_t target_rpm);

/**
 * @brief Retrieve current 3-phase gate output states.
 *
 * @param bldc Pointer to driver struct.
 * @param out  Pointer to destination output struct.
 * @return SYN_OK on success.
 */
SYN_Status syn_bldc_6step_get_phase_outputs(const SYN_BLDC_6Step *bldc, SYN_BLDC_PhaseOutputs *out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BLDC_6STEP_H */
