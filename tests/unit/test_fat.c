/**
 * @file test_fat.c
 * @brief Unit tests for syn_fat FAT16/FAT32 VFS filesystem adapter.
 */

#include "mocks/mock_port.h"
#include "syntropic/storage/syn_fat.h"
#include "syntropic/storage/syn_vfs.h"
#include "syntropic/util/syn_pack.h"
#include "unity/unity.h"

#include <string.h>

#define DISK_SECTORS 32U
#define SECTOR_BYTES 512U
#define SD_RX_FIFO_MAX 600U

static uint8_t s_disk[DISK_SECTORS][SECTOR_BYTES];
static uint32_t s_sparse_lba[2];
static uint8_t s_sparse_sec[2][SECTOR_BYTES];
static uint8_t s_rx_fifo[SD_RX_FIFO_MAX];
static size_t s_rx_head;
static size_t s_rx_tail;
static bool s_write_pending;
static uint32_t s_write_sector;
static uint32_t s_sd_read_calls;
static uint32_t s_sd_write_calls;
static uint32_t s_fail_read_at;
static uint32_t s_fail_write_at;

static void rx_fifo_clear(void)
{
    s_rx_head = 0U;
    s_rx_tail = 0U;
}

static void rx_fifo_push(uint8_t b)
{
    if (s_rx_tail < SD_RX_FIFO_MAX) {
        s_rx_fifo[s_rx_tail++] = b;
    }
}

static void rx_fifo_push_buf(const uint8_t *buf, size_t len)
{
    for (size_t i = 0U; i < len; i++) {
        rx_fifo_push(buf[i]);
    }
}

static SYN_Status sd_spi_emulator(uint8_t bus, const uint8_t *tx_buf, uint8_t *rx_buf, size_t len)
{
    (void)bus;

    if (s_write_pending && rx_buf == NULL && tx_buf != NULL && len == SECTOR_BYTES) {
        if (s_write_sector < DISK_SECTORS) {
            memcpy(s_disk[s_write_sector], tx_buf, SECTOR_BYTES);
        } else {
            for (size_t k = 0U; k < 2U; k++) {
                if (s_sparse_lba[k] != 0U && s_write_sector == s_sparse_lba[k]) {
                    memcpy(s_sparse_sec[k], tx_buf, SECTOR_BYTES);
                }
            }
        }
        s_write_pending = false;
        rx_fifo_clear();
        rx_fifo_push(0xFFU);
        rx_fifo_push(0xFFU);
        rx_fifo_push(0x05U);
        rx_fifo_push(0xFFU);
        return SYN_OK;
    }

    if (rx_buf == NULL && tx_buf != NULL && len == 6U && (tx_buf[0] & 0xC0U) == 0x40U) {
        uint8_t cmd = tx_buf[0] & 0x3FU;
        uint32_t arg = ((uint32_t)tx_buf[1] << 24) | ((uint32_t)tx_buf[2] << 16) |
                       ((uint32_t)tx_buf[3] << 8) | (uint32_t)tx_buf[4];
        rx_fifo_clear();
        s_write_pending = false;

        if (cmd == 0U) {
            rx_fifo_push(0x01U);
        } else if (cmd == 8U) {
            const uint8_t r7[] = {0x01U, 0x00U, 0x00U, 0x01U, 0xAAU};
            rx_fifo_push_buf(r7, sizeof(r7));
        } else if (cmd == 55U) {
            rx_fifo_push(0x01U);
        } else if (cmd == 41U) {
            rx_fifo_push(0x00U);
        } else if (cmd == 58U) {
            const uint8_t ocr[] = {0x00U, 0x40U, 0xFFU, 0x80U, 0x00U};
            rx_fifo_push_buf(ocr, sizeof(ocr));
        } else if (cmd == 9U) {
            uint8_t csd[20];
            memset(csd, 0, sizeof(csd));
            csd[0] = 0x00U;
            csd[1] = 0xFEU;
            csd[2] = 0x40U;
            csd[11] = 0x01U;
            csd[18] = 0xFFU;
            csd[19] = 0xFFU;
            rx_fifo_push_buf(csd, sizeof(csd));
        } else if (cmd == 17U) {
            s_sd_read_calls++;
            if (s_fail_read_at > 0U && s_sd_read_calls == s_fail_read_at) {
                rx_fifo_push(0x04U);
            } else {
                rx_fifo_push(0x00U);
                rx_fifo_push(0xFEU);
                if (arg < DISK_SECTORS) {
                    rx_fifo_push_buf(s_disk[arg], SECTOR_BYTES);
                } else if (s_sparse_lba[0] != 0U && arg == s_sparse_lba[0]) {
                    rx_fifo_push_buf(s_sparse_sec[0], SECTOR_BYTES);
                } else if (s_sparse_lba[1] != 0U && arg == s_sparse_lba[1]) {
                    rx_fifo_push_buf(s_sparse_sec[1], SECTOR_BYTES);
                } else {
                    for (size_t z = 0U; z < SECTOR_BYTES; z++) {
                        rx_fifo_push(0x00U);
                    }
                }
                rx_fifo_push(0xFFU);
                rx_fifo_push(0xFFU);
            }
        } else if (cmd == 24U) {
            s_sd_write_calls++;
            if (s_fail_write_at > 0U && s_sd_write_calls == s_fail_write_at) {
                rx_fifo_push(0x04U);
            } else {
                rx_fifo_push(0x00U);
                s_write_pending = true;
                s_write_sector = arg;
            }
        }
        return SYN_OK;
    }

    if (rx_buf != NULL) {
        for (size_t i = 0U; i < len; i++) {
            if (s_rx_head < s_rx_tail) {
                rx_buf[i] = s_rx_fifo[s_rx_head++];
            } else {
                rx_buf[i] = 0xFFU;
            }
        }
    }
    return SYN_OK;
}

static void sd_emu_reset_counters(void)
{
    s_sd_read_calls = 0U;
    s_sd_write_calls = 0U;
    s_fail_read_at = 0U;
    s_fail_write_at = 0U;
    s_write_pending = false;
    memset(s_sparse_lba, 0, sizeof(s_sparse_lba));
    memset(s_sparse_sec, 0, sizeof(s_sparse_sec));
    rx_fifo_clear();
}

static void format_fat16_superfloppy(uint8_t num_fats)
{
    memset(s_disk, 0, sizeof(s_disk));
    sd_emu_reset_counters();
    syn_vfs_init();
    mock_spi_set_transfer_callback(sd_spi_emulator);

    uint8_t *bpb = s_disk[0];
    syn_poke_u16_le(512U, bpb, 11);
    bpb[13] = 1U; /* 1 sector per cluster */
    syn_poke_u16_le(1U, bpb, 14);
    bpb[16] = num_fats;
    syn_poke_u16_le(16U, bpb, 17);          /* 16 root entries = 1 sector */
    syn_poke_u16_le(DISK_SECTORS, bpb, 19); /* total 16-bit sectors */
    syn_poke_u16_le(1U, bpb, 22);           /* 1 sector per FAT */
    bpb[510] = 0x55U;
    bpb[511] = 0xAAU;

    /* Reserve clusters 0 and 1 in FAT1 (and FAT2 if enabled) */
    syn_poke_u16_le(0xFFF8U, s_disk[1], 0);
    syn_poke_u16_le(0xFFFFU, s_disk[1], 2);
    if (num_fats > 1U) {
        syn_poke_u16_le(0xFFF8U, s_disk[2], 0);
        syn_poke_u16_le(0xFFFFU, s_disk[2], 2);
    }
}

