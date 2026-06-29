/**
 * @file syn_fwboot.h
 * @brief A/B firmware boot manager — slot selection, rollback, confirm.
 *
 * Works with syn_fwimage headers to manage dual firmware slots.
 * Integrates with syn_boot for crash-loop detection.
 *
 * @par Boot Flow
 * 1. Bootloader reads both slot headers
 * 2. syn_fwboot_select() picks the best slot to boot
 * 3. If selected slot is NEW → mark as TESTING before jumping
 * 4. Application calls syn_fwboot_confirm() after healthy startup
 * 5. If boot fails → next boot sees TESTING + unhealthy → rollback
 *
 * @par Usage
 * @code
 *   SYN_FwBootManager mgr;
 *   syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR);
 *
 *   // In bootloader: pick which slot to run
 *   uint8_t slot = syn_fwboot_select(&mgr);
 *   uint32_t entry = syn_fwboot_slot_addr(&mgr, slot)
 *                    + sizeof(SYN_FwImageHeader);
 *   // Jump to entry...
 *
 *   // In application: after successful startup
 *   syn_fwboot_confirm(&mgr);
 * @endcode
 * @ingroup syn_system
 */

#ifndef SYN_FWBOOT_H
#define SYN_FWBOOT_H

#include "../common/syn_defs.h"
#include "syn_fwimage.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Slot indices ───────────────────────────────────────────────────────── */

#define SYN_FW_SLOT_A  0      /**< Firmware slot A index.  */
#define SYN_FW_SLOT_B  1      /**< Firmware slot B index.  */
#define SYN_FW_SLOT_NONE  0xFF /**< No valid slot.          */

/* ── Boot manager ───────────────────────────────────────────────────────── */

/** @brief A/B firmware boot manager — slot selection and rollback state. */
typedef struct {
    uint32_t           slot_addr[2];   /**< Flash base of slot A and B     */
    SYN_FwImageHeader  slot_hdr[2];    /**< Cached headers                 */
    uint8_t            active_slot;    /**< Currently running slot          */
    bool               initialized;    /**< Init complete                   */
} SYN_FwBootManager;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the boot manager.
 *
 * Reads image headers from both slots.
 *
 * @param mgr     Boot manager instance.
 * @param slot_a  Flash base address of slot A.
 * @param slot_b  Flash base address of slot B.
 * @return SYN_OK on success.
 */
SYN_Status syn_fwboot_init(SYN_FwBootManager *mgr,
                            uint32_t slot_a, uint32_t slot_b);

/**
 * @brief Select the best slot to boot.
 *
 * Priority:
 * 1. TESTING slot (continue testing a new image)
 * 2. NEW slot (first boot of a new image — marks as TESTING)
 * 3. CONFIRMED slot with highest version
 * 4. Any CONFIRMED slot
 *
 * If a TESTING slot failed (called with rollback=true), it's marked
 * INVALID and the other CONFIRMED slot is selected.
 *
 * @param mgr       Boot manager.
 * @param rollback  true if the previous boot was unhealthy (trigger rollback).
 * @return Slot index (SYN_FW_SLOT_A or SYN_FW_SLOT_B), or SYN_FW_SLOT_NONE.
 */
uint8_t syn_fwboot_select(SYN_FwBootManager *mgr, bool rollback);

/**
 * @brief Confirm the currently active slot as good.
 *
 * Promotes state from TESTING to CONFIRMED. Call after the application
 * has started successfully (after syn_boot_mark_healthy).
 *
 * @param mgr  Boot manager.
 * @return SYN_OK if confirmed, SYN_ERROR if not in TESTING state.
 */
SYN_Status syn_fwboot_confirm(SYN_FwBootManager *mgr);

/**
 * @brief Get the flash base address of a slot.
 * @param mgr   Boot manager.
 * @param slot  Slot index (SYN_FW_SLOT_A or SYN_FW_SLOT_B).
 * @return Flash base address, or 0 if invalid slot.
 */
static inline uint32_t syn_fwboot_slot_addr(const SYN_FwBootManager *mgr,
                                             uint8_t slot)
{
    return (slot < 2) ? mgr->slot_addr[slot] : 0;
}

/**
 * @brief Get the cached header for a slot.
 * @param mgr   Boot manager.
 * @param slot  Slot index.
 * @return Pointer to cached header, or NULL if invalid slot.
 */
static inline const SYN_FwImageHeader *syn_fwboot_slot_header(
    const SYN_FwBootManager *mgr, uint8_t slot)
{
    return (slot < 2) ? &mgr->slot_hdr[slot] : NULL;
}

/**
 * @brief Get the currently active slot.
 * @param mgr  Boot manager.
 * @return Active slot index.
 */
static inline uint8_t syn_fwboot_active_slot(const SYN_FwBootManager *mgr)
{
    return mgr->active_slot;
}

/**
 * @brief Reload slot headers from flash.
 *
 * Call after an OTA update to refresh the cached state.
 *
 * @param mgr  Boot manager.
 * @return SYN_OK on success.
 */
SYN_Status syn_fwboot_refresh(SYN_FwBootManager *mgr);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FWBOOT_H */
