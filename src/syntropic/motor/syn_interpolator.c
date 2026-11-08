#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_INTERPOLATOR) || SYN_USE_INTERPOLATOR

/**
 * @file syn_interpolator.c
 * @brief Coordinated multi-axis linear & circular arc trajectory interpolator implementation.
 */

#include "../util/syn_assert.h"
#include "syn_interpolator.h"

#include <math.h>
#include <string.h>

/** @cond */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
/** @endcond */

/**
 * @brief Evaluate 3D cubic Bezier position B(u) at parameter u in [0, 1].
 * @param p0 Start control point.
 * @param p1 First interior control point.
 * @param p2 Second interior control point.
 * @param p3 End control point.
 * @param u  Curve parameter in [0, 1].
 * @return Evaluated 3D position on the curve.
 */
static SYN_Vector3F interp_eval_bezier_pos(SYN_Vector3F p0, SYN_Vector3F p1, SYN_Vector3F p2,
                                           SYN_Vector3F p3, float u)
{
    float u_inv = 1.0f - u;
    float b0 = u_inv * u_inv * u_inv;
    float b1 = 3.0f * u_inv * u_inv * u;
    float b2 = 3.0f * u_inv * u * u;
    float b3 = u * u * u;

    SYN_Vector3F pt = {
        .x = (b0 * p0.x) + (b1 * p1.x) + (b2 * p2.x) + (b3 * p3.x),
        .y = (b0 * p0.y) + (b1 * p1.y) + (b2 * p2.y) + (b3 * p3.y),
        .z = (b0 * p0.z) + (b1 * p1.z) + (b2 * p2.z) + (b3 * p3.z),
    };
    return pt;
}

/**
 * @brief Evaluate 3D cubic Bezier tangent derivative B'(u).
 * @param p0 Start control point.
 * @param p1 First interior control point.
 * @param p2 Second interior control point.
 * @param p3 End control point.
 * @param u  Curve parameter in [0, 1].
 * @return First derivative vector dB/du at u.
 */
static SYN_Vector3F interp_eval_bezier_deriv(SYN_Vector3F p0, SYN_Vector3F p1, SYN_Vector3F p2,
                                             SYN_Vector3F p3, float u)
{
    float u_inv = 1.0f - u;
    float c0 = 3.0f * u_inv * u_inv;
    float c1 = 6.0f * u_inv * u;
    float c2 = 3.0f * u * u;

    SYN_Vector3F d = {
        .x = (c0 * (p1.x - p0.x)) + (c1 * (p2.x - p1.x)) + (c2 * (p3.x - p2.x)),
        .y = (c0 * (p1.y - p0.y)) + (c1 * (p2.y - p1.y)) + (c2 * (p3.y - p2.y)),
        .z = (c0 * (p1.z - p0.z)) + (c1 * (p2.z - p1.z)) + (c2 * (p3.z - p2.z)),
    };
    return d;
}

/** @brief Largest exact single-precision float <= INT32_MAX (2147483520.0f). */
#define SYN_INTERP_MAX_I32_FLOAT 2147483520.0f

/**
 * @brief Check that all three components of a 3D vector are finite floats.
 * @param v Input 3D vector.
 * @return true if x, y, and z are all finite, false otherwise.
 */
static bool interp_vec_is_finite(SYN_Vector3F v)
{
    return isfinite(v.x) && isfinite(v.y) && isfinite(v.z);
}

/**
 * @brief Validate common scalar motion parameters for trajectory planning.
 * @param feedrate  Target feedrate (> 0, finite).
 * @param max_accel Maximum acceleration (>= 0, finite).
 * @param max_jerk  Maximum jerk (>= 0, finite).
 * @param step_res  Spatial step resolution (> 0, finite).
 * @return true if all scalar parameters are valid, false otherwise.
 */
