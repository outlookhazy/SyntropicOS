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

/**
 * @brief Validate and store a CommonName attribute value, rejecting NULs, overflow, and conflicts.
 *
 * @param val_elem Parsed ASN.1 attribute value element.
 * @param cn_out   Destination NUL-terminated CommonName buffer.
 * @param max_len  Capacity of @p cn_out in bytes.
 * @param has_cn   In/out flag tracking whether a CommonName was already seen in this Name.
 * @return true if the CommonName is valid and non-conflicting, false otherwise.
 */
static bool store_rdn_cn(const SYN_ASN1_Element *val_elem, char *cn_out, size_t max_len,
                         bool *has_cn)
{
    if (val_elem->length >= max_len) {
        return false;
    }
    if (val_elem->length > 0U && memchr(val_elem->value, '\0', val_elem->length) != NULL) {
        return false;
    }
    if (*has_cn) {
        return (strlen(cn_out) == val_elem->length) &&
               (val_elem->length == 0U || memcmp(cn_out, val_elem->value, val_elem->length) == 0);
    }
    if (val_elem->length > 0U) {
        (void)memcpy(cn_out, val_elem->value, val_elem->length);
    }
    cn_out[val_elem->length] = '\0';
    *has_cn = true;
    return true;
}

/**
 * @brief Parse one AttributeTypeAndValue SEQUENCE inside an RDN SET.
 *
 * @param atv_seq Parsed ASN.1 element expected to be an AttributeTypeAndValue SEQUENCE.
 * @param cn_out  Destination NUL-terminated CommonName buffer.
 * @param max_len Capacity of @p cn_out in bytes.
 * @param has_cn  In/out flag tracking whether a CommonName was already seen in this Name.
 * @return true if the AVA is structurally valid and any CommonName is valid, false otherwise.
 */
static bool parse_rdn_ava(const SYN_ASN1_Element *atv_seq, char *cn_out, size_t max_len,
                          bool *has_cn)
{
    if (atv_seq->tag != SYN_ASN1_TAG_SEQUENCE) {
        return false;
    }
    const uint8_t *atv_cur = atv_seq->value;
    size_t atv_len = atv_seq->length;
    SYN_ASN1_Element oid_elem;
    SYN_ASN1_Element val_elem;
    if (!syn_asn1_step(&atv_cur, &atv_len, &oid_elem) || oid_elem.tag != SYN_ASN1_TAG_OID ||
        !syn_asn1_step(&atv_cur, &atv_len, &val_elem) || atv_len != 0U) {
        return false;
    }
    if (syn_asn1_match_oid(&oid_elem, OID_COMMON_NAME, sizeof(OID_COMMON_NAME))) {
        return store_rdn_cn(&val_elem, cn_out, max_len, has_cn);
    }
    return true;
}

/**
 * @brief Parse an X.509 Name SEQUENCE and extract its CommonName (CN) if present.
 *
 * @param name_container Parsed ASN.1 Name element (expected SEQUENCE of RDN SETs).
 * @param cn_out         Destination NUL-terminated CommonName buffer.
 * @param max_len        Capacity of @p cn_out in bytes.
 * @return true if the Name structure and all CommonName attributes are valid, false otherwise.
 */
