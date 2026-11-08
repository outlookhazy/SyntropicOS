/**
 * @file syn_dlt645.c
 * @brief DL/T 645 Electricity Meter Communication Protocol Implementation.
 */

#include "syn_dlt645.h"

#include "../util/syn_assert.h"

#include <string.h>

/* ── Checksum Calculation ────────────────────────────────────────────────── */

uint8_t syn_dlt645_calc_checksum(const uint8_t *buf, size_t len)
{
    if (buf == NULL || len == 0) {
        return 0;
    }

    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    return (uint8_t)(sum & 0xFF);
}

/**
 * @brief Determine Data Identifier (DI) byte length for a DL/T 645 control code and version.
 * @param control DL/T 645 control byte.
 * @param version Protocol version (1997 or 2007).
 * @return DI field length in bytes (0, 2, or 4).
 */
static size_t dlt645_di_len(uint8_t control, SYN_DLT645_Ver version)
{
    if ((control & 0x40U) != 0U) {
        return 0U;
    }
    uint8_t base = (uint8_t)(control & 0x7FU);
    if ((control == 0x08U) || (base == 0x13U) || (base == 0x15U) || (base == 0x16U) ||
        (base == 0x17U)) {
        return 0U;
    }
    return (version == SYN_DLT645_VER_1997) ? 2U : 4U;
}

/**
 * @brief Decode Data Identifier (DI) bytes with -0x33 offset removal.
 * @param p      Pointer to start of encoded DI bytes.
 * @param di_len DI length in bytes (0, 2, or 4).
 * @return Decoded 32-bit Data Identifier (0 if di_len == 0).
 */
static uint32_t dlt645_decode_di(const uint8_t *p, size_t di_len)
{
    if (di_len == 2U) {
        uint8_t di0 = (uint8_t)(p[0] - SYN_DLT645_OFFSET);
        uint8_t di1 = (uint8_t)(p[1] - SYN_DLT645_OFFSET);
        return (uint32_t)di0 | ((uint32_t)di1 << 8U);
    }
    if (di_len == 4U) {
        uint8_t di0 = (uint8_t)(p[0] - SYN_DLT645_OFFSET);
        uint8_t di1 = (uint8_t)(p[1] - SYN_DLT645_OFFSET);
        uint8_t di2 = (uint8_t)(p[2] - SYN_DLT645_OFFSET);
        uint8_t di3 = (uint8_t)(p[3] - SYN_DLT645_OFFSET);
        return (uint32_t)di0 | ((uint32_t)di1 << 8U) | ((uint32_t)di2 << 16U) |
               ((uint32_t)di3 << 24U);
    }
    return 0U;
}

/* ── Encoder ────────────────────────────────────────────────────────────── */

size_t syn_dlt645_encode(const SYN_DLT645_Frame *frame, uint8_t *out_buf, size_t out_capacity)
{
    if (frame == NULL || out_buf == NULL) {
        return 0;
    }

    size_t di_len = dlt645_di_len(frame->control, frame->version);
    size_t data_len = di_len + frame->payload_len;
    size_t total_len = 10 + data_len + 2; /* 10 header + data_len + 1 CS + 1 EOF */

    if (out_capacity < total_len || data_len > 255 || frame->payload_len > sizeof(frame->payload)) {
        return 0;
    }

    size_t idx = 0;
    out_buf[idx++] = SYN_DLT645_SOF;

    /* 6-byte address */
    memcpy(&out_buf[idx], frame->address, SYN_DLT645_ADDR_LEN);
    idx += SYN_DLT645_ADDR_LEN;

    out_buf[idx++] = SYN_DLT645_SOF;
    out_buf[idx++] = frame->control;
    out_buf[idx++] = (uint8_t)data_len;

    /* Encode Data Identifier (DI) with +0x33 offset (little-endian) */
    if (di_len == 2U) {
        out_buf[idx++] = (uint8_t)((frame->data_id & 0xFFU) + SYN_DLT645_OFFSET);
        out_buf[idx++] = (uint8_t)(((frame->data_id >> 8U) & 0xFFU) + SYN_DLT645_OFFSET);
    } else if (di_len == 4U) {
        out_buf[idx++] = (uint8_t)((frame->data_id & 0xFFU) + SYN_DLT645_OFFSET);
        out_buf[idx++] = (uint8_t)(((frame->data_id >> 8U) & 0xFFU) + SYN_DLT645_OFFSET);
        out_buf[idx++] = (uint8_t)(((frame->data_id >> 16U) & 0xFFU) + SYN_DLT645_OFFSET);
        out_buf[idx++] = (uint8_t)(((frame->data_id >> 24U) & 0xFFU) + SYN_DLT645_OFFSET);
    }

    /* Encode Payload with +0x33 offset */
    for (uint8_t i = 0; i < frame->payload_len; i++) {
        out_buf[idx++] = (uint8_t)(frame->payload[i] + SYN_DLT645_OFFSET);
    }

    /* Modulo-256 Checksum over bytes 0 to idx-1 */
    out_buf[idx] = syn_dlt645_calc_checksum(out_buf, idx);
    idx++;

    out_buf[idx++] = SYN_DLT645_EOF;

    return idx;
}

