

# File syn\_blackbox.c

[**File List**](files.md) **>** [**log**](dir_1a216ccd2a39b426a7ff78fb20f852eb.md) **>** [**syn\_blackbox.c**](syn__blackbox_8c.md)

[Go to the documentation of this file](syn__blackbox_8c.md)


```C++

#include "syntropic/log/syn_blackbox.h"

#include <string.h>

size_t syn_blackbox_encode_varint(int32_t val, uint8_t *buf_out)
{
    if (!buf_out) {
        return 0; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }

    /* ZigZag encoding: maps signed integers to unsigned integers */
    uint32_t uval = ((uint32_t)val << 1) ^ (uint32_t)(val >> 31);
    size_t len = 0;

    while (uval >= 0x80) {
        buf_out[len++] = (uint8_t)((uval & 0x7F) | 0x80);
        uval >>= 7;
    }
    buf_out[len++] = (uint8_t)(uval & 0x7F);

    return len;
}

SYN_Status syn_blackbox_init(SYN_Blackbox *bb)
{
    if (!bb) {
        return SYN_INVALID_PARAM;
    }
    memset(bb, 0, sizeof(*bb));
    return SYN_OK;
}

SYN_Status syn_blackbox_encode_intra(SYN_Blackbox *bb, const SYN_Blackbox_Record *record,
                                     uint8_t *buf_out, size_t *out_len)
{
    if (!bb || !record || !buf_out || !out_len) {
        return SYN_INVALID_PARAM;
    }

    size_t idx = 0;
    buf_out[idx++] = SYN_BLACKBOX_FRAME_INTRA;

    idx += syn_blackbox_encode_varint((int32_t)record->iteration, &buf_out[idx]);
    idx += syn_blackbox_encode_varint((int32_t)record->time_us, &buf_out[idx]);

    for (int i = 0; i < 3; i++) {
        idx += syn_blackbox_encode_varint(record->gyro[i], &buf_out[idx]);
    }
    for (int i = 0; i < 3; i++) {
        idx += syn_blackbox_encode_varint(record->accel[i], &buf_out[idx]);
    }
    for (int i = 0; i < 4; i++) {
        idx += syn_blackbox_encode_varint(record->setpoint[i], &buf_out[idx]);
    }
    for (int i = 0; i < 4; i++) {
        idx += syn_blackbox_encode_varint(record->motor[i], &buf_out[idx]);
    }

    bb->last_record = *record;
    bb->frame_count++;
    bb->bytes_written += idx;
    *out_len = idx;

    return SYN_OK;
}

SYN_Status syn_blackbox_encode_delta(SYN_Blackbox *bb, const SYN_Blackbox_Record *record,
                                     uint8_t *buf_out, size_t *out_len)
{
    if (!bb || !record || !buf_out || !out_len) {
        return SYN_INVALID_PARAM;
    }

    size_t idx = 0;
    buf_out[idx++] = SYN_BLACKBOX_FRAME_DELTA;

    idx += syn_blackbox_encode_varint((int32_t)(record->iteration - bb->last_record.iteration),
                                      &buf_out[idx]);
    idx += syn_blackbox_encode_varint((int32_t)(record->time_us - bb->last_record.time_us),
                                      &buf_out[idx]);

    for (int i = 0; i < 3; i++) {
        idx += syn_blackbox_encode_varint(record->gyro[i] - bb->last_record.gyro[i], &buf_out[idx]);
    }
    for (int i = 0; i < 3; i++) {
        idx +=
            syn_blackbox_encode_varint(record->accel[i] - bb->last_record.accel[i], &buf_out[idx]);
    }
    for (int i = 0; i < 4; i++) {
        idx += syn_blackbox_encode_varint(record->setpoint[i] - bb->last_record.setpoint[i],
                                          &buf_out[idx]);
    }
    for (int i = 0; i < 4; i++) {
        idx += syn_blackbox_encode_varint(
            (int32_t)record->motor[i] - (int32_t)bb->last_record.motor[i], &buf_out[idx]);
    }

    bb->last_record = *record;
    bb->frame_count++;
    bb->bytes_written += idx;
    *out_len = idx;

    return SYN_OK;
}
```


