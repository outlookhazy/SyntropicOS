

# File syn\_matrix.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_matrix.h**](syn__matrix_8h.md)

[Go to the documentation of this file](syn__matrix_8h.md)


```C++

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

typedef struct {
    q16_t   *data;    
    uint8_t  rows;    
    uint8_t  cols;    
} SYN_Matrix;

/* ── Declaration macros ─────────────────────────────────────────────────── */

#define SYN_MAT_DECL(name, R, C)                                \
    q16_t name##_store[(R) * (C)];                              \
    SYN_Matrix name = { name##_store, (R), (C) }

#define SYN_MAT_STATIC(name, R, C)                              \
    static q16_t name##_store[(R) * (C)];                       \
    static SYN_Matrix name = { name##_store, (R), (C) }

#define SYN_MAT_INIT(name, storage, R, C)                       \
    SYN_Matrix name = { (storage), (R), (C) }

#define SYN_MAT_AT(m, r, c)  ((m)->data[(r) * (m)->cols + (c)])

#define SYN_MAT2_DECL(name)  SYN_MAT_DECL(name, 2, 2)
#define SYN_MAT3_DECL(name)  SYN_MAT_DECL(name, 3, 3)
#define SYN_MAT4_DECL(name)  SYN_MAT_DECL(name, 4, 4)

/* ── Core operations ────────────────────────────────────────────────────── */

void syn_matrix_identity(SYN_Matrix *m);

void syn_matrix_zero(SYN_Matrix *m);

void syn_matrix_copy(SYN_Matrix *dst, const SYN_Matrix *src);

void syn_matrix_add(const SYN_Matrix *a, const SYN_Matrix *b, SYN_Matrix *out);

void syn_matrix_sub(const SYN_Matrix *a, const SYN_Matrix *b, SYN_Matrix *out);

void syn_matrix_scale(const SYN_Matrix *a, q16_t scalar, SYN_Matrix *out);

/* ── Multiply ───────────────────────────────────────────────────────────── */

void syn_matrix_mul(const SYN_Matrix *a, const SYN_Matrix *b, SYN_Matrix *out);

void syn_matrix_mul_vec(const SYN_Matrix *m, const q16_t *v_in,
                        q16_t *v_out, uint8_t n_in);

/* ── Transpose, trace ───────────────────────────────────────────────────── */

void syn_matrix_transpose(const SYN_Matrix *a, SYN_Matrix *out);

q16_t syn_matrix_trace(const SYN_Matrix *m);

/* ── Determinant & inverse ──────────────────────────────────────────────── */

q16_t syn_matrix_det(const SYN_Matrix *m);

SYN_Status syn_matrix_inv(const SYN_Matrix *m, SYN_Matrix *out);

/* ── 2D transforms (3×3 homogeneous) ────────────────────────────────────── */

void syn_matrix_rotate_2d(SYN_Matrix *out, q16_t angle);

void syn_matrix_translate_2d(SYN_Matrix *out, q16_t tx, q16_t ty);

void syn_matrix_scale_2d(SYN_Matrix *out, q16_t sx, q16_t sy);

/* ── 3D transforms (4×4 homogeneous) ────────────────────────────────────── */

void syn_matrix_rotate_x(SYN_Matrix *out, q16_t angle);

void syn_matrix_rotate_y(SYN_Matrix *out, q16_t angle);

void syn_matrix_rotate_z(SYN_Matrix *out, q16_t angle);

void syn_matrix_translate_3d(SYN_Matrix *out, q16_t tx, q16_t ty, q16_t tz);

/* ── Vector helpers ─────────────────────────────────────────────────────── */

q16_t syn_vec_dot(const q16_t *a, const q16_t *b, uint8_t n);

void syn_vec3_cross(const q16_t *a, const q16_t *b, q16_t *out);

q16_t syn_vec_norm(const q16_t *v, uint8_t n);

SYN_Status syn_vec_normalize(const q16_t *v, q16_t *out, uint8_t n);

/* ── Linear Solvers ─────────────────────────────────────────────────────── */

SYN_Status syn_matrix_solve_lu(const SYN_Matrix *A, const SYN_Matrix *b, SYN_Matrix *x);

SYN_Status syn_matrix_solve_cholesky(const SYN_Matrix *A, const SYN_Matrix *b, SYN_Matrix *x);

SYN_Status syn_matrix_least_squares(const SYN_Matrix *A, const SYN_Matrix *b, SYN_Matrix *x);

/* ── Block & Outer Product Operations ──────────────────────────────────── */

SYN_Status syn_matrix_get_block(const SYN_Matrix *src, uint8_t r0, uint8_t c0, SYN_Matrix *dst);

SYN_Status syn_matrix_set_block(SYN_Matrix *dst, uint8_t r0, uint8_t c0, const SYN_Matrix *src);

SYN_Status syn_matrix_outer_product(const q16_t *u, uint8_t rows, const q16_t *v, uint8_t cols, SYN_Matrix *out);

SYN_Status syn_matrix_qr(const SYN_Matrix *A, SYN_Matrix *Q, SYN_Matrix *R);

SYN_Status syn_matrix_eigen_sym2(const SYN_Matrix *A, q16_t evals[2], SYN_Matrix *E);

SYN_Status syn_matrix_eigen_sym3(const SYN_Matrix *A, q16_t evals[3], SYN_Matrix *E);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MATRIX_H */
```


