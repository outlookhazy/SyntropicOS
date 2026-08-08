

# File syn\_bacnet.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_bacnet.c**](syn__bacnet_8c.md)

[Go to the documentation of this file](syn__bacnet_8c.md)


```C++

#include "syn_bacnet.h"

#include <string.h>

/* ── CRC Calculations (ISO 16484-5 Clause 9.3) ─────────────────────────── */

uint8_t syn_bacnet_crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0xFFU;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x01U) {
                crc = (uint8_t)((crc >> 1) ^ 0x91U);
            } else {
                crc = (uint8_t)(crc >> 1);
            }
        }
    }
    return (uint8_t)(~crc);
}

uint16_t syn_bacnet_crc16(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFFU;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001U) {
                crc = (uint16_t)((crc >> 1) ^ 0xA001U);
            } else {
                crc = (uint16_t)(crc >> 1);
            }
        }
    }
    return (uint16_t)(~crc);
}

/* ── MS/TP Framing ──────────────────────────────────────────────────────── */

size_t syn_bacnet_mstp_encode_frame(uint8_t frame_type, uint8_t dest_mac, uint8_t src_mac,
                                    const uint8_t *payload, uint16_t payload_len, uint8_t *out_buf)
{
    if (!out_buf)
        return 0;

    out_buf[0] = SYN_BACNET_MSTP_PREAMBLE_1;
    out_buf[1] = SYN_BACNET_MSTP_PREAMBLE_2;
    out_buf[2] = frame_type;
    out_buf[3] = dest_mac;
    out_buf[4] = src_mac;
    out_buf[5] = (uint8_t)((payload_len >> 8) & 0xFFU);
    out_buf[6] = (uint8_t)(payload_len & 0xFFU);
    out_buf[7] = syn_bacnet_crc8(&out_buf[2], 5);

    size_t idx = 8;
    if (payload && payload_len > 0) {
        memcpy(&out_buf[idx], payload, payload_len);
        idx += payload_len;

        uint16_t data_crc = syn_bacnet_crc16(payload, payload_len);
        out_buf[idx++] = (uint8_t)(data_crc & 0xFFU);
        out_buf[idx++] = (uint8_t)((data_crc >> 8) & 0xFFU);
    }

    return idx;
}

bool syn_bacnet_mstp_decode_frame(const uint8_t *buf, size_t len, SYN_BACnet_MSTP_Frame *frame)
{
    if (!buf || !frame || len < 8)
        return false;

    if (buf[0] != SYN_BACNET_MSTP_PREAMBLE_1 || buf[1] != SYN_BACNET_MSTP_PREAMBLE_2)
        return false;

    uint8_t header_crc = syn_bacnet_crc8(&buf[2], 5);
    if (header_crc != buf[7])
        return false;

    frame->frame_type = buf[2];
    frame->destination_mac = buf[3];
    frame->source_mac = buf[4];
    frame->data_len = ((uint16_t)buf[5] << 8) | (uint16_t)buf[6];

    if (frame->data_len > 501)
        return false;

    if (frame->data_len > 0) {
        if (len < (size_t)(8 + frame->data_len + 2))
            return false;

        memcpy(frame->payload, &buf[8], frame->data_len);

        uint16_t expected_crc = syn_bacnet_crc16(frame->payload, frame->data_len);
        uint16_t actual_crc =
            (uint16_t)buf[8 + frame->data_len] | ((uint16_t)buf[8 + frame->data_len + 1] << 8);

        if (expected_crc != actual_crc)
            return false;
    }

    return true;
}

/* ── Node & Object Management ───────────────────────────────────────────── */

SYN_Status syn_bacnet_node_init(SYN_BACnet_Node *node, uint8_t mac_address, uint32_t device_id)
{
    if (!node)
        return SYN_INVALID_PARAM;

    memset(node, 0, sizeof(SYN_BACnet_Node));
    node->mac_address = mac_address;
    node->device_id = device_id;
    node->max_master = 127;

    /* Automatically register Device Object */
    syn_bacnet_add_object(node, SYN_BACNET_OBJ_DEVICE, device_id, 0.0f,
                          "SyntropicOS-BACnet-Device");

    return SYN_OK;
}

SYN_Status syn_bacnet_add_object(SYN_BACnet_Node *node, uint8_t object_type, uint32_t instance_id,
                                 float init_value, const char *name)
{
    if (!node || node->object_count >= SYN_BACNET_MAX_OBJECTS)
        return SYN_ERROR;

    SYN_BACnet_Object *obj = &node->objects[node->object_count++];
    obj->object_type = object_type;
    obj->instance_id = instance_id;
    obj->present_value = init_value;
    obj->name = name;

    return SYN_OK;
}

/* ── Process Incoming APDU Requests ─────────────────────────────────────── */

SYN_Status syn_bacnet_node_process(SYN_BACnet_Node *node, const SYN_BACnet_MSTP_Frame *rx_frame,
                                   SYN_BACnet_MSTP_Frame *tx_frame, bool *has_tx)
{
    if (!node || !rx_frame || !tx_frame || !has_tx)
        return SYN_INVALID_PARAM;

    *has_tx = false;

    /* Ignore frames not addressed to local MAC or Broadcast */
    if (rx_frame->destination_mac != node->mac_address &&
        rx_frame->destination_mac != SYN_BACNET_BROADCAST_MAC) {
        return SYN_OK;
    }

    /* Handle Token / Poll For Master MS/TP control frames */
    if (rx_frame->frame_type == SYN_BACNET_MSTP_FRAME_TOKEN) {
        return SYN_OK;
    }

    if (rx_frame->frame_type == SYN_BACNET_MSTP_FRAME_POLL_FOR_MASTER) {
        if (rx_frame->destination_mac == node->mac_address) {
            tx_frame->frame_type = SYN_BACNET_MSTP_FRAME_REPLY_TO_POLL_FOR_MASTER;
            tx_frame->destination_mac = rx_frame->source_mac;
            tx_frame->source_mac = node->mac_address;
            tx_frame->data_len = 0;
            *has_tx = true;
        }
        return SYN_OK;
    }

    /* Process APDU Payload */
    if (rx_frame->data_len < 2) {
        return SYN_OK;
    }

    uint8_t pdu_type = rx_frame->payload[0] & 0xF0U;

    /* 1. Unconfirmed Request (PDU Type 0x10): Who-Is -> Respond with I-Am */
    if (pdu_type == 0x10U) {
        uint8_t service_choice = rx_frame->payload[1];
        if (service_choice == SYN_BACNET_SERVICE_UNCONFIRMED_WHO_IS) {
            uint8_t payload[16];
            payload[0] = 0x10;                                /* Unconfirmed APDU */
            payload[1] = SYN_BACNET_SERVICE_UNCONFIRMED_I_AM; /* Service Choice I-Am */

            /* Device Object Identifier (4 bytes) */
            payload[2] = 0xC4;
            payload[3] = (uint8_t)((node->device_id >> 16) & 0xFFU);
            payload[4] = (uint8_t)((node->device_id >> 8) & 0xFFU);
            payload[5] = (uint8_t)(node->device_id & 0xFFU);

            /* Vendor ID (2 bytes) */
            payload[6] = 0x21;
            payload[7] = 0x05;

            tx_frame->frame_type = SYN_BACNET_MSTP_FRAME_DATA_NOT_EXPECTING_REPLY;
            tx_frame->destination_mac = SYN_BACNET_BROADCAST_MAC;
            tx_frame->source_mac = node->mac_address;
            tx_frame->data_len = 8;
            memcpy(tx_frame->payload, payload, 8);
            *has_tx = true;
            return SYN_OK;
        }
    }

    /* 2. Confirmed Request (PDU Type 0x00) */
    if (pdu_type == 0x00U && rx_frame->data_len >= 4) {
        uint8_t invoke_id = rx_frame->payload[2];
        uint8_t service_choice = rx_frame->payload[3];

        if (service_choice == SYN_BACNET_SERVICE_CONFIRMED_READ_PROPERTY) {
            /* Decode requested instance_id if payload contains Context Tag 0 */
            uint32_t req_instance = 0;
            if (rx_frame->data_len >= 9 && rx_frame->payload[4] == 0x0C) {
                uint32_t obj_id = ((uint32_t)rx_frame->payload[5] << 24) |
                                  ((uint32_t)rx_frame->payload[6] << 16) |
                                  ((uint32_t)rx_frame->payload[7] << 8) |
                                  (uint32_t)rx_frame->payload[8];
                req_instance = obj_id & 0x003FFFFFU;
            }

            /* Look up requested object by instance_id */
            float val = 0.0f;
            bool found = false;

            for (size_t i = 0; i < node->object_count; i++) {
                if (node->objects[i].instance_id == req_instance || req_instance == 0) {
                    val = node->objects[i].present_value;
                    found = true;
                    break;
                }
            }

            if (found) {
                uint8_t payload[32];
                payload[0] = 0x30; /* Complex-ACK APDU */
                payload[1] = invoke_id;
                payload[2] = SYN_BACNET_SERVICE_CONFIRMED_READ_PROPERTY;

                /* Encoded Present Value Float */
                payload[3] = 0x44; /* Real tag */
                memcpy(&payload[4], &val, sizeof(float));

                tx_frame->frame_type = SYN_BACNET_MSTP_FRAME_DATA_NOT_EXPECTING_REPLY;
                tx_frame->destination_mac = rx_frame->source_mac;
                tx_frame->source_mac = node->mac_address;
                tx_frame->data_len = 8;
                memcpy(tx_frame->payload, payload, 8);
                *has_tx = true;
            }
            return SYN_OK;
        }

        if (service_choice == SYN_BACNET_SERVICE_CONFIRMED_WRITE_PROPERTY) {
            /* WriteProperty APDU: Object ID at payload[4..8], Property Value float at
             * payload[10..13] */
            uint32_t req_instance = 0;
            if (rx_frame->data_len >= 9 && rx_frame->payload[4] == 0x0C) {
                uint32_t obj_id = ((uint32_t)rx_frame->payload[5] << 24) |
                                  ((uint32_t)rx_frame->payload[6] << 16) |
                                  ((uint32_t)rx_frame->payload[7] << 8) |
                                  (uint32_t)rx_frame->payload[8];
                req_instance = obj_id & 0x003FFFFFU;
            }

            float new_val = 0.0f;
            if (rx_frame->data_len >= 14 && rx_frame->payload[9] == 0x44) {
                memcpy(&new_val, &rx_frame->payload[10], sizeof(float));
            }

            /* Update object value */
            for (size_t i = 0; i < node->object_count; i++) {
                if (node->objects[i].instance_id == req_instance || req_instance == 0) {
                    node->objects[i].present_value = new_val;
                    break;
                }
            }

            /* Return WriteProperty Simple-ACK APDU (PDU Type 0x20) */
            uint8_t payload[4];
            payload[0] = 0x20; /* Simple-ACK APDU */
            payload[1] = invoke_id;
            payload[2] = SYN_BACNET_SERVICE_CONFIRMED_WRITE_PROPERTY;

            tx_frame->frame_type = SYN_BACNET_MSTP_FRAME_DATA_NOT_EXPECTING_REPLY;
            tx_frame->destination_mac = rx_frame->source_mac;
            tx_frame->source_mac = node->mac_address;
            tx_frame->data_len = 3;
            memcpy(tx_frame->payload, payload, 3);
            *has_tx = true;
            return SYN_OK;
        }
    }

    return SYN_OK;
}
```


