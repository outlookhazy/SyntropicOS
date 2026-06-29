#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_BOOT) || SYN_USE_BOOT

/**
 * @file syn_fwboot.c
 * @brief A/B firmware boot manager implementation.
 */

#include "syn_fwboot.h"
#include "syn_fwimage.h"
#include "../util/syn_assert.h"
#include "../port/syn_port_flash.h"

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
static SYN_Status write_state(uint32_t addr, const SYN_FwImageHeader *hdr,
                               uint8_t new_state)
{
    SYN_FwImageHeader updated = *hdr;
    updated.state = new_state;
    syn_fwimage_seal_header(&updated);

    /* Erase sector containing the header and rewrite */
    SYN_Status st = syn_port_flash_erase(addr);
    if (st != SYN_OK) return st;

    return syn_port_flash_write(addr, &updated, sizeof(updated));
}

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_fwboot_init(SYN_FwBootManager *mgr,
                            uint32_t slot_a, uint32_t slot_b)
{
    SYN_ASSERT(mgr != NULL);

    memset(mgr, 0, sizeof(*mgr));
    mgr->slot_addr[0] = slot_a;
    mgr->slot_addr[1] = slot_b;
    mgr->active_slot  = SYN_FW_SLOT_NONE;

    /* Read headers from flash */
    read_header(slot_a, &mgr->slot_hdr[0]);
    read_header(slot_b, &mgr->slot_hdr[1]);

    mgr->initialized = true;
    return SYN_OK;
}

uint8_t syn_fwboot_select(SYN_FwBootManager *mgr, bool rollback)
{
    SYN_ASSERT(mgr != NULL && mgr->initialized);

    bool valid[2];
    valid[0] = syn_fwimage_header_valid(&mgr->slot_hdr[0]);
    valid[1] = syn_fwimage_header_valid(&mgr->slot_hdr[1]);

    /* Handle rollback: if a slot is TESTING, mark it INVALID */
    if (rollback) {
        for (int i = 0; i < 2; i++) {
            if (valid[i] && mgr->slot_hdr[i].state == SYN_FW_STATE_TESTING) {
                write_state(mgr->slot_addr[i], &mgr->slot_hdr[i],
                           SYN_FW_STATE_INVALID);
                mgr->slot_hdr[i].state = SYN_FW_STATE_INVALID;
                syn_fwimage_seal_header(&mgr->slot_hdr[i]);
                valid[i] = false; /* No longer bootable */
            }
        }
    }

    /* Priority 1: TESTING slot (resume testing) */
    for (int i = 0; i < 2; i++) {
        if (valid[i] && mgr->slot_hdr[i].state == SYN_FW_STATE_TESTING) {
            mgr->active_slot = (uint8_t)i;
            return (uint8_t)i;
        }
    }

    /* Priority 2: NEW slot (first boot of update — promote to TESTING) */
    for (int i = 0; i < 2; i++) {
        if (valid[i] && mgr->slot_hdr[i].state == SYN_FW_STATE_NEW) {
            write_state(mgr->slot_addr[i], &mgr->slot_hdr[i],
                       SYN_FW_STATE_TESTING);
            mgr->slot_hdr[i].state = SYN_FW_STATE_TESTING;
            syn_fwimage_seal_header(&mgr->slot_hdr[i]);
            mgr->active_slot = (uint8_t)i;
            return (uint8_t)i;
        }
    }

    /* Priority 3: CONFIRMED slot with highest version */
    uint8_t best = SYN_FW_SLOT_NONE;
    uint32_t best_ver = 0;
    for (int i = 0; i < 2; i++) {
        if (valid[i] && mgr->slot_hdr[i].state == SYN_FW_STATE_CONFIRMED) {
            if (best == SYN_FW_SLOT_NONE ||
                mgr->slot_hdr[i].version_code > best_ver) {
                best = (uint8_t)i;
                best_ver = mgr->slot_hdr[i].version_code;
            }
        }
    }

    mgr->active_slot = best;
    return best;
}

SYN_Status syn_fwboot_confirm(SYN_FwBootManager *mgr)
{
    SYN_ASSERT(mgr != NULL && mgr->initialized);

    if (mgr->active_slot >= 2) return SYN_ERROR;

    SYN_FwImageHeader *hdr = &mgr->slot_hdr[mgr->active_slot];
    if (hdr->state != SYN_FW_STATE_TESTING) return SYN_ERROR;

    SYN_Status st = write_state(mgr->slot_addr[mgr->active_slot],
                                 hdr, SYN_FW_STATE_CONFIRMED);
    if (st == SYN_OK) {
        hdr->state = SYN_FW_STATE_CONFIRMED;
        syn_fwimage_seal_header(hdr);
    }
    return st;
}

SYN_Status syn_fwboot_refresh(SYN_FwBootManager *mgr)
{
    SYN_ASSERT(mgr != NULL && mgr->initialized);

    read_header(mgr->slot_addr[0], &mgr->slot_hdr[0]);
    read_header(mgr->slot_addr[1], &mgr->slot_hdr[1]);
    return SYN_OK;
}

#endif /* SYN_USE_BOOT */
