/**
 * @file syn_port_flash.h
 * @brief Flash storage port interface — implement these for your platform.
 *
 * Used by the parameter store module for wear-leveled persistent storage.
 * The port exposes raw sector-level operations; SyntropicOS handles wear leveling.
 * @ingroup syn_system
 */

#ifndef SYN_PORT_FLASH_H
#define SYN_PORT_FLASH_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Erase a flash sector.
 *
 * @param addr  Start address of the sector (must be sector-aligned).
 * @return SYN_OK on success.
 */
SYN_Status syn_port_flash_erase(uint32_t addr);

/**
 * @brief Read from flash.
 *
 * @param addr  Source address in flash.
 * @param buf   Destination buffer.
 * @param len   Number of bytes to read.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_flash_read(uint32_t addr, void *buf, size_t len);

/**
 * @brief Write to flash.
 *
 * Flash must be erased before writing (writes can only clear bits).
 * The implementation should handle any alignment requirements.
 *
 * @param addr  Destination address in flash.
 * @param buf   Source data.
 * @param len   Number of bytes to write.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_flash_write(uint32_t addr, const void *buf, size_t len);

/**
 * @brief Get the sector size for the given address.
 *
 * @param addr  Address within the sector.
 * @return Sector size in bytes.
 */
uint32_t syn_port_flash_sector_size(uint32_t addr);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_FLASH_H */
