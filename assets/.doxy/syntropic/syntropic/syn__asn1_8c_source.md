

# File syn\_asn1.c

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_asn1.c**](syn__asn1_8c.md)

[Go to the documentation of this file](syn__asn1_8c.md)


```C++

#include "syntropic/crypto/syn_asn1.h"

#include <string.h>

bool syn_asn1_parse_element(const uint8_t *buf, size_t len, SYN_ASN1_Element *elem_out)
{
    if (buf == NULL || elem_out == NULL || len < 2) {
        return false;
    }

    memset(elem_out, 0, sizeof(SYN_ASN1_Element));

    size_t cur = 0;
    elem_out->tag = buf[cur++];
    elem_out->tag_class = elem_out->tag & SYN_ASN1_CLASS_MASK;
    elem_out->constructed = (elem_out->tag & SYN_ASN1_CONSTRUCTED_MASK) != 0;
    elem_out->tag_number = elem_out->tag & SYN_ASN1_TAG_NUM_MASK;

    /* High tag number extension (tag_number == 0x1F) not standard in basic DER certs,
     * but we support single byte tag parsing for standard universal & context-specific tags */
    if (elem_out->tag_number == 0x1FU) {
        return false;
    }

    uint8_t len_byte = buf[cur++];

    if ((len_byte & 0x80U) == 0) {
        /* Short form length: 0..127 */
        elem_out->length = len_byte;
    } else {
        /* Long form length: 0x80 | num_length_bytes */
        uint8_t num_len_bytes = len_byte & 0x7FU;
        if (num_len_bytes == 0 || num_len_bytes > sizeof(size_t) || (cur + num_len_bytes) > len) {
            return false;
        }

        size_t parsed_len = 0;
        for (uint8_t i = 0; i < num_len_bytes; i++) {
            parsed_len = (parsed_len << 8) | buf[cur++];
        }
        elem_out->length = parsed_len;
    }

    elem_out->header_len = cur;
    elem_out->value = buf + cur;

    if (cur > len || elem_out->length > (len - cur)) {
        return false;
    }

    return true;
}

bool syn_asn1_step(const uint8_t **buf, size_t *len, SYN_ASN1_Element *elem_out)
{
    if (buf == NULL || *buf == NULL || len == NULL || *len == 0) {
        return false;
    }

    SYN_ASN1_Element elem;
    if (!syn_asn1_parse_element(*buf, *len, &elem)) {
        return false;
    }

    size_t total_consumed = elem.header_len + elem.length;
    *buf += total_consumed;
    *len -= total_consumed;

    if (elem_out != NULL) {
        *elem_out = elem;
    }

    return true;
}

bool syn_asn1_enter_container(const SYN_ASN1_Element *container, const uint8_t **child_buf,
                              size_t *child_len)
{
    if (container == NULL || child_buf == NULL || child_len == NULL || !container->constructed) {
        return false;
    }

    *child_buf = container->value;
    *child_len = container->length;
    return true;
}

bool syn_asn1_match_oid(const SYN_ASN1_Element *elem, const uint8_t *expected_oid, size_t oid_len)
{
    if (elem == NULL || expected_oid == NULL || elem->tag != SYN_ASN1_TAG_OID) {
        return false;
    }

    if (elem->length != oid_len) {
        return false;
    }

    return (memcmp(elem->value, expected_oid, oid_len) == 0);
}

bool syn_asn1_get_integer(const SYN_ASN1_Element *elem, const uint8_t **int_out,
                          size_t *int_len_out)
{
    if (elem == NULL || int_out == NULL || int_len_out == NULL ||
        elem->tag != SYN_ASN1_TAG_INTEGER) {
        return false;
    }

    const uint8_t *val = elem->value;
    size_t val_len = elem->length;

    if (val_len == 0) {
        return false;
    }

    /* Strip leading 0x00 byte for positive integers where top bit of next byte is set */
    if (val_len > 1 && val[0] == 0x00U && (val[1] & 0x80U) != 0) {
        val++;
        val_len--;
    }

    *int_out = val;
    *int_len_out = val_len;
    return true;
}

bool syn_asn1_get_bit_string(const SYN_ASN1_Element *elem, const uint8_t **bits_out,
                             size_t *bit_len_out)
{
    if (elem == NULL || bits_out == NULL || bit_len_out == NULL ||
        elem->tag != SYN_ASN1_TAG_BIT_STRING) {
        return false;
    }

    if (elem->length < 1) {
        return false;
    }

    uint8_t unused_bits = elem->value[0];
    if (unused_bits > 7) {
        return false;
    }

    size_t total_payload_bytes = elem->length - 1;
    *bits_out = elem->value + 1;
    *bit_len_out = (total_payload_bytes * 8U) - unused_bits;
    return true;
}
```


