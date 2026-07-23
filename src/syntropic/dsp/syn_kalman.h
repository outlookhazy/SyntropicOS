/**
 * @file syn_kalman.h
 * @brief General-purpose fixed-point Kalman filter.
 *
 * Implements a discrete-time linear Kalman filter using Q16.16
 * matrix operations. Supports arbitrary state and measurement
 * dimensions (compile-time determined, no heap allocation).
 *
 * @par Algorithm
 * Predict:
 * - x̂⁻ = F · x̂
 * - P⁻  = F · P · Fᵀ + Q
 *
 * Update:
 * - y  = z − H · x̂⁻         (innovation)
 * - S  = H · P⁻ · Hᵀ + R    (innovation covariance)
 * - K  = P⁻ · Hᵀ · S⁻¹      (Kalman gain)
 * - x̂  = x̂⁻ + K · y          (state update)
 * - P  = (I − K · H) · P⁻    (covariance update)
 *
 * @par Usage — 2D position/velocity fusion
 * @code
 *   // State: [position, velocity] → 2×1
 *   // Measurement: [position]    → 1×1
 *   #define N_STATE 2
 *   #define N_MEAS  1
 *
 *   SYN_Kalman kf;
 *   SYN_Kalman_Config cfg;
 *
 *   // Allocate all matrices on the stack
 *   SYN_MAT_DECL(x,  N_STATE, 1);       // State vector
 *   SYN_MAT_DECL(P,  N_STATE, N_STATE); // Error covariance
 *   SYN_MAT_DECL(F,  N_STATE, N_STATE); // State transition
 *   SYN_MAT_DECL(Q,  N_STATE, N_STATE); // Process noise
 *   SYN_MAT_DECL(H,  N_MEAS,  N_STATE); // Measurement model
 *   SYN_MAT_DECL(R,  N_MEAS,  N_MEAS);  // Measurement noise
 *   SYN_MAT_DECL(z,  N_MEAS,  1);       // Measurement vector
 *
 *   cfg.x = &x;  cfg.P = &P;  cfg.F = &F;
 *   cfg.Q = &Q;  cfg.H = &H;  cfg.R = &R;
 *   cfg.n_state = N_STATE;
 *   cfg.n_meas  = N_MEAS;
 *
 *   // Set up F = [[1, dt], [0, 1]] for constant-velocity model
 *   syn_matrix_identity(&F);
 *   SYN_MAT_AT(&F, 0, 1) = Q16_FROM_FRAC(1, 100); // dt = 10ms
 *
 *   // H = [[1, 0]] — we only measure position
 *   syn_matrix_zero(&H);
 *   SYN_MAT_AT(&H, 0, 0) = Q16_ONE;
 *
 *   syn_kalman_init(&kf, &cfg);
 *
 *   // In control loop:
 *   z.data[0] = sensor_reading;
 *   syn_kalman_predict(&kf);
 *   syn_kalman_update(&kf, &z);
 *   q16_t filtered_pos = kf.cfg->x->data[0];
 * @endcode
 * @ingroup syn_dsp
 */

#ifndef SYN_KALMAN_H
#define SYN_KALMAN_H

#include "../util/syn_matrix.h"
#include "../common/syn_defs.h"

/**
 * @brief Maximum state dimension (default 6, override in syn_config.h).
 *
 * Controls the stack size of internal Kalman gain buffer during update.
 */
#ifndef SYN_KALMAN_MAX_STATE
#define SYN_KALMAN_MAX_STATE  6
#endif

/**
 * @brief Maximum measurement dimension (default 4, override in syn_config.h).
 */
#ifndef SYN_KALMAN_MAX_MEAS
#define SYN_KALMAN_MAX_MEAS   4
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Kalman filter configuration — all matrices are caller-owned.
 *
 * The caller allocates all matrices via SYN_MAT_DECL and passes
 * pointers in this config struct. The Kalman filter modifies x and P
 * in-place during predict/update.
 */
typedef struct {
    SYN_Matrix *x;    /**< State vector (n_state × 1)                  */
    SYN_Matrix *P;    /**< Error covariance (n_state × n_state)        */
    SYN_Matrix *F;    /**< State transition model (n_state × n_state)  */
    SYN_Matrix *Q;    /**< Process noise covariance (n_state × n_state)*/
    SYN_Matrix *H;    /**< Measurement model (n_meas × n_state)        */
    SYN_Matrix *R;    /**< Measurement noise covariance (n_meas × n_meas) */
    uint8_t n_state;  /**< State dimension                              */
    uint8_t n_meas;   /**< Measurement dimension                        */
} SYN_Kalman_Config;

