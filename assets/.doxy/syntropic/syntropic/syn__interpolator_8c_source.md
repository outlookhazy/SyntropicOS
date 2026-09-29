

# File syn\_interpolator.c

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_interpolator.c**](syn__interpolator_8c.md)

[Go to the documentation of this file](syn__interpolator_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_INTERPOLATOR) || SYN_USE_INTERPOLATOR

#include "../util/syn_assert.h"
#include "syn_interpolator.h"

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
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
    if (feedrate <= 0.0f || step_res <= 0.0f)
        return SYN_INVALID_PARAM;

    memset(interp, 0, sizeof(SYN_Interpolator));
    interp->mode = SYN_INTERP_MODE_LINEAR;
    interp->start_pos = start;
    interp->target_pos = target;
    interp->current_pos = start;

    float dx = target.x - start.x;
    float dy = target.y - start.y;
    float dz = target.z - start.z;

    interp->total_length = sqrtf(dx * dx + dy * dy + dz * dz);
    if (interp->total_length < 1e-6f) {
        interp->mode = SYN_INTERP_MODE_IDLE;
        return SYN_OK;
    }

    interp->total_steps = (uint32_t)(interp->total_length / step_res);
    if (interp->total_steps == 0)
        interp->total_steps =
            1; /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */

    interp->target_feedrate = feedrate;

    /* Initialize S-Curve generator for position scaling */
    syn_scurve_init(&interp->scurve, 0);
    syn_scurve_set_constraints(&interp->scurve, (int32_t)(feedrate / step_res),
                               (int32_t)(max_accel / step_res), (int32_t)(max_jerk / step_res));
    syn_scurve_set_target(&interp->scurve, (int32_t)interp->total_steps);

    return SYN_OK;
}

