

# File syn\_msp.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_msp.c**](syn__msp_8c.md)

[Go to the documentation of this file](syn__msp_8c.md)


```C++

#include "syntropic/proto/syn_msp.h"

#include <string.h>

enum {
    STATE_HEADER_START = 0,
    STATE_HEADER_M,
    STATE_HEADER_DIR,
    STATE_SIZE,
    STATE_CMD,
    STATE_PAYLOAD,
    STATE_CHECKSUM
};

SYN_Status syn_msp_init(SYN_MSP_Parser *parser)
{
    if (!parser) {
        return SYN_INVALID_PARAM;
    }
    memset(parser, 0, sizeof(*parser));
    return SYN_OK;
}

SYN_Status syn_msp_parse_byte(SYN_MSP_Parser *parser, uint8_t byte, SYN_MSP_Frame *frame)
{
    if (!parser) {
        return SYN_INVALID_PARAM;
    }

    switch (parser->state) {
    case STATE_HEADER_START:
        if (byte == SYN_MSP_HEADER_CHAR) {
            parser->state = STATE_HEADER_M;
        }
        return SYN_BUSY;

    case STATE_HEADER_M:
        if (byte == SYN_MSP_V1_CHAR) {
            parser->state = STATE_HEADER_DIR;
        } else {
            parser->state = STATE_HEADER_START;
        }
        return SYN_BUSY;

    case STATE_HEADER_DIR:
        if (byte == SYN_MSP_REQ_CHAR || byte == SYN_MSP_RESP_CHAR || byte == SYN_MSP_ERR_CHAR) {
            parser->dir_char = byte;
            parser->state = STATE_SIZE;
        } else {
            parser->state = STATE_HEADER_START;
        }
        return SYN_BUSY;

    case STATE_SIZE:
        parser->payload_len = byte;
        parser->checksum = byte;
        parser->payload_idx = 0;
        parser->state = STATE_CMD;
        return SYN_BUSY;

    case STATE_CMD:
        parser->cmd = byte;
        parser->checksum ^= byte;
        if (parser->payload_len == 0) {
            parser->state = STATE_CHECKSUM;
        } else if (parser->payload_len > SYN_MSP_MAX_PAYLOAD) {
            parser->state = STATE_HEADER_START;
            return SYN_ERROR;
        } else {
            parser->state = STATE_PAYLOAD;
        }
        return SYN_BUSY;

    case STATE_PAYLOAD:
        if (parser->payload_idx < SYN_MSP_MAX_PAYLOAD) {
            parser->payload[parser->payload_idx++] = byte;
        }
        parser->checksum ^= byte;
        if (parser->payload_idx >= parser->payload_len) {
            parser->state = STATE_CHECKSUM;
        }
        return SYN_BUSY;

    case STATE_CHECKSUM:
        parser->state = STATE_HEADER_START;
        if (parser->checksum == byte) {
            parser->frames_received++;
            parser->last_frame.dir_char = parser->dir_char;
            parser->last_frame.cmd = parser->cmd;
            parser->last_frame.payload_len = parser->payload_len;
            if (parser->payload_len > 0) {
                memcpy(parser->last_frame.payload, parser->payload, parser->payload_len);
            }
            if (frame) {
                *frame = parser->last_frame;
            }
            return SYN_OK;
        } else {
            parser->checksum_errors++;
            return SYN_ERROR;
        }

    default:
        parser->state = STATE_HEADER_START;
        return SYN_BUSY;
    }
}

SYN_Status syn_msp_encode_response(uint8_t cmd, const uint8_t *payload, uint8_t len,
                                   uint8_t *buf_out, size_t buf_size, size_t *out_len)
{
    if (!buf_out || !out_len) {
        return SYN_INVALID_PARAM;
    }

    /* Minimum required: 5 header bytes + len payload bytes + 1 checksum */
    size_t required = (size_t)5 + len + 1;
    if (buf_size < required) {
        return SYN_INVALID_PARAM;
    }

    size_t idx = 0;
    buf_out[idx++] = SYN_MSP_HEADER_CHAR;
    buf_out[idx++] = SYN_MSP_V1_CHAR;
    buf_out[idx++] = SYN_MSP_RESP_CHAR;
    buf_out[idx++] = len;
    buf_out[idx++] = cmd;

    uint8_t checksum = len ^ cmd;

    if (payload && len > 0) {
        for (uint8_t i = 0; i < len; i++) {
            buf_out[idx++] = payload[i];
            checksum ^= payload[i];
        }
    }

    buf_out[idx++] = checksum;
    *out_len = idx;

    return SYN_OK;
}
```