static bool interp_scalars_valid(float feedrate, float max_accel, float max_jerk, float step_res)
{
    return isfinite(feedrate) && (feedrate > 0.0f) && isfinite(max_accel) && (max_accel >= 0.0f) &&
           isfinite(max_jerk) && (max_jerk >= 0.0f) && isfinite(step_res) && (step_res > 0.0f);
}

/**
 * @brief Convert a non-negative float rate to int32_t with saturation to INT32_MAX.
 * @param val Non-negative float value (e.g., feedrate / step_res).
 * @return Saturated int32_t value in [0, INT32_MAX].
 */
static int32_t interp_clamp_rate_i32(float val)
{
    if (!isfinite(val) || (val >= SYN_INTERP_MAX_I32_FLOAT)) {
        return INT32_MAX;
    }
    if (val <= 0.0f) {
        return 0;
    }
    return (int32_t)val;
}

/**
 * @brief Finalize discrete step count and S-curve profile for a planned trajectory.
 * @param interp    Interpolator instance.
 * @param total_len Computed path length in user units.
 * @param feedrate  Target feedrate in units/sec.
 * @param max_accel Maximum acceleration in units/sec^2.
 * @param max_jerk  Maximum jerk in units/sec^3.
 * @param step_res  Spatial resolution per step.
 * @return SYN_OK on success, or SYN_INVALID_PARAM if step count exceeds INT32_MAX.
 */
static SYN_Status interp_configure_steps(SYN_Interpolator *interp, float total_len, float feedrate,
                                         float max_accel, float max_jerk, float step_res)
{
    float raw_steps = total_len / step_res;
    if (!isfinite(total_len) || !isfinite(raw_steps) || (raw_steps > SYN_INTERP_MAX_I32_FLOAT)) {
        interp->mode = SYN_INTERP_MODE_IDLE;
        return SYN_INVALID_PARAM;
    }

    interp->total_length = total_len;
    interp->total_steps = (uint32_t)raw_steps;
    if (interp->total_steps == 0U) {
        interp->total_steps = 1U;
    }
    interp->target_feedrate = feedrate;

    syn_scurve_init(&interp->scurve, 0);
    syn_scurve_set_constraints(&interp->scurve, interp_clamp_rate_i32(feedrate / step_res),
                               interp_clamp_rate_i32(max_accel / step_res),
                               interp_clamp_rate_i32(max_jerk / step_res));
    syn_scurve_set_target(&interp->scurve, (int32_t)interp->total_steps);
    return SYN_OK;
}

void syn_interpolator_init(SYN_Interpolator *interp)
{
    SYN_ASSERT(interp != NULL);
    memset(interp, 0, sizeof(SYN_Interpolator));
    interp->mode = SYN_INTERP_MODE_IDLE;
}

SYN_Status syn_interpolator_plan_linear(SYN_Interpolator *interp, SYN_Vector3F start,
                                        SYN_Vector3F target, float feedrate, float max_accel,
                                        float max_jerk, float step_res)
{
    SYN_ASSERT(interp != NULL);
    if (!interp_vec_is_finite(start) || !interp_vec_is_finite(target) ||
        !interp_scalars_valid(feedrate, max_accel, max_jerk, step_res)) {
        return SYN_INVALID_PARAM;
    }

    memset(interp, 0, sizeof(SYN_Interpolator));
    interp->mode = SYN_INTERP_MODE_LINEAR;
    interp->start_pos = start;
    interp->target_pos = target;
    interp->current_pos = start;

    float dx = target.x - start.x;
    float dy = target.y - start.y;
    float dz = target.z - start.z;

    float total_len = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
    if (total_len < 1e-6f) {
        interp->mode = SYN_INTERP_MODE_IDLE;
        return SYN_OK;
    }

    return interp_configure_steps(interp, total_len, feedrate, max_accel, max_jerk, step_res);
}

