/**
 * @file syn_x509.h
 * @brief Zero-Heap X.509 v3 Certificate & Chain Parser and Validator.
 * @ingroup syn_crypto
 */

#ifndef SYN_X509_H
#define SYN_X509_H

#include "../common/syn_defs.h"
#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/crypto/syn_p256.h"

#if (!defined(SYN_USE_X509) || SYN_USE_X509) && (!defined(SYN_USE_ASN1) || SYN_USE_ASN1) && \
    (!defined(SYN_USE_SHA256) || SYN_USE_SHA256) && SYN_P256_ENABLED && SYN_ED25519_ENABLED
/** @brief Non-zero when X.509 and required ASN.1/hash/signature dependencies are enabled. */
#define SYN_X509_ENABLED 1
#else
/** @brief Non-zero when X.509 and required ASN.1/hash/signature dependencies are enabled. */
#define SYN_X509_ENABLED 0
#endif

#if SYN_X509_ENABLED

#include "syntropic/crypto/syn_asn1.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Maximum CommonName string length (128). */
#define SYN_X509_MAX_NAME_LEN 128U
/** @brief Maximum public key buffer length (128). */
#define SYN_X509_PUBKEY_MAX_LEN 128U
/** @brief Maximum signature buffer length (256). */
#define SYN_X509_SIG_MAX_LEN 256U

/** Public Key Algorithm Types in X.509 SubjectPublicKeyInfo */
typedef enum {
    SYN_X509_ALGO_UNKNOWN = 0,
    SYN_X509_ALGO_ED25519,
    SYN_X509_ALGO_ECDSA_P256,
    SYN_X509_ALGO_RSA_PSS
} SYN_X509_Algo;

/** Parsed X.509 Certificate Data Structure. */
typedef struct {
    const uint8_t *tbs_bytes; /**< TBSCertificate raw payload for signature checking */
    size_t tbs_len;           /**< TBSCertificate length */
    uint32_t version;         /**< Certificate version (1, 2, or 3) */
    const uint8_t *serial;    /**< Serial number bytes */
    size_t serial_len;        /**< Serial number length */

    SYN_X509_Algo pubkey_algo;               /**< Subject public key algorithm */
    uint8_t pubkey[SYN_X509_PUBKEY_MAX_LEN]; /**< Extracted raw public key bytes */
    size_t pubkey_len;                       /**< Raw public key length */

    SYN_X509_Algo sig_algo;                  /**< Signature algorithm */
    uint8_t signature[SYN_X509_SIG_MAX_LEN]; /**< Signature payload bytes */
    size_t signature_len;                    /**< Signature length */

    char subject_cn[SYN_X509_MAX_NAME_LEN]; /**< Subject Common Name (CN) string */
    char issuer_cn[SYN_X509_MAX_NAME_LEN];  /**< Issuer Common Name (CN) string */

    bool is_ca; /**< BasicConstraints: cA = TRUE */
} SYN_X509_Cert;

/**
 * @brief Parse a raw DER-encoded X.509 certificate.
 *
 * @param der      Raw DER bytes of certificate.
 * @param der_len  Length of DER bytes.
 * @param cert_out [out] Output parsed certificate struct.
 * @return true if valid X.509 certificate parsed successfully.
 */
bool syn_x509_parse(const uint8_t *der, size_t der_len, SYN_X509_Cert *cert_out);

/**
 * @brief Verify certificate signature against an issuer's public key.
 *
 * @param cert             Parsed child certificate to verify.
 * @param issuer_pubkey    Issuer's public key.
 * @param issuer_pubkey_len Issuer's public key length.
 * @param algo             Algorithm of issuer's public key.
 * @return true if signature is valid.
 */
bool syn_x509_verify_signature(const SYN_X509_Cert *cert, const uint8_t *issuer_pubkey,
                               size_t issuer_pubkey_len, SYN_X509_Algo algo);

/**
 * @brief Validate a certificate chain back to a trusted Root CA certificate.
 *
 * @param cert        Leaf certificate.
 * @param root_ca     Trusted Root CA certificate.
 * @param expected_cn Expected Server Name (SNI) to match against leaf CN/SAN.
 * @return true if chain is valid and trusted.
 */
bool syn_x509_validate_chain(const SYN_X509_Cert *cert, const SYN_X509_Cert *root_ca,
                             const char *expected_cn);

#ifdef __cplusplus
}
#endif

#endif /* SYN_X509_ENABLED */

#endif /* SYN_X509_H */
