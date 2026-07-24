

# File syn\_canopen.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_canopen.h**](syn__canopen_8h.md)

[Go to the documentation of this file](syn__canopen_8h.md)


```C++

#ifndef SYN_CANOPEN_H
#define SYN_CANOPEN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_CANOPEN_NMT_CMD_START      0x01U 
#define SYN_CANOPEN_NMT_CMD_STOP       0x02U 
#define SYN_CANOPEN_NMT_CMD_PREOP      0x80U 
#define SYN_CANOPEN_NMT_CMD_RESET_NODE 0x81U 
#define SYN_CANOPEN_NMT_CMD_RESET_COMM 0x82U 
#define SYN_CANOPEN_NMT_STATE_BOOTUP      0x00U 
#define SYN_CANOPEN_NMT_STATE_STOPPED     0x04U 
#define SYN_CANOPEN_NMT_STATE_OPERATIONAL 0x05U 
#define SYN_CANOPEN_NMT_STATE_PREOP       0x7FU 
#define SYN_CANOPEN_SDO_ABORT_NONE               0x00000000UL 
#define SYN_CANOPEN_SDO_ABORT_TOGGLE_BIT         0x05030000UL 
#define SYN_CANOPEN_SDO_ABORT_UNSUPPORTED_ACCESS 0x06010000UL 
#define SYN_CANOPEN_SDO_ABORT_WRITE_ONLY         0x06010001UL 
#define SYN_CANOPEN_SDO_ABORT_READ_ONLY          0x06010002UL 
#define SYN_CANOPEN_SDO_ABORT_NOT_EXIST          0x06020000UL 
#define SYN_CANOPEN_SDO_ABORT_TYPE_MISMATCH      0x06070010UL 
#define SYN_CANOPEN_SDO_ABORT_SUBINDEX_NOT_EXIST 0x06090011UL 
#define SYN_CANOPEN_ACCESS_RO 0x01U 
#define SYN_CANOPEN_ACCESS_WO 0x02U 
#define SYN_CANOPEN_ACCESS_RW 0x03U 
#define SYN_CANOPEN_TYPE_I8  0x02U 
#define SYN_CANOPEN_TYPE_I16 0x03U 
#define SYN_CANOPEN_TYPE_I32 0x04U 
#define SYN_CANOPEN_TYPE_U8  0x05U 
#define SYN_CANOPEN_TYPE_U16 0x06U 
#define SYN_CANOPEN_TYPE_U32 0x07U 
typedef struct {
    uint16_t index;       
    uint8_t  subindex;    
    uint8_t  type;        
    uint8_t  access;      
    void    *data_ptr;    
    size_t   size;        
} SYN_CANOpenODEntry;

typedef struct {
    uint16_t cob_id;       
    uint16_t od_index;     
    uint8_t  od_subindex;  
    uint8_t  enabled;      
} SYN_CANOpenPDOMap;

typedef struct {
    uint8_t           node_id;       
    uint16_t          heartbeat_ms;  
    SYN_CANOpenPDOMap rpdo[4];       
    SYN_CANOpenPDOMap tpdo[4];       
} SYN_CANOpenNodeConfig;

typedef struct {
    uint8_t                 node_id;             
    uint8_t                 nmt_state;           
    uint16_t                heartbeat_timer_ms;  
    SYN_CANOpenNodeConfig   cfg;                 
    const SYN_CANOpenODEntry *od_table;          
    size_t                  od_count;            
    uint32_t                tx_cob_id;           
    uint8_t                 tx_data[8];          
    uint8_t                 tx_len;              
    bool                    tx_pending;          
} SYN_CANOpenNode;

SYN_Status syn_canopen_init(SYN_CANOpenNode *node,
                            const SYN_CANOpenNodeConfig *cfg,
                            const SYN_CANOpenODEntry *od_table,
                            size_t od_count);

SYN_Status syn_canopen_process_rx(SYN_CANOpenNode *node,
                                  uint32_t cob_id,
                                  const uint8_t *data,
                                  uint8_t len);

SYN_Status syn_canopen_update(SYN_CANOpenNode *node, uint32_t dt_ms);

SYN_Status syn_canopen_send_emcy(SYN_CANOpenNode *node, uint16_t err_code, uint8_t err_reg);

SYN_Status syn_canopen_od_read(SYN_CANOpenNode *node,
                               uint16_t index,
                               uint8_t subindex,
                               void *buf,
                               size_t buf_size,
                               size_t *out_len);

SYN_Status syn_canopen_od_write(SYN_CANOpenNode *node,
                                uint16_t index,
                                uint8_t subindex,
                                const void *data,
                                size_t len);

bool syn_canopen_get_tx(SYN_CANOpenNode *node,
                        uint32_t *out_cob_id,
                        uint8_t *out_data,
                        uint8_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CANOPEN_H */
```