static void format_fat32_mbr(uint8_t num_fats)
{
    memset(s_disk, 0, sizeof(s_disk));
    sd_emu_reset_counters();
    syn_vfs_init();
    mock_spi_set_transfer_callback(sd_spi_emulator);

    /* Sector 0: MBR pointing to LBA 2 */
    uint8_t *mbr = s_disk[0];
    mbr[446 + 4] = 0x0BU;
    syn_poke_u32_le(2U, mbr, 446 + 8);
    mbr[510] = 0x55U;
    mbr[511] = 0xAAU;

    /* Sector 2: FAT32 BPB */
    uint8_t *bpb = s_disk[2];
    syn_poke_u16_le(512U, bpb, 11);
    bpb[13] = 2U; /* 2 sectors per cluster */
    syn_poke_u16_le(1U, bpb, 14);
    bpb[16] = num_fats;
    syn_poke_u16_le(0U, bpb, 17);
    syn_poke_u16_le(0U, bpb, 22);
    syn_poke_u32_le(DISK_SECTORS - 2U, bpb, 32); /* total 32-bit sectors */
    syn_poke_u32_le(1U, bpb, 36);                /* 1 sector per FAT32 table */
    syn_poke_u32_le(2U, bpb, 44);                /* root_cluster = 2 */
    bpb[510] = 0x55U;
    bpb[511] = 0xAAU;

    /* FAT1 at sector 3 (and FAT2 at sector 4 if num_fats > 1): root cluster 2 = EOF */
    syn_poke_u32_le(0x0FFFFFF8U, s_disk[3], 0);
    syn_poke_u32_le(0x0FFFFFFFU, s_disk[3], 4);
    syn_poke_u32_le(0x0FFFFFFFU, s_disk[3], 8);
    if (num_fats > 1U) {
        memcpy(s_disk[4], s_disk[3], SECTOR_BYTES);
    }
}

static void test_fat_init_and_bpb_errors(void)
{
    TEST_ASSERT_NOT_NULL(syn_fat_get_ops());

    /* 1. syn_sd_init failure */
    format_fat16_superfloppy(2U);
    mock_spi_init_ok = false;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));
    mock_spi_init_ok = true;

    /* 2. Sector 0 read failure */
    format_fat16_superfloppy(2U);
    s_fail_read_at = 1U;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    /* 3. Invalid boot signature in sector 0 */
    format_fat16_superfloppy(2U);
    s_disk[0][510] = 0x00U;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    /* 4. Non-FAT MBR partition type + invalid bytes_per_sector */
    format_fat16_superfloppy(2U);
    s_disk[0][446 + 4] = 0x83U;
    syn_poke_u16_le(1024U, s_disk[0], 11);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    /* 5. sectors_per_cluster == 0 */
    format_fat16_superfloppy(2U);
    s_disk[0][13] = 0U;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    /* 6. MBR partition boot sector read failure */
    format_fat32_mbr(2U);
    s_fail_read_at = 2U;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    /* 7. BPB total_sectors <= non_data_sec and total_clusters == 0 */
    format_fat16_superfloppy(2U);
    syn_poke_u16_le(2U, s_disk[0], 19);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    format_fat16_superfloppy(2U);
    s_disk[0][13] = 2U;
    syn_poke_u16_le(5U, s_disk[0], 19);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    /* 8. BPB total_clusters clamped to fat_cap - 2 + duplicate VFS mount failure */
    format_fat16_superfloppy(2U);
    syn_poke_u16_le(600U, s_disk[0], 19);
    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));
    syn_vfs_init();
}

static void test_fat_null_and_pool_limits(void)
{
    format_fat16_superfloppy(1U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));

    const SYN_VfsOps *ops = syn_fat_get_ops();
    SYN_VfsFile empty_file;
    memset(&empty_file, 0, sizeof(empty_file));
    uint8_t buf[8] = {0};

    TEST_ASSERT_EQUAL_INT(-1, ops->open(NULL, "/a.txt", SYN_O_RDONLY, NULL));
    TEST_ASSERT_EQUAL_INT(-1, ops->open(&empty_file, NULL, SYN_O_RDONLY, NULL));
    TEST_ASSERT_EQUAL_INT(-1, ops->close(NULL));
    TEST_ASSERT_EQUAL_INT(-1, ops->close(&empty_file));
    TEST_ASSERT_EQUAL_INT(-1, ops->read(NULL, buf, 4U));
    TEST_ASSERT_EQUAL_INT(-1, ops->read(&empty_file, buf, 4U));
    TEST_ASSERT_EQUAL_INT(-1, ops->write(NULL, buf, 4U));
    TEST_ASSERT_EQUAL_INT(-1, ops->write(&empty_file, buf, 4U));
    TEST_ASSERT_EQUAL_INT32(-1, ops->seek(NULL, 0, SYN_SEEK_SET));
    TEST_ASSERT_EQUAL_INT32(-1, ops->seek(&empty_file, 0, SYN_SEEK_SET));
    TEST_ASSERT_EQUAL_INT32(-1, ops->tell(NULL));
    TEST_ASSERT_EQUAL_INT32(-1, ops->tell(&empty_file));
    TEST_ASSERT_EQUAL_INT(-1, ops->unlink(NULL, NULL));

    /* Exhaust open-file slot pool + test non-leading-slash long 8.3 name */
    SYN_VfsFile files[SYN_VFS_MAX_OPEN_FILES];
    memset(files, 0, sizeof(files));
    const char *names[SYN_VFS_MAX_OPEN_FILES] = {"longfilename.html", "/README1", "/f2.bin",
                                                 "/f3.dat"};
    for (int i = 0; i < SYN_VFS_MAX_OPEN_FILES; i++) {
        TEST_ASSERT_EQUAL_INT(0, ops->open(&files[i], names[i], SYN_O_CREAT | SYN_O_RDWR, NULL));
    }

    /* Valid file with NULL buffer when len > 0 */
    TEST_ASSERT_EQUAL_INT(-1, ops->read(&files[0], NULL, 4U));
    TEST_ASSERT_EQUAL_INT(-1, ops->write(&files[0], NULL, 4U));
    TEST_ASSERT_EQUAL_INT32(-2, ops->seek(&files[0], 0, 99));

    SYN_VfsFile extra;
    memset(&extra, 0, sizeof(extra));
    TEST_ASSERT_EQUAL_INT(-1, ops->open(&extra, "/overflow.txt", SYN_O_CREAT | SYN_O_RDWR, NULL));

    for (int i = 0; i < SYN_VFS_MAX_OPEN_FILES; i++) {
        TEST_ASSERT_EQUAL_INT(0, ops->close(&files[i]));
    }
    syn_vfs_init();
}

