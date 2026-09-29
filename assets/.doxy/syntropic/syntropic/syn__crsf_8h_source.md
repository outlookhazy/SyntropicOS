

# File syn\_crsf.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_crsf.h**](syn__crsf_8h.md)

[Go to the documentation of this file](syn__crsf_8h.md)


```C++

#ifndef SYN_CRSF_H
#define SYN_CRSF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_CRSF_MAX_PACKET_LEN 64U 
#define SYN_CRSF_NUM_CHANNELS 16U   
#define SYN_CRSF_ADDR_FC 0xC8U      
typedef enum {
    SYN_CRSF_TYPE_GPS = 0x02U,
    SYN_CRSF_TYPE_BATTERY = 0x08U,
    SYN_CRSF_TYPE_LINK_STATISTICS = 0x14U,
    SYN_CRSF_TYPE_RC_CHANNELS = 0x16U
} SYN_CRSF_FrameType;

typedef struct {
    uint16_t channels[SYN_CRSF_NUM_CHANNELS]; 
} SYN_CRSF_ChannelsFrame;

typedef struct {
    uint8_t uplink_rssi1;     
    uint8_t uplink_rssi2;     
    uint8_t uplink_quality;   
    int8_t uplink_snr;        
    uint8_t active_antenna;   
    uint8_t rf_mode;          
    uint8_t tx_power;         
    uint8_t downlink_rssi;    
    uint8_t downlink_quality; 
    int8_t downlink_snr;      
} SYN_CRSF_LinkStats;

typedef struct {
    uint8_t buf[SYN_CRSF_MAX_PACKET_LEN]; 
    uint8_t idx;                          
    uint8_t payload_len;                  
    uint32_t packets_received;            
    uint32_t crc_errors;                  
    SYN_CRSF_ChannelsFrame last_channels; 
    SYN_CRSF_LinkStats last_link_stats;   
} SYN_CRSF_Parser;

uint8_t syn_crsf_calc_crc(const uint8_t *buf, size_t len);

SYN_Status syn_crsf_init(SYN_CRSF_Parser *parser);

SYN_Status syn_crsf_parse_byte(SYN_CRSF_Parser *parser, uint8_t byte, SYN_CRSF_FrameType *type_out);

uint16_t syn_crsf_raw_to_us(uint16_t raw_val);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CRSF_H */
```


