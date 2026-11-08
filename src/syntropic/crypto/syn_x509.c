/**
 * @file syn_x509.c
 * @brief Zero-Heap X.509 v3 Certificate & Chain Parser and Validator implementation.
 */

#include "syntropic/crypto/syn_x509.h"

#if SYN_X509_ENABLED

#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/crypto/syn_p256.h"
#include "syntropic/crypto/syn_sha256.h"

#include <string.h>

/** @cond INTERNAL */

/* Standard OID Byte Sequences */
static const uint8_t OID_ED25519[] = {0x2B, 0x65, 0x70}; /* 1.3.101.112 ed25519 */
static const uint8_t OID_EC_PUBKEY[] = {0x2A, 0x86, 0x48, 0xCE,
                                        0x3D, 0x02, 0x01}; /* 1.2.840.10045.2.1 id-ecPublicKey */
static const uint8_t OID_PRIME256V1[] = {0x2A, 0x86, 0x48, 0xCE,
                                         0x3D, 0x03, 0x01, 0x07}; /* 1.2.840.10045.3.1.7 */
static const uint8_t OID_ECDSA_SHA256[] = {0x2A, 0x86, 0x48, 0xCE,
                                           0x3D, 0x04, 0x03, 0x02}; /* 1.2.840.10045.4.3.2 */
static const uint8_t OID_COMMON_NAME[] = {0x55, 0x04, 0x03};        /* 2.5.4.3 id-at-commonName */
static const uint8_t OID_KEY_USAGE[] = {0x55, 0x1D, 0x0F};          /* 2.5.29.15 keyUsage */
static const uint8_t OID_BASIC_CONSTRAINTS[] = {0x55, 0x1D, 0x13};  /* 2.5.29.19 basicConstraints */

static void parse_rdn_cn(const SYN_ASN1_Element *name_container, char *cn_out, size_t max_len)
{
    cn_out[0] = '\0';

    const uint8_t *cur;
    size_t len;
    if (!syn_asn1_enter_container(name_container, &cur, &len)) {
        return; /* LCOV_EXCL_LINE: Defensive check if Issuer/Subject RDN container is not a
                   constructed DER sequence */
    }

    while (len > 0) {
        SYN_ASN1_Element rdn_set;
        if (!syn_asn1_step(&cur, &len, &rdn_set)) {
            break; /* LCOV_EXCL_LINE: Defensive check if RDN SET element iteration fails inside
                      Issuer/Subject sequence */
        }

        const uint8_t *set_cur;
        size_t set_len;
        if (syn_asn1_enter_container(&rdn_set, &set_cur, &set_len)) {
            SYN_ASN1_Element atv_seq;
            if (syn_asn1_step(&set_cur, &set_len, &atv_seq)) {
                const uint8_t *atv_cur;
                size_t atv_len;
                if (syn_asn1_enter_container(&atv_seq, &atv_cur, &atv_len)) {
                    SYN_ASN1_Element oid_elem, val_elem;
                    if (syn_asn1_step(&atv_cur, &atv_len, &oid_elem) &&
                        syn_asn1_step(&atv_cur, &atv_len, &val_elem)) {
                        if (syn_asn1_match_oid(&oid_elem, OID_COMMON_NAME,
                                               sizeof(OID_COMMON_NAME))) {
                            size_t copy_len = val_elem.length;
                            /* LCOV_EXCL_START: Defensive truncation clamp for CommonName strings
                             * exceeding max buffer size */
                            if (copy_len >= max_len) {
                                copy_len = max_len - 1;
                            }
                            /* LCOV_EXCL_STOP */
                            memcpy(cn_out, val_elem.value, copy_len);
                            cn_out[copy_len] = '\0';
                        }
                    }
                }
            }
        }
    }
}

static bool parse_sig_alg(const SYN_ASN1_Element *alg_seq, SYN_X509_Algo *algo_out)
{
    if (alg_seq->tag != SYN_ASN1_TAG_SEQUENCE) {
        return false;
    }
    const uint8_t *cur = alg_seq->value;
    size_t len = alg_seq->length;
    SYN_ASN1_Element oid_elem;
    if (!syn_asn1_step(&cur, &len, &oid_elem)) {
        return false;
    }
    if (syn_asn1_match_oid(&oid_elem, OID_ED25519, sizeof(OID_ED25519))) {
        *algo_out = SYN_X509_ALGO_ED25519;
        return true;
    }
    if (syn_asn1_match_oid(&oid_elem, OID_ECDSA_SHA256, sizeof(OID_ECDSA_SHA256))) {
        *algo_out = SYN_X509_ALGO_ECDSA_P256;
        return true;
    }
    return false;
}

