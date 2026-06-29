/**
 * @file syn_param.h
 * @brief Persistent parameter store with wear leveling.
 *
 * Stores key-value parameters in flash with:
 *   - **Wear leveling**: Slot rotation across flash sectors
 *   - **Integrity**: CRC-16 on each slot
 *   - **Atomic writes**: Sequence numbers to identify the latest valid slot
 *   - **No dynamic allocation**: Fixed-size parameter block, caller-owned RAM
 *
 * @par Architecture
 *
 * The flash region is divided into sectors. Each sector contains N slots.
 * Each slot stores the full parameter block + header (sequence, CRC).
 * Writes go to the next free slot. When a sector is full, the next sector
 * is erased and writing continues there.
 *
 * ```
 * Sector 0:  [slot0][slot1][slot2]...[slotN]
 * Sector 1:  [slot0][slot1]...
 * Sector 2:  ...
 * ```
 *
 * The highest sequence number with a valid CRC is the active slot.
 *
 * @par Usage
 * @code
 *   // Define your parameters as a struct
 *   typedef struct {
 *       uint16_t brightness;
 *       int16_t  offset;
 *       uint8_t  mode;
 *   } MyParams;
 *
 *   static SYN_ParamStore store;
 *   static MyParams params;
 *
 *   syn_param_init(&store, FLASH_PARAM_START, 2, sizeof(MyParams));
 *   syn_param_load(&store, &params);   // loads latest valid or defaults
 *
 *   params.brightness = 80;
 *   syn_param_save(&store, &params);   // writes to next slot
 * @endcode
 * @ingroup syn_storage
 */

#ifndef SYN_PARAM_H
#define SYN_PARAM_H

#include "../common/syn_defs.h"
#include "../port/syn_port_flash.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Slot header ────────────────────────────────────────────────────────── */

/** @brief Magic number for parameter slot headers. */
#define SYN_PARAM_MAGIC  0xC0DEu

/** @brief Slot header — stored at the beginning of each parameter slot. */
typedef struct {
    uint16_t  magic;       /**< Magic number (SYN_PARAM_MAGIC)          */
    uint16_t  seq;         /**< Sequence number (higher = newer)          */
    uint16_t  data_size;   /**< Size of the parameter data                */
    uint16_t  crc;         /**< CRC-16 over the data                      */
    uint8_t   _pad[8];     /**< Padding to 16 bytes                       */
} SYN_ParamSlotHeader;

/* ── Parameter store ────────────────────────────────────────────────────── */

/** @brief Wear-leveled flash parameter store instance. */
typedef struct {
    uint32_t  flash_base;    /**< Start address of the flash region       */
    uint32_t  sector_size;   /**< Size of one flash sector (bytes)        */
    uint8_t   sector_count;  /**< Number of sectors allocated             */
    uint16_t  data_size;     /**< Size of the parameter data block        */
    uint16_t  slot_size;     /**< sizeof(header) + data_size, aligned     */
    uint16_t  slots_per_sector; /**< Slots that fit in one sector         */

    /* Current state */
    uint8_t   active_sector; /**< Sector containing the active slot       */
    uint16_t  active_slot;   /**< Slot index within the active sector     */
    uint16_t  next_seq;      /**< Next sequence number to use             */
    bool      initialized;   /**< Init complete                           */
} SYN_ParamStore;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the parameter store.
 *
 * Scans flash to find the latest valid slot. Must be called before
 * load or save.
 *
 * @param store         Store instance.
 * @param flash_base    Start address of the flash region for params.
 * @param sector_count  Number of flash sectors to use for wear leveling.
 * @param data_size     Size of the parameter struct in bytes.
 * @return SYN_OK if at least one valid slot was found,
 *         SYN_ERROR if flash is blank (use defaults).
 */
SYN_Status syn_param_init(SYN_ParamStore *store,
                            uint32_t flash_base,
                            uint8_t sector_count,
                            uint16_t data_size);

/**
 * @brief Load parameters from the latest valid slot.
 *
 * @param store  Initialized store.
 * @param data   Buffer to receive the parameter data (must be data_size bytes).
 * @return SYN_OK on success, SYN_ERROR if no valid data found.
 */
SYN_Status syn_param_load(const SYN_ParamStore *store, void *data);

/**
 * @brief Save parameters to the next slot (wear-leveled).
 *
 * Writes to the next free slot, rotating across sectors. When a sector
 * fills up, the next sector is erased and writing continues there.
 *
 * @param store  Initialized store.
 * @param data   Parameter data to save (must be data_size bytes).
 * @return SYN_OK on success, SYN_ERROR on flash write failure.
 */
SYN_Status syn_param_save(SYN_ParamStore *store, const void *data);

/**
 * @brief Erase all parameter data (factory reset).
 * @param store  Store instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_param_erase_all(SYN_ParamStore *store);

/**
 * @brief Get the current write count (approximate wear indicator).
 *
 * @param store  Store instance.
 * @return The sequence number, which increments with each save.
 */
static inline uint16_t syn_param_write_count(const SYN_ParamStore *store)
{
    return (uint16_t)(store->next_seq > 0 ? store->next_seq - 1 : 0);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_PARAM_H */
