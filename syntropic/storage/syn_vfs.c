#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_VFS) || SYN_USE_VFS

/**
 * @file syn_vfs.c
 * @brief Virtual File System (VFS) abstraction implementation.
 */

#include "syn_vfs.h"
#include "../util/syn_assert.h"
#include <string.h>

static SYN_VfsMount g_mounts[SYN_VFS_MAX_MOUNTS];  /**< Mount table.           */
static size_t       g_mount_count = 0;               /**< Number of active mounts. */

static SYN_VfsFile  g_files[SYN_VFS_MAX_OPEN_FILES]; /**< Open file handles.     */
static SYN_VfsDir   g_dirs[SYN_VFS_MAX_OPEN_DIRS];   /**< Open directory handles. */

void syn_vfs_init(void)
{
    g_mount_count = 0;
    memset(g_mounts, 0, sizeof(g_mounts));
    memset(g_files, 0, sizeof(g_files));
    memset(g_dirs, 0, sizeof(g_dirs));
}

SYN_Status syn_vfs_mount(const char *prefix, const SYN_VfsOps *ops, void *fs_data)
{
    SYN_ASSERT(prefix != NULL);
    SYN_ASSERT(ops != NULL);

    if (g_mount_count >= SYN_VFS_MAX_MOUNTS) {
        return SYN_ERROR;
    }

    /* Verify if prefix already exists to prevent duplicate mounts */
    for (size_t i = 0; i < g_mount_count; i++) {
        if (strcmp(g_mounts[i].prefix, prefix) == 0) {
            return SYN_ERROR;
        }
    }

    g_mounts[g_mount_count].prefix     = prefix;
    g_mounts[g_mount_count].prefix_len = strlen(prefix);
    g_mounts[g_mount_count].ops        = ops;
    g_mounts[g_mount_count].fs_data    = fs_data;
    g_mount_count++;

    return SYN_OK;
}

/**
 * @brief Find the mount point for a given path.
 * @param path      Absolute path.
 * @param rel_path  [out] Relative path within the mount.
 * @return Matching mount, or NULL.
 */
static const SYN_VfsMount *find_mount(const char *path, const char **rel_path)
{
    size_t longest_match = 0;
    const SYN_VfsMount *matched = NULL;

    for (size_t i = 0; i < g_mount_count; i++) {
        size_t len = g_mounts[i].prefix_len;
        if (strncmp(path, g_mounts[i].prefix, len) == 0) {
            /* Prefix match. Check if boundary is slash or end of string */
            if (path[len] == '\0' || path[len] == '/') {
                if (len > longest_match) {
                    longest_match = len;
                    matched = &g_mounts[i];
                }
            }
        }
    }

    if (matched) {
        *rel_path = path + longest_match;
        if (**rel_path == '\0') {
            *rel_path = "/";
        }
    }

    return matched;
}

int syn_vfs_open(const char *path, int flags)
{
    SYN_ASSERT(path != NULL);

    const char *rel_path = NULL;
    const SYN_VfsMount *m = find_mount(path, &rel_path);
    if (!m || !m->ops->open) {
        return -1; /* Mount not found or open not supported */
    }

    /* Find free file descriptor */
    int fd = -1;
    for (int i = 0; i < SYN_VFS_MAX_OPEN_FILES; i++) {
        if (!g_files[i].is_open) {
            fd = i;
            break;
        }
    }

    if (fd < 0) {
        return -2; /* No free file descriptors */
    }

    g_files[fd].ops     = m->ops;
    g_files[fd].fs_file = NULL;
    g_files[fd].is_open = true;

    int ret = m->ops->open(&g_files[fd], rel_path, flags, m->fs_data);
    if (ret < 0) {
        g_files[fd].is_open = false;
        return ret;
    }

    return fd;
}

int syn_vfs_close(int fd)
{
    if (fd < 0 || fd >= SYN_VFS_MAX_OPEN_FILES || !g_files[fd].is_open) {
        return -1;
    }

    int ret = 0;
    if (g_files[fd].ops->close) {
        ret = g_files[fd].ops->close(&g_files[fd]);
    }

    g_files[fd].is_open = false;
    return ret;
}

