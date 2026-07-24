

# File syn\_lss.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lss.h**](syn__lss_8h.md)

[Go to the documentation of this file](syn__lss_8h.md)


```C++

#ifndef SYN_LSS_H
#define SYN_LSS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_LSS_COB_ID_MASTER 0x7E5U 
#define SYN_LSS_COB_ID_SLAVE  0x7E4U 
#define SYN_LSS_CS_SWITCH_MODE_GLOBAL   0x04U 
#define SYN_LSS_CS_CONFIGURE_NODE_ID    0x11U 
#define SYN_LSS_CS_CONFIGURE_BIT_TIMING 0x13U 
#define SYN_LSS_CS_ACTIVATE_BIT_TIMING  0x15U 
#define SYN_LSS_CS_STORE_CONFIG         0x17U 
#define SYN_LSS_CS_SWITCH_MODE_SELECT   0x40U 
#define SYN_LSS_CS_INQUIRE_VENDOR       0x5AU 
#define SYN_LSS_CS_INQUIRE_PRODUCT      0x5BU 
#define SYN_LSS_CS_INQUIRE_REV          0x5CU 
#define SYN_LSS_CS_INQUIRE_SERIAL       0x5DU 
#define SYN_LSS_MODE_OPERATION     0x00U 
#define SYN_LSS_MODE_CONFIGURATION 0x01U 
#define SYN_LSS_BAUD_1000K 0x00U 
#define SYN_LSS_BAUD_800K  0x01U 
#define SYN_LSS_BAUD_500K  0x02U 
#define SYN_LSS_BAUD_250K  0x03U 
#define SYN_LSS_BAUD_125K  0x04U 
#define SYN_LSS_BAUD_100K  0x05U 
#define SYN_LSS_BAUD_50K   0x06U 
#define SYN_LSS_BAUD_20K   0x07U 
#define SYN_LSS_BAUD_10K   0x08U 
typedef struct {
    uint32_t vendor_id;     
    uint32_t product_code;  
    uint32_t revision_no;   
    uint32_t serial_no;     
} SYN_LSSAddress;

typedef struct {
    uint8_t        mode;        
    uint8_t        node_id;     
    uint8_t        baud_table;  
    SYN_LSSAddress addr;        
    bool           store_req;   
} SYN_LSSSlave;

void syn_lss_slave_init(SYN_LSSSlave *slave, const SYN_LSSAddress *addr, uint8_t initial_node_id);

bool syn_lss_slave_process(SYN_LSSSlave *slave, const SYN_CAN_Frame *frame, SYN_CAN_Frame *resp);

void syn_lss_master_build_switch_mode(SYN_CAN_Frame *frame, uint8_t mode);

void syn_lss_master_build_config_node_id(SYN_CAN_Frame *frame, uint8_t new_node_id);

void syn_lss_master_build_config_bit_timing(SYN_CAN_Frame *frame, uint8_t baud_table_idx);

void syn_lss_master_build_store_config(SYN_CAN_Frame *frame);

#ifdef __cplusplus
}
#endif

#endif /* SYN_LSS_H */
```