static SYN_X509_Algo parse_spki_algo(const SYN_ASN1_Element *algo_seq)
{
    const uint8_t *algo_cur = NULL;
    size_t algo_len = 0U;
    SYN_ASN1_Element oid_elem;
    if (!syn_asn1_enter_container(algo_seq, &algo_cur, &algo_len) ||
        !syn_asn1_step(&algo_cur, &algo_len, &oid_elem)) {
        return SYN_X509_ALGO_UNKNOWN;
    }
    if (syn_asn1_match_oid(&oid_elem, OID_ED25519, sizeof(OID_ED25519))) {
        return SYN_X509_ALGO_ED25519;
    }
    if (syn_asn1_match_oid(&oid_elem, OID_ECDSA_SHA256, sizeof(OID_ECDSA_SHA256))) {
        return SYN_X509_ALGO_ECDSA_P256;
    }
    SYN_ASN1_Element curve_elem;
    if (syn_asn1_match_oid(&oid_elem, OID_EC_PUBKEY, sizeof(OID_EC_PUBKEY)) &&
        syn_asn1_step(&algo_cur, &algo_len, &curve_elem) &&
        syn_asn1_match_oid(&curve_elem, OID_PRIME256V1, sizeof(OID_PRIME256V1))) {
        return SYN_X509_ALGO_ECDSA_P256;
    }
    return SYN_X509_ALGO_UNKNOWN;
}

static void parse_spki(const SYN_ASN1_Element *spki_elem, SYN_X509_Cert *cert_out)
{
    const uint8_t *spki_cur = NULL;
    size_t spki_len = 0U;
    if (!syn_asn1_enter_container(spki_elem, &spki_cur, &spki_len)) {
        return;
    }
    SYN_ASN1_Element algo_seq;
    SYN_ASN1_Element pub_bits_elem;
    if (!syn_asn1_step(&spki_cur, &spki_len, &algo_seq) ||
        !syn_asn1_step(&spki_cur, &spki_len, &pub_bits_elem)) {
        return;
    }

    cert_out->pubkey_algo = parse_spki_algo(&algo_seq);

    const uint8_t *bits_ptr = NULL;
    size_t bit_len = 0U;
    if (syn_asn1_get_bit_string(&pub_bits_elem, &bits_ptr, &bit_len)) {
        size_t byte_len = bit_len / 8U;
        if (byte_len <= sizeof(cert_out->pubkey)) {
            (void)memcpy(cert_out->pubkey, bits_ptr, byte_len);
            cert_out->pubkey_len = byte_len;
        }
    }
}

static bool parse_basic_constraints(const SYN_ASN1_Element *oct_elem, bool *is_ca_out)
{
    const uint8_t *cur = oct_elem->value;
    size_t len = oct_elem->length;
    SYN_ASN1_Element bc_seq;
    if (!syn_asn1_step(&cur, &len, &bc_seq) || bc_seq.tag != SYN_ASN1_TAG_SEQUENCE) {
        return false;
    }

    const uint8_t *bc_cur = bc_seq.value;
    size_t bc_len = bc_seq.length;
    if (bc_len > 0U) {
        SYN_ASN1_Element elem;
        if (!syn_asn1_step(&bc_cur, &bc_len, &elem)) {
            return false;
        }
        if (elem.tag == SYN_ASN1_TAG_BOOLEAN) {
            if (elem.length != 1U) {
                return false;
            }
            *is_ca_out = (elem.value[0] != 0x00U);
        }
    }
    return true;
}

static bool parse_key_usage(const SYN_ASN1_Element *oct_elem)
{
    const uint8_t *cur = oct_elem->value;
    size_t len = oct_elem->length;
    SYN_ASN1_Element ku_bits;
    const uint8_t *bits = NULL;
    size_t bit_len = 0U;
    return syn_asn1_step(&cur, &len, &ku_bits) &&
           syn_asn1_get_bit_string(&ku_bits, &bits, &bit_len);
}

