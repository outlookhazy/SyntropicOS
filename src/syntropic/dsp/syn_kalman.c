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

#include "../util/syn_assert.h"
#include "syn_kalman.h"

#include <string.h>

SYN_Status syn_kalman_init(SYN_Kalman *kf, const SYN_Kalman_Config *cfg)
{
    SYN_ASSERT(kf != NULL);
    SYN_ASSERT(cfg != NULL);

    uint8_t ns = cfg->n_state;
    uint8_t nm = cfg->n_meas;

    if (ns == 0 || ns > SYN_KALMAN_MAX_STATE || nm == 0 || nm > SYN_KALMAN_MAX_MEAS) {
        return SYN_INVALID_PARAM;
    }

    if (cfg->x == NULL || cfg->P == NULL || cfg->F == NULL || cfg->Q == NULL || cfg->H == NULL ||
        cfg->R == NULL) {
        return SYN_INVALID_PARAM;
    }

    /* Validate dimensions */
    if (cfg->x->rows != ns || cfg->x->cols != 1)
        return SYN_INVALID_PARAM;
    if (cfg->P->rows != ns || cfg->P->cols != ns)
        return SYN_INVALID_PARAM;
    if (cfg->F->rows != ns || cfg->F->cols != ns)
        return SYN_INVALID_PARAM;
    if (cfg->Q->rows != ns || cfg->Q->cols != ns)
        return SYN_INVALID_PARAM;
    if (cfg->H->rows != nm || cfg->H->cols != ns)
        return SYN_INVALID_PARAM;
    if (cfg->R->rows != nm || cfg->R->cols != nm)
        return SYN_INVALID_PARAM;

    kf->cfg = cfg;

    return SYN_OK;
}

/**
 * @brief Check that a matrix descriptor, backing buffer, and dimensions are valid.
 * @param m    Matrix descriptor to validate.
 * @param rows Expected row count.
 * @param cols Expected column count.
 * @return True if matrix is non-NULL with matching dimensions and non-NULL data.
 */
static bool kalman_mat_matches(const SYN_Matrix *m, uint8_t rows, uint8_t cols)
{
    return (m != NULL) && (m->data != NULL) && (m->rows == rows) && (m->cols == cols);
}

/**
 * @brief Validate that a Kalman filter instance has an initialized state configuration.
 * @param kf Kalman filter instance.
 * @return True if kf and kf->cfg are non-NULL with valid dimensions and state matrices.
 */
static bool kalman_cfg_ready(const SYN_Kalman *kf)
{
    if ((kf == NULL) || (kf->cfg == NULL)) {
        return false;
    }
    const SYN_Kalman_Config *c = kf->cfg;
    if ((c->n_state == 0U) || (c->n_state > SYN_KALMAN_MAX_STATE) || (c->n_meas == 0U) ||
        (c->n_meas > SYN_KALMAN_MAX_MEAS)) {
        return false;
    }
    return kalman_mat_matches(c->x, c->n_state, 1U) &&
           kalman_mat_matches(c->P, c->n_state, c->n_state) &&
           kalman_mat_matches(c->F, c->n_state, c->n_state) &&
           kalman_mat_matches(c->Q, c->n_state, c->n_state);
}

/**
 * @brief Validate configuration and scratch matrices required by syn_kalman_predict.
 * @param kf Kalman filter instance.
 * @return True if predict can execute safely, false otherwise.
 */
static bool kalman_predict_valid(const SYN_Kalman *kf)
{
    if (!kalman_cfg_ready(kf)) {
        return false;
    }
    uint8_t ns = kf->cfg->n_state;
    return kalman_mat_matches(kf->temp_n1, ns, 1U) && kalman_mat_matches(kf->temp_nn_1, ns, ns) &&
           kalman_mat_matches(kf->temp_nn_2, ns, ns);
}

/**
 * @brief Validate configuration, measurement vector, and scratch buffers for syn_kalman_update.
 * @param kf Kalman filter instance.
 * @param z  Measurement vector (n_meas x 1).
 * @return True if update can execute safely, false otherwise.
 */
static bool kalman_update_valid(const SYN_Kalman *kf, const SYN_Matrix *z)
{
    if (!kalman_predict_valid(kf)) {
        return false;
    }
    const SYN_Kalman_Config *c = kf->cfg;
    uint8_t ns = c->n_state;
    uint8_t nm = c->n_meas;
    if (!kalman_mat_matches(z, nm, 1U) || !kalman_mat_matches(c->H, nm, ns) ||
        !kalman_mat_matches(c->R, nm, nm)) {
        return false;
    }
    return kalman_mat_matches(kf->temp_nm, ns, nm) && kalman_mat_matches(kf->temp_mn, nm, ns) &&
           kalman_mat_matches(kf->temp_mm, nm, nm) && kalman_mat_matches(kf->temp_mm_2, nm, nm) &&
           kalman_mat_matches(kf->temp_m1, nm, 1U);
}

