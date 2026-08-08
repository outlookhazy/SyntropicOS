

# File syn\_devicenet.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_devicenet.c**](syn__devicenet_8c.md)

[Go to the documentation of this file](syn__devicenet_8c.md)


```C++

#include "syn_devicenet.h"

#include "../util/syn_pack.h"

#include <string.h>

/* Group 2 CAN Message ID offsets */
#define GROUP2_EXPLICIT_RESP_MSG_ID 3U
#define GROUP2_EXPLICIT_REQ_MSG_ID 4U
#define GROUP2_POLLED_CMD_MSG_ID 5U
#define GROUP2_POLLED_RESP_MSG_ID 6U

/* Group 4 CAN Message IDs */
#define GROUP4_DUP_MAC_REQ_MSG_ID 5U
#define GROUP4_DUP_MAC_RESP_MSG_ID 6U

static uint32_t make_group2_can_id(uint8_t mac_id, uint8_t msg_id)
{
    return (uint32_t)0x400U | ((uint32_t)(mac_id & 0x3FU) << 3) | (uint32_t)(msg_id & 0x07U);
}

static uint32_t make_group4_can_id(uint8_t mac_id, uint8_t msg_id)
{
    return (uint32_t)0x7C0U | ((uint32_t)(mac_id & 0x3FU) << 3) | (uint32_t)(msg_id & 0x07U);
}

bool syn_devicenet_init(SYN_DeviceNet_Node *node, uint8_t mac_id, SYN_DeviceNet_Baud baud)
{
    if ((node == NULL) || (mac_id > SYN_DEVICENET_MAX_MAC_ID)) {
        return false;
    }

    memset(node, 0, sizeof(*node));
    node->state = SYN_DEVICENET_STATE_DUP_MAC_CHECK;
    node->devicenet_obj.mac_id = mac_id;
    node->devicenet_obj.baud_rate = baud;
    node->devicenet_obj.bus_off_action = 0U;
    node->devicenet_obj.allocation_choice = 0x03U; /* Explicit + Polled I/O */

    /* CIP Identity defaults */
    node->identity.vendor_id = 0x005AU;   /* SyntropicOS Vendor ID */
    node->identity.device_type = 0x000CU; /* Generic Discrete I/O */
    node->identity.product_code = 0x0001U;
    node->identity.rev_major = 1U;
    node->identity.rev_minor = 0U;
    node->identity.status = 0x0000U;
    node->identity.serial_number = 0x12345678U;
    memcpy(node->identity.product_name, "Syntropic DeviceNet", 20);
    node->identity.quick_connect_enabled = false;

    node->dup_mac_timer_ms = SYN_DEVICENET_DUP_MAC_TIMEOUT_MS;
    node->dup_mac_failed = false;
    node->explicit_connected = false;
    node->polled_connected = false;

    return true;
}

void syn_devicenet_set_serial_number(SYN_DeviceNet_Node *node, uint32_t serial_number)
{
    if (node != NULL) {
        node->identity.serial_number = serial_number;
    }
}

bool syn_devicenet_set_assembly(SYN_DeviceNet_Node *node, uint8_t *in_buf, uint8_t in_len,
                                uint8_t *out_buf, uint8_t out_len)
{
    if (node == NULL) {
        return false;
    }

    if ((in_len > SYN_DEVICENET_MAX_PAYLOAD) || (out_len > SYN_DEVICENET_MAX_PAYLOAD)) {
        return false;
    }

    node->assembly.input_buf = in_buf;
    node->assembly.input_len = in_len;
    node->assembly.output_buf = out_buf;
    node->assembly.output_len = out_len;

    return true;
}

bool syn_devicenet_set_quickconnect(SYN_DeviceNet_Node *node, bool enable)
{
    if (node == NULL) {
        return false;
    }

    node->identity.quick_connect_enabled = enable;
    if (enable && (node->state == SYN_DEVICENET_STATE_DUP_MAC_CHECK)) {
        /* QuickConnect shortens Dup MAC startup check to 100ms */
        node->dup_mac_timer_ms = 100U;
    }

    return true;
}

void syn_devicenet_poll(SYN_DeviceNet_Node *node, uint32_t dt_ms)
{
    if ((node == NULL) || (node->state != SYN_DEVICENET_STATE_DUP_MAC_CHECK)) {
        return;
    }

    if (dt_ms >= node->dup_mac_timer_ms) {
        node->dup_mac_timer_ms = 0U;
        if (!node->dup_mac_failed) {
            node->state = SYN_DEVICENET_STATE_ONLINE;
        }
    } else {
        node->dup_mac_timer_ms -= dt_ms;
    }
}

static bool handle_explicit_message(SYN_DeviceNet_Node *node, const uint8_t *data, uint8_t len,
                                    uint8_t *tx_data, uint8_t *tx_len)
{
    if (len < 4U) {
        return false;
    }

    uint8_t mac = data[0] & 0x3FU;
    uint8_t service = data[1] & 0x7FU;
    uint8_t class_id = data[2];
    uint8_t attr_id = data[3];

    (void)mac;

    /* Single Fragment Response Header */
    tx_data[0] = node->devicenet_obj.mac_id;
    tx_data[1] = service | 0x80U; /* Success Response Service Code */

    uint8_t out_idx = 2U;

    if (service == 0x0EU) {      /* Get_Attribute_Single */
        if (class_id == 0x01U) { /* Identity Object */
            switch (attr_id) {
            case 1U: /* Vendor ID */
                syn_poke_u16_le(node->identity.vendor_id, tx_data, out_idx);
                out_idx += 2U;
                break;
            case 2U: /* Device Type */
                syn_poke_u16_le(node->identity.device_type, tx_data, out_idx);
                out_idx += 2U;
                break;
            case 3U: /* Product Code */
                syn_poke_u16_le(node->identity.product_code, tx_data, out_idx);
                out_idx += 2U;
                break;
            case 4U: /* Revision */
                tx_data[out_idx++] = node->identity.rev_major;
                tx_data[out_idx++] = node->identity.rev_minor;
                break;
            case 6U: /* Serial Number */
                syn_poke_u32_le(node->identity.serial_number, tx_data, out_idx);
                out_idx += 4U;
                break;
            case 9U: /* QuickConnect attribute */
                tx_data[out_idx++] = node->identity.quick_connect_enabled ? 1U : 0U;
                break;
            default:
                tx_data[1] = 0x94U; /* Error response */
                tx_data[2] = 0x14U; /* Attribute not supported */
                out_idx = 3U;
                break;
            }
        } else if (class_id == 0x03U) { /* DeviceNet Object */
            switch (attr_id) {
            case 1U: /* MAC ID */
                tx_data[out_idx++] = node->devicenet_obj.mac_id;
                break;
            case 2U: /* Baud Rate */
                tx_data[out_idx++] = (uint8_t)node->devicenet_obj.baud_rate;
                break;
            default:
                tx_data[1] = 0x94U;
                tx_data[2] = 0x14U;
                out_idx = 3U;
                break;
            }
        } else if (class_id == 0x04U) { /* Assembly Object */
            if ((node->assembly.input_buf != NULL) && (node->assembly.input_len > 0U)) {
                memcpy(&tx_data[out_idx], node->assembly.input_buf, node->assembly.input_len);
                out_idx += node->assembly.input_len;
            }
        } else {
            tx_data[1] = 0x94U;
            tx_data[2] = 0x05U; /* Class not supported */
            out_idx = 3U;
        }
    } else if (service == 0x10U) { /* Set_Attribute_Single */
        if (len < 5U) {
            tx_data[1] = 0x94U;
            tx_data[2] = 0x13U; /* Too little data */
            out_idx = 3U;
        } else if ((class_id == 0x01U) && (attr_id == 9U)) {
            node->identity.quick_connect_enabled = (data[4] != 0U);
        } else if ((class_id == 0x03U) && (attr_id == 1U)) {
            if (data[4] <= SYN_DEVICENET_MAX_MAC_ID) {
                node->devicenet_obj.mac_id = data[4];
            }
        }
    } else {
        tx_data[1] = 0x94U;
        tx_data[2] = 0x08U; /* Service not supported */
        out_idx = 3U;
    }

    *tx_len = out_idx;
    return true;
}

bool syn_devicenet_on_can_rx(SYN_DeviceNet_Node *node, uint32_t can_id, const uint8_t *data,
                             uint8_t len, uint32_t *tx_can_id, uint8_t *tx_data, uint8_t *tx_len)
{
    if ((node == NULL) || (data == NULL) || (tx_can_id == NULL) || (tx_data == NULL) ||
        (tx_len == NULL)) {
        return false;
    }

    uint8_t mac = node->devicenet_obj.mac_id;

    /* 1. Group 4 Duplicate MAC ID Check Request */
    uint32_t dup_req_id = make_group4_can_id(mac, GROUP4_DUP_MAC_REQ_MSG_ID);
    if (can_id == dup_req_id) {
        if (node->state == SYN_DEVICENET_STATE_ONLINE) {
            /* Respond to defend MAC ID */
            *tx_can_id = make_group4_can_id(mac, GROUP4_DUP_MAC_RESP_MSG_ID);
            syn_poke_u32_le(node->identity.serial_number, tx_data, 0);
            *tx_len = 4U;
            return true;
        } else if (node->state == SYN_DEVICENET_STATE_DUP_MAC_CHECK) {
            /* Duplicate MAC ID collision detected during startup! */
            node->dup_mac_failed = true;
            node->state = SYN_DEVICENET_STATE_OFFLINE;
            return false;
        }
    }

    if (node->state != SYN_DEVICENET_STATE_ONLINE) {
        return false;
    }

    /* 2. Group 2 Explicit Request */
    uint32_t exp_req_id = make_group2_can_id(mac, GROUP2_EXPLICIT_REQ_MSG_ID);
    if (can_id == exp_req_id) {
        node->explicit_connected = true;
        *tx_can_id = make_group2_can_id(mac, GROUP2_EXPLICIT_RESP_MSG_ID);
        return handle_explicit_message(node, data, len, tx_data, tx_len);
    }

    /* 3. Group 2 Polled I/O Command */
    uint32_t poll_cmd_id = make_group2_can_id(mac, GROUP2_POLLED_CMD_MSG_ID);
    if (can_id == poll_cmd_id) {
        node->polled_connected = true;

        /* Write Master outputs to local assembly output buffer */
        if ((node->assembly.output_buf != NULL) && (len > 0U)) {
            uint8_t copy_len = len > node->assembly.output_len ? node->assembly.output_len : len;
            memcpy(node->assembly.output_buf, data, copy_len);
        }

        /* Build Polled I/O Response frame with local assembly inputs */
        *tx_can_id = make_group2_can_id(mac, GROUP2_POLLED_RESP_MSG_ID);
        if ((node->assembly.input_buf != NULL) && (node->assembly.input_len > 0U)) {
            memcpy(tx_data, node->assembly.input_buf, node->assembly.input_len);
            *tx_len = node->assembly.input_len;
        } else {
            *tx_len = 0U;
        }
        return true;
    }

    return false;
}
```


