

# File syn\_ota.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_ota.c**](syn__ota_8c.md)

[Go to the documentation of this file](syn__ota_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_OTA) || SYN_USE_OTA

#include "../util/syn_assert.h"
#include "syn_ota.h"

#include <string.h>

/* ── LwM2M Object 5 Forward Types (if bound) ─────────────────────────────── */

typedef enum {
    LWM2M_FW_STATE_IDLE = 0,        
    LWM2M_FW_STATE_DOWNLOADING = 1, 
    LWM2M_FW_STATE_DOWNLOADED = 2,  
    LWM2M_FW_STATE_UPDATING = 3,    
} LwM2M_FwState;

typedef enum {
    LWM2M_FW_RES_DEFAULT = 0,        
    LWM2M_FW_RES_SUCCESS = 1,        
    LWM2M_FW_RES_NO_FLASH = 2,       
    LWM2M_FW_RES_OUT_OF_RAM = 3,     
    LWM2M_FW_RES_CONN_LOST = 4,      
    LWM2M_FW_RES_INTEGRITY_FAIL = 5, 
    LWM2M_FW_RES_BAD_PKG_TYPE = 6,   
    LWM2M_FW_RES_INVALID_URI = 7,    
} LwM2M_FwResult;

typedef struct {
    LwM2M_FwState state;   
    LwM2M_FwResult result; 
    char package_uri[128]; 
    char pkg_name[32];     
    char pkg_version[32];  
    bool update_requested; 
} LwM2M_FwCtx;

/* ── API Implementation ─────────────────────────────────────────────────── */

SYN_Status syn_ota_init(SYN_OTA_Manager *mgr, SYN_FwBootManager *boot_mgr, uint32_t slot_size,
                        uint8_t *page_buf, size_t page_buf_sz)
{
    if (mgr == NULL || boot_mgr == NULL || page_buf == NULL || page_buf_sz < 16U ||
        slot_size <= sizeof(SYN_FwImageHeader)) {
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
    if (mgr->state != SYN_OTA_STATE_IDLE && mgr->state != SYN_OTA_STATE_ERROR) {
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
    if (mgr->state != SYN_OTA_STATE_IDLE && mgr->state != SYN_OTA_STATE_ERROR) {
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
    if (mgr->state != SYN_OTA_STATE_IDLE && mgr->state != SYN_OTA_STATE_ERROR) {
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
    if (mgr->state != SYN_OTA_STATE_IDLE && mgr->state != SYN_OTA_STATE_ERROR) {
        return SYN_ERROR;
    }
    if (expected_total_sz == 0U ||
        (expected_total_sz + sizeof(SYN_FwImageHeader)) > mgr->slot_size) {
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

    mgr->target_slot_addr = mgr->boot_mgr->slot_addr[slot];
    mgr->expected_total_sz = expected_total_sz;
    mgr->target_version = target_version;
    mgr->expected_crc = expected_crc;

    uint32_t max_img_sz = mgr->slot_size - (uint32_t)sizeof(SYN_FwImageHeader);
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
    if (mgr->crypto_mode == SYN_OTA_CRYPTO_HMAC_SHA256 && mgr->hmac_key_set) {
        syn_fwupdate_set_key(&mgr->fw_upd, mgr->hmac_key, mgr->hmac_key_len);
    }
#endif

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    if (mgr->crypto_mode == SYN_OTA_CRYPTO_ED25519 && mgr->ed25519_key_set) {
        syn_fwupdate_set_public_key(&mgr->fw_upd, mgr->ed25519_pubkey);
    }
#endif

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
    if (mgr->crypto_mode == SYN_OTA_CRYPTO_AES_GCM && mgr->gcm_key_set) {
        (void)syn_fwupdate_set_aes_gcm_key(&mgr->fw_upd, mgr->gcm_key, mgr->gcm_key_len,
                                           mgr->gcm_iv, sizeof(mgr->gcm_iv));
    }
#endif

    mgr->state = SYN_OTA_STATE_DOWNLOADING;
    mgr->last_error = SYN_OTA_ERR_NONE;
    syn_ota_sync_lwm2m(mgr);
    return SYN_OK;
}

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

    if ((syn_ota_bytes_ingested(mgr) + chunk_sz) > mgr->expected_total_sz) {
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
#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
    if (mgr->crypto_mode == SYN_OTA_CRYPTO_AES_GCM) {
        if (expected_sig_or_tag == NULL || sig_len != 16U) {
            syn_ota_abort(mgr, SYN_OTA_ERR_INVALID_PARAM);
            return SYN_INVALID_PARAM;
        }
    }
#endif
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
        (void)sig_len;
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
    if (mgr == NULL || mgr->lwm2m_fw_ctx == NULL) {
        return;
    }

    LwM2M_FwCtx *ctx = (LwM2M_FwCtx *)mgr->lwm2m_fw_ctx;

    switch (mgr->state) {
    case SYN_OTA_STATE_IDLE:
        ctx->state = LWM2M_FW_STATE_IDLE;
        break;
    case SYN_OTA_STATE_DOWNLOADING:
        ctx->state = LWM2M_FW_STATE_DOWNLOADING;
        break;
    case SYN_OTA_STATE_DOWNLOADED:
    case SYN_OTA_STATE_VERIFYING:
    case SYN_OTA_STATE_READY_TO_APPLY:
        ctx->state = LWM2M_FW_STATE_DOWNLOADED;
        break;
    case SYN_OTA_STATE_APPLIED:
        ctx->state = LWM2M_FW_STATE_UPDATING;
        ctx->result = LWM2M_FW_RES_SUCCESS;
        break;
    case SYN_OTA_STATE_ERROR:
    default:
        ctx->state = LWM2M_FW_STATE_IDLE;
        break;
    }

    switch (mgr->last_error) {
    case SYN_OTA_ERR_NONE:
        if (mgr->state == SYN_OTA_STATE_APPLIED) {
            ctx->result = LWM2M_FW_RES_SUCCESS;
        } else {
            ctx->result = LWM2M_FW_RES_DEFAULT;
        }
        break;
    case SYN_OTA_ERR_OUT_OF_SPACE:
    case SYN_OTA_ERR_FLASH_ERASE:
    case SYN_OTA_ERR_FLASH_WRITE:
        ctx->result = LWM2M_FW_RES_NO_FLASH;
        break;
    case SYN_OTA_ERR_INTEGRITY_CHECK:
        ctx->result = LWM2M_FW_RES_INTEGRITY_FAIL;
        break;
    case SYN_OTA_ERR_UNSUPPORTED_CRYPTO:
    case SYN_OTA_ERR_INVALID_PARAM:
        ctx->result = LWM2M_FW_RES_BAD_PKG_TYPE;
        break;
    default:
        ctx->result = LWM2M_FW_RES_DEFAULT;
        break;
    }
}

#else
typedef int syn_ota_dummy_t;
#endif /* SYN_USE_OTA */
```


