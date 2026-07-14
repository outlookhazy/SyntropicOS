#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_BOOT) || SYN_USE_BOOT

/**
 * @file syn_fwupdate.c
 * @brief Streaming firmware updater implementation.
 */

#include "syn_fwupdate.h"
#include "syn_fwimage.h"
#include "../util/syn_assert.h"
#include "../util/syn_crc.h"
#include "../port/syn_port_flash.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Flush the page buffer to flash.
 * @param upd  Firmware update context.
 * @return SYN_OK on success.
 */
static SYN_Status flush_page(SYN_FwUpdate *upd)
{
    if (upd->page_buf_used == 0) return SYN_OK;

    uint32_t addr = upd->data_addr + upd->bytes_written;

    /* Check if we need to erase a new sector */
    uint32_t sector_size = syn_port_flash_sector_size(addr);
    if (sector_size > 0) {
        uint32_t sector_start = addr - (addr % sector_size);
        /* Erase if we're at a sector boundary and have written data before */
        if (addr == sector_start && upd->bytes_written > 0) {
            SYN_Status st = syn_port_flash_erase(sector_start);
            if (st != SYN_OK) return st;
        }
    }

    /* Write the buffered data */
    SYN_Status st = syn_port_flash_write(addr, upd->page_buf,
                                          upd->page_buf_used);
    if (st != SYN_OK) return st;

    upd->bytes_written += upd->page_buf_used;
    upd->page_buf_used = 0;

    return SYN_OK;
}

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_fwupdate_begin(SYN_FwUpdate *upd,
                               uint32_t slot_addr, uint32_t max_size,
                               uint8_t *page_buf, uint16_t page_buf_size)
{
    SYN_ASSERT(upd != NULL);
    SYN_ASSERT(page_buf != NULL);
    SYN_ASSERT(page_buf_size > 0);

    memset(upd, 0, sizeof(*upd));
    upd->slot_addr     = slot_addr;
    upd->data_addr     = slot_addr + (uint32_t)sizeof(SYN_FwImageHeader);
    upd->max_size      = max_size;
    upd->page_buf      = page_buf;
    upd->page_buf_size = page_buf_size;
    upd->crc_state     = SYN_CRC32_INIT;
    upd->active        = true;

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    /* HMAC is initialized lazily in set_key(); key_set starts false
     * because memset above zeroed the struct. */
#endif

    /* Erase the first sector (contains the header) */
    SYN_Status st = syn_port_flash_erase(slot_addr);
    if (st != SYN_OK) {
        upd->error = true;
        upd->active = false;
        return st;
    }

    /* Write a WRITING header so the bootloader knows this slot is dirty */
    SYN_FwImageHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = SYN_FW_MAGIC;
    hdr.state = SYN_FW_STATE_WRITING;
    syn_fwimage_seal_header(&hdr);

    st = syn_port_flash_write(slot_addr, &hdr, sizeof(hdr));
    if (st != SYN_OK) {
        upd->error = true;
        upd->active = false;
    }
    return st;
}

SYN_Status syn_fwupdate_write(SYN_FwUpdate *upd,
                               const uint8_t *data, size_t len)
{
    SYN_ASSERT(upd != NULL);
    if (!upd->active || upd->error) return SYN_ERROR;
    if (data == NULL || len == 0) return SYN_OK;

    /* Check size limit */
    if (upd->bytes_written + upd->page_buf_used + len > upd->max_size) {
        upd->error = true;
        return SYN_ERROR;
    }

    /* Update running CRC */
    upd->crc_state = syn_crc32_update(upd->crc_state, data, len);

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    /* Update running HMAC if key was provided */
    if (upd->key_set) {
        syn_hmac_sha256_update(&upd->hmac_ctx, data, len);
    }
#endif

    /* Buffer data and flush full pages */
    size_t offset = 0;
    while (offset < len) {
        size_t space = (size_t)(upd->page_buf_size - upd->page_buf_used);
        size_t to_copy = len - offset;
        if (to_copy > space) to_copy = space;

        memcpy(upd->page_buf + upd->page_buf_used, data + offset, to_copy);
        upd->page_buf_used += (uint16_t)to_copy;
        offset += to_copy;

        /* Flush if buffer is full */
        if (upd->page_buf_used >= upd->page_buf_size) {
            SYN_Status st = flush_page(upd);
            if (st != SYN_OK) {
                upd->error = true;
                return st;
            }
        }
    }

    return SYN_OK;
}