int syn_vfs_read(int fd, void *buf, size_t len)
{
    if (fd < 0 || fd >= SYN_VFS_MAX_OPEN_FILES || !g_files[fd].is_open) {
        return -1;
    }
    if (!g_files[fd].ops->read) {
        return -2;
    }
    return g_files[fd].ops->read(&g_files[fd], buf, len);
}

int syn_vfs_write(int fd, const void *buf, size_t len)
{
    if (fd < 0 || fd >= SYN_VFS_MAX_OPEN_FILES || !g_files[fd].is_open) {
        return -1;
    }
    if (!g_files[fd].ops->write) {
        return -2;
    }
    return g_files[fd].ops->write(&g_files[fd], buf, len);
}

int32_t syn_vfs_seek(int fd, int32_t offset, int whence)
{
    if (fd < 0 || fd >= SYN_VFS_MAX_OPEN_FILES || !g_files[fd].is_open) {
        return -1;
    }
    if (!g_files[fd].ops->seek) {
        return -2;
    }
    return g_files[fd].ops->seek(&g_files[fd], offset, whence);
}

int32_t syn_vfs_tell(int fd)
{
    if (fd < 0 || fd >= SYN_VFS_MAX_OPEN_FILES || !g_files[fd].is_open) {
        return -1;
    }
    if (!g_files[fd].ops->tell) {
        return -2;
    }
    return g_files[fd].ops->tell(&g_files[fd]);
}

int syn_vfs_unlink(const char *path)
{
    SYN_ASSERT(path != NULL);
    const char *rel_path = NULL;
    const SYN_VfsMount *m = find_mount(path, &rel_path);
    if (!m || !m->ops->unlink) {
        return -1;
    }
    return m->ops->unlink(rel_path, m->fs_data);
}

int syn_vfs_mkdir(const char *path)
{
    SYN_ASSERT(path != NULL);
    const char *rel_path = NULL;
    const SYN_VfsMount *m = find_mount(path, &rel_path);
    if (!m || !m->ops->mkdir) {
        return -1;
    }
    return m->ops->mkdir(rel_path, m->fs_data);
}

int syn_vfs_opendir(const char *path)
{
    SYN_ASSERT(path != NULL);
    const char *rel_path = NULL;
    const SYN_VfsMount *m = find_mount(path, &rel_path);
    if (!m || !m->ops->opendir) {
        return -1;
    }

    /* Find free dir descriptor */
    int dd = -1;
    for (int i = 0; i < SYN_VFS_MAX_OPEN_DIRS; i++) {
        if (!g_dirs[i].is_open) {
            dd = i;
            break;
        }
    }

    if (dd < 0) {
        return -2; /* No free dir slots */
    }

    g_dirs[dd].ops    = m->ops;
    g_dirs[dd].fs_dir = NULL;
    g_dirs[dd].is_open = true;

    int ret = m->ops->opendir(&g_dirs[dd], rel_path, m->fs_data);
    if (ret < 0) {
        g_dirs[dd].is_open = false;
        return ret;
    }

    return dd;
}

int syn_vfs_readdir(int dd, SYN_VfsDirEnt *ent)
{
    if (dd < 0 || dd >= SYN_VFS_MAX_OPEN_DIRS || !g_dirs[dd].is_open) {
        return -1;
    }
    if (!g_dirs[dd].ops->readdir) {
        return -2;
    }
    return g_dirs[dd].ops->readdir(&g_dirs[dd], ent);
}

int syn_vfs_closedir(int dd)
{
    if (dd < 0 || dd >= SYN_VFS_MAX_OPEN_DIRS || !g_dirs[dd].is_open) {
        return -1;
    }

    int ret = 0;
    if (g_dirs[dd].ops->closedir) {
        ret = g_dirs[dd].ops->closedir(&g_dirs[dd]);
    }

    g_dirs[dd].is_open = false;
    return ret;
}

#endif /* SYN_USE_VFS */
