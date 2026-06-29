#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_LFS) || SYN_USE_LFS

#if defined(SYN_USE_VFS) && !SYN_USE_VFS
  #error "syn_lfs requires SYN_USE_VFS=1"
#endif

/**
 * @file syn_lfs.c
 * @brief LittleFS filesystem VFS adapter implementation.
 */

#include "syn_lfs.h"
#include "../port/syn_port_flash.h"
#include "../util/syn_assert.h"

#if __has_include("lfs.h")
#include "lfs.h"

/* Context structs that pair lfs_t* with each file/dir — avoids relying on
 * internal lfs_file_t/lfs_dir_t back-pointers which are version-dependent. */
typedef struct {
    lfs_t       *lfs;
    lfs_file_t   file;
    bool         used;
} SYN_LfsFileCtx;

typedef struct {
    lfs_t       *lfs;
    lfs_dir_t    dir;
    bool         used;
} SYN_LfsDirCtx;

static SYN_LfsFileCtx g_lfs_files[SYN_VFS_MAX_OPEN_FILES];
static SYN_LfsDirCtx  g_lfs_dirs[SYN_VFS_MAX_OPEN_DIRS];

/* ── LittleFS block device callbacks ────────────────────────────────────── */

static int syn_lfs_bd_read(const struct lfs_config *c, lfs_block_t block,
                           lfs_off_t off, void *buffer, lfs_size_t size)
{
    const SYN_LfsConfig *cfg = (const SYN_LfsConfig *)c->context;
    uint32_t addr = cfg->start_addr + (block * cfg->block_size) + off;
    if (syn_port_flash_read(addr, buffer, size) != SYN_OK) {
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}

static int syn_lfs_bd_prog(const struct lfs_config *c, lfs_block_t block,
                           lfs_off_t off, const void *buffer, lfs_size_t size)
{
    const SYN_LfsConfig *cfg = (const SYN_LfsConfig *)c->context;
    uint32_t addr = cfg->start_addr + (block * cfg->block_size) + off;
    if (syn_port_flash_write(addr, buffer, size) != SYN_OK) {
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}

static int syn_lfs_bd_erase(const struct lfs_config *c, lfs_block_t block)
{
    const SYN_LfsConfig *cfg = (const SYN_LfsConfig *)c->context;
    uint32_t addr = cfg->start_addr + (block * cfg->block_size);
    if (syn_port_flash_erase(addr) != SYN_OK) {
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}

static int syn_lfs_bd_sync(const struct lfs_config *c)
{
    (void)c;
    return LFS_ERR_OK;
}

void syn_lfs_init_config(struct lfs_config *cfg, const SYN_LfsConfig *syn_cfg)
{
    SYN_ASSERT(cfg != NULL);
    SYN_ASSERT(syn_cfg != NULL);

    cfg->context = (void *)syn_cfg;
    cfg->read  = syn_lfs_bd_read;
    cfg->prog  = syn_lfs_bd_prog;
    cfg->erase = syn_lfs_bd_erase;
    cfg->sync  = syn_lfs_bd_sync;

    /* These values depend on the flash hardware limitations */
    cfg->read_size      = 16;
    cfg->prog_size      = 16;
    cfg->block_size     = syn_cfg->block_size;
    cfg->block_count    = syn_cfg->size / syn_cfg->block_size;
    cfg->cache_size     = 64;
    cfg->lookahead_size = 16;
    cfg->block_cycles   = 500;
}

/* ── VFS Mapping ────────────────────────────────────────────────────────── */

static int syn_lfs_vfs_open(SYN_VfsFile *file, const char *path, int flags, void *fs_data)
{
    lfs_t *lfs = (lfs_t *)fs_data;
    SYN_ASSERT(lfs != NULL);

    /* Allocate a context slot from the static pool */
    int f_idx = -1;
    for (int i = 0; i < SYN_VFS_MAX_OPEN_FILES; i++) {
        if (!g_lfs_files[i].used) {
            f_idx = i;
            break;
        }
    }
    if (f_idx < 0) {
        return -1;
    }

    g_lfs_files[f_idx].used = true;
    g_lfs_files[f_idx].lfs  = lfs;
    file->fs_file = &g_lfs_files[f_idx];

    /* Map flags */
    int lfs_flags = 0;
    int mode = flags & 0x03;
    if (mode == SYN_O_RDONLY) lfs_flags |= LFS_O_RDONLY;
    else if (mode == SYN_O_WRONLY) lfs_flags |= LFS_O_WRONLY;
    else if (mode == SYN_O_RDWR) lfs_flags |= LFS_O_RDWR;

    if (flags & SYN_O_CREAT)  lfs_flags |= LFS_O_CREAT;
    if (flags & SYN_O_APPEND) lfs_flags |= LFS_O_APPEND;
    if (flags & SYN_O_TRUNC)  lfs_flags |= LFS_O_TRUNC;

    int ret = lfs_file_open(lfs, &g_lfs_files[f_idx].file, path, lfs_flags);
    if (ret < 0) {
        g_lfs_files[f_idx].used = false;
        file->fs_file = NULL;
        return ret;
    }

    return 0;
}

static int syn_lfs_vfs_close(SYN_VfsFile *file)
{
    SYN_LfsFileCtx *ctx = (SYN_LfsFileCtx *)file->fs_file;
    if (!ctx) return -1;

    int ret = lfs_file_close(ctx->lfs, &ctx->file);
    ctx->used = false;
    file->fs_file = NULL;
    return ret;
}

static int syn_lfs_vfs_read(SYN_VfsFile *file, void *buf, size_t len)
{
    SYN_LfsFileCtx *ctx = (SYN_LfsFileCtx *)file->fs_file;
    if (!ctx) return -1;
    return (int)lfs_file_read(ctx->lfs, &ctx->file, buf, len);
}

static int syn_lfs_vfs_write(SYN_VfsFile *file, const void *buf, size_t len)
{
    SYN_LfsFileCtx *ctx = (SYN_LfsFileCtx *)file->fs_file;
    if (!ctx) return -1;
    return (int)lfs_file_write(ctx->lfs, &ctx->file, buf, len);
}

static int32_t syn_lfs_vfs_seek(SYN_VfsFile *file, int32_t offset, int whence)
{
    SYN_LfsFileCtx *ctx = (SYN_LfsFileCtx *)file->fs_file;
    if (!ctx) return -1;

    int lfs_whence = LFS_SEEK_SET;
    if (whence == SYN_SEEK_CUR) lfs_whence = LFS_SEEK_CUR;
    else if (whence == SYN_SEEK_END) lfs_whence = LFS_SEEK_END;

    return (int32_t)lfs_file_seek(ctx->lfs, &ctx->file, offset, lfs_whence);
}

static int32_t syn_lfs_vfs_tell(SYN_VfsFile *file)
{
    SYN_LfsFileCtx *ctx = (SYN_LfsFileCtx *)file->fs_file;
    if (!ctx) return -1;
    return (int32_t)lfs_file_tell(ctx->lfs, &ctx->file);
}

static int syn_lfs_vfs_unlink(const char *path, void *fs_data)
{
    lfs_t *lfs = (lfs_t *)fs_data;
    return lfs_remove(lfs, path);
}

static int syn_lfs_vfs_mkdir(const char *path, void *fs_data)
{
    lfs_t *lfs = (lfs_t *)fs_data;
    return lfs_mkdir(lfs, path);
}

static int syn_lfs_vfs_opendir(SYN_VfsDir *dir, const char *path, void *fs_data)
{
    lfs_t *lfs = (lfs_t *)fs_data;
    SYN_ASSERT(lfs != NULL);

    int d_idx = -1;
    for (int i = 0; i < SYN_VFS_MAX_OPEN_DIRS; i++) {
        if (!g_lfs_dirs[i].used) {
            d_idx = i;
            break;
        }
    }
    if (d_idx < 0) {
        return -1;
    }

    g_lfs_dirs[d_idx].used = true;
    g_lfs_dirs[d_idx].lfs  = lfs;
    dir->fs_dir = &g_lfs_dirs[d_idx];

    int ret = lfs_dir_open(lfs, &g_lfs_dirs[d_idx].dir, path);
    if (ret < 0) {
        g_lfs_dirs[d_idx].used = false;
        dir->fs_dir = NULL;
        return ret;
    }

    return 0;
}

static int syn_lfs_vfs_readdir(SYN_VfsDir *dir, SYN_VfsDirEnt *ent)
{
    SYN_LfsDirCtx *ctx = (SYN_LfsDirCtx *)dir->fs_dir;
    if (!ctx) return -1;

    struct lfs_info info;
    int ret = lfs_dir_read(ctx->lfs, &ctx->dir, &info);
    if (ret <= 0) {
        return ret; /* 0 for EOF, negative for error */
    }

    strncpy(ent->name, info.name, sizeof(ent->name) - 1);
    ent->name[sizeof(ent->name) - 1] = '\0';
    ent->size = info.size;
    ent->is_dir = (info.type == LFS_TYPE_DIR);

    return 1;
}

static int syn_lfs_vfs_closedir(SYN_VfsDir *dir)
{
    SYN_LfsDirCtx *ctx = (SYN_LfsDirCtx *)dir->fs_dir;
    if (!ctx) return -1;

    int ret = lfs_dir_close(ctx->lfs, &ctx->dir);
    ctx->used = false;
    dir->fs_dir = NULL;
    return ret;
}

static const SYN_VfsOps g_lfs_vfs_ops = {
    .open     = syn_lfs_vfs_open,
    .close    = syn_lfs_vfs_close,
    .read     = syn_lfs_vfs_read,
    .write    = syn_lfs_vfs_write,
    .seek     = syn_lfs_vfs_seek,
    .tell     = syn_lfs_vfs_tell,
    .unlink   = syn_lfs_vfs_unlink,
    .mkdir    = syn_lfs_vfs_mkdir,
    .opendir  = syn_lfs_vfs_opendir,
    .readdir  = syn_lfs_vfs_readdir,
    .closedir = syn_lfs_vfs_closedir
};

const SYN_VfsOps *syn_lfs_get_ops(void)
{
    return &g_lfs_vfs_ops;
}

#else

/* Mock stubs if littlefs header is not present */
const SYN_VfsOps *syn_lfs_get_ops(void)
{
    return NULL;
}

#endif

#endif /* SYN_USE_LFS */
