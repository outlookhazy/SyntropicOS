/**
 * @file syn_ota.c
 * @brief Secure Streaming OTA Orchestrator implementation.
 */

#include "syn_ota.h"

#if SYN_OTA_ENABLED

#include "../proto/syn_lwm2m.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Helpers ─────────────────────────────────────────────────────────────── */

/**
 * @brief Check whether OTA configuration or session start is allowed in @p st.
 * @param st  Current OTA manager state.
 * @return true when state is IDLE, ERROR, or APPLIED.
 */
static bool ota_can_configure(SYN_OTA_State st)
{
    return (st == SYN_OTA_STATE_IDLE || st == SYN_OTA_STATE_ERROR || st == SYN_OTA_STATE_APPLIED);
}

/**
 * @brief Validate signature/tag presence and exact length for active crypto mode.
 * @param mgr      OTA manager instance.
 * @param sig      Expected signature, HMAC, or GCM tag buffer.
 * @param sig_len  Length of @p sig in bytes.
 * @return SYN_OK on valid parameters, SYN_INVALID_PARAM otherwise.
 */
static SYN_Status ota_validate_finish_sig(const SYN_OTA_Manager *mgr, const uint8_t *sig,
                                          size_t sig_len)
{
    switch (mgr->crypto_mode) {
    case SYN_OTA_CRYPTO_AES_GCM:
        return (sig != NULL && sig_len == 16U) ? SYN_OK : SYN_INVALID_PARAM;
    case SYN_OTA_CRYPTO_HMAC_SHA256:
        return (sig != NULL && sig_len == 32U) ? SYN_OK : SYN_INVALID_PARAM;
    case SYN_OTA_CRYPTO_ED25519:
        return (sig != NULL && sig_len == 64U) ? SYN_OK : SYN_INVALID_PARAM;
    default:
        (void)sig;
        (void)sig_len;
        return SYN_OK;
    }
}

#if SYN_LWM2M_ENABLED
/**
 * @brief Map SYN_OTA_State to OMA LwM2M Object 5 state.
 * @param state  OTA state.
 * @return Corresponding SYN_LwM2M_FirmwareState.
 */
static SYN_LwM2M_FirmwareState ota_map_lwm2m_state(SYN_OTA_State state)
{
    switch (state) {
    case SYN_OTA_STATE_IDLE:
        return SYN_LWM2M_FW_STATE_IDLE;
    case SYN_OTA_STATE_DOWNLOADING:
        return SYN_LWM2M_FW_STATE_DOWNLOADING;
    case SYN_OTA_STATE_DOWNLOADED:
    case SYN_OTA_STATE_VERIFYING:
    case SYN_OTA_STATE_READY_TO_APPLY:
        return SYN_LWM2M_FW_STATE_DOWNLOADED;
    case SYN_OTA_STATE_APPLIED:
        return SYN_LWM2M_FW_STATE_UPDATING;
    case SYN_OTA_STATE_ERROR:
    default:
        return SYN_LWM2M_FW_STATE_IDLE;
    }
}

/**
 * @brief Map SYN_OTA_ErrorCode and state to OMA LwM2M Object 5 update result.
 * @param state  OTA state.
 * @param err    Last OTA error code.
 * @return Corresponding SYN_LwM2M_FirmwareResult.
 */
static SYN_LwM2M_FirmwareResult ota_map_lwm2m_result(SYN_OTA_State state, SYN_OTA_ErrorCode err)
{
    switch (err) {
    case SYN_OTA_ERR_NONE:
        return (state == SYN_OTA_STATE_APPLIED) ? SYN_LWM2M_FW_RESULT_SUCCESS
                                                : SYN_LWM2M_FW_RESULT_DEFAULT;
    case SYN_OTA_ERR_OUT_OF_SPACE:
    case SYN_OTA_ERR_FLASH_ERASE:
    case SYN_OTA_ERR_FLASH_WRITE:
        return SYN_LWM2M_FW_RESULT_NO_FLASH;
    case SYN_OTA_ERR_INTEGRITY_CHECK:
        return SYN_LWM2M_FW_RESULT_INTEGRITY_FAIL;
    case SYN_OTA_ERR_UNSUPPORTED_CRYPTO:
    case SYN_OTA_ERR_INVALID_PARAM:
        return SYN_LWM2M_FW_RESULT_BAD_PKG_TYPE;
    default:
        return SYN_LWM2M_FW_RESULT_DEFAULT;
    }
}
#endif