/* ── Parser ─────────────────────────────────────────────────────────────── */

SYN_Status syn_dlt645_parse(const uint8_t *in_buf, size_t len, SYN_DLT645_Ver version,
                            SYN_DLT645_Frame *out_frame)
{
    if (in_buf == NULL || out_frame == NULL || len < 12) {
        return SYN_INVALID_PARAM;
    }

    /* Strip leading preambles (0xFE) */
    size_t start = 0;
    while (start < len && in_buf[start] == SYN_DLT645_PREAMBLE) {
        start++;
    }

    size_t rem = len - start;
    if (rem < 12) {
        return SYN_INVALID_PARAM;
    }

    const uint8_t *buf = &in_buf[start];

    /* Validate SOF1 and SOF2 */
    if (buf[0] != SYN_DLT645_SOF || buf[7] != SYN_DLT645_SOF) {
        return SYN_ERROR;
    }

    uint8_t control = buf[8];
    uint8_t data_len = buf[9];
    size_t di_len = dlt645_di_len(control, version);

    if ((data_len < di_len) || (((size_t)data_len - di_len) > sizeof(out_frame->payload))) {
        return SYN_ERROR;
    }

    size_t total_frame_len = 10U + (size_t)data_len + 2U;
    if (rem < total_frame_len) {
        return SYN_INVALID_PARAM;
    }

    /* Validate EOF */
    if (buf[total_frame_len - 1] != SYN_DLT645_EOF) {
        return SYN_ERROR;
    }

    /* Validate Checksum */
    uint8_t calc_cs = syn_dlt645_calc_checksum(buf, total_frame_len - 2);
    uint8_t frame_cs = buf[total_frame_len - 2];
    if (calc_cs != frame_cs) {
        return SYN_ERROR;
    }

    /* Populate output frame struct */
    memset(out_frame, 0, sizeof(*out_frame));
    out_frame->version = version;
    memcpy(out_frame->address, &buf[1], SYN_DLT645_ADDR_LEN);
    out_frame->control = control;
    out_frame->data_id = dlt645_decode_di(&buf[10], di_len);

    /* Decode Payload (subtract 0x33) */
    size_t payload_bytes = (size_t)data_len - di_len;
    out_frame->payload_len = (uint8_t)payload_bytes;
    size_t payload_offset = 10U + di_len;

    for (uint8_t i = 0; i < out_frame->payload_len; i++) {
        out_frame->payload[i] = (uint8_t)(buf[payload_offset + i] - SYN_DLT645_OFFSET);
    }

    return SYN_OK;
}

/* ── Streaming Decoder ──────────────────────────────────────────────────── */

void syn_dlt645_decoder_init(SYN_DLT645_Decoder *dec, SYN_DLT645_Ver version,
                             SYN_DLT645_FrameCallback cb, void *ctx)
{
    SYN_ASSERT(dec != NULL);

    memset(dec, 0, sizeof(*dec));
    dec->version = version;
    dec->cb = cb;
    dec->cb_ctx = ctx;
}

/**
 * @brief Discard leading bytes up to the next 0x68 SOF marker for stream resynchronization.
 * @param dec Streaming DL/T 645 decoder instance.
 */
static void dlt645_shift_to_next_sof(SYN_DLT645_Decoder *dec)
{
    for (size_t i = 1U; i < dec->rx_len; i++) {
        if (dec->rx_buf[i] == SYN_DLT645_SOF) {
            dec->rx_len -= i;
            memmove(dec->rx_buf, &dec->rx_buf[i], dec->rx_len);
            return;
        }
    }
    dec->rx_len = 0U;
}

void syn_dlt645_decoder_feed(SYN_DLT645_Decoder *dec, uint8_t rx_byte)
{
    if (dec == NULL) {
        return;
    }

    /* Ignore preambles if buffer is empty */
    if (dec->rx_len == 0U && rx_byte == SYN_DLT645_PREAMBLE) {
        return;
    }

    /* Must start with SOF (0x68) */
    if (dec->rx_len == 0U && rx_byte != SYN_DLT645_SOF) {
        return;
    }

    /* Prevent overflow */
    if (dec->rx_len >= sizeof(dec->rx_buf)) {
        dec->rx_len = 0U;
        return;
    }

    dec->rx_buf[dec->rx_len++] = rx_byte;

    /* Check potential frame completion or resync on EOF byte */
    if (rx_byte == SYN_DLT645_EOF) {
        while (dec->rx_len >= 12U) {
            SYN_DLT645_Frame frame;
            SYN_Status st = syn_dlt645_parse(dec->rx_buf, dec->rx_len, dec->version, &frame);
            if (st == SYN_OK) {
                if (dec->cb != NULL) {
                    dec->cb(&frame, dec->cb_ctx);
                }
                dec->rx_len = 0U;
                break;
            }
            if (st != SYN_ERROR) {
                break;
            }
            dlt645_shift_to_next_sof(dec);
        }
    }
}