SYN_Status syn_fwupdate_finish(SYN_FwUpdate *upd,
                                uint32_t expected_crc,
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
                                const uint8_t *expected_hmac,
#endif
                                uint32_t version_code)
{
    SYN_ASSERT(upd != NULL);
    if (!upd->active || upd->error) return SYN_ERROR;

    /* Flush remaining data */
    SYN_Status st = flush_page(upd);
    if (st != SYN_OK) {
        upd->error = true;
        return st;
    }

    /* Finalize CRC */
    uint32_t computed_crc = syn_crc32_final(upd->crc_state);

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    /* Finalize HMAC before CRC check so the digest is available for
     * both verification and storage in the header. */
    uint8_t computed_hmac[32] = {0};
    if (upd->key_set) {
        syn_hmac_sha256_final(&upd->hmac_ctx, computed_hmac);
    }
#endif

    if (computed_crc != expected_crc) {
        /* CRC mismatch — mark slot invalid */
        syn_fwupdate_abort(upd);
        return SYN_ERROR;
    }

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    /* Verify HMAC if caller provided an expected value */
    if (expected_hmac != NULL && upd->key_set) {
        /* Constant-time compare to prevent timing side channels */
        uint8_t diff = 0;
        size_t i;
        for (i = 0; i < 32; i++) {
            diff |= computed_hmac[i] ^ expected_hmac[i];
        }
        if (diff != 0) {
            syn_fwupdate_abort(upd);
            return SYN_ERROR;
        }
    }
#endif

    /* Write the final header with state = NEW */
    SYN_FwImageHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic        = SYN_FW_MAGIC;
    hdr.version_code = version_code;
    hdr.image_size   = upd->bytes_written;
    hdr.image_crc    = computed_crc;
    hdr.state        = SYN_FW_STATE_NEW;

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    /* Store HMAC in header if key was set */
    if (upd->key_set) {
        memcpy(hdr.image_hmac, computed_hmac, 32);
    }
#endif

    syn_fwimage_seal_header(&hdr);

    /* Erase first sector to rewrite header */
    st = syn_port_flash_erase(upd->slot_addr);
    if (st != SYN_OK) {
        upd->error = true;
        syn_fwupdate_abort(upd);
        return st;
    }

    st = syn_port_flash_write(upd->slot_addr, &hdr, sizeof(hdr));
    if (st != SYN_OK) {
        upd->error = true;
        syn_fwupdate_abort(upd);
        return st;
    }

    upd->active = false;
    return SYN_OK;
}

void syn_fwupdate_abort(SYN_FwUpdate *upd)
{
    SYN_ASSERT(upd != NULL);
    if (!upd->active) return;

    /* Write an INVALID header */
    SYN_FwImageHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = SYN_FW_MAGIC;
    hdr.state = SYN_FW_STATE_INVALID;
    syn_fwimage_seal_header(&hdr);

    /* Best-effort — ignore errors during abort */
    syn_port_flash_erase(upd->slot_addr);
    syn_port_flash_write(upd->slot_addr, &hdr, sizeof(hdr));

    upd->active = false;
    upd->error  = true;
}

#endif /* SYN_USE_BOOT */

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC

void syn_fwupdate_set_key(SYN_FwUpdate *upd,
                           const void *key, size_t key_len)
{
    SYN_ASSERT(upd != NULL);
    SYN_ASSERT(key != NULL);
    syn_hmac_sha256_init(&upd->hmac_ctx, key, key_len);
    upd->key_set = true;
}

#endif /* SYN_FW_USE_HMAC */
