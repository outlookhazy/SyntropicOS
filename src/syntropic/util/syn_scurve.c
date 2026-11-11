#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_SCURVE) || SYN_USE_SCURVE

#include "syn_assert.h"
#include "syn_scurve.h"

#include <string.h>

void syn_scurve_init(SYN_SCurve *sc, int32_t initial_p)
{
    if (sc == NULL) {
        return;
    }
    SYN_ASSERT(sc != NULL);
    memset(sc, 0, sizeof(*sc));
    sc->p = initial_p;
    sc->target_p = initial_p;
    sc->v_max = 1;
    sc->a_max = 1;
    sc->j_max = 1;
    sc->done = true;
    sc->current_phase = 7;
}

void syn_scurve_set_constraints(SYN_SCurve *sc, int32_t v_max, int32_t a_max, int32_t j_max)
{
    if (sc == NULL) {
        return;
    }
    SYN_ASSERT(sc != NULL);
    sc->v_max = (v_max > 0) ? v_max : 1;
    sc->a_max = (a_max > 0) ? a_max : 1;
    sc->j_max = (j_max > 0) ? j_max : 1;
}

#include "syn_qmath.h"

static inline uint32_t syn_isqrt(uint32_t n)
{
    return syn_isqrt32(n);
}

/**
 * @brief Compute combined acceleration + deceleration distance for given (Tj, Ta).
 */
static uint64_t scurve_accel_decel_dist(const SYN_SCurve *sc, uint32_t Tj, uint32_t Ta)
{
    uint64_t vmax_reached = (uint64_t)(uint32_t)sc->j_max * (uint64_t)Tj * (uint64_t)Tj +
                            (uint64_t)(uint32_t)sc->a_max * (uint64_t)Ta;
    return vmax_reached * (2ULL * (uint64_t)Tj + (uint64_t)Ta);
}

/**
 * @brief Reduce (Tj, Ta) via binary search so accel+decel distance fits within d.
 */
static void scurve_reduce_short_profile(const SYN_SCurve *sc, uint64_t d, uint32_t *tj_io,
                                        uint32_t *ta_io)
{
    uint32_t Tj = *tj_io;
    uint32_t Ta = *ta_io;

    if (Ta > 0U && scurve_accel_decel_dist(sc, Tj, 0U) <= d) {
        uint32_t lo = 0U;
        uint32_t hi = Ta;
        while (lo < hi) {
            uint32_t mid = lo + ((hi - lo + 1U) / 2U);
            if (scurve_accel_decel_dist(sc, Tj, mid) <= d) {
                lo = mid;
            } else {
                hi = mid - 1U;
            }
        }
        *ta_io = lo;
        return;
    }

    *ta_io = 0U;
    uint32_t lo = 1U;
    uint32_t hi = Tj;
    while (lo < hi) {
        uint32_t mid = lo + ((hi - lo + 1U) / 2U);
        if (scurve_accel_decel_dist(sc, mid, 0U) <= d) {
            lo = mid;
        } else {
            hi = mid - 1U;
        }
    }
    *tj_io = lo;
}

void syn_scurve_set_target(SYN_SCurve *sc, int32_t target)
{
    if (sc == NULL) {
        return;
    }
    SYN_ASSERT(sc != NULL);

    if (sc->v_max <= 0)
        sc->v_max = 1;
    if (sc->a_max <= 0)
        sc->a_max = 1;
    if (sc->j_max <= 0)
        sc->j_max = 1;

    sc->target_p = target;
    int64_t dist = (int64_t)target - (int64_t)sc->p;

    if (dist == 0) {
        sc->done = true;
        sc->current_phase = 7;
        return;
    }

    sc->dir = (dist > 0) ? 1 : -1;
    uint64_t d = (dist < 0) ? (uint64_t)(-dist) : (uint64_t)dist;

    // We assume starting from v=0, a=0.
    // Ticks to reach a_max at j_max
    uint32_t Tj = (uint32_t)sc->a_max / (uint32_t)sc->j_max;
    if (Tj == 0U)
        Tj = 1U;

    // Max velocity we can reach just during jerk phases (accel up, then accel down)
    // a(t) goes to a_max in Tj ticks. v reached is j_max * Tj^2.
    uint64_t vj = (uint64_t)(uint32_t)sc->j_max * (uint64_t)Tj * (uint64_t)Tj;

    uint32_t Ta = 0U;
    if ((uint64_t)(uint32_t)sc->v_max > vj) {
        Ta = ((uint32_t)sc->v_max - (uint32_t)vj) / (uint32_t)sc->a_max;
    } else {
        // v_max is reached during jerk!
        Tj = syn_isqrt((uint32_t)sc->v_max / (uint32_t)sc->j_max);
        if (Tj == 0U)
            Tj = 1U;
        Ta = 0U;
    }

    // Total accel time = 2 * Tj + Ta
    // Distance covered during accel = v_max * (2*Tj + Ta) / 2
    // Decel is symmetric. So total distance without constant v phase is approx:
    // D_accel_decel = v_max * (2*Tj + Ta)
    uint64_t vmax_reached = (uint64_t)(uint32_t)sc->j_max * (uint64_t)Tj * (uint64_t)Tj +
                            (uint64_t)(uint32_t)sc->a_max * (uint64_t)Ta;
    uint64_t D_ad = vmax_reached * (2ULL * (uint64_t)Tj + (uint64_t)Ta);

    uint32_t Tv = 0U;
    if (d > D_ad) {
        uint64_t tv64 = (d - D_ad) / vmax_reached;
        Tv = (tv64 > (uint64_t)INT32_MAX) ? (uint32_t)INT32_MAX : (uint32_t)tv64;
    } else {
        scurve_reduce_short_profile(sc, d, &Tj, &Ta);
    }

    sc->phase_ticks[0] = (int32_t)Tj;
    sc->phase_ticks[1] = (int32_t)Ta;
    sc->phase_ticks[2] = (int32_t)Tj;
    sc->phase_ticks[3] = (int32_t)Tv;
    sc->phase_ticks[4] = (int32_t)Tj;
    sc->phase_ticks[5] = (int32_t)Ta;
    sc->phase_ticks[6] = (int32_t)Tj;

    // Handle rounding errors by putting remaining distance into Tv phase,
    // or just let it snap at the end. We'll snap at the end since we use integer math.

    sc->current_phase = 0;
    sc->ticks_in_phase = 0;
    sc->done = false;
    sc->v = 0;
    sc->a = 0;
}

