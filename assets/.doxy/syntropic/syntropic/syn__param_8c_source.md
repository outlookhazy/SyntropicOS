

# File syn\_param.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_param.c**](syn__param_8c.md)

[Go to the documentation of this file](syn__param_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_PARAM) || SYN_USE_PARAM


#include "../util/syn_crc.h"
#include "syn_param.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

static uint32_t align16_u32(uint32_t size)
{
    return (size + 15U) & ~15U;
}

static uint32_t slot_addr(const SYN_ParamStore *store, uint8_t sector, uint16_t slot)
{
    return store->flash_base + ((uint32_t)sector * store->sector_size) +
           ((uint32_t)slot * store->slot_size);
}

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

static SYN_Status is_slot_erased(const SYN_ParamStore *store, uint8_t sector, uint16_t slot,
                                 bool *erased)
{
    uint32_t addr = slot_addr(store, sector, slot);
    uint16_t remaining = store->slot_size;
    uint8_t chunk[32];

    *erased = true;
    while (remaining > 0U) {
        uint16_t len = (remaining > (uint16_t)sizeof(chunk)) ? (uint16_t)sizeof(chunk) : remaining;
        if (syn_port_flash_read(addr, chunk, len) != SYN_OK) {
            return SYN_ERROR;
        }
        for (uint16_t i = 0U; i < len; i++) {
            if (chunk[i] != 0xFFU) {
                *erased = false;
                return SYN_OK;
            }
        }
        addr += len;
        remaining = (uint16_t)(remaining - len);
    }
    return SYN_OK;
}

static SYN_Status prepare_next_slot(const SYN_ParamStore *store, uint8_t *out_sec,
                                    uint16_t *out_slot)
{
    if (!store->has_active_slot) {
        *out_sec = 0U;
        *out_slot = 0U;
        return syn_port_flash_erase(store->flash_base);
    }

    uint8_t sec = store->active_sector;
    for (uint32_t sl = (uint32_t)store->active_slot + 1UL; sl < (uint32_t)store->slots_per_sector;
         sl++) {
        bool erased = false;
        SYN_Status st = is_slot_erased(store, sec, (uint16_t)sl, &erased);
        if (st != SYN_OK) {
            return st;
        }
        if (erased) {
            *out_sec = sec;
            *out_slot = (uint16_t)sl;
            return SYN_OK;
        }
    }

    sec = (uint8_t)((sec + 1U) % store->sector_count);
    *out_sec = sec;
    *out_slot = 0U;
    return syn_port_flash_erase(store->flash_base + ((uint32_t)sec * store->sector_size));
}

static bool scan_best_slot(SYN_ParamStore *store, uint16_t *best_out)
{
    uint16_t best_seq = 0U;
    bool found = false;
    SYN_ParamSlotHeader hdr;
    uint32_t total_slots = (uint32_t)store->sector_count * (uint32_t)store->slots_per_sector;

    for (uint32_t idx = 0U; idx < total_slots; idx++) {
        uint8_t sec = (uint8_t)(idx / (uint32_t)store->slots_per_sector);
        uint16_t sl = (uint16_t)(idx % (uint32_t)store->slots_per_sector);
        if (read_slot(store, sec, sl, &hdr, NULL)) {
            if (!found || ((int16_t)(hdr.seq - best_seq) > 0)) {
                best_seq = hdr.seq;
                store->active_sector = sec;
                store->active_slot = sl;
                found = true;
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

    uint32_t max_slots = store->sector_size / aligned_slot;
    store->slot_size = (uint16_t)aligned_slot;
    store->slots_per_sector = (max_slots > 0xFFFFUL) ? 0xFFFFU : (uint16_t)max_slots;

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

    uint8_t sec = 0U;
    uint16_t sl = 0U;
    SYN_Status err = prepare_next_slot(store, &sec, &sl);
    if (err != SYN_OK) {
        return err;
    }

    SYN_ParamSlotHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = SYN_PARAM_MAGIC;
    hdr.seq = store->next_seq;
    hdr.data_size = store->data_size;
    hdr.crc = syn_crc16_ccitt(data, store->data_size);

    uint32_t addr = slot_addr(store, sec, sl);
    err = syn_port_flash_write(addr + (uint32_t)sizeof(hdr), data, store->data_size);
    if (err == SYN_OK) {
        err = syn_port_flash_write(addr, &hdr, sizeof(hdr));
    }
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
```


