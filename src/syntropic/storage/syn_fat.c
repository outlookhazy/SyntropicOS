#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_FAT) || SYN_USE_FAT

#if defined(SYN_USE_VFS) && !SYN_USE_VFS
#error "syn_fat requires SYN_USE_VFS=1"
#endif

/**
 * @file syn_fat.c
 * @brief Custom lightweight FAT16/FAT32 VFS adapter implementation.
 */

#include "../log/syn_log.h"
#include "../util/syn_pack.h"
#include "syn_fat.h"

#include <string.h>

/** @brief Logging tag for the FAT filesystem adapter. */
#define TAG "syn_fat"

/* ── Custom FAT volume context ──────────────────────────────────────────── */

/** @brief Internal FAT volume metadata parsed from the boot sector. */
typedef struct {
    uint16_t bytes_per_sector;   /**< Bytes per logical sector            */
    uint8_t sectors_per_cluster; /**< Sectors per allocation cluster      */
    uint16_t reserved_sectors;   /**< Reserved sectors before first FAT   */
    uint8_t num_fats;            /**< Number of FAT copies                */
    uint16_t fat16_root_entries; /**< Root dir entry count (FAT16 only)   */
    uint32_t fat_sectors;        /**< Sectors occupied by each FAT        */
    uint32_t root_cluster;       /**< Root directory cluster (FAT32 only) */
    bool is_fat32;               /**< true if FAT32, false if FAT16       */

    /* Calculated offsets (in sectors) */
    uint32_t fat_start_sector;  /**< First sector of FAT                 */
    uint32_t root_start_sector; /**< First sector of root dir (FAT16)    */
    uint32_t root_sectors;      /**< Sectors for root dir (FAT16)        */
    uint32_t data_start_sector; /**< First sector of data region         */
    uint32_t total_clusters;    /**< Total allocatable data clusters     */
} FAT_Volume;

/** @brief Mounted FAT volume state singleton. */
static FAT_Volume g_vol;

/** @brief Underlying SPI SD card driver instance. */
static SYN_SD g_sd;

/* ── Open file context pool ─────────────────────────────────────────────── */

/** @brief Open file tracking context (pooled). */
typedef struct {
    char name[11];          /**< 8.3 FAT directory name                 */
    uint32_t start_cluster; /**< First cluster of file data              */
    uint32_t size;          /**< File size in bytes                      */
    uint32_t offset;        /**< Current read/write position             */
    uint32_t dir_sector;    /**< Sector containing the dir entry         */
    uint32_t dir_offset;    /**< Byte offset of dir entry within sector  */
    int flags;              /**< Open flags (SYN_O_*)                    */
    bool used;              /**< true if this slot is in use             */
} SYN_FatFileCtx;

/** @brief Static pool of open FAT file contexts. */
static SYN_FatFileCtx g_fat_files[SYN_VFS_MAX_OPEN_FILES];

/* ── Helper Functions ───────────────────────────────────────────────────── */

/**
 * @brief Check whether an MBR partition type code corresponds to FAT16/FAT32.
 * @param type MBR partition type byte.
 * @return true if recognized FAT partition type, false otherwise.
 */
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

/**
 * @brief Inspect sector 0 for an MBR partition table entry and return its LBA start.
 * @param sector0 512-byte buffer containing LBA 0.
 * @return Partition start sector, or 0 if sector 0 is a superfloppy or invalid.
 */
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

/**
 * @brief Compute derived sector offsets and total allocatable clusters for @p vol.
 * @param vol          [in,out] Volume metadata structure.
 * @param bpb          512-byte boot sector buffer.
 * @param volume_start LBA sector offset of the volume boot sector.
 * @return true if geometry and cluster count are valid, false otherwise.
 */
