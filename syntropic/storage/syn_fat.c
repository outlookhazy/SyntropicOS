#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_FAT) || SYN_USE_FAT

#if defined(SYN_USE_VFS) && !SYN_USE_VFS
  #error "syn_fat requires SYN_USE_VFS=1"
#endif

#include "syn_fat.h"
#include "../log/syn_log.h"
#include "../util/syn_assert.h"
#include <string.h>

#define TAG "syn_fat"

/* ── Custom FAT volume context ──────────────────────────────────────────── */

/** @brief Internal FAT volume metadata parsed from the boot sector. */
typedef struct {
    uint16_t bytes_per_sector;    /**< Bytes per logical sector            */
    uint8_t  sectors_per_cluster; /**< Sectors per allocation cluster      */
    uint16_t reserved_sectors;    /**< Reserved sectors before first FAT   */
    uint8_t  num_fats;            /**< Number of FAT copies               */
    uint16_t fat16_root_entries;  /**< Root dir entry count (FAT16 only)   */
    uint32_t fat_sectors;         /**< Sectors occupied by each FAT        */
    uint32_t root_cluster;        /**< Root directory cluster (FAT32 only) */
    bool     is_fat32;            /**< true if FAT32, false if FAT16       */

    /* Calculated offsets (in sectors) */
    uint32_t fat_start_sector;    /**< First sector of FAT                 */
    uint32_t root_start_sector;   /**< First sector of root dir (FAT16)    */
    uint32_t root_sectors;        /**< Sectors for root dir (FAT16)        */
    uint32_t data_start_sector;   /**< First sector of data region         */
} FAT_Volume;

static FAT_Volume g_vol;
SYN_SD g_sd;

/* ── Open file context pool ─────────────────────────────────────────────── */

/** @brief Open file tracking context (pooled). */
typedef struct {
    char     name[11];       /**< 8.3 FAT directory name                 */
    uint32_t start_cluster;  /**< First cluster of file data              */
    uint32_t size;           /**< File size in bytes                      */
    uint32_t offset;         /**< Current read/write position             */
    uint32_t dir_sector;     /**< Sector containing the dir entry         */
    uint32_t dir_offset;     /**< Byte offset of dir entry within sector  */
    int      mode;           /**< Open mode (read, write, etc.)           */
    bool     used;           /**< true if this slot is in use             */
} SYN_FatFileCtx;

static SYN_FatFileCtx g_fat_files[SYN_VFS_MAX_OPEN_FILES];

/* ── Helper Functions ───────────────────────────────────────────────────── */

static uint32_t find_partition_start(const uint8_t *sector0)
{
    if (sector0[510] != 0x55 || sector0[511] != 0xAA) return 0;

    /* Check MBR partition 0 entry at offset 446 (0x1BE) */
    const uint8_t *part = &sector0[446];
    uint8_t type = part[4];
    if (type == 0x04 || type == 0x06 || type == 0x0B || type == 0x0C || type == 0x0E || 
        type == 0x14 || type == 0x16 || type == 0x1B || type == 0x1C) {
        uint32_t start = (uint32_t)part[8] | 
                         ((uint32_t)part[9] << 8) | 
                         ((uint32_t)part[10] << 16) | 
                         ((uint32_t)part[11] << 24);
        return start;
    }
    return 0;
}

