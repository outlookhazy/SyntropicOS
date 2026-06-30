#ifndef LFS_H
#define LFS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t lfs_size_t;
typedef uint32_t lfs_off_t;
typedef uint32_t lfs_block_t;
typedef int32_t  lfs_ssize_t;
typedef int32_t  lfs_soff_t;

struct lfs_config {
    void *context;
    int (*read)(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
    int (*prog)(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
    int (*erase)(const struct lfs_config *c, lfs_block_t block);
    int (*sync)(const struct lfs_config *c);
    
    lfs_size_t read_size;
    lfs_size_t prog_size;
    lfs_size_t block_size;
    lfs_size_t block_count;
    lfs_size_t cache_size;
    lfs_size_t lookahead_size;
    lfs_size_t block_cycles;
};

typedef struct {
    int dummy;
} lfs_t;

typedef struct {
    int dummy;
} lfs_file_t;

typedef struct {
    int dummy;
} lfs_dir_t;

struct lfs_info {
    uint8_t type;
    lfs_size_t size;
    char name[256];
};

#define LFS_ERR_OK          0
#define LFS_ERR_IO          -5

#define LFS_O_RDONLY        1
#define LFS_O_WRONLY        2
#define LFS_O_RDWR          3
#define LFS_O_CREAT         0x0100
#define LFS_O_APPEND        0x0200
#define LFS_O_TRUNC         0x0400

#define LFS_SEEK_SET        0
#define LFS_SEEK_CUR        1
#define LFS_SEEK_END        2

#define LFS_TYPE_REG        1
#define LFS_TYPE_DIR        2

/* Mock APIs used by syn_lfs.c */
int lfs_file_open(lfs_t *lfs, lfs_file_t *file, const char *path, int flags);
int lfs_file_close(lfs_t *lfs, lfs_file_t *file);
lfs_ssize_t lfs_file_read(lfs_t *lfs, lfs_file_t *file, void *buffer, lfs_size_t size);
lfs_ssize_t lfs_file_write(lfs_t *lfs, lfs_file_t *file, const void *buffer, lfs_size_t size);
lfs_soff_t lfs_file_seek(lfs_t *lfs, lfs_file_t *file, lfs_soff_t off, int whence);
lfs_soff_t lfs_file_tell(lfs_t *lfs, lfs_file_t *file);
int lfs_remove(lfs_t *lfs, const char *path);
int lfs_mkdir(lfs_t *lfs, const char *path);
int lfs_dir_open(lfs_t *lfs, lfs_dir_t *dir, const char *path);
int lfs_dir_read(lfs_t *lfs, lfs_dir_t *dir, struct lfs_info *info);
int lfs_dir_close(lfs_t *lfs, lfs_dir_t *dir);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LFS_H */