static bool parse_single_extension(const SYN_ASN1_Element *ext_seq, bool *is_ca_out)
{
    if (ext_seq->tag != SYN_ASN1_TAG_SEQUENCE) {
        return false;
    }
    const uint8_t *cur = ext_seq->value;
    size_t len = ext_seq->length;
    SYN_ASN1_Element oid_elem;
    SYN_ASN1_Element next_elem;
    if (!syn_asn1_step(&cur, &len, &oid_elem) || !syn_asn1_step(&cur, &len, &next_elem)) {
        return false;
    }
    bool is_critical = false;
    if (next_elem.tag == SYN_ASN1_TAG_BOOLEAN) {
        if (next_elem.length != 1U) {
            return false;
        }
        is_critical = (next_elem.value[0] != 0x00U);
        if (!syn_asn1_step(&cur, &len, &next_elem)) {
            return false;
        }
    }
    if (next_elem.tag != SYN_ASN1_TAG_OCTET_STRING) {
        return false;
    }
    if (syn_asn1_match_oid(&oid_elem, OID_BASIC_CONSTRAINTS, sizeof(OID_BASIC_CONSTRAINTS))) {
        return parse_basic_constraints(&next_elem, is_ca_out);
    }
    if (syn_asn1_match_oid(&oid_elem, OID_KEY_USAGE, sizeof(OID_KEY_USAGE))) {
        return parse_key_usage(&next_elem);
    }
    return !is_critical;
}

static bool parse_tbs_optional_fields(const uint8_t *tbs_cur, size_t tbs_len, bool *is_ca_out)
{
    while (tbs_len > 0U) {
        SYN_ASN1_Element opt_elem;
        if (!syn_asn1_step(&tbs_cur, &tbs_len, &opt_elem)) {
            return false;
        }
        if (opt_elem.tag_class == SYN_ASN1_CLASS_CONTEXT_SPECIFIC && opt_elem.tag_number == 3U) {
            if (!opt_elem.constructed) {
                return false;
            }
            const uint8_t *ext_cur = opt_elem.value;
            size_t ext_len = opt_elem.length;
            SYN_ASN1_Element exts_seq;
            if (!syn_asn1_step(&ext_cur, &ext_len, &exts_seq) ||
                exts_seq.tag != SYN_ASN1_TAG_SEQUENCE) {
                return false;
            }
            const uint8_t *list_cur = exts_seq.value;
            size_t list_len = exts_seq.length;
            while (list_len > 0U) {
                SYN_ASN1_Element ext_seq;
                if (!syn_asn1_step(&list_cur, &list_len, &ext_seq) ||
                    !parse_single_extension(&ext_seq, is_ca_out)) {
                    return false;
                }
            }
        }
    }
    return true;
}

static bool parse_tbs_version(const SYN_ASN1_Element *ver_container, SYN_X509_Cert *cert_out)
{
    const uint8_t *v_cur = NULL;
    size_t v_len = 0U;
    SYN_ASN1_Element ver_elem;
    const uint8_t *v_bytes = NULL;
    size_t v_bytes_len = 0U;
    if (!syn_asn1_enter_container(ver_container, &v_cur, &v_len) ||
        !syn_asn1_step(&v_cur, &v_len, &ver_elem) || v_len != 0U ||
        !syn_asn1_get_integer(&ver_elem, &v_bytes, &v_bytes_len) || v_bytes_len != 1U ||
        v_bytes[0] > 2U) {
        return false;
    }
    cert_out->version = (uint32_t)v_bytes[0] + 1U;
    return true;
}

static bool decode_ecdsa_der_int(const SYN_ASN1_Element *elem, uint8_t out32[32])
{
    const uint8_t *val = NULL;
    size_t vlen = 0U;
    if (!syn_asn1_get_integer(elem, &val, &vlen)) {
        return false;
    }
    if ((elem->value[0] & 0x80U) != 0U ||
        (elem->length > 1U && elem->value[0] == 0x00U && (elem->value[1] & 0x80U) == 0U) ||
        vlen > 32U) {
        return false;
    }
    (void)memset(out32, 0, 32U);
    (void)memcpy(&out32[32U - vlen], val, vlen);
    return true;
}