/**
 * @brief Kalman filter instance.
 *
 * Holds internal scratch matrices for intermediate calculations.
 * The caller must allocate the scratch space and pass it to
 * syn_kalman_init via the work buffer.
 */
typedef struct {
    const SYN_Kalman_Config *cfg;   /**< Configuration (caller-owned)    */

    /* Scratch matrices — caller allocates, filter uses internally */
    SYN_Matrix *temp_nn_1;  /**< Scratch n_state × n_state               */
    SYN_Matrix *temp_nn_2;  /**< Scratch n_state × n_state               */
    SYN_Matrix *temp_nm;    /**< Scratch n_state × n_meas                */
    SYN_Matrix *temp_mn;    /**< Scratch n_meas  × n_state               */
    SYN_Matrix *temp_mm;    /**< Scratch n_meas  × n_meas                */
    SYN_Matrix *temp_mm_2;  /**< Scratch n_meas  × n_meas (for inverse)  */
    SYN_Matrix *temp_n1;    /**< Scratch n_state × 1                     */
    SYN_Matrix *temp_m1;    /**< Scratch n_meas  × 1 (innovation)        */
} SYN_Kalman;

/**
 * @brief Convenience macro to declare all Kalman scratch matrices.
 *
 * Declares the 8 scratch matrices needed by SYN_Kalman on the stack.
 *
 * @param prefix  Name prefix for the scratch variables.
 * @param NS      State dimension.
 * @param NM      Measurement dimension.
 */
#define SYN_KALMAN_SCRATCH_DECL(prefix, NS, NM)                 \
    SYN_MAT_DECL(prefix##_nn1, NS, NS);                        \
    SYN_MAT_DECL(prefix##_nn2, NS, NS);                        \
    SYN_MAT_DECL(prefix##_nm,  NS, NM);                        \
    SYN_MAT_DECL(prefix##_mn,  NM, NS);                        \
    SYN_MAT_DECL(prefix##_mm,  NM, NM);                        \
    SYN_MAT_DECL(prefix##_mm2, NM, NM);                        \
    SYN_MAT_DECL(prefix##_n1,  NS, 1);                         \
    SYN_MAT_DECL(prefix##_m1,  NM, 1)

/**
 * @brief Assign scratch matrices to the Kalman filter instance.
 *
 * @param kf      Kalman filter instance.
 * @param prefix  Name prefix used in SYN_KALMAN_SCRATCH_DECL.
 */
#define SYN_KALMAN_SCRATCH_ASSIGN(kf, prefix)                   \
    do {                                                         \
        (kf)->temp_nn_1 = &prefix##_nn1;                        \
        (kf)->temp_nn_2 = &prefix##_nn2;                        \
        (kf)->temp_nm   = &prefix##_nm;                         \
        (kf)->temp_mn   = &prefix##_mn;                         \
        (kf)->temp_mm   = &prefix##_mm;                         \
        (kf)->temp_mm_2 = &prefix##_mm2;                        \
        (kf)->temp_n1   = &prefix##_n1;                         \
        (kf)->temp_m1   = &prefix##_m1;                         \
    } while (0)

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the Kalman filter.
 *
 * The caller must have populated cfg->F, cfg->H, cfg->Q, cfg->R,
 * and initial cfg->x and cfg->P before calling this.
 * Scratch matrices must be assigned via SYN_KALMAN_SCRATCH_ASSIGN.
 *
 * @param kf   Kalman filter instance.
 * @param cfg  Configuration (caller-owned, must outlive kf).
 * @return SYN_OK on success, SYN_INVALID_PARAM on dimension mismatch.
 */
SYN_Status syn_kalman_init(SYN_Kalman *kf, const SYN_Kalman_Config *cfg);

/**
 * @brief Predict step: propagate state and covariance forward.
 *
 * After this call:
 * - x̂⁻ = F · x̂  (state predicted forward)
 * - P⁻  = F · P · Fᵀ + Q  (covariance grows)
 *
 * @param kf  Kalman filter instance.
 */
void syn_kalman_predict(SYN_Kalman *kf);

/**
 * @brief Update step: incorporate a measurement.
 *
 * After this call:
 * - x̂ is corrected toward the measurement
 * - P is reduced (uncertainty decreased)
 *
 * @param kf  Kalman filter instance.
 * @param z   Measurement vector (n_meas × 1).
 * @return SYN_OK on success, SYN_ERROR if innovation covariance is singular.
 */
SYN_Status syn_kalman_update(SYN_Kalman *kf, const SYN_Matrix *z);

#ifdef __cplusplus
}
#endif

#endif /* SYN_KALMAN_H */
