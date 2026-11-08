/**
 * @file syn_hkdf.c
 * @brief HMAC-based Extract-and-Expand Key Derivation Function (HKDF-SHA256, RFC 5869).
 */

#include "syn_hkdf.h"

#if SYN_HKDF_ENABLED

#include <string.h>

/**
 * @brief Zeroize sensitive stack buffers without compiler dead-store elimination.
 * @param v Buffer pointer.
 * @param n Number of bytes to zeroize.
 */
static void syn_secure_zero(void *v, size_t n)
{
    volatile uint8_t *p = (volatile uint8_t *)v;
    while (n-- > 0U) {
        *p++ = 0U;
    }
}

void syn_hkdf_extract(const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len,
                      uint8_t prk_out[SYN_SHA256_DIGEST_SIZE])
{
    static const uint8_t zero_salt[SYN_SHA256_DIGEST_SIZE] = {0};

    if (prk_out == NULL) {
        return;
    }

    if (salt == NULL || salt_len == 0) {
        salt = zero_salt;
        salt_len = SYN_SHA256_DIGEST_SIZE;
    }

    if (ikm == NULL && ikm_len > 0) {
        ikm_len = 0;
    }

    syn_hmac_sha256(salt, salt_len, ikm, ikm_len, prk_out);
}

bool syn_hkdf_expand(const uint8_t *prk, size_t prk_len, const uint8_t *info, size_t info_len,
                     uint8_t *okm_out, size_t okm_len)
{
    if (prk == NULL || okm_out == NULL || prk_len < SYN_SHA256_DIGEST_SIZE) {
        return false;
    }

    if (okm_len == 0 || okm_len > (255U * SYN_SHA256_DIGEST_SIZE)) {
        return false;
    }

    if (info == NULL && info_len > 0) {
        info_len = 0;
    }

    uint8_t t[SYN_SHA256_DIGEST_SIZE] = {0};
    SYN_HMAC_SHA256 hmac;
    size_t t_len = 0;
    size_t written = 0;
    uint8_t counter = 1;

    while (written < okm_len) {
        syn_hmac_sha256_init(&hmac, prk, prk_len);

        if (counter > 1) {
            syn_hmac_sha256_update(&hmac, t, t_len);
        }

        if (info_len > 0) {
            syn_hmac_sha256_update(&hmac, info, info_len);
        }

        syn_hmac_sha256_update(&hmac, &counter, 1);
        syn_hmac_sha256_final(&hmac, t);
        t_len = SYN_SHA256_DIGEST_SIZE;

        size_t todo = okm_len - written;
        if (todo > SYN_SHA256_DIGEST_SIZE) {
            todo = SYN_SHA256_DIGEST_SIZE;
        }

        memcpy(okm_out + written, t, todo);
        written += todo;
        counter++;
    }

    syn_secure_zero(t, sizeof(t));
    syn_secure_zero(&hmac, sizeof(hmac));
    return true;
}

bool syn_hkdf(const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len,
              const uint8_t *info, size_t info_len, uint8_t *okm_out, size_t okm_len)
{
    uint8_t prk[SYN_SHA256_DIGEST_SIZE];
    syn_hkdf_extract(salt, salt_len, ikm, ikm_len, prk);
    bool ok = syn_hkdf_expand(prk, sizeof(prk), info, info_len, okm_out, okm_len);
    syn_secure_zero(prk, sizeof(prk));
    return ok;
}

bool syn_hkdf_expand_label(const uint8_t *secret, size_t secret_len, const char *label,
                           size_t label_len, const uint8_t *context, size_t context_len,
                           uint8_t *out, size_t out_len)
{
    if (secret == NULL || label == NULL || out == NULL || out_len == 0U || out_len > 0xFFFFU) {
        return false;
    }

    static const char prefix[] = "tls13 ";
    size_t prefix_len = 6U;

    if (label_len > (255U - prefix_len)) {
        return false;
    }

    size_t full_label_len = prefix_len + label_len;

    if (context == NULL && context_len > 0U) {
        context_len = 0U;
    }

    if (context_len > 255U) {
        return false;
    }

    /* HkdfLabel structure buffer:
     * uint16 length (2B) + uint8 label_len (1B) + label (full_label_len) +
     * uint8 context_len (1B) + context (context_len)
     */
    uint8_t hkdf_label[516];
    size_t offset = 0U;

    hkdf_label[offset++] = (uint8_t)((out_len >> 8U) & 0xFFU);
    hkdf_label[offset++] = (uint8_t)(out_len & 0xFFU);

    hkdf_label[offset++] = (uint8_t)full_label_len;
    memcpy(hkdf_label + offset, prefix, prefix_len);
    offset += prefix_len;
    memcpy(hkdf_label + offset, label, label_len);
    offset += label_len;

    hkdf_label[offset++] = (uint8_t)context_len;
    if (context_len > 0U) {
        memcpy(hkdf_label + offset, context, context_len);
        offset += context_len;
    }

    bool ok = syn_hkdf_expand(secret, secret_len, hkdf_label, offset, out, out_len);
    syn_secure_zero(hkdf_label, sizeof(hkdf_label));
    return ok;
}

