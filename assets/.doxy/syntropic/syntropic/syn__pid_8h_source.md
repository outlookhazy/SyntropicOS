

# File syn\_pid.h

[**File List**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_pid.h**](syn__pid_8h.md)

[Go to the documentation of this file](syn__pid_8h.md)


```C++

#ifndef SYN_PID_H
#define SYN_PID_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration ──────────────────────────────────────────────────────── */

typedef struct {
    int32_t  kp;              
    int32_t  ki;              
    int32_t  kd;              
    int32_t  scale;           
    int32_t  out_min;         
    int32_t  out_max;         
    int32_t  integral_max;    
    uint8_t  d_filter_alpha;  
} SYN_PID_Config;

#define SYN_PID_GAINS(kp_f, ki_f, kd_f, scale_val, omin, omax) \
    ((SYN_PID_Config){                                          \
        .kp    = (int32_t)((kp_f) * (scale_val)),               \
        .ki    = (int32_t)((ki_f) * (scale_val) * 1000),        \
        .kd    = (int32_t)((kd_f) * (scale_val)),               \
        .scale = (scale_val),                                   \
        .out_min = (omin),                                      \
        .out_max = (omax),                                      \
    })

/* ── PID instance ───────────────────────────────────────────────────────── */

typedef struct {
    SYN_PID_Config cfg;       
    int32_t  integral;        
    int32_t  prev_error;      
    int32_t  prev_d_filtered; 
    int32_t  output;          
    bool     first;           
} SYN_PID;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_pid_init(SYN_PID *pid, const SYN_PID_Config *cfg);

int32_t syn_pid_update(SYN_PID *pid, int32_t setpoint,
                        int32_t measured, uint32_t dt_ms);

void syn_pid_reset(SYN_PID *pid);

void syn_pid_set_gains(SYN_PID *pid, int32_t kp, int32_t ki, int32_t kd);

void syn_pid_set_limits(SYN_PID *pid, int32_t out_min, int32_t out_max);

static inline int32_t syn_pid_output(const SYN_PID *pid)
{
    return pid->output;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_PID_H */
```


