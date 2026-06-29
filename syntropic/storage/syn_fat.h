/**
 * @file syn_fat.h
 * @brief Custom lightweight FAT16/FAT32 VFS adapter.
 * @ingroup syn_storage
 */

#ifndef SYN_FAT_H
#define SYN_FAT_H

#include "syn_vfs.h"
#include "../drivers/syn_sd.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the VFS operations structure for the custom FAT filesystem.
 * @return Pointer to the static SYN_VfsOps vtable.
 */
const SYN_VfsOps *syn_fat_get_ops(void);

/**
 * @brief Initialize the SD card and mount the FAT filesystem in VFS at /sd.
 * 
 * @param spi_bus  SPI bus index.
 * @param cs       GPIO CS pin index.
 * @return SYN_OK on success, SYN_ERROR on failure.
 */
SYN_Status syn_fat_init(uint8_t spi_bus, SYN_GPIO_Pin cs);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FAT_H */
