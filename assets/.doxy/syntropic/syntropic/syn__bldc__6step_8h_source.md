

# File syn\_bldc\_6step.h

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_bldc\_6step.h**](syn__bldc__6step_8h.md)

[Go to the documentation of this file](syn__bldc__6step_8h.md)


```C++

#ifndef SYN_BLDC_6STEP_H
#define SYN_BLDC_6STEP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"
#include "syntropic/control/syn_pid.h"
#include "syntropic/util/syn_qmath.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    SYN_BLDC_GATE_OFF = 0, 
    SYN_BLDC_GATE_PWM = 1, 
    SYN_BLDC_GATE_LOW = 2, 
} SYN_BLDC_GateState;

typedef struct {
    SYN_BLDC_GateState u; 
    SYN_BLDC_GateState v; 
    SYN_BLDC_GateState w; 
    uint16_t duty;        
} SYN_BLDC_PhaseOutputs;

typedef enum {
    SYN_BLDC_DIR_CW = 1, 
    SYN_BLDC_DIR_CCW = 2 
} SYN_BLDC_Direction;

typedef enum {
    SYN_BLDC_STATE_STOPPED = 0, 
    SYN_BLDC_STATE_RUNNING = 1, 
    SYN_BLDC_STATE_FAULT = 2    
} SYN_BLDC_State;

typedef struct {
    uint8_t pole_pairs;     
    uint16_t pwm_frequency; 
} SYN_BLDC_Config;

typedef struct {
    SYN_BLDC_Config config;       
    SYN_BLDC_State state;         
    SYN_BLDC_Direction direction; 
    uint8_t current_step;         
    uint8_t hall_state;           
    uint16_t duty;                
    uint32_t hall_transitions;    
    uint32_t last_hall_tick_ms;   
    uint32_t rpm;                 
    SYN_PID speed_pid;            
    bool speed_pid_active;        
} SYN_BLDC_6Step;

SYN_Status syn_bldc_6step_init(SYN_BLDC_6Step *bldc, const SYN_BLDC_Config *cfg);

SYN_Status syn_bldc_6step_set_hall(SYN_BLDC_6Step *bldc, uint8_t hall_state,
                                   SYN_BLDC_PhaseOutputs *out);

SYN_Status syn_bldc_6step_set_duty(SYN_BLDC_6Step *bldc, uint16_t duty_0_to_1000);

SYN_Status syn_bldc_6step_set_direction(SYN_BLDC_6Step *bldc, SYN_BLDC_Direction dir);

SYN_Status syn_bldc_6step_start(SYN_BLDC_6Step *bldc);

SYN_Status syn_bldc_6step_stop(SYN_BLDC_6Step *bldc, SYN_BLDC_PhaseOutputs *out);

uint32_t syn_bldc_6step_update_speed(SYN_BLDC_6Step *bldc, uint32_t now_ms, uint32_t target_rpm);

SYN_Status syn_bldc_6step_get_phase_outputs(const SYN_BLDC_6Step *bldc, SYN_BLDC_PhaseOutputs *out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BLDC_6STEP_H */
```


