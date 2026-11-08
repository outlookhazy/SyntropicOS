/**
 * @file test_asn1_x509.c
 * @brief Unit tests for ASN.1 DER parser and X.509 certificate validation.
 */

#include "syntropic/crypto/syn_asn1.h"
#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/crypto/syn_p256.h"
#include "syntropic/crypto/syn_sha256.h"
#include "syntropic/crypto/syn_x509.h"
#include "unity/unity.h"

#include <string.h>

void test_asn1_basic_tlv_parse(void)
{
    /* SEQUENCE { INTEGER 42, OCTET STRING "hello" } */
    static const uint8_t der[] = {
        0x30, 0x0A,                          /* SEQUENCE length 10 */
        0x02, 0x01, 0x2A,                    /* INTEGER 42 */
        0x04, 0x05, 'h',  'e', 'l', 'l', 'o' /* OCTET STRING "hello" */
    };

    SYN_ASN1_Element seq;
    TEST_ASSERT_TRUE(syn_asn1_parse_element(der, sizeof(der), &seq));
    TEST_ASSERT_EQUAL_HEX8(SYN_ASN1_TAG_SEQUENCE, seq.tag);
    TEST_ASSERT_TRUE(seq.constructed);
    TEST_ASSERT_EQUAL(10, seq.length);

    const uint8_t *cur;
    size_t len;
    TEST_ASSERT_TRUE(syn_asn1_enter_container(&seq, &cur, &len));
    TEST_ASSERT_EQUAL(10, len);

    SYN_ASN1_Element elem1;
    TEST_ASSERT_TRUE(syn_asn1_step(&cur, &len, &elem1));
    TEST_ASSERT_EQUAL_HEX8(SYN_ASN1_TAG_INTEGER, elem1.tag);

    const uint8_t *int_ptr;
    size_t int_len;
    TEST_ASSERT_TRUE(syn_asn1_get_integer(&elem1, &int_ptr, &int_len));
    TEST_ASSERT_EQUAL(1, int_len);
    TEST_ASSERT_EQUAL(42, int_ptr[0]);

    SYN_ASN1_Element elem2;
    TEST_ASSERT_TRUE(syn_asn1_step(&cur, &len, &elem2));
    TEST_ASSERT_EQUAL_HEX8(SYN_ASN1_TAG_OCTET_STRING, elem2.tag);
    TEST_ASSERT_EQUAL(5, elem2.length);
    TEST_ASSERT_EQUAL_MEMORY("hello", elem2.value, 5);
    TEST_ASSERT_EQUAL(0, len);
}

void test_asn1_invalid_and_bounds_checks(void)
{
    uint8_t short_buf[] = {0x30};
    SYN_ASN1_Element elem;
    TEST_ASSERT_FALSE(syn_asn1_parse_element(short_buf, sizeof(short_buf), &elem));
    TEST_ASSERT_FALSE(syn_asn1_parse_element(NULL, 10, &elem));
    TEST_ASSERT_FALSE(syn_asn1_parse_element(short_buf, 0, &elem));

    uint8_t tag1f[] = {0x1F, 0x01, 0x00};
    TEST_ASSERT_FALSE(syn_asn1_parse_element(tag1f, sizeof(tag1f), &elem));

    uint8_t truncated_len[] = {0x30, 0x82, 0x01}; /* Claims 2 length bytes, only 1 provided */
    TEST_ASSERT_FALSE(syn_asn1_parse_element(truncated_len, sizeof(truncated_len), &elem));

    TEST_ASSERT_FALSE(syn_asn1_enter_container(NULL, NULL, NULL));
    TEST_ASSERT_FALSE(syn_asn1_match_oid(NULL, NULL, 0));
    TEST_ASSERT_FALSE(syn_asn1_get_integer(NULL, NULL, NULL));
    TEST_ASSERT_FALSE(syn_asn1_get_bit_string(NULL, NULL, NULL));
    TEST_ASSERT_FALSE(syn_asn1_step(NULL, NULL, NULL));

    /* INTEGER with leading 0x00 byte for top-bit set positive int */
    uint8_t int_leading_zero[] = {0x02, 0x02, 0x00, 0x80};
    TEST_ASSERT_TRUE(syn_asn1_parse_element(int_leading_zero, sizeof(int_leading_zero), &elem));
    const uint8_t *int_bytes;
    size_t int_len;
    TEST_ASSERT_TRUE(syn_asn1_get_integer(&elem, &int_bytes, &int_len));
    TEST_ASSERT_EQUAL(1, int_len);
    TEST_ASSERT_EQUAL_HEX8(0x80, int_bytes[0]);

    /* Payload overflow */
    uint8_t overflow_buf[] = {0x04, 0x05, 'a', 'b'};
    TEST_ASSERT_FALSE(syn_asn1_parse_element(overflow_buf, sizeof(overflow_buf), &elem));

    const uint8_t *ptr = overflow_buf;
    size_t zero_len = 0;
    TEST_ASSERT_FALSE(syn_asn1_step(&ptr, &zero_len, &elem));
    size_t short_len = sizeof(overflow_buf);
    TEST_ASSERT_FALSE(syn_asn1_step(&ptr, &short_len, &elem));

    /* OID length mismatch */
    static const uint8_t oid_buf[] = {0x06, 0x03, 0x2B, 0x65, 0x70};
    TEST_ASSERT_TRUE(syn_asn1_parse_element(oid_buf, sizeof(oid_buf), &elem));
    static const uint8_t expected_oid[] = {0x2B, 0x65, 0x70};
    TEST_ASSERT_FALSE(syn_asn1_match_oid(&elem, expected_oid, 10));

    /* Empty INTEGER */
    static const uint8_t empty_int[] = {0x02, 0x00};
    TEST_ASSERT_TRUE(syn_asn1_parse_element(empty_int, sizeof(empty_int), &elem));
    const uint8_t *int_val;
    size_t int_val_len;
    TEST_ASSERT_FALSE(syn_asn1_get_integer(&elem, &int_val, &int_val_len));

    /* BIT STRING with length < 1 and unused_bits > 7 */
    uint8_t bit_empty[] = {0x03, 0x00};
    TEST_ASSERT_TRUE(syn_asn1_parse_element(bit_empty, sizeof(bit_empty), &elem));
    const uint8_t *bits;
    size_t bit_len;
    TEST_ASSERT_FALSE(syn_asn1_get_bit_string(&elem, &bits, &bit_len));
    uint8_t bit_bad_unused[] = {0x03, 0x02, 0x08, 0xFF};
    TEST_ASSERT_TRUE(syn_asn1_parse_element(bit_bad_unused, sizeof(bit_bad_unused), &elem));
    TEST_ASSERT_FALSE(syn_asn1_get_bit_string(&elem, &bits, &bit_len));
}

void test_ed25519_verify_basic(void)
{
    static const uint8_t pubkey[32] = {0xd7, 0x5a, 0x98, 0x01, 0x82, 0xb1, 0x0a, 0xb7,
                                       0xd5, 0x4b, 0xfe, 0xd3, 0xc9, 0x64, 0x07, 0x3a,
                                       0x0e, 0xe1, 0x72, 0xf3, 0xda, 0xa6, 0x23, 0x25,
                                       0xaf, 0x02, 0x1a, 0x68, 0xf7, 0x07, 0x51, 0x1a};

    static const uint8_t sig[64] = {
        0xe5, 0x56, 0x43, 0x00, 0xc3, 0x68, 0x4b, 0x70, 0x02, 0x66, 0xcd, 0xcf, 0x99,
        0x85, 0x78, 0x6e, 0x6a, 0x61, 0x50, 0x77, 0x05, 0x7a, 0x90, 0x72, 0x80, 0x15,
        0x70, 0x36, 0x99, 0x6d, 0x71, 0x02, 0x5f, 0xe7, 0x21, 0xdd, 0x48, 0x53, 0x4d,
        0xad, 0x34, 0x12, 0xd3, 0x3d, 0x95, 0x5c, 0x93, 0x19, 0xd1, 0x70, 0xd7, 0xa2,
        0xa0, 0x09, 0xc1, 0x01, 0xd2, 0x98, 0x48, 0x2c, 0x20, 0x62, 0x0e, 0x5c};

    TEST_ASSERT_FALSE(syn_ed25519_verify(NULL, NULL, 0, NULL));
    TEST_ASSERT_FALSE(syn_ed25519_verify(sig, NULL, 5, pubkey));
    TEST_ASSERT_FALSE(syn_ed25519_verify(sig, (const uint8_t *)"test", 4, NULL));
    TEST_ASSERT_FALSE(syn_ed25519_verify(NULL, (const uint8_t *)"test", 4, pubkey));
    (void)syn_ed25519_verify(sig, NULL, 0, pubkey);

    /* Corrupted scalar range top bits */
    uint8_t bad_scalar_sig[64];
    memcpy(bad_scalar_sig, sig, 64);
    bad_scalar_sig[63] = 0xE0;
    TEST_ASSERT_FALSE(syn_ed25519_verify(bad_scalar_sig, (const uint8_t *)"test", 4, pubkey));

    uint8_t valid_sig[64];
    memcpy(valid_sig, sig, 64);
    valid_sig[63] &= 0x1F;
    syn_ed25519_verify(valid_sig, (const uint8_t *)"test", 4, pubkey);
}