static void test_fat16_read_write_seek_trunc_append_unlink(void)
{
    format_fat16_superfloppy(2U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));

    /* Open missing file without SYN_O_CREAT returns -2 */
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_open("/sd/missing.txt", SYN_O_RDONLY));

    int fd = syn_vfs_open("/sd/log.txt", SYN_O_CREAT | SYN_O_RDWR);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);

    /* Write 1200 bytes across 3 FAT16 clusters (512 + 512 + 176) */
    uint8_t tx[1200];
    for (size_t i = 0U; i < sizeof(tx); i++) {
        tx[i] = (uint8_t)(i & 0xFFU);
    }
    TEST_ASSERT_EQUAL_INT(1200, syn_vfs_write(fd, tx, sizeof(tx)));
    TEST_ASSERT_EQUAL_INT32(1200, syn_vfs_tell(fd));

    /* Seek to beginning (including negative clamp), overwrite 16 bytes in-place, follow chain */
    TEST_ASSERT_EQUAL_INT32(0, syn_vfs_seek(fd, -50, SYN_SEEK_SET));
    uint8_t patch[600];
    memset(patch, 0xA5, sizeof(patch));
    TEST_ASSERT_EQUAL_INT(600, syn_vfs_write(fd, patch, sizeof(patch)));
    TEST_ASSERT_EQUAL_INT32(600, syn_vfs_tell(fd));

    /* Seek CUR and END (including clamp above file size) */
    TEST_ASSERT_EQUAL_INT32(610, syn_vfs_seek(fd, 10, SYN_SEEK_CUR));
    TEST_ASSERT_EQUAL_INT32(1200, syn_vfs_seek(fd, 100, SYN_SEEK_END));
    TEST_ASSERT_EQUAL_INT32(0, syn_vfs_seek(fd, 0, SYN_SEEK_SET));

    uint8_t rx[1300];
    memset(rx, 0, sizeof(rx));
    TEST_ASSERT_EQUAL_INT(1200, syn_vfs_read(fd, rx, sizeof(rx)));
    TEST_ASSERT_EQUAL_MEMORY(patch, rx, 600U);
    TEST_ASSERT_EQUAL_MEMORY(tx + 600U, rx + 600U, 600U);
    /* Reading at EOF returns 0 */
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_read(fd, rx, 16U));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    /* Re-open with SYN_O_APPEND */
    fd = syn_vfs_open("/sd/log.txt", SYN_O_RDWR | SYN_O_APPEND);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    TEST_ASSERT_EQUAL_INT32(1200, syn_vfs_tell(fd));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    /* Re-open with SYN_O_TRUNC (success + SD read failure branch) */
    fd = syn_vfs_open("/sd/log.txt", SYN_O_RDWR | SYN_O_TRUNC);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    TEST_ASSERT_EQUAL_INT32(0, syn_vfs_seek(fd, 0, SYN_SEEK_END));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    /* Re-open with SYN_O_TRUNC: SD read failure (-6) and SD write failure (-7) */
    sd_emu_reset_counters();
    s_fail_read_at = 2U; /* 1:scan_root_dir, 2:fat_truncate_file dir read */
    TEST_ASSERT_EQUAL_INT(-6, syn_vfs_open("/sd/log.txt", SYN_O_RDWR | SYN_O_TRUNC));

    sd_emu_reset_counters();
    s_fail_write_at = 1U; /* 1:fat_truncate_file dir write */
    TEST_ASSERT_EQUAL_INT(-7, syn_vfs_open("/sd/log.txt", SYN_O_RDWR | SYN_O_TRUNC));
    sd_emu_reset_counters();

    /* Re-populate 2 clusters, then unlink to test multi-cluster FAT16 free + 0xE5 reuse */
    fd = syn_vfs_open("/sd/log.txt", SYN_O_RDWR);
    TEST_ASSERT_EQUAL_INT(700, syn_vfs_write(fd, tx, 700U));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    TEST_ASSERT_EQUAL_INT(0, syn_vfs_unlink("/sd/log.txt"));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_unlink("/sd/log.txt"));

    /* Create new file — reuses 0xE5 deleted slot */
    fd = syn_vfs_open("/sd/reuse.txt", SYN_O_CREAT | SYN_O_RDWR);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));
    syn_vfs_init();
}

