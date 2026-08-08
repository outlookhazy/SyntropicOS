

# File syn\_asn1.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_asn1.h**](syn__asn1_8h.md)

[Go to the documentation of this file](syn__asn1_8h.md)


```C++

#ifndef SYN_ASN1_H
#define SYN_ASN1_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_ASN1_TAG_BOOLEAN 0x01U
#define SYN_ASN1_TAG_INTEGER 0x02U
#define SYN_ASN1_TAG_BIT_STRING 0x03U
#define SYN_ASN1_TAG_OCTET_STRING 0x04U
#define SYN_ASN1_TAG_NULL 0x05U
#define SYN_ASN1_TAG_OID 0x06U
#define SYN_ASN1_TAG_SEQUENCE 0x30U
#define SYN_ASN1_TAG_SET 0x31U
#define SYN_ASN1_TAG_UTCTIME 0x17U
#define SYN_ASN1_TAG_GENERALIZEDTIME 0x18U

#define SYN_ASN1_CLASS_MASK 0xC0U
#define SYN_ASN1_CONSTRUCTED_MASK 0x20U
#define SYN_ASN1_TAG_NUM_MASK 0x1FU

#define SYN_ASN1_CLASS_UNIVERSAL 0x00U
#define SYN_ASN1_CLASS_APPLICATION 0x40U
#define SYN_ASN1_CLASS_CONTEXT_SPECIFIC 0x80U
#define SYN_ASN1_CLASS_PRIVATE 0xC0U

typedef struct {
    uint8_t tag;          
    uint8_t tag_class;    
    bool constructed;     
    uint8_t tag_number;   
    size_t length;        
    const uint8_t *value; 
    size_t header_len;    
} SYN_ASN1_Element;

bool syn_asn1_parse_element(const uint8_t *buf, size_t len, SYN_ASN1_Element *elem_out);

bool syn_asn1_step(const uint8_t **buf, size_t *len, SYN_ASN1_Element *elem_out);

bool syn_asn1_enter_container(const SYN_ASN1_Element *container, const uint8_t **child_buf,
                              size_t *child_len);

bool syn_asn1_match_oid(const SYN_ASN1_Element *elem, const uint8_t *expected_oid, size_t oid_len);

bool syn_asn1_get_integer(const SYN_ASN1_Element *elem, const uint8_t **int_out,
                          size_t *int_len_out);

bool syn_asn1_get_bit_string(const SYN_ASN1_Element *elem, const uint8_t **bits_out,
                             size_t *bit_len_out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ASN1_H */
```