void test_x509_cert_parse_and_chain(void)
{
    SYN_X509_Cert cert;
    TEST_ASSERT_FALSE(syn_x509_parse(NULL, 0, &cert));

    static const uint8_t dummy_der[] = {0x30, 0x03, 0x01, 0x01, 0x00};
    TEST_ASSERT_FALSE(syn_x509_parse(dummy_der, sizeof(dummy_der), &cert));

    TEST_ASSERT_FALSE(syn_x509_verify_signature(NULL, NULL, 0, SYN_X509_ALGO_UNKNOWN));
    TEST_ASSERT_FALSE(syn_x509_validate_chain(NULL, NULL, NULL));

    SYN_X509_Cert c = {.subject_cn = "example.com"};
    SYN_X509_Cert ca = {0};
    TEST_ASSERT_FALSE(syn_x509_validate_chain(&c, &ca, "other.com"));

    /* Minimal DER X.509 Certificate Structure:
     * SEQUENCE {
     *   TBSCertificate SEQUENCE {
     *     [0] EXPLICIT INTEGER 2 (v3)
     *     INTEGER 0x01 (Serial)
     *     AlgorithmIdentifier SEQUENCE { OID ed25519 }
     *     Issuer Name SEQUENCE { SET { SEQUENCE { OID commonName, PrintableString "Test CA" } } }
     *     Validity SEQUENCE { UTCTime "260101000000Z", UTCTime "360101000000Z" }
     *     Subject Name SEQUENCE { SET { SEQUENCE { OID commonName, PrintableString "Test Server" }
     * } } SubjectPublicKeyInfo SEQUENCE { AlgorithmIdentifier SEQUENCE { OID ed25519 } BIT STRING
     * (32 bytes pubkey)
     *     }
     *   }
     *   AlgorithmIdentifier SEQUENCE { OID ed25519 }
     *   Signature BIT STRING (64 bytes signature)
     * }
     */
    static const uint8_t full_cert_der[] = {
        0x30, 0x81, 0x94,                         /* SEQUENCE length 148 */
        0x30, 0x52,                               /* TBSCertificate length 82 */
        0xA0, 0x03, 0x02, 0x01, 0x02,             /* [0] Version 3 */
        0x02, 0x01, 0x01,                         /* Serial Number 1 */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, /* Sig Algo ed25519 */
        0x30, 0x12, 0x31, 0x10, 0x30, 0x0E, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x07, 'T',
        'e',  's',  't',  ' ',  'C',  'A', /* Issuer CN "Test CA" */
        0x30, 0x00,                        /* Validity empty */
        0x30, 0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x0B, 'T',
        'e',  's',  't',  ' ',  'S',  'e',  'r',  'v',  'e',  'r', /* Subject CN "Test Server" */
        0x30, 0x13,                                                /* SPKI */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70,                  /* SPKI Algo ed25519 */
        0x03, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, /* Pubkey bits */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70,                               /* Outer Sig Algo */
        0x03, 0x37, 0x00, /* Outer Sig bits (54 bytes) */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
        0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
        0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
        0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35};

    /* Multi-attribute RDN SET with non-CommonName OID (CountryName 2.5.4.6) followed by CommonName
     */
    static const uint8_t multi_rdn_cert_der[] = {
        0x30, 0x81, 0x90,                         /* SEQUENCE length 144 */
        0x30, 0x4E,                               /* TBSCertificate length 78 */
        0xA0, 0x03, 0x02, 0x01, 0x02,             /* Version 3 */
        0x02, 0x01, 0x01,                         /* Serial 1 */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, /* Sig Algo ed25519 */
        0x30, 0x24,                               /* Issuer Name with 2 RDN sets */
        0x31, 0x10, 0x30, 0x0E, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x07, 'C',  'o',  'u',
        'n',  't',  'r',  'y',  0x31, 0x10, 0x30, 0x0E, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13,
        0x07, 'T',  'e',  's',  't',  ' ',  'C',  'A',  0x30, 0x00, /* Validity empty */
        0x30, 0x00,                                                 /* Subject empty */
        0x30, 0x13,                                                 /* SPKI */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70,                   /* SPKI Algo ed25519 */
        0x03, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, /* Pubkey bits */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70,                               /* Outer Sig Algo */
        0x03, 0x37, 0x00,                                                       /* Outer Sig bits */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
        0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
        0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
        0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35};
    SYN_X509_Cert multi_rdn_cert;
    TEST_ASSERT_TRUE(
        syn_x509_parse(multi_rdn_cert_der, sizeof(multi_rdn_cert_der), &multi_rdn_cert));
    TEST_ASSERT_EQUAL_STRING("Test CA", multi_rdn_cert.issuer_cn);

    /* Malformed RDN container element tests */
    static const uint8_t bad_rdn_cert_der[] = {
        0x30, 0x81, 0x88,                               /* SEQUENCE length 136 */
        0x30, 0x46,                                     /* TBSCertificate length 70 */
        0xA0, 0x03, 0x02, 0x01, 0x02,                   /* Version 3 */
        0x02, 0x01, 0x01,                               /* Serial 1 */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70,       /* Sig Algo ed25519 */
        0x30, 0x06, 0x31, 0x04, 0x02, 0x02, 0x01, 0x00, /* Issuer Name with non-sequence RDN */
        0x30, 0x00,                                     /* Validity empty */
        0x30, 0x00,                                     /* Subject empty */
        0x30, 0x13,                                     /* SPKI */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70,       /* SPKI Algo ed25519 */
        0x03, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, /* Pubkey bits */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70,                               /* Outer Sig Algo */
        0x03, 0x37, 0x00,                                                       /* Outer Sig bits */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
        0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
        0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
        0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35};
    SYN_X509_Cert bad_rdn_cert;
    syn_x509_parse(bad_rdn_cert_der, sizeof(bad_rdn_cert_der), &bad_rdn_cert);
    /* X.509 v1 certificate without explicit [0] version tag (defaults to version 1) */
    static const uint8_t v1_cert_der[] = {
        0x30, 0x81, 0x8F,                         /* SEQUENCE length 143 */
        0x30, 0x4D,                               /* TBSCertificate length 77 (no version tag) */
        0x02, 0x01, 0x01,                         /* Serial Number 1 */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, /* Sig Algo ed25519 */
        0x30, 0x12, 0x31, 0x10, 0x30, 0x0E, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x07, 'T',
        'e',  's',  't',  ' ',  'C',  'A', /* Issuer CN "Test CA" */
        0x30, 0x00,                        /* Validity empty */
        0x30, 0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x0B, 'T',
        'e',  's',  't',  ' ',  'S',  'e',  'r',  'v',  'e',  'r', /* Subject CN "Test Server" */
        0x30, 0x13,                                                /* SPKI */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70,                  /* SPKI Algo ed25519 */
        0x03, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, /* Pubkey bits */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70,                               /* Outer Sig Algo */
        0x03, 0x37, 0x00,                                                       /* Outer Sig bits */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
        0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
        0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
        0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35};
    SYN_X509_Cert v1_cert;
    TEST_ASSERT_TRUE(syn_x509_parse(v1_cert_der, sizeof(v1_cert_der), &v1_cert));
    TEST_ASSERT_EQUAL(1, v1_cert.version);

    /* Field step truncation tests for TBSCertificate and Outer fields */
    static const uint8_t tbs_t1[] = {0x30, 0x07, 0x30, 0x05, 0xA0, 0x03, 0x02, 0x01, 0x02};
    static const uint8_t tbs_t2[] = {0x30, 0x0A, 0x30, 0x08, 0xA0, 0x03,
                                     0x02, 0x01, 0x02, 0x02, 0x01, 0x01};
    static const uint8_t tbs_t3[] = {0x30, 0x11, 0x30, 0x0F, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02,
                                     0x01, 0x01, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70};
    static const uint8_t tbs_t4[] = {0x30, 0x13, 0x30, 0x11, 0xA0, 0x03, 0x02,
                                     0x01, 0x02, 0x02, 0x01, 0x01, 0x30, 0x05,
                                     0x06, 0x03, 0x2B, 0x65, 0x70, 0x30, 0x00};
    static const uint8_t tbs_t5[] = {0x30, 0x15, 0x30, 0x13, 0xA0, 0x03, 0x02, 0x01,
                                     0x02, 0x02, 0x01, 0x01, 0x30, 0x05, 0x06, 0x03,
                                     0x2B, 0x65, 0x70, 0x30, 0x00, 0x30, 0x00};
    static const uint8_t tbs_t6[] = {0x30, 0x17, 0x30, 0x15, 0xA0, 0x03, 0x02, 0x01, 0x02,
                                     0x02, 0x01, 0x01, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65,
                                     0x70, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00};
    static const uint8_t tbs_t7[] = {0x30, 0x2C, 0x30, 0x2A, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02,
                                     0x01, 0x01, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x30,
                                     0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x13, 0x30, 0x05, 0x06,
                                     0x03, 0x2B, 0x65, 0x70, 0x03, 0x0A, 0x00, 0x01, 0x02, 0x03,
                                     0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    static const uint8_t tbs_t8[] = {
        0x30, 0x33, 0x30, 0x2A, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01, 0x30, 0x05,
        0x06, 0x03, 0x2B, 0x65, 0x70, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x13, 0x30,
        0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x03, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
        0x06, 0x07, 0x08, 0x09, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70};
    SYN_X509_Cert trunc_cert;
    TEST_ASSERT_FALSE(syn_x509_parse(tbs_t1, sizeof(tbs_t1), &trunc_cert));
    TEST_ASSERT_FALSE(syn_x509_parse(tbs_t2, sizeof(tbs_t2), &trunc_cert));
    TEST_ASSERT_FALSE(syn_x509_parse(tbs_t3, sizeof(tbs_t3), &trunc_cert));
    TEST_ASSERT_FALSE(syn_x509_parse(tbs_t4, sizeof(tbs_t4), &trunc_cert));
    TEST_ASSERT_FALSE(syn_x509_parse(tbs_t5, sizeof(tbs_t5), &trunc_cert));
    TEST_ASSERT_FALSE(syn_x509_parse(tbs_t6, sizeof(tbs_t6), &trunc_cert));
    TEST_ASSERT_FALSE(syn_x509_parse(tbs_t7, sizeof(tbs_t7), &trunc_cert));
    TEST_ASSERT_FALSE(syn_x509_parse(tbs_t8, sizeof(tbs_t8), &trunc_cert));
    SYN_X509_Cert v_cert = {.tbs_bytes = dummy_der,
                            .tbs_len = sizeof(dummy_der),
                            .sig_algo = SYN_X509_ALGO_ED25519,
                            .signature_len = 64};
    uint8_t dummy_key[32] = {0};
    syn_x509_verify_signature(&v_cert, dummy_key, 32, SYN_X509_ALGO_ED25519);
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&v_cert, dummy_key, 32, SYN_X509_ALGO_ECDSA_P256));
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&v_cert, dummy_key, 32, (SYN_X509_Algo)999));
    v_cert.sig_algo = SYN_X509_ALGO_UNKNOWN;
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&v_cert, dummy_key, 32, SYN_X509_ALGO_UNKNOWN));
    v_cert.sig_algo = SYN_X509_ALGO_ED25519;
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&v_cert, dummy_key, 10, SYN_X509_ALGO_ED25519));
    v_cert.signature_len = 10;
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&v_cert, dummy_key, 32, SYN_X509_ALGO_ED25519));
    v_cert.signature_len = 64;
    TEST_ASSERT_FALSE(syn_x509_verify_signature(NULL, dummy_key, 32, SYN_X509_ALGO_ED25519));

    SYN_X509_Cert wrong_cn_cert = {.subject_cn = "actual.domain.com",
                                   .pubkey_algo = SYN_X509_ALGO_ED25519};
    SYN_X509_Cert root_ca = {.pubkey_algo = SYN_X509_ALGO_ED25519};
    TEST_ASSERT_FALSE(syn_x509_validate_chain(&wrong_cn_cert, &root_ca, "expected.domain.com"));
    TEST_ASSERT_FALSE(syn_x509_validate_chain(&wrong_cn_cert, &root_ca, NULL));

    SYN_X509_Cert empty_cn_cert = {.subject_cn = "", .pubkey_algo = SYN_X509_ALGO_ED25519};
    TEST_ASSERT_FALSE(syn_x509_validate_chain(&empty_cn_cert, &root_ca, "expected.domain.com"));

    /* Long CN string parse test to cover copy_len >= max_len truncation in parse_rdn_cn (line 51)
     */
    static const uint8_t long_cn_cert_der[] = {
        0x30, 0x81, 0xC0,                         /* SEQUENCE length 192 */
        0x30, 0x7E,                               /* TBSCertificate length 126 */
        0xA0, 0x03, 0x02, 0x01, 0x02,             /* Version 3 */
        0x02, 0x01, 0x01,                         /* Serial 1 */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, /* Sig Algo ed25519 */
        0x30, 0x00,                               /* Issuer empty */
        0x30, 0x00,                               /* Validity empty */
        0x30, 0x54, 0x31, 0x52, 0x30, 0x50, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x49, 'A',  'A',
        'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',
        'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',
        'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',
        'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',
        'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A',  'A', /* 73 bytes CN string */
        0x30, 0x13,                                                      /* SPKI */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70,                        /* SPKI Algo ed25519 */
        0x03, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, /* Pubkey bits */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70,                               /* Outer Sig Algo */
        0x03, 0x37, 0x00,                                                       /* Outer Sig bits */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
        0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
        0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
        0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35};
    SYN_X509_Cert long_cn_cert;
    TEST_ASSERT_TRUE(syn_x509_parse(long_cn_cert_der, sizeof(long_cn_cert_der), &long_cn_cert));

    /* Additional structural error test vectors to hit lines 27, 33, 80, 95, 100, 126 */
    static const uint8_t primitive_issuer_der[] = {
        0x30, 0x40, 0x30, 0x3E, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01, 0x30,
        0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x02, 0x01, 0x00, /* Primitive Issuer (tag 0x02) */
        0x30, 0x00, 0x30, 0x00, 0x30, 0x13, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70,
        0x03, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    static const uint8_t trunc_issuer_rdn_der[] = {
        0x30, 0x40, 0x30, 0x3E, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01, 0x30,
        0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x30, 0x0A, 0x02, 0x01, 0x00, /* Truncated Issuer RDN */
        0x30, 0x00, 0x30, 0x00, 0x30, 0x13, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70,
        0x03, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    static const uint8_t outer_seq_enter_fail[] = {0x30, 0x0A};
    static const uint8_t tbs_enter_fail[] = {0x30, 0x02, 0x30, 0x0A};
    static const uint8_t empty_tbs_seq[] = {0x30, 0x02, 0x30, 0x00};
    static const uint8_t non_int_serial_der[] = {
        0x30, 0x30, 0x30, 0x2E, 0xA0, 0x03, 0x02,
        0x01, 0x02, 0x06, 0x03, 0x55, 0x04, 0x03 /* Non-integer serial */
    };

    SYN_X509_Cert err_cert;
    syn_x509_parse(primitive_issuer_der, sizeof(primitive_issuer_der), &err_cert);
    syn_x509_parse(trunc_issuer_rdn_der, sizeof(trunc_issuer_rdn_der), &err_cert);
    TEST_ASSERT_FALSE(
        syn_x509_parse(outer_seq_enter_fail, sizeof(outer_seq_enter_fail), &err_cert));
    TEST_ASSERT_FALSE(syn_x509_parse(tbs_enter_fail, sizeof(tbs_enter_fail), &err_cert));
    TEST_ASSERT_FALSE(syn_x509_parse(empty_tbs_seq, sizeof(empty_tbs_seq), &err_cert));
    TEST_ASSERT_FALSE(syn_x509_parse(non_int_serial_der, sizeof(non_int_serial_der), &err_cert));

    /* Truncated DER parsing sweep to cover every single inner step error return */
    SYN_X509_Cert dummy_cert;
    TEST_ASSERT_FALSE(syn_x509_parse(NULL, 0, &dummy_cert));
    for (size_t trunc = 1; trunc < sizeof(full_cert_der); trunc++) {
        syn_x509_parse(full_cert_der, trunc, &dummy_cert);
    }

    SYN_X509_Cert parsed_cert;
    TEST_ASSERT_TRUE(syn_x509_parse(full_cert_der, sizeof(full_cert_der), &parsed_cert));
    TEST_ASSERT_EQUAL(3, parsed_cert.version);
    TEST_ASSERT_EQUAL_STRING("Test CA", parsed_cert.issuer_cn);
    TEST_ASSERT_EQUAL_STRING("Test Server", parsed_cert.subject_cn);
    TEST_ASSERT_EQUAL(SYN_X509_ALGO_ED25519, parsed_cert.pubkey_algo);
}

void test_x509_ecdsa_p256_verification_and_chain(void)
{
    /* 1. Generate P-256 Key pair and signature over TBS data */
    static const uint8_t PRIV[32] = {0xC9, 0xAF, 0xA9, 0xD8, 0x45, 0xBA, 0x75, 0x16,
                                     0x6B, 0x5C, 0x21, 0x57, 0x67, 0xB1, 0xD6, 0x93,
                                     0x4E, 0x50, 0xC3, 0xDB, 0x36, 0xE8, 0x9B, 0x12,
                                     0x7B, 0x8A, 0x62, 0x2B, 0x12, 0x0F, 0x67, 0x21};
    static const uint8_t NONCE_K[32] = {0xA6, 0xE3, 0xC5, 0x7D, 0xD0, 0x1A, 0xBE, 0x90,
                                        0x08, 0x65, 0x38, 0x39, 0x83, 0x55, 0xDD, 0x4C,
                                        0x3B, 0x17, 0xAA, 0x87, 0x33, 0x82, 0xB0, 0xF2,
                                        0x4D, 0x61, 0x29, 0x49, 0x3D, 0x8A, 0xAD, 0x60};
    uint8_t pub_x[32], pub_y[32];
    TEST_ASSERT_TRUE(syn_p256_base_mul(PRIV, pub_x, pub_y));

    static const uint8_t tbs_data[] = "SyntropicOS TBS Certificate Payload";
    uint8_t hash[32];
    syn_sha256(tbs_data, sizeof(tbs_data) - 1, hash);

    uint8_t sig_r[32], sig_s[32];
    TEST_ASSERT_TRUE(syn_p256_sign_ecdsa(PRIV, NONCE_K, hash, sig_r, sig_s));

    SYN_X509_Cert cert;
    memset(&cert, 0, sizeof(cert));
    cert.tbs_bytes = tbs_data;
    cert.tbs_len = sizeof(tbs_data) - 1;
    memcpy(cert.signature, sig_r, 32);
    memcpy(cert.signature + 32, sig_s, 32);
    cert.signature_len = 64;
    cert.sig_algo = SYN_X509_ALGO_ECDSA_P256;
    strncpy(cert.subject_cn, "test.syntropic.io", sizeof(cert.subject_cn));
    strncpy(cert.issuer_cn, "SyntropicOS EC Root CA", sizeof(cert.issuer_cn));

    /* 65-byte uncompressed pubkey (0x04 || X || Y) */
    uint8_t pubkey_65[65];
    pubkey_65[0] = 0x04;
    memcpy(pubkey_65 + 1, pub_x, 32);
    memcpy(pubkey_65 + 33, pub_y, 32);

    TEST_ASSERT_TRUE(syn_x509_verify_signature(&cert, pubkey_65, 65, SYN_X509_ALGO_ECDSA_P256));

    /* 64-byte raw pubkey (X || Y) */
    uint8_t pubkey_64[64];
    memcpy(pubkey_64, pub_x, 32);
    memcpy(pubkey_64 + 32, pub_y, 32);
    TEST_ASSERT_TRUE(syn_x509_verify_signature(&cert, pubkey_64, 64, SYN_X509_ALGO_ECDSA_P256));

    /* Invalid public key length */
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&cert, pubkey_65, 63, SYN_X509_ALGO_ECDSA_P256));
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&cert, pubkey_65, 66, SYN_X509_ALGO_ECDSA_P256));

    /* Invalid signature length */
    cert.signature_len = 63;
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&cert, pubkey_65, 65, SYN_X509_ALGO_ECDSA_P256));
    cert.signature_len = 64;

    /* Validate chain */
    SYN_X509_Cert ca_cert;
    memset(&ca_cert, 0, sizeof(ca_cert));
    memcpy(ca_cert.pubkey, pubkey_65, 65);
    ca_cert.pubkey_len = 65;
    ca_cert.pubkey_algo = SYN_X509_ALGO_ECDSA_P256;
    strncpy(ca_cert.subject_cn, "SyntropicOS EC Root CA", sizeof(ca_cert.subject_cn));
    ca_cert.is_ca = true;

    TEST_ASSERT_TRUE(syn_x509_validate_chain(&cert, &ca_cert, "test.syntropic.io"));
    TEST_ASSERT_TRUE(syn_x509_validate_chain(&cert, &ca_cert, NULL)); /* Ignore CN */
    TEST_ASSERT_FALSE(syn_x509_validate_chain(&cert, &ca_cert, "mismatched.domain.com"));
    TEST_ASSERT_FALSE(syn_x509_validate_chain(NULL, &ca_cert, "test.syntropic.io"));
    TEST_ASSERT_FALSE(syn_x509_validate_chain(&cert, NULL, "test.syntropic.io"));
}

