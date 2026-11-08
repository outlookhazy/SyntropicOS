/**
 * @file syn_motor_output.h
 * @brief Abstract motor output interface.
 *
 * Decouples the motor controller from specific motor drivers.
 * Any motor (DC, stepper, brushless, etc.) implements this
 * three-function interface and can be driven by syn_motor_ctrl.
 *
 * @par Usage
 * @code
 *   // DC motor — use the factory:
 *   SYN_MotorOutput out = syn_dc_motor_output(&my_dc);
 *
 *   // Custom motor — wire directly:
 *   SYN_MotorOutput out = {
 *       .set_output = my_motor_set_output,
 *       .coast      = my_motor_coast,
 *       .brake      = my_motor_brake,
 *       .ctx        = &my_motor,
 *   };
 *
 *   // Pass to motor controller:
 *   cfg.motor = out;
 * @endcode
 * @ingroup syn_motor
 */

#ifndef SYN_MOTOR_OUTPUT_H
#define SYN_MOTOR_OUTPUT_H

#include "../common/syn_defs.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Abstract motor output interface (vtable).
 *
 * The motor controller calls these functions to drive the motor
 * without knowing the underlying hardware. This allows any motor
 * driver to be used with syn_motor_ctrl, syn_autotune, etc.
 */
typedef struct {
    /**
     * @brief Drive the motor at the given signed output level.
     *
     * The output range matches the controller's [output_min, output_max].
     * Positive = forward, negative = reverse, 0 = stop.
     * The implementation must map this to the appropriate hardware
     * command (PWM duty, step rate, etc.).
     *
     * @param ctx     User context (motor instance pointer).
     * @param output  Signed output level.
     */
    void (*set_output)(void *ctx, int32_t output);

    /**
     * @brief Coast — free-spin, both outputs off.
     *
     * Motor decelerates due to friction only. Called when the
     * controller enters idle or stops normally.
     *
     * @param ctx  User context.
     */
    void (*coast)(void *ctx);

    /**
     * @brief Active brake — short motor windings or active stop.
     *
     * Motor decelerates as quickly as possible. Called during
     * emergency stop (e-stop).
     *
     * @param ctx  User context.
     */
    void (*brake)(void *ctx);

    /** @brief User context passed to all callbacks. */
    void *ctx;
} SYN_MotorOutput;

#ifdef __cplusplus
}
#endif

#endif /* SYN_MOTOR_OUTPUT_H */
