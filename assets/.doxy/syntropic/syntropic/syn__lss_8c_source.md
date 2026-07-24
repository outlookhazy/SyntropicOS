

# File syn\_lss.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lss.c**](syn__lss_8c.md)

[Go to the documentation of this file](syn__lss_8c.md)


```C++

#include "syn_lss.h"
#include "../util/syn_assert.h"
#include <string.h>

void syn_lss_slave_init(SYN_LSSSlave *slave, const SYN_LSSAddress *addr, uint8_t initial_node_id)
{
    SYN_ASSERT(slave != NULL && addr != NULL);
    memset(slave, 0, sizeof(*slave));
    slave->mode       = SYN_LSS_MODE_OPERATION;
    slave->node_id    = initial_node_id;
    slave->baud_table = SYN_LSS_BAUD_250K;
    slave->addr       = *addr;
    slave->store_req  = false;
}

bool syn_lss_slave_process(SYN_LSSSlave *slave, const SYN_CAN_Frame *frame, SYN_CAN_Frame *resp)
{
    SYN_ASSERT(slave != NULL && frame != NULL && resp != NULL);

    /* LSS Requests must use COB-ID 0x7E5 and DLC 8 */
    if (frame->id != SYN_LSS_COB_ID_MASTER || frame->dlc < 8) {
        return false;
    }

    uint8_t cs = frame->data[0];
    memset(resp, 0, sizeof(*resp));
    resp->id = SYN_LSS_COB_ID_SLAVE;
    resp->dlc = 8;

    switch (cs) {
    case SYN_LSS_CS_SWITCH_MODE_GLOBAL:
        slave->mode = frame->data[1];
        return false; /* Global switch has no response */

    case SYN_LSS_CS_CONFIGURE_NODE_ID:
        if (slave->mode == SYN_LSS_MODE_CONFIGURATION) {
            uint8_t new_id = frame->data[1];
            resp->data[0] = SYN_LSS_CS_CONFIGURE_NODE_ID;
            if (new_id >= 1 && new_id <= 127) {
                slave->node_id = new_id;
                resp->data[1] = 0; /* Error code 0 = protocol error / OK */
                resp->data[2] = 0; /* Spec error 0 = success */
            } else {
                resp->data[1] = 1; /* Out of range error */
                resp->data[2] = 0xFF;
            }
            return true;
        }
        break;

    case SYN_LSS_CS_CONFIGURE_BIT_TIMING:
        if (slave->mode == SYN_LSS_MODE_CONFIGURATION) {
            uint8_t baud_idx = frame->data[2];
            resp->data[0] = SYN_LSS_CS_CONFIGURE_BIT_TIMING;
            if (baud_idx <= SYN_LSS_BAUD_10K) {
                slave->baud_table = baud_idx;
                resp->data[1] = 0; /* Success */
                resp->data[2] = 0;
            } else {
                resp->data[1] = 1; /* Unsupported bit timing */
                resp->data[2] = 0xFF;
            }
            return true;
        }
        break;

    case SYN_LSS_CS_STORE_CONFIG:
        if (slave->mode == SYN_LSS_MODE_CONFIGURATION) {
            slave->store_req = true;
            resp->data[0] = SYN_LSS_CS_STORE_CONFIG;
            resp->data[1] = 0; /* Success */
            resp->data[2] = 0;
            return true;
        }
        break;

    case SYN_LSS_CS_INQUIRE_VENDOR:
        if (slave->mode == SYN_LSS_MODE_CONFIGURATION) {
            resp->data[0] = SYN_LSS_CS_INQUIRE_VENDOR;
            resp->data[1] = (uint8_t)(slave->addr.vendor_id);
            resp->data[2] = (uint8_t)(slave->addr.vendor_id >> 8);
            resp->data[3] = (uint8_t)(slave->addr.vendor_id >> 16);
            resp->data[4] = (uint8_t)(slave->addr.vendor_id >> 24);
            return true;
        }
        break;

    case SYN_LSS_CS_INQUIRE_PRODUCT:
        if (slave->mode == SYN_LSS_MODE_CONFIGURATION) {
            resp->data[0] = SYN_LSS_CS_INQUIRE_PRODUCT;
            resp->data[1] = (uint8_t)(slave->addr.product_code);
            resp->data[2] = (uint8_t)(slave->addr.product_code >> 8);
            resp->data[3] = (uint8_t)(slave->addr.product_code >> 16);
            resp->data[4] = (uint8_t)(slave->addr.product_code >> 24);
            return true;
        }
        break;

    case SYN_LSS_CS_INQUIRE_REV:
        if (slave->mode == SYN_LSS_MODE_CONFIGURATION) {
            resp->data[0] = SYN_LSS_CS_INQUIRE_REV;
            resp->data[1] = (uint8_t)(slave->addr.revision_no);
            resp->data[2] = (uint8_t)(slave->addr.revision_no >> 8);
            resp->data[3] = (uint8_t)(slave->addr.revision_no >> 16);
            resp->data[4] = (uint8_t)(slave->addr.revision_no >> 24);
            return true;
        }
        break;

    case SYN_LSS_CS_INQUIRE_SERIAL:
        if (slave->mode == SYN_LSS_MODE_CONFIGURATION) {
            resp->data[0] = SYN_LSS_CS_INQUIRE_SERIAL;
            resp->data[1] = (uint8_t)(slave->addr.serial_no);
            resp->data[2] = (uint8_t)(slave->addr.serial_no >> 8);
            resp->data[3] = (uint8_t)(slave->addr.serial_no >> 16);
            resp->data[4] = (uint8_t)(slave->addr.serial_no >> 24);
            return true;
        }
        break;

    default:
        break;
    }

    return false;
}

void syn_lss_master_build_switch_mode(SYN_CAN_Frame *frame, uint8_t mode)
{
    SYN_ASSERT(frame != NULL);
    memset(frame, 0, sizeof(*frame));
    frame->id = SYN_LSS_COB_ID_MASTER;
    frame->dlc = 8;
    frame->data[0] = SYN_LSS_CS_SWITCH_MODE_GLOBAL;
    frame->data[1] = mode;
}

void syn_lss_master_build_config_node_id(SYN_CAN_Frame *frame, uint8_t new_node_id)
{
    SYN_ASSERT(frame != NULL);
    memset(frame, 0, sizeof(*frame));
    frame->id = SYN_LSS_COB_ID_MASTER;
    frame->dlc = 8;
    frame->data[0] = SYN_LSS_CS_CONFIGURE_NODE_ID;
    frame->data[1] = new_node_id;
}

void syn_lss_master_build_config_bit_timing(SYN_CAN_Frame *frame, uint8_t baud_table_idx)
{
    SYN_ASSERT(frame != NULL);
    memset(frame, 0, sizeof(*frame));
    frame->id = SYN_LSS_COB_ID_MASTER;
    frame->dlc = 8;
    frame->data[0] = SYN_LSS_CS_CONFIGURE_BIT_TIMING;
    frame->data[1] = 0; /* Selector: 0 = Table, 1 = Bitrate */
    frame->data[2] = baud_table_idx;
}

void syn_lss_master_build_store_config(SYN_CAN_Frame *frame)
{
    SYN_ASSERT(frame != NULL);
    memset(frame, 0, sizeof(*frame));
    frame->id = SYN_LSS_COB_ID_MASTER;
    frame->dlc = 8;
    frame->data[0] = SYN_LSS_CS_STORE_CONFIG;
}
```


