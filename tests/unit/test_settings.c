/**
 * @file test_settings.c
 * @brief Unit tests for syn_settings — persistent settings with change detection.
 *
 * Tests init (blank flash / pre-existing data), save, change detection,
 * reset to defaults, callback, reload, and no-op save optimization.
 */

#include "mocks/mock_port.h"
#include "syntropic/storage/syn_settings.h"
#include "syntropic/storage/syn_vfs.h"
#include "unity/unity.h"

#include <string.h>

/* ── Test data ─────────────────────────────────────────────────────────── */

typedef struct {
    int32_t velocity;
    int32_t accel;
    uint8_t mode;
    uint8_t _pad[3]; /* Align to 4 bytes */
} TestSettings;

static const TestSettings defaults = {.velocity = 500, .accel = 200, .mode = 1};

/* Flash layout: use base address 0, 4 sectors */
#define FLASH_BASE 0
#define SECTOR_COUNT 4

/* ── Callback tracking ─────────────────────────────────────────────────── */

static int change_cb_count;
static void *change_cb_data;
static void *change_cb_ctx;

static void on_change(void *data, void *ctx)
{
    change_cb_count++;
    change_cb_data = data;
    change_cb_ctx = ctx;
}

/* ── Tests ──────────────────────────────────────────────────────────────── */

static void test_settings_init_blank_flash(void)
{
    /* Flash is blank (all 0xFF from mock_port_reset) */
    TestSettings settings;
    SYN_Settings store;

    SYN_Status st =
        syn_settings_init(&store, FLASH_BASE, SECTOR_COUNT, &settings, sizeof(settings), &defaults);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Should have loaded defaults */
    TEST_ASSERT_EQUAL_INT32(500, settings.velocity);
    TEST_ASSERT_EQUAL_INT32(200, settings.accel);
    TEST_ASSERT_EQUAL_UINT8(1, settings.mode);
}

static void test_settings_save_and_reload(void)
{
    TestSettings settings;
    SYN_Settings store;

    syn_settings_init(&store, FLASH_BASE, SECTOR_COUNT, &settings, sizeof(settings), &defaults);

    /* Modify and save */
    settings.velocity = 800;
    SYN_Status st = syn_settings_save(&store);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Re-init from same flash — should load saved values */
    TestSettings settings2;
    SYN_Settings store2;

    st = syn_settings_init(&store2, FLASH_BASE, SECTOR_COUNT, &settings2, sizeof(settings2),
                           &defaults);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT32(800, settings2.velocity);
    TEST_ASSERT_EQUAL_INT32(200, settings2.accel); /* unchanged */
}

static void test_settings_change_detection(void)
{
    TestSettings settings;
    SYN_Settings store;

    syn_settings_init(&store, FLASH_BASE, SECTOR_COUNT, &settings, sizeof(settings), &defaults);

    /* Immediately after init, no changes */
    TEST_ASSERT_FALSE(syn_settings_changed(&store));

    /* Modify data in-place */
    settings.accel = 999;
    TEST_ASSERT_TRUE(syn_settings_changed(&store));

    /* Save — should clear the changed flag */
    syn_settings_save(&store);
    TEST_ASSERT_FALSE(syn_settings_changed(&store));
}

static void test_settings_save_noop_if_unchanged(void)
{
    TestSettings settings;
    SYN_Settings store;

    syn_settings_init(&store, FLASH_BASE, SECTOR_COUNT, &settings, sizeof(settings), &defaults);

    uint16_t crc_before = syn_settings_checksum(&store);

    /* Save without changing anything */
    SYN_Status st = syn_settings_save(&store);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Checksum should not have changed */
    TEST_ASSERT_EQUAL_UINT16(crc_before, syn_settings_checksum(&store));
}

