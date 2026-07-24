

# File syn\_autotune.h

[**File List**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_autotune.h**](syn__autotune_8h.md)

[Go to the documentation of this file](syn__autotune_8h.md)


```C++

#ifndef SYN_AUTOTUNE_H
#define SYN_AUTOTUNE_H

#include "../common/syn_defs.h"
#include "../motor/syn_motor_ctrl.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Mode ──────────────────────────────────────────────────────────────── */

typedef enum {
    SYN_ATUNE_MODE_FF_IDENT = 0,   
    SYN_ATUNE_MODE_RELAY    = 1,   
    SYN_ATUNE_MODE_AUTO     = 2,   
} SYN_AutoTune_Mode;

/* ── PID formula ───────────────────────────────────────────────────────── */

typedef enum {
    SYN_ATUNE_ZN_CLASSIC       = 0,  
    SYN_ATUNE_ZN_NO_OVERSHOOT  = 1,  
    SYN_ATUNE_TYREUS_LUYBEN    = 2,  
} SYN_AutoTune_Method;

/* ── State ─────────────────────────────────────────────────────────────── */

typedef enum {
    SYN_ATUNE_IDLE,
    SYN_ATUNE_PROBE,
    SYN_ATUNE_RAMP_UP,
    SYN_ATUNE_SETTLING,
    SYN_ATUNE_MEASURING,
    SYN_ATUNE_SETTLING_2,
    SYN_ATUNE_MEASURING_2,
    SYN_ATUNE_RELAY,
    SYN_ATUNE_BRAKING,
    SYN_ATUNE_RAMP_DOWN,
    SYN_ATUNE_DONE,
    SYN_ATUNE_ABORTED
} SYN_AutoTune_State;

typedef enum {
    SYN_ATUNE_FLAG_NONE     = 0,
    SYN_ATUNE_FLAG_IDENT_KV = (1 << 0), 
    SYN_ATUNE_FLAG_IDENT_KA = (1 << 1), 
    SYN_ATUNE_FLAG_TUNE_PID = (1 << 2), 
    SYN_ATUNE_FLAG_ALL = (SYN_ATUNE_FLAG_IDENT_KV | 
                          SYN_ATUNE_FLAG_IDENT_KA | 
                          SYN_ATUNE_FLAG_TUNE_PID)
} SYN_AutoTune_Flags;

/* ── Abort reason ──────────────────────────────────────────────────────── */

typedef enum {
    SYN_ATUNE_OK               = 0,  
    SYN_ATUNE_ABORT_POSITION   = 1,  
    SYN_ATUNE_ABORT_VELOCITY   = 2,  
    SYN_ATUNE_ABORT_SOFT_LIMIT = 3,  
    SYN_ATUNE_ABORT_WATCHDOG   = 4,  
    SYN_ATUNE_ABORT_USER       = 5,  
    SYN_ATUNE_ABORT_STALL      = 6,  
    SYN_ATUNE_ABORT_NO_MOTION  = 7,  
} SYN_AutoTune_AbortReason;

#define SYN_ATUNE_LOG_ID  0x4154  /* 'AT' */

typedef struct {
    uint8_t  state;      
    int16_t  output;     
    int32_t  position;   
    int32_t  velocity;   
} SYN_AutoTune_LogFrame;

typedef struct {
    int32_t  position_min;     
    int32_t  position_max;     
    int32_t  max_velocity;     
    uint32_t watchdog_ms;      
} SYN_AutoTune_Limits;

/* ── Configuration ─────────────────────────────────────────────────────── */

typedef struct {
    SYN_AutoTune_Mode   mode;        
    int32_t             test_output;

    /* FF identification timing */
    uint32_t            settle_ms;       
    uint32_t            measure_ms;      
    /* Relay feedback config */
    int32_t             setpoint;        
    uint8_t             relay_cycles;    
    SYN_AutoTune_Method method;          
    /* Telemetry (optional) */
    SYN_DataLog        *datalog;         
    SYN_AutoTune_Flags  flags;           
    /* ── Safety (all mandatory for heavy machinery) ─────────────── */
    SYN_AutoTune_Limits limits;          
    uint32_t            watchdog_ms;

    uint32_t            ramp_ms;

    uint16_t            gain_multiplier_pct;
} SYN_AutoTune_Config;

/* ── Result ────────────────────────────────────────────────────────────── */

typedef struct {
    /* Feedforward identification results */
    int32_t  ff_kv;           
    int32_t  ff_ka;           
    uint8_t  ff_scale;        
    int32_t  steady_velocity_1; 
    int32_t  steady_velocity_2; 
    /* Relay feedback results */
    int32_t  Ku;              
    uint32_t Tu_ms;           
    int32_t  kp;              
    int32_t  ki;              
    int32_t  kd;              
    uint8_t  pid_scale;       
} SYN_AutoTune_Result;

/* ── Auto-tuner instance ───────────────────────────────────────────────── */

typedef struct {
    SYN_AutoTune_Config   cfg;            
    SYN_AutoTune_Result   result;         
    SYN_AutoTune_State    state;          
    SYN_AutoTune_AbortReason abort_reason;
    SYN_MotorCtrl        *ctrl;           
    /* Internal state */
    int32_t               start_position; 
    uint32_t              phase_start_tick;
    uint32_t              last_update_tick;
    int32_t               current_output;  
    int32_t               start_output;    
    /* Steady-state monitoring */
    int32_t               history_v;       
    uint32_t              last_check_tick; 
    /* FF ident internals */
    int64_t               velocity_sum;   
    uint32_t              velocity_samples;
    int32_t               ka_v1;           
    int32_t               ka_v2;           
    uint32_t              ka_t1;           
    uint32_t              ka_t2;           
    bool                  ka_p1_captured; 
    bool                  ka_p2_captured; 
    /* Oscillation tracking */
    int32_t               relay_output;    
    uint8_t               half_cycles;     
    uint32_t              last_cross_tick;  
    uint32_t              period_sum;       
    uint8_t               period_count;    
    int32_t               osc_peak_pos;    
    int32_t               osc_peak_neg;    
    int32_t               amplitude_sum;   
    uint8_t               amplitude_count; 
    bool                  above_setpoint;  
} SYN_AutoTune;

/* ── API ───────────────────────────────────────────────────────────────── */

SYN_Status syn_autotune_init(SYN_AutoTune *at, SYN_MotorCtrl *ctrl,
                              const SYN_AutoTune_Config *cfg);

SYN_AutoTune_State syn_autotune_update(SYN_AutoTune *at);

static inline const SYN_AutoTune_Result *
syn_autotune_result(const SYN_AutoTune *at) { return &at->result; }

static inline SYN_AutoTune_AbortReason
syn_autotune_abort_reason(const SYN_AutoTune *at) { return at->abort_reason; }

void syn_autotune_apply(SYN_AutoTune *at);

void syn_autotune_abort(SYN_AutoTune *at);

/* ── One-call auto-tune API ────────────────────────────────────────────── */

SYN_Status syn_autotune_start(SYN_AutoTune *at, SYN_MotorCtrl *ctrl,
                               const SYN_AutoTune_Limits *limits,
                               SYN_AutoTune_Flags flags,
                               uint16_t gain_multiplier);

#ifdef __cplusplus
}
#endif

#endif /* SYN_AUTOTUNE_H */
```