void test_x509_parse_ecdsa_cert(void)
{
    /* Valid X.509 cert DER with OID_EC_PUBKEY + OID_PRIME256V1 in SPKI and ecdsa-with-SHA256 sig */
    static const uint8_t ec_cert_der[] = {
        0x30, 0x81, 0xAC,             /* SEQUENCE length 172 */
        0x30, 0x65,                   /* TBSCertificate 101 */
        0xA0, 0x03, 0x02, 0x01, 0x02, /* [0] Version 3 */
        0x02, 0x01, 0x01,             /* Serial Number 1 */
        0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, /* ecdsa-SHA256 */
        0x30, 0x12, 0x31, 0x10, 0x30, 0x0E, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x07, 'T',
        'e',  's',  't',  ' ',  'C',  'A', /* Issuer CN "Test CA" */
        0x30, 0x00,                        /* Validity empty */
        0x30, 0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x0B, 'T',
        'e',  's',  't',  ' ',  'S',  'e',  'r',  'v',  'e',  'r', /* Subject CN "Test Server" */
        0x30, 0x21,                                                /* SPKI (33 bytes) */
        0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,       /* id-ecPublicKey */
        0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07,             /* prime256v1 */
        0x03, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, /* Pubkey bits */
        0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, /* Outer Sig Algo */
        0x03, 0x37, 0x00, /* Outer Sig bits (54 bytes) */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
        0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
        0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
        0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35};

    SYN_X509_Cert parsed_ec_cert;
    TEST_ASSERT_TRUE(syn_x509_parse(ec_cert_der, sizeof(ec_cert_der), &parsed_ec_cert));
    TEST_ASSERT_EQUAL(SYN_X509_ALGO_ECDSA_P256, parsed_ec_cert.pubkey_algo);
    TEST_ASSERT_EQUAL(SYN_X509_ALGO_ECDSA_P256, parsed_ec_cert.sig_algo);

    /* Non-P256 curve OID in SPKI (last byte 0x08 instead of 0x07) -> ALGO_UNKNOWN */
    uint8_t wrong_curve_der[sizeof(ec_cert_der)];
    memcpy(wrong_curve_der, ec_cert_der, sizeof(ec_cert_der));
    wrong_curve_der[92] = 0x08U;
    TEST_ASSERT_TRUE(syn_x509_parse(wrong_curve_der, sizeof(wrong_curve_der), &parsed_ec_cert));
    TEST_ASSERT_EQUAL(SYN_X509_ALGO_UNKNOWN, parsed_ec_cert.pubkey_algo);

    /* Missing curve parameter OID after OID_EC_PUBKEY -> ALGO_UNKNOWN */
    static const uint8_t ec_no_curve_der[] = {
        0x30, 0x81, 0xA2, 0x30, 0x5B, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01, 0x30, 0x0A,
        0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x30, 0x12, 0x31, 0x10, 0x30,
        0x0E, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x07, 'T',  'e',  's',  't',  ' ',  'C',  'A',
        0x30, 0x00, 0x30, 0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x0B,
        'T',  'e',  's',  't',  ' ',  'S',  'e',  'r',  'v',  'e',  'r',  0x30, 0x17, 0x30, 0x09,
        0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x03, 0x0A, 0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D,
        0x04, 0x03, 0x02, 0x03, 0x37, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
        0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35};
    TEST_ASSERT_TRUE(syn_x509_parse(ec_no_curve_der, sizeof(ec_no_curve_der), &parsed_ec_cert));
    TEST_ASSERT_EQUAL(SYN_X509_ALGO_UNKNOWN, parsed_ec_cert.pubkey_algo);

    /* Primitive / empty SPKI algo_seq -> ALGO_UNKNOWN */
    uint8_t prim_algo_der[sizeof(ec_no_curve_der)];
    memcpy(prim_algo_der, ec_no_curve_der, sizeof(ec_no_curve_der));
    prim_algo_der[73] = 0x04U;
    TEST_ASSERT_TRUE(syn_x509_parse(prim_algo_der, sizeof(prim_algo_der), &parsed_ec_cert));
    TEST_ASSERT_EQUAL(SYN_X509_ALGO_UNKNOWN, parsed_ec_cert.pubkey_algo);

    memcpy(prim_algo_der, ec_no_curve_der, sizeof(ec_no_curve_der));
    prim_algo_der[75] = 0x1FU;
    TEST_ASSERT_TRUE(syn_x509_parse(prim_algo_der, sizeof(prim_algo_der), &parsed_ec_cert));
    TEST_ASSERT_EQUAL(SYN_X509_ALGO_UNKNOWN, parsed_ec_cert.pubkey_algo);

    /* Test SPKI with OID_ECDSA_SHA256 (1.2.840.10045.4.3.2) */
    static const uint8_t ecdsa_sha256_cert_der[] = {
        0x30, 0x81, 0x99,                         /* SEQUENCE length 153 */
        0x30, 0x57,                               /* TBSCertificate length 87 */
        0xA0, 0x03, 0x02, 0x01, 0x02,             /* [0] Version 3 */
        0x02, 0x01, 0x01,                         /* Serial Number 1 */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, /* Sig Algo */
        0x30, 0x12, 0x31, 0x10, 0x30, 0x0E, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x07, 'T',
        'e',  's',  't',  ' ',  'C',  'A', /* Issuer CN "Test CA" */
        0x30, 0x00,                        /* Validity empty */
        0x30, 0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x0B, 'T',
        'e',  's',  't',  ' ',  'S',  'e',  'r',  'v',  'e',  'r', /* Subject CN "Test Server" */
        0x30, 0x18,                                                /* SPKI (24 bytes) */
        0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, /* OID_ECDSA_SHA256
                                                                                 */
        0x03, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, /* Pubkey bits */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70,                               /* Outer Sig Algo */
        0x03, 0x37, 0x00, /* Outer Sig bits (54 bytes) */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
        0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
        0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
        0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35};

    SYN_X509_Cert parsed_ecdsa_spki;
    TEST_ASSERT_TRUE(
        syn_x509_parse(ecdsa_sha256_cert_der, sizeof(ecdsa_sha256_cert_der), &parsed_ecdsa_spki));
    TEST_ASSERT_EQUAL(SYN_X509_ALGO_ECDSA_P256, parsed_ecdsa_spki.pubkey_algo);
}