static void test_settings_reset_to_defaults(void)
{
    TestSettings settings;
    SYN_Settings store;

    syn_settings_init(&store, FLASH_BASE, SECTOR_COUNT, &settings, sizeof(settings), &defaults);

    /* Modify */
    settings.velocity = 9999;
    settings.accel = 0;
    settings.mode = 7;
    syn_settings_save(&store);

    /* Reset */
    SYN_Status st = syn_settings_reset(&store);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Should be back to defaults */
    TEST_ASSERT_EQUAL_INT32(500, settings.velocity);
    TEST_ASSERT_EQUAL_INT32(200, settings.accel);
    TEST_ASSERT_EQUAL_UINT8(1, settings.mode);

    /* Reload from flash to verify persisted */
    TestSettings settings2;
    SYN_Settings store2;
    syn_settings_init(&store2, FLASH_BASE, SECTOR_COUNT, &settings2, sizeof(settings2), &defaults);
    TEST_ASSERT_EQUAL_INT32(500, settings2.velocity);
}

static void test_settings_change_callback(void)
{
    TestSettings settings;
    SYN_Settings store;

    syn_settings_init(&store, FLASH_BASE, SECTOR_COUNT, &settings, sizeof(settings), &defaults);

    change_cb_count = 0;
    change_cb_data = NULL;
    change_cb_ctx = NULL;

    int ctx_value = 42;
    syn_settings_on_change(&store, on_change, &ctx_value);

    /* Save without changes — callback should NOT fire */
    syn_settings_save(&store);
    TEST_ASSERT_EQUAL_INT(0, change_cb_count);

    /* Save with changes — callback should fire */
    settings.mode = 5;
    syn_settings_save(&store);
    TEST_ASSERT_EQUAL_INT(1, change_cb_count);
    TEST_ASSERT_EQUAL_PTR(&settings, change_cb_data);
    TEST_ASSERT_EQUAL_PTR(&ctx_value, change_cb_ctx);
}

static void test_settings_reload_discards_changes(void)
{
    TestSettings settings;
    SYN_Settings store;

    syn_settings_init(&store, FLASH_BASE, SECTOR_COUNT, &settings, sizeof(settings), &defaults);

    /* Save known state */
    settings.velocity = 600;
    syn_settings_save(&store);

    /* Modify without saving */
    settings.velocity = 12345;
    TEST_ASSERT_TRUE(syn_settings_changed(&store));

    /* Reload from flash — should discard */
    SYN_Status st = syn_settings_reload(&store);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT32(600, settings.velocity);
    TEST_ASSERT_FALSE(syn_settings_changed(&store));
}

static void test_settings_checksum_changes_on_save(void)
{
    TestSettings settings;
    SYN_Settings store;

    syn_settings_init(&store, FLASH_BASE, SECTOR_COUNT, &settings, sizeof(settings), &defaults);

    uint16_t crc1 = syn_settings_checksum(&store);

    settings.velocity = 777;
    syn_settings_save(&store);

    uint16_t crc2 = syn_settings_checksum(&store);

    /* Checksums should differ after data change */
    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

/* ── Runner ─────────────────────────────────────────────────────────────── */

static void test_settings_export_and_import(void)
{
    TestSettings settings;
    SYN_Settings store;

    syn_settings_init(&store, FLASH_BASE, SECTOR_COUNT, &settings, sizeof(settings), &defaults);
    settings.velocity = 1234;

    uint8_t buf[64];
    int exported_len = syn_settings_export(&store, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_INT(sizeof(TestSettings), exported_len);

    TestSettings settings2;
    SYN_Settings store2;
    syn_settings_init(&store2, FLASH_BASE, SECTOR_COUNT, &settings2, sizeof(settings2), &defaults);

    SYN_Status st = syn_settings_import(&store2, buf, exported_len, true);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT32(1234, settings2.velocity);
}

static void test_settings_dual_bank(void)
{
    TestSettings settings;
    SYN_DualBankSettings db;

    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_dual_bank_init(&db, FLASH_BASE, FLASH_BASE + 2048, 2,
                                                          &settings, sizeof(settings), &defaults));

    TEST_ASSERT_EQUAL_INT(0, db.active_bank);

    /* Modify settings */
    settings.velocity = 888;
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_dual_bank_save(&db));
    TEST_ASSERT_EQUAL_INT(1, db.active_bank);

    /* Saving unchanged data returns OK immediately */
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_dual_bank_save(&db));
    TEST_ASSERT_EQUAL_INT(1, db.active_bank);
}

