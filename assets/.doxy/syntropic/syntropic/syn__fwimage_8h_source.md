

# File syn\_fwimage.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_fwimage.h**](syn__fwimage_8h.md)

[Go to the documentation of this file](syn__fwimage_8h.md)


```C++


#ifndef SYN_FWIMAGE_H
#define SYN_FWIMAGE_H

#include "../common/syn_defs.h"
#include "../port/syn_port_flash.h"
#include "../util/syn_crc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Magic number ───────────────────────────────────────────────────────── */

#define SYN_FW_MAGIC 0x53594E46u /* "SYNF" in little-endian */

#define SYN_FWIMAGE_AUTH_META_SIZE 12U

/* ── Image state ────────────────────────────────────────────────────────── */

typedef enum {
    SYN_FW_STATE_EMPTY = 0xFF,     
    SYN_FW_STATE_WRITING = 0x01,   
    SYN_FW_STATE_NEW = 0x02,       
    SYN_FW_STATE_TESTING = 0x03,   
    SYN_FW_STATE_CONFIRMED = 0x04, 
    SYN_FW_STATE_INVALID = 0x00,   
} SYN_FwState;

/* ── Image header ───────────────────────────────────────────────────────── */

typedef struct {
    uint32_t magic;        
    uint32_t version_code; 
    uint32_t image_size;   
    uint32_t image_crc;    
    uint8_t state;         
    uint8_t reserved[3];   
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    uint8_t image_hmac[32]; 
#endif
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    uint8_t image_sig[64]; 
#endif
    uint32_t header_crc; 
} SYN_FwImageHeader;

/* ── Validation ─────────────────────────────────────────────────────────── */

static inline uint32_t syn_fwimage_data_offset(uint32_t slot_addr)
{
    uint32_t sec_sz = syn_port_flash_sector_size(slot_addr);
    uint32_t min_sz = (uint32_t)sizeof(SYN_FwImageHeader);
    return (sec_sz > 0U) ? (((min_sz + sec_sz - 1U) / sec_sz) * sec_sz) : min_sz;
}

static inline bool syn_fwimage_header_valid(const SYN_FwImageHeader *hdr)
{
    if (hdr->magic != SYN_FW_MAGIC) {
        return false;
    }

    /* CRC-32 over all fields before header_crc, including padding and HMAC (if present).
     * offsetof correctly accounts for struct layout and padding. */
    uint32_t crc = syn_crc32(hdr, offsetof(SYN_FwImageHeader, header_crc));
    return (crc == hdr->header_crc);
}

static inline void syn_fwimage_seal_header(SYN_FwImageHeader *hdr)
{
    hdr->header_crc = syn_crc32(hdr, offsetof(SYN_FwImageHeader, header_crc));
}

static inline bool syn_fwimage_is_bootable(const SYN_FwImageHeader *hdr)
{
    if (!syn_fwimage_header_valid(hdr)) {
        return false;
    }
    return (hdr->state == SYN_FW_STATE_NEW || hdr->state == SYN_FW_STATE_TESTING ||
            hdr->state == SYN_FW_STATE_CONFIRMED);
}

static inline void syn_fwimage_encode_auth_meta(uint32_t version_code, uint32_t image_size,
                                                uint8_t out_meta[SYN_FWIMAGE_AUTH_META_SIZE])
{
    uint32_t fields[3];
    fields[0] = SYN_FW_MAGIC;
    fields[1] = version_code;
    fields[2] = image_size;
    for (size_t i = 0U; i < 3U; i++) {
        out_meta[i * 4U + 0U] = (uint8_t)(fields[i] & 0xFFU);
        out_meta[i * 4U + 1U] = (uint8_t)((fields[i] >> 8U) & 0xFFU);
        out_meta[i * 4U + 2U] = (uint8_t)((fields[i] >> 16U) & 0xFFU);
        out_meta[i * 4U + 3U] = (uint8_t)((fields[i] >> 24U) & 0xFFU);
    }
}

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
bool syn_fwimage_verify_signature(const SYN_FwImageHeader *hdr, uint32_t slot_addr,
                                  const uint8_t *public_key);
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYN_FWIMAGE_H */
```


