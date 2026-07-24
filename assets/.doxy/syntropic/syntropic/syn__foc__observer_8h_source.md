

# File syn\_foc\_observer.h

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_foc\_observer.h**](syn__foc__observer_8h.md)

[Go to the documentation of this file](syn__foc__observer_8h.md)


```C++

#ifndef SYN_FOC_OBSERVER_H
#define SYN_FOC_OBSERVER_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    q16_t R;       
    q16_t L;       
    q16_t G;       
    q16_t dt;      
    q16_t Kp_pll;  
    q16_t Ki_pll;  
} SYN_FOCObserverConfig;

typedef struct {
    SYN_FOCObserverConfig cfg; 
    q16_t I_alpha_est; 
    q16_t I_beta_est;  
    q16_t E_alpha_est; 
    q16_t E_beta_est;  
    q16_t theta_e;     
    q16_t omega_e;     
    q16_t pll_i_accum; 
} SYN_FOCObserver;

SYN_Status syn_foc_observer_init(SYN_FOCObserver *obs, const SYN_FOCObserverConfig *cfg);

SYN_Status syn_foc_observer_update(SYN_FOCObserver *obs, q16_t V_alpha, q16_t V_beta,
                                    q16_t I_alpha, q16_t I_beta);

q16_t syn_foc_observer_get_angle(const SYN_FOCObserver *obs);

q16_t syn_foc_observer_get_speed(const SYN_FOCObserver *obs);

void syn_foc_observer_reset(SYN_FOCObserver *obs);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FOC_OBSERVER_H */
```


