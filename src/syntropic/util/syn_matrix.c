#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_MATRIX) || SYN_USE_MATRIX

/**
 * @file syn_matrix.c
 * @brief Fixed-point Q16.16 matrix operations implementation.
 *
 * All operations use caller-owned storage. No heap allocation.
 * int64_t accumulators preserve full Q16 precision in multiply chains.
 */

#include "syn_matrix.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ════════════════════════════════════════════════════════════════════════ */
/*  Core operations                                                        */
/* ════════════════════════════════════════════════════════════════════════ */

void syn_matrix_identity(SYN_Matrix *m)
{
    SYN_ASSERT(m != NULL);
    SYN_ASSERT(m->rows == m->cols);  /* Must be square */

    memset(m->data, 0, (size_t)m->rows * m->cols * sizeof(q16_t));
    uint8_t i;
    for (i = 0; i < m->rows; i++) {
        SYN_MAT_AT(m, i, i) = Q16_ONE;
    }
}

void syn_matrix_zero(SYN_Matrix *m)
{
    SYN_ASSERT(m != NULL);
    memset(m->data, 0, (size_t)m->rows * m->cols * sizeof(q16_t));
}

void syn_matrix_copy(SYN_Matrix *dst, const SYN_Matrix *src)
{
    SYN_ASSERT(dst != NULL && src != NULL);
    SYN_ASSERT(dst->rows == src->rows && dst->cols == src->cols);
    memcpy(dst->data, src->data, (size_t)src->rows * src->cols * sizeof(q16_t));
}

void syn_matrix_add(const SYN_Matrix *a, const SYN_Matrix *b, SYN_Matrix *out)
{
    SYN_ASSERT(a != NULL && b != NULL && out != NULL);
    SYN_ASSERT(a->rows == b->rows && a->cols == b->cols);
    SYN_ASSERT(out->rows == a->rows && out->cols == a->cols);

    uint16_t n = (uint16_t)a->rows * a->cols;
    uint16_t i;
    for (i = 0; i < n; i++) {
        out->data[i] = a->data[i] + b->data[i];
    }
}

void syn_matrix_sub(const SYN_Matrix *a, const SYN_Matrix *b, SYN_Matrix *out)
{
    SYN_ASSERT(a != NULL && b != NULL && out != NULL);
    SYN_ASSERT(a->rows == b->rows && a->cols == b->cols);
    SYN_ASSERT(out->rows == a->rows && out->cols == a->cols);

    uint16_t n = (uint16_t)a->rows * a->cols;
    uint16_t i;
    for (i = 0; i < n; i++) {
        out->data[i] = a->data[i] - b->data[i];
    }
}

