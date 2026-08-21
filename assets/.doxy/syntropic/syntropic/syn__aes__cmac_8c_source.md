

# File syn\_aes\_cmac.c

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_aes\_cmac.c**](syn__aes__cmac_8c.md)

[Go to the documentation of this file](syn__aes__cmac_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_AES_CMAC) || SYN_USE_AES_CMAC

#include "syn_aes.h"
#include "syn_aes_cmac.h"

#include <string.h>

static void shl_128(const uint8_t in[16], uint8_t out[16])
{
    uint8_t carry = 0U;
    for (int i = 15; i >= 0; i--) {
        out[i] = (uint8_t)((in[i] << 1) | carry);
        carry = (uint8_t)((in[i] >> 7) & 1U);
    }
}

static void generate_subkeys(const SYN_AES_Context *ctx, uint8_t k1[16], uint8_t k2[16])
{
    const uint8_t zero_block[16] = {0};
    uint8_t l[16];

    /* Step 1: L := AES(K, const_Zero) */
    syn_aes_encrypt_block(ctx, zero_block, l);

    /* Step 2: Generate K1 */
    shl_128(l, k1);
    if ((l[0] & 0x80U) != 0U) {
        k1[15] ^= 0x87U;
    }

    /* Step 3: Generate K2 */
    shl_128(k1, k2);
    if ((k1[0] & 0x80U) != 0U) {
        k2[15] ^= 0x87U;
    }
}

SYN_Status syn_aes_cmac(const uint8_t key[SYN_AES_CMAC_KEY_SIZE], const uint8_t *msg,
                        size_t msg_len, uint8_t mac[SYN_AES_CMAC_TAG_SIZE])
{
    if (key == NULL || mac == NULL || (msg == NULL && msg_len > 0U)) {
        return SYN_INVALID_PARAM;
    }

    SYN_AES_Context ctx;
    (void)syn_aes_init(&ctx, key, SYN_AES_CMAC_KEY_SIZE);

    uint8_t k1[16];
    uint8_t k2[16];
    generate_subkeys(&ctx, k1, k2);

    /* Step 2 & 3: Compute number of blocks and complete-block flag */
    size_t n = (msg_len + 15U) / 16U;
    bool is_complete = false;

    if (n == 0U) {
        n = 1U;
        is_complete = false;
    } else if ((msg_len % 16U) == 0U) {
        is_complete = true;
    } else {
        is_complete = false;
    }

    /* Step 4: Calculate M_last */
    uint8_t m_last[16];
    if (is_complete) {
        for (size_t i = 0U; i < 16U; i++) {
            m_last[i] = (uint8_t)(msg[(n - 1U) * 16U + i] ^ k1[i]);
        }
    } else {
        size_t rem = msg_len % 16U;
        for (size_t i = 0U; i < rem; i++) {
            m_last[i] = msg[(n - 1U) * 16U + i];
        }
        m_last[rem] = 0x80U;
        for (size_t i = rem + 1U; i < 16U; i++) {
            m_last[i] = 0x00U;
        }
        for (size_t i = 0U; i < 16U; i++) {
            m_last[i] ^= k2[i];
        }
    }

    /* Step 5 & 6: CBC-MAC iteration */
    uint8_t x[16] = {0};
    uint8_t y[16];

    for (size_t i = 0U; i + 1U < n; i++) {
        for (size_t j = 0U; j < 16U; j++) {
            y[j] = (uint8_t)(x[j] ^ msg[i * 16U + j]);
        }
        syn_aes_encrypt_block(&ctx, y, x);
    }

    for (size_t j = 0U; j < 16U; j++) {
        y[j] = (uint8_t)(m_last[j] ^ x[j]);
    }
    syn_aes_encrypt_block(&ctx, y, mac);

    return SYN_OK;
}

bool syn_aes_cmac_verify(const uint8_t key[SYN_AES_CMAC_KEY_SIZE], const uint8_t *msg,
                         size_t msg_len, const uint8_t mac[SYN_AES_CMAC_TAG_SIZE])
{
    if (key == NULL || mac == NULL || (msg == NULL && msg_len > 0U)) {
        return false;
    }

    uint8_t computed[SYN_AES_CMAC_TAG_SIZE];
    (void)syn_aes_cmac(key, msg, msg_len, computed);

    /* Constant-time comparison */
    uint8_t diff = 0U;
    for (size_t i = 0U; i < SYN_AES_CMAC_TAG_SIZE; i++) {
        diff |= (uint8_t)(computed[i] ^ mac[i]);
    }

    return (diff == 0U);
}

#endif /* SYN_USE_AES_CMAC */
```