static bool fat_parse_bpb(FAT_Volume *vol, const uint8_t *bpb, uint32_t volume_start)
{
    if (bpb[510] != 0x55 || bpb[511] != 0xAA) return false;

    vol->bytes_per_sector = (uint16_t)bpb[11] | ((uint16_t)bpb[12] << 8);
    if (vol->bytes_per_sector != 512) return false;

    vol->sectors_per_cluster = bpb[13];
    if (vol->sectors_per_cluster == 0) return false;

    vol->reserved_sectors = (uint16_t)bpb[14] | ((uint16_t)bpb[15] << 8);
    vol->num_fats = bpb[16];
    vol->fat16_root_entries = (uint16_t)bpb[17] | ((uint16_t)bpb[18] << 8);

    uint16_t spf16 = (uint16_t)bpb[22] | ((uint16_t)bpb[23] << 8);
    if (spf16 != 0) {
        vol->fat_sectors = spf16;
        vol->is_fat32 = false;
        vol->root_cluster = 0;
    } else {
        vol->fat_sectors = (uint32_t)bpb[36] | ((uint32_t)bpb[37] << 8) | 
                           ((uint32_t)bpb[38] << 16) | ((uint32_t)bpb[39] << 24);
        vol->root_cluster = (uint32_t)bpb[44] | ((uint32_t)bpb[45] << 8) | 
                            ((uint32_t)bpb[46] << 16) | ((uint32_t)bpb[47] << 24);
        vol->is_fat32 = true;
    }

    vol->fat_start_sector = volume_start + vol->reserved_sectors;

    if (vol->is_fat32) {
        vol->root_start_sector = 0;
        vol->root_sectors = 0;
        vol->data_start_sector = vol->fat_start_sector + (vol->num_fats * vol->fat_sectors);
    } else {
        vol->root_start_sector = vol->fat_start_sector + (vol->num_fats * vol->fat_sectors);
        vol->root_sectors = ((vol->fat16_root_entries * 32) + 511) / 512;
        vol->data_start_sector = vol->root_start_sector + vol->root_sectors;
    }

    return true;
}

static uint32_t cluster_to_sector(const FAT_Volume *vol, uint32_t cluster)
{
    if (cluster < 2) return 0;
    return vol->data_start_sector + (cluster - 2) * vol->sectors_per_cluster;
}

static uint32_t read_fat_entry(const FAT_Volume *vol, uint32_t cluster)
{
    uint8_t sec_buf[512];
    uint32_t fat_sector;
    uint32_t offset;

    if (vol->is_fat32) {
        fat_sector = vol->fat_start_sector + ((cluster * 4) / 512);
        offset = (cluster * 4) % 512;
    } else {
        fat_sector = vol->fat_start_sector + ((cluster * 2) / 512);
        offset = (cluster * 2) % 512;
    }

    if (syn_sd_read(&g_sd, fat_sector, sec_buf) != SYN_OK) {
        return 0x0FFFFFFF;
    }

    if (vol->is_fat32) {
        uint32_t entry = (uint32_t)sec_buf[offset] | 
                         ((uint32_t)sec_buf[offset + 1] << 8) | 
                         ((uint32_t)sec_buf[offset + 2] << 16) | 
                         ((uint32_t)sec_buf[offset + 3] << 24);
        return entry & 0x0FFFFFFF;
    } else {
        uint16_t entry = (uint16_t)sec_buf[offset] | 
                         ((uint16_t)sec_buf[offset + 1] << 8);
        if (entry >= 0xFFF8) return 0x0FFFFFFF;
        return entry;
    }
}

static bool write_fat_entry(const FAT_Volume *vol, uint32_t cluster, uint32_t value)
{
    uint8_t sec_buf[512];
    uint32_t fat_sector;
    uint32_t offset;

    if (vol->is_fat32) {
        fat_sector = vol->fat_start_sector + ((cluster * 4) / 512);
        offset = (cluster * 4) % 512;
    } else {
        fat_sector = vol->fat_start_sector + ((cluster * 2) / 512);
        offset = (cluster * 2) % 512;
    }

    if (syn_sd_read(&g_sd, fat_sector, sec_buf) != SYN_OK) return false;

    if (vol->is_fat32) {
        sec_buf[offset]     = value & 0xFF;
        sec_buf[offset + 1] = (value >> 8) & 0xFF;
        sec_buf[offset + 2] = (value >> 16) & 0xFF;
        sec_buf[offset + 3] = (value >> 24) & 0xFF;
    } else {
        sec_buf[offset]     = value & 0xFF;
        sec_buf[offset + 1] = (value >> 8) & 0xFF;
    }

    if (syn_sd_write(&g_sd, fat_sector, sec_buf) != SYN_OK) return false;

    if (vol->num_fats > 1) {
        if (syn_sd_write(&g_sd, fat_sector + vol->fat_sectors, sec_buf) != SYN_OK) return false;
    }

    return true;
}

static uint32_t find_free_cluster(const FAT_Volume *vol)
{
    for (uint32_t c = 2; c < 65536; c++) {
        uint32_t entry = read_fat_entry(vol, c);
        if (entry == 0) return c;
    }
    return 0;
}