static void test_settings_dual_bank_reboot_selects_newest_bank(void)
{
    mock_port_reset();
    TestSettings data;
    SYN_DualBankSettings db;

    /* 1. Initial dual bank init (both blank): Bank A becomes active with defaults (velocity=500) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_dual_bank_init(&db, FLASH_BASE, FLASH_BASE + 2048, 2,
                                                          &data, sizeof(data), &defaults));
    TEST_ASSERT_EQUAL_INT(0, db.active_bank);
    TEST_ASSERT_EQUAL_INT32(500, data.velocity);

    /* 2. Save new setting -> written to inactive Bank B (seq=2), active_bank becomes 1 */
    data.velocity = 888;
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_dual_bank_save(&db));
    TEST_ASSERT_EQUAL_INT(1, db.active_bank);

    /* 3. Simulate reboot: fresh RAM buffer and fresh DualBankSettings instance */
    TestSettings reboot_data = {0};
    SYN_DualBankSettings reboot_db;

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_settings_dual_bank_init(&reboot_db, FLASH_BASE, FLASH_BASE + 2048, 2,
                                                  &reboot_data, sizeof(reboot_data), &defaults));

    /* Bank B has seq=2 while Bank A has seq=1 -> Bank B must be selected as active */
    TEST_ASSERT_EQUAL_INT(1, reboot_db.active_bank);
    TEST_ASSERT_EQUAL_INT32(888, reboot_data.velocity);

    /* 4. Subsequent save from reboot state must write to Bank A and succeed */
    reboot_data.velocity = 999;
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_dual_bank_save(&reboot_db));
    TEST_ASSERT_EQUAL_INT(0, reboot_db.active_bank);

    /* Verify reboot again selects Bank A (seq=3 > seq=2) */
    TestSettings reboot_data2 = {0};
    SYN_DualBankSettings reboot_db2;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_settings_dual_bank_init(&reboot_db2, FLASH_BASE, FLASH_BASE + 2048, 2,
                                                  &reboot_data2, sizeof(reboot_data2), &defaults));
    TEST_ASSERT_EQUAL_INT(0, reboot_db2.active_bank);
    TEST_ASSERT_EQUAL_INT32(999, reboot_data2.velocity);

    mock_port_reset();
}

static void test_settings_edge_cases(void)
{
    TestSettings settings;
    SYN_Settings store;

    syn_settings_init(&store, FLASH_BASE, SECTOR_COUNT, &settings, sizeof(settings), &defaults);

    /* Invalid parameters for export & import */
    TEST_ASSERT_EQUAL(-1, syn_settings_export(NULL, &settings, sizeof(settings)));
    TEST_ASSERT_EQUAL(-2, syn_settings_export(&store, &settings, 2));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_settings_import(NULL, &settings, sizeof(settings), false));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_settings_import(&store, &settings, 2, false));

    /* VFS invalid parameters */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_settings_export_vfs(NULL, "/cfg/set.bin"));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_settings_import_vfs(NULL, "/cfg/set.bin", false));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_settings_export_vfs(&store, "/invalid/path"));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_settings_import_vfs(&store, "/invalid/path", false));

    /* Import without save parameter (save = false) */
    TestSettings new_vals = {.velocity = 1234, .accel = 567, .mode = 2};
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_import(&store, &new_vals, sizeof(new_vals), false));
    TEST_ASSERT_EQUAL_INT32(1234, settings.velocity);
}

static uint8_t ram_vfs_buf[128];
static size_t ram_vfs_len = 0;
static int s_vfs_force_short = 0;