static void test_fat_io_and_allocation_errors(void)
{
    format_fat16_superfloppy(2U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));

    /* 1. Root directory read failure during scan */
    sd_emu_reset_counters();
    s_fail_read_at = 1U;
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_open("/sd/a.txt", SYN_O_RDONLY));

    /* 2. fat_create_file: empty-slot scan failure (read fail on 2nd scan) */
    sd_emu_reset_counters();
    s_fail_read_at = 2U;
    TEST_ASSERT_EQUAL_INT(-3, syn_vfs_open("/sd/a.txt", SYN_O_CREAT | SYN_O_RDWR));

    /* 3. fat_create_file: dir sector read fail (-6) and write fail (-7) */
    sd_emu_reset_counters();
    s_fail_read_at = 3U; /* 1:scan(false), 2:scan(true), 3:fat_create_file dir read */
    TEST_ASSERT_EQUAL_INT(-6, syn_vfs_open("/sd/a.txt", SYN_O_CREAT | SYN_O_RDWR));

    sd_emu_reset_counters();
    s_fail_write_at = 1U; /* 1:fat_create_file dir entry write */
    TEST_ASSERT_EQUAL_INT(-7, syn_vfs_open("/sd/a.txt", SYN_O_CREAT | SYN_O_RDWR));

    /* 4. Create valid 0-byte file and test initial cluster alloc + write/read/unlink errors */
    sd_emu_reset_counters();
    int fd = syn_vfs_open("/sd/err.txt", SYN_O_CREAT | SYN_O_RDWR);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);

    uint8_t blk[600];
    memset(blk, 0x33, sizeof(blk));

    /* Zero-length write succeeds without allocating a cluster */
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_write(fd, blk, 0U));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_read(fd, blk, 0U));

    /* fat_alloc_initial_cluster: find_free_cluster FAT read failure (-2) */
    sd_emu_reset_counters();
    s_fail_read_at = 1U;
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_write(fd, blk, 10U));

    /* fat_alloc_initial_cluster: write_fat_entry failures (-3) */
    sd_emu_reset_counters();
    s_fail_read_at = 2U; /* 1:find_free_cluster, 2:write_fat_entry read */
    TEST_ASSERT_EQUAL_INT(-3, syn_vfs_write(fd, blk, 10U));

    sd_emu_reset_counters();
    s_fail_write_at = 1U; /* FAT1 write fails */
    TEST_ASSERT_EQUAL_INT(-3, syn_vfs_write(fd, blk, 10U));

    sd_emu_reset_counters();
    s_fail_write_at = 2U; /* FAT2 write fails */
    TEST_ASSERT_EQUAL_INT(-3, syn_vfs_write(fd, blk, 10U));
    syn_poke_u16_le(0U, s_disk[1], 4);

    /* fat_alloc_initial_cluster: dir read fail (-5) and dir write fail (-6) */
    sd_emu_reset_counters();
    s_fail_read_at = 3U; /* 1:find_free, 2:write_fat, 3:dir read */
    TEST_ASSERT_EQUAL_INT(-5, syn_vfs_write(fd, blk, 10U));

    sd_emu_reset_counters();
    s_fail_write_at = 3U; /* 1:FAT1, 2:FAT2, 3:dir write */
    TEST_ASSERT_EQUAL_INT(-6, syn_vfs_write(fd, blk, 10U));

    /* fat_alloc_initial_cluster: disk full (-2) */
    for (uint32_t c = 2U; c < 30U; c++) {
        syn_poke_u16_le(0xFFFFU, s_disk[1], c * 2U);
    }
    sd_emu_reset_counters();
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_write(fd, blk, 10U));
    for (uint32_t c = 2U; c < 30U; c++) {
        syn_poke_u16_le(0U, s_disk[1], c * 2U);
    }

    /* Partial-sector RMW pre-read fail (-5) after initial cluster 2 is allocated */
    sd_emu_reset_counters();
    s_fail_read_at = 4U; /* 1:find_free, 2:write_fat, 3:dir read, 4:data sector RMW read */
    TEST_ASSERT_EQUAL_INT(-5, syn_vfs_write(fd, blk, 10U));

    /* Data sector write fail (-6) on full 512B write */
    sd_emu_reset_counters();
    s_fail_write_at = 1U;
    TEST_ASSERT_EQUAL_INT(-6, syn_vfs_write(fd, blk, 512U));

    /* Write 512B with directory-update read/write failing (-7), then succeed cleanly */
    sd_emu_reset_counters();
    s_fail_read_at = 1U;
    TEST_ASSERT_EQUAL_INT(-7, syn_vfs_write(fd, blk, 512U));
    TEST_ASSERT_EQUAL_INT32(0, syn_vfs_seek(fd, 0, SYN_SEEK_SET));

    sd_emu_reset_counters();
    s_fail_write_at = 2U; /* 1:data sector write, 2:dir sector size update */
    TEST_ASSERT_EQUAL_INT(-7, syn_vfs_write(fd, blk, 512U));
    TEST_ASSERT_EQUAL_INT32(0, syn_vfs_seek(fd, 0, SYN_SEEK_SET));

    sd_emu_reset_counters();
    TEST_ASSERT_EQUAL_INT(512, syn_vfs_write(fd, blk, 512U));

    /* Extend to 2nd cluster: transient FAT read failure returns -5 without truncating chain */
    syn_poke_u16_le(3U, s_disk[1], 4U);
    syn_poke_u16_le(0xFFFFU, s_disk[1], 6U);
    memcpy(s_disk[2], s_disk[1], SECTOR_BYTES);
    sd_emu_reset_counters();
    s_fail_read_at = 1U;
    TEST_ASSERT_EQUAL_INT(-5, syn_vfs_write(fd, blk, 10U));
    TEST_ASSERT_EQUAL_UINT16(3U, syn_peek_u16_le(s_disk[1], 4U));
    syn_poke_u16_le(0xFFFFU, s_disk[1], 4U);
    syn_poke_u16_le(0U, s_disk[1], 6U);
    memcpy(s_disk[2], s_disk[1], SECTOR_BYTES);

    /* Extend to 2nd cluster: fail linking curr_cluster (-3) and fail marking new EOF (-4) */
    sd_emu_reset_counters();
    s_fail_write_at = 1U;
    TEST_ASSERT_EQUAL_INT(-3, syn_vfs_write(fd, blk, 10U));

    sd_emu_reset_counters();
    s_fail_write_at = 3U; /* 1:FAT1(curr), 2:FAT2(curr), 3:FAT1(new_cluster) */
    TEST_ASSERT_EQUAL_INT(-4, syn_vfs_write(fd, blk, 10U));
    /* Restore cluster 2 to EOF */
    syn_poke_u16_le(0xFFFFU, s_disk[1], 4);
    syn_poke_u16_le(0xFFFFU, s_disk[2], 4);

    /* Fill all remaining FAT16 clusters (3..29) -> write cluster extend returns -2 */
    for (uint32_t c = 3U; c < 30U; c++) {
        syn_poke_u16_le(0xFFFFU, s_disk[1], c * 2U);
    }
    sd_emu_reset_counters();
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_write(fd, blk, 10U));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    /* Free clusters 3..29 for subsequent tests */
    for (uint32_t c = 3U; c < 30U; c++) {
        syn_poke_u16_le(0U, s_disk[1], c * 2U);
    }

    /* Read sector failure (-2) */
    fd = syn_vfs_open("/sd/err.txt", SYN_O_RDONLY);
    /* Fix size in dir sector 4 so read attempts SD sector read */
    syn_poke_u32_le(600U, s_disk[3], 28U);
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));
    fd = syn_vfs_open("/sd/err.txt", SYN_O_RDONLY);
    sd_emu_reset_counters();
    s_fail_read_at = 1U;
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_read(fd, blk, 16U));

    /* Premature EOF in get_cluster_for_offset (size=600, cluster 2=EOF) + FAT read error at 512 */
    sd_emu_reset_counters();
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_read(fd, blk, 600U));
    TEST_ASSERT_EQUAL_INT32(512, syn_vfs_seek(fd, 512, SYN_SEEK_SET));
    sd_emu_reset_counters();
    s_fail_read_at = 1U;
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_read(fd, blk, 16U));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    /* Unlink dir read failure (-2), dir write failure (-3), and free_cluster_chain read failure */
    sd_emu_reset_counters();
    s_fail_read_at = 2U;
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_unlink("/sd/err.txt"));

    sd_emu_reset_counters();
    s_fail_write_at = 1U;
    TEST_ASSERT_EQUAL_INT(-3, syn_vfs_unlink("/sd/err.txt"));

    /* Craft entry with start_cluster = 0 and size = 4 -> read rejects corrupt cluster 0 (-2) */
    syn_poke_u16_le(0U, s_disk[3], 26U);
    syn_poke_u32_le(4U, s_disk[3], 28U);
    sd_emu_reset_counters();
    fd = syn_vfs_open("/sd/err.txt", SYN_O_RDONLY);
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_read(fd, blk, 4U));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    /* Craft entry with out-of-range cluster 300 -> write rejects in resolve & write_fat_entry */
    syn_poke_u16_le(300U, s_disk[3], 26U);
    syn_poke_u32_le(512U, s_disk[3], 28U);
    fd = syn_vfs_open("/sd/err.txt", SYN_O_RDWR);
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_write(fd, blk, 4U));
    TEST_ASSERT_EQUAL_INT32(512, syn_vfs_seek(fd, 0, SYN_SEEK_END));
    TEST_ASSERT_EQUAL_INT(-3, syn_vfs_write(fd, blk, 4U));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    TEST_ASSERT_EQUAL_INT(0, syn_vfs_unlink("/sd/err.txt"));

    /* Full FAT16 root directory (all 16 slots occupied) -> scan_fat16_root returns false */
    for (uint32_t off = 0U; off < SECTOR_BYTES; off += 32U) {
        memset(&s_disk[3][off], 'X', 11U);
    }
    TEST_ASSERT_EQUAL_INT(-3, syn_vfs_open("/sd/new.txt", SYN_O_CREAT | SYN_O_RDWR));
    syn_vfs_init();
}

