#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_PARAM) || SYN_USE_PARAM

/**
 * @file syn_param.c
 * @brief Persistent parameter store with wear leveling.
 */

#include "../util/syn_crc.h"
#include "syn_param.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Align 32-bit size up to 16-byte boundary for flash write alignment.
 * @param size  Size to align.
 * @return Aligned size in bytes.
 */
static uint32_t align16_u32(uint32_t size)
{
    return (size + 15U) & ~15U;
}

/**
 * @brief Compute the flash address of a parameter slot.
 * @param store   Parameter store.
 * @param sector  Sector index.
 * @param slot    Slot index within sector.
 * @return Absolute flash address.
 */
static uint32_t slot_addr(const SYN_ParamStore *store, uint8_t sector, uint16_t slot)
{
    return store->flash_base + ((uint32_t)sector * store->sector_size) +
           ((uint32_t)slot * store->slot_size);
}

/**
 * @brief Verify CRC of a stored slot's data.
 * @param store         Parameter store.
 * @param sector        Sector index.
 * @param slot          Slot index.
 * @param expected_crc  Expected CRC value.
 * @return true if CRC matches.
 */
static bool verify_slot_crc(const SYN_ParamStore *store, uint8_t sector, uint16_t slot,
                            uint16_t expected_crc)
{
    uint32_t addr = slot_addr(store, sector, slot) + (uint32_t)sizeof(SYN_ParamSlotHeader);
    uint16_t crc = SYN_CRC16_CCITT_INIT;
    uint16_t remaining = store->data_size;
    uint8_t chunk[32];

    while (remaining > 0U) {
        uint16_t len = (remaining > (uint16_t)sizeof(chunk)) ? (uint16_t)sizeof(chunk) : remaining;
        if (syn_port_flash_read(addr, chunk, len) != SYN_OK) {
            return false;
        }
        crc = syn_crc16_ccitt_update(crc, chunk, len);
        addr += len;
        remaining = (uint16_t)(remaining - len);
    }
    return crc == expected_crc;
}

/**
 * @brief Read a slot header and optionally its payload from flash.
 * @param store   Parameter store.
 * @param sector  Sector index.
 * @param slot    Slot index.
 * @param hdr     [out] Slot header.
 * @param data    [out] Slot data buffer (or NULL to verify CRC in-place).
 * @return true on success.
 */
static bool read_slot(const SYN_ParamStore *store, uint8_t sector, uint16_t slot,
                      SYN_ParamSlotHeader *hdr, void *data)
{
    uint32_t addr = slot_addr(store, sector, slot);

    if (syn_port_flash_read(addr, hdr, sizeof(*hdr)) != SYN_OK) {
        return false;
    }

    if ((hdr->magic != SYN_PARAM_MAGIC) || (hdr->data_size != store->data_size)) {
        return false;
    }

    if (data != NULL) {
        if (syn_port_flash_read(addr + (uint32_t)sizeof(*hdr), data, store->data_size) != SYN_OK) {
            return false;
        }
        return syn_crc16_ccitt(data, store->data_size) == hdr->crc;
    }

    return verify_slot_crc(store, sector, slot, hdr->crc);
}

/**
 * @brief Scan all sectors and slots for the highest valid sequence number.
 * @param store    Parameter store instance.
 * @param best_out [out] Winning sequence number when found.
 * @return true if at least one valid slot was found.
 */