void syn_kalman_predict(SYN_Kalman *kf)
{
    if (!kalman_predict_valid(kf)) {
        return;
    }

    const SYN_Kalman_Config *c = kf->cfg;

    /* x̂⁻ = F · x̂ */
    syn_matrix_mul(c->F, c->x, kf->temp_n1);
    syn_matrix_copy(c->x, kf->temp_n1);

    /* P⁻ = F · P · Fᵀ + Q */
    syn_matrix_mul(c->F, c->P, kf->temp_nn_1);          /* F·P */
    syn_matrix_transpose(c->F, kf->temp_nn_2);          /* Fᵀ */
    syn_matrix_mul(kf->temp_nn_1, kf->temp_nn_2, c->P); /* F·P·Fᵀ → P */
    syn_matrix_add(c->P, c->Q, kf->temp_nn_1);          /* P + Q */
    syn_matrix_copy(c->P, kf->temp_nn_1);
}

SYN_Status syn_kalman_update(SYN_Kalman *kf, const SYN_Matrix *z)
{
    if (!kalman_update_valid(kf, z)) {
        return SYN_INVALID_PARAM;
    }

    const SYN_Kalman_Config *c = kf->cfg;
    uint8_t ns = c->n_state;
    uint8_t nm = c->n_meas;

    /* ── Innovation: y = z − H·x̂⁻ ────────────────────────────────────── */
    syn_matrix_mul(c->H, c->x, kf->temp_m1);     /* H·x → temp_m1 */
    syn_matrix_sub(z, kf->temp_m1, kf->temp_m1); /* y = z - H·x   */

    /* ── Innovation covariance: S = H·P⁻·Hᵀ + R ──────────────────────── */
    syn_matrix_mul(c->H, c->P, kf->temp_mn);               /* H·P  (nm×ns)   */
    syn_matrix_transpose(c->H, kf->temp_nm);               /* Hᵀ   (ns×nm)   */
    syn_matrix_mul(kf->temp_mn, kf->temp_nm, kf->temp_mm); /* H·P·Hᵀ (nm×nm) */
    syn_matrix_add(kf->temp_mm, c->R, kf->temp_mm_2);      /* S = HPHᵀ + R    */

    /* ── S⁻¹ ──────────────────────────────────────────────────────────── */
    SYN_Status inv_st = syn_matrix_inv(kf->temp_mm_2, kf->temp_mm);
    if (inv_st != SYN_OK)
        return SYN_ERROR;
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
    SYN_Matrix K = {k_buf, ns, nm};

    syn_matrix_mul(kf->temp_nm, kf->temp_mm, &K); /* Hᵀ·S⁻¹ */
    syn_matrix_mul(c->P, &K, kf->temp_nm);        /* P·Hᵀ·S⁻¹ */
    syn_matrix_copy(&K, kf->temp_nm);             /* K final  */

    /* ── State update: x̂ = x̂⁻ + K·y ──────────────────────────────────── */
    syn_matrix_mul(&K, kf->temp_m1, kf->temp_n1); /* K·y */
    syn_matrix_add(c->x, kf->temp_n1, c->x);      /* x += K·y */

    /* ── Covariance update: P = (I − K·H)·P⁻ ─────────────────────────── */
    syn_matrix_copy(kf->temp_nn_2, c->P);    /* Save P⁻ into temp_nn_2 */
    syn_matrix_mul(&K, c->H, kf->temp_nn_1); /* K·H (ns×ns) */

    /* Construct (I - K·H) in temp_nn_1 */
    for (uint8_t i = 0; i < ns; i++) {
        for (uint8_t j = 0; j < ns; j++) {
            q16_t eye = (i == j) ? Q16_ONE : 0;
            SYN_MAT_AT(kf->temp_nn_1, i, j) = eye - SYN_MAT_AT(kf->temp_nn_1, i, j);
        }
    }

    syn_matrix_mul(kf->temp_nn_1, kf->temp_nn_2, c->P); /* (I-K·H)·P⁻ → P */

    return SYN_OK;
}

#endif /* SYN_USE_KALMAN */
