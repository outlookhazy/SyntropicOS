/**
 * @file syn_matrix.h
 * @brief Fixed-point Q16.16 matrix operations — zero heap allocation.
 *
 * Supports compile-time-arbitrary matrix dimensions. Callers supply all
 * storage via stack or static arrays. The compiler constant-folds
 * dimension parameters and unrolls loops when sizes are compile-time
 * constants.
 *
 * @par Usage
 * @code
 *   // Declare three 3×3 matrices on the stack
 *   SYN_MAT_DECL(A, 3, 3);
 *   SYN_MAT_DECL(B, 3, 3);
 *   SYN_MAT_DECL(C, 3, 3);
 *
 *   syn_matrix_identity(&A);
 *   syn_matrix_rotate_2d(&B, Q16_PI_2);  // 90° rotation
 *   syn_matrix_mul(&A, &B, &C);          // C = A × B
 *
 *   // Non-square multiply: 2×3 × 3×4 = 2×4
 *   SYN_MAT_DECL(M, 2, 3);
 *   SYN_MAT_DECL(N, 3, 4);
 *   SYN_MAT_DECL(P, 2, 4);
 *   syn_matrix_mul(&M, &N, &P);
 *
 *   // Element access
 *   SYN_MAT_AT(&A, 1, 2) = Q16_FROM_INT(42);
 * @endcode
 * @ingroup syn_util
 */

#ifndef SYN_MATRIX_H
#define SYN_MATRIX_H

#include "syn_qmath.h"
#include "../common/syn_defs.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Matrix view (non-owning) ───────────────────────────────────────────── */

/**
 * @brief Fixed-point matrix descriptor (non-owning view).
 *
 * Points to caller-owned flat row-major Q16.16 storage.
 * Dimensions are typically compile-time constants, enabling the
 * compiler to constant-fold loop bounds and unroll.
 */
typedef struct {
    q16_t   *data;    /**< Flat row-major storage (caller-owned)    */
    uint8_t  rows;    /**< Number of rows                            */
    uint8_t  cols;    /**< Number of columns                         */
} SYN_Matrix;

/* ── Declaration macros ─────────────────────────────────────────────────── */