static void test_fat32_mbr_and_chained_root_dir(void)
{
    format_fat32_mbr(2U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));

    /* Populate cluster 2 (sectors 5, 6 = 32 entries) with dummy files, chain to cluster 3 */
    for (uint32_t s = 5U; s <= 6U; s++) {
        for (uint32_t off = 0U; off < SECTOR_BYTES; off += 32U) {
            memset(&s_disk[s][off], 'D', 11U);
        }
    }
    syn_poke_u32_le(3U, s_disk[3], 8U);          /* cluster 2 -> 3 */
    syn_poke_u32_le(0x0FFFFFFFU, s_disk[3], 12); /* cluster 3 -> EOF */
    memcpy(s_disk[4], s_disk[3], SECTOR_BYTES);

    /* Create file in chained cluster 3, write > 1 cluster (2 sectors = 1024B -> write 1100B) */
    int fd = syn_vfs_open("/sd/deep.txt", SYN_O_CREAT | SYN_O_RDWR);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);

    uint8_t payload[1100];
    for (size_t i = 0U; i < sizeof(payload); i++) {
        payload[i] = (uint8_t)(0xF0U ^ (i & 0xFFU));
    }
    TEST_ASSERT_EQUAL_INT(1100, syn_vfs_write(fd, payload, sizeof(payload)));
    TEST_ASSERT_EQUAL_INT32(0, syn_vfs_seek(fd, 0, SYN_SEEK_SET));

    uint8_t check[1100];
    TEST_ASSERT_EQUAL_INT(1100, syn_vfs_read(fd, check, sizeof(check)));
    TEST_ASSERT_EQUAL_MEMORY(payload, check, sizeof(payload));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    /* Test FAT32 read_fat_entry SD read error and scan_fat32_root SD read error */
    sd_emu_reset_counters();
    s_fail_read_at = 1U;
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_open("/sd/deep.txt", SYN_O_RDONLY));

    sd_emu_reset_counters();
    s_fail_read_at = 3U; /* sectors 5 & 6 read ok, then read_fat_entry(cluster 2) fails */
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_open("/sd/deep.txt", SYN_O_RDONLY));

    /* Unlink multi-cluster FAT32 file (including transient FAT read failure during chain free) */
    sd_emu_reset_counters();
    s_fail_read_at = 6U; /* 1..2:cluster2, 3:FAT(2->3), 4:cluster3, 5:dir read, 6:free read_fat */
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_unlink("/sd/deep.txt"));

    /* Cyclic FAT32 root chain (cluster 2 -> 3 -> 2) terminates via total_clusters limit */
    sd_emu_reset_counters();
    for (uint32_t s = 5U; s <= 8U; s++) {
        for (uint32_t off = 0U; off < SECTOR_BYTES; off += 32U) {
            memset(&s_disk[s][off], 'D', 11U);
        }
    }
    syn_poke_u32_le(3U, s_disk[3], 8U);
    syn_poke_u32_le(2U, s_disk[3], 12U);
    memcpy(s_disk[4], s_disk[3], SECTOR_BYTES);
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_open("/sd/missing.txt", SYN_O_RDONLY));
    syn_vfs_init();
}

