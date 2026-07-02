

# File syn\_stepper.h

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_stepper.h**](syn__stepper_8h.md)

[Go to the documentation of this file](syn__stepper_8h.md)


```C++

#ifndef SYN_STEPPER_H
#define SYN_STEPPER_H

#include "../common/syn_defs.h"
#include "../drivers/syn_gpio.h"
#include "../motor/syn_motor_output.h"
#include "../port/syn_port_system.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Stepper state ──────────────────────────────────────────────────────── */

typedef enum {
    SYN_STEPPER_IDLE    = 0,         
    SYN_STEPPER_ACCEL   = 1,         
    SYN_STEPPER_CRUISE  = 2,         
    SYN_STEPPER_DECEL   = 3,         
} SYN_StepperState;

typedef enum {
    SYN_STEPPER_CW  = 0,  
    SYN_STEPPER_CCW = 1,  
} SYN_StepperDir;

/* ── Stepper descriptor ─────────────────────────────────────────────────── */

typedef struct {
    /* Configuration */
    SYN_GPIO_Pin  step_pin;        
    SYN_GPIO_Pin  dir_pin;         
    SYN_GPIO_Pin  enable_pin;     
    bool           dir_invert;     
    bool           enable_invert;  
    /* Motion parameters */
    uint32_t       max_speed;      
    uint32_t       accel;          
    /* Internal state */
    uint8_t        state;          
    int32_t        position;       
    int32_t        target;         
    uint32_t       speed;          
    uint32_t       step_interval;  
    uint32_t       last_step_tick; 
    int32_t        steps_to_go;    
    int32_t        decel_start;    
    bool           step_state;     
} SYN_Stepper;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_stepper_init(SYN_Stepper *s,
                       SYN_GPIO_Pin step_pin,
                       SYN_GPIO_Pin dir_pin);

void syn_stepper_set_enable_pin(SYN_Stepper *s, SYN_GPIO_Pin pin,
                                 bool active_low);

void syn_stepper_set_speed(SYN_Stepper *s, uint32_t max_sps,
                            uint32_t accel_sps2);

void syn_stepper_move(SYN_Stepper *s, int32_t steps);

void syn_stepper_move_to(SYN_Stepper *s, int32_t position);

void syn_stepper_tick(SYN_Stepper *s);

void syn_stepper_stop(SYN_Stepper *s);

void syn_stepper_enable(const SYN_Stepper *s, bool enable);

static inline bool syn_stepper_is_moving(const SYN_Stepper *s)
{
    return s->state != (uint8_t)SYN_STEPPER_IDLE;
}

static inline int32_t syn_stepper_position(const SYN_Stepper *s)
{
    return s->position;
}

static inline void syn_stepper_set_position(SYN_Stepper *s, int32_t pos)
{
    s->position = pos;
}

SYN_MotorOutput syn_stepper_output(SYN_Stepper *stepper);

#ifdef __cplusplus
}
#endif

#endif /* SYN_STEPPER_H */
```