int32_t syn_scurve_update(SYN_SCurve *sc)
{
    if (sc->done)
        return sc->p;

    // Advance phase if needed
    while (sc->current_phase < 7 && sc->ticks_in_phase >= sc->phase_ticks[sc->current_phase]) {
        sc->current_phase++;
        sc->ticks_in_phase = 0;
    }

    if (sc->current_phase >= 7) {
        sc->p = sc->target_p;
        sc->v = 0;
        sc->a = 0;
        sc->done = true;
        return sc->p;
    }

    int32_t j = 0;
    switch (sc->current_phase) {
    case 0:
        j = sc->j_max;
        break;
    case 1:
        j = 0;
        break;
    case 2:
        j = -sc->j_max;
        break;
    case 3:
        j = 0;
        sc->a = 0;
        break; // Ensure a=0 during cruise
    case 4:
        j = -sc->j_max;
        break;
    case 5:
        j = 0;
        break;
    case 6:
        j = sc->j_max;
        break;
    }

    sc->j = j * sc->dir;
    sc->a += sc->j;
    sc->v += sc->a;
    sc->p += sc->v;

    sc->ticks_in_phase++;
    return sc->p;
}

void syn_scurve3d_plan(SYN_SCurve3D *sc3d, int32_t start_x, int32_t start_y, int32_t start_z,
                       int32_t target_x, int32_t target_y, int32_t target_z, int32_t v_max,
                       int32_t a_max, int32_t j_max)
{
    SYN_ASSERT(sc3d != NULL);
    sc3d->start_x = start_x;
    sc3d->start_y = start_y;
    sc3d->start_z = start_z;

    int64_t dx = (int64_t)target_x - (int64_t)start_x;
    int64_t dy = (int64_t)target_y - (int64_t)start_y;
    int64_t dz = (int64_t)target_z - (int64_t)start_z;
    sc3d->delta_x = (int32_t)dx;
    sc3d->delta_y = (int32_t)dy;
    sc3d->delta_z = (int32_t)dz;

    uint64_t adx = (dx < 0) ? (uint64_t)(-dx) : (uint64_t)dx;
    uint64_t ady = (dy < 0) ? (uint64_t)(-dy) : (uint64_t)dy;
    uint64_t adz = (dz < 0) ? (uint64_t)(-dz) : (uint64_t)dz;
    uint64_t dist = syn_isqrt64(adx * adx + ady * ady + adz * adz);
    if (dist > (uint64_t)INT32_MAX) {
        dist = (uint64_t)INT32_MAX;
    }
    sc3d->total_dist = (uint32_t)dist;

    syn_scurve_init(&sc3d->sc_master, 0);
    syn_scurve_set_constraints(&sc3d->sc_master, v_max, a_max, j_max);
    syn_scurve_set_target(&sc3d->sc_master, (int32_t)sc3d->total_dist);
}

bool syn_scurve3d_update(SYN_SCurve3D *sc3d, int32_t *out_x, int32_t *out_y, int32_t *out_z)
{
    SYN_ASSERT(sc3d != NULL && out_x != NULL && out_y != NULL && out_z != NULL);
    if (sc3d->sc_master.done) {
        *out_x = sc3d->start_x + sc3d->delta_x;
        *out_y = sc3d->start_y + sc3d->delta_y;
        *out_z = sc3d->start_z + sc3d->delta_z;
        return false;
    }

    int32_t master_p = syn_scurve_update(&sc3d->sc_master);
    if (sc3d->total_dist > 0) {
        int64_t ratio_q16 = ((int64_t)master_p << Q16_SHIFT) / sc3d->total_dist;
        *out_x = sc3d->start_x + (int32_t)(((int64_t)sc3d->delta_x * ratio_q16) >> Q16_SHIFT);
        *out_y = sc3d->start_y + (int32_t)(((int64_t)sc3d->delta_y * ratio_q16) >> Q16_SHIFT);
        *out_z = sc3d->start_z + (int32_t)(((int64_t)sc3d->delta_z * ratio_q16) >> Q16_SHIFT);
    } else {
        *out_x = sc3d->start_x;
        *out_y = sc3d->start_y;
        *out_z = sc3d->start_z;
    }

    return !sc3d->sc_master.done;
}

#endif /* SYN_USE_SCURVE */
