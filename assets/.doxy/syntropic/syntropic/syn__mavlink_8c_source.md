

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

static size_t mavlink_extract_tail(const SYN_MAVLINK_Parser *parser, uint8_t crc0, uint8_t crc1,
                                   uint8_t out[266])
{
    out[0] = parser->payload_len;
    out[1] = parser->incompat_flags;
    out[2] = parser->compat_flags;
    out[3] = parser->seq;
    out[4] = parser->sys_id;
    out[5] = parser->comp_id;
    out[6] = (uint8_t)(parser->msg_id & 0xFFU);
    out[7] = (uint8_t)((parser->msg_id >> 8U) & 0xFFU);
    out[8] = (uint8_t)((parser->msg_id >> 16U) & 0xFFU);
    size_t plen = parser->payload_len;
    if (plen > 0U) {
        memcpy(&out[9], parser->payload, plen);
    }
    out[9U + plen] = crc0;
    out[10U + plen] = crc1;
    return 11U + plen;
}

static void mavlink_step_header(SYN_MAVLINK_Parser *parser, uint8_t byte)
{
    parser->crc = syn_mavlink_crc_accumulate(byte, parser->crc);
    switch (parser->state) {
    case STATE_LEN:
        parser->payload_len = byte;
        parser->state = STATE_INCOMPAT;
        break;
    case STATE_INCOMPAT:
        parser->incompat_flags = byte;
        parser->state = STATE_COMPAT;
        break;
    case STATE_COMPAT:
        parser->compat_flags = byte;
        parser->state = STATE_SEQ;
        break;
    case STATE_SEQ:
        parser->seq = byte;
        parser->state = STATE_SYS_ID;
        break;
    case STATE_SYS_ID:
        parser->sys_id = byte;
        parser->state = STATE_COMP_ID;
        break;
    case STATE_COMP_ID:
        parser->comp_id = byte;
        parser->state = STATE_MSG_ID0;
        break;
    case STATE_MSG_ID0:
        parser->msg_id = byte;
        parser->state = STATE_MSG_ID1;
        break;
    case STATE_MSG_ID1:
        parser->msg_id |= ((uint32_t)byte << 8U);
        parser->state = STATE_MSG_ID2;
        break;
    default:
        parser->msg_id |= ((uint32_t)byte << 16U);
        parser->payload_idx = 0U;
        parser->state = (parser->payload_len == 0U) ? STATE_CRC0 : STATE_PAYLOAD;
        break;
    }
}

static SYN_Status mavlink_finish_frame(SYN_MAVLINK_Parser *parser, uint8_t crc1,
                                       SYN_MAVLINK_Frame *frame, bool *crc_failed)
{
    parser->state = STATE_STX;
    uint16_t rx_crc = (uint16_t)parser->payload_idx | ((uint16_t)crc1 << 8U);
    uint8_t extra_crc = get_crc_extra(parser->msg_id);
    parser->crc = syn_mavlink_crc_accumulate(extra_crc, parser->crc);

    if (parser->crc != rx_crc) {
        parser->crc_errors++;
        *crc_failed = true;
        return SYN_ERROR;
    }

    *crc_failed = false;
    parser->packets_received++;
    parser->last_frame.payload_len = parser->payload_len;
    parser->last_frame.incompat_flags = parser->incompat_flags;
    parser->last_frame.compat_flags = parser->compat_flags;
    parser->last_frame.seq = parser->seq;
    parser->last_frame.sys_id = parser->sys_id;
    parser->last_frame.comp_id = parser->comp_id;
    parser->last_frame.msg_id = parser->msg_id;
    if (parser->payload_len > 0U) {
        memcpy(parser->last_frame.payload, parser->payload, parser->payload_len);
    }
    if (frame != NULL) {
        *frame = parser->last_frame;
    }
    return SYN_OK;
}

