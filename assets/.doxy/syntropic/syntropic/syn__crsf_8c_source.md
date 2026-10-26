

# File syn\_crsf.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_crsf.c**](syn__crsf_8c.md)

[Go to the documentation of this file](syn__crsf_8c.md)


```C++

#include "syntropic/proto/syn_crsf.h"

#include <string.h>

uint8_t syn_crsf_calc_crc(const uint8_t *buf, size_t len)
{
    if (!buf) {
        return 0;
    }

    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= buf[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80U) {
                crc = (uint8_t)((crc << 1U) ^ 0xD5U);
            } else {
                crc <<= 1U;
            }
        }
    }
    return crc;
}

SYN_Status syn_crsf_init(SYN_CRSF_Parser *parser)
{
    if (!parser) {
        return SYN_INVALID_PARAM;
    }
    memset(parser, 0, sizeof(*parser));
    return SYN_OK;
}

static void parse_channels(const uint8_t *payload, SYN_CRSF_ChannelsFrame *channels)
{
    channels->channels[0] = (uint16_t)(((payload[0]) | (payload[1] << 8)) & 0x07FFU);
    channels->channels[1] = (uint16_t)(((payload[1] >> 3) | (payload[2] << 5)) & 0x07FFU);
    channels->channels[2] =
        (uint16_t)(((payload[2] >> 6) | (payload[3] << 2) | (payload[4] << 10)) & 0x07FFU);
    channels->channels[3] = (uint16_t)(((payload[4] >> 1) | (payload[5] << 7)) & 0x07FFU);
    channels->channels[4] = (uint16_t)(((payload[5] >> 4) | (payload[6] << 4)) & 0x07FFU);
    channels->channels[5] =
        (uint16_t)(((payload[6] >> 7) | (payload[7] << 1) | (payload[8] << 9)) & 0x07FFU);
    channels->channels[6] = (uint16_t)(((payload[8] >> 2) | (payload[9] << 6)) & 0x07FFU);
    channels->channels[7] = (uint16_t)(((payload[9] >> 5) | (payload[10] << 3)) & 0x07FFU);
    channels->channels[8] = (uint16_t)(((payload[11]) | (payload[12] << 8)) & 0x07FFU);
    channels->channels[9] = (uint16_t)(((payload[12] >> 3) | (payload[13] << 5)) & 0x07FFU);
    channels->channels[10] =
        (uint16_t)(((payload[13] >> 6) | (payload[14] << 2) | (payload[15] << 10)) & 0x07FFU);
    channels->channels[11] = (uint16_t)(((payload[15] >> 1) | (payload[16] << 7)) & 0x07FFU);
    channels->channels[12] = (uint16_t)(((payload[16] >> 4) | (payload[17] << 4)) & 0x07FFU);
    channels->channels[13] =
        (uint16_t)(((payload[17] >> 7) | (payload[18] << 1) | (payload[19] << 9)) & 0x07FFU);
    channels->channels[14] = (uint16_t)(((payload[19] >> 2) | (payload[20] << 6)) & 0x07FFU);
    channels->channels[15] = (uint16_t)(((payload[20] >> 5) | (payload[21] << 3)) & 0x07FFU);
}

static void parse_link_stats(const uint8_t *payload, SYN_CRSF_LinkStats *stats)
{
    stats->uplink_rssi1 = payload[0];
    stats->uplink_rssi2 = payload[1];
    stats->uplink_quality = payload[2];
    stats->uplink_snr = (int8_t)payload[3];
    stats->active_antenna = payload[4];
    stats->rf_mode = payload[5];
    stats->tx_power = payload[6];
    stats->downlink_rssi = payload[7];
    stats->downlink_quality = payload[8];
    stats->downlink_snr = (int8_t)payload[9];
}

SYN_Status syn_crsf_parse_byte(SYN_CRSF_Parser *parser, uint8_t byte, SYN_CRSF_FrameType *type_out)
{
    if (!parser) {
        return SYN_INVALID_PARAM;
    }

    if (parser->idx == 0) {
        if (byte != SYN_CRSF_ADDR_FC) {
            return SYN_ERROR;
        }
    } else if (parser->idx == 1) {
        parser->payload_len = byte;
        if (byte < 2U || byte > (SYN_CRSF_MAX_PACKET_LEN - 2U)) {
            parser->idx = 0;
            return SYN_ERROR;
        }
    }

    parser->buf[parser->idx++] = byte;

    if (parser->idx >= 2U && parser->idx == (size_t)(2U + parser->payload_len)) {
        parser->idx = 0;

        /* Verify CRC8 DVB-S2 over Type + Payload (bytes 2 .. 2+payload_len-2) */
        uint8_t expected_crc = syn_crsf_calc_crc(&parser->buf[2], parser->payload_len - 1U);
        uint8_t rx_crc = parser->buf[1U + parser->payload_len];

        if (expected_crc != rx_crc) {
            parser->crc_errors++;
            return SYN_ERROR;
        }

        parser->packets_received++;
        uint8_t type = parser->buf[2];

        if (type == SYN_CRSF_TYPE_RC_CHANNELS) {
            parse_channels(&parser->buf[3], &parser->last_channels);
        } else if (type == SYN_CRSF_TYPE_LINK_STATISTICS) {
            parse_link_stats(&parser->buf[3], &parser->last_link_stats);
        }

        if (type_out) {
            *type_out = (SYN_CRSF_FrameType)type;
        }

        return SYN_OK;
    }

    return SYN_BUSY;
}

uint16_t syn_crsf_raw_to_us(uint16_t raw_val)
{
    if (raw_val < 170U) {
        return 988U;
    }
    if (raw_val > 1908U) {
        return 2012U;
    }

    /*
     * Linear scaling:
     * raw range: [170, 1908] -> span = 1738
     * us range:  [988, 2012] -> span = 1024
     * us = 988 + ((raw_val - 170) * 1024) / 1738
     */
    uint32_t val = (uint32_t)(raw_val - 170U);
    uint32_t us = 988U + ((val * 1024U) / 1738U);
    return (uint16_t)us;
}
```