static bool scan_best_slot(SYN_ParamStore *store, uint16_t *best_out)
{
    uint16_t best_seq = 0U;
    bool found = false;
    SYN_ParamSlotHeader hdr;

    for (uint8_t sec = 0U; sec < store->sector_count; sec++) {
        for (uint16_t sl = 0U; sl < store->slots_per_sector; sl++) {
            if (read_slot(store, sec, sl, &hdr, NULL)) {
                if (!found || ((int16_t)(hdr.seq - best_seq) > 0)) {
                    best_seq = hdr.seq;
                    store->active_sector = sec;
                    store->active_slot = sl;
                    found = true;
                }
            }
        }
    }

    *best_out = best_seq;
    return found;
}

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_param_init(SYN_ParamStore *store, uint32_t flash_base, uint8_t sector_count,
                          uint16_t data_size)
{
    if ((store == NULL) || (sector_count == 0U) || (data_size == 0U)) {
        return SYN_INVALID_PARAM;
    }

    memset(store, 0, sizeof(*store));
    store->flash_base = flash_base;
    store->sector_count = sector_count;
    store->data_size = data_size;
    store->sector_size = syn_port_flash_sector_size(flash_base);

    uint32_t raw_slot = (uint32_t)sizeof(SYN_ParamSlotHeader) + (uint32_t)data_size;
    uint32_t aligned_slot = align16_u32(raw_slot);
    if ((aligned_slot > store->sector_size) || (aligned_slot > 0xFFFFU)) {
        return SYN_INVALID_PARAM;
    }

    store->slot_size = (uint16_t)aligned_slot;
    store->slots_per_sector = (uint16_t)(store->sector_size / aligned_slot);

    uint16_t best_seq = 0U;
    if (scan_best_slot(store, &best_seq)) {
        store->next_seq = (uint16_t)(best_seq + 1U);
        store->has_active_slot = true;
        store->initialized = true;
        return SYN_OK;
    }

    store->active_sector = 0U;
    store->active_slot = 0U;
    store->next_seq = 1U;
    store->has_active_slot = false;
    store->initialized = true;
    return SYN_ERROR;
}

SYN_Status syn_param_load(const SYN_ParamStore *store, void *data)
{
    if ((store == NULL) || !store->initialized || (data == NULL)) {
        return SYN_INVALID_PARAM;
    }

    SYN_ParamSlotHeader hdr;
    if (read_slot(store, store->active_sector, store->active_slot, &hdr, data)) {
        return SYN_OK;
    }

    return SYN_ERROR;
}

SYN_Status syn_param_save(SYN_ParamStore *store, const void *data)
{
    if ((store == NULL) || !store->initialized || (data == NULL)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t sec = store->active_sector;
    uint16_t sl = store->active_slot;

    if (store->has_active_slot) {
        sl++;
        if (sl >= store->slots_per_sector) {
            sl = 0U;
            sec = (uint8_t)((sec + 1U) % store->sector_count);

            SYN_Status err =
                syn_port_flash_erase(store->flash_base + ((uint32_t)sec * store->sector_size));
            if (err != SYN_OK) {
                return err;
            }
        }
    } else {
        SYN_Status err =
            syn_port_flash_erase(store->flash_base + ((uint32_t)sec * store->sector_size));
        if (err != SYN_OK) {
            return err;
        }
    }

    SYN_ParamSlotHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = SYN_PARAM_MAGIC;
    hdr.seq = store->next_seq;
    hdr.data_size = store->data_size;
    hdr.crc = syn_crc16_ccitt(data, store->data_size);

    uint32_t addr = slot_addr(store, sec, sl);
    SYN_Status err = syn_port_flash_write(addr + (uint32_t)sizeof(hdr), data, store->data_size);
    if (err != SYN_OK) {
        return err;
    }

    err = syn_port_flash_write(addr, &hdr, sizeof(hdr));
    if (err != SYN_OK) {
        return err;
    }

    store->active_sector = sec;
    store->active_slot = sl;
    store->next_seq++;
    store->has_active_slot = true;

    return SYN_OK;
}

SYN_Status syn_param_erase_all(SYN_ParamStore *store)
{
    if ((store == NULL) || !store->initialized) {
        return SYN_INVALID_PARAM;
    }

    for (uint8_t sec = 0U; sec < store->sector_count; sec++) {
        SYN_Status err =
            syn_port_flash_erase(store->flash_base + ((uint32_t)sec * store->sector_size));
        if (err != SYN_OK) {
            return err;
        }
    }

    store->active_sector = 0U;
    store->active_slot = 0U;
    store->next_seq = 1U;
    store->has_active_slot = false;

    return SYN_OK;
}

#endif /* SYN_USE_PARAM */