static bool fat_compute_layout(FAT_Volume *vol, const uint8_t *bpb, uint32_t volume_start)
{
    vol->fat_start_sector = volume_start + vol->reserved_sectors;

    if (vol->is_fat32) {
        vol->root_start_sector = 0U;
        vol->root_sectors = 0U;
        vol->data_start_sector = vol->fat_start_sector + (vol->num_fats * vol->fat_sectors);
    } else {
        vol->root_start_sector = vol->fat_start_sector + (vol->num_fats * vol->fat_sectors);
        vol->root_sectors = (((uint32_t)vol->fat16_root_entries * 32U) + 511U) / 512U;
        vol->data_start_sector = vol->root_start_sector + vol->root_sectors;
    }

    uint32_t total_sec = (uint32_t)syn_peek_u16_le(bpb, 19);
    if (total_sec == 0U) {
        total_sec = syn_peek_u32_le(bpb, 32);
    }

    uint32_t non_data_sec = vol->data_start_sector - volume_start;
    if (total_sec <= non_data_sec || vol->fat_sectors == 0U || vol->num_fats == 0U) {
        return false;
    }

    vol->total_clusters = (total_sec - non_data_sec) / (uint32_t)vol->sectors_per_cluster;
    uint32_t fat_cap = (vol->fat_sectors * 512U) / (vol->is_fat32 ? 4U : 2U);
    if (vol->total_clusters == 0U || fat_cap <= 2U) {
        return false;
    }
    if (vol->total_clusters > (fat_cap - 2U)) {
        vol->total_clusters = fat_cap - 2U;
    }

    return true;
}

/**
 * @brief Parse FAT16/FAT32 BIOS Parameter Block (BPB) from a volume boot sector.
 * @param vol          [out] Volume metadata structure to populate.
 * @param bpb          512-byte boot sector buffer.
 * @param volume_start LBA sector offset of the volume boot sector.
 * @return true if valid 512-byte-sector FAT16/FAT32 BPB, false otherwise.
 */
static bool fat_parse_bpb(FAT_Volume *vol, const uint8_t *bpb, uint32_t volume_start)
{
    if (bpb[510] != 0x55U || bpb[511] != 0xAAU) {
        return false;
    }

    vol->bytes_per_sector = syn_peek_u16_le(bpb, 11);
    if (vol->bytes_per_sector != 512U) {
        return false;
    }

    vol->sectors_per_cluster = bpb[13];
    if (vol->sectors_per_cluster == 0U) {
        return false;
    }

    vol->reserved_sectors = syn_peek_u16_le(bpb, 14);
    vol->num_fats = bpb[16];
    vol->fat16_root_entries = syn_peek_u16_le(bpb, 17);

    uint16_t spf16 = syn_peek_u16_le(bpb, 22);
    if (spf16 != 0U) {
        vol->fat_sectors = spf16;
        vol->is_fat32 = false;
        vol->root_cluster = 0U;
    } else {
        vol->fat_sectors = syn_peek_u32_le(bpb, 36);
        vol->root_cluster = syn_peek_u32_le(bpb, 44);
        vol->is_fat32 = true;
    }

    return fat_compute_layout(vol, bpb, volume_start);
}

/**
 * @brief Check whether @p cluster is within the volume's allocatable cluster range.
 * @param vol     Mounted FAT volume metadata.
 * @param cluster Cluster index to validate.
 * @return true if @p cluster is in [2, total_clusters + 1].
 */
static bool is_valid_cluster(const FAT_Volume *vol, uint32_t cluster)
{
    return (cluster >= 2U && cluster < (vol->total_clusters + 2U));
}

/**
 * @brief Convert a valid data cluster index (>= 2) to its first LBA sector.
 * @param vol     Mounted FAT volume metadata.
 * @param cluster Cluster number (>= 2).
 * @return First LBA sector of the cluster.
 */
static uint32_t cluster_to_sector(const FAT_Volume *vol, uint32_t cluster)
{
    return vol->data_start_sector + ((cluster - 2U) * vol->sectors_per_cluster);
}

/**
 * @brief Compute FAT table sector and byte offset within that sector for a cluster.
 * @param vol        Mounted FAT volume metadata.
 * @param cluster    Cluster number.
 * @param fat_sector [out] LBA sector in primary FAT.
 * @param offset     [out] Byte offset within the 512-byte FAT sector.
 */
static void fat_entry_location(const FAT_Volume *vol, uint32_t cluster, uint32_t *fat_sector,
                               uint32_t *offset)
{
    uint32_t byte_off = vol->is_fat32 ? (cluster * 4U) : (cluster * 2U);
    *fat_sector = vol->fat_start_sector + (byte_off / 512U);
    *offset = byte_off % 512U;
}

/**
 * @brief Read the FAT table entry for a given cluster.
 * @param vol     Mounted FAT volume metadata.
 * @param cluster   Cluster index to query.
 * @param out_entry [out] Next cluster index, 0 if free, or 0x0FFFFFFF on EOF.
 * @return true on success, false on SD sector read error.
 */
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