static bool decode_ecdsa_der_seq(const uint8_t *sig, size_t sig_len, uint8_t r32[32],
                                 uint8_t s32[32])
{
    SYN_ASN1_Element seq;
    if (!syn_asn1_parse_element(sig, sig_len, &seq) || (seq.header_len + seq.length) != sig_len) {
        return false;
    }
    const uint8_t *cur = seq.value;
    size_t rem = seq.length;
    SYN_ASN1_Element r_elem;
    SYN_ASN1_Element s_elem;
    if (!syn_asn1_step(&cur, &rem, &r_elem) || !syn_asn1_step(&cur, &rem, &s_elem) || rem != 0U) {
        return false;
    }
    return decode_ecdsa_der_int(&r_elem, r32) && decode_ecdsa_der_int(&s_elem, s32);
}

static bool decode_ecdsa_sig(const uint8_t *sig, size_t sig_len, uint8_t r32[32], uint8_t s32[32])
{
    if (sig_len > 0U && sig[0] == SYN_ASN1_TAG_SEQUENCE &&
        decode_ecdsa_der_seq(sig, sig_len, r32, s32)) {
        return true;
    }

    if (sig_len == 64U) {
        (void)memcpy(r32, sig, 32U);
        (void)memcpy(s32, &sig[32], 32U);
        return true;
    }
    return false;
}

static bool parse_tbs_cert_fields(const uint8_t *tbs_cur, size_t tbs_len, SYN_X509_Cert *cert_out,
                                  SYN_X509_Algo *tbs_sig_algo)
{
    SYN_ASN1_Element first_elem;
    if (!syn_asn1_step(&tbs_cur, &tbs_len, &first_elem)) {
        return false;
    }

    if (first_elem.tag_class == SYN_ASN1_CLASS_CONTEXT_SPECIFIC && first_elem.tag_number == 0U) {
        if (!parse_tbs_version(&first_elem, cert_out) ||
            !syn_asn1_step(&tbs_cur, &tbs_len, &first_elem)) {
            return false;
        }
    } else {
        cert_out->version = 1U;
    }

    if (!syn_asn1_get_integer(&first_elem, &cert_out->serial, &cert_out->serial_len)) {
        return false;
    }

    SYN_ASN1_Element sig_algo_elem;
    SYN_ASN1_Element issuer_elem;
    SYN_ASN1_Element validity_elem;
    SYN_ASN1_Element subject_elem;
    SYN_ASN1_Element spki_elem;

    if (!syn_asn1_step(&tbs_cur, &tbs_len, &sig_algo_elem) ||
        !parse_sig_alg(&sig_algo_elem, tbs_sig_algo) ||
        !syn_asn1_step(&tbs_cur, &tbs_len, &issuer_elem) ||
        !syn_asn1_step(&tbs_cur, &tbs_len, &validity_elem) ||
        !syn_asn1_step(&tbs_cur, &tbs_len, &subject_elem) ||
        !syn_asn1_step(&tbs_cur, &tbs_len, &spki_elem)) {
        return false;
    }

    parse_rdn_cn(&issuer_elem, cert_out->issuer_cn, sizeof(cert_out->issuer_cn));
    parse_rdn_cn(&subject_elem, cert_out->subject_cn, sizeof(cert_out->subject_cn));
    parse_spki(&spki_elem, cert_out);

    return parse_tbs_optional_fields(tbs_cur, tbs_len, &cert_out->is_ca);
}

static bool parse_outer_sig_fields(const uint8_t *cert_cur, size_t cert_len,
                                   SYN_X509_Algo tbs_sig_algo, SYN_X509_Cert *cert_out)
{
    SYN_ASN1_Element outer_sig_algo;
    SYN_ASN1_Element sig_bits_elem;
    SYN_X509_Algo outer_algo = SYN_X509_ALGO_UNKNOWN;
    if (!syn_asn1_step(&cert_cur, &cert_len, &outer_sig_algo) ||
        !parse_sig_alg(&outer_sig_algo, &outer_algo) || outer_algo != tbs_sig_algo ||
        !syn_asn1_step(&cert_cur, &cert_len, &sig_bits_elem)) {
        return false;
    }

    const uint8_t *sig_ptr = NULL;
    size_t sig_bit_len = 0U;
    if (!syn_asn1_get_bit_string(&sig_bits_elem, &sig_ptr, &sig_bit_len) ||
        (sig_bit_len % 8U) != 0U || (sig_bit_len / 8U) > sizeof(cert_out->signature)) {
        return false;
    }

    size_t sig_byte_len = sig_bit_len / 8U;
    (void)memcpy(cert_out->signature, sig_ptr, sig_byte_len);
    cert_out->signature_len = sig_byte_len;
    cert_out->sig_algo = outer_algo;
    return true;
}