static bool parse_rdn_cn(const SYN_ASN1_Element *name_container, char *cn_out, size_t max_len)
{
    cn_out[0] = '\0';
    if (name_container->tag != SYN_ASN1_TAG_SEQUENCE) {
        return false;
    }

    const uint8_t *cur = name_container->value;
    size_t len = name_container->length;
    bool has_cn = false;

    while (len > 0U) {
        SYN_ASN1_Element rdn_set;
        if (!syn_asn1_step(&cur, &len, &rdn_set) || rdn_set.tag != SYN_ASN1_TAG_SET ||
            rdn_set.length == 0U) {
            return false;
        }

        const uint8_t *set_cur = rdn_set.value;
        size_t set_len = rdn_set.length;
        while (set_len > 0U) {
            SYN_ASN1_Element atv_seq;
            if (!syn_asn1_step(&set_cur, &set_len, &atv_seq) ||
                !parse_rdn_ava(&atv_seq, cn_out, max_len, &has_cn)) {
                return false;
            }
        }
    }
    return true;
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

/**
 * @brief Parse a canonical DER BOOLEAN (0x00 for FALSE or 0xFF for TRUE).
 *
 * @param elem    Parsed ASN.1 BOOLEAN element.
 * @param val_out Output boolean value.
 * @return true if canonical DER boolean, false on BER non-canonical byte or bad length.
 */
static bool parse_der_boolean(const SYN_ASN1_Element *elem, bool *val_out)
{
    if (elem->length != 1U || (elem->value[0] != 0x00U && elem->value[0] != 0xFFU)) {
        return false;
    }
    *val_out = (elem->value[0] == 0xFFU);
    return true;
}

/**
 * @brief Parse BasicConstraints extnValue OCTET STRING payload.
 *
 * @param oct_elem  Parsed ASN.1 OCTET STRING wrapping BasicConstraints SEQUENCE.
 * @param is_ca_out Output cA flag.
 * @return true if valid BasicConstraints encoding, false otherwise.
 */
static bool parse_basic_constraints(const SYN_ASN1_Element *oct_elem, bool *is_ca_out)
{
    const uint8_t *cur = oct_elem->value;
    size_t len = oct_elem->length;
    SYN_ASN1_Element bc_seq;
    if (!syn_asn1_step(&cur, &len, &bc_seq) || bc_seq.tag != SYN_ASN1_TAG_SEQUENCE || len != 0U) {
        return false;
    }

    *is_ca_out = false;
    const uint8_t *bc_cur = bc_seq.value;
    size_t bc_len = bc_seq.length;
    if (bc_len > 0U) {
        SYN_ASN1_Element elem;
        if (!syn_asn1_step(&bc_cur, &bc_len, &elem)) {
            return false;
        }
        if (elem.tag == SYN_ASN1_TAG_BOOLEAN && !parse_der_boolean(&elem, is_ca_out)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Parse KeyUsage extnValue OCTET STRING payload.
 *
 * @param oct_elem Parsed ASN.1 OCTET STRING wrapping KeyUsage BIT STRING.
 * @return true if valid KeyUsage BIT STRING, false otherwise.
 */
static bool parse_key_usage(const SYN_ASN1_Element *oct_elem)
{
    const uint8_t *cur = oct_elem->value;
    size_t len = oct_elem->length;
    SYN_ASN1_Element ku_bits;
    const uint8_t *bits = NULL;
    size_t bit_len = 0U;
    return syn_asn1_step(&cur, &len, &ku_bits) && len == 0U &&
           syn_asn1_get_bit_string(&ku_bits, &bits, &bit_len);
}

/**
 * @brief Check whether @p oid_elem duplicates any extension OID in previously validated bytes.
 *
 * @param exts_buf   Start of Extensions SEQUENCE value bytes.
 * @param prev_bytes Byte offset of current extension within @p exts_buf.
 * @param oid_elem   Current extension OID element.
 * @return true if the OID already appeared in an earlier extension, false otherwise.
 */
static bool ext_oid_already_seen(const uint8_t *exts_buf, size_t prev_bytes,
                                 const SYN_ASN1_Element *oid_elem)
{
    const uint8_t *scan = exts_buf;
    size_t rem = prev_bytes;
    while (rem > 0U) {
        SYN_ASN1_Element prev_ext;
        SYN_ASN1_Element prev_oid;
        (void)syn_asn1_step(&scan, &rem, &prev_ext);
        const uint8_t *ext_cur = prev_ext.value;
        size_t ext_rem = prev_ext.length;
        (void)syn_asn1_step(&ext_cur, &ext_rem, &prev_oid);
        if (syn_asn1_match_oid(oid_elem, prev_oid.value, prev_oid.length)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Parse and validate a single X.509 v3 Extension SEQUENCE.
 *
 * @param ext_seq    Parsed Extension SEQUENCE element.
 * @param exts_buf   Start of enclosing Extensions SEQUENCE payload for duplicate-OID checks.
 * @param prev_bytes Number of validated bytes preceding @p ext_seq in @p exts_buf.
 * @param is_ca_out  Output cA flag updated by BasicConstraints.
 * @return true if valid and recognized (or non-critical), false otherwise.
 */
static bool parse_single_extension(const SYN_ASN1_Element *ext_seq, const uint8_t *exts_buf,
                                   size_t prev_bytes, bool *is_ca_out)
{
    if (ext_seq->tag != SYN_ASN1_TAG_SEQUENCE) {
        return false;
    }
    const uint8_t *cur = ext_seq->value;
    size_t len = ext_seq->length;
    SYN_ASN1_Element oid_elem;
    SYN_ASN1_Element next_elem;
    if (!syn_asn1_step(&cur, &len, &oid_elem) || oid_elem.tag != SYN_ASN1_TAG_OID ||
        !syn_asn1_step(&cur, &len, &next_elem) ||
        ext_oid_already_seen(exts_buf, prev_bytes, &oid_elem)) {
        return false;
    }
    bool is_critical = false;
    if (next_elem.tag == SYN_ASN1_TAG_BOOLEAN) {
        if (!parse_der_boolean(&next_elem, &is_critical) ||
            !syn_asn1_step(&cur, &len, &next_elem)) {
            return false;
        }
    }
    if (next_elem.tag != SYN_ASN1_TAG_OCTET_STRING || len != 0U) {
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

/**
 * @brief Parse the [3] EXPLICIT Extensions container of an X.509 v3 TBSCertificate.
 *
 * @param opt_elem  Parsed [3] context-specific element.
 * @param is_ca_out Output cA flag.
 * @return true if all extensions are valid and non-duplicated, false otherwise.
 */
static bool parse_extensions_block(const SYN_ASN1_Element *opt_elem, bool *is_ca_out)
{
    if (!opt_elem->constructed) {
        return false;
    }
    const uint8_t *ext_cur = opt_elem->value;
    size_t ext_len = opt_elem->length;
    SYN_ASN1_Element exts_seq;
    if (!syn_asn1_step(&ext_cur, &ext_len, &exts_seq) || exts_seq.tag != SYN_ASN1_TAG_SEQUENCE ||
        ext_len != 0U || exts_seq.length == 0U) {
        return false;
    }

    const uint8_t *list_cur = exts_seq.value;
    size_t list_len = exts_seq.length;
    while (list_len > 0U) {
        size_t prev_bytes = (size_t)(list_cur - exts_seq.value);
        SYN_ASN1_Element ext_seq;
        if (!syn_asn1_step(&list_cur, &list_len, &ext_seq) ||
            !parse_single_extension(&ext_seq, exts_seq.value, prev_bytes, is_ca_out)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Parse optional trailing fields ([1], [2], [3] Extensions) in TBSCertificate.
 *
 * @param tbs_cur   Remaining TBSCertificate cursor after SubjectPublicKeyInfo.
 * @param tbs_len   Remaining TBSCertificate byte count.
 * @param version   Parsed certificate version (1, 2, or 3).
 * @param is_ca_out Output cA flag.
 * @return true if optional fields conform to version rules and DER syntax, false otherwise.
 */
static bool parse_tbs_optional_fields(const uint8_t *tbs_cur, size_t tbs_len, uint32_t version,
                                      bool *is_ca_out)
{
    bool seen_exts = false;
    while (tbs_len > 0U) {
        SYN_ASN1_Element opt_elem;
        if (!syn_asn1_step(&tbs_cur, &tbs_len, &opt_elem)) {
            return false;
        }
        if (opt_elem.tag_class == SYN_ASN1_CLASS_CONTEXT_SPECIFIC && opt_elem.tag_number == 3U) {
            if (version != 3U || seen_exts || !parse_extensions_block(&opt_elem, is_ca_out)) {
                return false;
            }
            seen_exts = true;
        }
    }
    return true;
}

/**
 * @brief Parse explicit [0] Version field of TBSCertificate.
 *
 * @param ver_container Parsed [0] EXPLICIT container element.
 * @param cert_out      Output certificate receiving version (1..3).
 * @return true if valid X.509 version integer (0..2), false otherwise.
 */
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

/**
 * @brief Decode one positive canonical DER ECDSA coordinate INTEGER into a 32-byte big-endian slot.
 *
 * @param elem  Parsed ASN.1 INTEGER element.
 * @param out32 32-byte output buffer (left-padded with zeros).
 * @return true if valid positive DER integer of at most 32 value bytes, false otherwise.
 */
static bool decode_ecdsa_der_int(const SYN_ASN1_Element *elem, uint8_t out32[32])
{
    const uint8_t *val = NULL;
    size_t vlen = 0U;
    if (!syn_asn1_get_integer(elem, &val, &vlen) || vlen > 32U) {
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

/**
 * @brief Parse TBSCertificate leading fields: optional [0] Version, serialNumber, and signature.
 *
 * @param tbs_cur      In/out TBSCertificate cursor.
 * @param tbs_len      In/out TBSCertificate remaining byte count.
 * @param cert_out     Output certificate receiving version and serial slice.
 * @param tbs_sig_algo Output inner TBSCertificate signature algorithm.
 * @return true on valid header fields, false otherwise.
 */
static bool parse_tbs_header(const uint8_t **tbs_cur, size_t *tbs_len, SYN_X509_Cert *cert_out,
                             SYN_X509_Algo *tbs_sig_algo)
{
    SYN_ASN1_Element first_elem;
    if (!syn_asn1_step(tbs_cur, tbs_len, &first_elem)) {
        return false;
    }

    if (first_elem.tag_class == SYN_ASN1_CLASS_CONTEXT_SPECIFIC && first_elem.tag_number == 0U) {
        if (!parse_tbs_version(&first_elem, cert_out) ||
            !syn_asn1_step(tbs_cur, tbs_len, &first_elem)) {
            return false;
        }
    } else {
        cert_out->version = 1U;
    }

    SYN_ASN1_Element sig_algo_elem;
    return syn_asn1_get_integer(&first_elem, &cert_out->serial, &cert_out->serial_len) &&
           syn_asn1_step(tbs_cur, tbs_len, &sig_algo_elem) &&
           parse_sig_alg(&sig_algo_elem, tbs_sig_algo);
}

/**
 * @brief Parse all TBSCertificate fields inside a DER X.509 certificate.
 *
 * @param tbs_cur      TBSCertificate value bytes.
 * @param tbs_len      TBSCertificate value length.
 * @param cert_out     Output parsed certificate structure.
 * @param tbs_sig_algo Output inner TBSCertificate signature algorithm.
 * @return true if TBSCertificate is valid, false otherwise.
 */
static bool parse_tbs_cert_fields(const uint8_t *tbs_cur, size_t tbs_len, SYN_X509_Cert *cert_out,
                                  SYN_X509_Algo *tbs_sig_algo)
{
    if (!parse_tbs_header(&tbs_cur, &tbs_len, cert_out, tbs_sig_algo)) {
        return false;
    }

    SYN_ASN1_Element issuer_elem;
    SYN_ASN1_Element validity_elem;
    SYN_ASN1_Element subject_elem;
    SYN_ASN1_Element spki_elem;

    if (!syn_asn1_step(&tbs_cur, &tbs_len, &issuer_elem) ||
        !syn_asn1_step(&tbs_cur, &tbs_len, &validity_elem) ||
        !syn_asn1_step(&tbs_cur, &tbs_len, &subject_elem) ||
        !syn_asn1_step(&tbs_cur, &tbs_len, &spki_elem) ||
        !parse_rdn_cn(&issuer_elem, cert_out->issuer_cn, sizeof(cert_out->issuer_cn)) ||
        !parse_rdn_cn(&subject_elem, cert_out->subject_cn, sizeof(cert_out->subject_cn))) {
        return false;
    }

    parse_spki(&spki_elem, cert_out);

    return parse_tbs_optional_fields(tbs_cur, tbs_len, cert_out->version, &cert_out->is_ca);
}

/**
 * @brief Parse outer signatureAlgorithm and signatureValue fields of X.509 Certificate SEQUENCE.
 *
 * @param cert_cur     Cursor positioned after TBSCertificate inside outer Certificate SEQUENCE.
 * @param cert_len     Remaining bytes inside outer Certificate SEQUENCE.
 * @param tbs_sig_algo Algorithm parsed from TBSCertificate.signature for match check.
 * @param cert_out     Output parsed certificate structure.
 * @return true if outer signature fields are valid with no trailing elements, false otherwise.
 */
static bool parse_outer_sig_fields(const uint8_t *cert_cur, size_t cert_len,
                                   SYN_X509_Algo tbs_sig_algo, SYN_X509_Cert *cert_out)
{
    SYN_ASN1_Element outer_sig_algo;
    SYN_ASN1_Element sig_bits_elem;
    SYN_X509_Algo outer_algo = SYN_X509_ALGO_UNKNOWN;
    if (!syn_asn1_step(&cert_cur, &cert_len, &outer_sig_algo) ||
        !parse_sig_alg(&outer_sig_algo, &outer_algo) || outer_algo != tbs_sig_algo ||
        !syn_asn1_step(&cert_cur, &cert_len, &sig_bits_elem) || cert_len != 0U) {
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

    if (!syn_asn1_step(&ptr, &rem, &root_seq) || root_seq.tag != SYN_ASN1_TAG_SEQUENCE ||
        rem != 0U) {
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

/**
 * @brief Verify an ECDSA-P256 SHA-256 certificate signature against raw or 0x04-prefixed key.
 * @param cert              Parsed child certificate.
 * @param issuer_pubkey     64-byte raw (X||Y) or 65-byte uncompressed (0x04||X||Y) public key.
 * @param issuer_pubkey_len Length of @p issuer_pubkey in bytes.
 * @return true if ECDSA-P256 signature verifies over `cert->tbs_bytes`.
 */
static bool verify_ecdsa_p256_cert(const SYN_X509_Cert *cert, const uint8_t *issuer_pubkey,
                                   size_t issuer_pubkey_len)
{
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

bool syn_x509_verify_signature(const SYN_X509_Cert *cert, const uint8_t *issuer_pubkey,
                               size_t issuer_pubkey_len, SYN_X509_Algo algo)
{
    if (cert == NULL || issuer_pubkey == NULL || cert->tbs_bytes == NULL || cert->tbs_len == 0U ||
        cert->sig_algo != algo) {
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
        return verify_ecdsa_p256_cert(cert, issuer_pubkey, issuer_pubkey_len);
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
