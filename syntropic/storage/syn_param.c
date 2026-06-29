#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_PARAM) || SYN_USE_PARAM

/**
 * @file syn_param.c
 * @brief Persistent parameter store with wear leveling.
 */

#include "syn_param.h"
#include "../util/syn_assert.h"
#include "../util/syn_crc.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Align size up to 16-byte boundary for flash write alignment.
 * @param size  Size to align.
 * @return Aligned size.
 */
static uint16_t align16(uint16_t size)
{
    return (uint16_t)((size + 15u) & ~15u);
}

/**
 * @brief Compute the flash address of a parameter slot.
 * @param store   Parameter store.
 * @param sector  Sector index.
 * @param slot    Slot index within sector.
 * @return Absolute flash address.
 */
static uint32_t slot_addr(const SYN_ParamStore *store,
                          uint8_t sector, uint16_t slot)
{
    return store->flash_base +
           (uint32_t)sector * store->sector_size +
           (uint32_t)slot * store->slot_size;
}

/**
 * @brief Verify CRC of a stored slot's data.
 * @param store         Parameter store.
 * @param sector        Sector index.
 * @param slot          Slot index.
 * @param expected_crc  Expected CRC value.
 * @return true if CRC matches.
 */
static bool verify_slot_crc(const SYN_ParamStore *store, uint8_t sector, uint16_t slot, uint16_t expected_crc)
{
    uint32_t addr = slot_addr(store, sector, slot) + sizeof(SYN_ParamSlotHeader);
    uint16_t crc = SYN_CRC16_CCITT_INIT;
    uint16_t remaining = store->data_size;
    uint8_t chunk[32];
    
    while (remaining > 0) {
        uint16_t len = remaining > sizeof(chunk) ? sizeof(chunk) : remaining;
        if (syn_port_flash_read(addr, chunk, len) != SYN_OK) return false;
        crc = syn_crc16_ccitt_update(crc, chunk, len);
        addr += len;
        remaining -= len;
    }
    return crc == expected_crc;
}

/**
 * @brief Read a slot header and data from flash.
 * @param store   Parameter store.
 * @param sector  Sector index.
 * @param slot    Slot index.
 * @param hdr     [out] Slot header.
 * @param data    [out] Slot data.
 * @return true on success.
 */
static bool read_slot(const SYN_ParamStore *store,
                      uint8_t sector, uint16_t slot,
                      SYN_ParamSlotHeader *hdr, void *data)
{
    uint32_t addr = slot_addr(store, sector, slot);

    if (syn_port_flash_read(addr, hdr, sizeof(*hdr)) != SYN_OK) {
        return false;
    }

    if (hdr->magic != SYN_PARAM_MAGIC) return false;
    if (hdr->data_size != store->data_size) return false;

    /* Read data */
    if (data != NULL) {
        if (syn_port_flash_read(addr + sizeof(*hdr), data,
                                 store->data_size) != SYN_OK) {
            return false;
        }
    }

    /* Verify CRC */
    if (data != NULL) {
        uint16_t crc = syn_crc16_ccitt(data, store->data_size);
        if (crc != hdr->crc) return false;
    } else {
        if (!verify_slot_crc(store, sector, slot, hdr->crc)) return false;
    }

    return true;
}

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_param_init(SYN_ParamStore *store,
                            uint32_t flash_base,
                            uint8_t sector_count,
                            uint16_t data_size)
{
    SYN_ASSERT(store != NULL);
    SYN_ASSERT(sector_count > 0);
    SYN_ASSERT(data_size > 0);

    memset(store, 0, sizeof(*store));
    store->flash_base   = flash_base;
    store->sector_count = sector_count;
    store->data_size    = data_size;
    store->sector_size  = syn_port_flash_sector_size(flash_base);
    store->slot_size    = align16((uint16_t)(sizeof(SYN_ParamSlotHeader) +
                                            data_size));
    store->slots_per_sector = (uint16_t)(store->sector_size / store->slot_size);

    if (store->slots_per_sector == 0) {
        return SYN_ERROR; /* data too large for sector */
    }

    /* Scan all sectors and slots for the highest valid sequence number */
    uint16_t best_seq = 0;
    bool found = false;

    SYN_ParamSlotHeader hdr;

    for (uint8_t sec = 0; sec < sector_count; sec++) {
        for (uint16_t sl = 0; sl < store->slots_per_sector; sl++) {
            if (read_slot(store, sec, sl, &hdr, NULL)) {
                /* Can't verify CRC without reading data, so re-read
                 * with data for the best candidate only. For the scan,
                 * we just check magic + data_size. */
                if (!found || ((int16_t)(hdr.seq - best_seq) > 0)) {
                    best_seq = hdr.seq;
                    store->active_sector = sec;
                    store->active_slot   = sl;
                    found = true;
                }
            }
        }
    }

    if (found) {
        store->next_seq    = (uint16_t)(best_seq + 1);
        store->initialized = true;
        return SYN_OK;
    }

    /* No valid data found — flash is blank */
    store->active_sector = 0;
    store->active_slot   = 0;
    store->next_seq      = 1;
    store->initialized   = true;
    return SYN_ERROR;
}

