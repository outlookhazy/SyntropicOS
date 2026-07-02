

# File syn\_actuator.h

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_actuator.h**](syn__actuator_8h.md)

[Go to the documentation of this file](syn__actuator_8h.md)


```C++

#ifndef SYN_ACTUATOR_H
#define SYN_ACTUATOR_H

#include "../common/syn_defs.h"
#include "../motor/syn_dc_motor.h"
#include "../motor/syn_motor_ctrl.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration ──────────────────────────────────────────────────────── */

typedef struct {
    /* Motor */
    SYN_DCMotor    *dc_motor;       
    /* Position feedback */
    SYN_MotorCtrl_ReadPos read_pos;  
    void            *read_ctx;        
    int32_t          stroke_min;      
    int32_t          stroke_max;      
    /* Control */
    uint16_t         update_hz;       
    int32_t          pid_kp;          
    int32_t          pid_ki;          
    int32_t          pid_kd;          
    uint8_t          pid_scale;       
    int32_t          deadband;        
    /* Safety */
    uint16_t         stall_timeout_ms; 
    int32_t          stall_threshold;  
    SYN_ErrLog     *errlog;          
} SYN_Actuator_Config;

/* ── Actuator instance ──────────────────────────────────────────────────── */

typedef struct {
    SYN_MotorCtrl   ctrl;            
    int32_t          stroke_min;      
    int32_t          stroke_max;      
    int32_t          stroke_range;    
    int16_t          target_pct;      
    int16_t          current_pct;     
} SYN_Actuator;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_actuator_init(SYN_Actuator *act, const SYN_Actuator_Config *cfg);

void syn_actuator_set_position(SYN_Actuator *act, int16_t pct_x10);

int16_t syn_actuator_update(SYN_Actuator *act);

void syn_actuator_stop(SYN_Actuator *act);

static inline int16_t syn_actuator_position(const SYN_Actuator *act)
{
    return act->current_pct;
}

static inline bool syn_actuator_at_target(const SYN_Actuator *act)
{
    int16_t diff = act->target_pct - act->current_pct;
    if (diff < 0) diff = (int16_t)(-diff);
    return diff <= 5;  /* ±0.5% tolerance */
}

static inline bool syn_actuator_is_stalled(const SYN_Actuator *act)
{
    return act->ctrl.state == SYN_MCTRL_STALLED;
}

void syn_actuator_clear_stall(SYN_Actuator *act);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ACTUATOR_H */
```


