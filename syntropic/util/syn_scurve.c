#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SCURVE) || SYN_USE_SCURVE

#include "syn_scurve.h"
#include "syn_assert.h"
#include <string.h>


void syn_scurve_init(SYN_SCurve *sc, int32_t initial_p) {
    SYN_ASSERT(sc != NULL);
    memset(sc, 0, sizeof(*sc));
    sc->p = initial_p;
    sc->target_p = initial_p;
    sc->done = true;
    sc->current_phase = 7;
}

void syn_scurve_set_constraints(SYN_SCurve *sc, int32_t v_max, int32_t a_max, int32_t j_max) {
    SYN_ASSERT(sc != NULL);
    sc->v_max = v_max;
    sc->a_max = a_max;
    sc->j_max = j_max;
}

static uint32_t syn_isqrt(uint32_t n) {
    uint32_t root = 0;
    uint32_t bit = 1UL << 30;
    while (bit > n) bit >>= 2;
    while (bit != 0) {
        if (n >= root + bit) { n -= root + bit; root = (root >> 1) + bit; }
        else { root >>= 1; }
        bit >>= 2;
    }
    return root;
}

void syn_scurve_set_target(SYN_SCurve *sc, int32_t target) {
    SYN_ASSERT(sc != NULL);
    sc->target_p = target;
    int32_t dist = target - sc->p;
    
    if (dist == 0) {
        sc->done = true;
        sc->current_phase = 7;
        return;
    }
    
    sc->dir = SYN_SIGN(dist);
    uint32_t d = (uint32_t)SYN_ABS(dist);
    
    // We assume starting from v=0, a=0.
    // Ticks to reach a_max at j_max
    uint32_t Tj = (uint32_t)sc->a_max / (uint32_t)sc->j_max;
    if (Tj == 0) Tj = 1;
    
    // Max velocity we can reach just during jerk phases (accel up, then accel down)
    // a(t) goes to a_max in Tj ticks. v reached is j_max * Tj^2.
    uint32_t vj = (uint32_t)sc->j_max * Tj * Tj;
    
    uint32_t Ta = 0;
    if ((uint32_t)sc->v_max > vj) {
        Ta = ((uint32_t)sc->v_max - vj) / (uint32_t)sc->a_max;
    } else {
        // v_max is reached during jerk!
        Tj = syn_isqrt((uint32_t)sc->v_max / (uint32_t)sc->j_max);
        if (Tj == 0) Tj = 1;
        Ta = 0;
    }
    
    // Total accel time = 2 * Tj + Ta
    // Distance covered during accel = v_max * (2*Tj + Ta) / 2
    // Decel is symmetric. So total distance without constant v phase is approx:
    // D_accel_decel = v_max * (2*Tj + Ta)
    uint32_t vmax_reached = (uint32_t)sc->j_max * Tj * Tj + (uint32_t)sc->a_max * Ta;
    uint32_t D_ad = vmax_reached * (2u * Tj + Ta);
    
    uint32_t Tv = 0;
    if (d > D_ad) {
        Tv = (d - D_ad) / vmax_reached;
    } else {
        // We don't reach v_max. Profile is triangular or just jerk pulses.
        // For simplicity in this heuristic, scale everything down.
        // A simple trick is to divide Tj and Ta by 2 until it fits, but
        // an exact root is better. 
        // We will just do a simple iterative reduction for the test.
        while (D_ad > d && (Tj > 1u || Ta > 0u)) {
            if (Ta > 0u) Ta--;
            else if (Tj > 1u) Tj--;
            vmax_reached = (uint32_t)sc->j_max * Tj * Tj + (uint32_t)sc->a_max * Ta;
            D_ad = vmax_reached * (2u * Tj + Ta);
        }
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

int32_t syn_scurve_update(SYN_SCurve *sc) {
    if (sc->done) return sc->p;

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
        case 0: j = sc->j_max; break;
        case 1: j = 0; break;
        case 2: j = -sc->j_max; break;
        case 3: j = 0; sc->a = 0; break; // Ensure a=0 during cruise
        case 4: j = -sc->j_max; break;
        case 5: j = 0; break;
        case 6: j = sc->j_max; break;
    }

    sc->j = j * sc->dir;
    sc->a += sc->j;
    sc->v += sc->a;
    sc->p += sc->v;
    
    sc->ticks_in_phase++;
    return sc->p;
}

#endif /* SYN_USE_SCURVE */
