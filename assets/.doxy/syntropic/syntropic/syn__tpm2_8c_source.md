

# File syn\_tpm2.c

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_tpm2.c**](syn__tpm2_8c.md)

[Go to the documentation of this file](syn__tpm2_8c.md)


```C++

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_TPM2) || SYN_USE_TPM2

#include "../port/syn_port_system.h"
#include "syn_tpm2.h"

#include <string.h>

/* ── Internal Big-Endian Marshalling Helpers ─────────────────────────────── */

static bool tpm2_write_u16(uint8_t *buf, size_t *pos, size_t max_len, uint16_t v)
{
    if (*pos + 2U > max_len) {
        return false;
    }
    buf[*pos] = (uint8_t)((v >> 8U) & 0xFFU);
    buf[*pos + 1U] = (uint8_t)(v & 0xFFU);
    *pos += 2U;
    return true;
}

static bool tpm2_write_u32(uint8_t *buf, size_t *pos, size_t max_len, uint32_t v)
{
    if (*pos + 4U > max_len) {
        return false;
    }
    buf[*pos] = (uint8_t)((v >> 24U) & 0xFFU);
    buf[*pos + 1U] = (uint8_t)((v >> 16U) & 0xFFU);
    buf[*pos + 2U] = (uint8_t)((v >> 8U) & 0xFFU);
    buf[*pos + 3U] = (uint8_t)(v & 0xFFU);
    *pos += 4U;
    return true;
}

static bool tpm2_write_bytes(uint8_t *buf, size_t *pos, size_t max_len, const uint8_t *data,
                             size_t len)
{
    if (*pos + len > max_len) {
        return false;
    }
    if (len > 0U && data != NULL) {
        (void)memcpy(&buf[*pos], data, len);
    }
    *pos += len;
    return true;
}

static bool tpm2_read_u16(const uint8_t *buf, size_t *pos, size_t max_len, uint16_t *out_v)
{
    if (*pos + 2U > max_len) {
        return false;
    }
    if (out_v != NULL) {
        *out_v = ((uint16_t)buf[*pos] << 8U) | (uint16_t)buf[*pos + 1U];
    }
    *pos += 2U;
    return true;
}

static bool tpm2_read_u32(const uint8_t *buf, size_t *pos, size_t max_len, uint32_t *out_v)
{
    if (*pos + 4U > max_len) {
        return false;
    }
    if (out_v != NULL) {
        *out_v = ((uint32_t)buf[*pos] << 24U) | ((uint32_t)buf[*pos + 1U] << 16U) |
                 ((uint32_t)buf[*pos + 2U] << 8U) | (uint32_t)buf[*pos + 3U];
    }
    *pos += 4U;
    return true;
}

static bool tpm2_read_bytes(const uint8_t *buf, size_t *pos, size_t max_len, uint8_t *out_data,
                            size_t len)
{
    if (*pos + len > max_len) {
        return false;
    }
    if (len > 0U && out_data != NULL) {
        (void)memcpy(out_data, &buf[*pos], len);
    }
    *pos += len;
    return true;
}

static SYN_Status tpm2_execute_command(SYN_TPM2_Context *ctx, size_t cmd_len, size_t *out_resp_len)
{
    /* Patch paramSize in command header (bytes 2..5) */
    ctx->cfg.tx_buf[2] = (uint8_t)((cmd_len >> 24U) & 0xFFU);
    ctx->cfg.tx_buf[3] = (uint8_t)((cmd_len >> 16U) & 0xFFU);
    ctx->cfg.tx_buf[4] = (uint8_t)((cmd_len >> 8U) & 0xFFU);
    ctx->cfg.tx_buf[5] = (uint8_t)(cmd_len & 0xFFU);

    if (!syn_transport_send(ctx->cfg.transport, ctx->cfg.tx_buf, cmd_len)) {
        return SYN_ERROR;
    }

    size_t rx_len = 0U;
    if (!syn_transport_recv(ctx->cfg.transport, ctx->cfg.rx_buf, ctx->cfg.rx_buf_size, &rx_len) ||
        rx_len < 10U) {
        return SYN_ERROR;
    }

    size_t rpos = 0U;
    uint16_t tag = 0U;
    uint32_t param_size = 0U;
    uint32_t rc = 0U;

    (void)tpm2_read_u16(ctx->cfg.rx_buf, &rpos, rx_len, &tag);
    (void)tpm2_read_u32(ctx->cfg.rx_buf, &rpos, rx_len, &param_size);
    (void)tpm2_read_u32(ctx->cfg.rx_buf, &rpos, rx_len, &rc);

    ctx->last_rc = rc;
    if ((tag != SYN_TPM2_ST_NO_SESSIONS && tag != SYN_TPM2_ST_SESSIONS) ||
        rc != SYN_TPM2_RC_SUCCESS || param_size > rx_len) {
        return SYN_ERROR;
    }

    if (out_resp_len != NULL) {
        *out_resp_len = rx_len;
    }
    return SYN_OK;
}

static bool tpm2_write_pw_auth_session(uint8_t *tx, size_t *pos, size_t max_len)
{
    /* Auth area size = 9 bytes (Handle 4 + Nonce 2 + Attr 1 + Password 2) */
    if (!tpm2_write_u32(tx, pos, max_len, 9U)) {
        return false;
    }
    if (!tpm2_write_u32(tx, pos, max_len, SYN_TPM2_RS_PW)) {
        return false;
    }
    if (!tpm2_write_u16(tx, pos, max_len, 0U)) { /* Empty nonce */
        return false;
    }
    if (*pos + 1U > max_len) {
        return false;
    }
    tx[(*pos)++] = 0U;                           /* Session attributes */
    return tpm2_write_u16(tx, pos, max_len, 0U); /* Empty password */
}

/* ── Public API Implementation ───────────────────────────────────────────── */

SYN_Status syn_tpm2_init(SYN_TPM2_Context *ctx, const SYN_TPM2_Config *cfg)
{
    if (ctx == NULL || cfg == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (cfg->transport == NULL || cfg->rx_buf == NULL || cfg->tx_buf == NULL ||
        cfg->rx_buf_size < 256U || cfg->tx_buf_size < 256U) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(ctx, 0, sizeof(*ctx));
    ctx->cfg = *cfg;
    ctx->initialized = true;
    return SYN_OK;
}

SYN_Status syn_tpm2_startup(SYN_TPM2_Context *ctx, uint16_t startup_type)
{
    if (ctx == NULL || !ctx->initialized) {
        return SYN_INVALID_PARAM;
    }

    uint8_t *tx = ctx->cfg.tx_buf;
    size_t pos = 0U;
    size_t max_len = ctx->cfg.tx_buf_size;

    (void)tpm2_write_u16(tx, &pos, max_len, SYN_TPM2_ST_NO_SESSIONS);
    (void)tpm2_write_u32(tx, &pos, max_len, 0U); /* Placeholder for paramSize */
    (void)tpm2_write_u32(tx, &pos, max_len, SYN_TPM2_CC_STARTUP);
    (void)tpm2_write_u16(tx, &pos, max_len, startup_type);

    return tpm2_execute_command(ctx, pos, NULL);
}

SYN_Status syn_tpm2_self_test(SYN_TPM2_Context *ctx, bool full_test)
{
    if (ctx == NULL || !ctx->initialized) {
        return SYN_INVALID_PARAM;
    }

    uint8_t *tx = ctx->cfg.tx_buf;
    size_t pos = 0U;
    size_t max_len = ctx->cfg.tx_buf_size;

    (void)tpm2_write_u16(tx, &pos, max_len, SYN_TPM2_ST_NO_SESSIONS);
    (void)tpm2_write_u32(tx, &pos, max_len, 0U);
    (void)tpm2_write_u32(tx, &pos, max_len, SYN_TPM2_CC_SELFTEST);
    if (pos + 1U > max_len) {
        return SYN_ERROR;
    }
    tx[pos++] = full_test ? 1U : 0U;

    return tpm2_execute_command(ctx, pos, NULL);
}

SYN_Status syn_tpm2_get_random(SYN_TPM2_Context *ctx, uint16_t num_bytes, uint8_t *out_random,
                               uint16_t *out_len)
{
    if (ctx == NULL || !ctx->initialized || out_random == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t *tx = ctx->cfg.tx_buf;
    size_t pos = 0U;
    size_t max_len = ctx->cfg.tx_buf_size;

    (void)tpm2_write_u16(tx, &pos, max_len, SYN_TPM2_ST_NO_SESSIONS);
    (void)tpm2_write_u32(tx, &pos, max_len, 0U);
    (void)tpm2_write_u32(tx, &pos, max_len, SYN_TPM2_CC_GETRANDOM);
    (void)tpm2_write_u16(tx, &pos, max_len, num_bytes);

    size_t resp_len = 0U;
    if (tpm2_execute_command(ctx, pos, &resp_len) != SYN_OK) {
        return SYN_ERROR;
    }

    size_t rpos = 10U; /* Skip Tag, Size, RC */
    uint16_t rand_len = 0U;
    if (!tpm2_read_u16(ctx->cfg.rx_buf, &rpos, resp_len, &rand_len) ||
        !tpm2_read_bytes(ctx->cfg.rx_buf, &rpos, resp_len, out_random, rand_len)) {
        return SYN_ERROR;
    }

    if (out_len != NULL) {
        *out_len = rand_len;
    }
    return SYN_OK;
}

SYN_Status syn_tpm2_pcr_read(SYN_TPM2_Context *ctx, uint32_t pcr_index, uint16_t hash_alg,
                             uint8_t *out_digest, size_t *out_digest_len)
{
    if (ctx == NULL || !ctx->initialized || out_digest == NULL || pcr_index > 23U) {
        return SYN_INVALID_PARAM;
    }

    uint8_t *tx = ctx->cfg.tx_buf;
    size_t pos = 0U;
    size_t max_len = ctx->cfg.tx_buf_size;

    (void)tpm2_write_u16(tx, &pos, max_len, SYN_TPM2_ST_NO_SESSIONS);
    (void)tpm2_write_u32(tx, &pos, max_len, 0U);
    (void)tpm2_write_u32(tx, &pos, max_len, SYN_TPM2_CC_PCR_READ);

    /* TPML_PCR_SELECTION: count = 1 */
    (void)tpm2_write_u32(tx, &pos, max_len, 1U);
    (void)tpm2_write_u16(tx, &pos, max_len, hash_alg);
    if (pos + 4U > max_len) {
        return SYN_ERROR;
    }
    tx[pos++] = 3U; /* sizeofSelect = 3 bytes (24 PCRs) */
    tx[pos++] = (uint8_t)((1U << (pcr_index & 7U)) * (pcr_index < 8U ? 1U : 0U));
    tx[pos++] =
        (uint8_t)((1U << (pcr_index & 7U)) * ((pcr_index >= 8U && pcr_index < 16U) ? 1U : 0U));
    tx[pos++] = (uint8_t)((1U << (pcr_index & 7U)) * (pcr_index >= 16U ? 1U : 0U));

    size_t resp_len = 0U;
    if (tpm2_execute_command(ctx, pos, &resp_len) != SYN_OK) {
        return SYN_ERROR;
    }

    size_t rpos = 10U;
    uint32_t pcr_update_counter = 0U;
    (void)tpm2_read_u32(ctx->cfg.rx_buf, &rpos, resp_len, &pcr_update_counter);

    /* Skip selection echoes in response */
    uint32_t sel_count = 0U;
    (void)tpm2_read_u32(ctx->cfg.rx_buf, &rpos, resp_len, &sel_count);
    for (uint32_t i = 0U; i < sel_count; i++) {
        uint16_t alg = 0U;
        (void)tpm2_read_u16(ctx->cfg.rx_buf, &rpos, resp_len, &alg);
        if (rpos >= resp_len) {
            return SYN_ERROR;
        }
        uint8_t sz = ctx->cfg.rx_buf[rpos++];
        rpos += sz;
    }

    /* TPML_DIGEST: count, then TPM2B_DIGEST */
    uint32_t digest_count = 0U;
    (void)tpm2_read_u32(ctx->cfg.rx_buf, &rpos, resp_len, &digest_count);
    if (digest_count == 0U) {
        return SYN_ERROR;
    }

    uint16_t dlen = 0U;
    if (!tpm2_read_u16(ctx->cfg.rx_buf, &rpos, resp_len, &dlen) ||
        !tpm2_read_bytes(ctx->cfg.rx_buf, &rpos, resp_len, out_digest, dlen)) {
        return SYN_ERROR;
    }

    if (out_digest_len != NULL) {
        *out_digest_len = dlen;
    }
    return SYN_OK;
}

SYN_Status syn_tpm2_pcr_extend(SYN_TPM2_Context *ctx, uint32_t pcr_index, uint16_t hash_alg,
                               const uint8_t *in_digest, size_t digest_len)
{
    if (ctx == NULL || !ctx->initialized || in_digest == NULL || pcr_index > 23U ||
        digest_len == 0U) {
        return SYN_INVALID_PARAM;
    }

    uint8_t *tx = ctx->cfg.tx_buf;
    size_t pos = 0U;
    size_t max_len = ctx->cfg.tx_buf_size;

    (void)tpm2_write_u16(tx, &pos, max_len, SYN_TPM2_ST_SESSIONS);
    (void)tpm2_write_u32(tx, &pos, max_len, 0U);
    (void)tpm2_write_u32(tx, &pos, max_len, SYN_TPM2_CC_PCR_EXTEND);
    (void)tpm2_write_u32(tx, &pos, max_len, pcr_index);

    /* Session area */
    if (!tpm2_write_pw_auth_session(tx, &pos, max_len)) {
        return SYN_ERROR;
    }

    /* TPML_DIGEST_VALUES: count = 1, TPMT_HA (hashAlg, digest) */
    if (!tpm2_write_u32(tx, &pos, max_len, 1U) || !tpm2_write_u16(tx, &pos, max_len, hash_alg) ||
        !tpm2_write_bytes(tx, &pos, max_len, in_digest, digest_len)) {
        return SYN_ERROR;
    }

    return tpm2_execute_command(ctx, pos, NULL);
}

SYN_Status syn_tpm2_quote(SYN_TPM2_Context *ctx, uint32_t key_handle,
                          const uint8_t *qualifying_data, size_t qual_len, uint32_t pcr_mask,
                          SYN_TPM2_QuoteResult *out_quote)
{
    if (ctx == NULL || !ctx->initialized || out_quote == NULL ||
        (qualifying_data == NULL && qual_len > 0U)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t *tx = ctx->cfg.tx_buf;
    size_t pos = 0U;
    size_t max_len = ctx->cfg.tx_buf_size;

    (void)tpm2_write_u16(tx, &pos, max_len, SYN_TPM2_ST_SESSIONS);
    (void)tpm2_write_u32(tx, &pos, max_len, 0U);
    (void)tpm2_write_u32(tx, &pos, max_len, SYN_TPM2_CC_QUOTE);
    (void)tpm2_write_u32(tx, &pos, max_len, key_handle);

    /* Session Area */
    if (!tpm2_write_pw_auth_session(tx, &pos, max_len)) {
        return SYN_ERROR;
    }

    /* TPM2B_DATA qualifyingData */
    if (!tpm2_write_u16(tx, &pos, max_len, (uint16_t)qual_len) ||
        !tpm2_write_bytes(tx, &pos, max_len, qualifying_data, qual_len)) {
        return SYN_ERROR;
    }

    /* TPMT_SIG_SCHEME (scheme = TPM_ALG_NULL) */
    if (!tpm2_write_u16(tx, &pos, max_len, SYN_TPM2_ALG_NULL)) {
        return SYN_ERROR;
    }

    /* TPML_PCR_SELECTION */
    if (!tpm2_write_u32(tx, &pos, max_len, 1U) ||
        !tpm2_write_u16(tx, &pos, max_len, SYN_TPM2_ALG_SHA256) || (pos + 4U > max_len)) {
        return SYN_ERROR;
    }
    tx[pos++] = 3U;
    tx[pos++] = (uint8_t)(pcr_mask & 0xFFU);
    tx[pos++] = (uint8_t)((pcr_mask >> 8U) & 0xFFU);
    tx[pos++] = (uint8_t)((pcr_mask >> 16U) & 0xFFU);

    size_t resp_len = 0U;
    if (tpm2_execute_command(ctx, pos, &resp_len) != SYN_OK) {
        return SYN_ERROR;
    }

    size_t rpos = 10U;
    /* Skip parameter size if present in sessions response */
    uint32_t param_sz = 0U;
    (void)tpm2_read_u32(ctx->cfg.rx_buf, &rpos, resp_len, &param_sz);

    /* TPM2B_ATTEST quoted data */
    uint16_t attest_len = 0U;
    if (!tpm2_read_u16(ctx->cfg.rx_buf, &rpos, resp_len, &attest_len) ||
        attest_len > SYN_TPM2_MAX_QUOTE_LEN ||
        !tpm2_read_bytes(ctx->cfg.rx_buf, &rpos, resp_len, out_quote->attest_data, attest_len)) {
        return SYN_ERROR;
    }
    out_quote->attest_len = attest_len;

    /* TPMT_SIGNATURE signature */
    uint16_t sig_alg = 0U;
    (void)tpm2_read_u16(ctx->cfg.rx_buf, &rpos, resp_len, &sig_alg);
    uint16_t sig_len = 0U;
    if (!tpm2_read_u16(ctx->cfg.rx_buf, &rpos, resp_len, &sig_len) ||
        sig_len > SYN_TPM2_MAX_QUOTE_LEN ||
        !tpm2_read_bytes(ctx->cfg.rx_buf, &rpos, resp_len, out_quote->signature, sig_len)) {
        return SYN_ERROR;
    }
    out_quote->signature_len = sig_len;

    return SYN_OK;
}

SYN_Status syn_tpm2_nv_read(SYN_TPM2_Context *ctx, uint32_t auth_handle, uint32_t nv_index,
                            uint16_t offset, uint16_t size, uint8_t *out_data, uint16_t *out_len)
{
    if (ctx == NULL || !ctx->initialized || out_data == NULL || size == 0U) {
        return SYN_INVALID_PARAM;
    }

    uint8_t *tx = ctx->cfg.tx_buf;
    size_t pos = 0U;
    size_t max_len = ctx->cfg.tx_buf_size;

    (void)tpm2_write_u16(tx, &pos, max_len, SYN_TPM2_ST_SESSIONS);
    (void)tpm2_write_u32(tx, &pos, max_len, 0U);
    (void)tpm2_write_u32(tx, &pos, max_len, SYN_TPM2_CC_NV_READ);
    (void)tpm2_write_u32(tx, &pos, max_len, auth_handle);
    (void)tpm2_write_u32(tx, &pos, max_len, nv_index);

    if (!tpm2_write_pw_auth_session(tx, &pos, max_len)) {
        return SYN_ERROR;
    }

    if (!tpm2_write_u16(tx, &pos, max_len, size) || !tpm2_write_u16(tx, &pos, max_len, offset)) {
        return SYN_ERROR;
    }

    size_t resp_len = 0U;
    if (tpm2_execute_command(ctx, pos, &resp_len) != SYN_OK) {
        return SYN_ERROR;
    }

    size_t rpos = 10U;
    uint32_t param_sz = 0U;
    (void)tpm2_read_u32(ctx->cfg.rx_buf, &rpos, resp_len, &param_sz);

    uint16_t read_len = 0U;
    if (!tpm2_read_u16(ctx->cfg.rx_buf, &rpos, resp_len, &read_len) ||
        !tpm2_read_bytes(ctx->cfg.rx_buf, &rpos, resp_len, out_data, read_len)) {
        return SYN_ERROR;
    }

    if (out_len != NULL) {
        *out_len = read_len;
    }
    return SYN_OK;
}

SYN_Status syn_tpm2_nv_write(SYN_TPM2_Context *ctx, uint32_t auth_handle, uint32_t nv_index,
                             uint16_t offset, const uint8_t *in_data, uint16_t size)
{
    if (ctx == NULL || !ctx->initialized || in_data == NULL || size == 0U) {
        return SYN_INVALID_PARAM;
    }

    uint8_t *tx = ctx->cfg.tx_buf;
    size_t pos = 0U;
    size_t max_len = ctx->cfg.tx_buf_size;

    (void)tpm2_write_u16(tx, &pos, max_len, SYN_TPM2_ST_SESSIONS);
    (void)tpm2_write_u32(tx, &pos, max_len, 0U);
    (void)tpm2_write_u32(tx, &pos, max_len, SYN_TPM2_CC_NV_WRITE);
    (void)tpm2_write_u32(tx, &pos, max_len, auth_handle);
    (void)tpm2_write_u32(tx, &pos, max_len, nv_index);

    if (!tpm2_write_pw_auth_session(tx, &pos, max_len)) {
        return SYN_ERROR;
    }

    /* TPM2B_MAX_NV_BUFFER */
    if (!tpm2_write_u16(tx, &pos, max_len, size) ||
        !tpm2_write_bytes(tx, &pos, max_len, in_data, size) ||
        !tpm2_write_u16(tx, &pos, max_len, offset)) {
        return SYN_ERROR;
    }

    return tpm2_execute_command(ctx, pos, NULL);
}

uint32_t syn_tpm2_get_last_rc(const SYN_TPM2_Context *ctx)
{
    return (ctx != NULL) ? ctx->last_rc : 0U;
}

#endif /* !defined(SYN_USE_TPM2) || SYN_USE_TPM2 */
```