static int settings_vfs_open(SYN_VfsFile *file, const char *path, int flags, void *fs_data)
{
    (void)file;
    (void)path;
    (void)flags;
    (void)fs_data;
    return 0;
}

static int settings_vfs_close(SYN_VfsFile *file)
{
    (void)file;
    return 0;
}

static int settings_vfs_write(SYN_VfsFile *file, const void *buf, size_t len)
{
    (void)file;
    if (s_vfs_force_short != 0) {
        return 2;
    }
    if (len > sizeof(ram_vfs_buf)) {
        len = sizeof(ram_vfs_buf);
    }
    memcpy(ram_vfs_buf, buf, len);
    ram_vfs_len = len;
    return (int)len;
}

static int settings_vfs_read(SYN_VfsFile *file, void *buf, size_t len)
{
    (void)file;
    if (s_vfs_force_short != 0) {
        return 2;
    }
    if (len > ram_vfs_len) {
        len = ram_vfs_len;
    }
    memcpy(buf, ram_vfs_buf, len);
    return (int)len;
}

static const SYN_VfsOps settings_vfs_ops = {.open = settings_vfs_open,
                                            .close = settings_vfs_close,
                                            .read = settings_vfs_read,
                                            .write = settings_vfs_write,
                                            .seek = NULL};

static void test_settings_vfs_export_import(void)
{
    syn_vfs_init();
    syn_vfs_mount("/cfg", &settings_vfs_ops, NULL);

    TestSettings settings;
    SYN_Settings store;

    syn_settings_init(&store, FLASH_BASE, SECTOR_COUNT, &settings, sizeof(settings), &defaults);
    settings.velocity = 7777;
    settings.accel = 333;

    /* Export to VFS */
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_export_vfs(&store, "/cfg/settings.bin"));

    /* Import from VFS into clean instance */
    TestSettings settings2;
    SYN_Settings store2;
    syn_settings_init(&store2, FLASH_BASE + 2048, SECTOR_COUNT, &settings2, sizeof(settings2),
                      &defaults);

    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_import_vfs(&store2, "/cfg/settings.bin", true));
    TEST_ASSERT_EQUAL_INT32(7777, settings2.velocity);
    TEST_ASSERT_EQUAL_INT32(333, settings2.accel);
}

static void test_settings_dual_bank_fallback_to_defaults(void)
{
    TestSettings data;
    SYN_DualBankSettings db;

    /* Initialize dual bank where both banks default to defaults */
    SYN_Status st = syn_settings_dual_bank_init(&db, FLASH_BASE, FLASH_BASE + 4096, SECTOR_COUNT,
                                                &data, sizeof(data), &defaults);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(0, db.active_bank);
    TEST_ASSERT_EQUAL_INT32(500, data.velocity);

    /* Corrupt Bank A in flash so Bank B becomes active (if valid) */
    memset(mock_flash, 0, 4096);
    st = syn_settings_dual_bank_init(&db, FLASH_BASE, FLASH_BASE + 4096, SECTOR_COUNT, &data,
                                     sizeof(data), &defaults);
    TEST_ASSERT_EQUAL(SYN_OK, st);
}

static void test_settings_dual_bank_ping_pong(void)
{
    mock_port_reset();
    TestSettings data;
    SYN_DualBankSettings db;

    syn_settings_dual_bank_init(&db, FLASH_BASE, FLASH_BASE + 2048, 2, &data, sizeof(data),
                                &defaults);
    TEST_ASSERT_EQUAL(0, db.active_bank);

    data.velocity = 100;
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_dual_bank_save(&db));
    TEST_ASSERT_EQUAL(1, db.active_bank);

    data.velocity = 200;
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_dual_bank_save(&db));
    TEST_ASSERT_EQUAL(0, db.active_bank);
}