/** @endcond */

bool syn_x509_parse(const uint8_t *der, size_t der_len, SYN_X509_Cert *cert_out)
{
    if (der == NULL || der_len == 0U || cert_out == NULL) {
        return false;
    }

    (void)memset(cert_out, 0, sizeof(SYN_X509_Cert));

    SYN_ASN1_Element root_seq;
    const uint8_t *ptr = der;
    size_t rem = der_len;

    if (!syn_asn1_step(&ptr, &rem, &root_seq) || root_seq.tag != SYN_ASN1_TAG_SEQUENCE) {
        return false;
    }

    const uint8_t *cert_cur = root_seq.value;
    size_t cert_len = root_seq.length;

    /* 1. TBSCertificate (To Be Signed) */
    SYN_ASN1_Element tbs_elem;
    if (!syn_asn1_step(&cert_cur, &cert_len, &tbs_elem) || tbs_elem.tag != SYN_ASN1_TAG_SEQUENCE) {
        return false;
    }

    cert_out->tbs_bytes = tbs_elem.value - tbs_elem.header_len;
    cert_out->tbs_len = tbs_elem.header_len + tbs_elem.length;

    SYN_X509_Algo tbs_sig_algo = SYN_X509_ALGO_UNKNOWN;
    if (!parse_tbs_cert_fields(tbs_elem.value, tbs_elem.length, cert_out, &tbs_sig_algo)) {
        return false;
    }

    /* 2. SignatureAlgorithm outer Sequence & 3. SignatureValue BIT STRING */
    return parse_outer_sig_fields(cert_cur, cert_len, tbs_sig_algo, cert_out);
}

bool syn_x509_verify_signature(const SYN_X509_Cert *cert, const uint8_t *issuer_pubkey,
                               size_t issuer_pubkey_len, SYN_X509_Algo algo)
{
    if (cert == NULL || issuer_pubkey == NULL || cert->tbs_bytes == NULL || cert->tbs_len == 0U) {
        return false;
    }

    if (algo == SYN_X509_ALGO_ED25519) {
        if (issuer_pubkey_len != SYN_ED25519_PUBLIC_KEY_SIZE ||
            cert->signature_len != SYN_ED25519_SIGNATURE_SIZE) {
            return false;
        }
        return syn_ed25519_verify(cert->signature, cert->tbs_bytes, cert->tbs_len, issuer_pubkey);
    }

    if (algo == SYN_X509_ALGO_ECDSA_P256) {
        const uint8_t *px = issuer_pubkey;
        const uint8_t *py = issuer_pubkey + 32;
        if (issuer_pubkey_len == 65U && issuer_pubkey[0] == 0x04U) {
            px = issuer_pubkey + 1;
            py = issuer_pubkey + 33;
        } else if (issuer_pubkey_len != 64U) {
            return false;
        }

        uint8_t r32[32];
        uint8_t s32[32];
        if (!decode_ecdsa_sig(cert->signature, cert->signature_len, r32, s32)) {
            return false;
        }

        uint8_t hash[SYN_SHA256_DIGEST_SIZE];
        syn_sha256(cert->tbs_bytes, cert->tbs_len, hash);

        return syn_p256_verify_ecdsa(hash, r32, s32, px, py);
    }

    return false;
}

bool syn_x509_validate_chain(const SYN_X509_Cert *cert, const SYN_X509_Cert *root_ca,
                             const char *expected_cn)
{
    if (cert == NULL || root_ca == NULL) {
        return false;
    }

    if (!root_ca->is_ca || root_ca->subject_cn[0] == '\0' ||
        strcmp(cert->issuer_cn, root_ca->subject_cn) != 0 ||
        cert->sig_algo == SYN_X509_ALGO_UNKNOWN || cert->sig_algo != root_ca->pubkey_algo) {
        return false;
    }

    if (expected_cn != NULL && strcmp(cert->subject_cn, expected_cn) != 0) {
        return false;
    }

    return syn_x509_verify_signature(cert, root_ca->pubkey, root_ca->pubkey_len,
                                     root_ca->pubkey_algo);
}

#endif /* SYN_X509_ENABLED */