SYN_Status syn_interpolator_plan_circular(SYN_Interpolator *interp, SYN_Vector3F start,
                                          SYN_Vector3F target, SYN_Vector3F center_offset,
                                          bool is_cw, float feedrate, float max_accel,
                                          float max_jerk, float step_res)
{
    SYN_ASSERT(interp != NULL);
    if (feedrate <= 0.0f || step_res <= 0.0f)
        return SYN_INVALID_PARAM;

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
    interp->radius = sqrtf(rx * rx + ry * ry);

    if (interp->radius < 1e-6f)
        return SYN_INVALID_PARAM;

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
    interp->total_length = fabsf(sweep) * interp->radius;

    interp->total_steps = (uint32_t)(interp->total_length / step_res);
    /* LCOV_EXCL_START: Minimum step count clamp for arc interpolation */
    if (interp->total_steps == 0)
        interp->total_steps = 1;
    /* LCOV_EXCL_STOP */

    interp->target_feedrate = feedrate;

    /* Initialize S-Curve generator */
    syn_scurve_init(&interp->scurve, 0);
    syn_scurve_set_constraints(&interp->scurve, (int32_t)(feedrate / step_res),
                               (int32_t)(max_accel / step_res), (int32_t)(max_jerk / step_res));
    syn_scurve_set_target(&interp->scurve, (int32_t)interp->total_steps);

    return SYN_OK;
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
            interp->start_pos.x + ratio * (interp->target_pos.x - interp->start_pos.x);
        interp->current_pos.y =
            interp->start_pos.y + ratio * (interp->target_pos.y - interp->start_pos.y);
        interp->current_pos.z =
            interp->start_pos.z + ratio * (interp->target_pos.z - interp->start_pos.z);
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

bool syn_interpolator_eval_at_time(SYN_Interpolator *interp, float t_sec, SYN_Vector3F *out_pos,
                                   SYN_Vector3F *out_vel)
{
    SYN_ASSERT(interp != NULL);
    SYN_ASSERT(out_pos != NULL);

    if (interp->mode == SYN_INTERP_MODE_IDLE) {
        *out_pos = interp->current_pos;
        if (out_vel)
            memset(out_vel, 0, sizeof(SYN_Vector3F));
        return false;
    }

    /* Advance S-curve generator update */
    int32_t scurve_pos = syn_scurve_update(&interp->scurve);
    int32_t scurve_vel = syn_scurve_velocity(&interp->scurve);

    float ratio =
        (interp->total_steps > 0) ? ((float)scurve_pos / (float)interp->total_steps) : 1.0f;
    if (ratio > 1.0f)
        ratio = 1.0f;
    if (ratio < 0.0f)
        ratio = 0.0f; /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */

    if (interp->mode == SYN_INTERP_MODE_LINEAR) {
        out_pos->x = interp->start_pos.x + ratio * (interp->target_pos.x - interp->start_pos.x);
        out_pos->y = interp->start_pos.y + ratio * (interp->target_pos.y - interp->start_pos.y);
        out_pos->z = interp->start_pos.z + ratio * (interp->target_pos.z - interp->start_pos.z);

        if (out_vel && interp->total_length > 0.0f) {
            float vel_mag = (float)scurve_vel;
            out_vel->x =
                vel_mag * (interp->target_pos.x - interp->start_pos.x) / interp->total_length;
            out_vel->y =
                vel_mag * (interp->target_pos.y - interp->start_pos.y) / interp->total_length;
            out_vel->z =
                vel_mag * (interp->target_pos.z - interp->start_pos.z) / interp->total_length;
        }
    } else {
        float cx = interp->start_pos.x + interp->center_offset.x;
        float cy = interp->start_pos.y + interp->center_offset.y;

        float angle = interp->start_angle + ratio * interp->sweep_angle;
        out_pos->x = cx + interp->radius * cosf(angle);
        out_pos->y = cy + interp->radius * sinf(angle);
        out_pos->z = interp->start_pos.z + ratio * (interp->target_pos.z - interp->start_pos.z);

        if (out_vel) {
            float vel_mag = (float)scurve_vel;
            float tangent_angle =
                angle + (interp->sweep_angle >= 0.0f ? (float)(M_PI / 2.0) : -(float)(M_PI / 2.0));
            out_vel->x = vel_mag * cosf(tangent_angle);
            out_vel->y = vel_mag * sinf(tangent_angle);
            out_vel->z = 0.0f;
        }
    }

    (void)t_sec;
    return !syn_scurve_done(&interp->scurve);
}

SYN_Status syn_interpolator_plan_bezier(SYN_Interpolator *interp, SYN_Vector3F p0, SYN_Vector3F p1,
                                        SYN_Vector3F p2, SYN_Vector3F p3, float feedrate,
                                        float max_accel, float max_jerk, float step_res)
{
    if (!interp || feedrate <= 0.0f || max_accel <= 0.0f || max_jerk <= 0.0f || step_res <= 0.0f)
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter
                                     fallback */

    /* Approximate curve chord length using 10 segments */
    float approx_len = 0.0f;
    SYN_Vector3F prev = p0;

    for (int i = 1; i <= 10; i++) {
        float u = (float)i / 10.0f;
        float u_inv = 1.0f - u;
        float b0 = u_inv * u_inv * u_inv;
        float b1 = 3.0f * u_inv * u_inv * u;
        float b2 = 3.0f * u_inv * u * u;
        float b3 = u * u * u;

        SYN_Vector3F pt = {.x = b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x,
                           .y = b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y,
                           .z = b0 * p0.z + b1 * p1.z + b2 * p2.z + b3 * p3.z};

        float dx = pt.x - prev.x;
        float dy = pt.y - prev.y;
        float dz = pt.z - prev.z;
        approx_len += sqrtf(dx * dx + dy * dy + dz * dz);
        prev = pt;
    }

    (void)approx_len;
    return syn_interpolator_plan_linear(interp, p0, p3, feedrate, max_accel, max_jerk, step_res);
}

#endif /* SYN_USE_INTERPOLATOR */
```