static void test_settings_null_and_invalid_param_checks(void)
{
    TestSettings data;
    SYN_Settings store;
    SYN_DualBankSettings db;

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_settings_init(NULL, FLASH_BASE, SECTOR_COUNT, &data,
                                                           sizeof(data), &defaults));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_settings_init(&store, FLASH_BASE, SECTOR_COUNT, NULL,
                                                           sizeof(data), &defaults));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_settings_init(&store, FLASH_BASE, SECTOR_COUNT, &data, 0, &defaults));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_settings_save(NULL));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_settings_dual_bank_init(NULL, FLASH_BASE, FLASH_BASE + 2048, SECTOR_COUNT,
                                                  &data, sizeof(data), &defaults));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_settings_dual_bank_init(&db, FLASH_BASE, FLASH_BASE + 2048, SECTOR_COUNT,
                                                  NULL, sizeof(data), &defaults));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_settings_dual_bank_init(&db, FLASH_BASE, FLASH_BASE + 2048, SECTOR_COUNT,
                                                  &data, sizeof(data), NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_settings_dual_bank_init(&db, FLASH_BASE, FLASH_BASE + 2048, SECTOR_COUNT,
                                                  &data, 0, &defaults));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_settings_dual_bank_save(NULL));
}

static void test_settings_vfs_import_no_save(void)
{
    TestSettings settings;
    SYN_Settings store;
    syn_settings_init(&store, FLASH_BASE, SECTOR_COUNT, &settings, sizeof(settings), &defaults);
    settings.velocity = 5555;
    syn_settings_export_vfs(&store, "/cfg/nosave.bin");

    TestSettings settings2;
    SYN_Settings store2;
    syn_settings_init(&store2, FLASH_BASE, SECTOR_COUNT, &settings2, sizeof(settings2), &defaults);
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_import_vfs(&store2, "/cfg/nosave.bin", false));
    TEST_ASSERT_EQUAL_INT32(5555, settings2.velocity);
}

static void test_settings_dual_bank_bank_b_only_valid(void)
{
    mock_port_reset();
    TestSettings data;
    SYN_DualBankSettings db;

    /* Initialize Bank B with valid saved settings */
    SYN_Settings store_b;
    syn_settings_init(&store_b, 2048, 2, &data, sizeof(data), &defaults);
    data.velocity = 999;
    syn_settings_save(&store_b);

    /* Set mock_flash_fail_at = 12296 (inside sector 12288 data area) so Bank A erase fails during
     * init */
    mock_flash_fail_at = 12296;

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_settings_dual_bank_init(&db, 12288, 2048, 2, &data, sizeof(data), &defaults));
    TEST_ASSERT_EQUAL(1, db.active_bank);
    TEST_ASSERT_EQUAL_INT32(999, data.velocity);
    mock_port_reset();
}

static void test_settings_dual_bank_neither_bank_valid(void)
{
    mock_port_reset();
    TestSettings data;
    SYN_DualBankSettings db;

    /* Set mock_flash_fail_at = 12296 (fails Bank A erase) and mock_flash_write_fail_next = true
     * (fails Bank B write) */
    mock_flash_fail_at = 12296;
    mock_flash_write_fail_next = true;

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_settings_dual_bank_init(&db, 12288, 2048, 2, &data, sizeof(data), &defaults));
    TEST_ASSERT_EQUAL(0, db.active_bank);
    TEST_ASSERT_EQUAL_INT32(500, data.velocity);
    mock_port_reset();
}

static void test_settings_init_load_fail(void)
{
    mock_port_reset();
    TestSettings data, defaults_data = defaults;
    SYN_Settings store;

    /* First init writes valid settings to flash */
    syn_settings_init(&store, FLASH_BASE, 2, &data, sizeof(data), &defaults_data);

    /* Second init: syn_param_init returns SYN_OK, then transient flash read failure on
     * syn_param_load restores defaults (line 130) */
    mock_flash_read_fail_after = 2 * (int32_t)store.store.slots_per_sector + 1;
    TestSettings data2 = {.velocity = 9999, .accel = 99, .mode = 9};
    SYN_Settings store2;
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_settings_init(&store2, FLASH_BASE, 2, &data2, sizeof(data2), &defaults_data));
    TEST_ASSERT_EQUAL_INT32(500, data2.velocity);
    mock_port_reset();
}

