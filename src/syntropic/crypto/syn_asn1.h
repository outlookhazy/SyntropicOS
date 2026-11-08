/**
 * @file syn_asn1.h
 * @brief Zero-Heap ASN.1 DER (Distinguished Encoding Rules) TLV Parser.
 * @ingroup syn_crypto
 */

#ifndef SYN_ASN1_H
#define SYN_ASN1_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief ASN.1 Universal Tag Constants */
/** @brief ASN.1 BOOLEAN universal tag. */
#define SYN_ASN1_TAG_BOOLEAN 0x01U
/** @brief ASN.1 INTEGER universal tag. */
#define SYN_ASN1_TAG_INTEGER 0x02U
/** @brief ASN.1 BIT STRING universal tag. */
#define SYN_ASN1_TAG_BIT_STRING 0x03U
/** @brief ASN.1 OCTET STRING universal tag. */
#define SYN_ASN1_TAG_OCTET_STRING 0x04U
/** @brief ASN.1 NULL universal tag. */
#define SYN_ASN1_TAG_NULL 0x05U
/** @brief ASN.1 OID universal tag. */
#define SYN_ASN1_TAG_OID 0x06U
/** @brief ASN.1 SEQUENCE universal tag. */
#define SYN_ASN1_TAG_SEQUENCE 0x30U
/** @brief ASN.1 SET universal tag. */
#define SYN_ASN1_TAG_SET 0x31U
/** @brief ASN.1 UTCTime universal tag. */
#define SYN_ASN1_TAG_UTCTIME 0x17U
/** @brief ASN.1 GeneralizedTime universal tag. */
#define SYN_ASN1_TAG_GENERALIZEDTIME 0x18U

/** @brief ASN.1 Tag Class / Form Masking */
/** @brief Mask for ASN.1 tag class (top 2 bits). */
#define SYN_ASN1_CLASS_MASK 0xC0U
/** @brief Mask for ASN.1 constructed form (bit 5). */
#define SYN_ASN1_CONSTRUCTED_MASK 0x20U
/** @brief Mask for ASN.1 tag number (lower 5 bits). */
#define SYN_ASN1_TAG_NUM_MASK 0x1FU

/** @brief UNIVERSAL tag class (0x00). */
#define SYN_ASN1_CLASS_UNIVERSAL 0x00U
/** @brief APPLICATION tag class (0x40). */
#define SYN_ASN1_CLASS_APPLICATION 0x40U
/** @brief CONTEXT-SPECIFIC tag class (0x80). */
#define SYN_ASN1_CLASS_CONTEXT_SPECIFIC 0x80U
/** @brief PRIVATE tag class (0xC0). */
#define SYN_ASN1_CLASS_PRIVATE 0xC0U

/** @brief Parsed ASN.1 TLV element header. */
typedef struct {
    uint8_t tag;          /**< Full 8-bit tag byte */
    uint8_t tag_class;    /**< UNIVERSAL, CONTEXT_SPECIFIC, etc. */
    bool constructed;     /**< True if constructed container */
    uint8_t tag_number;   /**< Tag type number (0..31) */
    size_t length;        /**< Payload length in bytes */
    const uint8_t *value; /**< Pointer to payload bytes */
    size_t header_len;    /**< Total length of tag + length bytes */
} SYN_ASN1_Element;

/**
 * @brief Parse a single ASN.1 DER TLV element from buffer.
 *
 * @param buf      Data buffer.
 * @param len      Length of available bytes in buffer.
 * @param elem_out [out] Output parsed element.
 * @return true if valid TLV element parsed, false on truncation or encoding error.
 */
bool syn_asn1_parse_element(const uint8_t *buf, size_t len, SYN_ASN1_Element *elem_out);

/**
 * @brief Skip/advance past current TLV element in a DER buffer.
 *
 * @param buf      Pointer to buffer pointer (updated on success).
 * @param len      Pointer to available length (updated on success).
 * @param elem_out [out] Optional parsed element header.
 * @return true on success.
 */
bool syn_asn1_step(const uint8_t **buf, size_t *len, SYN_ASN1_Element *elem_out);

/**
 * @brief Unwraps a constructed container (SEQUENCE, SET, or context-specific tag) for child
 * iteration.
 *
 * @param container Pointer to container TLV element header.
 * @param child_buf [out] Sub-slice buffer pointer to container payload.
 * @param child_len [out] Length of container payload.
 * @return true if element is constructed and valid payload extracted.
 */
bool syn_asn1_enter_container(const SYN_ASN1_Element *container, const uint8_t **child_buf,
                              size_t *child_len);

/**
 * @brief Compare ASN.1 Object Identifier (OID) against expected byte array.
 *
 * @param elem         Parsed ASN.1 OID element.
 * @param expected_oid Expected encoded OID payload bytes.
 * @param oid_len      Length of expected OID payload bytes.
 * @return true if OIDs match.
 */
bool syn_asn1_match_oid(const SYN_ASN1_Element *elem, const uint8_t *expected_oid, size_t oid_len);

/**
 * @brief Extract raw integer bytes (stripping leading sign zero padding if present).
 *
 * @param elem        Parsed ASN.1 INTEGER element.
 * @param int_out     [out] Pointer to integer payload bytes.
 * @param int_len_out [out] Length of integer payload bytes.
 * @return true on success.
 */
bool syn_asn1_get_integer(const SYN_ASN1_Element *elem, const uint8_t **int_out,
                          size_t *int_len_out);

/**
 * @brief Extract Bit String payload (stripping unused bits count prefix byte).
 *
 * @param elem        Parsed ASN.1 BIT STRING element.
 * @param bits_out    [out] Pointer to raw bit string bytes.
 * @param bit_len_out [out] Number of valid bits in payload.
 * @return true on success.
 */
bool syn_asn1_get_bit_string(const SYN_ASN1_Element *elem, const uint8_t **bits_out,
                             size_t *bit_len_out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ASN1_H */