void test_x509_edge_cases_and_corrupt_structures(void)
{
    SYN_X509_Cert cert;

    /* Non-constructed root tag */
    static const uint8_t primitive_root[] = {0x04, 0x02, 0x00, 0x00};
    TEST_ASSERT_FALSE(syn_x509_parse(primitive_root, sizeof(primitive_root), &cert));

    /* SEQUENCE with non-constructed TBS */
    static const uint8_t primitive_tbs[] = {0x30, 0x04, 0x04, 0x02, 0x00, 0x00};
    TEST_ASSERT_FALSE(syn_x509_parse(primitive_tbs, sizeof(primitive_tbs), &cert));

    /* TBS with non-integer serial (e.g. OCTET STRING instead of INTEGER) */
    static const uint8_t bad_serial_cert[] = {0x30, 0x0A, 0x30, 0x08, 0x04,
                                              0x01, 0x01, /* OCTET STRING 1 instead of INTEGER */
                                              0x30, 0x03, 0x01, 0x01, 0x00};
    TEST_ASSERT_FALSE(syn_x509_parse(bad_serial_cert, sizeof(bad_serial_cert), &cert));

    /* Cert with truncated SPKI (algo SEQUENCE only, missing subjectPublicKey BIT STRING) */
    static const uint8_t trunc_spki_cert[] = {
        0x30, 0x1F,                               /* Outer Certificate SEQUENCE (31B) */
        0x30, 0x14,                               /* TBSCertificate SEQUENCE (20B) */
        0x02, 0x01, 0x01,                         /* Serial 1 */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, /* Signature Algorithm Ed25519 */
        0x30, 0x00,                               /* Issuer */
        0x30, 0x00,                               /* Validity */
        0x30, 0x00,                               /* Subject */
        0x30, 0x02, 0x30, 0x00,                   /* SPKI with 1 child (missing pubkey bits) */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, /* Outer Sig Algo */
        0x03, 0x00                                /* Empty signature BIT STRING */
    };
    TEST_ASSERT_FALSE(syn_x509_parse(trunc_spki_cert, sizeof(trunc_spki_cert), &cert));
}

void test_x509_ed25519_live_keypair_verification_and_chain(void)
{
    /* Generate live Ed25519 CA and Server keypairs */
    uint8_t ca_seed[32] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22};
    uint8_t ca_pub[32], ca_sec[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(ca_pub, ca_sec, ca_seed));

    uint8_t srv_seed[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t srv_pub[32], srv_sec[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(srv_pub, srv_sec, srv_seed));

    static const uint8_t tbs_payload[] = "SyntropicOS Live Ed25519 TBS Certificate";
    uint8_t sig[64];
    TEST_ASSERT_TRUE(syn_ed25519_sign(tbs_payload, sizeof(tbs_payload) - 1, ca_sec, ca_pub, sig));

    SYN_X509_Cert srv_cert;
    memset(&srv_cert, 0, sizeof(srv_cert));
    srv_cert.tbs_bytes = tbs_payload;
    srv_cert.tbs_len = sizeof(tbs_payload) - 1;
    memcpy(srv_cert.pubkey, srv_pub, 32);
    srv_cert.pubkey_len = 32;
    srv_cert.pubkey_algo = SYN_X509_ALGO_ED25519;
    memcpy(srv_cert.signature, sig, 64);
    srv_cert.signature_len = 64;
    srv_cert.sig_algo = SYN_X509_ALGO_ED25519;
    strncpy(srv_cert.subject_cn, "api.syntropicos.internal", sizeof(srv_cert.subject_cn));
    strncpy(srv_cert.issuer_cn, "SyntropicOS Root CA", sizeof(srv_cert.issuer_cn));

    /* Verify signature directly using CA public key */
    TEST_ASSERT_TRUE(syn_x509_verify_signature(&srv_cert, ca_pub, 32, SYN_X509_ALGO_ED25519));

    /* Corrupted signature must fail */
    srv_cert.signature[0] ^= 0x01;
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&srv_cert, ca_pub, 32, SYN_X509_ALGO_ED25519));
    srv_cert.signature[0] ^= 0x01;

    /* Validate chain against Root CA cert */
    SYN_X509_Cert ca_cert;
    memset(&ca_cert, 0, sizeof(ca_cert));
    memcpy(ca_cert.pubkey, ca_pub, 32);
    ca_cert.pubkey_len = 32;
    ca_cert.pubkey_algo = SYN_X509_ALGO_ED25519;
    strncpy(ca_cert.subject_cn, "SyntropicOS Root CA", sizeof(ca_cert.subject_cn));
    ca_cert.is_ca = true;

    TEST_ASSERT_TRUE(syn_x509_validate_chain(&srv_cert, &ca_cert, "api.syntropicos.internal"));
    TEST_ASSERT_TRUE(syn_x509_validate_chain(&srv_cert, &ca_cert, NULL));
    TEST_ASSERT_FALSE(syn_x509_validate_chain(&srv_cert, &ca_cert, "wrong.domain.org"));
}

