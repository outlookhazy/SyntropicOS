

# File syn\_mavlink.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_mavlink.c**](syn__mavlink_8c.md)

[Go to the documentation of this file](syn__mavlink_8c.md)


```C++

#include "syntropic/proto/syn_mavlink.h"

#include <string.h>

enum {
    STATE_STX = 0,
    STATE_LEN,
    STATE_INCOMPAT,
    STATE_COMPAT,
    STATE_SEQ,
    STATE_SYS_ID,
    STATE_COMP_ID,
    STATE_MSG_ID0,
    STATE_MSG_ID1,
    STATE_MSG_ID2,
    STATE_PAYLOAD,
    STATE_CRC0,
    STATE_CRC1
};

static uint8_t get_crc_extra(uint32_t msg_id)
{
    switch (msg_id) {
    case SYN_MAVLINK_MSG_HEARTBEAT:
        return 50;
    case SYN_MAVLINK_MSG_SYS_STATUS:
        return 124;
    case SYN_MAVLINK_MSG_ATTITUDE:
        return 39;
    case SYN_MAVLINK_MSG_GLOBAL_POSITION_INT:
        return 96;
    case SYN_MAVLINK_MSG_VFR_HUD:
        return 20;
    default:
        return 0;
    }
}

uint16_t syn_mavlink_crc_accumulate(uint8_t byte, uint16_t crc)
{
    uint8_t tmp = byte ^ (uint8_t)(crc & 0xFF);
    tmp ^= (tmp << 4);
    return (crc >> 8) ^ ((uint16_t)tmp << 8) ^ ((uint16_t)tmp << 3) ^ ((uint16_t)tmp >> 4);
}

SYN_Status syn_mavlink_init(SYN_MAVLINK_Parser *parser)
{
    if (!parser) {
        return SYN_INVALID_PARAM;
    }
    memset(parser, 0, sizeof(*parser));
    return SYN_OK;
}

SYN_Status syn_mavlink_parse_byte(SYN_MAVLINK_Parser *parser, uint8_t byte,
                                  SYN_MAVLINK_Frame *frame)
{
    if (!parser) {
        return SYN_INVALID_PARAM;
    }

    switch (parser->state) {
    case STATE_STX:
        if (byte == SYN_MAVLINK_STX_V2) {
            parser->state = STATE_LEN;
            parser->crc = 0xFFFF;
        }
        return SYN_BUSY;

    case STATE_LEN:
        parser->payload_len = byte;
        parser->crc = syn_mavlink_crc_accumulate(byte, parser->crc);
        parser->state = STATE_INCOMPAT;
        return SYN_BUSY;

    case STATE_INCOMPAT:
        parser->incompat_flags = byte;
        parser->crc = syn_mavlink_crc_accumulate(byte, parser->crc);
        parser->state = STATE_COMPAT;
        return SYN_BUSY;

    case STATE_COMPAT:
        parser->compat_flags = byte;
        parser->crc = syn_mavlink_crc_accumulate(byte, parser->crc);
        parser->state = STATE_SEQ;
        return SYN_BUSY;

    case STATE_SEQ:
        parser->seq = byte;
        parser->crc = syn_mavlink_crc_accumulate(byte, parser->crc);
        parser->state = STATE_SYS_ID;
        return SYN_BUSY;

    case STATE_SYS_ID:
        parser->sys_id = byte;
        parser->crc = syn_mavlink_crc_accumulate(byte, parser->crc);
        parser->state = STATE_COMP_ID;
        return SYN_BUSY;

    case STATE_COMP_ID:
        parser->comp_id = byte;
        parser->crc = syn_mavlink_crc_accumulate(byte, parser->crc);
        parser->state = STATE_MSG_ID0;
        return SYN_BUSY;

    case STATE_MSG_ID0:
        parser->msg_id = byte;
        parser->crc = syn_mavlink_crc_accumulate(byte, parser->crc);
        parser->state = STATE_MSG_ID1;
        return SYN_BUSY;

    case STATE_MSG_ID1:
        parser->msg_id |= ((uint32_t)byte << 8);
        parser->crc = syn_mavlink_crc_accumulate(byte, parser->crc);
        parser->state = STATE_MSG_ID2;
        return SYN_BUSY;

    case STATE_MSG_ID2:
        parser->msg_id |= ((uint32_t)byte << 16);
        parser->crc = syn_mavlink_crc_accumulate(byte, parser->crc);
        parser->payload_idx = 0;
        if (parser->payload_len == 0) {
            parser->state = STATE_CRC0;
        } else {
            parser->state = STATE_PAYLOAD;
        }
        return SYN_BUSY;

    case STATE_PAYLOAD:
        if (parser->payload_idx < SYN_MAVLINK_MAX_PAYLOAD_LEN) {
            parser->payload[parser->payload_idx++] = byte;
        }
        parser->crc = syn_mavlink_crc_accumulate(byte, parser->crc);
        if (parser->payload_idx >= parser->payload_len) {
            parser->state = STATE_CRC0;
        }
        return SYN_BUSY;

    case STATE_CRC0:
        /* Low byte of CRC */
        parser->state = STATE_CRC1;
        /* Store low byte for check */
        parser->payload_idx = byte;
        return SYN_BUSY;

    case STATE_CRC1: {
        parser->state = STATE_STX;
        uint16_t rx_crc = (uint16_t)parser->payload_idx | ((uint16_t)byte << 8);

        /* Accumulate CRC Extra byte */
        uint8_t extra_crc = get_crc_extra(parser->msg_id);
        parser->crc = syn_mavlink_crc_accumulate(extra_crc, parser->crc);

        if (parser->crc == rx_crc) {
            parser->packets_received++;
            parser->last_frame.payload_len = parser->payload_len;
            parser->last_frame.incompat_flags = parser->incompat_flags;
            parser->last_frame.compat_flags = parser->compat_flags;
            parser->last_frame.seq = parser->seq;
            parser->last_frame.sys_id = parser->sys_id;
            parser->last_frame.comp_id = parser->comp_id;
            parser->last_frame.msg_id = parser->msg_id;
            if (parser->payload_len > 0) {
                memcpy(parser->last_frame.payload, parser->payload, parser->payload_len);
            }
            if (frame) {
                *frame = parser->last_frame;
            }
            return SYN_OK;
        } else {
            parser->crc_errors++;
            return SYN_ERROR;
        }
    }

    default:
        parser->state = STATE_STX;
        return SYN_BUSY;
    }
}

SYN_Status syn_mavlink_encode_msg(uint8_t sys_id, uint8_t comp_id, uint8_t seq, uint32_t msg_id,
                                  uint8_t crc_extra, const uint8_t *payload, uint8_t payload_len,
                                  uint8_t *buf_out, size_t *out_len)
{
    if (!buf_out || !out_len) {
        return SYN_INVALID_PARAM;
    }

    uint8_t idx = 0;
    buf_out[idx++] = SYN_MAVLINK_STX_V2;
    buf_out[idx++] = payload_len;
    buf_out[idx++] = 0; /* Incompat flags */
    buf_out[idx++] = 0; /* Compat flags */
    buf_out[idx++] = seq;
    buf_out[idx++] = sys_id;
    buf_out[idx++] = comp_id;
    buf_out[idx++] = (uint8_t)(msg_id & 0xFF);
    buf_out[idx++] = (uint8_t)((msg_id >> 8) & 0xFF);
    buf_out[idx++] = (uint8_t)((msg_id >> 16) & 0xFF);

    uint16_t crc = 0xFFFF;
    for (uint8_t i = 1; i < idx; i++) {
        crc = syn_mavlink_crc_accumulate(buf_out[i], crc);
    }

    if (payload && payload_len > 0) {
        for (uint8_t i = 0; i < payload_len; i++) {
            buf_out[idx++] = payload[i];
            crc = syn_mavlink_crc_accumulate(payload[i], crc);
        }
    }

    /* Accumulate CRC Extra */
    crc = syn_mavlink_crc_accumulate(crc_extra, crc);

    buf_out[idx++] = (uint8_t)(crc & 0xFF);
    buf_out[idx++] = (uint8_t)((crc >> 8) & 0xFF);

    *out_len = idx;
    return SYN_OK;
}
```


