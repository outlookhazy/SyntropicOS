

# File syn\_sbus.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_sbus.c**](syn__sbus_8c.md)

[Go to the documentation of this file](syn__sbus_8c.md)


```C++

#include "syntropic/proto/syn_sbus.h"

#include <string.h>

SYN_Status syn_sbus_init(SYN_SBUS_Parser *parser)
{
    if (!parser) {
        return SYN_INVALID_PARAM;
    }

    memset(parser, 0, sizeof(*parser));
    return SYN_OK;
}

SYN_Status syn_sbus_decode_buffer(const uint8_t buf[SYN_SBUS_FRAME_SIZE], SYN_SBUS_Frame *frame)
{
    if (!buf || !frame) {
        return SYN_INVALID_PARAM;
    }

    if (buf[0] != SYN_SBUS_HEADER) {
        return SYN_ERROR;
    }

    /* Unpack 16 11-bit channels from bytes 1..22 */
    frame->channels[0] = (uint16_t)(((buf[1]) | (buf[2] << 8)) & 0x07FFU);
    frame->channels[1] = (uint16_t)(((buf[2] >> 3) | (buf[3] << 5)) & 0x07FFU);
    frame->channels[2] = (uint16_t)(((buf[3] >> 6) | (buf[4] << 2) | (buf[5] << 10)) & 0x07FFU);
    frame->channels[3] = (uint16_t)(((buf[5] >> 1) | (buf[6] << 7)) & 0x07FFU);
    frame->channels[4] = (uint16_t)(((buf[6] >> 4) | (buf[7] << 4)) & 0x07FFU);
    frame->channels[5] = (uint16_t)(((buf[7] >> 7) | (buf[8] << 1) | (buf[9] << 9)) & 0x07FFU);
    frame->channels[6] = (uint16_t)(((buf[9] >> 2) | (buf[10] << 6)) & 0x07FFU);
    frame->channels[7] = (uint16_t)(((buf[10] >> 5) | (buf[11] << 3)) & 0x07FFU);
    frame->channels[8] = (uint16_t)(((buf[12]) | (buf[13] << 8)) & 0x07FFU);
    frame->channels[9] = (uint16_t)(((buf[13] >> 3) | (buf[14] << 5)) & 0x07FFU);
    frame->channels[10] = (uint16_t)(((buf[14] >> 6) | (buf[15] << 2) | (buf[16] << 10)) & 0x07FFU);
    frame->channels[11] = (uint16_t)(((buf[16] >> 1) | (buf[17] << 7)) & 0x07FFU);
    frame->channels[12] = (uint16_t)(((buf[17] >> 4) | (buf[18] << 4)) & 0x07FFU);
    frame->channels[13] = (uint16_t)(((buf[18] >> 7) | (buf[19] << 1) | (buf[20] << 9)) & 0x07FFU);
    frame->channels[14] = (uint16_t)(((buf[20] >> 2) | (buf[21] << 6)) & 0x07FFU);
    frame->channels[15] = (uint16_t)(((buf[21] >> 5) | (buf[22] << 3)) & 0x07FFU);

    /* Flags in byte 23 */
    uint8_t flags = buf[23];
    frame->ch17 = (flags & 0x01U) != 0;
    frame->ch18 = (flags & 0x02U) != 0;
    frame->frame_loss = (flags & 0x04U) != 0;
    frame->failsafe = (flags & 0x08U) != 0;

    return SYN_OK;
}

SYN_Status syn_sbus_parse_byte(SYN_SBUS_Parser *parser, uint8_t byte, SYN_SBUS_Frame *frame)
{
    if (!parser) {
        return SYN_INVALID_PARAM;
    }

    /* Start of new frame */
    if (parser->idx == 0) {
        if (byte != SYN_SBUS_HEADER) {
            return SYN_ERROR;
        }
    }

    parser->buf[parser->idx++] = byte;

    if (parser->idx >= SYN_SBUS_FRAME_SIZE) {
        parser->idx = 0;
        SYN_Status status = syn_sbus_decode_buffer(parser->buf, &parser->last_frame);
        if (status == SYN_OK) {
            parser->frames_received++;
            if (parser->last_frame.frame_loss) {
                parser->frame_loss_count++;
            }
            if (parser->last_frame.failsafe) {
                parser->failsafe_count++;
            }
            if (frame) {
                *frame = parser->last_frame;
            }
            return SYN_OK;
        }
        return status;
    }

    return SYN_BUSY;
}

uint16_t syn_sbus_raw_to_us(uint16_t raw_val)
{
    if (raw_val < 172U) {
        return 1000U;
    }
    if (raw_val > 1811U) {
        return 2000U;
    }

    /*
     * Linear interpolation:
     * raw range: [172, 1811] -> span = 1639
     * us range:  [1000, 2000] -> span = 1000
     * us = 1000 + ((raw_val - 172) * 1000) / 1639
     */
    uint32_t val = (uint32_t)(raw_val - 172U);
    uint32_t us = 1000U + ((val * 1000U) / 1639U);
    return (uint16_t)us;
}
```


