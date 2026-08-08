

# File syn\_gbt27930.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_gbt27930.h**](syn__gbt27930_8h.md)

[Go to the documentation of this file](syn__gbt27930_8h.md)


```C++

#ifndef SYN_GBT27930_H
#define SYN_GBT27930_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_GBT27930) || SYN_USE_GBT27930

#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"
#include "syn_j1939.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_GBT27930_ADDR_BMS 0xF4U     
#define SYN_GBT27930_ADDR_CHARGER 0x56U 
#define SYN_GBT27930_PGN_CHM 0x002600U 
#define SYN_GBT27930_PGN_BHM 0x002700U 
#define SYN_GBT27930_PGN_CRM 0x000100U 
#define SYN_GBT27930_PGN_BRM 0x000200U 
#define SYN_GBT27930_PGN_BCP 0x000600U 
#define SYN_GBT27930_PGN_CML 0x000800U 
#define SYN_GBT27930_PGN_BRO 0x000900U 
#define SYN_GBT27930_PGN_CRO 0x000A00U 
#define SYN_GBT27930_PGN_BCL 0x001000U 
#define SYN_GBT27930_PGN_BCS 0x001100U 
#define SYN_GBT27930_PGN_CCS 0x001200U 
#define SYN_GBT27930_PGN_BSM 0x001300U 
#define SYN_GBT27930_PGN_BST 0x001900U 
#define SYN_GBT27930_PGN_CST 0x001A00U 
#define SYN_GBT27930_PGN_BSD 0x001C00U 
#define SYN_GBT27930_PGN_CSD 0x001D00U 
#define SYN_GBT27930_PGN_BEM 0x001E00U 
#define SYN_GBT27930_PGN_CEM 0x001F00U 
typedef enum {
    SYN_GBT27930_ROLE_BMS = 0,    
    SYN_GBT27930_ROLE_CHARGER = 1 
} SYN_GBT27930_Role;

typedef enum {
    SYN_GBT27930_STATE_IDLE = 0,
    SYN_GBT27930_STATE_HANDSHAKE = 1,
    SYN_GBT27930_STATE_PARAM_CONFIG = 2,
    SYN_GBT27930_STATE_CHARGING = 3,
    SYN_GBT27930_STATE_STOPPING = 4,
    SYN_GBT27930_STATE_ERROR = 5
} SYN_GBT27930_State;

typedef struct {
    uint8_t battery_type;       
    uint16_t rated_capacity_ah; 
    uint16_t rated_voltage_v;   
    uint16_t max_charge_volt_v; 
    uint16_t max_charge_curr_a; 
    uint16_t max_temp_c;        
    uint16_t max_cell_volt_v;   
    uint8_t vin[17];            
} SYN_GBT27930_BMS_Config;

typedef struct {
    uint16_t max_output_volt_v; 
    uint16_t min_output_volt_v; 
    uint16_t max_output_curr_a; 
    uint16_t min_output_curr_a; 
} SYN_GBT27930_Charger_Config;

typedef struct {
    uint16_t volt_demand_v;      
    uint16_t curr_demand_a;      
    uint8_t charge_mode;         
    uint16_t measured_volt_v;    
    uint16_t measured_curr_a;    
    uint8_t soc_percent;         
    uint16_t remaining_time_min; 
    uint16_t max_cell_volt_v;    
    uint8_t max_cell_temp_c;     
    uint8_t min_cell_temp_c;     
} SYN_GBT27930_Telemetry;

typedef struct {
    SYN_GBT27930_Role role;                  
    SYN_GBT27930_State state;                
    SYN_GBT27930_BMS_Config bms_cfg;         
    SYN_GBT27930_Charger_Config charger_cfg; 
    SYN_GBT27930_Telemetry telemetry;        
    bool ready_for_charging;                 
    bool peer_ready_for_charging;            
    uint32_t timer_ms;                       
    uint32_t timeout_ms;                     
    uint8_t stop_reason;                     
    uint8_t fault_code;                      
} SYN_GBT27930_Session;

void syn_gbt27930_init(SYN_GBT27930_Session *session, SYN_GBT27930_Role role);

SYN_Status syn_gbt27930_start_handshake(SYN_GBT27930_Session *session);

SYN_Status syn_gbt27930_process_rx_frame(SYN_GBT27930_Session *session, const SYN_CAN_Frame *frame);

bool syn_gbt27930_step(SYN_GBT27930_Session *session, uint32_t dt_ms, SYN_CAN_Frame *tx_frame);

void syn_gbt27930_stop_charging(SYN_GBT27930_Session *session, uint8_t reason);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_GBT27930 */

#endif /* SYN_GBT27930_H */
```


