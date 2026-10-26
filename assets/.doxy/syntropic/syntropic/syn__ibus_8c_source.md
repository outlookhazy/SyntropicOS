

# File syn\_ibus.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ibus.c**](syn__ibus_8c.md)

[Go to the documentation of this file](syn__ibus_8c.md)


```C++

#include "syntropic/proto/syn_ibus.h"

#include <string.h>

SYN_Status syn_ibus_init(SYN_IBUS_Parser *parser)
{
    if (!parser) {
        return SYN_INVALID_PARAM;
    }
    memset(parser, 0, sizeof(*parser));
    return SYN_OK;
}

uint16_t syn_ibus_calc_checksum(const uint8_t buf[30])
{
    if (!buf) {
        return 0;
    }
    uint16_t sum = 0xFFFFU;
    for (size_t i = 0; i < 30; i++) {
        sum -= buf[i];
    }
    return sum;
}

SYN_Status syn_ibus_parse_byte(SYN_IBUS_Parser *parser, uint8_t byte, SYN_IBUS_Frame *frame)
{
    if (!parser) {
        return SYN_INVALID_PARAM;
    }

    if (parser->idx == 0) {
        if (byte != SYN_IBUS_HEADER1) {
            return SYN_ERROR;
        }
    } else if (parser->idx == 1) {
        if (byte != SYN_IBUS_HEADER2) {
            parser->idx = 0;
            return SYN_ERROR;
        }
    }

    parser->buf[parser->idx++] = byte;

    if (parser->idx >= SYN_IBUS_FRAME_SIZE) {
        parser->idx = 0;

        uint16_t expected_sum = syn_ibus_calc_checksum(parser->buf);
        uint16_t rx_sum = (uint16_t)(parser->buf[30] | (parser->buf[31] << 8));

        if (expected_sum != rx_sum) {
            parser->checksum_errors++;
            return SYN_ERROR;
        }

        /* Unpack 14 16-bit little-endian channels */
        for (size_t i = 0; i < SYN_IBUS_NUM_CHANNELS; i++) {
            uint16_t ch = (uint16_t)(parser->buf[2 + (i * 2)] | (parser->buf[3 + (i * 2)] << 8));
            parser->last_frame.channels[i] = ch;
        }

        parser->frames_received++;
        if (frame) {
            *frame = parser->last_frame;
        }

        return SYN_OK;
    }

    return SYN_BUSY;
}
```


