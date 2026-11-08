/**
 * @file syn_ota.h
 * @brief Secure Streaming OTA Orchestrator — zero-heap, transport-agnostic.
 *
 * Coordinates streaming firmware updates across heterogeneous transports
 * (CoAP Block2, HTTP, BLE, LwM2M Object 5, UART/YMODEM) directly into flash.
 *
 * Integrates dual-bank flash slot selection (syn_fwboot), streaming page
 * flushing and flash sector management (syn_fwupdate), cryptographic verification
 * (CRC-32, HMAC-SHA256, Ed25519, AES-GCM), and LwM2M Object 5 lifecycle state sync.
 *
 * @par Example Usage
 * @code
 *   static uint8_t page_buf[256];
 *   SYN_FwBootManager boot_mgr;
 *   SYN_OTA_Manager ota;
 *
 *   syn_fwboot_init(&boot_mgr, SLOT_A_ADDR, SLOT_B_ADDR);
 *   syn_ota_init(&ota, &boot_mgr, SLOT_SIZE, page_buf, sizeof(page_buf));
 *
 *   // Optional: configure cryptographic verification (e.g. Ed25519)
 *   syn_ota_set_verification_key(&ota, SYN_OTA_CRYPTO_ED25519, pubkey, 32U);
 *
 *   // Begin OTA session
 *   syn_ota_begin(&ota, fw_total_len, new_version_code, expected_crc32);
 *
 *   // Stream incoming chunks from transport
 *   while (receiving_chunks) {
 *       syn_ota_write_chunk(&ota, chunk_data, chunk_len);
 *   }
 *
 *   // Finish and verify signature
 *   if (syn_ota_finish(&ota, expected_signature, 64U) == SYN_OK) {
 *       syn_ota_apply(&ota); // Ready to reboot into new image
 *   }
 * @endcode
 *
 * @ingroup syn_system
 */

#ifndef SYN_OTA_H
#define SYN_OTA_H

#include "../common/syn_defs.h"

#if (!defined(SYN_USE_OTA) || SYN_USE_OTA) && (!defined(SYN_USE_BOOT) || SYN_USE_BOOT)
/** @brief Non-zero when OTA subsystem and bootloader dependencies are enabled. */
#define SYN_OTA_ENABLED 1
#else
/** @brief Non-zero when OTA subsystem and bootloader dependencies are enabled. */
#define SYN_OTA_ENABLED 0
#endif

#if SYN_OTA_ENABLED

#include "syn_fwboot.h"
#include "syn_fwimage.h"
#include "syn_fwupdate.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Enums & Types ───────────────────────────────────────────────────────── */

/**
 * @brief OTA update lifecycle states.
 */
typedef enum {
    SYN_OTA_STATE_IDLE = 0,       /**< 0: Idle / No update active          */
    SYN_OTA_STATE_DOWNLOADING,    /**< 1: Streaming chunks into flash slot */
    SYN_OTA_STATE_DOWNLOADED,     /**< 2: All chunks received in flash     */
    SYN_OTA_STATE_VERIFYING,      /**< 3: Verifying crypto / signatures    */
    SYN_OTA_STATE_READY_TO_APPLY, /**< 4: Slot valid & sealed, ready to boot */
    SYN_OTA_STATE_APPLIED,        /**< 5: Scheduled for execution on reboot */
    SYN_OTA_STATE_ERROR,          /**< 6: Failed / Aborted                 */
} SYN_OTA_State;

/**
 * @brief Cryptographic verification mode for the OTA image.
 */
typedef enum {
    SYN_OTA_CRYPTO_NONE = 0,    /**< 0: CRC-32 only                      */
    SYN_OTA_CRYPTO_HMAC_SHA256, /**< 1: HMAC-SHA256 authenticated tag    */
    SYN_OTA_CRYPTO_ED25519,     /**< 2: Ed25519 digital signature        */
    SYN_OTA_CRYPTO_AES_GCM,     /**< 3: AES-GCM authenticated decryption */
} SYN_OTA_CryptoMode;

/**
 * @brief OTA Error Codes.
 */
typedef enum {
    SYN_OTA_ERR_NONE = 0,           /**< 0: No error                         */
    SYN_OTA_ERR_INVALID_PARAM,      /**< 1: Invalid parameters or NULL ptrs  */
    SYN_OTA_ERR_NO_FLASH_SLOT,      /**< 2: Target flash slot invalid/unavailable */
    SYN_OTA_ERR_FLASH_ERASE,        /**< 3: Flash sector erase failed        */
    SYN_OTA_ERR_FLASH_WRITE,        /**< 4: Flash page write failed          */
    SYN_OTA_ERR_OUT_OF_SPACE,       /**< 5: Firmware size exceeds slot capacity */
    SYN_OTA_ERR_INTEGRITY_CHECK,    /**< 6: CRC or signature mismatch        */
    SYN_OTA_ERR_UNSUPPORTED_CRYPTO, /**< 7: Crypto mode disabled at compile time */
    SYN_OTA_ERR_INVALID_STATE,      /**< 8: Operation invalid for current state */
} SYN_OTA_ErrorCode;

