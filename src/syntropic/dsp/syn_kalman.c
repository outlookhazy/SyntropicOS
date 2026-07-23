#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_KALMAN) || SYN_USE_KALMAN

/**
 * @file syn_kalman.c
 * @brief General-purpose fixed-point Kalman filter implementation.
 *
 * All operations use caller-owned SYN_Matrix instances. No heap allocation.
 * Uses int64_t accumulator matrix multiply for full Q16 precision.
 */

#include "syn_kalman.h"
#include "../util/syn_assert.h"

#include <string.h>

SYN_Status syn_kalman_init(SYN_Kalman *kf, const SYN_Kalman_Config *cfg)
{
    SYN_ASSERT(kf != NULL);
    SYN_ASSERT(cfg != NULL);

    uint8_t ns = cfg->n_state;
    uint8_t nm = cfg->n_meas;

    /* Validate dimensions */
    if (cfg->x->rows != ns || cfg->x->cols != 1) return SYN_INVALID_PARAM;
    if (cfg->P->rows != ns || cfg->P->cols != ns) return SYN_INVALID_PARAM;
    if (cfg->F->rows != ns || cfg->F->cols != ns) return SYN_INVALID_PARAM;
    if (cfg->Q->rows != ns || cfg->Q->cols != ns) return SYN_INVALID_PARAM;
    if (cfg->H->rows != nm || cfg->H->cols != ns) return SYN_INVALID_PARAM;
    if (cfg->R->rows != nm || cfg->R->cols != nm) return SYN_INVALID_PARAM;

    kf->cfg = cfg;

    return SYN_OK;
}

void syn_kalman_predict(SYN_Kalman *kf)
{
    SYN_ASSERT(kf != NULL && kf->cfg != NULL);

    const SYN_Kalman_Config *c = kf->cfg;

    /* x̂⁻ = F · x̂ */
    syn_matrix_mul(c->F, c->x, kf->temp_n1);
    syn_matrix_copy(c->x, kf->temp_n1);

    /* P⁻ = F · P · Fᵀ + Q */
    syn_matrix_mul(c->F, c->P, kf->temp_nn_1);          /* F·P */
    syn_matrix_transpose(c->F, kf->temp_nn_2);           /* Fᵀ */
    syn_matrix_mul(kf->temp_nn_1, kf->temp_nn_2, c->P);  /* F·P·Fᵀ → P */
    syn_matrix_add(c->P, c->Q, kf->temp_nn_1);           /* P + Q */
    syn_matrix_copy(c->P, kf->temp_nn_1);
}

SYN_Status syn_kalman_update(SYN_Kalman *kf, const SYN_Matrix *z)
{
    SYN_ASSERT(kf != NULL && z != NULL);

    const SYN_Kalman_Config *c = kf->cfg;
    uint8_t ns = c->n_state;
    uint8_t nm = c->n_meas;

    /* ── Innovation: y = z − H·x̂⁻ ────────────────────────────────────── */
    syn_matrix_mul(c->H, c->x, kf->temp_m1);       /* H·x → temp_m1 */
    syn_matrix_sub(z, kf->temp_m1, kf->temp_m1);    /* y = z - H·x   */

    /* ── Innovation covariance: S = H·P⁻·Hᵀ + R ──────────────────────── */
    syn_matrix_mul(c->H, c->P, kf->temp_mn);        /* H·P  (nm×ns)   */
    syn_matrix_transpose(c->H, kf->temp_nm);         /* Hᵀ   (ns×nm)   */
    syn_matrix_mul(kf->temp_mn, kf->temp_nm, kf->temp_mm);  /* H·P·Hᵀ (nm×nm) */
    syn_matrix_add(kf->temp_mm, c->R, kf->temp_mm_2);       /* S = HPHᵀ + R    */

    /* ── S⁻¹ ──────────────────────────────────────────────────────────── */
    SYN_Status inv_st = syn_matrix_inv(kf->temp_mm_2, kf->temp_mm);
    if (inv_st != SYN_OK) return SYN_ERROR;
    /* temp_mm = S⁻¹ (nm × nm) */

    /* ── Kalman gain: K = P·Hᵀ·S⁻¹ ────────────────────────────────────
     *
     * temp_nm already holds Hᵀ (ns × nm).
     * We compute K in two steps to avoid aliasing:
     *   1. Hᵀ·S⁻¹ → (ns×nm)(nm×nm) = (ns×nm), store on stack as K
     *   2. K_final = P · K_partial → (ns×ns)(ns×nm) = (ns×nm)
     *
     * K is at most SYN_KALMAN_MAX_STATE × SYN_KALMAN_MAX_MEAS
     * elements. For a 6-state 4-measurement filter this is 24 × 4 = 96 bytes.
     */
    q16_t k_buf[SYN_KALMAN_MAX_STATE * SYN_KALMAN_MAX_MEAS];
    SYN_Matrix K_partial = { k_buf, ns, nm };
    SYN_Matrix K         = { k_buf, ns, nm };

    syn_matrix_mul(kf->temp_nm, kf->temp_mm, &K_partial);   /* Hᵀ·S⁻¹ */
    syn_matrix_mul(c->P, &K_partial, kf->temp_nm);          /* P·Hᵀ·S⁻¹ */
    syn_matrix_copy(&K, kf->temp_nm);                        /* K final  */

    /* ── State update: x̂ = x̂⁻ + K·y ──────────────────────────────────── */
    syn_matrix_mul(&K, kf->temp_m1, kf->temp_n1);   /* K·y */
    syn_matrix_add(c->x, kf->temp_n1, c->x);        /* x += K·y */

    /* ── Covariance update: P = (I − K·H)·P⁻ ─────────────────────────── */
    syn_matrix_mul(&K, c->H, kf->temp_nn_1);         /* K·H (ns×ns) */
    syn_matrix_identity(kf->temp_nn_2);                /* I */
    syn_matrix_sub(kf->temp_nn_2, kf->temp_nn_1, kf->temp_nn_1);  /* I - K·H */

    syn_matrix_copy(kf->temp_nn_2, c->P);             /* save P⁻  */
    syn_matrix_mul(kf->temp_nn_1, kf->temp_nn_2, c->P);  /* (I-K·H)·P⁻ → P */

    return SYN_OK;
}

#endif /* SYN_USE_KALMAN */
