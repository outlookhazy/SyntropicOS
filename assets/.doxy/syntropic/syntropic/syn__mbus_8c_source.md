

# File syn\_mbus.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_mbus.c**](syn__mbus_8c.md)

[Go to the documentation of this file](syn__mbus_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_MBUS) || SYN_USE_MBUS

#include "../util/syn_assert.h"
#include "syn_mbus.h"

#include <string.h>

/* ── Decoder state machine internal states ───────────────────────────────── */

enum {
    STATE_IDLE = 0,
    STATE_SHORT_C,
    STATE_SHORT_A,
    STATE_SHORT_CHK,
    STATE_SHORT_STOP,

    STATE_LONG_L1,
    STATE_LONG_L2,
    STATE_LONG_SOF2,
    STATE_LONG_BODY,
};

/* ── Checksum Calculation ───────────────────────────────────────────────── */

uint8_t syn_mbus_calc_checksum(const uint8_t *data, size_t len)
{
    if (data == NULL || len == 0) {
        return 0;
    }

    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum = (uint8_t)(sum + data[i]);
    }
    return sum;
}

/* ── One-Shot Encoders ───────────────────────────────────────────────────── */

SYN_Status syn_mbus_encode_ack(uint8_t *buf, size_t cap, size_t *out_len)
{
    if (buf == NULL || out_len == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (cap < 1) {
        return SYN_ERROR;
    }

    buf[0] = SYN_MBUS_ACK_BYTE;
    *out_len = 1;
    return SYN_OK;
}

SYN_Status syn_mbus_encode_short(uint8_t c_field, uint8_t a_field, uint8_t *buf, size_t cap,
                                 size_t *out_len)
{
    if (buf == NULL || out_len == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (cap < 5) {
        return SYN_ERROR;
    }

    buf[0] = SYN_MBUS_START_SHORT;
    buf[1] = c_field;
    buf[2] = a_field;
    buf[3] = (uint8_t)(c_field + a_field);
    buf[4] = SYN_MBUS_STOP;

    *out_len = 5;
    return SYN_OK;
}

SYN_Status syn_mbus_encode_control(uint8_t c_field, uint8_t a_field, uint8_t ci_field, uint8_t *buf,
                                   size_t cap, size_t *out_len)
{
    return syn_mbus_encode_long(c_field, a_field, ci_field, NULL, 0, buf, cap, out_len);
}

SYN_Status syn_mbus_encode_long(uint8_t c_field, uint8_t a_field, uint8_t ci_field,
                                const uint8_t *payload, uint8_t payload_len, uint8_t *buf,
                                size_t cap, size_t *out_len)
{
    if (buf == NULL || out_len == NULL) {
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter
                                     fallback */
    }
    if (payload == NULL && payload_len > 0) {
        return SYN_INVALID_PARAM;
    }
    if (payload_len > SYN_MBUS_MAX_PAYLOAD) {
        return SYN_INVALID_PARAM;
    }

    size_t total_len = (size_t)payload_len + 9u;
    if (cap < total_len) {
        return SYN_ERROR;
    }

    uint8_t l_field = (uint8_t)(payload_len + 3u);

    buf[0] = SYN_MBUS_START_LONG;
    buf[1] = l_field;
    buf[2] = l_field;
    buf[3] = SYN_MBUS_START_LONG;
    buf[4] = c_field;
    buf[5] = a_field;
    buf[6] = ci_field;

    if (payload_len > 0) {
        memcpy(&buf[7], payload, payload_len);
    }

    /* Checksum is modulo-256 sum of C, A, CI, and Payload */
    uint8_t chk = syn_mbus_calc_checksum(&buf[4], (size_t)l_field);
    buf[7 + payload_len] = chk;
    buf[8 + payload_len] = SYN_MBUS_STOP;

    *out_len = total_len;
    return SYN_OK;
}

/* ── One-Shot Decoder ────────────────────────────────────────────────────── */

SYN_Status syn_mbus_decode_frame(const uint8_t *buf, size_t len, SYN_MBUS_Frame *frame)
{
    if (buf == NULL || frame == NULL || len == 0) {
        return SYN_INVALID_PARAM;
    }

    memset(frame, 0, sizeof(*frame));

    /* Case 1: Single character ACK (0xE5) */
    if (len == 1 && buf[0] == SYN_MBUS_ACK_BYTE) {
        frame->type = SYN_MBUS_FRAME_TYPE_SINGLE_ACK;
        frame->checksum_valid = true;
        return SYN_OK;
    }

    /* Case 2: Short frame (5 bytes: 0x10 | C | A | Checksum | 0x16) */
    if (buf[0] == SYN_MBUS_START_SHORT) {
        if (len < 5) {
            return SYN_INVALID_PARAM;
        }
        if (buf[4] != SYN_MBUS_STOP) {
            return SYN_ERROR;
        }

        frame->type = SYN_MBUS_FRAME_TYPE_SHORT;
        frame->c_field = buf[1];
        frame->a_field = buf[2];
        frame->checksum = buf[3];

        uint8_t calc_chk = (uint8_t)(buf[1] + buf[2]);
        frame->checksum_valid = (calc_chk == buf[3]);

        if (!frame->checksum_valid) {
            return SYN_ERROR;
        }
        return SYN_OK;
    }

    /* Case 3: Control / Long frame (0x68 | L | L | 0x68 | C | A | CI | Payload... | Checksum |
     * 0x16) */
    if (buf[0] == SYN_MBUS_START_LONG) {
        if (len < 9) {
            return SYN_INVALID_PARAM;
        }
        uint8_t l1 = buf[1];
        uint8_t l2 = buf[2];

        if (l1 != l2) {
            return SYN_ERROR;
        }
        if (buf[3] != SYN_MBUS_START_LONG) {
            return SYN_ERROR;
        }

        size_t expected_total = (size_t)l1 + 6u;
        if (len < expected_total) {
            return SYN_INVALID_PARAM;
        }
        if (buf[expected_total - 1] != SYN_MBUS_STOP) {
            return SYN_ERROR;
        }

        frame->c_field = buf[4];
        frame->a_field = buf[5];
        frame->ci_field = buf[6];
        frame->checksum = buf[expected_total - 2];

        if (l1 < 3) {
            return SYN_ERROR; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback
                               */
        }
        frame->payload_len = (uint8_t)(l1 - 3u);

        if (frame->payload_len == 0) {
            frame->type = SYN_MBUS_FRAME_TYPE_CONTROL;
        } else {
            frame->type = SYN_MBUS_FRAME_TYPE_LONG;
            memcpy(frame->payload, &buf[7], frame->payload_len);
        }

        uint8_t calc_chk = syn_mbus_calc_checksum(&buf[4], (size_t)l1);
        frame->checksum_valid = (calc_chk == frame->checksum);

        if (!frame->checksum_valid) {
            return SYN_ERROR;
        }
        return SYN_OK;
    }

    return SYN_ERROR;
}

/* ── Streaming State Machine Decoder ─────────────────────────────────────── */

void syn_mbus_decoder_init(SYN_MBUS_Decoder *dec, SYN_MBUS_FrameCallback callback, void *ctx)
{
    if (dec == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }

    memset(dec, 0, sizeof(*dec));
    dec->callback = callback;
    dec->ctx = ctx;
    dec->state = STATE_IDLE;
}

void syn_mbus_decoder_reset(SYN_MBUS_Decoder *dec)
{
    if (dec == NULL) {
        return;
    }

    dec->rx_idx = 0;
    dec->expected_len = 0;
    dec->state = STATE_IDLE;
}

void syn_mbus_decoder_feed(SYN_MBUS_Decoder *dec, uint8_t byte)
{
    if (dec == NULL) {
        return;
    }

    switch (dec->state) {
    case STATE_IDLE:
        dec->rx_idx = 0;
        dec->rx_buf[dec->rx_idx++] = byte;

        if (byte == SYN_MBUS_ACK_BYTE) {
            /* Single character ACK complete! */
            SYN_MBUS_Frame frame;
            if (syn_mbus_decode_frame(dec->rx_buf, dec->rx_idx, &frame) == SYN_OK) {
                if (dec->callback != NULL) {
                    dec->callback(&frame, dec->ctx);
                }
            }
            syn_mbus_decoder_reset(dec);
        } else if (byte == SYN_MBUS_START_SHORT) {
            dec->state = STATE_SHORT_C;
        } else if (byte == SYN_MBUS_START_LONG) {
            dec->state = STATE_LONG_L1;
        }
        break;

    /* ── Short Frame Parsing ────────────────────────────────────────────── */
    case STATE_SHORT_C:
        dec->rx_buf[dec->rx_idx++] = byte;
        dec->state = STATE_SHORT_A;
        break;

    case STATE_SHORT_A:
        dec->rx_buf[dec->rx_idx++] = byte;
        dec->state = STATE_SHORT_CHK;
        break;

    case STATE_SHORT_CHK:
        dec->rx_buf[dec->rx_idx++] = byte;
        dec->state = STATE_SHORT_STOP;
        break;

    case STATE_SHORT_STOP:
        dec->rx_buf[dec->rx_idx++] = byte;
        if (byte == SYN_MBUS_STOP) {
            SYN_MBUS_Frame frame;
            if (syn_mbus_decode_frame(dec->rx_buf, dec->rx_idx, &frame) == SYN_OK) {
                if (dec->callback != NULL) {
                    dec->callback(&frame, dec->ctx);
                }
            }
        }
        syn_mbus_decoder_reset(dec);
        break;

    /* ── Control / Long Frame Parsing ───────────────────────────────────── */
    case STATE_LONG_L1:
        dec->rx_buf[dec->rx_idx++] = byte;
        dec->state = STATE_LONG_L2;
        break;

    case STATE_LONG_L2:
        dec->rx_buf[dec->rx_idx++] = byte;
        if (byte != dec->rx_buf[1]) {
            /* Length byte mismatch -> reset */
            syn_mbus_decoder_reset(dec);
            break;
        }
        dec->expected_len = (size_t)byte + 6u;
        dec->state = STATE_LONG_SOF2;
        break;

    case STATE_LONG_SOF2:
        dec->rx_buf[dec->rx_idx++] = byte;
        if (byte != SYN_MBUS_START_LONG) {
            syn_mbus_decoder_reset(dec);
            break;
        }
        dec->state = STATE_LONG_BODY;
        break;

    case STATE_LONG_BODY:
        dec->rx_buf[dec->rx_idx++] = byte;
        if (dec->rx_idx >= dec->expected_len) {
            if (byte == SYN_MBUS_STOP) {
                SYN_MBUS_Frame frame;
                if (syn_mbus_decode_frame(dec->rx_buf, dec->rx_idx, &frame) == SYN_OK) {
                    if (dec->callback != NULL) {
                        dec->callback(&frame, dec->ctx);
                    }
                }
            }
            syn_mbus_decoder_reset(dec);
        }
        break;

    /* LCOV_EXCL_START: Default case reset for M-Bus decoder state machine */
    default:
        syn_mbus_decoder_reset(dec);
        break;
        /* LCOV_EXCL_STOP */
    }
}

#endif /* SYN_USE_MBUS */
```


