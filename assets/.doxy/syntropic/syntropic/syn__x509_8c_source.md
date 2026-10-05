

# File syn\_x509.c

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_x509.c**](syn__x509_8c.md)

[Go to the documentation of this file](syn__x509_8c.md)


```C++

#include "syntropic/crypto/syn_x509.h"

#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/crypto/syn_p256.h"
#include "syntropic/crypto/syn_sha256.h"

#include <string.h>

/* Standard OID Byte Sequences */
static const uint8_t OID_ED25519[] = {0x2B, 0x65, 0x70}; /* 1.3.101.112 ed25519 */
static const uint8_t OID_EC_PUBKEY[] = {0x2A, 0x86, 0x48, 0xCE,
                                        0x3D, 0x02, 0x01}; /* 1.2.840.10045.2.1 id-ecPublicKey */
static const uint8_t OID_ECDSA_SHA256[] = {0x2A, 0x86, 0x48, 0xCE,
                                           0x3D, 0x04, 0x03, 0x02}; /* 1.2.840.10045.4.3.2 */
static const uint8_t OID_COMMON_NAME[] = {0x55, 0x04, 0x03};        /* 2.5.4.3 id-at-commonName */

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

bool syn_x509_parse(const uint8_t *der, size_t der_len, SYN_X509_Cert *cert_out)
{
    if (der == NULL || der_len == 0 || cert_out == NULL) {
        return false;
    }

    memset(cert_out, 0, sizeof(SYN_X509_Cert));

    SYN_ASN1_Element root_seq;
    const uint8_t *ptr = der;
    size_t rem = der_len;

    if (!syn_asn1_step(&ptr, &rem, &root_seq) || root_seq.tag != SYN_ASN1_TAG_SEQUENCE ||
        !root_seq.constructed) {
        return false;
    }

    const uint8_t *cert_cur = root_seq.value;
    size_t cert_len = root_seq.length;

    /* 1. TBSCertificate (To Be Signed) */
    SYN_ASN1_Element tbs_elem;
    if (!syn_asn1_step(&cert_cur, &cert_len, &tbs_elem) || tbs_elem.tag != SYN_ASN1_TAG_SEQUENCE ||
        !tbs_elem.constructed) {
        return false;
    }

    cert_out->tbs_bytes = tbs_elem.value - tbs_elem.header_len;
    cert_out->tbs_len = tbs_elem.header_len + tbs_elem.length;

    const uint8_t *tbs_cur = tbs_elem.value;
    size_t tbs_len = tbs_elem.length;

    /* Check Version (Optional [0] EXPLICIT INTEGER) */
    SYN_ASN1_Element first_elem;
    if (!syn_asn1_step(&tbs_cur, &tbs_len, &first_elem)) {
        return false;
    }

    if (first_elem.tag_class == SYN_ASN1_CLASS_CONTEXT_SPECIFIC && first_elem.tag_number == 0) {
        const uint8_t *v_cur;
        size_t v_len;
        if (syn_asn1_enter_container(&first_elem, &v_cur, &v_len)) {
            SYN_ASN1_Element ver_elem;
            if (syn_asn1_step(&v_cur, &v_len, &ver_elem)) {
                const uint8_t *v_bytes;
                size_t v_bytes_len;
                if (syn_asn1_get_integer(&ver_elem, &v_bytes, &v_bytes_len) && v_bytes_len > 0) {
                    cert_out->version = v_bytes[v_bytes_len - 1] + 1U; /* v1=0, v2=1, v3=2 */
                }
            }
        }
        /* Read next for Serial Number */
        if (!syn_asn1_step(&tbs_cur, &tbs_len, &first_elem)) {
            return false;
        }
    } else {
        cert_out->version = 1U; /* Default v1 */
    }

    /* Serial Number */
    if (!syn_asn1_get_integer(&first_elem, &cert_out->serial, &cert_out->serial_len)) {
        return false;
    }

    /* Signature Algorithm Identifier */
    SYN_ASN1_Element sig_algo_elem;
    if (!syn_asn1_step(&tbs_cur, &tbs_len, &sig_algo_elem)) {
        return false;
    }

    /* Issuer Name */
    SYN_ASN1_Element issuer_elem;
    if (!syn_asn1_step(&tbs_cur, &tbs_len, &issuer_elem)) {
        return false;
    }
    parse_rdn_cn(&issuer_elem, cert_out->issuer_cn, sizeof(cert_out->issuer_cn));

    /* Validity Period */
    SYN_ASN1_Element validity_elem;
    if (!syn_asn1_step(&tbs_cur, &tbs_len, &validity_elem)) {
        return false;
    }

    /* Subject Name */
    SYN_ASN1_Element subject_elem;
    if (!syn_asn1_step(&tbs_cur, &tbs_len, &subject_elem)) {
        return false;
    }
    parse_rdn_cn(&subject_elem, cert_out->subject_cn, sizeof(cert_out->subject_cn));

    /* SubjectPublicKeyInfo */
    SYN_ASN1_Element spki_elem;
    if (!syn_asn1_step(&tbs_cur, &tbs_len, &spki_elem)) {
        return false;
    }

    const uint8_t *spki_cur;
    size_t spki_len;
    if (syn_asn1_enter_container(&spki_elem, &spki_cur, &spki_len)) {
        SYN_ASN1_Element algo_seq, pub_bits_elem;
        if (syn_asn1_step(&spki_cur, &spki_len, &algo_seq) &&
            syn_asn1_step(&spki_cur, &spki_len, &pub_bits_elem)) {
            const uint8_t *algo_cur;
            size_t algo_len;
            if (syn_asn1_enter_container(&algo_seq, &algo_cur, &algo_len)) {
                SYN_ASN1_Element oid_elem;
                if (syn_asn1_step(&algo_cur, &algo_len, &oid_elem)) {
                    if (syn_asn1_match_oid(&oid_elem, OID_ED25519, sizeof(OID_ED25519))) {
                        cert_out->pubkey_algo = SYN_X509_ALGO_ED25519;
                    } else if (syn_asn1_match_oid(&oid_elem, OID_EC_PUBKEY,
                                                  sizeof(OID_EC_PUBKEY)) ||
                               syn_asn1_match_oid(&oid_elem, OID_ECDSA_SHA256,
                                                  sizeof(OID_ECDSA_SHA256))) {
                        cert_out->pubkey_algo = SYN_X509_ALGO_ECDSA_P256;
                    }
                }
            }
            const uint8_t *bits_ptr;
            size_t bit_len;
            if (syn_asn1_get_bit_string(&pub_bits_elem, &bits_ptr, &bit_len)) {
                size_t byte_len = bit_len / 8;
                if (byte_len <= sizeof(cert_out->pubkey)) {
                    memcpy(cert_out->pubkey, bits_ptr, byte_len);
                    cert_out->pubkey_len = byte_len;
                }
            }
        }
    }

    /* 2. SignatureAlgorithm outer Sequence */
    SYN_ASN1_Element outer_sig_algo;
    if (!syn_asn1_step(&cert_cur, &cert_len, &outer_sig_algo)) {
        return false;
    }

    /* 3. SignatureValue BIT STRING */
    SYN_ASN1_Element sig_bits_elem;
    if (!syn_asn1_step(&cert_cur, &cert_len, &sig_bits_elem)) {
        return false;
    }

    const uint8_t *sig_ptr;
    size_t sig_bit_len;
    if (syn_asn1_get_bit_string(&sig_bits_elem, &sig_ptr, &sig_bit_len)) {
        size_t byte_len = sig_bit_len / 8;
        if (byte_len <= sizeof(cert_out->signature)) {
            memcpy(cert_out->signature, sig_ptr, byte_len);
            cert_out->signature_len = byte_len;
        }
    }

    cert_out->sig_algo = cert_out->pubkey_algo;
    return true;
}

bool syn_x509_verify_signature(const SYN_X509_Cert *cert, const uint8_t *issuer_pubkey,
                               size_t issuer_pubkey_len, SYN_X509_Algo algo)
{
    if (cert == NULL || issuer_pubkey == NULL || cert->tbs_bytes == NULL || cert->tbs_len == 0) {
        return false;
    }

    if (algo == SYN_X509_ALGO_ED25519) {
        if (issuer_pubkey_len != SYN_ED25519_PUBLIC_KEY_SIZE ||
            cert->signature_len != SYN_ED25519_SIGNATURE_SIZE) {
            return false; /* LCOV_EXCL_LINE: Defensive check if Ed25519 public key or signature
                             length is invalid */
        }
        return syn_ed25519_verify(cert->signature, cert->tbs_bytes, cert->tbs_len, issuer_pubkey);
    }

    if (algo == SYN_X509_ALGO_ECDSA_P256) {
        const uint8_t *px = issuer_pubkey;
        const uint8_t *py = issuer_pubkey + 32;
        if (issuer_pubkey_len == 65 && issuer_pubkey[0] == 0x04) {
            px = issuer_pubkey + 1;
            py = issuer_pubkey + 33;
        } else if (issuer_pubkey_len != 64) {
            return false;
        }

        if (cert->signature_len != 64) {
            return false;
        }

        uint8_t hash[SYN_SHA256_DIGEST_SIZE];
        syn_sha256(cert->tbs_bytes, cert->tbs_len, hash);

        return syn_p256_verify_ecdsa(hash, cert->signature, cert->signature + 32, px, py);
    }

    return false;
}

bool syn_x509_validate_chain(const SYN_X509_Cert *cert, const SYN_X509_Cert *root_ca,
                             const char *expected_cn)
{
    if (cert == NULL || root_ca == NULL) {
        return false;
    }

    if (expected_cn != NULL) {
        if (strcmp(cert->subject_cn, expected_cn) != 0) {
            return false;
        }
    }

    return syn_x509_verify_signature(cert, root_ca->pubkey, root_ca->pubkey_len,
                                     root_ca->pubkey_algo);
}
```


