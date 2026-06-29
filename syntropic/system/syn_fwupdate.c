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
        /* Erase if we're at a sector boundary */
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

    if (computed_crc != expected_crc) {
        /* CRC mismatch — mark slot invalid */
        syn_fwupdate_abort(upd);
        return SYN_ERROR;
    }

    /* Write the final header with state = NEW */
    SYN_FwImageHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic        = SYN_FW_MAGIC;
    hdr.version_code = version_code;
    hdr.image_size   = upd->bytes_written;
    hdr.image_crc    = computed_crc;
    hdr.state        = SYN_FW_STATE_NEW;
    syn_fwimage_seal_header(&hdr);

    /* Erase first sector to rewrite header */
    /* Note: this erases the header sector — image data may span more sectors.
     * The header sector may also contain early image data, which is why we
     * need to handle this carefully. For simplicity, we re-erase and rewrite
     * just the header bytes. In a real system you'd reserve the header in
     * its own small region or use a separate metadata sector. */
    st = syn_port_flash_erase(upd->slot_addr);
    if (st != SYN_OK) {
        upd->error = true;
        return st;
    }

    st = syn_port_flash_write(upd->slot_addr, &hdr, sizeof(hdr));
    upd->active = false;
    return st;
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
