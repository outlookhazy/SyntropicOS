/**
 * @file syn_protobuf.c
 * @brief Zero-Allocation Protocol Buffers Serializer & Deserializer Implementation.
 * @ingroup syn_util
 */

#include "syn_protobuf.h"

#include <string.h>

#define PB_MAX_FIELD_NUMBER 0x1FFFFFFFU /**< Maximum valid 29-bit Protobuf field number */
#define PB_MAX_WIRE_TYPE 5U             /**< Maximum valid Protobuf wire type ID */

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
    size_t saved_offset = enc->offset;
    while (value >= 0x80U) {
        if (enc->offset >= enc->capacity) {
            enc->offset = saved_offset;
            return SYN_ERROR;
        }
        enc->buf[enc->offset++] = (uint8_t)((value & 0x7FU) | 0x80U);
        value >>= 7U;
    }
    if (enc->offset >= enc->capacity) {
        enc->offset = saved_offset;
        return SYN_ERROR;
    }
    enc->buf[enc->offset++] = (uint8_t)(value & 0x7FU);
    return SYN_OK;
}

SYN_Status syn_pb_encode_tag(SYN_PB_Encoder *enc, uint32_t field_number, uint8_t wire_type)
{
    if ((enc == NULL) || (field_number == 0U) || (field_number > PB_MAX_FIELD_NUMBER) ||
        (wire_type > PB_MAX_WIRE_TYPE)) {
        return SYN_INVALID_PARAM;
    }
    uint64_t key = (((uint64_t)field_number) << 3U) | (uint64_t)(wire_type & 0x07U);
    return syn_pb_encode_varint(enc, key);
}

SYN_Status syn_pb_encode_int32(SYN_PB_Encoder *enc, uint32_t field_number, int32_t value)
{
    if (enc == NULL) {
        return SYN_INVALID_PARAM;
    }
    size_t saved_offset = enc->offset;
    if (syn_pb_encode_tag(enc, field_number, SYN_PB_WIRE_VARINT) != SYN_OK) {
        return SYN_ERROR;
    }
    if (syn_pb_encode_varint(enc, (uint64_t)(int64_t)value) != SYN_OK) {
        enc->offset = saved_offset;
        return SYN_ERROR;
    }
    return SYN_OK;
}

SYN_Status syn_pb_encode_uint32(SYN_PB_Encoder *enc, uint32_t field_number, uint32_t value)
{
    if (enc == NULL) {
        return SYN_INVALID_PARAM;
    }
    size_t saved_offset = enc->offset;
    if (syn_pb_encode_tag(enc, field_number, SYN_PB_WIRE_VARINT) != SYN_OK) {
        return SYN_ERROR;
    }
    if (syn_pb_encode_varint(enc, value) != SYN_OK) {
        enc->offset = saved_offset;
        return SYN_ERROR;
    }
    return SYN_OK;
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
    size_t saved_offset = enc->offset;
    if (syn_pb_encode_tag(enc, field_number, SYN_PB_WIRE_FIXED32) != SYN_OK) {
        return SYN_ERROR;
    }
    if ((enc->offset > enc->capacity) || ((enc->capacity - enc->offset) < 4U)) {
        enc->offset = saved_offset;
        return SYN_ERROR;
    }
    enc->buf[enc->offset++] = (uint8_t)(value & 0xFFU);
    enc->buf[enc->offset++] = (uint8_t)((value >> 8U) & 0xFFU);
    enc->buf[enc->offset++] = (uint8_t)((value >> 16U) & 0xFFU);
    enc->buf[enc->offset++] = (uint8_t)((value >> 24U) & 0xFFU);
    return SYN_OK;
}

SYN_Status syn_pb_encode_bytes(SYN_PB_Encoder *enc, uint32_t field_number, const uint8_t *bytes,
                               size_t len)
{
    if ((enc == NULL) || (bytes == NULL)) {
        return SYN_INVALID_PARAM;
    }
    size_t saved_offset = enc->offset;
    if (syn_pb_encode_tag(enc, field_number, SYN_PB_WIRE_LENGTH_DELIMITED) != SYN_OK) {
        return SYN_ERROR;
    }
    if ((syn_pb_encode_varint(enc, len) != SYN_OK) || (len > (enc->capacity - enc->offset))) {
        enc->offset = saved_offset;
        return SYN_ERROR;
    }
    if (len > 0U) {
        memcpy(&enc->buf[enc->offset], bytes, len);
        enc->offset += len;
    }
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
    uint64_t val = 0U;
    uint32_t shift = 0U;
    while (dec->offset < dec->size) {
        uint8_t b = dec->buf[dec->offset++];
        if ((shift == 63U) && ((b & 0xFEU) != 0U)) {
            return SYN_ERROR;
        }
        val |= ((uint64_t)(b & 0x7FU)) << shift;
        if ((b & 0x80U) == 0U) {
            *value = val;
            return SYN_OK;
        }
        shift += 7U;
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
    size_t saved_offset = dec->offset;
    uint64_t key = 0U;
    if (syn_pb_decode_varint(dec, &key) != SYN_OK) {
        dec->offset = saved_offset;
        return SYN_ERROR;
    }
    uint64_t field_num = key >> 3U;
    uint8_t wt = (uint8_t)(key & 0x07U);
    if ((field_num == 0ULL) || (field_num > (uint64_t)PB_MAX_FIELD_NUMBER) ||
        (wt > PB_MAX_WIRE_TYPE)) {
        dec->offset = saved_offset;
        return SYN_ERROR;
    }
    tag->field_number = (uint32_t)field_num;
    tag->wire_type = wt;
    return SYN_OK;
}

SYN_Status syn_pb_decode_fixed32(SYN_PB_Decoder *dec, uint32_t *value)
{
    if ((dec == NULL) || (value == NULL)) {
        return SYN_INVALID_PARAM;
    }
    if ((dec->size < 4U) || (dec->offset > (dec->size - 4U))) {
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
    uint64_t l = 0U;
    if (syn_pb_decode_varint(dec, &l) != SYN_OK) {
        return SYN_ERROR;
    }
    size_t rem = dec->size - dec->offset;
    if (l > (uint64_t)rem) {
        return SYN_ERROR;
    }
    size_t n = (size_t)l;
    *bytes = &dec->buf[dec->offset];
    *len = n;
    dec->offset += n;
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
