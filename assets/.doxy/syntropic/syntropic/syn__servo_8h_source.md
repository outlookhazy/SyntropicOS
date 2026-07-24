

# File syn\_servo.h

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_servo.h**](syn__servo_8h.md)

[Go to the documentation of this file](syn__servo_8h.md)


```C++

#ifndef SYN_SERVO_H
#define SYN_SERVO_H

#include "../port/syn_port_system.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Servo descriptor ───────────────────────────────────────────────────── */

typedef struct {
    /* Configuration */
    uint16_t  pulse_min;    
    uint16_t  pulse_max;    
    uint16_t  angle_range;  
    /* State */
    uint16_t  current_us;   
    uint16_t  target_us;    
    int32_t   rate;         
    uint32_t  last_tick;    
} SYN_Servo;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_servo_init(SYN_Servo *servo, uint16_t pulse_min,
                     uint16_t pulse_max, uint16_t angle_range);

void syn_servo_set_angle(SYN_Servo *servo, uint16_t angle);

void syn_servo_set_pulse(SYN_Servo *servo, uint16_t us);

void syn_servo_move_to(SYN_Servo *servo, uint16_t angle, uint16_t duration);

void syn_servo_update(SYN_Servo *servo);

static inline uint16_t syn_servo_get_pulse_us(const SYN_Servo *servo)
{
    return servo->current_us;
}

static inline uint16_t syn_servo_get_angle(const SYN_Servo *servo)
{
    uint32_t range_us = (uint32_t)(servo->pulse_max - servo->pulse_min);
    if (range_us == 0) return 0;
    return (uint16_t)(((uint32_t)(servo->current_us - servo->pulse_min) *
                       servo->angle_range) / range_us);
}

static inline bool syn_servo_at_target(const SYN_Servo *servo)
{
    return servo->current_us == servo->target_us;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_SERVO_H */
```