void syn_matrix_scale(const SYN_Matrix *a, q16_t scalar, SYN_Matrix *out)
{
    SYN_ASSERT(a != NULL && out != NULL);
    SYN_ASSERT(out->rows == a->rows && out->cols == a->cols);

    uint16_t n = (uint16_t)a->rows * a->cols;
    uint16_t i;
    for (i = 0; i < n; i++) {
        out->data[i] = q16_mul(a->data[i], scalar);
    }
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  Multiply                                                               */
/* ════════════════════════════════════════════════════════════════════════ */

void syn_matrix_mul(const SYN_Matrix *a, const SYN_Matrix *b, SYN_Matrix *out)
{
    SYN_ASSERT(a != NULL && b != NULL && out != NULL);
    SYN_ASSERT(a->cols == b->rows);
    SYN_ASSERT(out->rows == a->rows && out->cols == b->cols);

    uint8_t i, j, k;
    for (i = 0; i < a->rows; i++) {
        for (j = 0; j < b->cols; j++) {
            int64_t acc = 0;
            for (k = 0; k < a->cols; k++) {
                acc += (int64_t)SYN_MAT_AT(a, i, k) * SYN_MAT_AT(b, k, j);
            }
            SYN_MAT_AT(out, i, j) = (q16_t)(acc >> Q16_SHIFT);
        }
    }
}

void syn_matrix_mul_vec(const SYN_Matrix *m, const q16_t *v_in,
                        q16_t *v_out, uint8_t n_in)
{
    SYN_ASSERT(m != NULL && v_in != NULL && v_out != NULL);
    SYN_ASSERT(m->cols == n_in);

    uint8_t i, k;
    for (i = 0; i < m->rows; i++) {
        int64_t acc = 0;
        for (k = 0; k < n_in; k++) {
            acc += (int64_t)SYN_MAT_AT(m, i, k) * v_in[k];
        }
        v_out[i] = (q16_t)(acc >> Q16_SHIFT);
    }
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  Transpose, trace                                                       */
/* ════════════════════════════════════════════════════════════════════════ */

void syn_matrix_transpose(const SYN_Matrix *a, SYN_Matrix *out)
{
    SYN_ASSERT(a != NULL && out != NULL);
    SYN_ASSERT(out->rows == a->cols && out->cols == a->rows);

    uint8_t i, j;
    for (i = 0; i < a->rows; i++) {
        for (j = 0; j < a->cols; j++) {
            SYN_MAT_AT(out, j, i) = SYN_MAT_AT(a, i, j);
        }
    }
}

q16_t syn_matrix_trace(const SYN_Matrix *m)
{
    SYN_ASSERT(m != NULL);
    SYN_ASSERT(m->rows == m->cols);

    q16_t sum = 0;
    uint8_t i;
    for (i = 0; i < m->rows; i++) {
        sum += SYN_MAT_AT(m, i, i);
    }
    return sum;
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  Determinant                                                            */
/* ════════════════════════════════════════════════════════════════════════ */

/**
 * @brief 2×2 determinant: ad - bc.
 * @param d  Flat 4-element array [a, b, c, d] (row-major).
 * @return Determinant in Q16.
 */
static q16_t det_2x2(const q16_t *d)
{
    return (q16_t)(((int64_t)d[0] * d[3] - (int64_t)d[1] * d[2]) >> Q16_SHIFT);
}

/**
 * @brief 3×3 determinant via cofactor expansion along row 0.
 * @param d  Flat 9-element array (row-major).
 * @return Determinant in Q16.
 */
static q16_t det_3x3(const q16_t *d)
{
    /* a(ei - fh) - b(di - fg) + c(dh - eg) */
    int64_t a = d[0], b = d[1], c = d[2];
    int64_t det_a = ((int64_t)d[4] * d[8] - (int64_t)d[5] * d[7]) >> Q16_SHIFT;
    int64_t det_b = ((int64_t)d[3] * d[8] - (int64_t)d[5] * d[6]) >> Q16_SHIFT;
    int64_t det_c = ((int64_t)d[3] * d[7] - (int64_t)d[4] * d[6]) >> Q16_SHIFT;

    int64_t result = (a * det_a - b * det_b + c * det_c) >> Q16_SHIFT;
    return (q16_t)result;
}

/**
 * @brief 4×4 determinant via cofactor expansion along row 0.
 * @param d  Flat 16-element array (row-major).
 * @return Determinant in Q16.
 */
static q16_t det_4x4(const q16_t *d)
{
    /* Cofactor expansion along first row */
    q16_t minor0[9], minor1[9], minor2[9], minor3[9];

    /* Minor of d[0]: rows 1-3, cols 1-3 */
    minor0[0] = d[5];  minor0[1] = d[6];  minor0[2] = d[7];
    minor0[3] = d[9];  minor0[4] = d[10]; minor0[5] = d[11];
    minor0[6] = d[13]; minor0[7] = d[14]; minor0[8] = d[15];

    /* Minor of d[1]: rows 1-3, cols 0,2,3 */
    minor1[0] = d[4];  minor1[1] = d[6];  minor1[2] = d[7];
    minor1[3] = d[8];  minor1[4] = d[10]; minor1[5] = d[11];
    minor1[6] = d[12]; minor1[7] = d[14]; minor1[8] = d[15];

    /* Minor of d[2]: rows 1-3, cols 0,1,3 */
    minor2[0] = d[4];  minor2[1] = d[5];  minor2[2] = d[7];
    minor2[3] = d[8];  minor2[4] = d[9];  minor2[5] = d[11];
    minor2[6] = d[12]; minor2[7] = d[13]; minor2[8] = d[15];

    /* Minor of d[3]: rows 1-3, cols 0,1,2 */
    minor3[0] = d[4];  minor3[1] = d[5];  minor3[2] = d[6];
    minor3[3] = d[8];  minor3[4] = d[9];  minor3[5] = d[10];
    minor3[6] = d[12]; minor3[7] = d[13]; minor3[8] = d[14];

    int64_t result = ((int64_t)d[0] * det_3x3(minor0)) >> Q16_SHIFT;
    result -= ((int64_t)d[1] * det_3x3(minor1)) >> Q16_SHIFT;
    result += ((int64_t)d[2] * det_3x3(minor2)) >> Q16_SHIFT;
    result -= ((int64_t)d[3] * det_3x3(minor3)) >> Q16_SHIFT;

    return (q16_t)result;
}

q16_t syn_matrix_det(const SYN_Matrix *m)
{
    SYN_ASSERT(m != NULL);
    SYN_ASSERT(m->rows == m->cols);

    switch (m->rows) {
    case 1:  return m->data[0];
    case 2:  return det_2x2(m->data);
    case 3:  return det_3x3(m->data);
    case 4:  return det_4x4(m->data);
    default: return 0;  /* Unsupported */
    }
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  Inverse                                                                */
/* ════════════════════════════════════════════════════════════════════════ */

/**
 * @brief 2×2 inverse via adjugate/determinant.
 * @param m    Input 2×2 matrix.
 * @param out  Output 2×2 inverse.
 * @return SYN_OK or SYN_ERROR if singular.
 */
static SYN_Status inv_2x2(const SYN_Matrix *m, SYN_Matrix *out)
{
    q16_t det = det_2x2(m->data);
    if (det == 0) return SYN_ERROR;

    /* [a b]^-1 = (1/det) * [ d -b]
     * [c d]                [-c  a] */
    SYN_MAT_AT(out, 0, 0) =  q16_div(m->data[3], det);
    SYN_MAT_AT(out, 0, 1) = -q16_div(m->data[1], det);
    SYN_MAT_AT(out, 1, 0) = -q16_div(m->data[2], det);
    SYN_MAT_AT(out, 1, 1) =  q16_div(m->data[0], det);

    return SYN_OK;
}

/**
 * @brief 3×3 inverse via adjugate/determinant.
 * @param m    Input 3×3 matrix.
 * @param out  Output 3×3 inverse.
 * @return SYN_OK or SYN_ERROR if singular.
 */
static SYN_Status inv_3x3(const SYN_Matrix *m, SYN_Matrix *out)
{
    q16_t det = det_3x3(m->data);
    if (det == 0) return SYN_ERROR;

    const q16_t *d = m->data;

    /* Cofactor matrix (transposed = adjugate) divided by det */
    /* Row 0 of adjugate (cofactors of column 0) */
    q16_t c00 = (q16_t)(((int64_t)d[4] * d[8] - (int64_t)d[5] * d[7]) >> Q16_SHIFT);
    q16_t c01 = (q16_t)(((int64_t)d[2] * d[7] - (int64_t)d[1] * d[8]) >> Q16_SHIFT);
    q16_t c02 = (q16_t)(((int64_t)d[1] * d[5] - (int64_t)d[2] * d[4]) >> Q16_SHIFT);

    q16_t c10 = (q16_t)(((int64_t)d[5] * d[6] - (int64_t)d[3] * d[8]) >> Q16_SHIFT);
    q16_t c11 = (q16_t)(((int64_t)d[0] * d[8] - (int64_t)d[2] * d[6]) >> Q16_SHIFT);
    q16_t c12 = (q16_t)(((int64_t)d[2] * d[3] - (int64_t)d[0] * d[5]) >> Q16_SHIFT);

    q16_t c20 = (q16_t)(((int64_t)d[3] * d[7] - (int64_t)d[4] * d[6]) >> Q16_SHIFT);
    q16_t c21 = (q16_t)(((int64_t)d[1] * d[6] - (int64_t)d[0] * d[7]) >> Q16_SHIFT);
    q16_t c22 = (q16_t)(((int64_t)d[0] * d[4] - (int64_t)d[1] * d[3]) >> Q16_SHIFT);

    SYN_MAT_AT(out, 0, 0) = q16_div(c00, det);
    SYN_MAT_AT(out, 0, 1) = q16_div(c01, det);
    SYN_MAT_AT(out, 0, 2) = q16_div(c02, det);
    SYN_MAT_AT(out, 1, 0) = q16_div(c10, det);
    SYN_MAT_AT(out, 1, 1) = q16_div(c11, det);
    SYN_MAT_AT(out, 1, 2) = q16_div(c12, det);
    SYN_MAT_AT(out, 2, 0) = q16_div(c20, det);
    SYN_MAT_AT(out, 2, 1) = q16_div(c21, det);
    SYN_MAT_AT(out, 2, 2) = q16_div(c22, det);

    return SYN_OK;
}

/**
 * @brief 4×4 inverse via Gauss-Jordan elimination with partial pivoting.
 * @param m    Input 4×4 matrix.
 * @param out  Output 4×4 inverse.
 * @return SYN_OK or SYN_ERROR if singular.
 */
static SYN_Status inv_4x4(const SYN_Matrix *m, SYN_Matrix *out)
{
    /* Work on a copy augmented with identity: [M | I] */
    q16_t aug[4][8];
    uint8_t i, j, k;

    /* Initialize augmented matrix */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            aug[i][j] = SYN_MAT_AT(m, i, j);
            aug[i][j + 4] = (i == j) ? Q16_ONE : 0;
        }
    }

    /* Gauss-Jordan elimination with partial pivoting */
    for (k = 0; k < 4; k++) {
        /* Find pivot */
        uint8_t max_row = k;
        q16_t max_val = q16_abs(aug[k][k]);
        for (i = k + 1; i < 4; i++) {
            q16_t val = q16_abs(aug[i][k]);
            if (val > max_val) {
                max_val = val;
                max_row = i;
            }
        }

        if (max_val == 0) return SYN_ERROR;  /* Singular */

        /* Swap rows */
        if (max_row != k) {
            for (j = 0; j < 8; j++) {
                q16_t tmp = aug[k][j];
                aug[k][j] = aug[max_row][j];
                aug[max_row][j] = tmp;
            }
        }

        /* Scale pivot row so aug[k][k] = 1.0 */
        q16_t pivot = aug[k][k];
        for (j = 0; j < 8; j++) {
            aug[k][j] = q16_div(aug[k][j], pivot);
        }

        /* Eliminate column k from all other rows */
        for (i = 0; i < 4; i++) {
            if (i == k) continue;
            q16_t factor = aug[i][k];
            for (j = 0; j < 8; j++) {
                aug[i][j] -= q16_mul(factor, aug[k][j]);
            }
        }
    }

    /* Extract inverse from right half of augmented matrix */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            SYN_MAT_AT(out, i, j) = aug[i][j + 4];
        }
    }

    return SYN_OK;
}