SYN_Status syn_interpolator_plan_circular(SYN_Interpolator *interp, SYN_Vector3F start,
                                          SYN_Vector3F target, SYN_Vector3F center_offset,
                                          bool is_cw, float feedrate, float max_accel,
                                          float max_jerk, float step_res)
{
    SYN_ASSERT(interp != NULL);
    if (!interp_vec_is_finite(start) || !interp_vec_is_finite(target) ||
        !interp_vec_is_finite(center_offset) ||
        !interp_scalars_valid(feedrate, max_accel, max_jerk, step_res)) {
        return SYN_INVALID_PARAM;
    }

    memset(interp, 0, sizeof(SYN_Interpolator));
    interp->mode = is_cw ? SYN_INTERP_MODE_CIRCULAR_CW : SYN_INTERP_MODE_CIRCULAR_CCW;
    interp->start_pos = start;
    interp->target_pos = target;
    interp->center_offset = center_offset;
    interp->current_pos = start;

    /* Center X, Y relative to start */
    float cx = start.x + center_offset.x;
    float cy = start.y + center_offset.y;

    float rx = -center_offset.x;
    float ry = -center_offset.y;
    interp->radius = sqrtf((rx * rx) + (ry * ry));

    if (!isfinite(interp->radius) || (interp->radius < 1e-6f)) {
        return SYN_INVALID_PARAM;
    }

    interp->start_angle = atan2f(-center_offset.y, -center_offset.x);
    float end_angle = atan2f(target.y - cy, target.x - cx);

    float sweep = end_angle - interp->start_angle;

    if (is_cw) {
        if (sweep >= 0.0f)
            sweep -=
                (float)(2.0 *
                        M_PI); /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */
    } else {
        if (sweep <= 0.0f)
            sweep +=
                (float)(2.0 *
                        M_PI); /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */
    }

    interp->sweep_angle = sweep;
    float total_len = fabsf(sweep) * interp->radius;

    return interp_configure_steps(interp, total_len, feedrate, max_accel, max_jerk, step_res);
}

bool syn_interpolator_step(SYN_Interpolator *interp, SYN_Vector3F *out_pos)
{
    SYN_ASSERT(interp != NULL);
    SYN_ASSERT(out_pos != NULL);

    if (interp->mode == SYN_INTERP_MODE_IDLE) {
        *out_pos = interp->current_pos;
        return false;
    }

    if (interp->step_index >= interp->total_steps) {
        interp->current_pos = interp->target_pos;
        *out_pos = interp->current_pos;
        interp->mode = SYN_INTERP_MODE_IDLE;
        return false;
    }

    interp->step_index++;
    float ratio = (float)interp->step_index / (float)interp->total_steps;
    float s = ratio * interp->total_length;
    interp->current_dist = s;

    if (interp->mode == SYN_INTERP_MODE_LINEAR) {
        interp->current_pos.x =
            interp->start_pos.x + (ratio * (interp->target_pos.x - interp->start_pos.x));
        interp->current_pos.y =
            interp->start_pos.y + (ratio * (interp->target_pos.y - interp->start_pos.y));
        interp->current_pos.z =
            interp->start_pos.z + (ratio * (interp->target_pos.z - interp->start_pos.z));
    } else if (interp->mode == SYN_INTERP_MODE_BEZIER) {
        interp->current_pos = interp_eval_bezier_pos(interp->start_pos, interp->bezier_p1,
                                                     interp->bezier_p2, interp->target_pos, ratio);
    } else {
        float cx = interp->start_pos.x + interp->center_offset.x;
        float cy = interp->start_pos.y + interp->center_offset.y;

        float angle = interp->start_angle + ratio * interp->sweep_angle;
        interp->current_pos.x = cx + interp->radius * cosf(angle);
        interp->current_pos.y = cy + interp->radius * sinf(angle);
        interp->current_pos.z =
            interp->start_pos.z + ratio * (interp->target_pos.z - interp->start_pos.z);
    }

    *out_pos = interp->current_pos;

    if (interp->step_index >= interp->total_steps) {
        interp->mode = SYN_INTERP_MODE_IDLE;
    }

    return true;
}