static void path_to_fat_name(const char *path, char *fat_name)
{
    if (path[0] == '/') path++;

    memset(fat_name, ' ', 11);

    int i = 0;
    while (path[i] != '\0' && path[i] != '.' && i < 8) {
        char c = path[i];
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
        fat_name[i] = c;
        i++;
    }

    while (path[i] != '\0' && path[i] != '.') {
        i++;
    }

    if (path[i] == '.') {
        i++;
        int j = 0;
        while (path[i] != '\0' && j < 3) {
            char c = path[i];
            if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
            fat_name[8 + j] = c;
            i++;
            j++;
        }
    }
}

/** @brief Result of a directory scan — location of a matching entry. */
typedef struct {
    uint32_t sector;        /**< Sector containing the entry              */
    uint32_t offset;        /**< Byte offset within that sector           */
    uint32_t start_cluster; /**< First cluster of the found file          */
    uint32_t file_size;     /**< Size in bytes of the found file          */
    bool     found;         /**< true if a matching entry was found       */
} DirEntryLoc;

static bool scan_root_dir(const FAT_Volume *vol, const char *fat_name, DirEntryLoc *loc, bool find_empty_slot)
{
    uint8_t sec_buf[512];
    loc->found = false;

    if (vol->is_fat32) {
        uint32_t cluster = vol->root_cluster;
        while (cluster < 0x0F000000) {
            uint32_t start_sector = cluster_to_sector(vol, cluster);
            for (uint8_t s = 0; s < vol->sectors_per_cluster; s++) {
                uint32_t current_sector = start_sector + s;
                if (syn_sd_read(&g_sd, current_sector, sec_buf) != SYN_OK) return false;

                for (uint32_t off = 0; off < 512; off += 32) {
                    uint8_t first_char = sec_buf[off];
                    if (first_char == 0x00 && !find_empty_slot) {
                        return false;
                    }

                    if (find_empty_slot) {
                        if (first_char == 0x00 || first_char == 0xE5) {
                            loc->sector = current_sector;
                            loc->offset = off;
                            loc->found = true;
                            return true;
                        }
                    } else {
                        if (first_char != 0xE5 && memcmp(&sec_buf[off], fat_name, 11) == 0) {
                            loc->sector = current_sector;
                            loc->offset = off;
                            loc->start_cluster = (uint32_t)sec_buf[off + 26] | 
                                                 ((uint32_t)sec_buf[off + 27] << 8) |
                                                 ((uint32_t)sec_buf[off + 20] << 16) | 
                                                 ((uint32_t)sec_buf[off + 21] << 24);
                            loc->file_size = (uint32_t)sec_buf[off + 28] |
                                             ((uint32_t)sec_buf[off + 29] << 8) |
                                             ((uint32_t)sec_buf[off + 30] << 16) |
                                             ((uint32_t)sec_buf[off + 31] << 24);
                            loc->found = true;
                            return true;
                        }
                    }
                }
            }
            cluster = read_fat_entry(vol, cluster);
        }
    } else {
        for (uint32_t s = 0; s < vol->root_sectors; s++) {
            uint32_t current_sector = vol->root_start_sector + s;
            if (syn_sd_read(&g_sd, current_sector, sec_buf) != SYN_OK) return false;

            for (uint32_t off = 0; off < 512; off += 32) {
                uint8_t first_char = sec_buf[off];
                if (first_char == 0x00 && !find_empty_slot) {
                    return false;
                }

                if (find_empty_slot) {
                    if (first_char == 0x00 || first_char == 0xE5) {
                        loc->sector = current_sector;
                        loc->offset = off;
                        loc->found = true;
                        return true;
                    }
                } else {
                    if (first_char != 0xE5 && memcmp(&sec_buf[off], fat_name, 11) == 0) {
                        loc->sector = current_sector;
                        loc->offset = off;
                        loc->start_cluster = (uint32_t)sec_buf[off + 26] | 
                                             ((uint32_t)sec_buf[off + 27] << 8);
                        loc->file_size = (uint32_t)sec_buf[off + 28] |
                                         ((uint32_t)sec_buf[off + 29] << 8) |
                                         ((uint32_t)sec_buf[off + 30] << 16) |
                                         ((uint32_t)sec_buf[off + 31] << 24);
                        loc->found = true;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

static uint32_t get_cluster_for_offset(const FAT_Volume *vol, uint32_t start_cluster, uint32_t offset)
{
    uint32_t cluster_size = vol->sectors_per_cluster * 512;
    uint32_t target_idx = offset / cluster_size;

    uint32_t curr_cluster = start_cluster;
    for (uint32_t i = 0; i < target_idx; i++) {
        uint32_t next = read_fat_entry(vol, curr_cluster);
        if (next >= (vol->is_fat32 ? 0x0FFFFFF8 : 0xFFF8)) {
            return curr_cluster;
        }
        curr_cluster = next;
    }
    return curr_cluster;
}

/* ── VFS Mappings ───────────────────────────────────────────────────────── */

static int syn_fat_vfs_open(SYN_VfsFile *file, const char *path, int flags, void *fs_data)
{
    (void)fs_data;

    char fat_name[11];
    path_to_fat_name(path, fat_name);

    int f_idx = -1;
    for (int i = 0; i < SYN_VFS_MAX_OPEN_FILES; i++) {
        if (!g_fat_files[i].used) {
            f_idx = i;
            break;
        }
    }
    if (f_idx < 0) return -1;

    DirEntryLoc loc;
    bool exists = scan_root_dir(&g_vol, fat_name, &loc, false);
    int access_mode = flags & 0x03;

    if (!exists) {
        if (!(flags & SYN_O_CREAT)) {
            return -2;
        }

        DirEntryLoc empty_slot;
        if (!scan_root_dir(&g_vol, fat_name, &empty_slot, true)) {
            return -3;
        }

        uint32_t free_cluster = find_free_cluster(&g_vol);
        if (free_cluster == 0) {
            return -4;
        }

        if (!write_fat_entry(&g_vol, free_cluster, 0x0FFFFFFF)) {
            return -5;
        }

        uint8_t sec_buf[512];
        if (syn_sd_read(&g_sd, empty_slot.sector, sec_buf) != SYN_OK) return -6;

        uint8_t *entry = &sec_buf[empty_slot.offset];
        memcpy(entry, fat_name, 11);
        entry[11] = 0x20;
        memset(entry + 12, 0, 8);

        entry[20] = (free_cluster >> 16) & 0xFF;
        entry[21] = (free_cluster >> 24) & 0xFF;
        entry[26] = free_cluster & 0xFF;
        entry[27] = (free_cluster >> 8) & 0xFF;

        memset(entry + 28, 0, 4);

        if (syn_sd_write(&g_sd, empty_slot.sector, sec_buf) != SYN_OK) return -7;

        loc.sector = empty_slot.sector;
        loc.offset = empty_slot.offset;
        loc.start_cluster = free_cluster;
        loc.file_size = 0;
    }

    g_fat_files[f_idx].used = true;
    memcpy(g_fat_files[f_idx].name, fat_name, 11);
    g_fat_files[f_idx].start_cluster = loc.start_cluster;
    g_fat_files[f_idx].size = loc.file_size;
    g_fat_files[f_idx].offset = 0;
    g_fat_files[f_idx].dir_sector = loc.sector;
    g_fat_files[f_idx].dir_offset = loc.offset;
    g_fat_files[f_idx].mode = access_mode;

    file->fs_file = &g_fat_files[f_idx];

    if (flags & SYN_O_TRUNC) {
        g_fat_files[f_idx].size = 0;
        uint8_t sec_buf[512];
        if (syn_sd_read(&g_sd, loc.sector, sec_buf) == SYN_OK) {
            memset(&sec_buf[loc.offset + 28], 0, 4);
            syn_sd_write(&g_sd, loc.sector, sec_buf);
        }
    }

    if (flags & SYN_O_APPEND) {
        g_fat_files[f_idx].offset = g_fat_files[f_idx].size;
    }

    return 0;
}

static int syn_fat_vfs_close(SYN_VfsFile *file)
{
    SYN_FatFileCtx *ctx = (SYN_FatFileCtx *)file->fs_file;
    if (!ctx) return -1;
    ctx->used = false;
    file->fs_file = NULL;
    return 0;
}

static int syn_fat_vfs_read(SYN_VfsFile *file, void *buf, size_t len)
{
    SYN_FatFileCtx *ctx = (SYN_FatFileCtx *)file->fs_file;
    if (!ctx) return -1;

    if (ctx->offset >= ctx->size) return 0;
    if (ctx->offset + len > ctx->size) {
        len = ctx->size - ctx->offset;
    }

    uint8_t sec_buf[512];
    uint32_t bytes_read = 0;
    uint8_t *dest = (uint8_t *)buf;
    uint32_t cluster_size = g_vol.sectors_per_cluster * 512;

    while (bytes_read < len) {
        uint32_t curr_cluster = get_cluster_for_offset(&g_vol, ctx->start_cluster, ctx->offset);
        uint32_t byte_in_cluster = ctx->offset % cluster_size;
        uint32_t sector_in_cluster = byte_in_cluster / 512;
        uint32_t byte_in_sector = byte_in_cluster % 512;

        uint32_t sector = cluster_to_sector(&g_vol, curr_cluster) + sector_in_cluster;

        if (syn_sd_read(&g_sd, sector, sec_buf) != SYN_OK) {
            return -2;
        }

        uint32_t to_copy = 512 - byte_in_sector;
        if (to_copy > (len - bytes_read)) {
            to_copy = len - bytes_read;
        }

        memcpy(dest + bytes_read, sec_buf + byte_in_sector, to_copy);
        bytes_read += to_copy;
        ctx->offset += to_copy;
    }

    return (int)bytes_read;
}

static int syn_fat_vfs_write(SYN_VfsFile *file, const void *buf, size_t len)
{
    SYN_FatFileCtx *ctx = (SYN_FatFileCtx *)file->fs_file;
    if (!ctx) return -1;

    uint8_t sec_buf[512];
    uint32_t bytes_written = 0;
    const uint8_t *src = (const uint8_t *)buf;
    uint32_t cluster_size = g_vol.sectors_per_cluster * 512;

    while (bytes_written < len) {
        uint32_t target_idx = ctx->offset / cluster_size;
        uint32_t curr_cluster = ctx->start_cluster;

        for (uint32_t i = 0; i < target_idx; i++) {
            uint32_t next = read_fat_entry(&g_vol, curr_cluster);
            if (next >= (g_vol.is_fat32 ? 0x0FFFFFF8 : 0xFFF8)) {
                uint32_t new_cluster = find_free_cluster(&g_vol);
                if (new_cluster == 0) return -2;

                if (!write_fat_entry(&g_vol, curr_cluster, new_cluster)) return -3;
                if (!write_fat_entry(&g_vol, new_cluster, 0x0FFFFFFF)) return -4;

                curr_cluster = new_cluster;
            } else {
                curr_cluster = next;
            }
        }

        uint32_t byte_in_cluster = ctx->offset % cluster_size;
        uint32_t sector_in_cluster = byte_in_cluster / 512;
        uint32_t byte_in_sector = byte_in_cluster % 512;

        uint32_t sector = cluster_to_sector(&g_vol, curr_cluster) + sector_in_cluster;

        if (byte_in_sector != 0 || (len - bytes_written) < 512) {
            if (syn_sd_read(&g_sd, sector, sec_buf) != SYN_OK) {
                return -5;
            }
        }

        uint32_t to_write = 512 - byte_in_sector;
        if (to_write > (len - bytes_written)) {
            to_write = len - bytes_written;
        }

        memcpy(sec_buf + byte_in_sector, src + bytes_written, to_write);

        if (syn_sd_write(&g_sd, sector, sec_buf) != SYN_OK) {
            return -6;
        }

        bytes_written += to_write;
        ctx->offset += to_write;
        if (ctx->offset > ctx->size) {
            ctx->size = ctx->offset;
        }
    }

    uint8_t dir_buf[512];
    if (syn_sd_read(&g_sd, ctx->dir_sector, dir_buf) == SYN_OK) {
        dir_buf[ctx->dir_offset + 28] = ctx->size & 0xFF;
        dir_buf[ctx->dir_offset + 29] = (ctx->size >> 8) & 0xFF;
        dir_buf[ctx->dir_offset + 30] = (ctx->size >> 16) & 0xFF;
        dir_buf[ctx->dir_offset + 31] = (ctx->size >> 24) & 0xFF;
        syn_sd_write(&g_sd, ctx->dir_sector, dir_buf);
    }

    return (int)bytes_written;
}

static int32_t syn_fat_vfs_seek(SYN_VfsFile *file, int32_t offset, int whence)
{
    SYN_FatFileCtx *ctx = (SYN_FatFileCtx *)file->fs_file;
    if (!ctx) return -1;

    int32_t target_offset = 0;
    if (whence == SYN_SEEK_SET) {
        target_offset = offset;
    } else if (whence == SYN_SEEK_CUR) {
        target_offset = (int32_t)ctx->offset + offset;
    } else if (whence == SYN_SEEK_END) {
        target_offset = (int32_t)ctx->size + offset;
    } else {
        return -2;
    }

    if (target_offset < 0) target_offset = 0;
    if ((uint32_t)target_offset > ctx->size) target_offset = ctx->size;

    ctx->offset = (uint32_t)target_offset;
    return (int32_t)ctx->offset;
}

static int32_t syn_fat_vfs_tell(SYN_VfsFile *file)
{
    const SYN_FatFileCtx *ctx = (const SYN_FatFileCtx *)file->fs_file;
    if (!ctx) return -1;
    return (int32_t)ctx->offset;
}

static int syn_fat_vfs_unlink(const char *path, void *fs_data)
{
    (void)fs_data;

    char fat_name[11];
    path_to_fat_name(path, fat_name);

    DirEntryLoc loc;
    if (!scan_root_dir(&g_vol, fat_name, &loc, false)) {
        return -1;
    }

    uint8_t sec_buf[512];
    if (syn_sd_read(&g_sd, loc.sector, sec_buf) != SYN_OK) return -2;
    sec_buf[loc.offset] = 0xE5;
    if (syn_sd_write(&g_sd, loc.sector, sec_buf) != SYN_OK) return -3;

    uint32_t cluster = loc.start_cluster;
    uint32_t eof_val = g_vol.is_fat32 ? 0x0FFFFFF8 : 0xFFF8;
    while (cluster >= 2 && cluster < eof_val) {
        uint32_t next = read_fat_entry(&g_vol, cluster);
        write_fat_entry(&g_vol, cluster, 0);
        cluster = next;
    }

    return 0;
}

static const SYN_VfsOps g_fat_vfs_ops = {
    .open     = syn_fat_vfs_open,
    .close    = syn_fat_vfs_close,
    .read     = syn_fat_vfs_read,
    .write    = syn_fat_vfs_write,
    .seek     = syn_fat_vfs_seek,
    .tell     = syn_fat_vfs_tell,
    .unlink   = syn_fat_vfs_unlink,
    .mkdir    = NULL,
    .opendir  = NULL,
    .readdir  = NULL,
    .closedir = NULL
};

const SYN_VfsOps *syn_fat_get_ops(void)
{
    return &g_fat_vfs_ops;
}

SYN_Status syn_fat_init(uint8_t spi_bus, SYN_GPIO_Pin cs)
{
    if (syn_sd_init(&g_sd, spi_bus, cs) != SYN_OK) {
        SYN_LOG_E(TAG, "syn_sd_init failed");
        return SYN_ERROR;
    }

    uint8_t sector0[512];
    if (syn_sd_read(&g_sd, 0, sector0) != SYN_OK) {
        SYN_LOG_E(TAG, "Failed to read Sector 0");
        return SYN_ERROR;
    }

    uint32_t part_start = find_partition_start(sector0);
    uint8_t bpb_buf[512];

    if (part_start != 0) {
        SYN_LOG_I(TAG, "FAT partition found starting at sector %u", (unsigned)part_start);
        if (syn_sd_read(&g_sd, part_start, bpb_buf) != SYN_OK) {
            SYN_LOG_E(TAG, "Failed to read partition Boot Sector");
            return SYN_ERROR;
        }
    } else {
        memcpy(bpb_buf, sector0, 512);
    }

    if (!fat_parse_bpb(&g_vol, bpb_buf, part_start)) {
        SYN_LOG_E(TAG, "Invalid Boot Sector BPB format");
        return SYN_ERROR;
    }

    SYN_LOG_I(TAG, "Mounted %s filesystem: sectors/cluster=%u reserved=%u", 
              g_vol.is_fat32 ? "FAT32" : "FAT16", 
              (unsigned)g_vol.sectors_per_cluster,
              (unsigned)g_vol.reserved_sectors);

    memset(g_fat_files, 0, sizeof(g_fat_files));

    if (syn_vfs_mount("/sd", &g_fat_vfs_ops, NULL) != SYN_OK) {
        SYN_LOG_E(TAG, "VFS mount /sd failed");
        return SYN_ERROR;
    }

    return SYN_OK;
}

#endif /* SYN_USE_FAT */
