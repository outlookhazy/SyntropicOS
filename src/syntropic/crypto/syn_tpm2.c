/**
 * @file syn_tpm2.c
 * @brief TCG TPM 2.0 Command Marshaller & Measured Boot Engine Implementation.
 */

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_TPM2) || SYN_USE_TPM2

#include "../port/syn_port_system.h"
#include "syn_tpm2.h"

#include <string.h>

/** @brief TPM 2.0 RSASSA-PKCS1-v1_5 signature algorithm ID. */
#define TPM2_ALG_RSASSA 0x0014U
/** @brief TPM 2.0 RSASSA-PSS signature algorithm ID. */
#define TPM2_ALG_RSAPSS 0x0016U
/** @brief TPM 2.0 ECDSA signature algorithm ID. */
#define TPM2_ALG_ECDSA 0x0018U

/* ── Internal Big-Endian Marshalling Helpers ─────────────────────────────── */

/**
 * @brief Write 16-bit unsigned integer in big-endian network byte order.
 * @param buf Target buffer.
 * @param pos Current offset pointer (updated on write).
 * @param max_len Maximum buffer capacity.
 * @param v Value to write.
 * @return true on success, false on overflow.
 */
static bool tpm2_write_u16(uint8_t *buf, size_t *pos, size_t max_len, uint16_t v)
{
    if (*pos > max_len || (max_len - *pos) < 2U) {
        return false;
    }
    buf[*pos] = (uint8_t)((v >> 8U) & 0xFFU);
    buf[*pos + 1U] = (uint8_t)(v & 0xFFU);
    *pos += 2U;
    return true;
}

/**
 * @brief Write 32-bit unsigned integer in big-endian network byte order.
 * @param buf Target buffer.
 * @param pos Current offset pointer (updated on write).
 * @param max_len Maximum buffer capacity.
 * @param v Value to write.
 * @return true on success, false on overflow.
 */
static bool tpm2_write_u32(uint8_t *buf, size_t *pos, size_t max_len, uint32_t v)
{
    if (*pos > max_len || (max_len - *pos) < 4U) {
        return false;
    }
    buf[*pos] = (uint8_t)((v >> 24U) & 0xFFU);
    buf[*pos + 1U] = (uint8_t)((v >> 16U) & 0xFFU);
    buf[*pos + 2U] = (uint8_t)((v >> 8U) & 0xFFU);
    buf[*pos + 3U] = (uint8_t)(v & 0xFFU);
    *pos += 4U;
    return true;
}

/**
 * @brief Write raw byte array into buffer.
 * @param buf Target buffer.
 * @param pos Current offset pointer.
 * @param max_len Maximum buffer capacity.
 * @param data Source byte array.
 * @param len Number of bytes to copy.
 * @return true on success, false on overflow.
 */
static bool tpm2_write_bytes(uint8_t *buf, size_t *pos, size_t max_len, const uint8_t *data,
                             size_t len)
{
    if (*pos > max_len || len > (max_len - *pos)) {
        return false;
    }
    if (len > 0U && data != NULL) {
        (void)memcpy(&buf[*pos], data, len);
    }
    *pos += len;
    return true;
}

/**
 * @brief Read 16-bit unsigned integer from big-endian buffer.
 * @param buf Source buffer.
 * @param pos Current offset pointer.
 * @param max_len Total available bytes.
 * @param out_v Output pointer.
 * @return true on success, false on underflow.
 */
static bool tpm2_read_u16(const uint8_t *buf, size_t *pos, size_t max_len, uint16_t *out_v)
{
    if (*pos > max_len || (max_len - *pos) < 2U) {
        return false;
    }
    if (out_v != NULL) {
        *out_v = ((uint16_t)buf[*pos] << 8U) | (uint16_t)buf[*pos + 1U];
    }
    *pos += 2U;
    return true;
}

/**
 * @brief Read 32-bit unsigned integer from big-endian buffer.
 * @param buf Source buffer.
 * @param pos Current offset pointer.
 * @param max_len Total available bytes.
 * @param out_v Output pointer.
 * @return true on success, false on underflow.
 */
