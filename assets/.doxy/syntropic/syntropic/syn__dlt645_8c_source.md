

# File syn\_dlt645.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_dlt645.c**](syn__dlt645_8c.md)

[Go to the documentation of this file](syn__dlt645_8c.md)


```C++

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

/* ── Encoder ────────────────────────────────────────────────────────────── */

size_t syn_dlt645_encode(const SYN_DLT645_Frame *frame, uint8_t *out_buf, size_t out_capacity)
{
    if (frame == NULL || out_buf == NULL) {
        return 0;
    }

    size_t di_len = (frame->version == SYN_DLT645_VER_1997) ? 2 : 4;
    size_t data_len = di_len + frame->payload_len;
    size_t total_len = 10 + data_len + 2; /* 10 header + data_len + 1 CS + 1 EOF */

    if (out_capacity < total_len) {
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
    if (frame->version == SYN_DLT645_VER_1997) {
        out_buf[idx++] = (uint8_t)((frame->data_id & 0xFF) + SYN_DLT645_OFFSET);
        out_buf[idx++] = (uint8_t)(((frame->data_id >> 8) & 0xFF) + SYN_DLT645_OFFSET);
    } else {
        out_buf[idx++] = (uint8_t)((frame->data_id & 0xFF) + SYN_DLT645_OFFSET);
        out_buf[idx++] = (uint8_t)(((frame->data_id >> 8) & 0xFF) + SYN_DLT645_OFFSET);
        out_buf[idx++] = (uint8_t)(((frame->data_id >> 16) & 0xFF) + SYN_DLT645_OFFSET);
        out_buf[idx++] = (uint8_t)(((frame->data_id >> 24) & 0xFF) + SYN_DLT645_OFFSET);
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

    uint8_t data_len = buf[9];
    size_t di_len = (version == SYN_DLT645_VER_1997) ? 2 : 4;

    if (data_len < di_len) {
        return SYN_ERROR;
    }

    size_t total_frame_len = 10 + data_len + 2;
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
    out_frame->control = buf[8];

    /* Decode Data Identifier (subtract 0x33) */
    if (version == SYN_DLT645_VER_1997) {
        uint8_t di0 = (uint8_t)(buf[10] - SYN_DLT645_OFFSET);
        uint8_t di1 = (uint8_t)(buf[11] - SYN_DLT645_OFFSET);
        out_frame->data_id = (uint32_t)di0 | ((uint32_t)di1 << 8);
    } else {
        uint8_t di0 = (uint8_t)(buf[10] - SYN_DLT645_OFFSET);
        uint8_t di1 = (uint8_t)(buf[11] - SYN_DLT645_OFFSET);
        uint8_t di2 = (uint8_t)(buf[12] - SYN_DLT645_OFFSET);
        uint8_t di3 = (uint8_t)(buf[13] - SYN_DLT645_OFFSET);
        out_frame->data_id =
            (uint32_t)di0 | ((uint32_t)di1 << 8) | ((uint32_t)di2 << 16) | ((uint32_t)di3 << 24);
    }

    /* Decode Payload (subtract 0x33) */
    size_t payload_bytes = data_len - di_len;
    if (payload_bytes > sizeof(out_frame->payload)) {
        return SYN_ERROR;
    }
    out_frame->payload_len = (uint8_t)payload_bytes;
    size_t payload_offset = 10 + di_len;

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

void syn_dlt645_decoder_feed(SYN_DLT645_Decoder *dec, uint8_t rx_byte)
{
    if (dec == NULL) {
        return;
    }

    /* Ignore preambles if buffer is empty */
    if (dec->rx_len == 0 && rx_byte == SYN_DLT645_PREAMBLE) {
        return;
    }

    /* Must start with SOF (0x68) */
    if (dec->rx_len == 0 && rx_byte != SYN_DLT645_SOF) {
        return;
    }

    /* Prevent overflow */
    if (dec->rx_len >= sizeof(dec->rx_buf)) {
        dec->rx_len = 0;
        return;
    }

    dec->rx_buf[dec->rx_len++] = rx_byte;

    /* Check potential frame completion on EOF byte */
    if (rx_byte == SYN_DLT645_EOF && dec->rx_len >= 12) {
        SYN_DLT645_Frame frame;
        if (syn_dlt645_parse(dec->rx_buf, dec->rx_len, dec->version, &frame) == SYN_OK) {
            if (dec->cb != NULL) {
                dec->cb(&frame, dec->cb_ctx);
            }
            dec->rx_len = 0; /* Reset for next frame */
        }
    }
}
```