/** @brief Declare a matrix with automatic (stack) storage. */
#define SYN_MAT_DECL(name, R, C)                                \
    q16_t name##_store[(R) * (C)];                              \
    SYN_Matrix name = { name##_store, (R), (C) }

/** @brief Declare a matrix with static storage (zero-initialized). */
#define SYN_MAT_STATIC(name, R, C)                              \
    static q16_t name##_store[(R) * (C)];                       \
    static SYN_Matrix name = { name##_store, (R), (C) }

/** @brief Initialize a matrix view over existing storage. */
#define SYN_MAT_INIT(name, storage, R, C)                       \
    SYN_Matrix name = { (storage), (R), (C) }

/** @brief Element access (row, col) — evaluates to an lvalue. */
#define SYN_MAT_AT(m, r, c)  ((m)->data[(r) * (m)->cols + (c)])

/** @brief Shorthand for 2×2. */
#define SYN_MAT2_DECL(name)  SYN_MAT_DECL(name, 2, 2)
/** @brief Shorthand for 3×3. */
#define SYN_MAT3_DECL(name)  SYN_MAT_DECL(name, 3, 3)
/** @brief Shorthand for 4×4. */
#define SYN_MAT4_DECL(name)  SYN_MAT_DECL(name, 4, 4)

/* ── Core operations ────────────────────────────────────────────────────── */

/**
 * @brief Set matrix to identity.
 * @param m  Square matrix.
 */
void syn_matrix_identity(SYN_Matrix *m);

/**
 * @brief Set all elements to zero.
 * @param m  Matrix to clear.
 */
void syn_matrix_zero(SYN_Matrix *m);

/**
 * @brief Copy matrix contents.
 * @param dst  Destination (must match src dimensions).
 * @param src  Source matrix.
 */
void syn_matrix_copy(SYN_Matrix *dst, const SYN_Matrix *src);

/**
 * @brief Element-wise addition: out = A + B.
 * @param a    First operand.
 * @param b    Second operand (same dimensions as a).
 * @param out  Result (same dimensions as a).
 */
void syn_matrix_add(const SYN_Matrix *a, const SYN_Matrix *b, SYN_Matrix *out);

/**
 * @brief Element-wise subtraction: out = A − B.
 * @param a    Minuend.
 * @param b    Subtrahend (same dimensions as a).
 * @param out  Result (same dimensions as a).
 */
void syn_matrix_sub(const SYN_Matrix *a, const SYN_Matrix *b, SYN_Matrix *out);

/**
 * @brief Scalar multiply: out = A × scalar.
 * @param a       Input matrix.
 * @param scalar  Q16 scalar multiplier.
 * @param out     Result (same dimensions as a).
 */
void syn_matrix_scale(const SYN_Matrix *a, q16_t scalar, SYN_Matrix *out);

/* ── Multiply ───────────────────────────────────────────────────────────── */

/**
 * @brief Matrix multiply: out = A × B.
 *
 * Requires: A.cols == B.rows, out is A.rows × B.cols.
 * Uses int64_t accumulator for full Q16 precision.
 *
 * @param a    Left operand (R₁ × C₁).
 * @param b    Right operand (C₁ × C₂).
 * @param out  Result (R₁ × C₂). Must not alias a or b.
 */
void syn_matrix_mul(const SYN_Matrix *a, const SYN_Matrix *b, SYN_Matrix *out);

/**
 * @brief Matrix–vector multiply: out = M × v.
 *
 * @param m       Square or rectangular matrix (R × C).
 * @param v_in    Input vector (C elements).
 * @param v_out   Output vector (R elements). Must not alias v_in.
 * @param n_in    Length of input vector (must equal m->cols).
 */
void syn_matrix_mul_vec(const SYN_Matrix *m, const q16_t *v_in,
                        q16_t *v_out, uint8_t n_in);

/* ── Transpose, trace ───────────────────────────────────────────────────── */

/**
 * @brief Transpose: out = Aᵀ.
 * @param a    Input (R × C).
 * @param out  Output (C × R). Must not alias a.
 */
void syn_matrix_transpose(const SYN_Matrix *a, SYN_Matrix *out);

/**
 * @brief Trace (sum of diagonal elements).
 * @param m  Square matrix.
 * @return Trace in Q16.
 */
q16_t syn_matrix_trace(const SYN_Matrix *m);

/* ── Determinant & inverse ──────────────────────────────────────────────── */

/**
 * @brief Determinant of a square matrix.
 *
 * Supports 2×2, 3×3, and 4×4. Uses cofactor expansion for 2×2/3×3
 * and LU decomposition for 4×4.
 *
 * @param m  Square matrix.
 * @return Determinant in Q16.
 */
q16_t syn_matrix_det(const SYN_Matrix *m);

/**
 * @brief Matrix inverse: out = M⁻¹.
 *
 * Supports 2×2, 3×3, and 4×4 square matrices.
 * Returns SYN_ERROR if the matrix is singular (determinant ≈ 0).
 *
 * @param m    Input square matrix.
 * @param out  Output inverse (same dimensions). Must not alias m.
 * @return SYN_OK on success, SYN_ERROR if singular.
 */
SYN_Status syn_matrix_inv(const SYN_Matrix *m, SYN_Matrix *out);

/* ── 2D transforms (3×3 homogeneous) ────────────────────────────────────── */

/**
 * @brief Set 3×3 matrix to a 2D rotation.
 * @param out    Output 3×3 matrix.
 * @param angle  Rotation angle in Q16 radians (counter-clockwise).
 */
void syn_matrix_rotate_2d(SYN_Matrix *out, q16_t angle);

/**
 * @brief Set 3×3 matrix to a 2D translation.
 * @param out  Output 3×3 matrix.
 * @param tx   X translation in Q16.
 * @param ty   Y translation in Q16.
 */
void syn_matrix_translate_2d(SYN_Matrix *out, q16_t tx, q16_t ty);

/**
 * @brief Set 3×3 matrix to a 2D scale.
 * @param out  Output 3×3 matrix.
 * @param sx   X scale factor in Q16.
 * @param sy   Y scale factor in Q16.
 */
void syn_matrix_scale_2d(SYN_Matrix *out, q16_t sx, q16_t sy);

/* ── 3D transforms (4×4 homogeneous) ────────────────────────────────────── */

/**
 * @brief Set 4×4 matrix to a rotation around the X axis.
 * @param out    Output 4×4 matrix.
 * @param angle  Rotation angle in Q16 radians.
 */
void syn_matrix_rotate_x(SYN_Matrix *out, q16_t angle);

/**
 * @brief Set 4×4 matrix to a rotation around the Y axis.
 * @param out    Output 4×4 matrix.
 * @param angle  Rotation angle in Q16 radians.
 */
void syn_matrix_rotate_y(SYN_Matrix *out, q16_t angle);

/**
 * @brief Set 4×4 matrix to a rotation around the Z axis.
 * @param out    Output 4×4 matrix.
 * @param angle  Rotation angle in Q16 radians.
 */
void syn_matrix_rotate_z(SYN_Matrix *out, q16_t angle);

/**
 * @brief Set 4×4 matrix to a 3D translation.
 * @param out  Output 4×4 matrix.
 * @param tx   X translation in Q16.
 * @param ty   Y translation in Q16.
 * @param tz   Z translation in Q16.
 */
void syn_matrix_translate_3d(SYN_Matrix *out, q16_t tx, q16_t ty, q16_t tz);

/* ── Vector helpers ─────────────────────────────────────────────────────── */

/**
 * @brief Dot product of two n-element Q16 vectors.
 * @param a  First vector.
 * @param b  Second vector.
 * @param n  Number of elements.
 * @return Dot product in Q16.
 */
q16_t syn_vec_dot(const q16_t *a, const q16_t *b, uint8_t n);

/**
 * @brief Cross product of two 3-element Q16 vectors.
 * @param a    First vector (3 elements).
 * @param b    Second vector (3 elements).
 * @param out  Output vector (3 elements). Must not alias a or b.
 */
void syn_vec3_cross(const q16_t *a, const q16_t *b, q16_t *out);

/**
 * @brief Magnitude (Euclidean norm) of an n-element Q16 vector.
 * @param v  Vector.
 * @param n  Number of elements.
 * @return |v| in Q16.
 */
q16_t syn_vec_norm(const q16_t *v, uint8_t n);

/**
 * @brief Normalize a vector to unit length.
 * @param v    Input vector (n elements).
 * @param out  Output unit vector (n elements). May alias v.
 * @param n    Number of elements.
 * @return SYN_OK on success, SYN_ERROR if zero-length vector.
 */
SYN_Status syn_vec_normalize(const q16_t *v, q16_t *out, uint8_t n);

/* ── Linear Solvers ─────────────────────────────────────────────────────── */

/**
 * @brief Solve general linear system A · x = b via LU decomposition.
 *
 * Uses Doolittle LU decomposition with partial pivoting (PA = LU).
 * A must be a square N×N non-singular matrix. b and x must be N×1 vectors.
 *
 * @param A  Square system matrix (N×N).
 * @param b  Right-hand side vector (N×1).
 * @param x  Output solution vector (N×1). May alias b.
 * @return SYN_OK on success, SYN_ERROR if A is singular or dimensions invalid.
 */
SYN_Status syn_matrix_solve_lu(const SYN_Matrix *A, const SYN_Matrix *b, SYN_Matrix *x);

/**
 * @brief Solve symmetric positive-definite system A · x = b via Cholesky.
 *
 * Decomposes A into L · Lᵀ where L is lower-triangular, then solves
 * L · y = b and Lᵀ · x = y. Faster and more numerically stable than LU
 * for symmetric positive-definite matrices (e.g. covariance or normal eqns).
 *
 * @param A  Symmetric positive-definite system matrix (N×N).
 * @param b  Right-hand side vector (N×1).
 * @param x  Output solution vector (N×1). May alias b.
 * @return SYN_OK on success, SYN_ERROR if A is not positive-definite.
 */
SYN_Status syn_matrix_solve_cholesky(const SYN_Matrix *A, const SYN_Matrix *b, SYN_Matrix *x);

/**
 * @brief Solve overdetermined system A · x ≈ b via Normal Equations (Least Squares).
 *
 * Minimizes ||A·x − b||₂ for an M×N system (M ≥ N).
 * Solves (Aᵀ · A) · x = Aᵀ · b using Cholesky or LU decomposition.
 *
 * @param A  Overdetermined system matrix (M×N, M ≥ N).
 * @param b  Right-hand side measurement vector (M×1).
 * @param x  Output solution parameter vector (N×1).
 * @return SYN_OK on success, SYN_ERROR if AᵀA is singular.
 */
SYN_Status syn_matrix_least_squares(const SYN_Matrix *A, const SYN_Matrix *b, SYN_Matrix *x);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MATRIX_H */
