

# File syn\_kalman.h

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_kalman.h**](syn__kalman_8h.md)

[Go to the documentation of this file](syn__kalman_8h.md)


```C++

#ifndef SYN_KALMAN_H
#define SYN_KALMAN_H

#include "../util/syn_matrix.h"
#include "../common/syn_defs.h"

#ifndef SYN_KALMAN_MAX_STATE
#define SYN_KALMAN_MAX_STATE  6
#endif

#ifndef SYN_KALMAN_MAX_MEAS
#define SYN_KALMAN_MAX_MEAS   4
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    SYN_Matrix *x;    
    SYN_Matrix *P;    
    SYN_Matrix *F;    
    SYN_Matrix *Q;    
    SYN_Matrix *H;    
    SYN_Matrix *R;    
    uint8_t n_state;  
    uint8_t n_meas;   
} SYN_Kalman_Config;

typedef struct {
    const SYN_Kalman_Config *cfg;   
    /* Scratch matrices — caller allocates, filter uses internally */
    SYN_Matrix *temp_nn_1;  
    SYN_Matrix *temp_nn_2;  
    SYN_Matrix *temp_nm;    
    SYN_Matrix *temp_mn;    
    SYN_Matrix *temp_mm;    
    SYN_Matrix *temp_mm_2;  
    SYN_Matrix *temp_n1;    
    SYN_Matrix *temp_m1;    
} SYN_Kalman;

#define SYN_KALMAN_SCRATCH_DECL(prefix, NS, NM)                 \
    SYN_MAT_DECL(prefix##_nn1, NS, NS);                        \
    SYN_MAT_DECL(prefix##_nn2, NS, NS);                        \
    SYN_MAT_DECL(prefix##_nm,  NS, NM);                        \
    SYN_MAT_DECL(prefix##_mn,  NM, NS);                        \
    SYN_MAT_DECL(prefix##_mm,  NM, NM);                        \
    SYN_MAT_DECL(prefix##_mm2, NM, NM);                        \
    SYN_MAT_DECL(prefix##_n1,  NS, 1);                         \
    SYN_MAT_DECL(prefix##_m1,  NM, 1)

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

SYN_Status syn_kalman_init(SYN_Kalman *kf, const SYN_Kalman_Config *cfg);

void syn_kalman_predict(SYN_Kalman *kf);

SYN_Status syn_kalman_update(SYN_Kalman *kf, const SYN_Matrix *z);

#ifdef __cplusplus
}
#endif

#endif /* SYN_KALMAN_H */
```


