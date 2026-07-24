

# File syn\_foc.h

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_foc.h**](syn__foc_8h.md)

[Go to the documentation of this file](syn__foc_8h.md)


```C++

#ifndef SYN_FOC_H
#define SYN_FOC_H

#include "../util/syn_qmath.h"
#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Types ──────────────────────────────────────────────────────────────── */

typedef struct {
    q16_t a;   
    q16_t b;   
    q16_t c;   
} SYN_FOC_ABC;

typedef struct {
    q16_t alpha;  
    q16_t beta;   
} SYN_FOC_AB;

typedef struct {
    q16_t d;   
    q16_t q;   
} SYN_FOC_DQ;

/* ── Clarke transform ───────────────────────────────────────────────────── */

void syn_foc_clarke(const SYN_FOC_ABC *abc, SYN_FOC_AB *ab);

void syn_foc_inv_clarke(const SYN_FOC_AB *ab, SYN_FOC_ABC *abc);

/* ── Park transform ─────────────────────────────────────────────────────── */

void syn_foc_park(const SYN_FOC_AB *ab, q16_t theta, SYN_FOC_DQ *dq);

void syn_foc_inv_park(const SYN_FOC_DQ *dq, q16_t theta, SYN_FOC_AB *ab);

/* ── Utility ────────────────────────────────────────────────────────────── */

void syn_foc_svpwm(const SYN_FOC_AB *ab, q16_t v_bus,
                   q16_t *duty_a, q16_t *duty_b, q16_t *duty_c);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FOC_H */
```


