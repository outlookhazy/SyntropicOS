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

/** @brief Firmware update context - manages streaming writes to flash. */
typedef struct {
    uint32_t slot_addr;     /**< Flash base address of target slot     */
    uint32_t data_addr;     /**< Flash address for image data (after hdr) */
    uint32_t erased_up_to;  /**< End address of erased flash region    */
    uint32_t max_size;      /**< Maximum image size (excl. header)     */
    uint32_t bytes_written; /**< Total bytes written so far            */
    uint32_t crc_state;     /**< Running CRC-32 state                  */

    uint8_t *page_buf;      /**< Caller-provided write buffer          */
    uint16_t page_buf_size; /**< Buffer size (flash page granularity)  */
    uint16_t page_buf_used; /**< Bytes currently buffered              */

    bool active; /**< Update in progress?                   */
    bool error;  /**< Error occurred?                       */

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    SYN_HMAC_SHA256 hmac_ctx; /**< Running HMAC-SHA256 context           */
    bool key_set;             /**< HMAC key was provided?                */
#endif

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    SYN_SHA512_Ctx sha512_ctx; /**< Running SHA-512 context for Ed25519   */
    uint8_t public_key[32];    /**< Expected Ed25519 public key           */
    bool pubkey_set;           /**< Public key was provided?              */
#endif

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
    SYN_AES_Context gcm_aes;    /**< AES block cipher context              */
    uint8_t gcm_h[16];          /**< GHASH subkey H                        */
    uint8_t gcm_j0[16];         /**< Initial counter block J0              */
    uint8_t gcm_ctr[16];        /**< Current CTR counter block             */
    uint8_t gcm_s[16];          /**< Running GHASH accumulator             */
    uint8_t gcm_partial_ct[16]; /**< Partial ciphertext block buffer       */
    uint8_t gcm_partial_used;   /**< Bytes used in partial buffer          */
    uint8_t gcm_stream_buf[16]; /**< Buffered CTR keystream block          */
    uint8_t gcm_stream_used;    /**< Number of keystream bytes used        */
    uint64_t gcm_total_bytes;   /**< Total ciphertext bytes processed      */
    bool gcm_key_set;           /**< GCM key and IV configured?            */
#endif
} SYN_FwUpdate;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Begin a firmware update.
 *
 * Erases the header sector of the target slot and prepares for writing.
 * The image header is written at slot_addr; image data starts at
 * slot_addr + syn_fwimage_data_offset(slot_addr).
 *
 * @param upd           Updater instance.
 * @param slot_addr     Flash base address of the target slot.
 * @param max_size      Maximum image size in bytes (excl. header).
 * @param page_buf      Caller-provided page buffer.
 * @param page_buf_size Buffer size (should match flash write granularity).
 * @return SYN_OK on success, SYN_ERROR on flash erase failure.
 */
SYN_Status syn_fwupdate_begin(SYN_FwUpdate *upd, uint32_t slot_addr, uint32_t max_size,
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
SYN_Status syn_fwupdate_write(SYN_FwUpdate *upd, const uint8_t *data, size_t len);

/**
 * @brief Finalize the update.
 *
 * Flushes any remaining buffered data, verifies the CRC-32 matches
 * the expected value, and writes the image header with state = NEW.
 *
 * @param upd            Updater instance.
 * @param expected_crc   Expected CRC-32 of the full image.
 * @param version_code   Version code for the new image.
 * @return SYN_OK if verification passes and header written,
 *         SYN_ERROR on mismatch or flash failure.
 */
SYN_Status syn_fwupdate_finish(SYN_FwUpdate *upd, uint32_t expected_crc,
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
                               const uint8_t *expected_hmac,
#endif
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
                               const uint8_t *expected_sig,
#endif
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

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
/**
 * @brief Set the HMAC signing key for the current update.
 *
 * Must be called after syn_fwupdate_begin() and before the first
 * syn_fwupdate_write(). When set, the updater computes a running
 * HMAC-SHA256 alongside the CRC-32.
 *
 * @param upd      Updater instance.
 * @param key      HMAC key.
 * @param key_len  Key length in bytes.
 */
void syn_fwupdate_set_key(SYN_FwUpdate *upd, const void *key, size_t key_len);
#endif

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
/**
 * @brief Set the Ed25519 public key for the current update.
 *
 * Must be called after syn_fwupdate_begin() and before the first
 * syn_fwupdate_write(). When set, the updater computes a running
 * SHA-512 challenge digest alongside the CRC-32.
 *
 * @param upd         Updater instance.
 * @param public_key  32-byte Ed25519 public key.
 */
void syn_fwupdate_set_public_key(SYN_FwUpdate *upd, const uint8_t *public_key);
#endif

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
/**
 * @brief Set the AES-GCM decryption key and IV for encrypted streaming firmware updates.
 *
 * Must be called after syn_fwupdate_begin() and before the first syn_fwupdate_write().
 *
 * @param upd     Updater instance.
 * @param key     Secret key (16, 24, or 32 bytes).
 * @param key_len Key length in bytes.
 * @param iv      12-byte initialization vector / nonce.
 * @param iv_len  IV length in bytes (must be 12).
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid inputs.
 */
SYN_Status syn_fwupdate_set_aes_gcm_key(SYN_FwUpdate *upd, const uint8_t *key, size_t key_len,
                                        const uint8_t *iv, size_t iv_len);

/**
 * @brief Finalize an encrypted update and verify 16-byte AES-GCM authentication tag.
 *
 * @param upd          Updater instance.
 * @param expected_tag Expected 16-byte GCM authentication tag.
 * @param version_code Version code for the new image.
 * @return SYN_OK on tag verification success and header written, SYN_ERROR on tag mismatch or
 * flash error.
 */
SYN_Status syn_fwupdate_finish_gcm(SYN_FwUpdate *upd, const uint8_t expected_tag[16],
                                   uint32_t version_code);
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYN_FWUPDATE_H */
