

# File syn\_dc\_motor.h

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_dc\_motor.h**](syn__dc__motor_8h.md)

[Go to the documentation of this file](syn__dc__motor_8h.md)


```C++

#ifndef SYN_DC_MOTOR_H
#define SYN_DC_MOTOR_H

#include "../common/syn_defs.h"
#include "../drivers/syn_gpio.h"
#include "../motor/syn_motor_output.h"
#include "../port/syn_port_system.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── DC motor control mode ──────────────────────────────────────────────── */

typedef enum {
    SYN_DC_MODE_PWM_DIR    = 0,
    SYN_DC_MODE_DUAL_PWM   = 1,
} SYN_DCMotorMode;

#define SYN_DC_MOTOR_DUTY_MAX_DEFAULT  1000

/* ── DC motor descriptor ────────────────────────────────────────────────── */

typedef struct {
    /* Configuration */
    SYN_GPIO_Pin   pin_a;       
    SYN_GPIO_Pin   pin_b;       
    uint8_t         mode;        
    bool            invert;      
    /* State */
    int32_t         speed;       
    int32_t         target;      
    int32_t         ramp_rate;   
    uint32_t        last_tick;   
    int32_t         duty_max;

    void          (*set_duty)(SYN_GPIO_Pin pin, uint16_t duty, void *ctx);
    void           *duty_ctx;   
} SYN_DCMotor;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_dc_motor_init(SYN_DCMotor *motor, SYN_GPIO_Pin pin_a,
                        SYN_GPIO_Pin pin_b, SYN_DCMotorMode mode);

void syn_dc_motor_set_duty_callback(SYN_DCMotor *motor,
                                     void (*cb)(SYN_GPIO_Pin, uint16_t, void *),
                                     void *ctx);

void syn_dc_motor_set_speed(SYN_DCMotor *motor, int32_t speed);

void syn_dc_motor_ramp_to(SYN_DCMotor *motor, int32_t speed,
                           uint16_t duration);

void syn_dc_motor_update(SYN_DCMotor *motor);

void syn_dc_motor_coast(SYN_DCMotor *motor);

void syn_dc_motor_brake(SYN_DCMotor *motor);

static inline int32_t syn_dc_motor_get_speed(const SYN_DCMotor *motor)
{
    return motor->speed;
}

static inline bool syn_dc_motor_at_target(const SYN_DCMotor *motor)
{
    return motor->speed == motor->target;
}

void syn_dc_motor_set_duty_max(SYN_DCMotor *motor, int32_t duty_max);

SYN_MotorOutput syn_dc_motor_output(SYN_DCMotor *motor);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DC_MOTOR_H */
```