static void test_settings_vfs_short_read_write_error(void)
{
    mock_port_reset();
    TestSettings data;
    SYN_Settings store;
    syn_settings_init(&store, FLASH_BASE, 2, &data, sizeof(data), &defaults);

    /* Export to unmounted path returns SYN_ERROR (lines 161 & 179) */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_settings_export_vfs(&store, "/invalid/file.bin"));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_settings_import_vfs(&store, "/invalid/file.bin", false));
    mock_port_reset();
}

static void test_settings_init_save_fail(void)
{
    mock_port_reset();
    TestSettings data;
    SYN_Settings store;

    /* Fail flash erase/write on first save in syn_settings_init (lines 56-57) */
    mock_flash_write_fail_next = true;
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_settings_init(&store, FLASH_BASE, 2, &data, sizeof(data), &defaults));
    mock_port_reset();
}

static void test_r5_settings_regressions(void)
{
    mock_port_reset();
    TestSettings data;
    SYN_Settings store;
    SYN_DualBankSettings db;

    /* Invalid geometry propagation from syn_param_init */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_settings_init(&store, FLASH_BASE, 0, &data, sizeof(data), &defaults));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_settings_init(&store, FLASH_BASE, 2, &data, 65530U, &defaults));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_settings_dual_bank_init(&db, 0, 2048, 0, &data, sizeof(data), &defaults));

    SYN_DualBankSettings uninit_db;
    memset(&uninit_db, 0, sizeof(uninit_db));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_settings_dual_bank_save(&uninit_db));

    /* 1. Blank init + immediate reboot before Bank B is ever written (valid_a && !valid_b) */
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_settings_dual_bank_init(&db, 0, 2048, 2, &data, sizeof(data), &defaults));
    TEST_ASSERT_EQUAL_UINT8(0U, db.active_bank);

    TestSettings rdata = {0};
    SYN_DualBankSettings rdb;
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_settings_dual_bank_init(&rdb, 0, 2048, 2, &rdata, sizeof(rdata), &defaults));
    TEST_ASSERT_EQUAL_UINT8(0U, rdb.active_bank);
    TEST_ASSERT_EQUAL_INT32(500, rdata.velocity);

    /* 2. R5-S11: A (500) -> B (888) -> A (500) revert persists across reboot */
    change_cb_count = 0;
    syn_settings_on_change(&rdb.bank_b, on_change, NULL);
    rdata.velocity = 888;
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_dual_bank_save(&rdb));
    TEST_ASSERT_EQUAL_UINT8(1U, rdb.active_bank);
    TEST_ASSERT_EQUAL_INT(1, change_cb_count);
    TEST_ASSERT_EQUAL_UINT16(rdb.bank_a.store.next_seq, rdb.bank_b.store.next_seq);

    /* Revert 888 -> 500 (same value as old Bank A slot 0, must still commit seq=3!) */
    rdata.velocity = 500;
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_dual_bank_save(&rdb));
    TEST_ASSERT_EQUAL_UINT8(0U, rdb.active_bank);
    TEST_ASSERT_EQUAL_UINT16(4U, rdb.bank_a.store.next_seq);
    TEST_ASSERT_EQUAL_UINT16(4U, rdb.bank_b.store.next_seq);

    /* Flash write failure on dual_bank_save leaves active_bank unchanged */
    rdata.velocity = 777;
    mock_flash_write_fail_next = true;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_settings_dual_bank_save(&rdb));
    TEST_ASSERT_EQUAL_UINT8(0U, rdb.active_bank);

    /* Reboot must load reverted 500 from Bank A (seq=3 > Bank B seq=2) */
    TestSettings rdata2 = {0};
    SYN_DualBankSettings rdb2;
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_settings_dual_bank_init(&rdb2, 0, 2048, 2, &rdata2, sizeof(rdata2), &defaults));
    TEST_ASSERT_EQUAL_UINT8(0U, rdb2.active_bank);
    TEST_ASSERT_EQUAL_INT32(500, rdata2.velocity);

    /* 3. Single-bank recovery: Bank A blank while Bank B holds seq=5 must not overwrite with seq=1
     */
    mock_port_reset();
    SYN_Settings sb;
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_init(&sb, 2048, 2, &data, sizeof(data), &defaults));
    for (int i = 1; i <= 4; i++) {
        data.velocity = 600 + i;
        TEST_ASSERT_EQUAL(SYN_OK, syn_settings_save(&sb));
    }
    TestSettings bdata = {0};
    SYN_DualBankSettings bdb;
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_settings_dual_bank_init(&bdb, 0, 2048, 2, &bdata, sizeof(bdata), &defaults));
    TEST_ASSERT_EQUAL_UINT8(1U, bdb.active_bank);
    TEST_ASSERT_EQUAL_INT32(604, bdata.velocity);
    TEST_ASSERT_EQUAL_UINT16(6U, bdb.bank_a.store.next_seq);
    TEST_ASSERT_EQUAL_UINT16(6U, bdb.bank_b.store.next_seq);

    /* 4. Short VFS read/write error handling */
    syn_vfs_init();
    syn_vfs_mount("/cfg", &settings_vfs_ops, NULL);
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_settings_init(&store, FLASH_BASE, 2, &data, sizeof(data), &defaults));
    s_vfs_force_short = 1;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_settings_export_vfs(&store, "/cfg/short.bin"));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_settings_import_vfs(&store, "/cfg/short.bin", false));
    s_vfs_force_short = 0;
    mock_port_reset();
}

