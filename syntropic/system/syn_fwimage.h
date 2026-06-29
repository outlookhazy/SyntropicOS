/**
 * @file syn_fwimage.h
 * @brief Firmware image header — standardized format for OTA slots.
 *
 * Each firmware slot (A or B) has this header at a known flash offset.
 * The bootloader and OTA updater use it to decide which image to boot,
 * whether an update succeeded, and when to rollback.
 *
 * Layout in flash:
 * ```
 * [SYN_FwImageHeader (24 bytes)] [firmware binary (image_size bytes)]
 * ```
 *
 * @par Lifecycle
 * 1. OTA begins:   header.state = WRITING
 * 2. OTA complete: header.state = NEW (CRC verified)
 * 3. First boot:   bootloader sets state = TESTING
 * 4. App healthy:  app calls confirm → state = CONFIRMED
 * 5. App crashes:  bootloader sees TESTING + unhealthy → state = INVALID, rollback
 * @ingroup syn_system
 */

#ifndef SYN_FWIMAGE_H
#define SYN_FWIMAGE_H

#include "../common/syn_defs.h"
#include "../util/syn_crc.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Magic number ───────────────────────────────────────────────────────── */

/** @brief Magic number identifying a SyntropicOS firmware image header ("SYNF"). */
#define SYN_FW_MAGIC  0x53594E46u  /* "SYNF" in little-endian */

/* ── Image state ────────────────────────────────────────────────────────── */

/** @brief Firmware image lifecycle state. */
typedef enum {
    SYN_FW_STATE_EMPTY     = 0xFF,  /**< Erased flash (no image)           */
    SYN_FW_STATE_WRITING   = 0x01,  /**< Partially written (update in progress) */
    SYN_FW_STATE_NEW       = 0x02,  /**< Fully written + CRC valid         */
    SYN_FW_STATE_TESTING   = 0x03,  /**< Booted but not yet confirmed      */
    SYN_FW_STATE_CONFIRMED = 0x04,  /**< Confirmed good                    */
    SYN_FW_STATE_INVALID   = 0x00,  /**< Failed validation or rollback     */
} SYN_FwState;

/* ── Image header ───────────────────────────────────────────────────────── */

/** @brief Firmware image header — stored at the start of each flash slot (24 bytes). */
typedef struct {
    uint32_t  magic;            /**< SYN_FW_MAGIC                          */
    uint32_t  version_code;     /**< Packed version (SYN_VERSION_CODE fmt) */
    uint32_t  image_size;       /**< Firmware binary size (excl. header)   */
    uint32_t  image_crc;        /**< CRC-32 over the firmware binary       */
    uint8_t   state;            /**< SYN_FwState                           */
    uint8_t   reserved[3];      /**< Pad to 4-byte alignment               */
    uint32_t  header_crc;       /**< CRC-32 over bytes 0..19 of header     */
} SYN_FwImageHeader;            /* 24 bytes total */

/* ── Validation ─────────────────────────────────────────────────────────── */

/**
 * @brief Validate a firmware image header.
 *
 * Checks magic number and header CRC.
 * Does NOT verify the image data CRC (use syn_fwimage_verify_data for that).
 *
 * @param hdr  Header to validate.
 * @return true if header is valid.
 */
static inline bool syn_fwimage_header_valid(const SYN_FwImageHeader *hdr)
{
    if (hdr->magic != SYN_FW_MAGIC) return false;

    /* CRC-32 over the first 20 bytes (everything except header_crc) */
    uint32_t crc = syn_crc32(hdr, offsetof(SYN_FwImageHeader, header_crc));
    return (crc == hdr->header_crc);
}

/**
 * @brief Compute the header CRC and write it into the header.
 *
 * Call after setting all other fields.
 *
 * @param hdr  Header to seal.
 */
static inline void syn_fwimage_seal_header(SYN_FwImageHeader *hdr)
{
    hdr->header_crc = syn_crc32(hdr, offsetof(SYN_FwImageHeader, header_crc));
}

/**
 * @brief Check if a slot contains a bootable image.
 *
 * An image is bootable if the header is valid and state is
 * NEW, TESTING, or CONFIRMED.
 *
 * @param hdr  Header to check.
 * @return true if bootable.
 */
static inline bool syn_fwimage_is_bootable(const SYN_FwImageHeader *hdr)
{
    if (!syn_fwimage_header_valid(hdr)) return false;
    return (hdr->state == SYN_FW_STATE_NEW ||
            hdr->state == SYN_FW_STATE_TESTING ||
            hdr->state == SYN_FW_STATE_CONFIRMED);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_FWIMAGE_H */
