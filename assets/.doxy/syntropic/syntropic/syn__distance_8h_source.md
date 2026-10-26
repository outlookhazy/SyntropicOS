

# File syn\_distance.h

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_distance.h**](syn__distance_8h.md)

[Go to the documentation of this file](syn__distance_8h.md)


```C++

#ifndef SYN_DISTANCE_H
#define SYN_DISTANCE_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_DISTANCE_ULTRASONIC = 0, 
    SYN_DISTANCE_TOF_LASER = 1,  
    SYN_DISTANCE_INFRARED = 2    
} SYN_DistanceType;

typedef struct {
    SYN_DistanceType type;        
    SYN_GPIO_Pin trig_pin;        
    SYN_GPIO_Pin echo_pin;        
    uint32_t last_distance_mm;    
    uint32_t min_range_mm;        
    uint32_t max_range_mm;        
    bool obstacle_detected;       
    uint32_t proximity_thresh_mm; 
} SYN_Distance;

SYN_Status syn_distance_init(SYN_Distance *sensor, SYN_GPIO_Pin trig_pin, SYN_GPIO_Pin echo_pin,
                             uint32_t min_mm, uint32_t max_mm, SYN_DistanceType type);

void syn_distance_feed_pulse(SYN_Distance *sensor, uint32_t pulse_us);

void syn_distance_set_proximity_threshold(SYN_Distance *sensor, uint32_t thresh_mm);

uint32_t syn_distance_get_mm(const SYN_Distance *sensor);

uint32_t syn_distance_get_cm(const SYN_Distance *sensor);

bool syn_distance_is_obstacle_detected(const SYN_Distance *sensor);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DISTANCE_H */
```


