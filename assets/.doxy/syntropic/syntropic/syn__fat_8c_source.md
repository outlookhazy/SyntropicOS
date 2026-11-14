

# File syn\_fat.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_fat.c**](syn__fat_8c.md)

[Go to the documentation of this file](syn__fat_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_FAT) || SYN_USE_FAT

#if defined(SYN_USE_VFS) && !SYN_USE_VFS
#error "syn_fat requires SYN_USE_VFS=1"
#endif


#include "../log/syn_log.h"
#include "../util/syn_pack.h"
#include "syn_fat.h"

#include <limits.h>
#include <string.h>

#define TAG "syn_fat"

/* ── Custom FAT volume context ──────────────────────────────────────────── */

typedef struct {
    uint16_t bytes_per_sector;   
    uint8_t sectors_per_cluster; 
    uint16_t reserved_sectors;   
    uint8_t num_fats;            
    uint16_t fat16_root_entries; 
    uint32_t fat_sectors;        
    uint32_t root_cluster;       
    bool is_fat32;               

    /* Calculated offsets (in sectors) */
    uint32_t fat_start_sector;  
    uint32_t root_start_sector; 
    uint32_t root_sectors;      
    uint32_t data_start_sector; 
    uint32_t total_clusters;    
} FAT_Volume;

static FAT_Volume g_vol;

static SYN_SD g_sd;

/* ── Open file context pool ─────────────────────────────────────────────── */

typedef struct {
    char name[11];          
    uint32_t start_cluster; 
    uint32_t size;          
    uint32_t offset;        
    uint32_t dir_sector;    
    uint32_t dir_offset;    
    int flags;              
    bool used;              
} SYN_FatFileCtx;

static SYN_FatFileCtx g_fat_files[SYN_VFS_MAX_OPEN_FILES];

/* ── Helper Functions ───────────────────────────────────────────────────── */

