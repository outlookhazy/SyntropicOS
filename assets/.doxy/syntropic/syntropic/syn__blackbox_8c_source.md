

# File syn\_blackbox.c

[**File List**](files.md) **>** [**log**](dir_1a216ccd2a39b426a7ff78fb20f852eb.md) **>** [**syn\_blackbox.c**](syn__blackbox_8c.md)

[Go to the documentation of this file](syn__blackbox_8c.md)


```C++


#include "syntropic/log/syn_blackbox.h"

#include <string.h>

static size_t encode_uvarint32(uint32_t uval, uint8_t *buf_out)
{
    size_t len = 0U;
    while (uval >= 0x80U) {
        buf_out[len++] = (uint8_t)((uval & 0x7FU) | 0x80U);
        uval >>= 7U;
    }
    buf_out[len++] = (uint8_t)(uval & 0x7FU);
    return len;
}

static size_t encode_varint64(int64_t val, uint8_t *buf_out)
{
    uint64_t uval = (val < 0) ? (((uint64_t)(~(uint64_t)val) << 1U) | 1ULL) : ((uint64_t)val << 1U);
    size_t len = 0U;
    while (uval >= 0x80ULL) {
        buf_out[len++] = (uint8_t)((uval & 0x7FULL) | 0x80ULL);
        uval >>= 7U;
    }
    buf_out[len++] = (uint8_t)(uval & 0x7FULL);
    return len;
}

size_t syn_blackbox_encode_varint(int32_t val, uint8_t *buf_out)
{
    if (buf_out == NULL) {
        return 0U;
    }

    uint32_t raw = (uint32_t)val;
    uint32_t uval = (raw << 1U) ^ (0U - (raw >> 31U));
    return encode_uvarint32(uval, buf_out);
}

SYN_Status syn_blackbox_init(SYN_Blackbox *bb)
{
    if (bb == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(bb, 0, sizeof(*bb));
    return SYN_OK;
}

SYN_Status syn_blackbox_encode_intra_ex(SYN_Blackbox *bb, const SYN_Blackbox_Record *record,
                                        uint8_t *buf_out, size_t buf_cap, size_t *out_len)
{
    if (bb == NULL || record == NULL || buf_out == NULL || out_len == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t scratch[SYN_BLACKBOX_MAX_FRAME_SIZE];
    size_t idx = 0U;
    scratch[idx++] = SYN_BLACKBOX_FRAME_INTRA;

    idx += encode_uvarint32(record->iteration, &scratch[idx]);
    idx += encode_uvarint32(record->time_us, &scratch[idx]);

    for (int i = 0; i < 3; i++) {
        idx += syn_blackbox_encode_varint(record->gyro[i], &scratch[idx]);
    }
    for (int i = 0; i < 3; i++) {
        idx += syn_blackbox_encode_varint(record->accel[i], &scratch[idx]);
    }
    for (int i = 0; i < 4; i++) {
        idx += syn_blackbox_encode_varint(record->setpoint[i], &scratch[idx]);
    }
    for (int i = 0; i < 4; i++) {
        idx += syn_blackbox_encode_varint(record->motor[i], &scratch[idx]);
    }

    if (idx > buf_cap) {
        return SYN_INVALID_PARAM;
    }

    memcpy(buf_out, scratch, idx);
    bb->last_record = *record;
    bb->frame_count++;
    bb->bytes_written += (uint32_t)idx;
    *out_len = idx;

    return SYN_OK;
}

SYN_Status syn_blackbox_encode_intra(SYN_Blackbox *bb, const SYN_Blackbox_Record *record,
                                     uint8_t *buf_out, size_t *out_len)
{
    return syn_blackbox_encode_intra_ex(bb, record, buf_out, SYN_BLACKBOX_MAX_FRAME_SIZE, out_len);
}

SYN_Status syn_blackbox_encode_delta_ex(SYN_Blackbox *bb, const SYN_Blackbox_Record *record,
                                        uint8_t *buf_out, size_t buf_cap, size_t *out_len)
{
    if (bb == NULL || record == NULL || buf_out == NULL || out_len == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t scratch[SYN_BLACKBOX_MAX_FRAME_SIZE];
    size_t idx = 0U;
    scratch[idx++] = SYN_BLACKBOX_FRAME_DELTA;

    idx += encode_varint64((int64_t)record->iteration - (int64_t)bb->last_record.iteration,
                           &scratch[idx]);
    idx +=
        encode_varint64((int64_t)record->time_us - (int64_t)bb->last_record.time_us, &scratch[idx]);

    for (int i = 0; i < 3; i++) {
        idx += syn_blackbox_encode_varint(
            (int32_t)record->gyro[i] - (int32_t)bb->last_record.gyro[i], &scratch[idx]);
    }
    for (int i = 0; i < 3; i++) {
        idx += syn_blackbox_encode_varint(
            (int32_t)record->accel[i] - (int32_t)bb->last_record.accel[i], &scratch[idx]);
    }
    for (int i = 0; i < 4; i++) {
        idx += syn_blackbox_encode_varint(
            (int32_t)record->setpoint[i] - (int32_t)bb->last_record.setpoint[i], &scratch[idx]);
    }
    for (int i = 0; i < 4; i++) {
        idx += syn_blackbox_encode_varint(
            (int32_t)record->motor[i] - (int32_t)bb->last_record.motor[i], &scratch[idx]);
    }

    if (idx > buf_cap) {
        return SYN_INVALID_PARAM;
    }

    memcpy(buf_out, scratch, idx);
    bb->last_record = *record;
    bb->frame_count++;
    bb->bytes_written += (uint32_t)idx;
    *out_len = idx;

    return SYN_OK;
}

SYN_Status syn_blackbox_encode_delta(SYN_Blackbox *bb, const SYN_Blackbox_Record *record,
                                     uint8_t *buf_out, size_t *out_len)
{
    return syn_blackbox_encode_delta_ex(bb, record, buf_out, SYN_BLACKBOX_MAX_FRAME_SIZE, out_len);
}
```


