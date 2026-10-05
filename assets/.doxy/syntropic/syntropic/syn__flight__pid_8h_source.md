

# File syn\_flight\_pid.h

[**File List**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_flight\_pid.h**](syn__flight__pid_8h.md)

[Go to the documentation of this file](syn__flight__pid_8h.md)


```C++

#ifndef SYN_FLIGHT_PID_H
#define SYN_FLIGHT_PID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"
#include "syntropic/control/syn_pid.h"
#include "syntropic/util/syn_qmath.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    q16_t gyro_roll;   
    q16_t gyro_pitch;  
    q16_t gyro_yaw;    
    q16_t angle_roll;  
    q16_t angle_pitch; 
} SYN_Flight_IMU;

typedef struct {
    uint16_t throttle_us; 
    q16_t roll_target;    
    q16_t pitch_target;   
    q16_t yaw_target;     
    bool angle_mode;      
} SYN_Flight_Commands;

typedef struct {
    uint16_t m1; 
    uint16_t m2; 
    uint16_t m3; 
    uint16_t m4; 
} SYN_Flight_MotorOutputs;

typedef struct {
    SYN_PID pid_rate_roll;   
    SYN_PID pid_rate_pitch;  
    SYN_PID pid_rate_yaw;    
    SYN_PID pid_angle_roll;  
    SYN_PID pid_angle_pitch; 
} SYN_Flight_Controller;

SYN_Status syn_flight_init(SYN_Flight_Controller *fc);

SYN_Status syn_flight_update(SYN_Flight_Controller *fc, const SYN_Flight_IMU *imu,
                             const SYN_Flight_Commands *cmd, uint32_t dt_ms,
                             SYN_Flight_MotorOutputs *motors);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FLIGHT_PID_H */
```