#if SYN_HKDF_SHA512_ENABLED

/* ── HKDF-SHA384 ─────────────────────────────────────────────────────────── */

void syn_hkdf_sha384_extract(const uint8_t *salt, size_t salt_len, const uint8_t *ikm,
                             size_t ikm_len, uint8_t prk_out[SYN_SHA384_DIGEST_SIZE])
{
    static const uint8_t zero_salt[SYN_SHA384_DIGEST_SIZE] = {0};

    if (prk_out == NULL) {
        return;
    }

    if (salt == NULL || salt_len == 0U) {
        salt = zero_salt;
        salt_len = SYN_SHA384_DIGEST_SIZE;
    }

    if (ikm == NULL && ikm_len > 0U) {
        ikm_len = 0U;
    }

    syn_hmac_sha384(salt, salt_len, ikm, ikm_len, prk_out);
}

bool syn_hkdf_sha384_expand(const uint8_t *prk, size_t prk_len, const uint8_t *info,
                            size_t info_len, uint8_t *okm_out, size_t okm_len)
{
    if (prk == NULL || okm_out == NULL || prk_len < SYN_SHA384_DIGEST_SIZE) {
        return false;
    }

    if (okm_len == 0U || okm_len > (255U * SYN_SHA384_DIGEST_SIZE)) {
        return false;
    }

    if (info == NULL && info_len > 0U) {
        info_len = 0U;
    }

    uint8_t t[SYN_SHA384_DIGEST_SIZE] = {0};
    SYN_HMAC_SHA384 hmac;
    size_t t_len = 0U;
    size_t written = 0U;
    uint8_t counter = 1U;

    while (written < okm_len) {
        syn_hmac_sha384_init(&hmac, prk, prk_len);

        if (counter > 1U) {
            syn_hmac_sha384_update(&hmac, t, t_len);
        }

        if (info_len > 0U) {
            syn_hmac_sha384_update(&hmac, info, info_len);
        }

        syn_hmac_sha384_update(&hmac, &counter, 1U);
        syn_hmac_sha384_final(&hmac, t);
        t_len = SYN_SHA384_DIGEST_SIZE;

        size_t todo = okm_len - written;
        if (todo > SYN_SHA384_DIGEST_SIZE) {
            todo = SYN_SHA384_DIGEST_SIZE;
        }

        memcpy(okm_out + written, t, todo);
        written += todo;
        counter++;
    }

    syn_secure_zero(t, sizeof(t));
    syn_secure_zero(&hmac, sizeof(hmac));
    return true;
}

bool syn_hkdf_sha384(const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len,
                     const uint8_t *info, size_t info_len, uint8_t *okm_out, size_t okm_len)
{
    uint8_t prk[SYN_SHA384_DIGEST_SIZE];
    syn_hkdf_sha384_extract(salt, salt_len, ikm, ikm_len, prk);
    bool ok = syn_hkdf_sha384_expand(prk, sizeof(prk), info, info_len, okm_out, okm_len);
    syn_secure_zero(prk, sizeof(prk));
    return ok;
}

bool syn_hkdf_sha384_expand_label(const uint8_t *secret, size_t secret_len, const char *label,
                                  size_t label_len, const uint8_t *context, size_t context_len,
                                  uint8_t *out, size_t out_len)
{
    if (secret == NULL || label == NULL || out == NULL || out_len == 0U || out_len > 0xFFFFU) {
        return false;
    }

    static const char prefix[] = "tls13 ";
    size_t prefix_len = 6U;

    if (label_len > (255U - prefix_len)) {
        return false;
    }

    size_t full_label_len = prefix_len + label_len;

    if (context == NULL && context_len > 0U) {
        context_len = 0U;
    }

    if (context_len > 255U) {
        return false;
    }

    uint8_t hkdf_label[516];
    size_t offset = 0U;

    hkdf_label[offset++] = (uint8_t)((out_len >> 8U) & 0xFFU);
    hkdf_label[offset++] = (uint8_t)(out_len & 0xFFU);

    hkdf_label[offset++] = (uint8_t)full_label_len;
    memcpy(hkdf_label + offset, prefix, prefix_len);
    offset += prefix_len;
    memcpy(hkdf_label + offset, label, label_len);
    offset += label_len;

    hkdf_label[offset++] = (uint8_t)context_len;
    if (context_len > 0U) {
        memcpy(hkdf_label + offset, context, context_len);
        offset += context_len;
    }

    bool ok = syn_hkdf_sha384_expand(secret, secret_len, hkdf_label, offset, out, out_len);
    syn_secure_zero(hkdf_label, sizeof(hkdf_label));
    return ok;
}