static bool fat_is_name_open(const char fat_name[11])
{
    for (size_t i = 0U; i < (size_t)SYN_VFS_MAX_OPEN_FILES; i++) {
        if (g_fat_files[i].used && memcmp(g_fat_files[i].name, fat_name, 11U) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_fat_partition_type(uint8_t type)
{
    static const uint8_t types[] = {0x04U, 0x06U, 0x0BU, 0x0CU, 0x0EU, 0x14U, 0x16U, 0x1BU, 0x1CU};
    for (size_t i = 0U; i < sizeof(types); i++) {
        if (type == types[i]) {
            return true;
        }
    }
    return false;
}

static uint32_t find_partition_start(const uint8_t *sector0)
{
    if (sector0[510] != 0x55U || sector0[511] != 0xAAU) {
        return 0U;
    }

    /* Check MBR partition 0 entry at offset 446 (0x1BE) */
    const uint8_t *part = &sector0[446];
    if (is_fat_partition_type(part[4])) {
        return syn_peek_u32_le(part, 8);
    }
    return 0U;
}

static bool is_valid_cluster(const FAT_Volume *vol, uint32_t cluster)
{
    uint32_t max_valid = vol->is_fat32 ? 0x0FFFFFF6U : 0xFFF6U;
    if ((cluster < 2U) || (cluster > max_valid)) {
        return false;
    }
    return ((cluster - 2U) < vol->total_clusters);
}

static bool fat_compute_layout(FAT_Volume *vol, const uint8_t *bpb, uint32_t volume_start)
{
    if ((vol->fat_sectors == 0U) || (vol->num_fats == 0U)) {
        return false;
    }

    uint64_t fat_start = (uint64_t)volume_start + (uint64_t)vol->reserved_sectors;
    uint64_t all_fats = (uint64_t)vol->num_fats * (uint64_t)vol->fat_sectors;
    uint64_t root_secs =
        vol->is_fat32 ? 0ULL : ((((uint64_t)vol->fat16_root_entries * 32ULL) + 511ULL) / 512ULL);
    uint64_t root_start = vol->is_fat32 ? 0ULL : (fat_start + all_fats);
    uint64_t data_start = fat_start + all_fats + root_secs;

    uint32_t total_sec = (uint32_t)syn_peek_u16_le(bpb, 19);
    if (total_sec == 0U) {
        total_sec = syn_peek_u32_le(bpb, 32);
    }

    uint64_t non_data_sec = (uint64_t)vol->reserved_sectors + all_fats + root_secs;
    uint64_t vol_end = (uint64_t)volume_start + (uint64_t)total_sec;
    if ((vol_end > (uint64_t)UINT32_MAX) || ((uint64_t)total_sec <= non_data_sec)) {
        return false;
    }

    vol->fat_start_sector = (uint32_t)fat_start;
    vol->root_start_sector = (uint32_t)root_start;
    vol->root_sectors = (uint32_t)root_secs;
    vol->data_start_sector = (uint32_t)data_start;

    uint64_t clusters = ((uint64_t)total_sec - non_data_sec) / (uint64_t)vol->sectors_per_cluster;
    uint64_t fat_cap = ((uint64_t)vol->fat_sectors * 512ULL) / (vol->is_fat32 ? 4ULL : 2ULL);
    uint64_t max_clusters = vol->is_fat32 ? 0x0FFFFFF5ULL : 0xFFF5ULL;
    if ((clusters == 0ULL) || (fat_cap <= 2ULL)) {
        return false;
    }
    if (clusters > (fat_cap - 2ULL)) {
        clusters = fat_cap - 2ULL;
    }
    if (clusters > max_clusters) {
        clusters = max_clusters;
    }

    vol->total_clusters = (uint32_t)clusters;
    return true;
}

static bool fat_valid_spc(uint8_t spc)
{
    return (spc >= 1U) && (spc <= 128U) && ((spc & (uint8_t)(spc - 1U)) == 0U);
}

static bool fat_parse_bpb(FAT_Volume *vol, const uint8_t *bpb, uint32_t volume_start)
{
    if (bpb[510] != 0x55U || bpb[511] != 0xAAU) {
        return false;
    }

    vol->bytes_per_sector = syn_peek_u16_le(bpb, 11);
    vol->sectors_per_cluster = bpb[13];
    vol->reserved_sectors = syn_peek_u16_le(bpb, 14);
    if (vol->bytes_per_sector != 512U || !fat_valid_spc(vol->sectors_per_cluster) ||
        vol->reserved_sectors == 0U) {
        return false;
    }

    vol->num_fats = bpb[16];
    vol->fat16_root_entries = syn_peek_u16_le(bpb, 17);

    uint16_t spf16 = syn_peek_u16_le(bpb, 22);
    if (spf16 != 0U) {
        if (vol->fat16_root_entries == 0U) {
            return false;
        }
        vol->fat_sectors = spf16;
        vol->is_fat32 = false;
        vol->root_cluster = 0U;
    } else {
        if (vol->fat16_root_entries != 0U) {
            return false;
        }
        vol->fat_sectors = syn_peek_u32_le(bpb, 36);
        vol->root_cluster = syn_peek_u32_le(bpb, 44);
        vol->is_fat32 = true;
    }

    if (!fat_compute_layout(vol, bpb, volume_start)) {
        return false;
    }
    if (vol->is_fat32 && !is_valid_cluster(vol, vol->root_cluster)) {
        return false;
    }
    return true;
}

static uint32_t cluster_to_sector(const FAT_Volume *vol, uint32_t cluster)
{
    return vol->data_start_sector + ((cluster - 2U) * vol->sectors_per_cluster);
}

static void fat_entry_location(const FAT_Volume *vol, uint32_t cluster, uint32_t *fat_sector,
                               uint32_t *offset)
{
    uint32_t byte_off = vol->is_fat32 ? (cluster * 4U) : (cluster * 2U);
    *fat_sector = vol->fat_start_sector + (byte_off / 512U);
    *offset = byte_off % 512U;
}

static bool read_fat_entry(const FAT_Volume *vol, uint32_t cluster, uint32_t *out_entry)
{
    if (!is_valid_cluster(vol, cluster)) {
        *out_entry = 0x0FFFFFFFU;
        return true;
    }

    uint8_t sec_buf[512];
    uint32_t fat_sector = 0U;
    uint32_t offset = 0U;

    fat_entry_location(vol, cluster, &fat_sector, &offset);

    if (syn_sd_read(&g_sd, fat_sector, sec_buf) != SYN_OK) {
        return false;
    }

    if (vol->is_fat32) {
        uint32_t entry = syn_peek_u32_le(sec_buf, offset);
        *out_entry = entry & 0x0FFFFFFFU;
        return true;
    }

    uint16_t entry16 = syn_peek_u16_le(sec_buf, offset);
    *out_entry = (entry16 >= 0xFFF8U) ? 0x0FFFFFFFU : (uint32_t)entry16;
    return true;
}

static SYN_Status write_fat_entry(const FAT_Volume *vol, uint32_t cluster, uint32_t value)
{
    if (!is_valid_cluster(vol, cluster)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t sec_buf[512];
    uint32_t fat_sector = 0U;
    uint32_t offset = 0U;

    fat_entry_location(vol, cluster, &fat_sector, &offset);

    if (syn_sd_read(&g_sd, fat_sector, sec_buf) != SYN_OK) {
        return SYN_ERROR;
    }

    if (vol->is_fat32) {
        uint32_t existing = syn_peek_u32_le(sec_buf, offset);
        uint32_t merged = (existing & 0xF0000000U) | (value & 0x0FFFFFFFU);
        syn_poke_u32_le(merged, sec_buf, offset);
    } else {
        syn_poke_u16_le((uint16_t)value, sec_buf, offset);
    }

    if (syn_sd_write(&g_sd, fat_sector, sec_buf) != SYN_OK) {
        return SYN_ERROR;
    }

    if (vol->num_fats > 1U) {
        if (syn_sd_write(&g_sd, fat_sector + vol->fat_sectors, sec_buf) != SYN_OK) {
            return SYN_ERROR;
        }
    }

    return SYN_OK;
}

static uint32_t find_free_cluster(const FAT_Volume *vol)
{
    uint32_t limit = vol->total_clusters + 2U;
    for (uint32_t c = 2U; c < limit; c++) {
        uint32_t entry = 0U;
        if (!read_fat_entry(vol, c, &entry)) {
            return 0U;
        }
        if (entry == 0U) {
            return c;
        }
    }
    return 0U;
}

static void free_cluster_chain(const FAT_Volume *vol, uint32_t start_cluster)
{
    uint32_t cluster = start_cluster;
    uint32_t limit = vol->total_clusters;
    while (is_valid_cluster(vol, cluster) && limit > 0U) {
        uint32_t next = 0U;
        if (!read_fat_entry(vol, cluster, &next)) {
            break;
        }
        (void)write_fat_entry(vol, cluster, 0U);
        cluster = next;
        limit--;
    }
}

static char fat_toupper(char c)
{
    if (c >= 'a' && c <= 'z') {
        return (char)(c - 'a' + 'A');
    }
    return c;
}

static void format_83_chars(const char *name, char fat_name[11])
{
    memset(fat_name, ' ', 11);

    int i = 0;
    while (name[i] != '\0' && name[i] != '.' && i < 8) {
        fat_name[i] = fat_toupper(name[i]);
        i++;
    }

    while (name[i] != '\0' && name[i] != '.') {
        i++;
    }

    if (name[i] == '.') {
        i++;
        int j = 0;
        while (name[i] != '\0' && j < 3) {
            fat_name[8 + j] = fat_toupper(name[i]);
            i++;
            j++;
        }
    }
}

static int path_to_fat_name(const char *path, char fat_name[11])
{
    if (path == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (path[0] == '/') {
        path++;
    }
    if (path[0] == '\0' || path[0] == '.' || (uint8_t)path[0] == 0xE5U) {
        return SYN_INVALID_PARAM;
    }
    for (size_t k = 0U; path[k] != '\0'; k++) {
        if (path[k] == '/') {
            return SYN_NOT_FOUND;
        }
        if ((uint8_t)path[k] <= 0x20U) {
            return SYN_INVALID_PARAM;
        }
    }

    format_83_chars(path, fat_name);
    return SYN_OK;
}

static void fat_name_to_cstr(const char fat_name[11], char out[64])
{
    size_t pos = 0U;
    for (size_t i = 0U; i < 8U && fat_name[i] != ' '; i++) {
        out[pos++] = fat_name[i];
    }
    if (fat_name[8] != ' ') {
        out[pos++] = '.';
        for (size_t j = 0U; j < 3U && fat_name[8U + j] != ' '; j++) {
            out[pos++] = fat_name[8U + j];
        }
    }
    out[pos] = '\0';
}

typedef struct {
    uint32_t sector;        
    uint32_t offset;        
    uint32_t start_cluster; 
    uint32_t file_size;     
    uint8_t attr;           
    bool found;             
} DirEntryLoc;

typedef enum {
    DIR_SCAN_CONTINUE = 0, 
    DIR_SCAN_FOUND = 1,    
    DIR_SCAN_END = 2       
} DirScanResult;

static DirScanResult inspect_dir_sector(const FAT_Volume *vol, const uint8_t *sec_buf, uint32_t sec,
                                        const char *fat_name, DirEntryLoc *loc, bool empty)
{
    for (uint32_t off = 0U; off < 512U; off += 32U) {
        uint8_t first_char = sec_buf[off];
        if (first_char == 0x00U && !empty) {
            return DIR_SCAN_END;
        }

        if (empty) {
            if (first_char == 0x00U || first_char == 0xE5U) {
                loc->sector = sec;
                loc->offset = off;
                loc->found = true;
                return DIR_SCAN_FOUND;
            }
        } else if (first_char != 0xE5U && (sec_buf[off + 11U] & 0x08U) == 0U &&
                   memcmp(&sec_buf[off], fat_name, 11) == 0) {
            uint32_t hi = 0U;
            if (vol->is_fat32) {
                hi = (uint32_t)syn_peek_u16_le(&sec_buf[off], 20) << 16;
            }
            loc->sector = sec;
            loc->offset = off;
            loc->start_cluster = (uint32_t)syn_peek_u16_le(&sec_buf[off], 26) | hi;
            loc->file_size = syn_peek_u32_le(&sec_buf[off], 28);
            loc->attr = sec_buf[off + 11U];
            loc->found = true;
            return DIR_SCAN_FOUND;
        }
    }
    return DIR_SCAN_CONTINUE;
}

static bool scan_fat32_root(const FAT_Volume *vol, const char *fat_name, DirEntryLoc *loc,
                            bool empty)
{
    uint8_t sec_buf[512];
    uint32_t cluster = vol->root_cluster;
    uint32_t limit = vol->total_clusters;

    while (is_valid_cluster(vol, cluster) && limit > 0U) {
        uint32_t start_sector = cluster_to_sector(vol, cluster);
        for (uint8_t s = 0U; s < vol->sectors_per_cluster; s++) {
            uint32_t sec = start_sector + s;
            if (syn_sd_read(&g_sd, sec, sec_buf) != SYN_OK) {
                return false;
            }
            DirScanResult res = inspect_dir_sector(vol, sec_buf, sec, fat_name, loc, empty);
            if (res != DIR_SCAN_CONTINUE) {
                return (res == DIR_SCAN_FOUND);
            }
        }
        if (!read_fat_entry(vol, cluster, &cluster)) {
            return false;
        }
        limit--;
    }
    return false;
}

static bool scan_fat16_root(const FAT_Volume *vol, const char *fat_name, DirEntryLoc *loc,
                            bool empty)
{
    uint8_t sec_buf[512];

    for (uint32_t s = 0U; s < vol->root_sectors; s++) {
        uint32_t sec = vol->root_start_sector + s;
        if (syn_sd_read(&g_sd, sec, sec_buf) != SYN_OK) {
            return false;
        }
        DirScanResult res = inspect_dir_sector(vol, sec_buf, sec, fat_name, loc, empty);
        if (res != DIR_SCAN_CONTINUE) {
            return (res == DIR_SCAN_FOUND);
        }
    }
    return false;
}

static bool scan_root_dir(const FAT_Volume *vol, const char *name, DirEntryLoc *loc, bool empty)
{
    loc->found = false;
    if (vol->is_fat32) {
        return scan_fat32_root(vol, name, loc, empty);
    }
    return scan_fat16_root(vol, name, loc, empty);
}

static bool get_cluster_for_offset(const FAT_Volume *vol, uint32_t start_cluster, uint32_t offset,
                                   uint32_t *out_cluster)
{
    uint32_t cluster_size = (uint32_t)vol->sectors_per_cluster * 512U;
    uint32_t target_idx = offset / cluster_size;

    uint32_t curr_cluster = start_cluster;
    for (uint32_t i = 0U; i < target_idx; i++) {
        if (i >= vol->total_clusters) {
            return false;
        }
        uint32_t next = 0U;
        if (!read_fat_entry(vol, curr_cluster, &next) || !is_valid_cluster(vol, next)) {
            return false;
        }
        curr_cluster = next;
    }
    *out_cluster = curr_cluster;
    return true;
}

static int alloc_file_slot(void)
{
    for (int i = 0; i < SYN_VFS_MAX_OPEN_FILES; i++) {
        if (!g_fat_files[i].used) {
            return i;
        }
    }
    return -1;
}

static int fat_create_file(const char *fat_name, DirEntryLoc *loc)
{
    DirEntryLoc empty_slot;
    if (!scan_root_dir(&g_vol, fat_name, &empty_slot, true)) {
        return -3;
    }

    uint8_t sec_buf[512];
    if (syn_sd_read(&g_sd, empty_slot.sector, sec_buf) != SYN_OK) {
        return -6;
    }

    uint8_t *entry = &sec_buf[empty_slot.offset];
    memcpy(entry, fat_name, 11);
    entry[11] = 0x20U;
    memset(entry + 12, 0, 20);

    if (syn_sd_write(&g_sd, empty_slot.sector, sec_buf) != SYN_OK) {
        return -7;
    }

    loc->sector = empty_slot.sector;
    loc->offset = empty_slot.offset;
    loc->start_cluster = 0U;
    loc->file_size = 0U;
    loc->attr = 0x20U;
    loc->found = true;
    return 0;
}

static int fat_truncate_file(SYN_FatFileCtx *ctx)
{
    uint8_t sec_buf[512];
    if (syn_sd_read(&g_sd, ctx->dir_sector, sec_buf) != SYN_OK) {
        return -6;
    }

    uint8_t *entry = &sec_buf[ctx->dir_offset];
    syn_poke_u16_le(0U, entry, 20U);
    syn_poke_u16_le(0U, entry, 26U);
    syn_poke_u32_le(0U, entry, 28U);

    if (syn_sd_write(&g_sd, ctx->dir_sector, sec_buf) != SYN_OK) {
        return -7;
    }

    free_cluster_chain(&g_vol, ctx->start_cluster);
    ctx->start_cluster = 0U;
    ctx->size = 0U;
    ctx->offset = 0U;
    return 0;
}

static int fat_alloc_initial_cluster(SYN_FatFileCtx *ctx)
{
    uint32_t clus = find_free_cluster(&g_vol);
    if (clus == 0U) {
        return -2;
    }
    if (write_fat_entry(&g_vol, clus, 0x0FFFFFFFU) != SYN_OK) {
        return -3;
    }

    uint8_t sec_buf[512];
    if (syn_sd_read(&g_sd, ctx->dir_sector, sec_buf) != SYN_OK) {
        (void)write_fat_entry(&g_vol, clus, 0U);
        return -5;
    }

    uint8_t *entry = &sec_buf[ctx->dir_offset];
    syn_poke_u16_le((uint16_t)(clus >> 16), entry, 20U);
    syn_poke_u16_le((uint16_t)(clus & 0xFFFFU), entry, 26U);
    if (syn_sd_write(&g_sd, ctx->dir_sector, sec_buf) != SYN_OK) {
        (void)write_fat_entry(&g_vol, clus, 0U);
        return -6;
    }

    ctx->start_cluster = clus;
    return 0;
}

/* ── VFS Mappings ───────────────────────────────────────────────────────── */

static bool fat_flags_wants_write(int flags)
{
    return (((uint32_t)flags & 0x03U) != (uint32_t)SYN_O_RDONLY);
}

static int fat_check_open_conflict(const char fat_name[11], int flags)
{
    uint32_t acc = (uint32_t)flags & 0x03U;
    if (acc == 0x03U ||
        (acc == (uint32_t)SYN_O_RDONLY && ((uint32_t)flags & (uint32_t)SYN_O_TRUNC) != 0U)) {
        return SYN_INVALID_PARAM;
    }

    bool req_write = fat_flags_wants_write(flags);
    for (size_t i = 0U; i < (size_t)SYN_VFS_MAX_OPEN_FILES; i++) {
        if (g_fat_files[i].used && memcmp(g_fat_files[i].name, fat_name, 11U) == 0) {
            if (req_write || fat_flags_wants_write(g_fat_files[i].flags)) {
                return SYN_BUSY;
            }
        }
    }
    return SYN_OK;
}

static bool fat_open_attr_allowed(uint8_t attr, int flags)
{
    if ((attr & 0x10U) != 0U) {
        return false;
    }
    if (fat_flags_wants_write(flags) && (attr & 0x01U) != 0U) {
        return false;
    }
    return true;
}

static int syn_fat_vfs_open(SYN_VfsFile *file, const char *path, int flags, void *fs_data)
{
    (void)fs_data;
    if (file == NULL || path == NULL) {
        return -1;
    }

    char fat_name[11];
    int name_rc = path_to_fat_name(path, fat_name);
    if (name_rc != SYN_OK) {
        return name_rc;
    }
    int conf_rc = fat_check_open_conflict(fat_name, flags);
    if (conf_rc != SYN_OK) {
        return conf_rc;
    }

    int f_idx = alloc_file_slot();
    if (f_idx < 0) {
        return -1;
    }

    DirEntryLoc loc;
    if (!scan_root_dir(&g_vol, fat_name, &loc, false)) {
        if ((flags & SYN_O_CREAT) == 0) {
            return -2;
        }
        int create_rc = fat_create_file(fat_name, &loc);
        if (create_rc < 0) {
            return create_rc;
        }
    } else if (!fat_open_attr_allowed(loc.attr, flags)) {
        return SYN_INVALID_PARAM;
    }

    g_fat_files[f_idx].used = true;
    memcpy(g_fat_files[f_idx].name, fat_name, 11);
    g_fat_files[f_idx].start_cluster = loc.start_cluster;
    g_fat_files[f_idx].size = loc.file_size;
    g_fat_files[f_idx].offset = 0U;
    g_fat_files[f_idx].dir_sector = loc.sector;
    g_fat_files[f_idx].dir_offset = loc.offset;
    g_fat_files[f_idx].flags = flags;

    file->fs_file = &g_fat_files[f_idx];

    if ((flags & SYN_O_TRUNC) != 0) {
        int trc = fat_truncate_file(&g_fat_files[f_idx]);
        if (trc < 0) {
            g_fat_files[f_idx].used = false;
            file->fs_file = NULL;
            return trc;
        }
    }

    if ((flags & SYN_O_APPEND) != 0) {
        g_fat_files[f_idx].offset = g_fat_files[f_idx].size;
    }

    return 0;
}

static int syn_fat_vfs_close(SYN_VfsFile *file)
{
    if (file == NULL || file->fs_file == NULL) {
        return -1;
    }
    SYN_FatFileCtx *ctx = (SYN_FatFileCtx *)file->fs_file;
    ctx->used = false;
    file->fs_file = NULL;
    return 0;
}

static size_t fat_clamp_io_len(size_t len, uint32_t max_rem)
{
    if (len > (size_t)INT_MAX) {
        len = (size_t)INT_MAX;
    }
    if (len > (size_t)max_rem) {
        len = (size_t)max_rem;
    }
    return len;
}

static int syn_fat_vfs_read(SYN_VfsFile *file, void *buf, size_t len)
{
    if (file == NULL || file->fs_file == NULL || (len > 0U && buf == NULL)) {
        return -1;
    }
    SYN_FatFileCtx *ctx = (SYN_FatFileCtx *)file->fs_file;
    if (((uint32_t)ctx->flags & 0x03U) == (uint32_t)SYN_O_WRONLY) {
        return SYN_INVALID_PARAM;
    }

    if (len == 0U || ctx->offset >= ctx->size) {
        return 0;
    }
    if (!is_valid_cluster(&g_vol, ctx->start_cluster)) {
        return -2;
    }
    len = fat_clamp_io_len(len, ctx->size - ctx->offset);

    uint8_t sec_buf[512];
    uint32_t bytes_read = 0U;
    uint8_t *dest = (uint8_t *)buf;
    uint32_t cluster_size = (uint32_t)g_vol.sectors_per_cluster * 512U;

    while (bytes_read < len) {
        uint32_t curr_cluster = 0U;
        if (!get_cluster_for_offset(&g_vol, ctx->start_cluster, ctx->offset, &curr_cluster)) {
            return -2;
        }
        uint32_t byte_in_cluster = ctx->offset % cluster_size;
        uint32_t sector_in_cluster = byte_in_cluster / 512U;
        uint32_t byte_in_sector = byte_in_cluster % 512U;

        uint32_t sector = cluster_to_sector(&g_vol, curr_cluster) + sector_in_cluster;
        if (syn_sd_read(&g_sd, sector, sec_buf) != SYN_OK) {
            return -2;
        }

        uint32_t to_copy = 512U - byte_in_sector;
        if (to_copy > (len - bytes_read)) {
            to_copy = (uint32_t)(len - bytes_read);
        }

        memcpy(dest + bytes_read, sec_buf + byte_in_sector, to_copy);
        bytes_read += to_copy;
        ctx->offset += to_copy;
    }

    return (int)bytes_read;
}

static int fat_resolve_write_cluster(uint32_t start_cluster, uint32_t target_idx,
                                     uint32_t *out_cluster)
{
    uint32_t curr_cluster = start_cluster;
    uint32_t eof_val = g_vol.is_fat32 ? 0x0FFFFFF8U : 0xFFF8U;

    for (uint32_t i = 0U; i < target_idx; i++) {
        if (i >= g_vol.total_clusters) {
            return SYN_INVALID_PARAM;
        }
        uint32_t next = 0U;
        if (!read_fat_entry(&g_vol, curr_cluster, &next)) {
            return -5;
        }
        if (next >= eof_val) {
            uint32_t new_cluster = find_free_cluster(&g_vol);
            if (new_cluster == 0U) {
                return -2;
            }
            if (write_fat_entry(&g_vol, curr_cluster, new_cluster) != SYN_OK) {
                return -3;
            }
            if (write_fat_entry(&g_vol, new_cluster, 0x0FFFFFFFU) != SYN_OK) {
                return -4;
            }
            curr_cluster = new_cluster;
        } else {
            if (!is_valid_cluster(&g_vol, next)) {
                return SYN_INVALID_PARAM;
            }
            curr_cluster = next;
        }
    }

    if (!is_valid_cluster(&g_vol, curr_cluster)) {
        return SYN_INVALID_PARAM;
    }

    *out_cluster = curr_cluster;
    return 0;
}

static int fat_write_sector_step(SYN_FatFileCtx *ctx, const uint8_t *src, size_t rem,
                                 uint8_t sec_buf[512], uint32_t *out_chunk)
{
    uint32_t cluster_size = (uint32_t)g_vol.sectors_per_cluster * 512U;
    uint32_t curr_cluster = 0U;
    int rc =
        fat_resolve_write_cluster(ctx->start_cluster, ctx->offset / cluster_size, &curr_cluster);
    if (rc < 0) {
        return rc;
    }

    uint32_t byte_in_cluster = ctx->offset % cluster_size;
    uint32_t sector = cluster_to_sector(&g_vol, curr_cluster) + (byte_in_cluster / 512U);
    uint32_t byte_in_sector = byte_in_cluster % 512U;

    if (byte_in_sector != 0U || rem < 512U) {
        if (syn_sd_read(&g_sd, sector, sec_buf) != SYN_OK) {
            return -5;
        }
    }

    uint32_t to_write = 512U - byte_in_sector;
    if (to_write > rem) {
        to_write = (uint32_t)rem;
    }

    memcpy(sec_buf + byte_in_sector, src, to_write);
    if (syn_sd_write(&g_sd, sector, sec_buf) != SYN_OK) {
        return -6;
    }

    ctx->offset += to_write;
    if (ctx->offset > ctx->size) {
        ctx->size = ctx->offset;
    }
    *out_chunk = to_write;
    return 0;
}

static int fat_sync_dir_size(const SYN_FatFileCtx *ctx, uint8_t sec_buf[512])
{
    if (syn_sd_read(&g_sd, ctx->dir_sector, sec_buf) != SYN_OK) {
        return -7;
    }
    syn_poke_u32_le(ctx->size, sec_buf, ctx->dir_offset + 28U);
    if (syn_sd_write(&g_sd, ctx->dir_sector, sec_buf) != SYN_OK) {
        return -7;
    }
    return 0;
}

static int syn_fat_vfs_write(SYN_VfsFile *file, const void *buf, size_t len)
{
    if (file == NULL || file->fs_file == NULL || (len > 0U && buf == NULL)) {
        return -1;
    }
    SYN_FatFileCtx *ctx = (SYN_FatFileCtx *)file->fs_file;
    if (((uint32_t)ctx->flags & 0x03U) == (uint32_t)SYN_O_RDONLY) {
        return SYN_INVALID_PARAM;
    }
    if (((uint32_t)ctx->flags & (uint32_t)SYN_O_APPEND) != 0U) {
        ctx->offset = ctx->size;
    }
    len = fat_clamp_io_len(len, UINT32_MAX - ctx->offset);
    if (len == 0U) {
        return 0;
    }
    if (ctx->start_cluster == 0U) {
        int alloc_rc = fat_alloc_initial_cluster(ctx);
        if (alloc_rc < 0) {
            return alloc_rc;
        }
    }

    uint8_t sec_buf[512];
    uint32_t bytes_written = 0U;
    const uint8_t *src = (const uint8_t *)buf;

    while (bytes_written < len) {
        uint32_t chunk = 0U;
        int rc =
            fat_write_sector_step(ctx, src + bytes_written, len - bytes_written, sec_buf, &chunk);
        if (rc < 0) {
            return rc;
        }
        bytes_written += chunk;
    }

    int sync_rc = fat_sync_dir_size(ctx, sec_buf);
    if (sync_rc < 0) {
        return sync_rc;
    }

    return (int)bytes_written;
}

static int32_t syn_fat_vfs_seek(SYN_VfsFile *file, int32_t offset, int whence)
{
    if (file == NULL || file->fs_file == NULL) {
        return -1;
    }
    SYN_FatFileCtx *ctx = (SYN_FatFileCtx *)file->fs_file;

    int64_t target_offset = 0;
    if (whence == SYN_SEEK_SET) {
        target_offset = (int64_t)offset;
    } else if (whence == SYN_SEEK_CUR) {
        target_offset = (int64_t)ctx->offset + (int64_t)offset;
    } else if (whence == SYN_SEEK_END) {
        target_offset = (int64_t)ctx->size + (int64_t)offset;
    } else {
        return -2;
    }

    if (target_offset < 0) {
        target_offset = 0;
    }
    if (target_offset > (int64_t)ctx->size) {
        target_offset = (int64_t)ctx->size;
    }
    if (target_offset > (int64_t)INT32_MAX) {
        return -2;
    }

    ctx->offset = (uint32_t)target_offset;
    return (int32_t)ctx->offset;
}

static int32_t syn_fat_vfs_tell(SYN_VfsFile *file)
{
    if (file == NULL || file->fs_file == NULL) {
        return -1;
    }
    const SYN_FatFileCtx *ctx = (const SYN_FatFileCtx *)file->fs_file;
    if (ctx->offset > (uint32_t)INT32_MAX) {
        return -2;
    }
    return (int32_t)ctx->offset;
}

static int syn_fat_vfs_unlink(const char *path, void *fs_data)
{
    (void)fs_data;
    if (path == NULL) {
        return -1;
    }

    char fat_name[11];
    int name_rc = path_to_fat_name(path, fat_name);
    if (name_rc != SYN_OK) {
        return name_rc;
    }
    if (fat_is_name_open(fat_name)) {
        return SYN_BUSY;
    }

    DirEntryLoc loc;
    if (!scan_root_dir(&g_vol, fat_name, &loc, false)) {
        return -1;
    }
    if ((loc.attr & 0x11U) != 0U) {
        return SYN_INVALID_PARAM;
    }

    uint8_t sec_buf[512];
    if (syn_sd_read(&g_sd, loc.sector, sec_buf) != SYN_OK) {
        return -2;
    }
    sec_buf[loc.offset] = 0xE5U;
    if (syn_sd_write(&g_sd, loc.sector, sec_buf) != SYN_OK) {
        return -3;
    }

    free_cluster_chain(&g_vol, loc.start_cluster);
    return 0;
}

static int syn_fat_vfs_stat(const char *path, SYN_VfsDirEnt *ent, void *fs_data)
{
    (void)fs_data;
    if (ent == NULL) {
        return SYN_INVALID_PARAM;
    }

    char fat_name[11];
    int name_rc = path_to_fat_name(path, fat_name);
    if (name_rc != SYN_OK) {
        return name_rc;
    }

    DirEntryLoc loc;
    if (!scan_root_dir(&g_vol, fat_name, &loc, false)) {
        return SYN_NOT_FOUND;
    }

    memset(ent, 0, sizeof(*ent));
    fat_name_to_cstr(fat_name, ent->name);
    ent->size = loc.file_size;
    ent->is_dir = (loc.attr & 0x10U) != 0U;
    return 0;
}

static int syn_fat_vfs_rename(const char *old_path, const char *new_path, void *fs_data)
{
    char old_name[11];
    char new_name[11];
    int rc = path_to_fat_name(old_path, old_name);
    if (rc != SYN_OK) {
        return rc;
    }
    rc = path_to_fat_name(new_path, new_name);
    if (rc != SYN_OK) {
        return rc;
    }

    DirEntryLoc old_loc;
    if (!scan_root_dir(&g_vol, old_name, &old_loc, false)) {
        return SYN_NOT_FOUND;
    }
    if ((old_loc.attr & 0x11U) != 0U) {
        return SYN_INVALID_PARAM;
    }
    if (memcmp(old_name, new_name, 11U) == 0) {
        return 0;
    }
    if (fat_is_name_open(old_name) || fat_is_name_open(new_name)) {
        return SYN_BUSY;
    }

    DirEntryLoc dst_loc;
    if (scan_root_dir(&g_vol, new_name, &dst_loc, false)) {
        int urc = syn_fat_vfs_unlink(new_path, fs_data);
        if (urc != 0) {
            return urc;
        }
    }

    uint8_t sec_buf[512];
    if (syn_sd_read(&g_sd, old_loc.sector, sec_buf) != SYN_OK) {
        return -2;
    }
    memcpy(&sec_buf[old_loc.offset], new_name, 11U);
    if (syn_sd_write(&g_sd, old_loc.sector, sec_buf) != SYN_OK) {
        return -3;
    }

    return 0;
}

static const SYN_VfsOps g_fat_vfs_ops = {.open = syn_fat_vfs_open,
                                         .close = syn_fat_vfs_close,
                                         .read = syn_fat_vfs_read,
                                         .write = syn_fat_vfs_write,
                                         .seek = syn_fat_vfs_seek,
                                         .tell = syn_fat_vfs_tell,
                                         .stat = syn_fat_vfs_stat,
                                         .rename = syn_fat_vfs_rename,
                                         .unlink = syn_fat_vfs_unlink,
                                         .mkdir = NULL,
                                         .opendir = NULL,
                                         .readdir = NULL,
                                         .closedir = NULL};

const SYN_VfsOps *syn_fat_get_ops(void)
{
    return &g_fat_vfs_ops;
}

SYN_Status syn_fat_init(uint8_t spi_bus, SYN_GPIO_Pin cs)
{
    if (syn_sd_init(&g_sd, spi_bus, cs) != SYN_OK) {
        SYN_LOG_E(TAG, "%s", "syn_sd_init failed");
        return SYN_ERROR;
    }

    uint8_t sec_buf[512];
    if (syn_sd_read(&g_sd, 0U, sec_buf) != SYN_OK) {
        SYN_LOG_E(TAG, "%s", "Failed to read Sector 0");
        return SYN_ERROR;
    }

    uint32_t part_start = find_partition_start(sec_buf);
    if (part_start != 0U) {
        SYN_LOG_I(TAG, "FAT partition found starting at sector %u", (unsigned)part_start);
        if (syn_sd_read(&g_sd, part_start, sec_buf) != SYN_OK) {
            SYN_LOG_E(TAG, "%s", "Failed to read partition Boot Sector");
            return SYN_ERROR;
        }
    }

    if (!fat_parse_bpb(&g_vol, sec_buf, part_start)) {
        SYN_LOG_E(TAG, "%s", "Invalid Boot Sector BPB format");
        return SYN_ERROR;
    }

    SYN_LOG_I(TAG, "Mounted %s filesystem: sectors/cluster=%u reserved=%u",
              g_vol.is_fat32 ? "FAT32" : "FAT16", (unsigned)g_vol.sectors_per_cluster,
              (unsigned)g_vol.reserved_sectors);

    memset(g_fat_files, 0, sizeof(g_fat_files));

    if (syn_vfs_mount("/sd", &g_fat_vfs_ops, NULL) != SYN_OK) {
        SYN_LOG_E(TAG, "%s", "VFS mount /sd failed");
        return SYN_ERROR;
    }

    return SYN_OK;
}

#endif /* SYN_USE_FAT */
```