static bool tpm2_read_u32(const uint8_t *buf, size_t *pos, size_t max_len, uint32_t *out_v)
{
    if (*pos > max_len || (max_len - *pos) < 4U) {
        return false;
    }
    if (out_v != NULL) {
        *out_v = ((uint32_t)buf[*pos] << 24U) | ((uint32_t)buf[*pos + 1U] << 16U) |
                 ((uint32_t)buf[*pos + 2U] << 8U) | (uint32_t)buf[*pos + 3U];
    }
    *pos += 4U;
    return true;
}

/**
 * @brief Read raw byte array from buffer.
 * @param buf Source buffer.
 * @param pos Current offset pointer.
 * @param max_len Total available bytes.
 * @param out_data Output destination.
 * @param len Number of bytes to copy.
 * @return true on success, false on underflow.
 */
static bool tpm2_read_bytes(const uint8_t *buf, size_t *pos, size_t max_len, uint8_t *out_data,
                            size_t len)
{
    if (*pos > max_len || len > (max_len - *pos)) {
        return false;
    }
    if (len > 0U && out_data != NULL) {
        (void)memcpy(out_data, &buf[*pos], len);
    }
    *pos += len;
    return true;
}

/**
 * @brief Execute TPM 2.0 command over transport and parse standard response header.
 * @param ctx Context instance.
 * @param cmd_len Total command buffer length.
 * @param out_resp_len Output response length.
 * @return SYN_OK on TPM success, SYN_ERROR on transport or TPM RC error.
 */
static SYN_Status tpm2_execute_command(SYN_TPM2_Context *ctx, size_t cmd_len, size_t *out_resp_len)
{
    /* Patch paramSize in command header (bytes 2..5) */
    uint32_t ulen = (uint32_t)cmd_len;
    ctx->cfg.tx_buf[2] = (uint8_t)((ulen >> 24U) & 0xFFU);
    ctx->cfg.tx_buf[3] = (uint8_t)((ulen >> 16U) & 0xFFU);
    ctx->cfg.tx_buf[4] = (uint8_t)((ulen >> 8U) & 0xFFU);
    ctx->cfg.tx_buf[5] = (uint8_t)(ulen & 0xFFU);

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
        rc != SYN_TPM2_RC_SUCCESS || param_size < 10U || (size_t)param_size > rx_len) {
        return SYN_ERROR;
    }

    if (out_resp_len != NULL) {
        *out_resp_len = (size_t)param_size;
    }
    return SYN_OK;
}

