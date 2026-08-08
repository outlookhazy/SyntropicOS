

# File syn\_hkdf.c

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_hkdf.c**](syn__hkdf_8c.md)

[Go to the documentation of this file](syn__hkdf_8c.md)


```C++

#include "syntropic/crypto/syn_hkdf.h"

#include <string.h>

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

    uint8_t t[SYN_SHA256_DIGEST_SIZE];
    size_t t_len = 0;
    size_t written = 0;
    uint8_t counter = 1;

    while (written < okm_len) {
        SYN_HMAC_SHA256 hmac;
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

    return true;
}

bool syn_hkdf(const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len,
              const uint8_t *info, size_t info_len, uint8_t *okm_out, size_t okm_len)
{
    uint8_t prk[SYN_SHA256_DIGEST_SIZE];
    syn_hkdf_extract(salt, salt_len, ikm, ikm_len, prk);
    return syn_hkdf_expand(prk, sizeof(prk), info, info_len, okm_out, okm_len);
}

bool syn_hkdf_expand_label(const uint8_t *secret, size_t secret_len, const char *label,
                           size_t label_len, const uint8_t *context, size_t context_len,
                           uint8_t *out, size_t out_len)
{
    if (secret == NULL || label == NULL || out == NULL || out_len == 0) {
        return false;
    }

    static const char prefix[] = "tls13 ";
    size_t prefix_len = 6;
    size_t full_label_len = prefix_len + label_len;

    if (full_label_len > 255) {
        return false;
    }

    if (context == NULL && context_len > 0) {
        context_len = 0;
    }

    if (context_len > 255) {
        return false;
    }

    /* HkdfLabel structure buffer:
     * uint16 length (2B) + uint8 label_len (1B) + label (full_label_len) +
     * uint8 context_len (1B) + context (context_len)
     */
    uint8_t hkdf_label[512];
    size_t offset = 0;

    hkdf_label[offset++] = (uint8_t)((out_len >> 8) & 0xFF);
    hkdf_label[offset++] = (uint8_t)(out_len & 0xFF);

    hkdf_label[offset++] = (uint8_t)full_label_len;
    memcpy(hkdf_label + offset, prefix, prefix_len);
    offset += prefix_len;
    memcpy(hkdf_label + offset, label, label_len);
    offset += label_len;

    hkdf_label[offset++] = (uint8_t)context_len;
    if (context_len > 0) {
        memcpy(hkdf_label + offset, context, context_len);
        offset += context_len;
    }

    return syn_hkdf_expand(secret, secret_len, hkdf_label, offset, out, out_len);
}
```