static int32_t s_tier_a_seek_len = 0;
static int s_tier_a_rewind_fail = 0;

static int32_t tier_a_vfs_seek(SYN_VfsFile *file, int32_t offset, int whence)
{
    (void)file;
    (void)offset;
    if (whence == SYN_SEEK_END) {
        return s_tier_a_seek_len;
    }
    if ((whence == SYN_SEEK_SET) && (s_tier_a_rewind_fail != 0)) {
        return -1;
    }
    return 0;
}

static const SYN_VfsOps tier_a_vfs_ops = {.open = settings_vfs_open,
                                          .close = settings_vfs_close,
                                          .read = settings_vfs_read,
                                          .write = settings_vfs_write,
                                          .seek = tier_a_vfs_seek};

static void test_tier_a_settings_dual_bank_and_vfs_guards(void)
{
    mock_port_reset();
    TestSettings data;
    SYN_DualBankSettings db;

    /* 1. Bank A (seq=1) + Bank B (seq=2, vel=888) + Bank A (seq=3, vel=999):
     *    Fail Bank A load in select_dual_bank -> must fall back to Bank B (888) */
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_settings_dual_bank_init(&db, 0U, 2048U, 2U, &data, sizeof(data), &defaults));
    data.velocity = 888;
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_dual_bank_save(&db));
    data.velocity = 999;
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_dual_bank_save(&db));

    int32_t slots = 2 * (int32_t)db.bank_a.store.slots_per_sector;
    mock_flash_read_fail_after = (slots + 2) + (slots + 1);
    TestSettings fb_data = {.velocity = 12345};
    SYN_DualBankSettings fb_db;
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_dual_bank_init(&fb_db, 0U, 2048U, 2U, &fb_data,
                                                          sizeof(fb_data), &defaults));
    TEST_ASSERT_EQUAL_UINT8(1U, fb_db.active_bank);
    TEST_ASSERT_EQUAL_INT32(888, fb_data.velocity);

    /* 2. Only Bank A valid (seq=1), fail Bank A load -> must fall back to defaults (500) */
    mock_port_reset();
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_settings_dual_bank_init(&db, 0U, 2048U, 2U, &data, sizeof(data), &defaults));
    mock_flash_read_fail_after = (slots + 1) + slots;
    fb_data.velocity = -9999;
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_dual_bank_init(&fb_db, 0U, 2048U, 2U, &fb_data,
                                                          sizeof(fb_data), &defaults));
    TEST_ASSERT_EQUAL_UINT8(0U, fb_db.active_bank);
    TEST_ASSERT_EQUAL_INT32(500, fb_data.velocity);

    /* 3. Seekable VFS size check (oversized/undersized/rewind-fail) + short-read RAM restore */
    mock_port_reset();
    syn_vfs_init();
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/scfg", &tier_a_vfs_ops, NULL));
    SYN_Settings store;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_settings_init(&store, FLASH_BASE, 2U, &data, sizeof(data), &defaults));
    data.velocity = 777;
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_save(&store));
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_export_vfs(&store, "/scfg/ok.bin"));

    /* Oversized file rejected before touching data */
    s_tier_a_seek_len = (int32_t)sizeof(data) + 4;
    s_tier_a_rewind_fail = 0;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_settings_import_vfs(&store, "/scfg/ok.bin", false));
    TEST_ASSERT_EQUAL_INT32(777, data.velocity);

    /* Rewind failure rejected */
    s_tier_a_seek_len = (int32_t)sizeof(data);
    s_tier_a_rewind_fail = 1;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_settings_import_vfs(&store, "/scfg/ok.bin", false));
    s_tier_a_rewind_fail = 0;

    /* Valid size + import succeeds */
    TestSettings imported = {0};
    SYN_Settings store2;
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_init(&store2, FLASH_BASE + 2048U, 2U, &imported,
                                                sizeof(imported), &defaults));
    TEST_ASSERT_EQUAL(SYN_OK, syn_settings_import_vfs(&store2, "/scfg/ok.bin", false));
    TEST_ASSERT_EQUAL_INT32(777, imported.velocity);

    /* Short read restores from flash store (777), or from defaults (500) if flash load fails */
    memset(ram_vfs_buf, 0xEE, sizeof(ram_vfs_buf));
    s_vfs_force_short = 1;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_settings_import_vfs(&store, "/scfg/ok.bin", false));
    TEST_ASSERT_EQUAL_INT32(777, data.velocity);

    memset(&mock_flash[FLASH_BASE], 0x00, 2048U);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_settings_import_vfs(&store, "/scfg/ok.bin", false));
    TEST_ASSERT_EQUAL_INT32(500, data.velocity);

    s_vfs_force_short = 0;
    mock_port_reset();
}