static void test_r5_fat_regressions(void)
{
    /* 1. R5-S04: Writing to pre-existing 0-byte file (start_cluster == 0) preserves Sector 0 &
     * FAT[0..1] */
    format_fat16_superfloppy(2U);
    uint8_t sector0_golden[SECTOR_BYTES];
    memcpy(sector0_golden, s_disk[0], SECTOR_BYTES);

    /* Manually place a standard 0-byte directory entry "ZERO    TXT" with FstClus=0 in root sector
     * 3 */
    memcpy(&s_disk[3][0], "ZERO    TXT", 11U);
    s_disk[3][11] = 0x20U;
    syn_poke_u16_le(0U, s_disk[3], 20U);
    syn_poke_u16_le(0U, s_disk[3], 26U);
    syn_poke_u32_le(0U, s_disk[3], 28U);

    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));

    int fd = syn_vfs_open("/sd/zero.txt", SYN_O_WRONLY);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    uint8_t rx_tmp[16] = {0};
    /* R5-S09: SYN_O_WRONLY rejects read */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_read(fd, rx_tmp, sizeof(rx_tmp)));

    const char msg[] = "HelloZeroCluster!";
    TEST_ASSERT_EQUAL_INT((int)sizeof(msg), syn_vfs_write(fd, msg, sizeof(msg)));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    /* Sector 0 (BPB) and FAT[0..1] must be completely untouched */
    TEST_ASSERT_EQUAL_UINT8_ARRAY(sector0_golden, s_disk[0], SECTOR_BYTES);
    TEST_ASSERT_EQUAL_UINT16(0xFFF8U, syn_peek_u16_le(s_disk[1], 0U));
    TEST_ASSERT_EQUAL_UINT16(0xFFFFU, syn_peek_u16_le(s_disk[1], 2U));
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(2U, syn_peek_u16_le(s_disk[3], 26U));

    /* Remount and verify data + RDONLY write rejection + APPEND + seek behavior */
    syn_vfs_init();
    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));
    fd = syn_vfs_open("/sd/zero.txt", SYN_O_RDONLY);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_write(fd, msg, 4U));
    char check[32] = {0};
    TEST_ASSERT_EQUAL_INT((int)sizeof(msg), syn_vfs_read(fd, check, sizeof(check)));
    TEST_ASSERT_EQUAL_MEMORY(msg, check, sizeof(msg));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    fd = syn_vfs_open("/sd/zero.txt", SYN_O_WRONLY | SYN_O_APPEND);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    TEST_ASSERT_EQUAL_INT32(0, syn_vfs_seek(fd, 0, SYN_SEEK_SET));
    TEST_ASSERT_EQUAL_INT(4, syn_vfs_write(fd, "TAIL", 4U));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    SYN_VfsDirEnt ent;
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_stat("/sd/zero.txt", &ent));
    TEST_ASSERT_EQUAL_STRING("ZERO.TXT", ent.name);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)(sizeof(msg) + 4U), ent.size);
    TEST_ASSERT_FALSE(ent.is_dir);

    /* 2. R5-S09: SYN_O_TRUNC frees multi-cluster chain and resets start_cluster = 0 */
    fd = syn_vfs_open("/sd/big.bin", SYN_O_CREAT | SYN_O_RDWR);
    uint8_t big[1100];
    memset(big, 0x77, sizeof(big));
    TEST_ASSERT_EQUAL_INT(1100, syn_vfs_write(fd, big, sizeof(big)));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));
    /* Clusters 3, 4, 5 were allocated */
    TEST_ASSERT_NOT_EQUAL(0U, syn_peek_u16_le(s_disk[1], 6U));
    TEST_ASSERT_NOT_EQUAL(0U, syn_peek_u16_le(s_disk[1], 8U));
    TEST_ASSERT_NOT_EQUAL(0U, syn_peek_u16_le(s_disk[1], 10U));

    fd = syn_vfs_open("/sd/big.bin", SYN_O_RDWR | SYN_O_TRUNC);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));
    TEST_ASSERT_EQUAL_UINT16(0U, syn_peek_u16_le(s_disk[1], 6U));
    TEST_ASSERT_EQUAL_UINT16(0U, syn_peek_u16_le(s_disk[1], 8U));
    TEST_ASSERT_EQUAL_UINT16(0U, syn_peek_u16_le(s_disk[1], 10U));
    TEST_ASSERT_EQUAL_UINT16(0U, syn_peek_u16_le(s_disk[3], 32U + 26U));

    /* 3. R5-S09: Subdirectory / dot / empty paths rejected cleanly; stat & rename coverage */
    TEST_ASSERT_LESS_THAN_INT(0, syn_vfs_open("/sd/sub/file.txt", SYN_O_CREAT | SYN_O_RDWR));
    TEST_ASSERT_LESS_THAN_INT(0, syn_vfs_open("/sd/.dot", SYN_O_CREAT | SYN_O_RDWR));
    TEST_ASSERT_LESS_THAN_INT(0, syn_vfs_open("/sd/", SYN_O_CREAT | SYN_O_RDWR));
    TEST_ASSERT_LESS_THAN_INT(0, syn_vfs_stat("/sd/sub/file.txt", &ent));
    TEST_ASSERT_LESS_THAN_INT(0, syn_vfs_stat("/sd/missing.txt", &ent));
    TEST_ASSERT_LESS_THAN_INT(0, syn_vfs_unlink("/sd/sub/file.txt"));
    TEST_ASSERT_LESS_THAN_INT(0, syn_vfs_rename("/sd/sub/a.txt", "/sd/b.txt"));
    TEST_ASSERT_LESS_THAN_INT(0, syn_vfs_rename("/sd/zero.txt", "/sd/sub/b.txt"));
    TEST_ASSERT_LESS_THAN_INT(0, syn_vfs_rename("/sd/nope.txt", "/sd/b.txt"));

    const SYN_VfsOps *ops = syn_fat_get_ops();
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, ops->stat("/zero.txt", NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, ops->stat(NULL, &ent, NULL));

    /* Extensionless file stat + self-rename + overwrite rename + SD failure branches */
    fd = syn_vfs_open("/sd/README1", SYN_O_CREAT | SYN_O_RDWR);
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_stat("/sd/README1", &ent));
    TEST_ASSERT_EQUAL_STRING("README1", ent.name);

    TEST_ASSERT_EQUAL_INT(0, syn_vfs_rename("/sd/README1", "/sd/README1"));

    /* Destination overwrite error when unlink write fails */
    sd_emu_reset_counters();
    s_fail_write_at = 1U;
    TEST_ASSERT_EQUAL_INT(-3, syn_vfs_rename("/sd/zero.txt", "/sd/README1"));

    /* Destination overwrite succeeds, plus SD read/write errors on rename */
    sd_emu_reset_counters();
    s_fail_read_at = 3U; /* 1:scan old, 2:scan dst, 3:read old dir sector */
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_rename("/sd/zero.txt", "/sd/moved.txt"));

    sd_emu_reset_counters();
    s_fail_write_at = 1U;
    TEST_ASSERT_EQUAL_INT(-3, syn_vfs_rename("/sd/zero.txt", "/sd/moved.txt"));

    sd_emu_reset_counters();
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_rename("/sd/zero.txt", "/sd/README1"));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_stat("/sd/README1", &ent));
    TEST_ASSERT_EQUAL_UINT32((uint32_t)(sizeof(msg) + 4U), ent.size);

    /* 4. R5-S09: FAT32 top-4-bit reservation mask preserved in write_fat_entry */
    format_fat32_mbr(2U);
    /* Set reserved top nibble 0xA on free cluster 3 (offset 12 in FAT1 sector 3) */
    syn_poke_u32_le(0xA0000000U, s_disk[3], 12U);
    memcpy(s_disk[4], s_disk[3], SECTOR_BYTES);
    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));

    fd = syn_vfs_open("/sd/f32.bin", SYN_O_CREAT | SYN_O_RDWR);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    TEST_ASSERT_EQUAL_INT(8, syn_vfs_write(fd, "12345678", 8U));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));
    TEST_ASSERT_EQUAL_HEX32(0xAFFFFFFFU, syn_peek_u32_le(s_disk[3], 12U));
    TEST_ASSERT_EQUAL_HEX32(0xAFFFFFFFU, syn_peek_u32_le(s_disk[4], 12U));

    /* 5. Volume-ID (0x08), LFN (0x0F), directory (0x10), and read-only (0x01) entry rules */
    format_fat16_superfloppy(2U);
    memcpy(&s_disk[3][0], "VOLID   TXT", 11U);
    s_disk[3][11] = 0x08U;
    memcpy(&s_disk[3][32], "LFN     TXT", 11U);
    s_disk[3][32 + 11] = 0x0FU;
    memcpy(&s_disk[3][64], "SUBDIR     ", 11U);
    s_disk[3][64 + 11] = 0x10U;
    memcpy(&s_disk[3][96], "RDONLY  TXT", 11U);
    s_disk[3][96 + 11] = 0x01U;
    memcpy(&s_disk[3][128], "NORMAL  TXT", 11U);
    s_disk[3][128 + 11] = 0x20U;
    syn_vfs_init();
    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));

    /* Volume-ID and LFN entries must be ignored during filename lookup */
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_open("/sd/volid.txt", SYN_O_RDONLY));
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_open("/sd/lfn.txt", SYN_O_RDONLY));

    /* Directory entry can be stat'd (is_dir == true), but open/unlink/rename are rejected */
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_stat("/sd/subdir", &ent));
    TEST_ASSERT_TRUE(ent.is_dir);
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_open("/sd/subdir", SYN_O_RDONLY));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_unlink("/sd/subdir"));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_rename("/sd/subdir", "/sd/moved"));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_rename("/sd/normal.txt", "/sd/subdir"));

    /* Read-only file allows SYN_O_RDONLY, rejects write/trunc/unlink/rename */
    fd = syn_vfs_open("/sd/rdonly.txt", SYN_O_RDONLY);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_open("/sd/rdonly.txt", SYN_O_WRONLY));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_open("/sd/rdonly.txt", SYN_O_RDWR));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_vfs_open("/sd/rdonly.txt", SYN_O_RDONLY | SYN_O_TRUNC));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_unlink("/sd/rdonly.txt"));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_rename("/sd/rdonly.txt", "/sd/moved.txt"));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_rename("/sd/normal.txt", "/sd/rdonly.txt"));

    syn_vfs_init();
}