/**
 * @brief Automatic slot selection constant.
 */
#define SYN_OTA_SLOT_AUTO 0xFFU

/* ── Context Structure ───────────────────────────────────────────────────── */

/**
 * @brief Zero-heap Streaming OTA Manager context.
 */
typedef struct {
    SYN_FwBootManager *boot_mgr; /**< Pointer to dual-bank boot manager   */
    SYN_FwUpdate fw_upd;         /**< Underlying streaming flash updater  */
    uint8_t *page_buf;           /**< Flash write page buffer             */
    size_t page_buf_sz;          /**< Page buffer size                    */
    uint32_t slot_size;          /**< Maximum slot size in bytes          */

    uint8_t target_slot;       /**< Chosen slot (SLOT_A, SLOT_B, or AUTO) */
    uint32_t target_slot_addr; /**< Target slot base flash address      */

    SYN_OTA_State state;            /**< Current OTA lifecycle state         */
    SYN_OTA_ErrorCode last_error;   /**< Last recorded error code            */
    SYN_OTA_CryptoMode crypto_mode; /**< Selected cryptographic mode         */

    uint32_t expected_total_sz; /**< Declared total firmware size (bytes)*/
    uint32_t target_version;    /**< Declared new version code           */
    uint32_t expected_crc;      /**< Expected CRC-32 of image data       */

    void *lwm2m_fw_ctx; /**< Optional SYN_LwM2M_FirmwareContext* */

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    uint8_t hmac_key[32]; /**< HMAC-SHA256 secret key              */
    size_t hmac_key_len;  /**< HMAC key length                     */
    bool hmac_key_set;    /**< HMAC key configured flag            */
#endif

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    uint8_t ed25519_pubkey[32]; /**< Ed25519 public key (32 bytes)       */
    bool ed25519_key_set;       /**< Ed25519 key configured flag         */
#endif

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
    uint8_t gcm_key[32]; /**< AES-GCM secret key                  */
    size_t gcm_key_len;  /**< AES key length (16, 24, 32 bytes)   */
    uint8_t gcm_iv[12];  /**< AES-GCM 12-byte IV/Nonce            */
    bool gcm_key_set;    /**< GCM key configured flag             */
#endif
} SYN_OTA_Manager;

/* ── API ─────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the OTA manager.
 *
 * @param mgr          OTA manager instance to initialize.
 * @param boot_mgr     Dual-bank boot manager instance.
 * @param slot_size    Total flash capacity per slot in bytes (must exceed header sector size).
 * @param page_buf     Caller-provided page-aligned flash write buffer.
 * @param page_buf_sz  Size of page buffer (must match flash write granularity).
 * @return SYN_OK on success, SYN_INVALID_PARAM on NULL or invalid inputs.
 */
SYN_Status syn_ota_init(SYN_OTA_Manager *mgr, SYN_FwBootManager *boot_mgr, uint32_t slot_size,
                        uint8_t *page_buf, size_t page_buf_sz);

/**
 * @brief Configure target flash slot (explicit slot index or automatic selection).
 *
 * If set to SYN_OTA_SLOT_AUTO, syn_ota_begin() will automatically pick the
 * currently inactive slot based on boot_mgr->active_slot.
 *
 * @param mgr       OTA manager instance.
 * @param slot_idx  SYN_FW_SLOT_A (0), SYN_FW_SLOT_B (1), or SYN_OTA_SLOT_AUTO (0xFF).
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid slot index.
 */
SYN_Status syn_ota_set_target_slot(SYN_OTA_Manager *mgr, uint8_t slot_idx);

/**
 * @brief Configure cryptographic verification key.
 *
 * @param mgr      OTA manager instance.
 * @param mode     Cryptographic verification mode.
 * @param key      Secret key or public key buffer.
 * @param key_len  Length of key buffer.
 * @return SYN_OK on success, SYN_INVALID_PARAM or SYN_ERROR on unsupported mode.
 */
SYN_Status syn_ota_set_verification_key(SYN_OTA_Manager *mgr, SYN_OTA_CryptoMode mode,
                                        const uint8_t *key, size_t key_len);

/**
 * @brief Configure AES-GCM decryption key and IV for encrypted updates.
 *
 * @param mgr      OTA manager instance.
 * @param key      AES key (16, 24, or 32 bytes).
 * @param key_len  Key length in bytes.
 * @param iv       12-byte initialization vector / nonce.
 * @param iv_len   Length of IV (must be 12).
 * @return SYN_OK on success, SYN_INVALID_PARAM or SYN_ERROR if AES-GCM disabled.
 */
