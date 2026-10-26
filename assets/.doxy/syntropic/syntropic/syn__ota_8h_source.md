

# File syn\_ota.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_ota.h**](syn__ota_8h.md)

[Go to the documentation of this file](syn__ota_8h.md)


```C++

#ifndef SYN_OTA_H
#define SYN_OTA_H

#include "../common/syn_defs.h"
#include "syn_fwboot.h"
#include "syn_fwimage.h"
#include "syn_fwupdate.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(SYN_USE_OTA) || SYN_USE_OTA

/* ── Enums & Types ───────────────────────────────────────────────────────── */

typedef enum {
    SYN_OTA_STATE_IDLE = 0,       
    SYN_OTA_STATE_DOWNLOADING,    
    SYN_OTA_STATE_DOWNLOADED,     
    SYN_OTA_STATE_VERIFYING,      
    SYN_OTA_STATE_READY_TO_APPLY, 
    SYN_OTA_STATE_APPLIED,        
    SYN_OTA_STATE_ERROR,          
} SYN_OTA_State;

typedef enum {
    SYN_OTA_CRYPTO_NONE = 0,    
    SYN_OTA_CRYPTO_HMAC_SHA256, 
    SYN_OTA_CRYPTO_ED25519,     
    SYN_OTA_CRYPTO_AES_GCM,     
} SYN_OTA_CryptoMode;

typedef enum {
    SYN_OTA_ERR_NONE = 0,           
    SYN_OTA_ERR_INVALID_PARAM,      
    SYN_OTA_ERR_NO_FLASH_SLOT,      
    SYN_OTA_ERR_FLASH_ERASE,        
    SYN_OTA_ERR_FLASH_WRITE,        
    SYN_OTA_ERR_OUT_OF_SPACE,       
    SYN_OTA_ERR_INTEGRITY_CHECK,    
    SYN_OTA_ERR_UNSUPPORTED_CRYPTO, 
    SYN_OTA_ERR_INVALID_STATE,      
} SYN_OTA_ErrorCode;

#define SYN_OTA_SLOT_AUTO 0xFFU

/* ── Context Structure ───────────────────────────────────────────────────── */

typedef struct {
    SYN_FwBootManager *boot_mgr; 
    SYN_FwUpdate fw_upd;         
    uint8_t *page_buf;           
    size_t page_buf_sz;          
    uint32_t slot_size;          
    uint8_t target_slot;       
    uint32_t target_slot_addr; 
    SYN_OTA_State state;            
    SYN_OTA_ErrorCode last_error;   
    SYN_OTA_CryptoMode crypto_mode; 
    uint32_t expected_total_sz; 
    uint32_t target_version;    
    uint32_t expected_crc;      
    void *lwm2m_fw_ctx; 
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    uint8_t hmac_key[32]; 
    size_t hmac_key_len;  
    bool hmac_key_set;    
#endif

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    uint8_t ed25519_pubkey[32]; 
    bool ed25519_key_set;       
#endif

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
    uint8_t gcm_key[32]; 
    size_t gcm_key_len;  
    uint8_t gcm_iv[12];  
    bool gcm_key_set;    
#endif
} SYN_OTA_Manager;

/* ── API ─────────────────────────────────────────────────────────────────── */

SYN_Status syn_ota_init(SYN_OTA_Manager *mgr, SYN_FwBootManager *boot_mgr, uint32_t slot_size,
                        uint8_t *page_buf, size_t page_buf_sz);

SYN_Status syn_ota_set_target_slot(SYN_OTA_Manager *mgr, uint8_t slot_idx);

SYN_Status syn_ota_set_verification_key(SYN_OTA_Manager *mgr, SYN_OTA_CryptoMode mode,
                                        const uint8_t *key, size_t key_len);

SYN_Status syn_ota_set_aes_gcm_params(SYN_OTA_Manager *mgr, const uint8_t *key, size_t key_len,
                                      const uint8_t *iv, size_t iv_len);

SYN_Status syn_ota_begin(SYN_OTA_Manager *mgr, uint32_t expected_total_sz, uint32_t target_version,
                         uint32_t expected_crc);

SYN_Status syn_ota_write_chunk(SYN_OTA_Manager *mgr, const uint8_t *chunk, size_t chunk_sz);

SYN_Status syn_ota_finish(SYN_OTA_Manager *mgr, const uint8_t *expected_sig_or_tag, size_t sig_len);

SYN_Status syn_ota_apply(SYN_OTA_Manager *mgr);

void syn_ota_abort(SYN_OTA_Manager *mgr, SYN_OTA_ErrorCode err);

SYN_Status syn_ota_get_progress(const SYN_OTA_Manager *mgr, uint32_t *out_written,
                                uint32_t *out_total, uint8_t *out_percent);

SYN_OTA_State syn_ota_get_state(const SYN_OTA_Manager *mgr);

SYN_OTA_ErrorCode syn_ota_get_last_error(const SYN_OTA_Manager *mgr);

SYN_Status syn_ota_bind_lwm2m(SYN_OTA_Manager *mgr, void *lwm2m_fw_ctx);

void syn_ota_sync_lwm2m(SYN_OTA_Manager *mgr);

#endif /* SYN_USE_OTA */

#ifdef __cplusplus
}
#endif

#endif /* SYN_OTA_H */
```


