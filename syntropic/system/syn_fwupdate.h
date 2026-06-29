/**
 * @file syn_fwupdate.h
 * @brief Streaming firmware updater — transport-agnostic, zero-alloc.
 *
 * Receives firmware data in arbitrary-sized chunks, writes to flash,
 * and computes a running CRC-32. On completion, verifies the CRC and
 * writes the image header. On abort, marks the slot as invalid.
 *
 * The caller provides a page-aligned write buffer (typically 256 bytes,
 * matching flash write granularity). Data is buffered until a full page
 * is ready, then flushed to flash.
 *
 * @par Usage
 * @code
 *   static uint8_t page_buf[256];
 *   SYN_FwUpdate upd;
 *
 *   syn_fwupdate_begin(&upd, SLOT_B_ADDR, SLOT_B_SIZE,
 *                       page_buf, sizeof(page_buf));
 *
 *   // Feed chunks from HTTP, UART, BLE, etc.
 *   while (have_data) {
 *       syn_fwupdate_write(&upd, chunk, chunk_len);
 *   }
 *
 *   syn_fwupdate_finish(&upd, expected_crc, new_version);
 * @endcode
 * @ingroup syn_system
 */

#ifndef SYN_FWUPDATE_H
#define SYN_FWUPDATE_H

#include "../common/syn_defs.h"
#include "syn_fwimage.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Updater state ──────────────────────────────────────────────────────── */

/** @brief Firmware update context - manages streaming writes to flash. */
typedef struct {
    uint32_t  slot_addr;        /**< Flash base address of target slot     */
    uint32_t  data_addr;        /**< Flash address for image data (after hdr) */
    uint32_t  max_size;         /**< Maximum image size (excl. header)     */
    uint32_t  bytes_written;    /**< Total bytes written so far            */
    uint32_t  crc_state;        /**< Running CRC-32 state                  */

    uint8_t  *page_buf;         /**< Caller-provided write buffer          */
    uint16_t  page_buf_size;    /**< Buffer size (flash page granularity)  */
    uint16_t  page_buf_used;    /**< Bytes currently buffered              */

    bool      active;           /**< Update in progress?                   */
    bool      error;            /**< Error occurred?                       */
} SYN_FwUpdate;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Begin a firmware update.
 *
 * Erases the first sector of the target slot and prepares for writing.
 * The image header is written at slot_addr; image data starts at
 * slot_addr + sizeof(SYN_FwImageHeader).
 *
 * @param upd           Updater instance.
 * @param slot_addr     Flash base address of the target slot.
 * @param max_size      Maximum image size in bytes (excl. header).
 * @param page_buf      Caller-provided page buffer.
 * @param page_buf_size Buffer size (should match flash write granularity).
 * @return SYN_OK on success, SYN_ERROR on flash erase failure.
 */
SYN_Status syn_fwupdate_begin(SYN_FwUpdate *upd,
                               uint32_t slot_addr, uint32_t max_size,
                               uint8_t *page_buf, uint16_t page_buf_size);

/**
 * @brief Write a chunk of firmware data.
 *
 * Data is buffered until a full page is ready, then flushed to flash.
 * Automatically erases sectors as needed.
 *
 * @param upd   Updater instance.
 * @param data  Firmware data chunk.
 * @param len   Chunk length.
 * @return SYN_OK on success, SYN_ERROR on flash write failure or overflow.
 */
SYN_Status syn_fwupdate_write(SYN_FwUpdate *upd,
                               const uint8_t *data, size_t len);

/**
 * @brief Finalize the update.
 *
 * Flushes any remaining buffered data, verifies the CRC-32 matches
 * the expected value, and writes the image header with state = NEW.
 *
 * @param upd           Updater instance.
 * @param expected_crc  Expected CRC-32 of the full image.
 * @param version_code  Version code for the new image.
 * @return SYN_OK if CRC matches and header written,
 *         SYN_ERROR on CRC mismatch or flash failure.
 */
SYN_Status syn_fwupdate_finish(SYN_FwUpdate *upd,
                                uint32_t expected_crc,
                                uint32_t version_code);

/**
 * @brief Abort the update.
 *
 * Marks the slot as INVALID and cleans up.
 *
 * @param upd  Updater instance.
 */
void syn_fwupdate_abort(SYN_FwUpdate *upd);

/**
 * @brief Get bytes written so far.
 * @param upd  Updater instance.
 * @return Bytes written.
 */
static inline uint32_t syn_fwupdate_progress(const SYN_FwUpdate *upd)
{
    return upd->bytes_written;
}

/**
 * @brief Check if an update is in progress.
 * @param upd  Updater instance.
 * @return true if active.
 */
static inline bool syn_fwupdate_active(const SYN_FwUpdate *upd)
{
    return upd->active;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_FWUPDATE_H */