void run_settings_tests(void)
{
    RUN_TEST(test_settings_init_blank_flash);
    RUN_TEST(test_settings_save_and_reload);
    RUN_TEST(test_settings_change_detection);
    RUN_TEST(test_settings_save_noop_if_unchanged);
    RUN_TEST(test_settings_reset_to_defaults);
    RUN_TEST(test_settings_change_callback);
    RUN_TEST(test_settings_reload_discards_changes);
    RUN_TEST(test_settings_checksum_changes_on_save);
    RUN_TEST(test_settings_export_and_import);
    RUN_TEST(test_settings_dual_bank);
    RUN_TEST(test_settings_dual_bank_reboot_selects_newest_bank);
    RUN_TEST(test_settings_edge_cases);
    RUN_TEST(test_settings_vfs_export_import);
    RUN_TEST(test_settings_dual_bank_fallback_to_defaults);
    RUN_TEST(test_settings_dual_bank_ping_pong);
    RUN_TEST(test_settings_null_and_invalid_param_checks);
    RUN_TEST(test_settings_vfs_import_no_save);
    RUN_TEST(test_settings_dual_bank_bank_b_only_valid);
    RUN_TEST(test_settings_dual_bank_neither_bank_valid);
    RUN_TEST(test_settings_init_load_fail);
    RUN_TEST(test_settings_vfs_short_read_write_error);
    RUN_TEST(test_settings_init_save_fail);
    RUN_TEST(test_r5_settings_regressions);
    RUN_TEST(test_tier_a_settings_dual_bank_and_vfs_guards);
}