SYN_Status syn_ota_set_aes_gcm_params(SYN_OTA_Manager *mgr, const uint8_t *key, size_t key_len,
                                      const uint8_t *iv, size_t iv_len);

/**
 * @brief Begin a new OTA firmware update session.
 *
 * Resolves target slot, erases initial flash sector, primes running digests,
 * and enters SYN_OTA_STATE_DOWNLOADING.
 *
 * @param mgr                 OTA manager instance.
 * @param expected_total_sz   Total expected firmware binary size (excl header).
 * @param target_version      Version code for the incoming firmware.
 * @param expected_crc        Expected CRC-32 checksum (0 to compute dynamically).
 * @return SYN_OK on success, error code on flash erase or configuration failure.
 */
SYN_Status syn_ota_begin(SYN_OTA_Manager *mgr, uint32_t expected_total_sz, uint32_t target_version,
                         uint32_t expected_crc);

/**
 * @brief Write a chunk of incoming firmware stream into the target flash slot.
 *
 * @param mgr       OTA manager instance.
 * @param chunk     Incoming data chunk buffer.
 * @param chunk_sz  Length of chunk in bytes.
 * @return SYN_OK on success, SYN_ERROR on flash write failure, overflow, or invalid state.
 */
SYN_Status syn_ota_write_chunk(SYN_OTA_Manager *mgr, const uint8_t *chunk, size_t chunk_sz);

/**
 * @brief Finalize and verify the downloaded firmware image.
 *
 * Flushes page buffers, verifies CRC-32 and cryptographic signature / HMAC / GCM tag,
 * and writes the new image header to mark the slot as SYN_FW_STATE_NEW.
 *
 * @param mgr                  OTA manager instance.
 * @param expected_sig_or_tag  Expected signature, HMAC, or GCM tag (NULL if CRC-32 only).
 * @param sig_len              Length of expected signature / tag buffer.
 * @return SYN_OK on successful verification, SYN_ERROR on verification failure.
 */
SYN_Status syn_ota_finish(SYN_OTA_Manager *mgr, const uint8_t *expected_sig_or_tag, size_t sig_len);

/**
 * @brief Mark the verified slot as ready for immediate boot on next system restart.
 *
 * Promotes OTA state to SYN_OTA_STATE_APPLIED and synchronizes LwM2M Object 5.
 *
 * @param mgr OTA manager instance.
 * @return SYN_OK on success, SYN_ERROR if not in READY_TO_APPLY state.
 */
SYN_Status syn_ota_apply(SYN_OTA_Manager *mgr);

/**
 * @brief Abort an active or failed OTA session.
 *
 * Invalidate the target flash slot header and resets state to SYN_OTA_STATE_IDLE or ERROR.
 *
 * @param mgr OTA manager instance.
 * @param err Error code triggering the abort.
 */
void syn_ota_abort(SYN_OTA_Manager *mgr, SYN_OTA_ErrorCode err);

/**
 * @brief Query current OTA download and flash write progress.
 *
 * @param mgr          OTA manager instance.
 * @param out_written  [out] Bytes written so far (optional, may be NULL).
 * @param out_total    [out] Total expected bytes (optional, may be NULL).
 * @param out_percent  [out] Integer percentage 0..100 (optional, may be NULL).
 * @return SYN_OK on success, SYN_INVALID_PARAM on NULL mgr.
 */
SYN_Status syn_ota_get_progress(const SYN_OTA_Manager *mgr, uint32_t *out_written,
                                uint32_t *out_total, uint8_t *out_percent);

/**
 * @brief Get current OTA lifecycle state.
 *
 * @param mgr OTA manager instance.
 * @return Current SYN_OTA_State.
 */
SYN_OTA_State syn_ota_get_state(const SYN_OTA_Manager *mgr);

/**
 * @brief Get last recorded error code.
 *
 * @param mgr OTA manager instance.
 * @return Last SYN_OTA_ErrorCode.
 */
SYN_OTA_ErrorCode syn_ota_get_last_error(const SYN_OTA_Manager *mgr);

/**
 * @brief Bind an OMA LwM2M Object 5 (Firmware Update) context to the OTA manager.
 *
 * Allows automatic bidirectional synchronization between OTA progress/states and LwM2M.
 *
 * @param mgr          OTA manager instance.
 * @param lwm2m_fw_ctx Pointer to SYN_LwM2M_FirmwareContext instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_ota_bind_lwm2m(SYN_OTA_Manager *mgr, void *lwm2m_fw_ctx);

/**
 * @brief Synchronize OTA state and results into bound LwM2M Object 5 context.
 *
 * @param mgr OTA manager instance.
 */
void syn_ota_sync_lwm2m(SYN_OTA_Manager *mgr);

#ifdef __cplusplus
}
#endif

#endif /* SYN_OTA_ENABLED */

#endif /* SYN_OTA_H */