/**
 * @brief Evaluate linear segment position and optional velocity at normalized ratio.
 * @param interp     Interpolator instance.
 * @param ratio      Normalized path ratio in [0, 1].
 * @param scurve_vel Instantaneous S-curve scalar speed.
 * @param out_pos    Output 3D position.
 * @param out_vel    Optional output 3D velocity.
 */
static void interp_eval_linear_at(const SYN_Interpolator *interp, float ratio, int32_t scurve_vel,
                                  SYN_Vector3F *out_pos, SYN_Vector3F *out_vel)
{
    float dx = interp->target_pos.x - interp->start_pos.x;
    float dy = interp->target_pos.y - interp->start_pos.y;
    float dz = interp->target_pos.z - interp->start_pos.z;
    out_pos->x = interp->start_pos.x + (ratio * dx);
    out_pos->y = interp->start_pos.y + (ratio * dy);
    out_pos->z = interp->start_pos.z + (ratio * dz);

    if (out_vel != NULL) {
        float scale =
            (interp->total_length > 1e-6f) ? ((float)scurve_vel / interp->total_length) : 0.0f;
        out_vel->x = dx * scale;
        out_vel->y = dy * scale;
        out_vel->z = dz * scale;
    }
}

/**
 * @brief Evaluate cubic Bezier curve position and optional velocity at normalized ratio.
 * @param interp     Interpolator instance.
 * @param ratio      Normalized curve parameter u in [0, 1].
 * @param scurve_vel Instantaneous S-curve scalar speed.
 * @param out_pos    Output 3D position.
 * @param out_vel    Optional output 3D velocity.
 */
static void interp_eval_bezier_at(const SYN_Interpolator *interp, float ratio, int32_t scurve_vel,
                                  SYN_Vector3F *out_pos, SYN_Vector3F *out_vel)
{
    *out_pos = interp_eval_bezier_pos(interp->start_pos, interp->bezier_p1, interp->bezier_p2,
                                      interp->target_pos, ratio);

    if (out_vel != NULL) {
        SYN_Vector3F tang = interp_eval_bezier_deriv(interp->start_pos, interp->bezier_p1,
                                                     interp->bezier_p2, interp->target_pos, ratio);
        float tang_norm = sqrtf((tang.x * tang.x) + (tang.y * tang.y) + (tang.z * tang.z));
        if (tang_norm > 1e-6f) {
            float scale = (float)scurve_vel / tang_norm;
            out_vel->x = tang.x * scale;
            out_vel->y = tang.y * scale;
            out_vel->z = tang.z * scale;
        } else {
            memset(out_vel, 0, sizeof(SYN_Vector3F));
        }
    }
}

/**
 * @brief Evaluate circular arc position and optional velocity at normalized ratio.
 * @param interp     Interpolator instance.
 * @param ratio      Normalized path ratio in [0, 1].
 * @param scurve_vel Instantaneous S-curve scalar speed.
 * @param out_pos    Output 3D position.
 * @param out_vel    Optional output 3D velocity.
 */
static void interp_eval_circular_at(const SYN_Interpolator *interp, float ratio, int32_t scurve_vel,
                                    SYN_Vector3F *out_pos, SYN_Vector3F *out_vel)
{
    float cx = interp->start_pos.x + interp->center_offset.x;
    float cy = interp->start_pos.y + interp->center_offset.y;

    float angle = interp->start_angle + ratio * interp->sweep_angle;
    out_pos->x = cx + interp->radius * cosf(angle);
    out_pos->y = cy + interp->radius * sinf(angle);
    out_pos->z = interp->start_pos.z + ratio * (interp->target_pos.z - interp->start_pos.z);

    if (out_vel != NULL) {
        float vel_mag = (float)scurve_vel;
        float tangent_angle =
            angle + (interp->sweep_angle >= 0.0f ? (float)(M_PI / 2.0) : -(float)(M_PI / 2.0));
        out_vel->x = vel_mag * cosf(tangent_angle);
        out_vel->y = vel_mag * sinf(tangent_angle);
        out_vel->z = 0.0f;
    }
}

