/**
 * @file syn_vfs.h
 * @brief Virtual File System (VFS) abstraction layer.
 * @ingroup syn_storage
 */

#ifndef SYN_VFS_H
#define SYN_VFS_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup vfs_limits VFS Static Limits
 *  Override in syn_config.h if needed.
 *  @{ */
#ifndef SYN_VFS_MAX_OPEN_FILES
  #define SYN_VFS_MAX_OPEN_FILES  4  /**< Max simultaneously open files  */
#endif

#ifndef SYN_VFS_MAX_OPEN_DIRS
  #define SYN_VFS_MAX_OPEN_DIRS   2  /**< Max simultaneously open dirs   */
#endif

#ifndef SYN_VFS_MAX_MOUNTS
  #define SYN_VFS_MAX_MOUNTS      2  /**< Max mounted filesystems        */
#endif
/** @} */

/** @defgroup vfs_flags File Opening Flags
 *  @{ */
#define SYN_O_RDONLY  0x00  /**< Open for reading only               */
#define SYN_O_WRONLY  0x01  /**< Open for writing only               */
#define SYN_O_RDWR    0x02  /**< Open for reading and writing        */
#define SYN_O_CREAT   0x04  /**< Create file if it does not exist    */
#define SYN_O_APPEND  0x08  /**< Append writes to end of file        */
#define SYN_O_TRUNC   0x10  /**< Truncate file to zero length        */
/** @} */

/** @defgroup vfs_seek Seek Origins
 *  @{ */
#define SYN_SEEK_SET  0  /**< Seek relative to start of file        */
#define SYN_SEEK_CUR  1  /**< Seek relative to current position     */
#define SYN_SEEK_END  2  /**< Seek relative to end of file          */
/** @} */

/**
 * @brief Directory entry returned by syn_vfs_readdir().
 */
typedef struct {
    char     name[64];  /**< Entry name (null-terminated)        */
    uint32_t size;      /**< File size in bytes (0 for dirs)     */
    bool     is_dir;    /**< true if the entry is a directory    */
} SYN_VfsDirEnt;

typedef struct SYN_VfsFile SYN_VfsFile;  /**< Forward declaration of file descriptor   */
typedef struct SYN_VfsDir SYN_VfsDir;    /**< Forward declaration of directory descriptor */

/**
 * @brief VFS operations structure. Implement these for your filesystem (e.g. LittleFS).
 */
typedef struct {
    int     (*open)(SYN_VfsFile *file, const char *path, int flags, void *fs_data);  /**< Open a file          */
    int     (*close)(SYN_VfsFile *file);                                              /**< Close a file         */
    int     (*read)(SYN_VfsFile *file, void *buf, size_t len);                        /**< Read from a file     */
    int     (*write)(SYN_VfsFile *file, const void *buf, size_t len);                 /**< Write to a file      */
    int32_t (*seek)(SYN_VfsFile *file, int32_t offset, int whence);                   /**< Seek in a file       */
    int32_t (*tell)(SYN_VfsFile *file);                                               /**< Get file position    */
    int     (*unlink)(const char *path, void *fs_data);                               /**< Delete a file        */
    int     (*mkdir)(const char *path, void *fs_data);                                /**< Create a directory   */
    int     (*opendir)(SYN_VfsDir *dir, const char *path, void *fs_data);             /**< Open a directory     */
    int     (*readdir)(SYN_VfsDir *dir, SYN_VfsDirEnt *ent);                          /**< Read directory entry */
    int     (*closedir)(SYN_VfsDir *dir);                                             /**< Close a directory    */
} SYN_VfsOps;

/**
 * @brief Open file descriptor.
 */
struct SYN_VfsFile {
    const SYN_VfsOps *ops;      /**< Filesystem operations vtable                  */
    void             *fs_file;  /**< Filesystem-specific file context pointer       */
    bool              is_open;  /**< true while the descriptor is in use            */
};

/**
 * @brief Open directory descriptor.
 */
struct SYN_VfsDir {
    const SYN_VfsOps *ops;      /**< Filesystem operations vtable                  */
    void             *fs_dir;   /**< Filesystem-specific directory context pointer  */
    bool              is_open;  /**< true while the descriptor is in use            */
};

/**
 * @brief Mount point binding a path prefix to a filesystem.
 */
typedef struct {
    const char       *prefix;      /**< Mount path prefix (e.g. "/flash")             */
    size_t            prefix_len;  /**< Cached strlen(prefix) for fast matching        */
    const SYN_VfsOps *ops;         /**< Filesystem operations for this mount           */
    void             *fs_data;     /**< Filesystem instance pointer (e.g. lfs_t *)     */
} SYN_VfsMount;

/* ── API ────────────────────────────────────────────────────────────────── */

/** @brief Initialize the VFS registry. Clears all mounts and descriptors. */
void syn_vfs_init(void);

/**
 * @brief Mount a filesystem at a path prefix.
 * @param prefix  Mount path prefix (e.g. "/flash").
 * @param ops     Filesystem operations vtable.
 * @param fs_data Opaque filesystem context (e.g. lfs_t *).
 * @return SYN_OK on success, SYN_ERROR if table is full or prefix exists.
 */
SYN_Status syn_vfs_mount(const char *prefix, const SYN_VfsOps *ops, void *fs_data);

/**
 * @brief Open a file.
 * @param path  Absolute path including mount prefix.
 * @param flags Combination of SYN_O_* flags.
 * @return File descriptor (>= 0) on success, or negative error code.
 */
int syn_vfs_open(const char *path, int flags);

/**
 * @brief Close an open file descriptor.
 * @param fd File descriptor returned by syn_vfs_open().
 * @return 0 on success, or negative error code.
 */
int syn_vfs_close(int fd);

/**
 * @brief Read from an open file.
 * @param fd  File descriptor.
 * @param buf Destination buffer.
 * @param len Maximum bytes to read.
 * @return Bytes read on success, or negative error code.
 */
int syn_vfs_read(int fd, void *buf, size_t len);

/**
 * @brief Write to an open file.
 * @param fd  File descriptor.
 * @param buf Source buffer.
 * @param len Bytes to write.
 * @return Bytes written on success, or negative error code.
 */
int syn_vfs_write(int fd, const void *buf, size_t len);

/**
 * @brief Seek in an open file.
 * @param fd     File descriptor.
 * @param offset Byte offset.
 * @param whence SYN_SEEK_SET, SYN_SEEK_CUR, or SYN_SEEK_END.
 * @return New absolute offset on success, or negative error code.
 */
int32_t syn_vfs_seek(int fd, int32_t offset, int whence);

/**
 * @brief Get current position in an open file.
 * @param fd File descriptor.
 * @return Current offset, or negative error code.
 */
int32_t syn_vfs_tell(int fd);

/**
 * @brief Delete a file.
 * @param path Absolute path including mount prefix.
 * @return 0 on success, or negative error code.
 */
int syn_vfs_unlink(const char *path);

/**
 * @brief Create a directory.
 * @param path Absolute path including mount prefix.
 * @return 0 on success, or negative error code.
 */
int syn_vfs_mkdir(const char *path);

/**
 * @brief Open a directory for iteration.
 * @param path Absolute path including mount prefix.
 * @return Directory descriptor (>= 0) on success, or negative error code.
 */
int syn_vfs_opendir(const char *path);

/**
 * @brief Read the next directory entry.
 * @param dd  Directory descriptor returned by syn_vfs_opendir().
 * @param ent [out] Filled with the next entry.
 * @return 1 if an entry was read, 0 at end-of-directory, or negative error.
 */
int syn_vfs_readdir(int dd, SYN_VfsDirEnt *ent);

/**
 * @brief Close an open directory descriptor.
 * @param dd Directory descriptor.
 * @return 0 on success, or negative error code.
 */
int syn_vfs_closedir(int dd);

#ifdef __cplusplus
}
#endif

#endif /* SYN_VFS_H */