static void test_tier_a_fat_bpb_and_chain_bounds(void)
{
    /* 1. BPB num_fats == 0 and 32-bit layout overflow guards */
    format_fat16_superfloppy(0U);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    format_fat32_mbr(2U);
    syn_poke_u32_le(0x80000001UL, s_disk[2], 36U);
    syn_poke_u32_le(0x1000UL, s_disk[2], 32U);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    format_fat32_mbr(2U);
    syn_poke_u32_le(0xFFFFFFFFUL, s_disk[2], 32U);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    /* 2. FAT32 invalid root_cluster (< 2 and out-of-range) */
    format_fat32_mbr(2U);
    syn_poke_u32_le(0U, s_disk[2], 44U);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    format_fat32_mbr(2U);
    syn_poke_u32_le(500U, s_disk[2], 44U);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    /* 3. Corrupt chain entry (0xFFF7 bad cluster) and cyclic chain bound */
    format_fat16_superfloppy(2U);
    memcpy(&s_disk[3][0], "BADCHAINBIN", 11U);
    s_disk[3][11] = 0x20U;
    syn_poke_u16_le(2U, s_disk[3], 26U);
    syn_poke_u32_le(32768UL, s_disk[3], 28U);
    syn_poke_u16_le(0xFFF7U, s_disk[1], 4U);
    memcpy(s_disk[2], s_disk[1], SECTOR_BYTES);

    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));
    int fd = syn_vfs_open("/sd/badchain.bin", SYN_O_RDWR);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);

    uint8_t io_buf[16] = {0};
    TEST_ASSERT_EQUAL_INT32(512, syn_vfs_seek(fd, 512, SYN_SEEK_SET));
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_read(fd, io_buf, sizeof(io_buf)));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_write(fd, io_buf, sizeof(io_buf)));

    /* Cyclic chain 2 -> 3 -> 2 must terminate cleanly at total_clusters hops */
    syn_poke_u16_le(3U, s_disk[1], 4U);
    syn_poke_u16_le(2U, s_disk[1], 6U);
    memcpy(s_disk[2], s_disk[1], SECTOR_BYTES);
    TEST_ASSERT_EQUAL_INT32(20000, syn_vfs_seek(fd, 20000, SYN_SEEK_SET));
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_read(fd, io_buf, sizeof(io_buf)));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_write(fd, io_buf, sizeof(io_buf)));

    /* Premature EOF in cluster chain (file_size = 32768 > 1 cluster, cluster 2 -> 0xFFFF EOF)
     * must return read error (-2) instead of repeating cluster 2 data */
    syn_poke_u16_le(0xFFFFU, s_disk[1], 4U);
    memcpy(s_disk[2], s_disk[1], SECTOR_BYTES);
    TEST_ASSERT_EQUAL_INT32(1024, syn_vfs_seek(fd, 1024, SYN_SEEK_SET));
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_read(fd, io_buf, sizeof(io_buf)));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    syn_vfs_init();
}

static void test_tier_a_fat_io_and_seek_bounds(void)
{
    format_fat16_superfloppy(2U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));

    int fd = syn_vfs_open("/sd/tiny.bin", SYN_O_CREAT | SYN_O_RDWR);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    TEST_ASSERT_EQUAL_INT(4, syn_vfs_write(fd, "ABCD", 4U));

    uint8_t rx[8] = {0};
    TEST_ASSERT_EQUAL_INT32(0, syn_vfs_seek(fd, 0, SYN_SEEK_SET));
    TEST_ASSERT_EQUAL_INT(4, syn_vfs_read(fd, rx, (size_t)0x7FFFFFFFUL + 16U));
    TEST_ASSERT_EQUAL_MEMORY("ABCD", rx, 4U);

    /* Subtraction clamp prevents 32-bit offset + len wraparound */
    TEST_ASSERT_EQUAL_INT32(2, syn_vfs_seek(fd, 2, SYN_SEEK_SET));
    TEST_ASSERT_EQUAL_INT(2, syn_vfs_read(fd, rx, ~(size_t)0U));
    TEST_ASSERT_EQUAL_MEMORY("CD", rx, 2U);
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    /* Write length clamps (INT_MAX and UINT32_MAX - offset) + >2GiB seek guard */
    memcpy(&s_disk[3][32], "HUGE    BIN", 11U);
    s_disk[3][32 + 11] = 0x20U;
    syn_poke_u16_le(0U, s_disk[3], 32U + 26U);
    syn_poke_u32_le(0xFFFFFFFCUL, s_disk[3], 32U + 28U);
    for (uint32_t c = 3U; c < 30U; c++) {
        syn_poke_u16_le(0xFFFFU, s_disk[1], c * 2U);
    }

    fd = syn_vfs_open("/sd/huge.bin", SYN_O_WRONLY);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    uint8_t tx[8] = {0};
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_write(fd, tx, (size_t)0x7FFFFFFFUL + 16U));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    fd = syn_vfs_open("/sd/huge.bin", SYN_O_WRONLY | SYN_O_APPEND);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    TEST_ASSERT_EQUAL_INT32(-2, syn_vfs_tell(fd));
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_write(fd, tx, 16U));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    fd = syn_vfs_open("/sd/huge.bin", SYN_O_RDONLY);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    TEST_ASSERT_EQUAL_INT32(-2, syn_vfs_seek(fd, 0, SYN_SEEK_END));
    TEST_ASSERT_EQUAL_INT32(100, syn_vfs_seek(fd, 100, SYN_SEEK_SET));
    TEST_ASSERT_EQUAL_INT32(-2, syn_vfs_seek(fd, 0x7FFFFFFF, SYN_SEEK_CUR));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    syn_vfs_init();
}

