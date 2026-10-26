

# File syn\_tpm2.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_tpm2.h**](syn__tpm2_8h.md)

[Go to the documentation of this file](syn__tpm2_8h.md)


```C++

#ifndef SYN_TPM2_H
#define SYN_TPM2_H

#include "../common/syn_defs.h"
#include "../net/syn_transport.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(SYN_USE_TPM2) || SYN_USE_TPM2

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants & Structures ─────────────────────────────────────────────── */

#define SYN_TPM2_ST_NO_SESSIONS 0x8001U 
#define SYN_TPM2_ST_SESSIONS 0x8002U    
#define SYN_TPM2_RC_SUCCESS 0x00000000U 
/* TPM 2.0 Command Codes (TPM_CC_*) */
#define SYN_TPM2_CC_STARTUP 0x00000144U    
#define SYN_TPM2_CC_SELFTEST 0x00000143U   
#define SYN_TPM2_CC_GETRANDOM 0x0000017BU  
#define SYN_TPM2_CC_PCR_READ 0x0000017EU   
#define SYN_TPM2_CC_PCR_EXTEND 0x00000182U 
#define SYN_TPM2_CC_QUOTE 0x00000158U      
#define SYN_TPM2_CC_NV_READ 0x0000014EU    
#define SYN_TPM2_CC_NV_WRITE 0x00000137U   
/* TPM 2.0 Algorithm Identifiers (TPM_ALG_*) */
#define SYN_TPM2_ALG_SHA256 0x000BU 
#define SYN_TPM2_ALG_SHA384 0x000CU 
#define SYN_TPM2_ALG_NULL 0x0010U   
/* TPM 2.0 Standard Handles & Constants */
#define SYN_TPM2_RH_OWNER 0x40000001U    
#define SYN_TPM2_RH_PLATFORM 0x4000000CU 
#define SYN_TPM2_RS_PW 0x40000009U       
#define SYN_TPM2_SU_CLEAR 0x0000U 
#define SYN_TPM2_SU_STATE 0x0001U 
#define SYN_TPM2_MAX_DIGEST_LEN 48U 
#define SYN_TPM2_MAX_QUOTE_LEN 256U 
/* ── Result Types ───────────────────────────────────────────────────────── */

typedef struct {
    uint8_t attest_data[SYN_TPM2_MAX_QUOTE_LEN]; 
    uint16_t attest_len;                         
    uint8_t signature[SYN_TPM2_MAX_QUOTE_LEN];   
    uint16_t signature_len;                      
} SYN_TPM2_QuoteResult;

typedef struct {
    SYN_Transport *transport; 
    uint8_t *rx_buf;          
    size_t rx_buf_size;       
    uint8_t *tx_buf;          
    size_t tx_buf_size;       
} SYN_TPM2_Config;

typedef struct {
    SYN_TPM2_Config cfg; 
    uint32_t last_rc;    
    bool initialized;    
} SYN_TPM2_Context;

/* ── Public API ─────────────────────────────────────────────────────────── */

SYN_Status syn_tpm2_init(SYN_TPM2_Context *ctx, const SYN_TPM2_Config *cfg);

SYN_Status syn_tpm2_startup(SYN_TPM2_Context *ctx, uint16_t startup_type);

SYN_Status syn_tpm2_self_test(SYN_TPM2_Context *ctx, bool full_test);

SYN_Status syn_tpm2_get_random(SYN_TPM2_Context *ctx, uint16_t num_bytes, uint8_t *out_random,
                               uint16_t *out_len);

SYN_Status syn_tpm2_pcr_read(SYN_TPM2_Context *ctx, uint32_t pcr_index, uint16_t hash_alg,
                             uint8_t *out_digest, size_t *out_digest_len);

SYN_Status syn_tpm2_pcr_extend(SYN_TPM2_Context *ctx, uint32_t pcr_index, uint16_t hash_alg,
                               const uint8_t *in_digest, size_t digest_len);

SYN_Status syn_tpm2_quote(SYN_TPM2_Context *ctx, uint32_t key_handle,
                          const uint8_t *qualifying_data, size_t qual_len, uint32_t pcr_mask,
                          SYN_TPM2_QuoteResult *out_quote);

SYN_Status syn_tpm2_nv_read(SYN_TPM2_Context *ctx, uint32_t auth_handle, uint32_t nv_index,
                            uint16_t offset, uint16_t size, uint8_t *out_data, uint16_t *out_len);

SYN_Status syn_tpm2_nv_write(SYN_TPM2_Context *ctx, uint32_t auth_handle, uint32_t nv_index,
                             uint16_t offset, const uint8_t *in_data, uint16_t size);

uint32_t syn_tpm2_get_last_rc(const SYN_TPM2_Context *ctx);

#ifdef __cplusplus
}
#endif

#endif /* !defined(SYN_USE_TPM2) || SYN_USE_TPM2 */

#endif /* SYN_TPM2_H */
```