/* ── API Implementation ─────────────────────────────────────────────────── */

SYN_Status syn_ota_init(SYN_OTA_Manager *mgr, SYN_FwBootManager *boot_mgr, uint32_t slot_size,
                        uint8_t *page_buf, size_t page_buf_sz)
{
    if (mgr == NULL || boot_mgr == NULL || page_buf == NULL || page_buf_sz < 16U ||
        page_buf_sz > (size_t)UINT16_MAX) {
        return SYN_INVALID_PARAM;
    }

    uint32_t hdr_off_a = syn_fwimage_data_offset(boot_mgr->slot_addr[0]);
    uint32_t hdr_off_b = syn_fwimage_data_offset(boot_mgr->slot_addr[1]);
    uint32_t hdr_off = (hdr_off_a >= hdr_off_b) ? hdr_off_a : hdr_off_b;
    if (slot_size <= hdr_off) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(mgr, 0, sizeof(*mgr));
    mgr->boot_mgr = boot_mgr;
    mgr->slot_size = slot_size;
    mgr->page_buf = page_buf;
    mgr->page_buf_sz = page_buf_sz;
    mgr->target_slot = SYN_OTA_SLOT_AUTO;
    mgr->state = SYN_OTA_STATE_IDLE;
    mgr->last_error = SYN_OTA_ERR_NONE;
    mgr->crypto_mode = SYN_OTA_CRYPTO_NONE;

    return SYN_OK;
}