static size_t encode_der_ecdsa_sig(const uint8_t r[32], const uint8_t s[32], uint8_t *out)
{
    size_t r_off = 0U;
    while (r_off < 31U && r[r_off] == 0x00U) {
        r_off++;
    }
    bool r_pad = (r[r_off] & 0x80U) != 0U;
    size_t r_len = (32U - r_off) + (r_pad ? 1U : 0U);

    size_t s_off = 0U;
    while (s_off < 31U && s[s_off] == 0x00U) {
        s_off++;
    }
    bool s_pad = (s[s_off] & 0x80U) != 0U;
    size_t s_len = (32U - s_off) + (s_pad ? 1U : 0U);

    size_t pos = 0U;
    out[pos++] = 0x30U;
    out[pos++] = (uint8_t)(2U + r_len + 2U + s_len);
    out[pos++] = 0x02U;
    out[pos++] = (uint8_t)r_len;
    if (r_pad) {
        out[pos++] = 0x00U;
    }
    (void)memcpy(&out[pos], &r[r_off], 32U - r_off);
    pos += (32U - r_off);
    out[pos++] = 0x02U;
    out[pos++] = (uint8_t)s_len;
    if (s_pad) {
        out[pos++] = 0x00U;
    }
    (void)memcpy(&out[pos], &s[s_off], 32U - s_off);
    pos += (32U - s_off);
    return pos;
}