/**
 * @brief Write a FAT table entry (preserving FAT32 top 4 bits and updating FAT2).
 * @param vol     Mounted FAT volume metadata.
 * @param cluster Cluster index to update.
 * @param value   New cluster chain value (0 for free, 0x0FFFFFFF for EOF).
 * @return SYN_OK on success, SYN_INVALID_PARAM if cluster out of range, SYN_ERROR on I/O failure.
 */
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

/**
 * @brief Scan the FAT for the first unallocated cluster within [2, total_clusters + 1].
 * @param vol Mounted FAT volume metadata.
 * @return Free cluster number (>= 2), or 0 if none available.
 */
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

/**
 * @brief Free an entire cluster chain starting at @p start_cluster.
 * @param vol           Mounted FAT volume metadata.
 * @param start_cluster First cluster in the chain (no-op if < 2).
 */
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

/**
 * @brief Convert an ASCII lowercase letter to uppercase.
 * @param c Input character.
 * @return Uppercase equivalent if 'a'..'z', else @p c unchanged.
 */
static char fat_toupper(char c)
{
    if (c >= 'a' && c <= 'z') {
        return (char)(c - 'a' + 'A');
    }
    return c;
}

/**
 * @brief Encode a single validated filename component into an 11-byte 8.3 buffer.
 * @param name     Validated filename without slashes.
 * @param fat_name [out] 11-byte space-padded 8.3 name buffer.
 */
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

/**
 * @brief Validate a root-relative VFS path and convert it into an 11-byte 8.3 FAT name.
 * @param path     Relative file path (with or without single leading '/').
 * @param fat_name [out] 11-byte 8.3 name buffer.
 * @return SYN_OK on success, SYN_INVALID_PARAM on empty/dot path, SYN_NOT_FOUND on subdirectory.
 */