/**
 * @brief Helper to write standard empty Password Authorization Session area.
 * @param tx Command buffer.
 * @param pos Offset pointer.
 * @param max_len Maximum length.
 * @return true on success.
 */
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
    if (*pos >= max_len) {
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
    if (pos >= max_len) {
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
    if (!tpm2_read_u16(ctx->cfg.rx_buf, &rpos, resp_len, &rand_len) || rand_len > num_bytes ||
        !tpm2_read_bytes(ctx->cfg.rx_buf, &rpos, resp_len, out_random, rand_len)) {
        return SYN_ERROR;
    }

    if (out_len != NULL) {
        *out_len = rand_len;
    }
    return SYN_OK;
}

/**
 * @brief Skip marshalled TPML_PCR_SELECTION entries in a TPM2_PCR_Read response.
 * @param rx       Response buffer.
 * @param rpos     Current read offset (updated in place).
 * @param resp_len Total response byte length.
 * @return true if all selection bitmaps fit within resp_len, false otherwise.
 */
static bool tpm2_skip_pcr_selections(const uint8_t *rx, size_t *rpos, size_t resp_len)
{
    uint32_t sel_count = 0U;
    if (!tpm2_read_u32(rx, rpos, resp_len, &sel_count)) {
        return false;
    }
    for (uint32_t i = 0U; i < sel_count; i++) {
        uint16_t alg = 0U;
        if (!tpm2_read_u16(rx, rpos, resp_len, &alg) || *rpos >= resp_len) {
            return false;
        }
        uint8_t sz = rx[(*rpos)++];
        if ((size_t)sz > (resp_len - *rpos)) {
            return false;
        }
        *rpos += sz;
    }
    return true;
}

/**
 * @brief Build a TPM2_PCR_Read request packet for a single PCR index.
 * @param tx        Command transmit buffer.
 * @param max_len   Transmit buffer capacity.
 * @param pcr_index Target PCR index (0..23).
 * @param hash_alg  PCR bank hash algorithm ID.
 * @param out_pos   Output marshalled command length.
 * @return true on success, false if tx buffer is too small.
 */
static bool tpm2_build_pcr_read_cmd(uint8_t *tx, size_t max_len, uint32_t pcr_index,
                                    uint16_t hash_alg, size_t *out_pos)
{
    size_t pos = 0U;
    (void)tpm2_write_u16(tx, &pos, max_len, SYN_TPM2_ST_NO_SESSIONS);
    (void)tpm2_write_u32(tx, &pos, max_len, 0U);
    (void)tpm2_write_u32(tx, &pos, max_len, SYN_TPM2_CC_PCR_READ);
    (void)tpm2_write_u32(tx, &pos, max_len, 1U);
    (void)tpm2_write_u16(tx, &pos, max_len, hash_alg);
    if ((max_len - pos) < 4U) {
        return false;
    }
    tx[pos++] = 3U;
    tx[pos++] = (uint8_t)((1U << (pcr_index & 7U)) * (pcr_index < 8U ? 1U : 0U));
    tx[pos++] =
        (uint8_t)((1U << (pcr_index & 7U)) * ((pcr_index >= 8U && pcr_index < 16U) ? 1U : 0U));
    tx[pos++] = (uint8_t)((1U << (pcr_index & 7U)) * (pcr_index >= 16U ? 1U : 0U));
    *out_pos = pos;
    return true;
}

/**
 * @brief Parse TPM2_PCR_Read response parameters and extract the first PCR digest.
 * @param rx             Response buffer.
 * @param resp_len       Total response byte length.
 * @param hash_alg       PCR bank hash algorithm ID.
 * @param out_digest     Caller digest buffer.
 * @param out_digest_len Optional output digest length pointer.
 * @return SYN_OK on valid digest extraction, SYN_ERROR otherwise.
 */
static SYN_Status tpm2_parse_pcr_read_resp(const uint8_t *rx, size_t resp_len, uint16_t hash_alg,
                                           uint8_t *out_digest, size_t *out_digest_len)
{
    size_t rpos = 10U;
    uint32_t pcr_update_counter = 0U;
    uint32_t digest_count = 0U;
    if (!tpm2_read_u32(rx, &rpos, resp_len, &pcr_update_counter) ||
        !tpm2_skip_pcr_selections(rx, &rpos, resp_len) ||
        !tpm2_read_u32(rx, &rpos, resp_len, &digest_count) || digest_count == 0U) {
        return SYN_ERROR;
    }

    size_t digest_cap = (hash_alg == SYN_TPM2_ALG_SHA256) ? 32U : SYN_TPM2_MAX_DIGEST_LEN;
    uint16_t dlen = 0U;
    if (!tpm2_read_u16(rx, &rpos, resp_len, &dlen) || dlen == 0U || (size_t)dlen > digest_cap ||
        !tpm2_read_bytes(rx, &rpos, resp_len, out_digest, dlen)) {
        return SYN_ERROR;
    }

    if (out_digest_len != NULL) {
        *out_digest_len = dlen;
    }
    return SYN_OK;
}

SYN_Status syn_tpm2_pcr_read(SYN_TPM2_Context *ctx, uint32_t pcr_index, uint16_t hash_alg,
                             uint8_t *out_digest, size_t *out_digest_len)
{
    if (ctx == NULL || !ctx->initialized || out_digest == NULL || pcr_index > 23U) {
        return SYN_INVALID_PARAM;
    }

    size_t pos = 0U;
    size_t resp_len = 0U;
    if (!tpm2_build_pcr_read_cmd(ctx->cfg.tx_buf, ctx->cfg.tx_buf_size, pcr_index, hash_alg,
                                 &pos) ||
        tpm2_execute_command(ctx, pos, &resp_len) != SYN_OK) {
        return SYN_ERROR;
    }

    return tpm2_parse_pcr_read_resp(ctx->cfg.rx_buf, resp_len, hash_alg, out_digest,
                                    out_digest_len);
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

/**
 * @brief Read a 16-bit length-prefixed TPM2B byte blob with caller capacity check.
 * @param rx       Response buffer.
 * @param rpos     Current read offset (updated in place).
 * @param resp_len Total response byte length.
 * @param dst      Destination buffer.
 * @param dst_cap  Maximum destination buffer capacity.
 * @param out_len  Output read byte count.
 * @return true if blob fits both response and destination bounds, false otherwise.
 */
static bool tpm2_read_sized_blob(const uint8_t *rx, size_t *rpos, size_t resp_len, uint8_t *dst,
                                 size_t dst_cap, uint16_t *out_len)
{
    uint16_t blen = 0U;
    if (!tpm2_read_u16(rx, rpos, resp_len, &blen) || (size_t)blen > dst_cap ||
        !tpm2_read_bytes(rx, rpos, resp_len, dst, blen)) {
        return false;
    }
    *out_len = blen;
    return true;
}

/**
 * @brief Read a TPM2B_ECC_PARAMETER coordinate (1..32 bytes) right-aligned into 32 bytes.
 * @param rx       Response buffer.
 * @param rpos     Current read offset (updated in place).
 * @param resp_len Total response byte length.
 * @param dst32    32-byte big-endian output slot (zero-padded on the left).
 * @return true if coordinate length is in [1, 32] and bytes are available, false otherwise.
 */
static bool tpm2_read_ecdsa_coord(const uint8_t *rx, size_t *rpos, size_t resp_len,
                                  uint8_t dst32[32])
{
    uint16_t clen = 0U;
    if (!tpm2_read_u16(rx, rpos, resp_len, &clen) || clen == 0U || clen > 32U) {
        return false;
    }
    (void)memset(dst32, 0, 32U);
    return tpm2_read_bytes(rx, rpos, resp_len, &dst32[32U - clen], clen);
}

/**
 * @brief Unmarshal a TCG TPM 2.0 TPMT_SIGNATURE structure (Part 2 Section 11.3.4).
 * @param rx        Response buffer.
 * @param rpos      Current read offset (updated in place).
 * @param resp_len  Total response byte length.
 * @param out_quote Quote output structure receiving signature bytes and length.
 * @return true on valid RSASSA/RSAPSS or ECDSA signature, false on TPM_ALG_NULL or error.
 */
static bool tpm2_unmarshal_signature(const uint8_t *rx, size_t *rpos, size_t resp_len,
                                     SYN_TPM2_QuoteResult *out_quote)
{
    uint16_t sig_alg = 0U;
    if (!tpm2_read_u16(rx, rpos, resp_len, &sig_alg) || sig_alg == SYN_TPM2_ALG_NULL) {
        return false;
    }

    uint16_t hash_alg = 0U;
    if (!tpm2_read_u16(rx, rpos, resp_len, &hash_alg)) {
        return false;
    }

    if (sig_alg == TPM2_ALG_RSASSA || sig_alg == TPM2_ALG_RSAPSS) {
        return tpm2_read_sized_blob(rx, rpos, resp_len, out_quote->signature,
                                    SYN_TPM2_MAX_QUOTE_LEN, &out_quote->signature_len);
    }

    if (sig_alg == TPM2_ALG_ECDSA) {
        if (!tpm2_read_ecdsa_coord(rx, rpos, resp_len, &out_quote->signature[0]) ||
            !tpm2_read_ecdsa_coord(rx, rpos, resp_len, &out_quote->signature[32])) {
            return false;
        }
        out_quote->signature_len = 64U;
        return true;
    }

    return false;
}

/**
 * @brief Marshal a TPM2_Quote command buffer.
 * @param tx              Command transmit buffer.
 * @param max_len         Transmit buffer capacity.
 * @param key_handle      Signing key handle.
 * @param qualifying_data Nonce / qualifying data bytes.
 * @param qual_len        Qualifying data byte length.
 * @param pcr_mask        24-bit PCR selection mask.
 * @param out_pos         Output marshalled command length.
 * @return true on success, false if transmit buffer overflows.
 */
static bool tpm2_build_quote_cmd(uint8_t *tx, size_t max_len, uint32_t key_handle,
                                 const uint8_t *qualifying_data, size_t qual_len, uint32_t pcr_mask,
                                 size_t *out_pos)
{
    size_t pos = 0U;
    (void)tpm2_write_u16(tx, &pos, max_len, SYN_TPM2_ST_SESSIONS);
    (void)tpm2_write_u32(tx, &pos, max_len, 0U);
    (void)tpm2_write_u32(tx, &pos, max_len, SYN_TPM2_CC_QUOTE);
    (void)tpm2_write_u32(tx, &pos, max_len, key_handle);

    if (!tpm2_write_pw_auth_session(tx, &pos, max_len) ||
        !tpm2_write_u16(tx, &pos, max_len, (uint16_t)qual_len) ||
        !tpm2_write_bytes(tx, &pos, max_len, qualifying_data, qual_len) ||
        !tpm2_write_u16(tx, &pos, max_len, SYN_TPM2_ALG_NULL) ||
        !tpm2_write_u32(tx, &pos, max_len, 1U) ||
        !tpm2_write_u16(tx, &pos, max_len, SYN_TPM2_ALG_SHA256) || ((max_len - pos) < 4U)) {
        return false;
    }

    tx[pos++] = 3U;
    tx[pos++] = (uint8_t)(pcr_mask & 0xFFU);
    tx[pos++] = (uint8_t)((pcr_mask >> 8U) & 0xFFU);
    tx[pos++] = (uint8_t)((pcr_mask >> 16U) & 0xFFU);
    *out_pos = pos;
    return true;
}

SYN_Status syn_tpm2_quote(SYN_TPM2_Context *ctx, uint32_t key_handle,
                          const uint8_t *qualifying_data, size_t qual_len, uint32_t pcr_mask,
                          SYN_TPM2_QuoteResult *out_quote)
{
    if (ctx == NULL || !ctx->initialized || out_quote == NULL || qual_len > 64U ||
        (qualifying_data == NULL && qual_len > 0U)) {
        return SYN_INVALID_PARAM;
    }

    size_t pos = 0U;
    size_t resp_len = 0U;
    if (!tpm2_build_quote_cmd(ctx->cfg.tx_buf, ctx->cfg.tx_buf_size, key_handle, qualifying_data,
                              qual_len, pcr_mask, &pos) ||
        tpm2_execute_command(ctx, pos, &resp_len) != SYN_OK) {
        return SYN_ERROR;
    }

    size_t rpos = 10U;
    uint32_t param_sz = 0U;
    if (!tpm2_read_u32(ctx->cfg.rx_buf, &rpos, resp_len, &param_sz) ||
        !tpm2_read_sized_blob(ctx->cfg.rx_buf, &rpos, resp_len, out_quote->attest_data,
                              SYN_TPM2_MAX_QUOTE_LEN, &out_quote->attest_len) ||
        !tpm2_unmarshal_signature(ctx->cfg.rx_buf, &rpos, resp_len, out_quote)) {
        return SYN_ERROR;
    }

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
    uint16_t read_len = 0U;
    if (!tpm2_read_u32(ctx->cfg.rx_buf, &rpos, resp_len, &param_sz) ||
        !tpm2_read_sized_blob(ctx->cfg.rx_buf, &rpos, resp_len, out_data, size, &read_len)) {
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