SYN_Status syn_matrix_inv(const SYN_Matrix *m, SYN_Matrix *out)
{
    SYN_ASSERT(m != NULL && out != NULL);
    SYN_ASSERT(m->rows == m->cols);
    SYN_ASSERT(out->rows == m->rows && out->cols == m->cols);

    switch (m->rows) {
    case 2:  return inv_2x2(m, out);
    case 3:  return inv_3x3(m, out);
    case 4:  return inv_4x4(m, out);
    default: return SYN_ERROR;  /* Unsupported dimension */
    }
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  2D transforms (3×3 homogeneous)                                        */
/* ════════════════════════════════════════════════════════════════════════ */

void syn_matrix_rotate_2d(SYN_Matrix *out, q16_t angle)
{
    SYN_ASSERT(out != NULL);
    SYN_ASSERT(out->rows == 3 && out->cols == 3);

    q16_t c = q16_cos(angle);
    q16_t s = q16_sin(angle);

    syn_matrix_identity(out);
    SYN_MAT_AT(out, 0, 0) =  c;
    SYN_MAT_AT(out, 0, 1) = -s;
    SYN_MAT_AT(out, 1, 0) =  s;
    SYN_MAT_AT(out, 1, 1) =  c;
}

void syn_matrix_translate_2d(SYN_Matrix *out, q16_t tx, q16_t ty)
{
    SYN_ASSERT(out != NULL);
    SYN_ASSERT(out->rows == 3 && out->cols == 3);

    syn_matrix_identity(out);
    SYN_MAT_AT(out, 0, 2) = tx;
    SYN_MAT_AT(out, 1, 2) = ty;
}

void syn_matrix_scale_2d(SYN_Matrix *out, q16_t sx, q16_t sy)
{
    SYN_ASSERT(out != NULL);
    SYN_ASSERT(out->rows == 3 && out->cols == 3);

    syn_matrix_zero(out);
    SYN_MAT_AT(out, 0, 0) = sx;
    SYN_MAT_AT(out, 1, 1) = sy;
    SYN_MAT_AT(out, 2, 2) = Q16_ONE;
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  3D transforms (4×4 homogeneous)                                        */
/* ════════════════════════════════════════════════════════════════════════ */

void syn_matrix_rotate_x(SYN_Matrix *out, q16_t angle)
{
    SYN_ASSERT(out != NULL);
    SYN_ASSERT(out->rows == 4 && out->cols == 4);

    q16_t c = q16_cos(angle);
    q16_t s = q16_sin(angle);

    syn_matrix_identity(out);
    SYN_MAT_AT(out, 1, 1) =  c;
    SYN_MAT_AT(out, 1, 2) = -s;
    SYN_MAT_AT(out, 2, 1) =  s;
    SYN_MAT_AT(out, 2, 2) =  c;
}

void syn_matrix_rotate_y(SYN_Matrix *out, q16_t angle)
{
    SYN_ASSERT(out != NULL);
    SYN_ASSERT(out->rows == 4 && out->cols == 4);

    q16_t c = q16_cos(angle);
    q16_t s = q16_sin(angle);

    syn_matrix_identity(out);
    SYN_MAT_AT(out, 0, 0) =  c;
    SYN_MAT_AT(out, 0, 2) =  s;
    SYN_MAT_AT(out, 2, 0) = -s;
    SYN_MAT_AT(out, 2, 2) =  c;
}

void syn_matrix_rotate_z(SYN_Matrix *out, q16_t angle)
{
    SYN_ASSERT(out != NULL);
    SYN_ASSERT(out->rows == 4 && out->cols == 4);

    q16_t c = q16_cos(angle);
    q16_t s = q16_sin(angle);

    syn_matrix_identity(out);
    SYN_MAT_AT(out, 0, 0) =  c;
    SYN_MAT_AT(out, 0, 1) = -s;
    SYN_MAT_AT(out, 1, 0) =  s;
    SYN_MAT_AT(out, 1, 1) =  c;
}

void syn_matrix_translate_3d(SYN_Matrix *out, q16_t tx, q16_t ty, q16_t tz)
{
    SYN_ASSERT(out != NULL);
    SYN_ASSERT(out->rows == 4 && out->cols == 4);

    syn_matrix_identity(out);
    SYN_MAT_AT(out, 0, 3) = tx;
    SYN_MAT_AT(out, 1, 3) = ty;
    SYN_MAT_AT(out, 2, 3) = tz;
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  Vector helpers                                                         */
/* ════════════════════════════════════════════════════════════════════════ */

q16_t syn_vec_dot(const q16_t *a, const q16_t *b, uint8_t n)
{
    SYN_ASSERT(a != NULL && b != NULL);

    int64_t acc = 0;
    uint8_t i;
    for (i = 0; i < n; i++) {
        acc += (int64_t)a[i] * b[i];
    }
    return (q16_t)(acc >> Q16_SHIFT);
}

void syn_vec3_cross(const q16_t *a, const q16_t *b, q16_t *out)
{
    SYN_ASSERT(a != NULL && b != NULL && out != NULL);

    out[0] = (q16_t)(((int64_t)a[1] * b[2] - (int64_t)a[2] * b[1]) >> Q16_SHIFT);
    out[1] = (q16_t)(((int64_t)a[2] * b[0] - (int64_t)a[0] * b[2]) >> Q16_SHIFT);
    out[2] = (q16_t)(((int64_t)a[0] * b[1] - (int64_t)a[1] * b[0]) >> Q16_SHIFT);
}

q16_t syn_vec_norm(const q16_t *v, uint8_t n)
{
    SYN_ASSERT(v != NULL);

    int64_t sum = 0;
    uint8_t i;
    for (i = 0; i < n; i++) {
        sum += (int64_t)v[i] * v[i];
    }

    /* sum is in Q32.32. Convert to Q16.16 for q16_sqrt. */
    q16_t sum_q16 = (q16_t)(sum >> Q16_SHIFT);
    return q16_sqrt(sum_q16);
}

SYN_Status syn_vec_normalize(const q16_t *v, q16_t *out, uint8_t n)
{
    SYN_ASSERT(v != NULL && out != NULL);

    q16_t mag = syn_vec_norm(v, n);
    if (mag == 0) return SYN_ERROR;

    uint8_t i;
    for (i = 0; i < n; i++) {
        out[i] = q16_div(v[i], mag);
    }
    return SYN_OK;
}

#endif /* SYN_USE_MATRIX */
