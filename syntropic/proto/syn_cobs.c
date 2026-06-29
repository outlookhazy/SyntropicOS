#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_COBS) || SYN_USE_COBS

/**
 * @file syn_cobs.c
 * @brief COBS packet framing implementation.
 */

#include "syn_cobs.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── One-shot encode ────────────────────────────────────────────────────── */

size_t syn_cobs_encode(const void *src, size_t src_len, void *dst)
{
    const uint8_t *s = (const uint8_t *)src;
    uint8_t *d = (uint8_t *)dst;

    if (src_len == 0) return 0;

    uint8_t *code_ptr = d++;   /* pointer to the code byte */
    uint8_t code = 1;

    for (size_t i = 0; i < src_len; i++) {
        if (s[i] == 0x00) {
            /* End of a run — write code byte */
            *code_ptr = code;
            code_ptr = d++;
            code = 1;
        } else {
            *d++ = s[i];
            code++;
            if (code == 0xFF) {
                /* Max run length — emit and start new run */
                *code_ptr = code;
                code_ptr = d++;
                code = 1;
            }
        }
    }
    *code_ptr = code;

    return (size_t)(d - (uint8_t *)dst);
}

/* ── One-shot decode ────────────────────────────────────────────────────── */

size_t syn_cobs_decode(const void *src, size_t src_len, void *dst)
{
    const uint8_t *s = (const uint8_t *)src;
    uint8_t *d = (uint8_t *)dst;
    size_t remaining = src_len;

    if (src_len == 0) return 0;

    while (remaining > 0) {
        uint8_t code = *s++;
        remaining--;

        if (code == 0) {
            /* Invalid: 0x00 inside encoded data */
            return 0;
        }

        uint8_t run = (uint8_t)(code - 1);
        if (run > remaining) {
            return 0; /* malformed */
        }

        memmove(d, s, run);
        d += run;
        s += run;
        remaining -= run;

        if (code < 0xFF && remaining > 0) {
            *d++ = 0x00; /* restore the zero */
        }
    }

    return (size_t)(d - (uint8_t *)dst);
}

/* ── Streaming decoder ──────────────────────────────────────────────────── */

void syn_cobs_decoder_init(SYN_COBS_Decoder *dec,
                            uint8_t *buf, size_t buf_size,
                            SYN_COBS_PacketCallback callback,
                            void *ctx)
{
    SYN_ASSERT(dec != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(buf_size > 0);

    dec->buf      = buf;
    dec->buf_size = buf_size;
    dec->idx      = 0;
    dec->callback = callback;
    dec->ctx      = ctx;
}

void syn_cobs_decoder_feed(SYN_COBS_Decoder *dec, uint8_t byte)
{
    SYN_ASSERT(dec != NULL);

    if (byte == 0x00) {
        /* Delimiter — decode accumulated data */
        if (dec->idx > 0 && dec->callback != NULL) {
            /* Decode in-place */
            size_t dec_len = syn_cobs_decode(dec->buf, dec->idx, dec->buf);
            if (dec_len > 0) {
                dec->callback(dec->buf, dec_len, dec->ctx);
            }
        }
        dec->idx = 0;
    } else {
        /* Accumulate */
        if (dec->idx < dec->buf_size) {
            dec->buf[dec->idx++] = byte;
        } else {
            /* Overflow — discard frame */
            dec->idx = 0;
        }
    }
}

void syn_cobs_decoder_reset(SYN_COBS_Decoder *dec)
{
    SYN_ASSERT(dec != NULL);
    dec->idx = 0;
}

#endif /* SYN_USE_COBS */
