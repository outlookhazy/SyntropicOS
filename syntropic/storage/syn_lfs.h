/**
 * @file syn_lfs.h
 * @brief LittleFS filesystem VFS adapter.
 * @ingroup syn_storage
 */

#ifndef SYN_LFS_H
#define SYN_LFS_H

#include "syn_vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LittleFS partition configuration.
 */
typedef struct {
    uint32_t start_addr;  /**< Partition start address in flash          */
    uint32_t size;        /**< Partition size in bytes                   */
    uint32_t block_size;  /**< Erase block size in bytes                 */
} SYN_LfsConfig;

/**
 * @brief Get the VFS operations structure for LittleFS.
 * @return Pointer to the static SYN_VfsOps vtable, or NULL if LittleFS is not available.
 */
const SYN_VfsOps *syn_lfs_get_ops(void);

#if __has_include("lfs.h")
#include "lfs.h"

/**
 * @brief Initialize a littlefs config structure with callbacks pointing to syn_port_flash.
 *
 * @param cfg         Pointer to struct lfs_config to fill.
 * @param syn_cfg     Pointer to Syntropic LittleFS configuration context.
 */
void syn_lfs_init_config(struct lfs_config *cfg, const SYN_LfsConfig *syn_cfg);
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYN_LFS_H */