/* ── HKDF-SHA512 ─────────────────────────────────────────────────────────── */

void syn_hkdf_sha512_extract(const uint8_t *salt, size_t salt_len, const uint8_t *ikm,
                             size_t ikm_len, uint8_t prk_out[SYN_SHA512_DIGEST_SIZE])
{
    static const uint8_t zero_salt[SYN_SHA512_DIGEST_SIZE] = {0};

    if (prk_out == NULL) {
        return;
    }

    if (salt == NULL || salt_len == 0U) {
        salt = zero_salt;
        salt_len = SYN_SHA512_DIGEST_SIZE;
    }

    if (ikm == NULL && ikm_len > 0U) {
        ikm_len = 0U;
    }

    syn_hmac_sha512(salt, salt_len, ikm, ikm_len, prk_out);
}

bool syn_hkdf_sha512_expand(const uint8_t *prk, size_t prk_len, const uint8_t *info,
                            size_t info_len, uint8_t *okm_out, size_t okm_len)
{
    if (prk == NULL || okm_out == NULL || prk_len < SYN_SHA512_DIGEST_SIZE) {
        return false;
    }

    if (okm_len == 0U || okm_len > (255U * SYN_SHA512_DIGEST_SIZE)) {
        return false;
    }

    if (info == NULL && info_len > 0U) {
        info_len = 0U;
    }

    uint8_t t[SYN_SHA512_DIGEST_SIZE] = {0};
    SYN_HMAC_SHA512 hmac;
    size_t t_len = 0U;
    size_t written = 0U;
    uint8_t counter = 1U;

    while (written < okm_len) {
        syn_hmac_sha512_init(&hmac, prk, prk_len);

        if (counter > 1U) {
            syn_hmac_sha512_update(&hmac, t, t_len);
        }

        if (info_len > 0U) {
            syn_hmac_sha512_update(&hmac, info, info_len);
        }

        syn_hmac_sha512_update(&hmac, &counter, 1U);
        syn_hmac_sha512_final(&hmac, t);
        t_len = SYN_SHA512_DIGEST_SIZE;

        size_t todo = okm_len - written;
        if (todo > SYN_SHA512_DIGEST_SIZE) {
            todo = SYN_SHA512_DIGEST_SIZE;
        }

        memcpy(okm_out + written, t, todo);
        written += todo;
        counter++;
    }

    syn_secure_zero(t, sizeof(t));
    syn_secure_zero(&hmac, sizeof(hmac));
    return true;
}

bool syn_hkdf_sha512(const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len,
                     const uint8_t *info, size_t info_len, uint8_t *okm_out, size_t okm_len)
{
    uint8_t prk[SYN_SHA512_DIGEST_SIZE];
    syn_hkdf_sha512_extract(salt, salt_len, ikm, ikm_len, prk);
    bool ok = syn_hkdf_sha512_expand(prk, sizeof(prk), info, info_len, okm_out, okm_len);
    syn_secure_zero(prk, sizeof(prk));
    return ok;
}

bool syn_hkdf_sha512_expand_label(const uint8_t *secret, size_t secret_len, const char *label,
                                  size_t label_len, const uint8_t *context, size_t context_len,
                                  uint8_t *out, size_t out_len)
{
    if (secret == NULL || label == NULL || out == NULL || out_len == 0U || out_len > 0xFFFFU) {
        return false;
    }

    static const char prefix[] = "tls13 ";
    size_t prefix_len = 6U;

    if (label_len > (255U - prefix_len)) {
        return false;
    }

    size_t full_label_len = prefix_len + label_len;

    if (context == NULL && context_len > 0U) {
        context_len = 0U;
    }

    if (context_len > 255U) {
        return false;
    }

    uint8_t hkdf_label[516];
    size_t offset = 0U;

    hkdf_label[offset++] = (uint8_t)((out_len >> 8U) & 0xFFU);
    hkdf_label[offset++] = (uint8_t)(out_len & 0xFFU);

    hkdf_label[offset++] = (uint8_t)full_label_len;
    memcpy(hkdf_label + offset, prefix, prefix_len);
    offset += prefix_len;
    memcpy(hkdf_label + offset, label, label_len);
    offset += label_len;

    hkdf_label[offset++] = (uint8_t)context_len;
    if (context_len > 0U) {
        memcpy(hkdf_label + offset, context, context_len);
        offset += context_len;
    }

    bool ok = syn_hkdf_sha512_expand(secret, secret_len, hkdf_label, offset, out, out_len);
    syn_secure_zero(hkdf_label, sizeof(hkdf_label));
    return ok;
}

#endif /* SYN_HKDF_SHA512_ENABLED */

#endif /* SYN_HKDF_ENABLED */
