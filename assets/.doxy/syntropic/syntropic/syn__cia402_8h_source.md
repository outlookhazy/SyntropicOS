

# File syn\_cia402.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cia402.h**](syn__cia402_8h.md)

[Go to the documentation of this file](syn__cia402_8h.md)


```C++

#ifndef SYN_CIA402_H
#define SYN_CIA402_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_CIA402) || SYN_USE_CIA402

#include "../common/syn_defs.h"
#include "syn_canopen.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── CiA 402 Device Control States (FSM) ─────────────────────────────────── */

typedef enum {
    SYN_CIA402_STATE_NOT_READY_TO_SWITCH_ON = 0, 
    SYN_CIA402_STATE_NOT_READY = 0,              
    SYN_CIA402_STATE_SWITCH_ON_DISABLED = 1,     
    SYN_CIA402_STATE_READY_TO_SWITCH_ON = 2,     
    SYN_CIA402_STATE_SWITCHED_ON = 3,            
    SYN_CIA402_STATE_OPERATION_ENABLED = 4,      
    SYN_CIA402_STATE_QUICK_STOP_ACTIVE = 5,      
    SYN_CIA402_STATE_FAULT_REACTION_ACTIVE = 6,  
    SYN_CIA402_STATE_FAULT = 7,                  
} SYN_CIA402_State;

typedef SYN_CIA402_State SYN_CiA402State;

/* ── CiA 402 Modes of Operation (0x6060 / 0x6061) ────────────────────────── */

typedef enum {
    SYN_CIA402_MODE_NO_MODE = 0,              
    SYN_CIA402_MODE_PROFILE_POSITION = 1,     
    SYN_CIA402_MODE_PP = 1,                   
    SYN_CIA402_MODE_PROFILE_VELOCITY = 3,     
    SYN_CIA402_MODE_PV = 3,                   
    SYN_CIA402_MODE_PROFILE_TORQUE = 4,       
    SYN_CIA402_MODE_PT = 4,                   
    SYN_CIA402_MODE_HOMING = 6,               
    SYN_CIA402_MODE_HM = 6,                   
    SYN_CIA402_MODE_CYCLIC_SYNC_POSITION = 8, 
    SYN_CIA402_MODE_CSP = 8,                  
    SYN_CIA402_MODE_CYCLIC_SYNC_VELOCITY = 9, 
    SYN_CIA402_MODE_CSV = 9,                  
    SYN_CIA402_MODE_CYCLIC_SYNC_TORQUE = 10,  
    SYN_CIA402_MODE_CST = 10,                 
} SYN_CIA402_Mode;

/* ── Controlword (0x6040) Bit Masks ──────────────────────────────────────── */

#define SYN_CIA402_CW_SWITCH_ON (1U << 0)        
#define SYN_CIA402_CW_ENABLE_VOLTAGE (1U << 1)   
#define SYN_CIA402_CW_QUICK_STOP (1U << 2)       
#define SYN_CIA402_CW_ENABLE_OPERATION (1U << 3) 
#define SYN_CIA402_CW_NEW_SETPOINT (1U << 4) 
#define SYN_CIA402_CW_CHANGE_IMMEDIATELY                                                  \
    (1U << 5)                               
#define SYN_CIA402_CW_RELATIVE (1U << 6)    
#define SYN_CIA402_CW_FAULT_RESET (1U << 7) 
#define SYN_CIA402_CW_HALT (1U << 8)        
#define SYN_CIA402_CW_CHANGE_ON_SETPOINT (1U << 9) 
/* ── Statusword (0x6041) Bit Masks ───────────────────────────────────────── */

#define SYN_CIA402_SW_READY_TO_SWITCH_ON (1U << 0) 
#define SYN_CIA402_SW_SWITCHED_ON (1U << 1)        
#define SYN_CIA402_SW_OPERATION_ENABLED (1U << 2)  
#define SYN_CIA402_SW_FAULT (1U << 3)              
#define SYN_CIA402_SW_VOLTAGE_ENABLED (1U << 4)    
#define SYN_CIA402_SW_QUICK_STOP \
    (1U << 5) 
#define SYN_CIA402_SW_SWITCH_ON_DISABLED (1U << 6) 
#define SYN_CIA402_SW_WARNING (1U << 7)            
#define SYN_CIA402_SW_MANUFACTURER_8 (1U << 8)     
#define SYN_CIA402_SW_REMOTE (1U << 9)             
#define SYN_CIA402_SW_TARGET_REACHED (1U << 10)  
#define SYN_CIA402_SW_INTERNAL_LIMIT (1U << 11)  
#define SYN_CIA402_SW_SETPOINT_ACK (1U << 12)    
#define SYN_CIA402_SW_SPEED_ZERO (1U << 12)      
#define SYN_CIA402_SW_HOMING_ATTAINED (1U << 12) 
#define SYN_CIA402_SW_FOLLOWING_ERROR (1U << 13) 
#define SYN_CIA402_SW_HOMING_ERROR (1U << 13)    
/* ── Standard CiA 402 Object Dictionary Indices ──────────────────────────── */

#define SYN_CIA402_OD_CONTROLWORD 0x6040U                
#define SYN_CIA402_OD_STATUSWORD 0x6041U                 
#define SYN_CIA402_OD_QUICK_STOP_OPTION 0x605AU          
#define SYN_CIA402_OD_MODES_OF_OPERATION 0x6060U         
#define SYN_CIA402_OD_MODES_OF_OPERATION_DISPLAY 0x6061U 
#define SYN_CIA402_OD_POSITION_DEMAND_VALUE 0x6062U      
#define SYN_CIA402_OD_POSITION_ACTUAL_VALUE 0x6064U      
#define SYN_CIA402_OD_FOLLOWING_ERROR_WINDOW 0x6065U     
#define SYN_CIA402_OD_POSITION_WINDOW 0x6067U            
#define SYN_CIA402_OD_VELOCITY_ACTUAL_VALUE 0x606CU      
#define SYN_CIA402_OD_TARGET_TORQUE 0x6071U              
#define SYN_CIA402_OD_MAX_TORQUE 0x6072U                 
#define SYN_CIA402_OD_TORQUE_ACTUAL_VALUE 0x6077U        
#define SYN_CIA402_OD_TARGET_POSITION 0x607AU            
#define SYN_CIA402_OD_SOFTWARE_POS_LIMIT_MIN 0x607DU     
#define SYN_CIA402_OD_SOFTWARE_POS_LIMIT_MAX 0x607EU     
#define SYN_CIA402_OD_PROFILE_VELOCITY 0x6081U           
#define SYN_CIA402_OD_PROFILE_ACCELERATION 0x6083U       
#define SYN_CIA402_OD_PROFILE_DECELERATION 0x6084U       
#define SYN_CIA402_OD_QUICK_STOP_DECELERATION 0x6085U    
#define SYN_CIA402_OD_TORQUE_SLOPE 0x6087U               
#define SYN_CIA402_OD_HOMING_METHOD 0x6098U              
#define SYN_CIA402_OD_HOMING_SPEED_FAST 0x6099U          
#define SYN_CIA402_OD_HOMING_SPEED_SLOW 0x609AU          
#define SYN_CIA402_OD_HOMING_ACCELERATION 0x609BU        
#define SYN_CIA402_OD_TARGET_VELOCITY 0x60FFU            
/* ── Configuration & Output Types ────────────────────────────────────────── */

typedef struct {
    int32_t sw_limit_min;             
    int32_t sw_limit_max;             
    bool sw_limits_enabled;           
    uint32_t max_profile_velocity;    
    uint32_t profile_acceleration;    
    uint32_t profile_deceleration;    
    uint32_t quick_stop_deceleration; 
    uint32_t following_err_window;    
    uint32_t position_window;         
    uint16_t max_torque;              
    uint32_t torque_slope;            
} SYN_CIA402_Config;

typedef SYN_CIA402_Config SYN_CiA402Config;

typedef struct {
    bool power_enabled;   
    bool brake_released;  
    SYN_CIA402_Mode mode; 
    int32_t position_cmd; 
    int32_t velocity_cmd; 
    int16_t torque_cmd;   
} SYN_CIA402_Setpoints;

typedef struct {
    /* State & Mode */
    SYN_CIA402_State state;        
    int8_t mode_of_operation;      
    int8_t mode_of_operation_disp; 
    uint16_t controlword;          
    uint16_t statusword;           
    uint16_t prev_controlword;     
    uint16_t error_code;           
    int16_t quick_stop_option;     
    /* Live Feedback & Status Variables */
    int32_t actual_position; 
    int32_t actual_velocity; 
    int16_t actual_torque;   
    bool target_reached;     
    bool setpoint_ack;       
    float current_speed;     
    /* Live Object Dictionary Variables */
    int32_t position_demand;    
    uint32_t following_err_win; 
    uint32_t position_window;   
    int16_t target_torque;      
    uint16_t max_torque;        
    int32_t target_position;    
    int32_t sw_limit_min;       
    int32_t sw_limit_max;       
    uint32_t profile_vel;       
    uint32_t profile_acc;       
    uint32_t profile_dec;       
    uint32_t quick_stop_dec;    
    uint32_t torque_slope;      
    int8_t homing_method;       
    uint32_t homing_speed_fast; 
    uint32_t homing_speed_slow; 
    uint32_t homing_accel;      
    int32_t target_velocity;    
    /* Internal Motion Profiler State */
    int64_t current_pos_fxp;    
    int64_t current_vel_fxp;    
    int32_t current_torque_fxp; 
    bool sw_limits_enabled;     
    bool homing_in_progress;    
    bool homing_attained;       
    bool homing_error;          
} SYN_CIA402_Axis;

typedef SYN_CIA402_Axis SYN_CiA402Drive;

/* ── Public API ─────────────────────────────────────────────────────────── */

SYN_Status syn_cia402_init(SYN_CIA402_Axis *axis, const SYN_CIA402_Config *cfg);

SYN_Status syn_cia402_set_controlword(SYN_CIA402_Axis *axis, uint16_t cw);

uint16_t syn_cia402_get_controlword(const SYN_CIA402_Axis *axis);

uint16_t syn_cia402_get_statusword(const SYN_CIA402_Axis *axis);

SYN_CIA402_State syn_cia402_get_state(const SYN_CIA402_Axis *axis);

SYN_Status syn_cia402_set_mode(SYN_CIA402_Axis *axis, int8_t mode);

int8_t syn_cia402_get_mode(const SYN_CIA402_Axis *axis);

int8_t syn_cia402_get_mode_display(const SYN_CIA402_Axis *axis);

SYN_Status syn_cia402_set_target_position(SYN_CIA402_Axis *axis, int32_t pos);

SYN_Status syn_cia402_set_target_velocity(SYN_CIA402_Axis *axis, int32_t vel);

SYN_Status syn_cia402_set_target_torque(SYN_CIA402_Axis *axis, int16_t torque);

SYN_Status syn_cia402_feed_feedback(SYN_CIA402_Axis *axis, int32_t pos_actual, int32_t vel_actual,
                                    int16_t torque_actual);

SYN_Status syn_cia402_report_actuals(SYN_CIA402_Axis *axis, int32_t pos_actual, int32_t vel_actual,
                                     int16_t torque_actual);

SYN_Status syn_cia402_update(SYN_CIA402_Axis *axis, uint32_t dt_ms);

SYN_Status syn_cia402_step(SYN_CIA402_Axis *axis, uint32_t dt_us,
                           SYN_CIA402_Setpoints *out_setpoints);

SYN_Status syn_cia402_trigger_fault(SYN_CIA402_Axis *axis, uint16_t error_code);

size_t syn_cia402_populate_od(SYN_CANOpenODEntry *od_table, size_t max_entries,
                              SYN_CIA402_Axis *axis);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_CIA402 */

#endif /* SYN_CIA402_H */
```


