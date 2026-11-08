#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_BOOT) || SYN_USE_BOOT

/**
 * @file syn_fwboot.c
 * @brief A/B firmware boot manager implementation.
 */

#include "../port/syn_port_flash.h"
#include "../util/syn_assert.h"
#include "syn_fwboot.h"
#include "syn_fwimage.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Read a firmware image header from flash.
 * @param addr  Flash address.
 * @param hdr   [out] Header struct.
 * @return SYN_OK on success.
 */
static SYN_Status read_header(uint32_t addr, SYN_FwImageHeader *hdr)
{
    return syn_port_flash_read(addr, hdr, sizeof(*hdr));
}

/**
 * @brief Write a new state to a firmware header (erase + rewrite).
 * @param addr       Flash address of the header.
 * @param hdr        Current header.
 * @param new_state  New state value.
 * @return SYN_OK on success.
 */
static SYN_Status write_state(uint32_t addr, const SYN_FwImageHeader *hdr, uint8_t new_state)
{
    SYN_FwImageHeader updated = *hdr;
    updated.state = new_state;
    syn_fwimage_seal_header(&updated);

    /* Erase reserved header sector(s) and rewrite */
    uint32_t hdr_span = syn_fwimage_data_offset(addr);
    uint32_t erased = 0U;
    while (erased < hdr_span) {
        uint32_t raw_sec = syn_port_flash_sector_size(addr + erased);
        uint32_t sec_sz = (raw_sec > 0U) ? raw_sec : hdr_span;
        SYN_Status st = syn_port_flash_erase(addr + erased);
        if (st != SYN_OK) {
            return st;
        }
        erased += sec_sz;
    }

    return syn_port_flash_write(addr, &updated, sizeof(updated));
}

/**
 * @brief Invalidate any TESTING slot during rollback recovery.
 * @param mgr    Boot manager instance.
 * @param valid  Per-slot header validity flags (updated in place).
 */
static void fwboot_rollback_testing(SYN_FwBootManager *mgr, bool valid[2])
{
    for (uint8_t i = 0U; i < 2U; i++) {
        if (valid[i] && mgr->slot_hdr[i].state == SYN_FW_STATE_TESTING) {
            (void)write_state(mgr->slot_addr[i], &mgr->slot_hdr[i], SYN_FW_STATE_INVALID);
            mgr->slot_hdr[i].state = SYN_FW_STATE_INVALID;
            syn_fwimage_seal_header(&mgr->slot_hdr[i]);
            valid[i] = false;
        }
    }
}

/**
 * @brief Pick the CONFIRMED slot with the highest version code.
 * @param mgr    Boot manager instance.
 * @param valid  Per-slot header validity flags.
 * @return Selected slot index, or SYN_FW_SLOT_NONE.
 */
static uint8_t fwboot_pick_confirmed(const SYN_FwBootManager *mgr, const bool valid[2])
{
    uint8_t best = SYN_FW_SLOT_NONE;
    uint32_t best_ver = 0U;
    for (uint8_t i = 0U; i < 2U; i++) {
        if (valid[i] && mgr->slot_hdr[i].state == SYN_FW_STATE_CONFIRMED) {
            if (best == SYN_FW_SLOT_NONE || mgr->slot_hdr[i].version_code > best_ver) {
                best = i;
                best_ver = mgr->slot_hdr[i].version_code;
            }
        }
    }
    return best;
}

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_fwboot_init(SYN_FwBootManager *mgr, uint32_t slot_a, uint32_t slot_b)
{
    SYN_ASSERT(mgr != NULL);

    memset(mgr, 0, sizeof(*mgr));
    mgr->slot_addr[0] = slot_a;
    mgr->slot_addr[1] = slot_b;
    mgr->active_slot = SYN_FW_SLOT_NONE;
    mgr->initialized = true;

    return syn_fwboot_refresh(mgr);
}

uint8_t syn_fwboot_select(SYN_FwBootManager *mgr, bool rollback)
{
    SYN_ASSERT(mgr != NULL && mgr->initialized);

    bool valid[2];
    valid[0] = syn_fwimage_header_valid(&mgr->slot_hdr[0]);
    valid[1] = syn_fwimage_header_valid(&mgr->slot_hdr[1]);

    if (rollback) {
        fwboot_rollback_testing(mgr, valid);
    }

    /* Priority 1: TESTING slot (resume testing) */
    for (uint8_t i = 0U; i < 2U; i++) {
        if (valid[i] && mgr->slot_hdr[i].state == SYN_FW_STATE_TESTING) {
            mgr->active_slot = i;
            return i;
        }
    }

    /* Priority 2: NEW slot (first boot of update — promote to TESTING) */
    for (uint8_t i = 0U; i < 2U; i++) {
        if (valid[i] && mgr->slot_hdr[i].state == SYN_FW_STATE_NEW) {
            if (write_state(mgr->slot_addr[i], &mgr->slot_hdr[i], SYN_FW_STATE_TESTING) == SYN_OK) {
                mgr->slot_hdr[i].state = SYN_FW_STATE_TESTING;
                syn_fwimage_seal_header(&mgr->slot_hdr[i]);
                mgr->active_slot = i;
                return i;
            }
            mgr->slot_hdr[i].state = SYN_FW_STATE_INVALID;
            valid[i] = false;
        }
    }

    /* Priority 3: CONFIRMED slot with highest version */
    uint8_t best = fwboot_pick_confirmed(mgr, valid);
    mgr->active_slot = best;
    return best;
}

SYN_Status syn_fwboot_confirm(SYN_FwBootManager *mgr)
{
    SYN_ASSERT(mgr != NULL && mgr->initialized);

    if (mgr->active_slot >= 2U) {
        return SYN_ERROR;
    }

    SYN_FwImageHeader *hdr = &mgr->slot_hdr[mgr->active_slot];
    if (hdr->state != SYN_FW_STATE_TESTING) {
        return SYN_ERROR;
    }

    SYN_Status st = write_state(mgr->slot_addr[mgr->active_slot], hdr, SYN_FW_STATE_CONFIRMED);
    if (st != SYN_OK) {
        hdr->state = SYN_FW_STATE_INVALID;
        return st;
    }

    hdr->state = SYN_FW_STATE_CONFIRMED;
    syn_fwimage_seal_header(hdr);
    return SYN_OK;
}

SYN_Status syn_fwboot_refresh(SYN_FwBootManager *mgr)
{
    SYN_ASSERT(mgr != NULL && mgr->initialized);

    SYN_Status result = SYN_OK;
    for (uint8_t i = 0U; i < 2U; i++) {
        SYN_Status st = read_header(mgr->slot_addr[i], &mgr->slot_hdr[i]);
        if (st != SYN_OK) {
            memset(&mgr->slot_hdr[i], 0, sizeof(mgr->slot_hdr[i]));
            mgr->slot_hdr[i].state = SYN_FW_STATE_INVALID;
            result = st;
        }
    }
    return result;
}

#endif /* SYN_USE_BOOT */