SYN_Status syn_param_load(const SYN_ParamStore *store, void *data)
{
    SYN_ASSERT(store != NULL);
    SYN_ASSERT(store->initialized);
    SYN_ASSERT(data != NULL);

    SYN_ParamSlotHeader hdr;
    if (read_slot(store, store->active_sector, store->active_slot,
                  &hdr, data)) {
        /* Verify CRC with actual data */
        uint16_t crc = syn_crc16_ccitt(data, store->data_size);
        if (crc == hdr.crc) {
            return SYN_OK;
        }
    }

    return SYN_ERROR;
}

SYN_Status syn_param_save(SYN_ParamStore *store, const void *data)
{
    SYN_ASSERT(store != NULL);
    SYN_ASSERT(store->initialized);
    SYN_ASSERT(data != NULL);

    /* Determine next write position */
    uint8_t  sec = store->active_sector;
    uint16_t sl  = store->active_slot;

    /* If this isn't the first write, advance to next slot */
    if (store->next_seq > 1) {
        sl++;
        if (sl >= store->slots_per_sector) {
            /* Move to next sector */
            sl = 0;
            sec = (uint8_t)((sec + 1) % store->sector_count);

            /* Erase the new sector */
            SYN_Status err = syn_port_flash_erase(
                store->flash_base + (uint32_t)sec * store->sector_size);
            if (err != SYN_OK) return err;
        }
    } else {
        /* First write ever — erase the initial sector */
        SYN_Status err = syn_port_flash_erase(
            store->flash_base + (uint32_t)sec * store->sector_size);
        if (err != SYN_OK) return err;
    }

    /* Build header */
    SYN_ParamSlotHeader hdr;
    hdr.magic     = SYN_PARAM_MAGIC;
    hdr.seq       = store->next_seq;
    hdr.data_size = store->data_size;
    hdr.crc       = syn_crc16_ccitt(data, store->data_size);

    /* Write header */
    uint32_t addr = slot_addr(store, sec, sl);
    SYN_Status err = syn_port_flash_write(addr, &hdr, sizeof(hdr));
    if (err != SYN_OK) return err;

    /* Write data */
    err = syn_port_flash_write(addr + sizeof(hdr), data, store->data_size);
    if (err != SYN_OK) return err;

    /* Update state */
    store->active_sector = sec;
    store->active_slot   = sl;
    store->next_seq++;

    return SYN_OK;
}

SYN_Status syn_param_erase_all(SYN_ParamStore *store)
{
    SYN_ASSERT(store != NULL);
    SYN_ASSERT(store->initialized);

    for (uint8_t sec = 0; sec < store->sector_count; sec++) {
        SYN_Status err = syn_port_flash_erase(
            store->flash_base + (uint32_t)sec * store->sector_size);
        if (err != SYN_OK) return err;
    }

    store->active_sector = 0;
    store->active_slot   = 0;
    store->next_seq      = 1;

    return SYN_OK;
}

#endif /* SYN_USE_PARAM */
