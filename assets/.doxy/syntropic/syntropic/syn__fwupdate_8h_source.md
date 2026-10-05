

# File syn\_fwupdate.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_fwupdate.h**](syn__fwupdate_8h.md)

[Go to the documentation of this file](syn__fwupdate_8h.md)


```C++

#ifndef SYN_FWUPDATE_H
#define SYN_FWUPDATE_H

#include "../common/syn_defs.h"
#include "syn_fwimage.h"

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
#include "../crypto/syn_hmac.h"
#endif

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
#include "../crypto/syn_ed25519.h"
#endif

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
#include "../crypto/syn_aes.h"
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Updater state ──────────────────────────────────────────────────────── */

typedef struct {
    uint32_t slot_addr;     
    uint32_t data_addr;     
    uint32_t max_size;      
    uint32_t bytes_written; 
    uint32_t crc_state;     
    uint8_t *page_buf;      
    uint16_t page_buf_size; 
    uint16_t page_buf_used; 
    bool active; 
    bool error;  
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    SYN_HMAC_SHA256 hmac_ctx; 
    bool key_set;             
#endif

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    SYN_SHA512_Ctx sha512_ctx; 
    uint8_t public_key[32];    
    bool pubkey_set;           
#endif

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
    SYN_AES_Context gcm_aes;    
    uint8_t gcm_h[16];          
    uint8_t gcm_j0[16];         
    uint8_t gcm_ctr[16];        
    uint8_t gcm_s[16];          
    uint8_t gcm_partial_ct[16]; 
    uint8_t gcm_partial_used;   
    uint8_t gcm_stream_buf[16]; 
    uint8_t gcm_stream_used;    
    uint64_t gcm_total_bytes;   
    bool gcm_key_set;           
#endif
} SYN_FwUpdate;

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_fwupdate_begin(SYN_FwUpdate *upd, uint32_t slot_addr, uint32_t max_size,
                              uint8_t *page_buf, uint16_t page_buf_size);

SYN_Status syn_fwupdate_write(SYN_FwUpdate *upd, const uint8_t *data, size_t len);

SYN_Status syn_fwupdate_finish(SYN_FwUpdate *upd, uint32_t expected_crc,
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
                               const uint8_t *expected_hmac,
#endif
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
                               const uint8_t *expected_sig,
#endif
                               uint32_t version_code);

void syn_fwupdate_abort(SYN_FwUpdate *upd);

static inline uint32_t syn_fwupdate_progress(const SYN_FwUpdate *upd)
{
    return upd->bytes_written;
}

static inline bool syn_fwupdate_active(const SYN_FwUpdate *upd)
{
    return upd->active;
}

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
void syn_fwupdate_set_key(SYN_FwUpdate *upd, const void *key, size_t key_len);
#endif

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
void syn_fwupdate_set_public_key(SYN_FwUpdate *upd, const uint8_t *public_key);
#endif

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
SYN_Status syn_fwupdate_set_aes_gcm_key(SYN_FwUpdate *upd, const uint8_t *key, size_t key_len,
                                        const uint8_t *iv, size_t iv_len);

SYN_Status syn_fwupdate_finish_gcm(SYN_FwUpdate *upd, const uint8_t expected_tag[16],
                                   uint32_t version_code);
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYN_FWUPDATE_H */
```