static void test_fat_open_fd_guard_and_multi_fat_sector_chain(void)
{
    /* 1. Reject unlink and rename (both source and destination) while target file is open */
    format_fat16_superfloppy(2U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));

    int fd_a = syn_vfs_open("/sd/open_a.txt", SYN_O_CREAT | SYN_O_RDWR);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd_a);
    TEST_ASSERT_EQUAL_INT(4, syn_vfs_write(fd_a, "OPEN", 4U));

    int fd_b = syn_vfs_open("/sd/closed_b.txt", SYN_O_CREAT | SYN_O_RDWR);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd_b);
    TEST_ASSERT_EQUAL_INT(4, syn_vfs_write(fd_b, "DATA", 4U));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd_b));

    SYN_VfsDirEnt ent;
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_stat("/sd/open_a.txt", &ent));
    TEST_ASSERT_EQUAL_UINT32(4U, ent.size);
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_vfs_unlink("/sd/open_a.txt"));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_vfs_rename("/sd/open_a.txt", "/sd/moved.txt"));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_vfs_rename("/sd/closed_b.txt", "/sd/open_a.txt"));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_rename("/sd/open_a.txt", "/sd/open_a.txt"));

    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd_a));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_rename("/sd/closed_b.txt", "/sd/open_a.txt"));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_unlink("/sd/open_a.txt"));

    /* 2. Multi-FAT-sector cluster chain (cluster 255 in FAT sec 1 -> cluster 256 in FAT sec 2) */
    format_fat16_superfloppy(2U);
    memset(s_disk[2], 0,
           SECTOR_BYTES); /* Clear FAT2-at-spf=1 markers; s_disk[2] is now FAT1 sec 2 */
    syn_poke_u16_le(300U, s_disk[0], 19); /* total sectors = 300 */
    syn_poke_u16_le(2U, s_disk[0], 22);   /* 2 sectors per FAT (FAT1: 1..2, FAT2: 3..4, root: 5) */
    s_sparse_lba[0] = 6U + (255U - 2U);   /* LBA 259 = cluster 255 */
    s_sparse_lba[1] = 6U + (256U - 2U);   /* LBA 260 = cluster 256 */

    /* Mark clusters 2..254 used so allocation starts at cluster 255 and crosses to 256 */
    for (uint32_t c = 2U; c <= 254U; c++) {
        syn_poke_u16_le(0xFFFFU, s_disk[1], c * 2U);
    }
    memcpy(s_disk[3], s_disk[1], SECTOR_BYTES);

    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));

    int fd_cross = syn_vfs_open("/sd/cross.bin", SYN_O_CREAT | SYN_O_RDWR);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd_cross);

    uint8_t chunk[64];
    for (uint32_t blk = 0U; blk < 11U; blk++) {
        for (uint32_t i = 0U; i < sizeof(chunk); i++) {
            chunk[i] = (uint8_t)((blk * 64U + i) ^ 0xA5U);
        }
        TEST_ASSERT_EQUAL_INT(64, syn_vfs_write(fd_cross, chunk, sizeof(chunk)));
    }

    /* Verify FAT1 (sectors 1..2) and FAT2 (sectors 3..4) link 255 -> 256 -> EOF */
    TEST_ASSERT_EQUAL_UINT16(256U, syn_peek_u16_le(s_disk[1], 255U * 2U));
    TEST_ASSERT_EQUAL_UINT16(0xFFFFU, syn_peek_u16_le(s_disk[2], 0U));
    TEST_ASSERT_EQUAL_UINT16(256U, syn_peek_u16_le(s_disk[3], 255U * 2U));
    TEST_ASSERT_EQUAL_UINT16(0xFFFFU, syn_peek_u16_le(s_disk[4], 0U));

    TEST_ASSERT_EQUAL_INT32(0, syn_vfs_seek(fd_cross, 0, SYN_SEEK_SET));
    for (uint32_t blk = 0U; blk < 11U; blk++) {
        memset(chunk, 0, sizeof(chunk));
        TEST_ASSERT_EQUAL_INT(64, syn_vfs_read(fd_cross, chunk, sizeof(chunk)));
        for (uint32_t i = 0U; i < sizeof(chunk); i++) {
            TEST_ASSERT_EQUAL_UINT8((uint8_t)((blk * 64U + i) ^ 0xA5U), chunk[i]);
        }
    }
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd_cross));

    TEST_ASSERT_EQUAL_INT(0, syn_vfs_unlink("/sd/cross.bin"));
    TEST_ASSERT_EQUAL_UINT16(0U, syn_peek_u16_le(s_disk[1], 255U * 2U));
    TEST_ASSERT_EQUAL_UINT16(0U, syn_peek_u16_le(s_disk[2], 0U));
    TEST_ASSERT_EQUAL_UINT16(0U, syn_peek_u16_le(s_disk[3], 255U * 2U));
    TEST_ASSERT_EQUAL_UINT16(0U, syn_peek_u16_le(s_disk[4], 0U));

    syn_vfs_init();
}

static void test_fat_concurrent_open_coherency_bpb_and_name_guards(void)
{
    /* 1. Untrusted BPB geometry: reserved_sectors == 0, bad spc, root_entries, and max clusters */
    format_fat16_superfloppy(2U);
    syn_poke_u16_le(0U, s_disk[0], 14U); /* reserved_sectors == 0 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    format_fat16_superfloppy(2U);
    s_disk[0][13] = 3U; /* non-power-of-two sectors_per_cluster */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    format_fat16_superfloppy(2U);
    s_disk[0][13] = 255U; /* sectors_per_cluster > 128 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    format_fat16_superfloppy(2U);
    syn_poke_u16_le(0U, s_disk[0], 17U); /* FAT16 with fat16_root_entries == 0 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    format_fat32_mbr(2U);
    syn_poke_u16_le(16U, s_disk[2], 17U); /* FAT32 with fat16_root_entries != 0 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fat_init(0U, 1U));

    /* FAT16 cluster cap at 0xFFF5 when fat_cap > 0xFFF7 and total_sec32 > 70000 */
    format_fat16_superfloppy(2U);
    syn_poke_u16_le(0U, s_disk[0], 19U);
    syn_poke_u32_le(70000UL, s_disk[0], 32U);
    syn_poke_u16_le(256U, s_disk[0], 22U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));
    syn_vfs_init();

    /* 2. Filename validation: 0xE5 tombstone, spaces/control chars, and invalid open flags */
    format_fat16_superfloppy(2U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_fat_init(0U, 1U));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_vfs_open("/sd/\xE5tomb.txt", SYN_O_CREAT | SYN_O_RDWR));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_open("/sd/   ", SYN_O_CREAT | SYN_O_RDWR));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_vfs_open("/sd/bad\x01.txt", SYN_O_CREAT | SYN_O_RDWR));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_vfs_open("/sd/ok.txt", 0x03));

    /* 3. Concurrent open coherency + SYN_O_RDONLY | SYN_O_TRUNC rejection on writable file */
    int fd_w = syn_vfs_open("/sd/shared.txt", SYN_O_CREAT | SYN_O_RDWR);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd_w);
    TEST_ASSERT_EQUAL_INT(5, syn_vfs_write(fd_w, "HELLO", 5U));

    /* While writer is open, any second open (reader, writer, or trunc) must return SYN_BUSY */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_vfs_open("/sd/shared.txt", SYN_O_RDONLY));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_vfs_open("/sd/shared.txt", SYN_O_RDWR | SYN_O_TRUNC));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd_w));

    /* Writable file must reject SYN_O_RDONLY | SYN_O_TRUNC without truncating contents */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_vfs_open("/sd/shared.txt", SYN_O_RDONLY | SYN_O_TRUNC));

    /* Multiple concurrent SYN_O_RDONLY handles are allowed, while concurrent write/trunc is BUSY */
    int fd_r1 = syn_vfs_open("/sd/shared.txt", SYN_O_RDONLY);
    int fd_r2 = syn_vfs_open("/sd/shared.txt", SYN_O_RDONLY);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd_r1);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd_r2);
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_vfs_open("/sd/shared.txt", SYN_O_WRONLY));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_vfs_open("/sd/shared.txt", SYN_O_RDWR | SYN_O_TRUNC));

    char rx[8] = {0};
    TEST_ASSERT_EQUAL_INT(5, syn_vfs_read(fd_r1, rx, 5U));
    TEST_ASSERT_EQUAL_MEMORY("HELLO", rx, 5U);
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd_r1));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd_r2));

    syn_vfs_init();
}

void run_fat_tests(void)
{
    RUN_TEST(test_fat_init_and_bpb_errors);
    RUN_TEST(test_fat_null_and_pool_limits);
    RUN_TEST(test_fat16_read_write_seek_trunc_append_unlink);
    RUN_TEST(test_fat_io_and_allocation_errors);
    RUN_TEST(test_fat32_mbr_and_chained_root_dir);
    RUN_TEST(test_r5_fat_regressions);
    RUN_TEST(test_tier_a_fat_bpb_and_chain_bounds);
    RUN_TEST(test_tier_a_fat_io_and_seek_bounds);
    RUN_TEST(test_fat_open_fd_guard_and_multi_fat_sector_chain);
    RUN_TEST(test_fat_concurrent_open_coherency_bpb_and_name_guards);
}
