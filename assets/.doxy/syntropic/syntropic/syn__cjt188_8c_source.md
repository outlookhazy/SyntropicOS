

# File syn\_cjt188.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cjt188.c**](syn__cjt188_8c.md)

[Go to the documentation of this file](syn__cjt188_8c.md)


```C++

#include "syn_cjt188.h"

#include "../util/syn_pack.h"

#include <string.h>

uint8_t syn_cjt188_checksum(const uint8_t *buf, size_t len)
{
    if (!buf || len == 0) {
        return 0;
    }
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    return (uint8_t)(sum & 0xFFU);
}

size_t syn_cjt188_encode_read_req(uint8_t *out_buf, size_t buf_size, uint8_t meter_type,
                                  const uint8_t meter_id[5], const uint8_t vendor_id[2],
                                  uint16_t data_id, uint8_t seq)
{
    /*
     * Frame layout with 4-byte preamble:
     * [0..3]   FE FE FE FE
     * [4]      68 (Start)
     * [5]      Meter Type (T)
     * [6..10]  Meter ID (5 bytes)
     * [11..12] Vendor ID (2 bytes)
     * [13]     Control Code C (0x01)
     * [14]     Length L (0x03)
     * [15..16] Data ID DI (2 bytes: e.g. 0x1F, 0x90)
     * [17]     SER Sequence Number
     * [18]     CS Checksum (sum from byte 4 to 17)
     * [19]     16 (End)
     * Total length: 20 bytes
     */
    if (!out_buf || !meter_id || !vendor_id || buf_size < 20) {
        return 0;
    }

    /* Preamble */
    out_buf[0] = SYN_CJT188_PREAMBLE_BYTE;
    out_buf[1] = SYN_CJT188_PREAMBLE_BYTE;
    out_buf[2] = SYN_CJT188_PREAMBLE_BYTE;
    out_buf[3] = SYN_CJT188_PREAMBLE_BYTE;

    /* Start of CJ/T 188 frame */
    size_t start_idx = 4;
    out_buf[start_idx] = SYN_CJT188_START_BYTE;
    out_buf[start_idx + 1] = meter_type;

    memcpy(&out_buf[start_idx + 2], meter_id, 5);
    memcpy(&out_buf[start_idx + 7], vendor_id, 2);

    out_buf[start_idx + 9] = SYN_CJT188_CTRL_READ_DATA;
    out_buf[start_idx + 10] = 0x03; /* Fixed length: Data ID (2) + SER (1) */

    /* Data ID in little-endian byte order as per CJ/T 188 standard */
    out_buf[start_idx + 11] = (uint8_t)(data_id & 0xFFU);
    out_buf[start_idx + 12] = (uint8_t)((data_id >> 8) & 0xFFU);
    out_buf[start_idx + 13] = seq;

    /* Compute CS over [start_idx .. start_idx + 13] (14 bytes total) */
    uint8_t cs = syn_cjt188_checksum(&out_buf[start_idx], 14);
    out_buf[start_idx + 14] = cs;
    out_buf[start_idx + 15] = SYN_CJT188_END_BYTE;

    return 20;
}

size_t syn_cjt188_encode_valve_ctrl(uint8_t *out_buf, size_t buf_size, uint8_t meter_type,
                                    const uint8_t meter_id[5], const uint8_t vendor_id[2],
                                    bool open_valve, uint8_t seq)
{
    /*
     * Frame layout with 4-byte preamble:
     * [0..3]   FE FE FE FE
     * [4]      68 (Start)
     * [5]      Meter Type (T)
     * [6..10]  Meter ID (5 bytes)
     * [11..12] Vendor ID (2 bytes)
     * [13]     Control Code C (0x04)
     * [14]     Length L (0x04: DI(2) + SER(1) + Cmd(1))
     * [15..16] Data ID (0x17, 0xA0)
     * [17]     SER Sequence Number
     * [18]     Command byte (0x55 Open / 0x99 Close)
     * [19]     CS Checksum
     * [20]     16 (End)
     * Total length: 21 bytes
     */
    if (!out_buf || !meter_id || !vendor_id || buf_size < 21) {
        return 0;
    }

    out_buf[0] = SYN_CJT188_PREAMBLE_BYTE;
    out_buf[1] = SYN_CJT188_PREAMBLE_BYTE;
    out_buf[2] = SYN_CJT188_PREAMBLE_BYTE;
    out_buf[3] = SYN_CJT188_PREAMBLE_BYTE;

    size_t start_idx = 4;
    out_buf[start_idx] = SYN_CJT188_START_BYTE;
    out_buf[start_idx + 1] = meter_type;

    memcpy(&out_buf[start_idx + 2], meter_id, 5);
    memcpy(&out_buf[start_idx + 7], vendor_id, 2);

    out_buf[start_idx + 9] = SYN_CJT188_CTRL_WRITE_DATA;
    out_buf[start_idx + 10] = 0x04; /* DI (2) + SER (1) + Cmd (1) */

    out_buf[start_idx + 11] = (uint8_t)(SYN_CJT188_DI_VALVE_CONTROL & 0xFFU);
    out_buf[start_idx + 12] = (uint8_t)((SYN_CJT188_DI_VALVE_CONTROL >> 8) & 0xFFU);
    out_buf[start_idx + 13] = seq;
    out_buf[start_idx + 14] = open_valve ? SYN_CJT188_VALVE_OPEN : SYN_CJT188_VALVE_CLOSE;

    uint8_t cs = syn_cjt188_checksum(&out_buf[start_idx], 15);
    out_buf[start_idx + 15] = cs;
    out_buf[start_idx + 16] = SYN_CJT188_END_BYTE;

    return 21;
}

bool syn_cjt188_parse_frame(const uint8_t *buf, size_t len, SYN_CJT188_Frame *out_frame)
{
    if (!buf || !out_frame || len < SYN_CJT188_MIN_FRAME_SIZE) {
        return false;
    }

    /* Skip leading 0xFE preamble bytes if present */
    size_t offset = 0;
    while (offset < len && buf[offset] == SYN_CJT188_PREAMBLE_BYTE) {
        offset++;
    }

    if ((len - offset) < SYN_CJT188_MIN_FRAME_SIZE) {
        return false;
    }

    /* Verify Start Byte 0x68 */
    if (buf[offset] != SYN_CJT188_START_BYTE) {
        return false;
    }

    size_t data_len = buf[offset + 10];
    size_t expected_total = 11 + data_len + 2; /* 11 header bytes + L + CS + 16 */

    if ((len - offset) < expected_total) {
        return false;
    }

    /* Verify End Byte 0x16 */
    if (buf[offset + expected_total - 1] != SYN_CJT188_END_BYTE) {
        return false;
    }

    /* Verify Checksum */
    uint8_t expected_cs = buf[offset + expected_total - 2];
    uint8_t actual_cs = syn_cjt188_checksum(&buf[offset], 11 + data_len);
    if (expected_cs != actual_cs) {
        return false;
    }

    /* Populate frame structure */
    out_frame->meter_type = buf[offset + 1];
    memcpy(out_frame->meter_id, &buf[offset + 2], 5);
    memcpy(out_frame->vendor_id, &buf[offset + 7], 2);
    out_frame->ctrl = buf[offset + 9];
    out_frame->len = (uint8_t)data_len;

    if (data_len >= 3) {
        out_frame->data_id = (uint16_t)buf[offset + 11] | ((uint16_t)buf[offset + 12] << 8);
        out_frame->seq = buf[offset + 13];
        out_frame->payload = &buf[offset + 14];
        out_frame->payload_len = data_len - 3;
    } else {
        out_frame->data_id = 0;
        out_frame->seq = 0;
        out_frame->payload = NULL;
        out_frame->payload_len = 0;
    }

    return true;
}

void syn_cjt188_decoder_init(SYN_CJT188_Decoder *decoder)
{
    if (decoder) {
        memset(decoder, 0, sizeof(SYN_CJT188_Decoder));
    }
}

bool syn_cjt188_decoder_feed(SYN_CJT188_Decoder *decoder, uint8_t byte, SYN_CJT188_Frame *out_frame)
{
    if (!decoder || !out_frame) {
        return false;
    }

    /* Handle preamble bytes */
    if (!decoder->in_frame) {
        if (byte == SYN_CJT188_PREAMBLE_BYTE) {
            return false; /* Ignore preamble */
        }
        if (byte == SYN_CJT188_START_BYTE) {
            decoder->in_frame = true;
            decoder->index = 0;
            decoder->buf[decoder->index++] = byte;
            decoder->expected_len = 0;
            return false;
        }
        return false;
    }

    /* Collect frame bytes */
    if (decoder->index < SYN_CJT188_MAX_FRAME_SIZE) {
        decoder->buf[decoder->index++] = byte;
    } else {
        /* Buffer overflow, reset decoder */
        syn_cjt188_decoder_init(decoder);
        return false;
    }

    /* Read length byte once offset 10 is reached */
    if (decoder->index == 11) {
        uint8_t data_len = decoder->buf[10];
        decoder->expected_len = 11 + data_len + 2; /* 11 header + L + CS + 0x16 */
        if (decoder->expected_len > SYN_CJT188_MAX_FRAME_SIZE) {
            syn_cjt188_decoder_init(decoder);
            return false;
        }
    }

    /* When expected total length reached, attempt parsing */
    if (decoder->expected_len > 0 && decoder->index >= decoder->expected_len) {
        bool ok = syn_cjt188_parse_frame(decoder->buf, decoder->index, out_frame);
        syn_cjt188_decoder_init(decoder);
        return ok;
    }

    return false;
}
```