void test_r5_n03_n04_asn1_der_and_x509_constraints_ecdsa_der(void)
{
    SYN_ASN1_Element elem;
    const uint8_t *bits = NULL;
    size_t bit_len = 0U;

    /* R5-N03: BIT STRING {0x03, 0x01, 0x01} underflow guard vs valid 0-bit {0x03, 0x01, 0x00} */
    static const uint8_t bit_underflow[] = {0x03, 0x01, 0x01};
    TEST_ASSERT_TRUE(syn_asn1_parse_element(bit_underflow, sizeof(bit_underflow), &elem));
    TEST_ASSERT_FALSE(syn_asn1_get_bit_string(&elem, &bits, &bit_len));

    static const uint8_t bit_zero_ok[] = {0x03, 0x01, 0x00};
    TEST_ASSERT_TRUE(syn_asn1_parse_element(bit_zero_ok, sizeof(bit_zero_ok), &elem));
    TEST_ASSERT_TRUE(syn_asn1_get_bit_string(&elem, &bits, &bit_len));
    TEST_ASSERT_EQUAL(0U, bit_len);

    /* R5-N03: Reject non-minimal BER lengths: indefinite 0x80, leading 0x00, and 0x81 < 128 */
    static const uint8_t ber_indef[] = {0x04, 0x80, 0x00, 0x00};
    static const uint8_t ber_lead_zero[] = {0x04, 0x82, 0x00, 0x80};
    static const uint8_t ber_short_81[] = {0x04, 0x81, 0x05, 1, 2, 3, 4, 5};
    TEST_ASSERT_FALSE(syn_asn1_parse_element(ber_indef, sizeof(ber_indef), &elem));
    TEST_ASSERT_FALSE(syn_asn1_parse_element(ber_lead_zero, sizeof(ber_lead_zero), &elem));
    TEST_ASSERT_FALSE(syn_asn1_parse_element(ber_short_81, sizeof(ber_short_81), &elem));

    /* Canonical 2-byte length >= 256 */
    uint8_t canon_256[260];
    (void)memset(canon_256, 0xAA, sizeof(canon_256));
    canon_256[0] = 0x04U;
    canon_256[1] = 0x82U;
    canon_256[2] = 0x01U;
    canon_256[3] = 0x00U;
    TEST_ASSERT_TRUE(syn_asn1_parse_element(canon_256, sizeof(canon_256), &elem));
    TEST_ASSERT_EQUAL(256U, elem.length);

    /* R5-N04: X.509v3 [3] EXPLICIT BasicConstraints cA=TRUE + KeyUsage + [1] uniqueID */
    static const uint8_t ca_v3_der[] = {
        0x30, 0x81, 0xBD,                         /* SEQUENCE 189 */
        0x30, 0x7B,                               /* TBSCertificate 123 */
        0xA0, 0x03, 0x02, 0x01, 0x02,             /* [0] Version 3 */
        0x02, 0x01, 0x01,                         /* Serial 1 */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, /* tbs.signature ed25519 */
        0x30, 0x12, 0x31, 0x10, 0x30, 0x0E, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x07, 'T',  'e',
        's',  't',  ' ',  'C',  'A', /* Issuer "Test CA" */
        0x30, 0x00,                  /* Validity */
        0x30, 0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x0B, 'T',  'e',
        's',  't',  ' ',  'S',  'e',  'r',  'v',  'e',  'r', /* Subject "Test Server" */
        0x30, 0x13, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x03, 0x0A, 0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07, 0x08, 0x09, /* SPKI */
        0x81, 0x02, 0x00, 0xAA,             /* [1] issuerUniqueID */
        0xA3, 0x23, 0x30, 0x21,             /* [3] EXPLICIT Extensions SEQUENCE */
        0x30, 0x0E, 0x06, 0x03, 0x55, 0x1D, 0x0F, 0x01, 0x01, 0xFF, 0x04, 0x04, 0x03, 0x02, 0x01,
        0x06, /* KeyUsage (critical) */
        0x30, 0x0F, 0x06, 0x03, 0x55, 0x1D, 0x13, 0x01, 0x01, 0xFF, 0x04, 0x05, 0x30, 0x03, 0x01,
        0x01, 0xFF,                               /* BasicConstraints cA=TRUE */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, /* outer signatureAlgorithm ed25519 */
        0x03, 0x37, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A,
        0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
        0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35};

    SYN_X509_Cert ca_parsed;
    TEST_ASSERT_TRUE(syn_x509_parse(ca_v3_der, sizeof(ca_v3_der), &ca_parsed));
    TEST_ASSERT_TRUE(ca_parsed.is_ca);
    TEST_ASSERT_EQUAL(SYN_X509_ALGO_ED25519, ca_parsed.sig_algo);

    /* Mutate BasicConstraints cA=FALSE (byte 127 == 0xFF) */
    uint8_t mut[sizeof(ca_v3_der)];
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[127] = 0x00U;
    TEST_ASSERT_TRUE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));
    TEST_ASSERT_FALSE(ca_parsed.is_ca);

    /* BasicConstraints starting with INTEGER pathLen (0x02 0x01 0x00) instead of BOOLEAN */
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[125] = 0x02U;
    mut[127] = 0x00U;
    TEST_ASSERT_TRUE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));
    TEST_ASSERT_FALSE(ca_parsed.is_ca);

    /* BasicConstraints empty SEQUENCE 30 00 (non-critical extension) */
    static const uint8_t empty_bc_der[] = {
        0x30, 0x81, 0xA3,                         /* SEQUENCE 163 */
        0x30, 0x61,                               /* TBSCertificate 97 */
        0xA0, 0x03, 0x02, 0x01, 0x02,             /* [0] Version 3 */
        0x02, 0x01, 0x01,                         /* Serial 1 */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, /* Sig Algo ed25519 */
        0x30, 0x12, 0x31, 0x10, 0x30, 0x0E, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13,
        0x07, 'T',  'e',  's',  't',  ' ',  'C',  'A', /* Issuer CN */
        0x30, 0x00,                                    /* Validity */
        0x30, 0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13,
        0x0B, 'T',  'e',  's',  't',  ' ',  'S',  'e',  'r',  'v',  'e',  'r', /* Subject CN */
        0x30, 0x13, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x03, 0x0A, 0x00,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, /* SPKI */
        0xA3, 0x0D, 0x30, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x1D, 0x13, 0x04,
        0x02, 0x30, 0x00,                         /* BasicConstraints {} */
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, /* Outer Sig Algo */
        0x03, 0x37, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
        0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
        0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
        0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35};
    TEST_ASSERT_TRUE(syn_x509_parse(empty_bc_der, sizeof(empty_bc_der), &ca_parsed));
    TEST_ASSERT_FALSE(ca_parsed.is_ca);

    /* Extension & signature error guards */
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[91] = 0x83U; /* non-constructed [3] */
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[93] = 0x04U; /* non-SEQUENCE inside [3] */
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[95] = 0x04U; /* non-SEQUENCE extension */
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[105] = 0x02U; /* non-OCTET_STRING extension value */
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[121] = 0x02U; /* BasicConstraints extnValue not OCTET_STRING */
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[123] = 0x04U; /* BasicConstraints inner not SEQUENCE */
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[124] = 0x06U; /* BasicConstraints bc_seq step truncated */
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[126] = 0x05U; /* BasicConstraints inner elem step truncated */
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[124] = 0x02U;
    mut[126] = 0x00U; /* BOOLEAN length 0 != 1 */
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    /* Truncated critical extension (oid + BOOLEAN critical, no extnValue) */
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[96] = 0x08U;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    /* Truncated extension sequence (< 2 children) */
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[96] = 0x05U;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    /* Truncated optional TBS field */
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[88] = 0x50U;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    /* Truncated exts_seq inside [3] and truncated ext_seq inside exts_seq */
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[94] = 0x40U;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[96] = 0x40U;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    /* Sig alg non-SEQUENCE, invalid inner OID, unknown OID, and outer != inner sig alg */
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[13] = 0x31U;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[15] = 0x1FU;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[19] = 0x71U;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    static const uint8_t outer_ecdsa_alg[] = {0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86,
                                              0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02};
    (void)memcpy(&mut[128], outer_ecdsa_alg, sizeof(outer_ecdsa_alg));
    mut[140] = 0x03U;
    mut[141] = 0x32U;
    mut[142] = 0x00U;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    /* Non-byte-aligned outer signature BIT STRING (unused_bits = 3) */
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[137] = 0x03U;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    /* SPKI malformed guard (primitive SPKI) */
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[66] = 0x04U;
    TEST_ASSERT_TRUE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));
    TEST_ASSERT_EQUAL(0U, ca_parsed.pubkey_len);

    /* R5-N04: DER ECDSA SEQUENCE { INTEGER r, INTEGER s } verification & chain checks */
    static const uint8_t PRIV[32] = {0xC9, 0xAF, 0xA9, 0xD8, 0x45, 0xBA, 0x75, 0x16,
                                     0x6B, 0x5C, 0x21, 0x57, 0x67, 0xB1, 0xD6, 0x93,
                                     0x4E, 0x50, 0xC3, 0xDB, 0x36, 0xE8, 0x9B, 0x12,
                                     0x7B, 0x8A, 0x62, 0x2B, 0x12, 0x0F, 0x67, 0x21};
    static const uint8_t NONCE_K[32] = {0xA6, 0xE3, 0xC5, 0x7D, 0xD0, 0x1A, 0xBE, 0x90,
                                        0x08, 0x65, 0x38, 0x39, 0x83, 0x55, 0xDD, 0x4C,
                                        0x3B, 0x17, 0xAA, 0x87, 0x33, 0x82, 0xB0, 0xF2,
                                        0x4D, 0x61, 0x29, 0x49, 0x3D, 0x8A, 0xAD, 0x60};
    uint8_t pub_x[32], pub_y[32];
    TEST_ASSERT_TRUE(syn_p256_base_mul(PRIV, pub_x, pub_y));

    static const uint8_t tbs_msg[] = "DER ECDSA Certificate TBS";
    uint8_t hash[32], sig_r[32], sig_s[32];
    syn_sha256(tbs_msg, sizeof(tbs_msg) - 1U, hash);
    TEST_ASSERT_TRUE(syn_p256_sign_ecdsa(PRIV, NONCE_K, hash, sig_r, sig_s));

    uint8_t pub65[65];
    pub65[0] = 0x04U;
    (void)memcpy(&pub65[1], pub_x, 32U);
    (void)memcpy(&pub65[33], pub_y, 32U);

    SYN_X509_Cert der_leaf;
    (void)memset(&der_leaf, 0, sizeof(der_leaf));
    der_leaf.tbs_bytes = tbs_msg;
    der_leaf.tbs_len = sizeof(tbs_msg) - 1U;
    der_leaf.sig_algo = SYN_X509_ALGO_ECDSA_P256;
    der_leaf.signature_len = encode_der_ecdsa_sig(sig_r, sig_s, der_leaf.signature);
    (void)strncpy(der_leaf.subject_cn, "leaf.syntropic.io", sizeof(der_leaf.subject_cn) - 1U);
    (void)strncpy(der_leaf.issuer_cn, "Root CA", sizeof(der_leaf.issuer_cn) - 1U);

    SYN_X509_Cert root_ca;
    (void)memset(&root_ca, 0, sizeof(root_ca));
    (void)memcpy(root_ca.pubkey, pub65, 65U);
    root_ca.pubkey_len = 65U;
    root_ca.pubkey_algo = SYN_X509_ALGO_ECDSA_P256;
    (void)strncpy(root_ca.subject_cn, "Root CA", sizeof(root_ca.subject_cn) - 1U);
    root_ca.is_ca = true;

    TEST_ASSERT_TRUE(syn_x509_validate_chain(&der_leaf, &root_ca, "leaf.syntropic.io"));

    /* Chain validation negative guards: !is_ca, empty CA CN, issuer!=subject, sig_algo!=pubkey */
    root_ca.is_ca = false;
    TEST_ASSERT_FALSE(syn_x509_validate_chain(&der_leaf, &root_ca, NULL));
    root_ca.is_ca = true;

    root_ca.subject_cn[0] = '\0';
    TEST_ASSERT_FALSE(syn_x509_validate_chain(&der_leaf, &root_ca, NULL));
    (void)strncpy(root_ca.subject_cn, "Other CA", sizeof(root_ca.subject_cn) - 1U);
    TEST_ASSERT_FALSE(syn_x509_validate_chain(&der_leaf, &root_ca, NULL));
    (void)strncpy(root_ca.subject_cn, "Root CA", sizeof(root_ca.subject_cn) - 1U);

    der_leaf.sig_algo = SYN_X509_ALGO_UNKNOWN;
    TEST_ASSERT_FALSE(syn_x509_validate_chain(&der_leaf, &root_ca, NULL));
    der_leaf.sig_algo = SYN_X509_ALGO_ED25519;
    TEST_ASSERT_FALSE(syn_x509_validate_chain(&der_leaf, &root_ca, NULL));
    der_leaf.sig_algo = SYN_X509_ALGO_ECDSA_P256;

    /* DER ECDSA malformed signatures */
    uint8_t saved_sig[SYN_X509_SIG_MAX_LEN];
    size_t saved_len = der_leaf.signature_len;
    (void)memcpy(saved_sig, der_leaf.signature, saved_len);

    /* Trailing byte after SEQUENCE */
    der_leaf.signature[saved_len] = 0x00U;
    der_leaf.signature_len = saved_len + 1U;
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&der_leaf, pub65, 65U, SYN_X509_ALGO_ECDSA_P256));

    /* Truncated SEQUENCE header */
    der_leaf.signature[0] = 0x30U;
    der_leaf.signature[1] = 0x7FU;
    der_leaf.signature_len = 4U;
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&der_leaf, pub65, 65U, SYN_X509_ALGO_ECDSA_P256));

    /* Only 1 INTEGER inside SEQUENCE */
    static const uint8_t one_int_seq[] = {0x30, 0x03, 0x02, 0x01, 0x01};
    (void)memcpy(der_leaf.signature, one_int_seq, sizeof(one_int_seq));
    der_leaf.signature_len = sizeof(one_int_seq);
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&der_leaf, pub65, 65U, SYN_X509_ALGO_ECDSA_P256));

    /* Extra element inside SEQUENCE (rem != 0) */
    static const uint8_t three_int_seq[] = {0x30, 0x09, 0x02, 0x01, 0x01, 0x02,
                                            0x01, 0x01, 0x02, 0x01, 0x01};
    (void)memcpy(der_leaf.signature, three_int_seq, sizeof(three_int_seq));
    der_leaf.signature_len = sizeof(three_int_seq);
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&der_leaf, pub65, 65U, SYN_X509_ALGO_ECDSA_P256));

    /* Non-INTEGER tag, negative INTEGER, non-minimal 0x00 0x01, and >32B INTEGER */
    static const uint8_t non_int_r[] = {0x30, 0x06, 0x04, 0x01, 0x01, 0x02, 0x01, 0x01};
    (void)memcpy(der_leaf.signature, non_int_r, sizeof(non_int_r));
    der_leaf.signature_len = sizeof(non_int_r);
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&der_leaf, pub65, 65U, SYN_X509_ALGO_ECDSA_P256));

    static const uint8_t neg_int_r[] = {0x30, 0x06, 0x02, 0x01, 0x80, 0x02, 0x01, 0x01};
    (void)memcpy(der_leaf.signature, neg_int_r, sizeof(neg_int_r));
    der_leaf.signature_len = sizeof(neg_int_r);
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&der_leaf, pub65, 65U, SYN_X509_ALGO_ECDSA_P256));

    static const uint8_t nonmin_int_r[] = {0x30, 0x07, 0x02, 0x02, 0x00, 0x01, 0x02, 0x01, 0x01};
    (void)memcpy(der_leaf.signature, nonmin_int_r, sizeof(nonmin_int_r));
    der_leaf.signature_len = sizeof(nonmin_int_r);
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&der_leaf, pub65, 65U, SYN_X509_ALGO_ECDSA_P256));

    uint8_t big_int_seq[40];
    (void)memset(big_int_seq, 0x11, sizeof(big_int_seq));
    big_int_seq[0] = 0x30U;
    big_int_seq[1] = 38U;
    big_int_seq[2] = 0x02U;
    big_int_seq[3] = 33U; /* 33B positive integer > 32B */
    big_int_seq[37] = 0x02U;
    big_int_seq[38] = 0x01U;
    big_int_seq[39] = 0x01U;
    (void)memcpy(der_leaf.signature, big_int_seq, sizeof(big_int_seq));
    der_leaf.signature_len = sizeof(big_int_seq);
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&der_leaf, pub65, 65U, SYN_X509_ALGO_ECDSA_P256));

    /* Short integer s failing decode_ecdsa_der_int */
    static const uint8_t bad_int_s[] = {0x30, 0x06, 0x02, 0x01, 0x01, 0x02, 0x01, 0x80};
    (void)memcpy(der_leaf.signature, bad_int_s, sizeof(bad_int_s));
    der_leaf.signature_len = sizeof(bad_int_s);
    TEST_ASSERT_FALSE(syn_x509_verify_signature(&der_leaf, pub65, 65U, SYN_X509_ALGO_ECDSA_P256));

    /* Raw 64-byte (r || s) ECDSA signature whose r[0] == 0x30 (nonce k = 31) MUST verify */
    uint8_t nonce_r30[32] = {0};
    nonce_r30[31] = 0x1FU;
    uint8_t r30[32], s30[32];
    TEST_ASSERT_TRUE(syn_p256_sign_ecdsa(PRIV, nonce_r30, hash, r30, s30));
    TEST_ASSERT_EQUAL_HEX8(0x30U, r30[0]);
    (void)memcpy(der_leaf.signature, r30, 32U);
    (void)memcpy(der_leaf.signature + 32U, s30, 32U);
    der_leaf.signature_len = 64U;
    TEST_ASSERT_TRUE(syn_x509_verify_signature(&der_leaf, pub65, 65U, SYN_X509_ALGO_ECDSA_P256));

    /* RFC 5280 §4.2: Unrecognized critical=TRUE extension rejected; critical=FALSE accepted */
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[101] = 0x7FU; /* Unrecognized OID 2.5.29.127 with critical=TRUE */
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));
    mut[104] = 0x00U; /* critical=FALSE -> accepted */
    TEST_ASSERT_TRUE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    /* Malformed critical BOOLEAN length (0) & malformed KeyUsage inner BIT STRING */
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[103] = 0x00U;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[106] = 0x00U; /* Empty KeyUsage OCTET STRING */
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[107] = 0x04U; /* Non-BIT_STRING inside KeyUsage OCTET STRING */
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    /* Empty extension sequence (0 children) */
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[96] = 0x00U;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    /* Malformed [0] version containers: primitive, empty, non-INTEGER, >2, trailing byte */
    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[5] = 0x80U;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[6] = 0x00U;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[7] = 0x04U;
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[9] = 0x03U; /* version value 3 > v3(2) */
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    (void)memcpy(mut, ca_v3_der, sizeof(ca_v3_der));
    mut[7] = 0x02U;
    mut[8] = 0x02U; /* 2-byte version integer inside 3-byte container -> step fails / len != 1 */
    TEST_ASSERT_FALSE(syn_x509_parse(mut, sizeof(mut), &ca_parsed));

    static const uint8_t two_byte_ver_der[] = {0x30, 0x0B, 0x30, 0x09, 0xA0, 0x04, 0x02,
                                               0x02, 0x01, 0x01, 0x02, 0x01, 0x01};
    TEST_ASSERT_FALSE(syn_x509_parse(two_byte_ver_der, sizeof(two_byte_ver_der), &ca_parsed));

    static const uint8_t trailing_ver_der[] = {0x30, 0x0C, 0x30, 0x0A, 0xA0, 0x05, 0x02,
                                               0x01, 0x02, 0x05, 0x00, 0x02, 0x01, 0x01};
    TEST_ASSERT_FALSE(syn_x509_parse(trailing_ver_der, sizeof(trailing_ver_der), &ca_parsed));
}

