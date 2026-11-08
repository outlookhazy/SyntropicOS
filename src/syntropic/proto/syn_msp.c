/**
 * @file syn_msp.c
 * @brief Zero-Heap MSP v1 (MultiWii Serial Protocol v1) Encoder & Decoder Implementation.
 */

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

/**
 * @brief Check whether byte is a valid MSP v1 direction indicator ('<', '>', or '!').
 * @param byte Incoming byte.
 * @return true if valid MSP v1 direction indicator, false otherwise.
 */
static bool msp_is_dir_char(uint8_t byte)
{
    return (byte == SYN_MSP_REQ_CHAR) || (byte == SYN_MSP_RESP_CHAR) || (byte == SYN_MSP_ERR_CHAR);
}

/**
 * @brief Process MSP v1 3-byte header synchronization states ('$M<' / '$M>' / '$M!').
 * @param parser Pointer to MSP parser instance.
 * @param byte   Incoming serial byte.
 * @return true if byte was consumed in a header state, false otherwise.
 */
static bool msp_parse_header(SYN_MSP_Parser *parser, uint8_t byte)
{
    if (parser->state == STATE_HEADER_START) {
        if (byte == SYN_MSP_HEADER_CHAR) {
            parser->state = STATE_HEADER_M;
        }
        return true;
    }

    if (parser->state == STATE_HEADER_M) {
        if (byte == SYN_MSP_V1_CHAR) {
            parser->state = STATE_HEADER_DIR;
        } else if (byte != SYN_MSP_HEADER_CHAR) {
            parser->state = STATE_HEADER_START;
        }
        return true;
    }

    if (parser->state == STATE_HEADER_DIR) {
        if (msp_is_dir_char(byte)) {
            parser->dir_char = byte;
            parser->state = STATE_SIZE;
        } else if (byte == SYN_MSP_HEADER_CHAR) {
            parser->state = STATE_HEADER_M;
        } else {
            parser->state = STATE_HEADER_START;
        }
        return true;
    }

    return false;
}

/**
 * @brief Verify trailing XOR checksum and finalize decoded MSP frame.
 * @param parser Pointer to MSP parser instance.
 * @param byte   Received checksum byte.
 * @param frame  Optional output frame pointer.
 * @return SYN_OK on matching checksum, SYN_ERROR on mismatch.
 */
static SYN_Status msp_finish_checksum(SYN_MSP_Parser *parser, uint8_t byte, SYN_MSP_Frame *frame)
{
    parser->state = STATE_HEADER_START;
    if (parser->checksum != byte) {
        parser->checksum_errors++;
        return SYN_ERROR;
    }

    parser->frames_received++;
    parser->last_frame.dir_char = parser->dir_char;
    parser->last_frame.cmd = parser->cmd;
    parser->last_frame.payload_len = parser->payload_len;
    if (parser->payload_len > 0U) {
        memcpy(parser->last_frame.payload, parser->payload, parser->payload_len);
    }
    if (frame) {
        *frame = parser->last_frame;
    }
    return SYN_OK;
}

SYN_Status syn_msp_parse_byte(SYN_MSP_Parser *parser, uint8_t byte, SYN_MSP_Frame *frame)
{
    if (!parser) {
        return SYN_INVALID_PARAM;
    }

    if (msp_parse_header(parser, byte)) {
        return SYN_BUSY;
    }

    switch (parser->state) {
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
        return msp_finish_checksum(parser, byte, frame);

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
    if ((payload == NULL) && (len > 0U)) {
        return SYN_INVALID_PARAM;
    }

    size_t idx = 0;
    buf_out[idx++] = SYN_MSP_HEADER_CHAR;
    buf_out[idx++] = SYN_MSP_V1_CHAR;
    buf_out[idx++] = SYN_MSP_RESP_CHAR;
    buf_out[idx++] = len;
    buf_out[idx++] = cmd;

    uint8_t checksum = len ^ cmd;

    for (uint8_t i = 0U; i < len; i++) {
        buf_out[idx++] = payload[i];
        checksum ^= payload[i];
    }

    buf_out[idx++] = checksum;
    *out_len = idx;

    return SYN_OK;
}
