

# File syn\_n2k.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_n2k.h**](syn__n2k_8h.md)

[Go to the documentation of this file](syn__n2k_8h.md)


```C++

#ifndef SYN_N2K_H
#define SYN_N2K_H

#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"
#include "syn_j1939.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Network Management */
#define SYN_N2K_PGN_ISO_ACKNOWLEDGEMENT 59392U  
#define SYN_N2K_PGN_ISO_REQUEST         59904U  
#define SYN_N2K_PGN_ISO_ADDRESS_CLAIM   60928U  
#define SYN_N2K_PGN_SYSTEM_TIME         126992U 
#define SYN_N2K_PGN_PRODUCT_INFO        126996U 
/* Steering, Heading & Navigation */
#define SYN_N2K_PGN_RUDDER               127245U 
#define SYN_N2K_PGN_VESSEL_HEADING      127250U 
#define SYN_N2K_PGN_RATE_OF_TURN        127251U 
#define SYN_N2K_PGN_ATTITUDE            127257U 
#define SYN_N2K_PGN_MAGNETIC_VARIATION  127258U 
#define SYN_N2K_PGN_SPEED_WATER         128259U 
#define SYN_N2K_PGN_WATER_DEPTH         128267U 
#define SYN_N2K_PGN_DISTANCE_LOG        128275U 
#define SYN_N2K_PGN_POS_RAPID           129025U 
#define SYN_N2K_PGN_COG_SOG_RAPID       129026U 
#define SYN_N2K_PGN_GNSS_POS            129029U 
/* Power & Battery Systems */
#define SYN_N2K_PGN_INVERTER_STATUS     127501U 
#define SYN_N2K_PGN_AC_INPUT_STATUS     127503U 
#define SYN_N2K_PGN_AC_OUTPUT_STATUS    127504U 
#define SYN_N2K_PGN_FLUID_LEVEL         127505U 
#define SYN_N2K_PGN_DC_DETAILED_STATUS  127506U 
#define SYN_N2K_PGN_CHARGER_STATUS      127507U 
#define SYN_N2K_PGN_BATTERY_STATUS      127508U 
#define SYN_N2K_PGN_CHARGER_CONFIG      127509U 
/* Engine & Transmission */
#define SYN_N2K_PGN_ENGINE_RAPID        127488U 
#define SYN_N2K_PGN_ENGINE_DYNAMIC      127489U 
#define SYN_N2K_PGN_TRANSMISSION_PARAMS 127493U 
#define SYN_N2K_PGN_TRIM_TAB            127497U 
/* Environment */
#define SYN_N2K_PGN_WIND_DATA           130306U 
#define SYN_N2K_PGN_ENV_PARAMS          130310U 
#define SYN_N2K_PGN_ENVIRONMENTAL_ACT   130311U 
typedef struct {
    int32_t latitude_1e7;  
    int32_t longitude_1e7; 
} SYN_N2K_PositionRapid;

typedef struct {
    uint8_t  sid;             
    uint8_t  cog_ref;         
    uint16_t cog_rad_1e4;     
    uint16_t sog_m_s_1e2;     
} SYN_N2K_CogSogRapid;

typedef struct {
    uint8_t  sid;             
    uint16_t heading_rad_1e4; 
    int16_t  deviation_rad_1e4;
    int16_t  variation_rad_1e4;
    uint8_t  heading_ref;     
} SYN_N2K_VesselHeading;

typedef struct {
    uint8_t  sid;             
    uint8_t  instance;        
    uint16_t voltage_1e2;     
    int16_t  current_1e1;     
    uint16_t temperature_1e1; 
} SYN_N2K_BatteryStatus;

typedef struct {
    uint8_t  sid;             
    uint8_t  instance;        
    uint8_t  dc_type;         
    uint8_t  state_of_charge; 
    uint8_t  state_of_health; 
    uint16_t time_to_go_min;  
    uint16_t capacity_ah_1e1; 
} SYN_N2K_DcDetailedStatus;

typedef struct {
    uint8_t  sid;             
    uint16_t water_temp_1e2;  
    uint16_t air_temp_1e2;    
    uint16_t pressure_pa_1e2; 
} SYN_N2K_EnvParams;

typedef struct {
    bool     active;           
    uint8_t  seq_id;           
    uint8_t  expected_frame;   
    uint8_t  sa;               
    uint32_t pgn;              
    uint8_t  total_bytes;      
    uint8_t  received_bytes;   
    uint8_t  data[223];        
} SYN_N2K_FastPacketRx;

SYN_Status syn_n2k_encode_position_rapid(uint8_t sa, const SYN_N2K_PositionRapid *pos, SYN_CAN_Frame *frame);

SYN_Status syn_n2k_decode_position_rapid(const SYN_CAN_Frame *frame, SYN_N2K_PositionRapid *pos);

SYN_Status syn_n2k_encode_cog_sog_rapid(uint8_t sa, const SYN_N2K_CogSogRapid *cog_sog, SYN_CAN_Frame *frame);

SYN_Status syn_n2k_decode_cog_sog_rapid(const SYN_CAN_Frame *frame, SYN_N2K_CogSogRapid *cog_sog);

SYN_Status syn_n2k_encode_heading(uint8_t sa, const SYN_N2K_VesselHeading *heading, SYN_CAN_Frame *frame);

SYN_Status syn_n2k_decode_heading(const SYN_CAN_Frame *frame, SYN_N2K_VesselHeading *heading);

SYN_Status syn_n2k_encode_battery(uint8_t sa, const SYN_N2K_BatteryStatus *battery, SYN_CAN_Frame *frame);

SYN_Status syn_n2k_decode_battery(const SYN_CAN_Frame *frame, SYN_N2K_BatteryStatus *battery);

SYN_Status syn_n2k_encode_dc_detailed(uint8_t sa, const SYN_N2K_DcDetailedStatus *dc, SYN_CAN_Frame *frame);

SYN_Status syn_n2k_decode_dc_detailed(const SYN_CAN_Frame *frame, SYN_N2K_DcDetailedStatus *dc);

SYN_Status syn_n2k_encode_environment(uint8_t sa, const SYN_N2K_EnvParams *env, SYN_CAN_Frame *frame);

SYN_Status syn_n2k_decode_environment(const SYN_CAN_Frame *frame, SYN_N2K_EnvParams *env);

SYN_Status syn_n2k_fastpacket_process(SYN_N2K_FastPacketRx *rx,
                                      const SYN_CAN_Frame *frame,
                                      uint32_t target_pgn,
                                      const uint8_t **out_payload,
                                      size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_N2K_H */
```