static SYN_Status mavlink_step_raw(SYN_MAVLINK_Parser *parser, uint8_t byte,
                                   SYN_MAVLINK_Frame *frame, uint8_t *crc0_out, bool *crc_failed)
{
    *crc_failed = false;
    if (parser->state == STATE_STX) {
        if (byte == SYN_MAVLINK_STX_V2) {
            parser->state = STATE_LEN;
            parser->crc = 0xFFFFU;
        }
        return SYN_BUSY;
    }
    if (parser->state <= STATE_MSG_ID2) {
        mavlink_step_header(parser, byte);
        return SYN_BUSY;
    }
    if (parser->state == STATE_PAYLOAD) {
        parser->payload[parser->payload_idx++] = byte;
        parser->crc = syn_mavlink_crc_accumulate(byte, parser->crc);
        if (parser->payload_idx >= parser->payload_len) {
            parser->state = STATE_CRC0;
        }
        return SYN_BUSY;
    }
    if (parser->state == STATE_CRC0) {
        parser->state = STATE_CRC1;
        parser->payload_idx = byte;
        return SYN_BUSY;
    }
    if (parser->state == STATE_CRC1) {
        *crc0_out = parser->payload_idx;
        return mavlink_finish_frame(parser, byte, frame, crc_failed);
    }
    parser->state = STATE_STX;
    return SYN_BUSY;
}

SYN_Status syn_mavlink_parse_byte(SYN_MAVLINK_Parser *parser, uint8_t byte,
                                  SYN_MAVLINK_Frame *frame)
{
    if (!parser) {
        return SYN_INVALID_PARAM;
    }

    uint8_t crc0 = 0U;
    bool crc_failed = false;
    SYN_Status st = mavlink_step_raw(parser, byte, frame, &crc0, &crc_failed);
    if (!crc_failed) {
        return st;
    }

    uint8_t pending[266];
    size_t head = 0U;
    size_t tail = mavlink_extract_tail(parser, crc0, byte, pending);
    SYN_Status result = SYN_ERROR;
    while (head < tail) {
        uint8_t inner_crc0 = 0U;
        bool inner_failed = false;
        SYN_Status inner_st =
            mavlink_step_raw(parser, pending[head++], frame, &inner_crc0, &inner_failed);
        if (inner_st == SYN_OK) {
            result = SYN_OK;
        } else if (inner_failed) {
            size_t rem_back = (size_t)parser->payload_len + 11U;
            head -= rem_back;
            (void)mavlink_extract_tail(parser, inner_crc0, pending[head + rem_back - 1U],
                                       &pending[head]);
        }
    }
    return result;
}

SYN_Status syn_mavlink_encode_msg(uint8_t sys_id, uint8_t comp_id, uint8_t seq, uint32_t msg_id,
                                  uint8_t crc_extra, const uint8_t *payload, uint8_t payload_len,
                                  uint8_t *buf_out, size_t *out_len)
{
    if ((buf_out == NULL) || (out_len == NULL) || ((payload == NULL) && (payload_len > 0U)) ||
        (msg_id > 0xFFFFFFU)) {
        return SYN_INVALID_PARAM;
    }

    uint16_t idx = 0U;
    buf_out[idx++] = SYN_MAVLINK_STX_V2;
    buf_out[idx++] = payload_len;
    buf_out[idx++] = 0U; /* Incompat flags */
    buf_out[idx++] = 0U; /* Compat flags */
    buf_out[idx++] = seq;
    buf_out[idx++] = sys_id;
    buf_out[idx++] = comp_id;
    buf_out[idx++] = (uint8_t)(msg_id & 0xFFU);
    buf_out[idx++] = (uint8_t)((msg_id >> 8U) & 0xFFU);
    buf_out[idx++] = (uint8_t)((msg_id >> 16U) & 0xFFU);

    uint16_t crc = 0xFFFFU;
    for (uint16_t i = 1U; i < idx; i++) {
        crc = syn_mavlink_crc_accumulate(buf_out[i], crc);
    }

    for (uint16_t i = 0U; i < (uint16_t)payload_len; i++) {
        buf_out[idx++] = payload[i];
        crc = syn_mavlink_crc_accumulate(payload[i], crc);
    }

    /* Accumulate CRC Extra */
    crc = syn_mavlink_crc_accumulate(crc_extra, crc);

    buf_out[idx++] = (uint8_t)(crc & 0xFFU);
    buf_out[idx++] = (uint8_t)((crc >> 8U) & 0xFFU);

    *out_len = (size_t)idx;
    return SYN_OK;
}
```