static int path_to_fat_name(const char *path, char fat_name[11])
{
    if (path == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (path[0] == '/') {
        path++;
    }
    if (path[0] == '\0' || path[0] == '.') {
        return SYN_INVALID_PARAM;
    }
    if (strchr(path, '/') != NULL) {
        return SYN_NOT_FOUND;
    }

    format_83_chars(path, fat_name);
    return SYN_OK;
}

/**
 * @brief Convert an 11-byte space-padded 8.3 FAT name into a NUL-terminated C string.
 * @param fat_name 11-byte 8.3 FAT name.
 * @param out      [out] Output buffer (at least 13 bytes).
 */
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

/** @brief Result of a directory scan — location of a matching or free entry. */
typedef struct {
    uint32_t sector;        /**< Sector containing the entry              */
    uint32_t offset;        /**< Byte offset within that sector           */
    uint32_t start_cluster; /**< First cluster of the found file          */
    uint32_t file_size;     /**< Size in bytes of the found file          */
    uint8_t attr;           /**< FAT directory attribute byte             */
    bool found;             /**< true if a matching entry was found       */
} DirEntryLoc;

/** @brief Per-sector directory inspection outcome. */
typedef enum {
    DIR_SCAN_CONTINUE = 0, /**< No match in this sector; keep scanning   */
    DIR_SCAN_FOUND = 1,    /**< Matching entry or free slot found        */
    DIR_SCAN_END = 2       /**< End-of-directory marker (0x00) reached   */
} DirScanResult;

/**
 * @brief Inspect the 16 directory entries within a single 512-byte sector.
 * @param vol      Mounted FAT volume metadata.
 * @param sec_buf  512-byte directory sector buffer.
 * @param sec      LBA sector number of @p sec_buf.
 * @param fat_name 11-byte target 8.3 name (ignored if @p empty is true).
 * @param loc      [out] Populated on match.
 * @param empty    true to locate a free/deleted slot, false to match @p fat_name.
 * @return DIR_SCAN_FOUND, DIR_SCAN_END, or DIR_SCAN_CONTINUE.
 */
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

/**
 * @brief Scan a FAT32 cluster-chained root directory.
 * @param vol      Mounted FAT volume metadata.
 * @param fat_name 11-byte 8.3 filename.
 * @param loc      [out] Resulting entry location.
 * @param empty    true to locate a free slot, false to find @p fat_name.
 * @return true if found, false otherwise.
 */
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

/**
 * @brief Scan a FAT16 fixed-sector root directory region.
 * @param vol      Mounted FAT volume metadata.
 * @param fat_name 11-byte 8.3 filename.
 * @param loc      [out] Resulting entry location.
 * @param empty    true to locate a free slot, false to find @p fat_name.
 * @return true if found, false otherwise.
 */
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

/**
 * @brief Scan the volume root directory for a matching 8.3 entry or empty slot.
 * @param vol   Mounted FAT volume metadata.
 * @param name  11-byte 8.3 filename.
 * @param loc   [out] Entry location descriptor.
 * @param empty true to search for an unused slot, false to match @p name.
 * @return true if found, false otherwise.
 */
static bool scan_root_dir(const FAT_Volume *vol, const char *name, DirEntryLoc *loc, bool empty)
{
    loc->found = false;
    if (vol->is_fat32) {
        return scan_fat32_root(vol, name, loc, empty);
    }
    return scan_fat16_root(vol, name, loc, empty);
}

/**
 * @brief Traverse a file's cluster chain to the cluster covering @p offset.
 * @param vol           Mounted FAT volume metadata.
 * @param start_cluster First cluster of the file.
 * @param offset        Byte offset within the file.
 * @param out_cluster   [out] Cluster index covering @p offset (or last valid cluster on EOF).
 * @return true on success, false on SD FAT read failure.
 */
static bool get_cluster_for_offset(const FAT_Volume *vol, uint32_t start_cluster, uint32_t offset,
                                   uint32_t *out_cluster)
{
    uint32_t cluster_size = (uint32_t)vol->sectors_per_cluster * 512U;
    uint32_t target_idx = offset / cluster_size;
    uint32_t eof_val = vol->is_fat32 ? 0x0FFFFFF8U : 0xFFF8U;

    uint32_t curr_cluster = start_cluster;
    for (uint32_t i = 0U; i < target_idx; i++) {
        uint32_t next = 0U;
        if (!read_fat_entry(vol, curr_cluster, &next)) {
            return false;
        }
        if (next >= eof_val) {
            *out_cluster = curr_cluster;
            return true;
        }
        curr_cluster = next;
    }
    *out_cluster = curr_cluster;
    return true;
}

/**
 * @brief Allocate an unused file context slot from the static pool.
 * @return Pool index (0 .. SYN_VFS_MAX_OPEN_FILES - 1), or -1 if full.
 */
static int alloc_file_slot(void)
{
    for (int i = 0; i < SYN_VFS_MAX_OPEN_FILES; i++) {
        if (!g_fat_files[i].used) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Create a new empty 0-byte 8.3 file entry in the root directory (start_cluster = 0).
 * @param fat_name 11-byte 8.3 filename.
 * @param loc      [out] Populated with the newly created directory entry metadata.
 * @return 0 on success, or negative error code (-3, -6, -7).
 */
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

/**
 * @brief Truncate an open file to 0 bytes, resetting directory cluster fields and freeing chain.
 * @param ctx Open file context.
 * @return 0 on success, or negative error code (-6, -7).
 */
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

/**
 * @brief Allocate the first data cluster (>= 2) for a 0-byte file and persist to directory entry.
 * @param ctx Open file context whose start_cluster is 0.
 * @return 0 on success, or negative error code (-2, -3, -5, -6).
 */
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

/**
 * @brief Check whether @p flags are permitted for an existing directory entry's FAT attributes.
 * @param attr  FAT directory attribute byte.
 * @param flags Requested SYN_O_* open flags.
 * @return true if opening is allowed, false if directory or read-only violation.
 */
static bool fat_open_attr_allowed(uint8_t attr, int flags)
{
    if ((attr & 0x10U) != 0U) {
        return false;
    }
    bool wants_write = (((uint32_t)flags & 0x03U) != (uint32_t)SYN_O_RDONLY) ||
                       (((uint32_t)flags & (uint32_t)SYN_O_TRUNC) != 0U);
    if (wants_write && (attr & 0x01U) != 0U) {
        return false;
    }
    return true;
}

/**
 * @brief VFS open callback for FAT16/FAT32 files.
 * @param file    VFS file descriptor.
 * @param path    Mount-relative file path.
 * @param flags   Open flags (SYN_O_*).
 * @param fs_data Unused filesystem context pointer.
 * @return 0 on success, or negative error code.
 */
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

/**
 * @brief VFS close callback releasing the pooled FAT file context.
 * @param file VFS file descriptor.
 * @return 0 on success, -1 if invalid.
 */
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

/**
 * @brief VFS read callback reading sequential bytes from a FAT file.
 * @param file VFS file descriptor.
 * @param buf  Destination buffer.
 * @param len  Maximum bytes to read.
 * @return Bytes read (>= 0), or negative error code.
 */
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
    if (ctx->offset + len > ctx->size) {
        len = ctx->size - ctx->offset;
    }

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

/**
 * @brief Traverse or extend a file's cluster chain to reach cluster index @p target_idx.
 * @param start_cluster First cluster of the file.
 * @param target_idx    Zero-based sequential cluster offset within the file.
 * @param out_cluster   [out] Resolved or newly allocated cluster number.
 * @return 0 on success, or negative error code (-2 .. -5).
 */
static int fat_resolve_write_cluster(uint32_t start_cluster, uint32_t target_idx,
                                     uint32_t *out_cluster)
{
    uint32_t curr_cluster = start_cluster;
    uint32_t eof_val = g_vol.is_fat32 ? 0x0FFFFFF8U : 0xFFF8U;

    for (uint32_t i = 0U; i < target_idx; i++) {
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
            curr_cluster = next;
        }
    }

    if (!is_valid_cluster(&g_vol, curr_cluster)) {
        return SYN_INVALID_PARAM;
    }

    *out_cluster = curr_cluster;
    return 0;
}

/**
 * @brief Write a single sector slice during sequential file write.
 * @param ctx        Open file context.
 * @param src        Source data pointer for the current chunk.
 * @param rem        Remaining bytes to write in the call.
 * @param sec_buf    Caller-owned 512-byte sector scratch buffer.
 * @param out_chunk  [out] Bytes written in this step.
 * @return 0 on success, or negative error code.
 */
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

/**
 * @brief VFS write callback writing sequential bytes and extending clusters as needed.
 * @param file VFS file descriptor.
 * @param buf  Source buffer.
 * @param len  Number of bytes to write.
 * @return Bytes written (>= 0), or negative error code.
 */
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

    if (syn_sd_read(&g_sd, ctx->dir_sector, sec_buf) != SYN_OK) {
        return -7;
    }
    syn_poke_u32_le(ctx->size, sec_buf, ctx->dir_offset + 28U);
    if (syn_sd_write(&g_sd, ctx->dir_sector, sec_buf) != SYN_OK) {
        return -7;
    }

    return (int)bytes_written;
}

/**
 * @brief VFS seek callback updating the current read/write cursor.
 * @param file   VFS file descriptor.
 * @param offset Signed byte offset relative to @p whence.
 * @param whence SYN_SEEK_SET, SYN_SEEK_CUR, or SYN_SEEK_END.
 * @return New absolute offset, or negative error code.
 */
static int32_t syn_fat_vfs_seek(SYN_VfsFile *file, int32_t offset, int whence)
{
    if (file == NULL || file->fs_file == NULL) {
        return -1;
    }
    SYN_FatFileCtx *ctx = (SYN_FatFileCtx *)file->fs_file;

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

    if (target_offset < 0) {
        target_offset = 0;
    }
    if ((uint32_t)target_offset > ctx->size) {
        target_offset = (int32_t)ctx->size;
    }

    ctx->offset = (uint32_t)target_offset;
    return (int32_t)ctx->offset;
}

/**
 * @brief VFS tell callback returning the current file offset.
 * @param file VFS file descriptor.
 * @return Current byte offset, or -1 if invalid.
 */
static int32_t syn_fat_vfs_tell(SYN_VfsFile *file)
{
    if (file == NULL || file->fs_file == NULL) {
        return -1;
    }
    const SYN_FatFileCtx *ctx = (const SYN_FatFileCtx *)file->fs_file;
    return (int32_t)ctx->offset;
}

/**
 * @brief VFS unlink callback marking a directory entry deleted and freeing its cluster chain.
 * @param path    Mount-relative file path.
 * @param fs_data Unused filesystem context pointer.
 * @return 0 on success, or negative error code.
 */
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

/**
 * @brief VFS stat callback retrieving 8.3 entry metadata from the root directory.
 * @param path    Mount-relative file path.
 * @param ent     [out] Directory entry metadata.
 * @param fs_data Unused filesystem context pointer.
 * @return 0 on success, or negative error code.
 */
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

/**
 * @brief VFS rename callback renaming an 8.3 root directory entry in place.
 * @param old_path Mount-relative source path.
 * @param new_path Mount-relative destination path.
 * @param fs_data  Unused filesystem context pointer.
 * @return 0 on success, or negative error code.
 */
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

/** @brief Static VFS operations table for the FAT16/FAT32 adapter. */
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