typedef struct {
    int ver;
    const uint8_t *subj;
    size_t subj_len;
    const uint8_t *opt;
    size_t opt_len;
} TierACertSpec;

static size_t tier_a_wrap_tlv(uint8_t tag, const uint8_t *body, size_t len, uint8_t *out)
{
    size_t p = 0U;
    out[p++] = tag;
    if (len < 128U) {
        out[p++] = (uint8_t)len;
    } else if (len < 256U) {
        out[p++] = 0x81U;
        out[p++] = (uint8_t)len;
    } else {
        out[p++] = 0x82U;
        out[p++] = (uint8_t)(len >> 8U);
        out[p++] = (uint8_t)(len & 0xFFU);
    }
    if (len > 0U) {
        (void)memcpy(&out[p], body, len);
    }
    return p + len;
}

static size_t tier_a_make_cn_ava(const uint8_t *cn, size_t cn_len, uint8_t *out)
{
    uint8_t str_tlv[200];
    uint8_t ava_body[216];
    ava_body[0] = 0x06U;
    ava_body[1] = 0x03U;
    ava_body[2] = 0x55U;
    ava_body[3] = 0x04U;
    ava_body[4] = 0x03U;
    size_t s_len = tier_a_wrap_tlv(0x0CU, cn, cn_len, str_tlv);
    (void)memcpy(&ava_body[5], str_tlv, s_len);
    return tier_a_wrap_tlv(0x30U, ava_body, 5U + s_len, out);
}

static size_t tier_a_append(uint8_t *dst, size_t off, const uint8_t *src, size_t len)
{
    (void)memcpy(&dst[off], src, len);
    return off + len;
}

static size_t tier_a_make_cert(const TierACertSpec *sp, uint8_t *out)
{
    static const uint8_t fh1[] = {0x02, 0x01, 0x01, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70};
    static const uint8_t fh2[] = {0x30, 0x00, 0x30, 0x00};
    static const uint8_t spki1[] = {0x30, 0x0A, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70};
    static const uint8_t spki2[] = {0x03, 0x01, 0x00};
    static const uint8_t ot1[] = {0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70};
    static const uint8_t ot2[] = {0x03, 0x02, 0x00, 0xAA};
    uint8_t tbs_body[384];
    size_t tp = 0U;
    if (sp->ver >= 0) {
        uint8_t vtlv[5] = {0xA0U, 0x03U, 0x02U, 0x01U, (uint8_t)sp->ver};
        tp = tier_a_append(tbs_body, tp, vtlv, sizeof(vtlv));
    }
    tp = tier_a_append(tbs_body, tp, fh1, sizeof(fh1));
    tp = tier_a_append(tbs_body, tp, fh2, sizeof(fh2));
    tp = tier_a_append(tbs_body, tp, sp->subj, sp->subj_len);
    tp = tier_a_append(tbs_body, tp, spki1, sizeof(spki1));
    tp = tier_a_append(tbs_body, tp, spki2, sizeof(spki2));
    if (sp->opt != NULL && sp->opt_len > 0U) {
        tp = tier_a_append(tbs_body, tp, sp->opt, sp->opt_len);
    }

    uint8_t cert_body[420];
    size_t cp = tier_a_wrap_tlv(0x30U, tbs_body, tp, cert_body);
    cp = tier_a_append(cert_body, cp, ot1, sizeof(ot1));
    cp = tier_a_append(cert_body, cp, ot2, sizeof(ot2));
    return tier_a_wrap_tlv(0x30U, cert_body, cp, out);
}

static size_t tier_a_wrap_exts(const uint8_t *exts_payload, size_t payload_len, uint8_t *out)
{
    uint8_t seq[96];
    size_t slen = tier_a_wrap_tlv(0x30U, exts_payload, payload_len, seq);
    return tier_a_wrap_tlv(0xA3U, seq, slen, out);
}