SYN_Status syn_ota_set_target_slot(SYN_OTA_Manager *mgr, uint8_t slot_idx)
{
    if (mgr == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (!ota_can_configure(mgr->state)) {
        return SYN_ERROR;
    }
    if (slot_idx != SYN_FW_SLOT_A && slot_idx != SYN_FW_SLOT_B && slot_idx != SYN_OTA_SLOT_AUTO) {
        return SYN_INVALID_PARAM;
    }

    mgr->target_slot = slot_idx;
    return SYN_OK;
}

SYN_Status syn_ota_set_verification_key(SYN_OTA_Manager *mgr, SYN_OTA_CryptoMode mode,
                                        const uint8_t *key, size_t key_len)
{
    if (mgr == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (!ota_can_configure(mgr->state)) {
        return SYN_ERROR;
    }

    switch (mode) {
    case SYN_OTA_CRYPTO_NONE:
        mgr->crypto_mode = SYN_OTA_CRYPTO_NONE;
        return SYN_OK;

    case SYN_OTA_CRYPTO_HMAC_SHA256:
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
        if (key == NULL || key_len == 0U || key_len > sizeof(mgr->hmac_key)) {
            return SYN_INVALID_PARAM;
        }
        (void)memcpy(mgr->hmac_key, key, key_len);
        mgr->hmac_key_len = key_len;
        mgr->hmac_key_set = true;
        mgr->crypto_mode = SYN_OTA_CRYPTO_HMAC_SHA256;
        return SYN_OK;
#else
        mgr->last_error = SYN_OTA_ERR_UNSUPPORTED_CRYPTO;
        return SYN_ERROR;
#endif

    case SYN_OTA_CRYPTO_ED25519:
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
        if (key == NULL || key_len != 32U) {
            return SYN_INVALID_PARAM;
        }
        (void)memcpy(mgr->ed25519_pubkey, key, 32U);
        mgr->ed25519_key_set = true;
        mgr->crypto_mode = SYN_OTA_CRYPTO_ED25519;
        return SYN_OK;
#else
        mgr->last_error = SYN_OTA_ERR_UNSUPPORTED_CRYPTO;
        return SYN_ERROR;
#endif

    default:
        return SYN_INVALID_PARAM;
    }
}

SYN_Status syn_ota_set_aes_gcm_params(SYN_OTA_Manager *mgr, const uint8_t *key, size_t key_len,
                                      const uint8_t *iv, size_t iv_len)
{
    if (mgr == NULL || key == NULL || iv == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (!ota_can_configure(mgr->state)) {
        return SYN_ERROR;
    }

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
    if ((key_len != 16U && key_len != 24U && key_len != 32U) || iv_len != 12U) {
        return SYN_INVALID_PARAM;
    }

    (void)memcpy(mgr->gcm_key, key, key_len);
    mgr->gcm_key_len = key_len;
    (void)memcpy(mgr->gcm_iv, iv, 12U);
    mgr->gcm_key_set = true;
    mgr->crypto_mode = SYN_OTA_CRYPTO_AES_GCM;
    return SYN_OK;
#else
    mgr->last_error = SYN_OTA_ERR_UNSUPPORTED_CRYPTO;
    return SYN_ERROR;
#endif
}

SYN_Status syn_ota_begin(SYN_OTA_Manager *mgr, uint32_t expected_total_sz, uint32_t target_version,
                         uint32_t expected_crc)
{
    if (mgr == NULL || mgr->boot_mgr == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (!ota_can_configure(mgr->state)) {
        return SYN_ERROR;
    }
    if (expected_total_sz == 0U) {
        mgr->last_error = SYN_OTA_ERR_OUT_OF_SPACE;
        return SYN_INVALID_PARAM;
    }

    /* Resolve destination flash slot */
    uint8_t slot = mgr->target_slot;
    if (slot == SYN_OTA_SLOT_AUTO) {
        uint8_t active = mgr->boot_mgr->active_slot;
        slot = (active == SYN_FW_SLOT_A) ? (uint8_t)SYN_FW_SLOT_B : (uint8_t)SYN_FW_SLOT_A;
    }

    if (slot > 1U) {
        mgr->last_error = SYN_OTA_ERR_NO_FLASH_SLOT;
        mgr->state = SYN_OTA_STATE_ERROR;
        syn_ota_sync_lwm2m(mgr);
        return SYN_ERROR;
    }

    uint32_t target_addr = mgr->boot_mgr->slot_addr[slot];
    uint32_t hdr_offset = syn_fwimage_data_offset(target_addr);
    uint32_t max_img_sz = (mgr->slot_size > hdr_offset) ? (mgr->slot_size - hdr_offset) : 0U;
    if (expected_total_sz > max_img_sz) {
        mgr->last_error = SYN_OTA_ERR_OUT_OF_SPACE;
        return SYN_INVALID_PARAM;
    }

    mgr->target_slot_addr = target_addr;
    mgr->expected_total_sz = expected_total_sz;
    mgr->target_version = target_version;
    mgr->expected_crc = expected_crc;

    SYN_Status st = syn_fwupdate_begin(&mgr->fw_upd, mgr->target_slot_addr, max_img_sz,
                                       mgr->page_buf, (uint16_t)mgr->page_buf_sz);
    if (st != SYN_OK) {
        mgr->last_error = SYN_OTA_ERR_FLASH_ERASE;
        mgr->state = SYN_OTA_STATE_ERROR;
        syn_ota_sync_lwm2m(mgr);
        return st;
    }

    /* Configure cryptographic verification hooks */
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    if (mgr->crypto_mode == SYN_OTA_CRYPTO_HMAC_SHA256) {
        syn_fwupdate_set_key(&mgr->fw_upd, mgr->hmac_key, mgr->hmac_key_len);
    }
#endif

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    if (mgr->crypto_mode == SYN_OTA_CRYPTO_ED25519) {
        syn_fwupdate_set_public_key(&mgr->fw_upd, mgr->ed25519_pubkey);
    }
#endif

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
    if (mgr->crypto_mode == SYN_OTA_CRYPTO_AES_GCM) {
        (void)syn_fwupdate_set_aes_gcm_key(&mgr->fw_upd, mgr->gcm_key, mgr->gcm_key_len,
                                           mgr->gcm_iv, sizeof(mgr->gcm_iv));
    }
#endif

    mgr->state = SYN_OTA_STATE_DOWNLOADING;
    mgr->last_error = SYN_OTA_ERR_NONE;
    syn_ota_sync_lwm2m(mgr);
    return SYN_OK;
}

/**
 * @brief Calculate total bytes ingested across flash and in-memory page buffer.
 * @param mgr  OTA manager instance.
 * @return Total ingested bytes.
 */
static inline uint32_t syn_ota_bytes_ingested(const SYN_OTA_Manager *mgr)
{
    return mgr->fw_upd.bytes_written + (uint32_t)mgr->fw_upd.page_buf_used;
}

SYN_Status syn_ota_write_chunk(SYN_OTA_Manager *mgr, const uint8_t *chunk, size_t chunk_sz)
{
    if (mgr == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (chunk_sz == 0U) {
        return SYN_OK;
    }
    if (mgr->state != SYN_OTA_STATE_DOWNLOADING) {
        return SYN_ERROR;
    }
    if (chunk == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint32_t ingested = syn_ota_bytes_ingested(mgr);
    if (ingested > mgr->expected_total_sz ||
        chunk_sz > (size_t)(mgr->expected_total_sz - ingested)) {
        syn_ota_abort(mgr, SYN_OTA_ERR_OUT_OF_SPACE);
        return SYN_ERROR;
    }

    SYN_Status st = syn_fwupdate_write(&mgr->fw_upd, chunk, chunk_sz);
    if (st != SYN_OK) {
        syn_ota_abort(mgr, SYN_OTA_ERR_FLASH_WRITE);
        return st;
    }

    if (syn_ota_bytes_ingested(mgr) == mgr->expected_total_sz) {
        mgr->state = SYN_OTA_STATE_DOWNLOADED;
    }

    syn_ota_sync_lwm2m(mgr);
    return SYN_OK;
}

SYN_Status syn_ota_finish(SYN_OTA_Manager *mgr, const uint8_t *expected_sig_or_tag, size_t sig_len)
{
    if (mgr == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (mgr->state != SYN_OTA_STATE_DOWNLOADING && mgr->state != SYN_OTA_STATE_DOWNLOADED) {
        return SYN_ERROR;
    }
    if (ota_validate_finish_sig(mgr, expected_sig_or_tag, sig_len) != SYN_OK) {
        syn_ota_abort(mgr, SYN_OTA_ERR_INVALID_PARAM);
        return SYN_INVALID_PARAM;
    }
    if (syn_ota_bytes_ingested(mgr) != mgr->expected_total_sz) {
        syn_ota_abort(mgr, SYN_OTA_ERR_INTEGRITY_CHECK);
        return SYN_ERROR;
    }

    mgr->state = SYN_OTA_STATE_VERIFYING;

    SYN_Status st = SYN_ERROR;

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
    if (mgr->crypto_mode == SYN_OTA_CRYPTO_AES_GCM) {
        st = syn_fwupdate_finish_gcm(&mgr->fw_upd, expected_sig_or_tag, mgr->target_version);
    } else
#endif
    {
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
        const uint8_t *hmac_ptr =
            (mgr->crypto_mode == SYN_OTA_CRYPTO_HMAC_SHA256) ? expected_sig_or_tag : NULL;
#endif
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
        const uint8_t *sig_ptr =
            (mgr->crypto_mode == SYN_OTA_CRYPTO_ED25519) ? expected_sig_or_tag : NULL;
#endif

        st = syn_fwupdate_finish(&mgr->fw_upd, mgr->expected_crc,
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
                                 hmac_ptr,
#endif
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
                                 sig_ptr,
#endif
                                 mgr->target_version);
    }

    if (st != SYN_OK) {
        syn_ota_abort(mgr, SYN_OTA_ERR_INTEGRITY_CHECK);
        return st;
    }

    (void)syn_fwboot_refresh(mgr->boot_mgr);
    mgr->state = SYN_OTA_STATE_READY_TO_APPLY;
    mgr->last_error = SYN_OTA_ERR_NONE;
    syn_ota_sync_lwm2m(mgr);
    return SYN_OK;
}

SYN_Status syn_ota_apply(SYN_OTA_Manager *mgr)
{
    if (mgr == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (mgr->state != SYN_OTA_STATE_READY_TO_APPLY) {
        return SYN_ERROR;
    }

    mgr->state = SYN_OTA_STATE_APPLIED;
    syn_ota_sync_lwm2m(mgr);
    return SYN_OK;
}

void syn_ota_abort(SYN_OTA_Manager *mgr, SYN_OTA_ErrorCode err)
{
    if (mgr == NULL) {
        return;
    }

    if (mgr->fw_upd.active) {
        syn_fwupdate_abort(&mgr->fw_upd);
    }

    mgr->state = SYN_OTA_STATE_ERROR;
    mgr->last_error = err;
    syn_ota_sync_lwm2m(mgr);
}

SYN_Status syn_ota_get_progress(const SYN_OTA_Manager *mgr, uint32_t *out_written,
                                uint32_t *out_total, uint8_t *out_percent)
{
    if (mgr == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint32_t written = syn_ota_bytes_ingested(mgr);
    uint32_t total = mgr->expected_total_sz;

    if (out_written != NULL) {
        *out_written = written;
    }
    if (out_total != NULL) {
        *out_total = total;
    }
    if (out_percent != NULL) {
        if (total == 0U) {
            *out_percent = 0U;
        } else {
            uint64_t pct = ((uint64_t)written * 100ULL) / (uint64_t)total;
            if (pct > 100ULL) {
                pct = 100ULL;
            }
            *out_percent = (uint8_t)pct;
        }
    }

    return SYN_OK;
}

SYN_OTA_State syn_ota_get_state(const SYN_OTA_Manager *mgr)
{
    if (mgr == NULL) {
        return SYN_OTA_STATE_ERROR;
    }
    return mgr->state;
}

SYN_OTA_ErrorCode syn_ota_get_last_error(const SYN_OTA_Manager *mgr)
{
    if (mgr == NULL) {
        return SYN_OTA_ERR_INVALID_PARAM;
    }
    return mgr->last_error;
}

SYN_Status syn_ota_bind_lwm2m(SYN_OTA_Manager *mgr, void *lwm2m_fw_ctx)
{
    if (mgr == NULL) {
        return SYN_INVALID_PARAM;
    }
    mgr->lwm2m_fw_ctx = lwm2m_fw_ctx;
    syn_ota_sync_lwm2m(mgr);
    return SYN_OK;
}

void syn_ota_sync_lwm2m(SYN_OTA_Manager *mgr)
{
#if SYN_LWM2M_ENABLED
    if (mgr == NULL || mgr->lwm2m_fw_ctx == NULL) {
        return;
    }

    SYN_LwM2M_FirmwareContext *ctx = (SYN_LwM2M_FirmwareContext *)mgr->lwm2m_fw_ctx;
    ctx->state = ota_map_lwm2m_state(mgr->state);
    ctx->result = ota_map_lwm2m_result(mgr->state, mgr->last_error);
#else
    (void)mgr;
#endif
}

#endif /* SYN_OTA_ENABLED */
