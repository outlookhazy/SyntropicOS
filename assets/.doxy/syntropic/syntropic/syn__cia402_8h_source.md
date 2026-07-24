

# File syn\_cia402.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cia402.h**](syn__cia402_8h.md)

[Go to the documentation of this file](syn__cia402_8h.md)


```C++

#ifndef SYN_CIA402_H
#define SYN_CIA402_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../common/syn_defs.h"
#include "../util/syn_scurve.h"
#include "syn_canopen.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_CIA402_STATE_NOT_READY           = 0, 
    SYN_CIA402_STATE_SWITCH_ON_DISABLED = 1, 
    SYN_CIA402_STATE_READY_TO_SWITCH_ON = 2, 
    SYN_CIA402_STATE_SWITCHED_ON        = 3, 
    SYN_CIA402_STATE_OPERATION_ENABLED   = 4, 
    SYN_CIA402_STATE_QUICK_STOP_ACTIVE   = 5, 
    SYN_CIA402_STATE_FAULT_REACTION      = 6, 
    SYN_CIA402_STATE_FAULT               = 7  
} SYN_CiA402State;

typedef enum {
    SYN_CIA402_MODE_NO_MODE = 0,  
    SYN_CIA402_MODE_PP      = 1,  
    SYN_CIA402_MODE_PV      = 3,  
    SYN_CIA402_MODE_PT      = 4,  
    SYN_CIA402_MODE_HM      = 6,  
    SYN_CIA402_MODE_CSP     = 8,  
    SYN_CIA402_MODE_CSV     = 9,  
    SYN_CIA402_MODE_CST     = 10  
} SYN_CiA402Mode;

#define SYN_CIA402_CW_SWITCH_ON       (1U << 0) 
#define SYN_CIA402_CW_ENABLE_VOLTAGE  (1U << 1) 
#define SYN_CIA402_CW_QUICK_STOP      (1U << 2) 
#define SYN_CIA402_CW_ENABLE_OP       (1U << 3) 
#define SYN_CIA402_CW_NEW_SETPOINT    (1U << 4) 
#define SYN_CIA402_CW_CHANGE_IMMED    (1U << 5) 
#define SYN_CIA402_CW_ABS_REL         (1U << 6) 
#define SYN_CIA402_CW_FAULT_RESET     (1U << 7) 
#define SYN_CIA402_CW_HALT            (1U << 8) 
#define SYN_CIA402_SW_READY_TO_SWITCH_ON (1U << 0)  
#define SYN_CIA402_SW_SWITCHED_ON        (1U << 1)  
#define SYN_CIA402_SW_OPERATION_ENABLED  (1U << 2)  
#define SYN_CIA402_SW_FAULT              (1U << 3)  
#define SYN_CIA402_SW_VOLTAGE_ENABLED    (1U << 4)  
#define SYN_CIA402_SW_QUICK_STOP         (1U << 5)  
#define SYN_CIA402_SW_SWITCH_ON_DISABLED (1U << 6)  
#define SYN_CIA402_SW_WARNING            (1U << 7)  
#define SYN_CIA402_SW_TARGET_REACHED     (1U << 10) 
#define SYN_CIA402_SW_INTERNAL_LIMIT     (1U << 11) 
#define SYN_CIA402_SW_SETPOINT_ACK       (1U << 12) 
typedef struct {
    int32_t max_profile_velocity;   
    int32_t profile_acceleration;   
    int32_t profile_deceleration;   
    int32_t quick_stop_deceleration;
    int16_t max_torque;             
} SYN_CiA402Config;

typedef struct {
    SYN_CiA402State state;          
    SYN_CiA402Config cfg;           
    /* 0x6000-series Object Dictionary Variables */
    uint16_t controlword;           
    uint16_t statusword;            
    int8_t   mode_of_operation;     
    int8_t   mode_display;          
    int32_t  target_position;       
    int32_t  actual_position;       
    int32_t  target_velocity;       
    int32_t  actual_velocity;       
    int16_t  target_torque;         
    int16_t  actual_torque;         
    uint16_t error_code;            
    /* Motion Trajectory & State Flags */
    SYN_SCurve scurve;              
    float   current_speed;          
    float   current_pos;            
    bool    setpoint_ack;           
    bool    target_reached;         
    uint16_t prev_controlword;      
} SYN_CiA402Drive;

SYN_Status syn_cia402_init(SYN_CiA402Drive *drive, const SYN_CiA402Config *cfg);

SYN_Status syn_cia402_set_controlword(SYN_CiA402Drive *drive, uint16_t controlword);

uint16_t syn_cia402_get_statusword(const SYN_CiA402Drive *drive);

SYN_Status syn_cia402_set_mode(SYN_CiA402Drive *drive, int8_t mode);

SYN_Status syn_cia402_set_target_position(SYN_CiA402Drive *drive, int32_t target_pos);

SYN_Status syn_cia402_set_target_velocity(SYN_CiA402Drive *drive, int32_t target_vel);

SYN_Status syn_cia402_set_target_torque(SYN_CiA402Drive *drive, int16_t target_torque);

SYN_Status syn_cia402_update(SYN_CiA402Drive *drive, uint32_t dt_ms);

SYN_Status syn_cia402_report_actuals(SYN_CiA402Drive *drive, int32_t actual_pos, int32_t actual_vel, int16_t actual_torque);

SYN_Status syn_cia402_trigger_fault(SYN_CiA402Drive *drive, uint16_t error_code);

size_t syn_cia402_populate_od(SYN_CANOpenODEntry *entries, size_t max_entries, SYN_CiA402Drive *drive);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CIA402_H */
```


