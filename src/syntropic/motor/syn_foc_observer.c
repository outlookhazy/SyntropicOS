/**
 * @file syn_foc_observer.c
 * @brief Sensorless FOC Rotor Position & Speed Sliding Mode Observer implementation.
 */

#include "syn_foc_observer.h"
#include "../util/syn_assert.h"
#include <string.h>

SYN_Status syn_foc_observer_init(SYN_FOCObserver *obs, const SYN_FOCObserverConfig *cfg)
{
    if (obs == NULL || cfg == NULL) return SYN_INVALID_PARAM;
    if (cfg->R <= 0 || cfg->L <= 0 || cfg->G <= 0 || cfg->dt <= 0) return SYN_INVALID_PARAM;

    memset(obs, 0, sizeof(*obs));
    obs->cfg = *cfg;

    return SYN_OK;
}

void syn_foc_observer_reset(SYN_FOCObserver *obs)
{
    if (obs == NULL) return;

    obs->I_alpha_est = 0;
    obs->I_beta_est  = 0;
    obs->E_alpha_est = 0;
    obs->E_beta_est  = 0;
    obs->theta_e     = 0;
    obs->omega_e     = 0;
    obs->pll_i_accum = 0;
}

SYN_Status syn_foc_observer_update(SYN_FOCObserver *obs, q16_t V_alpha, q16_t V_beta,
                                    q16_t I_alpha, q16_t I_beta)
{
    if (obs == NULL) return SYN_INVALID_PARAM;

    q16_t R  = obs->cfg.R;
    q16_t L  = obs->cfg.L;
    q16_t G  = obs->cfg.G;
    q16_t dt = obs->cfg.dt;

    /* Current estimation error: I_est - I_meas */
    q16_t err_ia = obs->I_alpha_est - I_alpha;
    q16_t err_ib = obs->I_beta_est - I_beta;

    /* Sliding mode switching control with linear boundary layer */
    q16_t Z_alpha = q16_clamp(q16_mul(G, err_ia), -G, G);
    q16_t Z_beta  = q16_clamp(q16_mul(G, err_ib), -G, G);

    /* Current estimator integration: dI = (V - R*I_est - Z) * (dt / L) */
    q16_t dt_over_L = q16_div(dt, L);

    q16_t delta_I_alpha = q16_mul(V_alpha - q16_mul(R, obs->I_alpha_est) - Z_alpha, dt_over_L);
    q16_t delta_I_beta  = q16_mul(V_beta  - q16_mul(R, obs->I_beta_est)  - Z_beta,  dt_over_L);

    obs->I_alpha_est += delta_I_alpha;
    obs->I_beta_est  += delta_I_beta;

    /* Lowpass filter sliding control output Z to extract estimated back-EMF */
    q16_t alpha_lpf = Q16_FROM_FLOAT(0.15); /* LPF smoothing factor */
    obs->E_alpha_est += q16_mul(alpha_lpf, Z_alpha - obs->E_alpha_est);
    obs->E_beta_est  += q16_mul(alpha_lpf, Z_beta  - obs->E_beta_est);

    /* Software PLL Phase Detector with normalized back-EMF vector */
    int64_t E_sq = ((int64_t)obs->E_alpha_est * obs->E_alpha_est) + ((int64_t)obs->E_beta_est * obs->E_beta_est);
    E_sq >>= Q16_SHIFT;

    q16_t pll_err = 0;
    q16_t min_E_sq = Q16_FROM_FLOAT(0.0001);

    if (E_sq > min_E_sq) {
        q16_t E_mag = q16_sqrt((q16_t)E_sq);
        if (E_mag > 0) {
            q16_t E_alpha_n = q16_div(obs->E_alpha_est, E_mag);
            q16_t E_beta_n  = q16_div(obs->E_beta_est,  E_mag);

            /* If starting up from zero speed, seed angle and initial speed estimate */
            if (obs->omega_e == 0 && obs->pll_i_accum == 0) {
                obs->theta_e = q16_atan2(-obs->E_alpha_est, obs->E_beta_est);
                obs->pll_i_accum = Q16_FROM_INT(80);
            }

            q16_t cos_th = q16_cos(obs->theta_e);
            q16_t sin_th = q16_sin(obs->theta_e);

            pll_err = -q16_mul(E_alpha_n, cos_th) - q16_mul(E_beta_n, sin_th);
        }
    }

    /* PI loop filter with anti-windup clamp (using 64-bit int to prevent Q16 underflow) */
    int64_t Ki_err = ((int64_t)obs->cfg.Ki_pll * pll_err) >> Q16_SHIFT;
    q16_t delta_i = (q16_t)((Ki_err * dt) >> Q16_SHIFT);

    obs->pll_i_accum += delta_i;
    q16_t max_speed = Q16_FROM_INT(2000);
    obs->pll_i_accum = q16_clamp(obs->pll_i_accum, -max_speed, max_speed);

    obs->omega_e = q16_clamp(obs->pll_i_accum + q16_mul(obs->cfg.Kp_pll, pll_err), -max_speed, max_speed);

    /* Position integrator: theta = theta + omega * dt, wrapped to [0, 2pi) */
    obs->theta_e += q16_mul(obs->omega_e, dt);
    while (obs->theta_e >= Q16_2_PI) {
        obs->theta_e -= Q16_2_PI;
    }
    while (obs->theta_e < 0) {
        obs->theta_e += Q16_2_PI;
    }

    return SYN_OK;
}

q16_t syn_foc_observer_get_angle(const SYN_FOCObserver *obs)
{
    if (obs == NULL) return 0;
    return obs->theta_e;
}

q16_t syn_foc_observer_get_speed(const SYN_FOCObserver *obs)
{
    if (obs == NULL) return 0;
    return obs->omega_e;
}
