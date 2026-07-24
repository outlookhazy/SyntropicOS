/**
 * @file syn_interpolator.h
 * @brief Coordinated multi-axis linear & circular arc trajectory interpolator.
 * @ingroup syn_motor
 *
 * Provides zero-allocation, fixed-step multi-axis DDA (Digital Differential
 * Analyzer) vector trajectory generation for coordinated motion control across
 * 2D/3D linear segments and 2D circular arcs.
 */

#ifndef SYN_INTERPOLATOR_H
#define SYN_INTERPOLATOR_H

#include "../common/syn_defs.h"
#include "../util/syn_scurve.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Interpolation mode enum */
typedef enum {
    SYN_INTERP_MODE_IDLE = 0,
    SYN_INTERP_MODE_LINEAR,       /**< 2D or 3D linear segment */
    SYN_INTERP_MODE_CIRCULAR_CW,  /**< 2D Clockwise arc */
    SYN_INTERP_MODE_CIRCULAR_CCW, /**< 2D Counter-clockwise arc */
} SYN_Interp_Mode;

/** @brief 3D Vector position in steps or user units */
typedef struct {
    float x; /**< X-axis position coordinate */
    float y; /**< Y-axis position coordinate */
    float z; /**< Z-axis position coordinate */
} SYN_Vector3F;

/** @brief Multi-Axis Interpolator configuration and state machine */
typedef struct {
    SYN_Interp_Mode mode;       /**< Active interpolation mode */

    /* Endpoints & Geometry */
    SYN_Vector3F start_pos;      /**< Segment start point (X, Y, Z) */
    SYN_Vector3F target_pos;     /**< Segment target point (X, Y, Z) */
    SYN_Vector3F center_offset;  /**< Circle center offset relative to start (I, J, K) */

    /* Current State */
    SYN_Vector3F current_pos;   /**< Calculated current position */
    float        total_length;  /**< Total path length in units */
    float        current_dist;  /**< Current traveled distance along path */
    float        radius;        /**< Arc radius (for circular mode) */
    float        start_angle;   /**< Arc start angle in radians */
    float        sweep_angle;   /**< Arc total sweep angle in radians */

    /* Speed & Acceleration Profiling */
    SYN_SCurve   scurve;        /**< S-Curve velocity profile generator */
    float        target_feedrate; /**< Max velocity along path (units/sec) */

    uint32_t     total_steps;   /**< Number of discrete steps in move */
    uint32_t     step_index;    /**< Current step index (0 .. total_steps) */
} SYN_Interpolator;

/**
 * @brief Initialize a Multi-Axis Interpolator.
 * @param interp Pointer to interpolator struct.
 */
void syn_interpolator_init(SYN_Interpolator *interp);

/**
 * @brief Plan a Linear 2D/3D motion move.
 *
 * @param interp      Pointer to interpolator struct.
 * @param start       Start position (X, Y, Z).
 * @param target      Target position (X, Y, Z).
 * @param feedrate    Desired vector velocity (units/sec).
 * @param max_accel   Max acceleration (units/sec^2).
 * @param max_jerk    Max jerk (units/sec^3).
 * @param step_res    Spatial resolution per step (units per step, e.g. 0.001mm).
 * @return SYN_OK on success.
 */
SYN_Status syn_interpolator_plan_linear(SYN_Interpolator *interp,
                                         SYN_Vector3F start, SYN_Vector3F target,
                                         float feedrate, float max_accel, float max_jerk,
                                         float step_res);

/**
 * @brief Plan a Circular 2D arc move in XY plane.
 *
 * @param interp      Pointer to interpolator struct.
 * @param start       Start position (X, Y).
 * @param target      Target position (X, Y).
 * @param center_offset Center offset relative to start (I = center.x - start.x, J = center.y - start.y).
 * @param is_cw       true for Clockwise, false for Counter-Clockwise.
 * @param feedrate    Desired tangential speed (units/sec).
 * @param max_accel   Max acceleration (units/sec^2).
 * @param max_jerk    Max jerk (units/sec^3).
 * @param step_res    Spatial resolution per step (units per step).
 * @return SYN_OK on success.
 */
SYN_Status syn_interpolator_plan_circular(SYN_Interpolator *interp,
                                           SYN_Vector3F start, SYN_Vector3F target,
                                           SYN_Vector3F center_offset, bool is_cw,
                                           float feedrate, float max_accel, float max_jerk,
                                           float step_res);

/**
 * @brief Advance the interpolator by one discrete step.
 *
 * @param interp   Pointer to interpolator struct.
 * @param out_pos  Out: Calculated position after step.
 * @return true if move active and advanced, false if move complete.
 */
bool syn_interpolator_step(SYN_Interpolator *interp, SYN_Vector3F *out_pos);

/**
 * @brief Evaluate the interpolator at a specific time offset `t` seconds.
 *
 * @param interp   Pointer to interpolator struct.
 * @param t_sec    Time offset from start of move in seconds.
 * @param out_pos  Out: Position vector at time t.
 * @param out_vel  Out: Velocity vector at time t (optional, can be NULL).
 * @return true if t <= move total time.
 */
bool syn_interpolator_eval_at_time(SYN_Interpolator *interp, float t_sec,
                                   SYN_Vector3F *out_pos, SYN_Vector3F *out_vel);

#ifdef __cplusplus
}
#endif

#endif /* SYN_INTERPOLATOR_H */