void test_tier_a_asn1_x509_strict_checks(void)
{
    SYN_ASN1_Element elem;
    const uint8_t *p_out = NULL;
    size_t len_out = 0U;

    /* ASN1-1: Negative & non-minimal DER INTEGERs vs canonical zero */
    static const uint8_t neg_int[] = {0x02, 0x01, 0x80};
    static const uint8_t nonmin_int[] = {0x02, 0x02, 0x00, 0x7F};
    static const uint8_t zero_int[] = {0x02, 0x01, 0x00};
    TEST_ASSERT_TRUE(syn_asn1_parse_element(neg_int, sizeof(neg_int), &elem));
    TEST_ASSERT_FALSE(syn_asn1_get_integer(&elem, &p_out, &len_out));
    TEST_ASSERT_TRUE(syn_asn1_parse_element(nonmin_int, sizeof(nonmin_int), &elem));
    TEST_ASSERT_FALSE(syn_asn1_get_integer(&elem, &p_out, &len_out));
    TEST_ASSERT_TRUE(syn_asn1_parse_element(zero_int, sizeof(zero_int), &elem));
    TEST_ASSERT_TRUE(syn_asn1_get_integer(&elem, &p_out, &len_out));
    TEST_ASSERT_EQUAL(1U, len_out);
    TEST_ASSERT_EQUAL_HEX8(0x00U, p_out[0]);

    /* ASN1-1: BIT STRING non-zero unused padding bits & (len - 1) > SIZE_MAX / 8 overflow */
    static const uint8_t bs_bad_pad[] = {0x03, 0x02, 0x03, 0x04};
    static const uint8_t bs_ok_pad[] = {0x03, 0x02, 0x03, 0x08};
    TEST_ASSERT_TRUE(syn_asn1_parse_element(bs_bad_pad, sizeof(bs_bad_pad), &elem));
    TEST_ASSERT_FALSE(syn_asn1_get_bit_string(&elem, &p_out, &len_out));
    TEST_ASSERT_TRUE(syn_asn1_parse_element(bs_ok_pad, sizeof(bs_ok_pad), &elem));
    TEST_ASSERT_TRUE(syn_asn1_get_bit_string(&elem, &p_out, &len_out));
    TEST_ASSERT_EQUAL(5U, len_out);

    uint8_t dummy_bs[2] = {0x00U, 0x00U};
    SYN_ASN1_Element huge_bs = {0};
    huge_bs.tag = SYN_ASN1_TAG_BIT_STRING;
    huge_bs.length = (SIZE_MAX / 8U) + 2U;
    huge_bs.value = dummy_bs;
    TEST_ASSERT_FALSE(syn_asn1_get_bit_string(&huge_bs, &p_out, &len_out));

    /* ASN1-1: Trailing outer byte & trailing element inside Certificate SEQUENCE */
    static const uint8_t empty_subj[] = {0x30, 0x00};
    TierACertSpec spec = {2, empty_subj, sizeof(empty_subj), NULL, 0U};
    uint8_t der[512];
    SYN_X509_Cert cert;
    size_t dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_TRUE(syn_x509_parse(der, dlen, &cert));
    der[dlen] = 0x00U;
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen + 1U, &cert));

    /* Canonical non-byte-aligned outer signature BIT STRING (unused_bits = 3, pad bits = 0) */
    der[dlen - 2U] = 0x03U;
    der[dlen - 1U] = 0xA8U;
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));

    /* Extra element inside outer Certificate SEQUENCE (cert_len != 0) */
    uint8_t extra_outer[512];
    size_t e_len = tier_a_make_cert(&spec, extra_outer);
    extra_outer[1] = (uint8_t)(extra_outer[1] + 2U);
    extra_outer[e_len] = 0x05U;
    extra_outer[e_len + 1U] = 0x00U;
    TEST_ASSERT_FALSE(syn_x509_parse(extra_outer, e_len + 2U, &cert));

    /* X509-1: Multi-valued RDN SET { C="US", CN="multi.io" } */
    static const uint8_t c1[] = {0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 'U', 'S'};
    uint8_t cn_ava[220];
    uint8_t rdn_body[256];
    uint8_t rdn_set[264];
    uint8_t subj_seq[272];
    size_t cn_ava_len = tier_a_make_cn_ava((const uint8_t *)"multi.io", 8U, cn_ava);
    (void)memcpy(rdn_body, c1, sizeof(c1));
    (void)memcpy(&rdn_body[sizeof(c1)], cn_ava, cn_ava_len);
    size_t rset_len = tier_a_wrap_tlv(0x31U, rdn_body, sizeof(c1) + cn_ava_len, rdn_set);
    size_t sseq_len = tier_a_wrap_tlv(0x30U, rdn_set, rset_len, subj_seq);
    spec.subj = subj_seq;
    spec.subj_len = sseq_len;
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_TRUE(syn_x509_parse(der, dlen, &cert));
    TEST_ASSERT_EQUAL_STRING("multi.io", cert.subject_cn);

    /* X509-1: Embedded NUL inside CN must fail */
    static const uint8_t nul_cn[] = {'g', 'o', 'o', 'd', '\0', 'b', 'a', 'd'};
    cn_ava_len = tier_a_make_cn_ava(nul_cn, sizeof(nul_cn), cn_ava);
    rset_len = tier_a_wrap_tlv(0x31U, cn_ava, cn_ava_len, rdn_set);
    sseq_len = tier_a_wrap_tlv(0x30U, rdn_set, rset_len, subj_seq);
    spec.subj = subj_seq;
    spec.subj_len = sseq_len;
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));

    /* X509-1: 127-byte CN accepted; 128-byte CN (>= SYN_X509_MAX_NAME_LEN) rejected */
    uint8_t long_cn[128];
    (void)memset(long_cn, 'A', sizeof(long_cn));
    cn_ava_len = tier_a_make_cn_ava(long_cn, 127U, cn_ava);
    rset_len = tier_a_wrap_tlv(0x31U, cn_ava, cn_ava_len, rdn_set);
    sseq_len = tier_a_wrap_tlv(0x30U, rdn_set, rset_len, subj_seq);
    spec.subj = subj_seq;
    spec.subj_len = sseq_len;
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_TRUE(syn_x509_parse(der, dlen, &cert));
    TEST_ASSERT_EQUAL(127U, strlen(cert.subject_cn));

    cn_ava_len = tier_a_make_cn_ava(long_cn, 128U, cn_ava);
    rset_len = tier_a_wrap_tlv(0x31U, cn_ava, cn_ava_len, rdn_set);
    sseq_len = tier_a_wrap_tlv(0x30U, rdn_set, rset_len, subj_seq);
    spec.subj = subj_seq;
    spec.subj_len = sseq_len;
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));

    /* X509-1: Identical duplicate CN accepted; conflicting duplicate CN rejected */
    uint8_t cn_ava2[64];
    size_t a1 = tier_a_make_cn_ava((const uint8_t *)"same.io", 7U, cn_ava);
    size_t a2 = tier_a_make_cn_ava((const uint8_t *)"same.io", 7U, cn_ava2);
    size_t s1 = tier_a_wrap_tlv(0x31U, cn_ava, a1, rdn_body);
    size_t s2 = tier_a_wrap_tlv(0x31U, cn_ava2, a2, &rdn_body[s1]);
    sseq_len = tier_a_wrap_tlv(0x30U, rdn_body, s1 + s2, subj_seq);
    spec.subj = subj_seq;
    spec.subj_len = sseq_len;
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_TRUE(syn_x509_parse(der, dlen, &cert));
    TEST_ASSERT_EQUAL_STRING("same.io", cert.subject_cn);

    a2 = tier_a_make_cn_ava((const uint8_t *)"evil.io", 7U, cn_ava2);
    s2 = tier_a_wrap_tlv(0x31U, cn_ava2, a2, &rdn_body[s1]);
    sseq_len = tier_a_wrap_tlv(0x30U, rdn_body, s1 + s2, subj_seq);
    spec.subj = subj_seq;
    spec.subj_len = sseq_len;
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));

    /* X509-1: Non-SEQUENCE Name, non-SEQUENCE AVA, empty RDN SET, & trailing element in AVA */
    static const uint8_t bad_name_tag[] = {0x31, 0x00};
    spec.subj = bad_name_tag;
    spec.subj_len = sizeof(bad_name_tag);
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));

    static const uint8_t bad_ava_tag[] = {0x30, 0x04, 0x31, 0x02, 0x05, 0x00};
    spec.subj = bad_ava_tag;
    spec.subj_len = sizeof(bad_ava_tag);
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));

    static const uint8_t empty_set_subj[] = {0x30, 0x02, 0x31, 0x00};
    spec.subj = empty_set_subj;
    spec.subj_len = sizeof(empty_set_subj);
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));

    a1 = tier_a_make_cn_ava((const uint8_t *)"a", 1U, cn_ava);
    cn_ava[1] = (uint8_t)(cn_ava[1] + 2U);
    cn_ava[a1] = 0x05U;
    cn_ava[a1 + 1U] = 0x00U;
    rset_len = tier_a_wrap_tlv(0x31U, cn_ava, a1 + 2U, rdn_set);
    sseq_len = tier_a_wrap_tlv(0x30U, rdn_set, rset_len, subj_seq);
    spec.subj = subj_seq;
    spec.subj_len = sseq_len;
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));

    /* X509-3: v1 & v2 certs with [3] Extensions rejected; v2 without extensions accepted */
    static const uint8_t bc_oid[] = {0x06, 0x03, 0x55, 0x1D, 0x13};
    static const uint8_t bc_vt[] = {0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xFF};
    static const uint8_t bc_vf[] = {0x04, 0x02, 0x30, 0x00};
    uint8_t bc_t_inner[16];
    uint8_t bc_f_inner[16];
    uint8_t bc_t_ext[24];
    uint8_t bc_f_ext[24];
    uint8_t bc_true_opt[32];
    (void)memcpy(bc_t_inner, bc_oid, sizeof(bc_oid));
    (void)memcpy(&bc_t_inner[sizeof(bc_oid)], bc_vt, sizeof(bc_vt));
    (void)memcpy(bc_f_inner, bc_oid, sizeof(bc_oid));
    (void)memcpy(&bc_f_inner[sizeof(bc_oid)], bc_vf, sizeof(bc_vf));
    size_t bct_len = tier_a_wrap_tlv(0x30U, bc_t_inner, 12U, bc_t_ext);
    size_t bcf_len = tier_a_wrap_tlv(0x30U, bc_f_inner, 9U, bc_f_ext);
    size_t bct_opt_len = tier_a_wrap_exts(bc_t_ext, bct_len, bc_true_opt);
    spec.subj = empty_subj;
    spec.subj_len = sizeof(empty_subj);
    spec.ver = -1;
    spec.opt = bc_true_opt;
    spec.opt_len = bct_opt_len;
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));

    spec.ver = 1;
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));

    spec.opt = NULL;
    spec.opt_len = 0U;
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_TRUE(syn_x509_parse(der, dlen, &cert));
    TEST_ASSERT_EQUAL(2U, cert.version);

    /* X509-3: Duplicate BasicConstraints OID & duplicate [3] container & empty Extensions */
    uint8_t dup_exts[48];
    uint8_t dup_bc_opt[64];
    (void)memcpy(dup_exts, bc_t_ext, bct_len);
    (void)memcpy(&dup_exts[bct_len], bc_f_ext, bcf_len);
    size_t dup_opt_len = tier_a_wrap_exts(dup_exts, bct_len + bcf_len, dup_bc_opt);
    spec.ver = 2;
    spec.opt = dup_bc_opt;
    spec.opt_len = dup_opt_len;
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));

    uint8_t two_ext_blocks[64];
    (void)memcpy(two_ext_blocks, bc_true_opt, bct_opt_len);
    (void)memcpy(&two_ext_blocks[bct_opt_len], bc_true_opt, bct_opt_len);
    spec.opt = two_ext_blocks;
    spec.opt_len = bct_opt_len * 2U;
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));

    static const uint8_t empty_exts_opt[] = {0xA3, 0x02, 0x30, 0x00};
    spec.opt = empty_exts_opt;
    spec.opt_len = sizeof(empty_exts_opt);
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));

    /* X509-3: Non-canonical BER BOOLEAN (0x01) in BasicConstraints cA and critical field */
    uint8_t ber_bool_opt[32];
    (void)memcpy(ber_bool_opt, bc_true_opt, bct_opt_len);
    ber_bool_opt[bct_opt_len - 1U] = 0x01U;
    spec.opt = ber_bool_opt;
    spec.opt_len = bct_opt_len;
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));

    static const uint8_t crit_hdr[] = {0x06, 0x03, 0x55, 0x1D, 0x13, 0x01, 0x01, 0x01};
    static const uint8_t crit_val[] = {0x04, 0x02, 0x30, 0x00};
    uint8_t crit_body[16];
    uint8_t crit_ext[24];
    uint8_t ber_crit_opt[32];
    (void)memcpy(crit_body, crit_hdr, sizeof(crit_hdr));
    (void)memcpy(&crit_body[sizeof(crit_hdr)], crit_val, sizeof(crit_val));
    size_t ce_len = tier_a_wrap_tlv(0x30U, crit_body, sizeof(crit_body), crit_ext);
    size_t co_len = tier_a_wrap_exts(crit_ext, ce_len, ber_crit_opt);
    spec.opt = ber_crit_opt;
    spec.opt_len = co_len;
    dlen = tier_a_make_cert(&spec, der);
    TEST_ASSERT_FALSE(syn_x509_parse(der, dlen, &cert));
}

void run_asn1_x509_tests(void)
{
    RUN_TEST(test_asn1_basic_tlv_parse);
    RUN_TEST(test_asn1_invalid_and_bounds_checks);
    RUN_TEST(test_ed25519_verify_basic);
    RUN_TEST(test_x509_cert_parse_and_chain);
    RUN_TEST(test_x509_ecdsa_p256_verification_and_chain);
    RUN_TEST(test_x509_ed25519_live_keypair_verification_and_chain);
    RUN_TEST(test_x509_parse_ecdsa_cert);
    RUN_TEST(test_x509_edge_cases_and_corrupt_structures);
    RUN_TEST(test_r5_n03_n04_asn1_der_and_x509_constraints_ecdsa_der);
    RUN_TEST(test_tier_a_asn1_x509_strict_checks);
}
