

# File syn\_sensor\_fusion.h

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_sensor\_fusion.h**](syn__sensor__fusion_8h.md)

[Go to the documentation of this file](syn__sensor__fusion_8h.md)


```C++

#ifndef SYN_SENSOR_FUSION_H
#define SYN_SENSOR_FUSION_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    q16_t w; 
    q16_t x; 
    q16_t y; 
    q16_t z; 
} SYN_Quaternion;

typedef struct {
    q16_t roll_rad;  
    q16_t pitch_rad; 
    q16_t yaw_rad;   
} SYN_EulerAngles;

typedef struct {
    q16_t Kp;         
    q16_t Ki;         
    q16_t dt;         
    SYN_Quaternion q; 
    q16_t e_int[3];   
} SYN_SensorFusion;

void syn_sensor_fusion_init(SYN_SensorFusion *f, q16_t Kp, q16_t Ki, q16_t dt);

SYN_Status syn_sensor_fusion_update(SYN_SensorFusion *f, q16_t gx, q16_t gy, q16_t gz,
                                     q16_t ax, q16_t ay, q16_t az);

SYN_Status syn_sensor_fusion_get_quaternion(const SYN_SensorFusion *f, SYN_Quaternion *q);

SYN_Status syn_sensor_fusion_get_euler(const SYN_SensorFusion *f, SYN_EulerAngles *euler);

void syn_sensor_fusion_reset(SYN_SensorFusion *f);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SENSOR_FUSION_H */
```


