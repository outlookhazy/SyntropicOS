

# File syn\_protobuf.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_protobuf.c**](syn__protobuf_8c.md)

[Go to the documentation of this file](syn__protobuf_8c.md)


```C++

#include "syn_protobuf.h"

#include <string.h>

SYN_Status syn_pb_encoder_init(SYN_PB_Encoder *enc, uint8_t *buf, size_t capacity)
{
    if ((enc == NULL) || (buf == NULL) || (capacity == 0U)) {
        return SYN_INVALID_PARAM;
    }
    enc->buf = buf;
    enc->capacity = capacity;
    enc->offset = 0U;
    return SYN_OK;
}

SYN_Status syn_pb_encode_varint(SYN_PB_Encoder *enc, uint64_t value)
{
    if (enc == NULL) {
        return SYN_INVALID_PARAM;
    }
    while (value >= 0x80U) {
        if (enc->offset >= enc->capacity) {
            return SYN_ERROR; /* LCOV_EXCL_LINE: Defensive capacity guard */
        }
        enc->buf[enc->offset++] = (uint8_t)((value & 0x7FU) | 0x80U);
        value >>= 7;
    }
    if (enc->offset >= enc->capacity) {
        return SYN_ERROR;
    }
    enc->buf[enc->offset++] = (uint8_t)(value & 0x7FU);
    return SYN_OK;
}

SYN_Status syn_pb_encode_tag(SYN_PB_Encoder *enc, uint32_t field_number, uint8_t wire_type)
{
    if ((enc == NULL) || (field_number == 0U)) {
        return SYN_INVALID_PARAM;
    }
    uint32_t key = (field_number << 3) | (wire_type & 0x07U);
    return syn_pb_encode_varint(enc, key);
}

SYN_Status syn_pb_encode_int32(SYN_PB_Encoder *enc, uint32_t field_number, int32_t value)
{
    if (enc == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (syn_pb_encode_tag(enc, field_number, SYN_PB_WIRE_VARINT) != SYN_OK) {
        return SYN_ERROR;
    }
    return syn_pb_encode_varint(enc, (uint64_t)(int64_t)value);
}

SYN_Status syn_pb_encode_uint32(SYN_PB_Encoder *enc, uint32_t field_number, uint32_t value)
{
    if (enc == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (syn_pb_encode_tag(enc, field_number, SYN_PB_WIRE_VARINT) != SYN_OK) {
        return SYN_ERROR;
    }
    return syn_pb_encode_varint(enc, value);
}

SYN_Status syn_pb_encode_bool(SYN_PB_Encoder *enc, uint32_t field_number, bool value)
{
    return syn_pb_encode_uint32(enc, field_number, value ? 1U : 0U);
}

SYN_Status syn_pb_encode_fixed32(SYN_PB_Encoder *enc, uint32_t field_number, uint32_t value)
{
    if (enc == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (syn_pb_encode_tag(enc, field_number, SYN_PB_WIRE_FIXED32) != SYN_OK) {
        return SYN_ERROR;
    }
    if (enc->offset + 4U > enc->capacity) {
        return SYN_ERROR; /* LCOV_EXCL_LINE: Defensive capacity guard */
    }
    enc->buf[enc->offset++] = (uint8_t)(value & 0xFFU);
    enc->buf[enc->offset++] = (uint8_t)((value >> 8) & 0xFFU);
    enc->buf[enc->offset++] = (uint8_t)((value >> 16) & 0xFFU);
    enc->buf[enc->offset++] = (uint8_t)((value >> 24) & 0xFFU);
    return SYN_OK;
}

SYN_Status syn_pb_encode_bytes(SYN_PB_Encoder *enc, uint32_t field_number, const uint8_t *bytes,
                               size_t len)
{
    if ((enc == NULL) || (bytes == NULL)) {
        return SYN_INVALID_PARAM;
    }
    if (syn_pb_encode_tag(enc, field_number, SYN_PB_WIRE_LENGTH_DELIMITED) != SYN_OK) {
        return SYN_ERROR;
    }
    if (syn_pb_encode_varint(enc, len) != SYN_OK) {
        return SYN_ERROR; /* LCOV_EXCL_LINE: Defensive varint guard */
    }
    if (enc->offset + len > enc->capacity) {
        return SYN_ERROR; /* LCOV_EXCL_LINE: Defensive capacity guard */
    }
    memcpy(&enc->buf[enc->offset], bytes, len);
    enc->offset += len;
    return SYN_OK;
}

SYN_Status syn_pb_encode_string(SYN_PB_Encoder *enc, uint32_t field_number, const char *str)
{
    if (str == NULL) {
        return SYN_INVALID_PARAM;
    }
    return syn_pb_encode_bytes(enc, field_number, (const uint8_t *)str, strlen(str));
}

SYN_Status syn_pb_decoder_init(SYN_PB_Decoder *dec, const uint8_t *buf, size_t size)
{
    if ((dec == NULL) || (buf == NULL)) {
        return SYN_INVALID_PARAM;
    }
    dec->buf = buf;
    dec->size = size;
    dec->offset = 0U;
    return SYN_OK;
}

SYN_Status syn_pb_decode_varint(SYN_PB_Decoder *dec, uint64_t *value)
{
    if ((dec == NULL) || (value == NULL)) {
        return SYN_INVALID_PARAM;
    }
    uint64_t val = 0;
    uint32_t shift = 0;
    while (dec->offset < dec->size) {
        uint8_t b = dec->buf[dec->offset++];
        val |= ((uint64_t)(b & 0x7FU)) << shift;
        if ((b & 0x80U) == 0U) {
            *value = val;
            return SYN_OK;
        }
        shift += 7U;
        if (shift >= 64U) {
            return SYN_ERROR; /* Overflow */
        }
    }
    return SYN_ERROR;
}

SYN_Status syn_pb_decode_tag(SYN_PB_Decoder *dec, SYN_PB_Tag *tag)
{
    if ((dec == NULL) || (tag == NULL)) {
        return SYN_INVALID_PARAM;
    }
    if (dec->offset >= dec->size) {
        return SYN_ERROR; /* End of stream */
    }
    uint64_t key = 0;
    if (syn_pb_decode_varint(dec, &key) != SYN_OK) {
        return SYN_ERROR;
    }
    tag->field_number = (uint32_t)(key >> 3);
    tag->wire_type = (uint8_t)(key & 0x07U);
    return SYN_OK;
}

SYN_Status syn_pb_decode_fixed32(SYN_PB_Decoder *dec, uint32_t *value)
{
    if ((dec == NULL) || (value == NULL)) {
        return SYN_INVALID_PARAM;
    }
    if (dec->offset + 4U > dec->size) {
        return SYN_ERROR;
    }
    uint32_t val = ((uint32_t)dec->buf[dec->offset]) |
                   (((uint32_t)dec->buf[dec->offset + 1U]) << 8) |
                   (((uint32_t)dec->buf[dec->offset + 2U]) << 16) |
                   (((uint32_t)dec->buf[dec->offset + 3U]) << 24);
    dec->offset += 4U;
    *value = val;
    return SYN_OK;
}

SYN_Status syn_pb_decode_bytes(SYN_PB_Decoder *dec, const uint8_t **bytes, size_t *len)
{
    if ((dec == NULL) || (bytes == NULL) || (len == NULL)) {
        return SYN_INVALID_PARAM;
    }
    uint64_t l = 0;
    if (syn_pb_decode_varint(dec, &l) != SYN_OK) {
        return SYN_ERROR;
    }
    if (dec->offset + (size_t)l > dec->size) {
        return SYN_ERROR;
    }
    *bytes = &dec->buf[dec->offset];
    *len = (size_t)l;
    dec->offset += (size_t)l;
    return SYN_OK;
}

SYN_Status syn_pb_skip_field(SYN_PB_Decoder *dec, uint8_t wire_type)
{
    if (dec == NULL) {
        return SYN_INVALID_PARAM;
    }
    uint64_t dummy_v;
    uint32_t dummy_f;
    const uint8_t *dummy_b;
    size_t dummy_l;

    switch (wire_type) {
    /* LCOV_EXCL_START */
    case SYN_PB_WIRE_VARINT:
        return syn_pb_decode_varint(dec, &dummy_v);
    /* LCOV_EXCL_STOP */
    case SYN_PB_WIRE_FIXED32:
        return syn_pb_decode_fixed32(dec, &dummy_f);
    case SYN_PB_WIRE_LENGTH_DELIMITED:
        return syn_pb_decode_bytes(dec, &dummy_b, &dummy_l);
    case SYN_PB_WIRE_FIXED64:
        if (dec->offset + 8U > dec->size) {
            return SYN_ERROR;
        }
        dec->offset += 8U;
        return SYN_OK;
    default:
        return SYN_ERROR;
    }
}
```