bool syn_interpolator_eval_at_time(SYN_Interpolator *interp, float t_sec, SYN_Vector3F *out_pos,
                                   SYN_Vector3F *out_vel)
{
    SYN_ASSERT(interp != NULL);
    SYN_ASSERT(out_pos != NULL);

    if (interp->mode == SYN_INTERP_MODE_IDLE) {
        *out_pos = interp->current_pos;
        if (out_vel) {
            memset(out_vel, 0, sizeof(SYN_Vector3F));
        }
        return false;
    }

    /* Advance S-curve generator update */
    int32_t scurve_pos = syn_scurve_update(&interp->scurve);
    int32_t scurve_vel = syn_scurve_velocity(&interp->scurve);

    float ratio =
        (interp->total_steps > 0U) ? ((float)scurve_pos / (float)interp->total_steps) : 1.0f;
    if (ratio > 1.0f) {
        ratio = 1.0f;
    }
    if (ratio < 0.0f) {
        ratio = 0.0f; /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */
    }

    if (interp->mode == SYN_INTERP_MODE_LINEAR) {
        interp_eval_linear_at(interp, ratio, scurve_vel, out_pos, out_vel);
    } else if (interp->mode == SYN_INTERP_MODE_BEZIER) {
        interp_eval_bezier_at(interp, ratio, scurve_vel, out_pos, out_vel);
    } else {
        interp_eval_circular_at(interp, ratio, scurve_vel, out_pos, out_vel);
    }

    (void)t_sec;
    return !syn_scurve_done(&interp->scurve);
}

SYN_Status syn_interpolator_plan_bezier(SYN_Interpolator *interp, SYN_Vector3F p0, SYN_Vector3F p1,
                                        SYN_Vector3F p2, SYN_Vector3F p3, float feedrate,
                                        float max_accel, float max_jerk, float step_res)
{
    if ((interp == NULL) || !interp_vec_is_finite(p0) || !interp_vec_is_finite(p1) ||
        !interp_vec_is_finite(p2) || !interp_vec_is_finite(p3) ||
        !interp_scalars_valid(feedrate, max_accel, max_jerk, step_res) || (max_accel <= 0.0f) ||
        (max_jerk <= 0.0f)) {
        return SYN_INVALID_PARAM;
    }

    /* Approximate cubic Bezier arc length across 32 chord subdivisions */
    float approx_len = 0.0f;
    SYN_Vector3F prev = p0;

    for (uint32_t i = 1U; i <= 32U; i++) {
        float u = (float)i * (1.0f / 32.0f);
        SYN_Vector3F pt = interp_eval_bezier_pos(p0, p1, p2, p3, u);
        float dx = pt.x - prev.x;
        float dy = pt.y - prev.y;
        float dz = pt.z - prev.z;
        approx_len += sqrtf((dx * dx) + (dy * dy) + (dz * dz));
        prev = pt;
    }

    memset(interp, 0, sizeof(SYN_Interpolator));
    interp->mode = SYN_INTERP_MODE_BEZIER;
    interp->start_pos = p0;
    interp->bezier_p1 = p1;
    interp->bezier_p2 = p2;
    interp->target_pos = p3;
    interp->current_pos = p0;
    interp->total_length = approx_len;

    if (approx_len < 1e-6f) {
        interp->mode = SYN_INTERP_MODE_IDLE;
        return SYN_OK;
    }

    return interp_configure_steps(interp, approx_len, feedrate, max_accel, max_jerk, step_res);
}

#endif /* SYN_USE_INTERPOLATOR */
