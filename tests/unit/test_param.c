/**
 * @file test_param.c
 * @brief Unity tests for syn_param.
 */

#include "mocks/mock_port.h"
#include "syntropic/storage/syn_param.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

typedef struct {
    uint16_t brightness;
    int16_t offset;
    uint8_t mode;
    uint8_t _pad;
} TestParams;

static void test_param_store(void)
{
    /* Erase flash */
    memset(mock_flash, 0xFF, sizeof(mock_flash));

    SYN_ParamStore store;
    /* 4 sectors of 1024 bytes each */
    SYN_Status st = syn_param_init(&store, 0, 4, sizeof(TestParams));
    TEST_ASSERT_EQUAL(SYN_ERROR, st);

    /* Save defaults */
    TestParams params = {.brightness = 80, .offset = -10, .mode = 3};
    st = syn_param_save(&store, &params);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Load back */
    TestParams loaded;
    memset(&loaded, 0, sizeof(loaded));
    st = syn_param_load(&store, &loaded);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(80, loaded.brightness);
    TEST_ASSERT_EQUAL_INT(-10, loaded.offset);
    TEST_ASSERT_EQUAL_INT(3, loaded.mode);

    /* Save again (goes to next slot) */
    params.brightness = 90;
    st = syn_param_save(&store, &params);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Re-init and load (should find latest) */
    SYN_ParamStore store2;
    st = syn_param_init(&store2, 0, 4, sizeof(TestParams));
    TEST_ASSERT_EQUAL(SYN_OK, st);

    memset(&loaded, 0, sizeof(loaded));
    st = syn_param_load(&store2, &loaded);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(90, loaded.brightness);

    /* Wear leveling: save many times and verify it wraps sectors */
    int i;
    for (i = 0; i < 200; i++) {
        params.brightness = (uint16_t)(i & 0xFFFF);
        st = syn_param_save(&store, &params);
        TEST_ASSERT_EQUAL(SYN_OK, st);
    }

    memset(&loaded, 0, sizeof(loaded));
    st = syn_param_load(&store, &loaded);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(199, loaded.brightness);

    /* Erase all */
    st = syn_param_erase_all(&store);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    st = syn_param_load(&store, &loaded);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);

    /* After erase_all, next_seq resets to 0 → write_count returns 0 */
    uint16_t wc = syn_param_write_count(&store);
    TEST_ASSERT_EQUAL_UINT16(0, wc);
}

/** data_size exceeds sector capacity — exercises line 132 (slots_per_sector == 0) */
static void test_param_data_too_large(void)
{
    mock_port_reset();
    SYN_ParamStore store;
    /* MOCK_FLASH_SECTOR=1024; set data_size >> sector so slot doesn't fit */
    SYN_Status st = syn_param_init(&store, 0, 1, 1024u);
    /* Should return SYN_INVALID_PARAM (slot exceeds sector capacity) */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, st);
}

static void test_param_sector_wrap(void)
{
    mock_port_reset();
    SYN_ParamStore store;
    TestParams p = {.brightness = 100, .offset = 20, .mode = 1};

    /* 2 sectors of 1024 bytes */
    SYN_Status st = syn_param_init(&store, 0, 2, sizeof(TestParams));
    TEST_ASSERT_EQUAL(SYN_ERROR, st); /* blank flash */

    /* Perform enough saves to fill sector 0 and wrap to sector 1 */
    uint16_t slots_per_sec = store.slots_per_sector;
    for (uint16_t i = 0; i <= slots_per_sec; i++) {
        p.brightness = (uint16_t)i;
        st = syn_param_save(&store, &p);
        TEST_ASSERT_EQUAL(SYN_OK, st);
    }

    TestParams loaded = {0};
    st = syn_param_load(&store, &loaded);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_UINT16(slots_per_sec, loaded.brightness);
}

static void test_param_load_crc_corrupted_slot(void)
{
    mock_port_reset();
    SYN_ParamStore store;
    TestParams p = {.brightness = 100, .offset = 20, .mode = 1};

    syn_param_init(&store, 0, 2, sizeof(TestParams));
    syn_param_save(&store, &p);

    /* Corrupt payload CRC in flash */
    mock_flash[sizeof(SYN_ParamSlotHeader)] ^= 0xFF;

    TestParams loaded = {0};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_load(&store, &loaded));
}

static void test_param_corrupt_header_data_size(void)
{
    mock_port_reset();
    SYN_ParamStore store;
    TestParams p = {.brightness = 100, .offset = 20, .mode = 1};

    syn_param_init(&store, 0, 2, sizeof(p));
    syn_param_save(&store, &p);

    /* Corrupt slot header data_size in flash */
    SYN_ParamSlotHeader *hdr = (SYN_ParamSlotHeader *)mock_flash;
    hdr->data_size = (uint16_t)(sizeof(p) + 5);

    TestParams loaded;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_load(&store, &loaded));
}

static void test_param_flash_erase_error_returns_failure(void)
{
    mock_port_reset();
    SYN_ParamStore store;
    TestParams p = {.brightness = 100, .offset = 20, .mode = 1};
    syn_param_init(&store, 0, 2, sizeof(p));
    mock_flash_fail_at = 0;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_erase_all(&store));
    mock_flash_fail_at = 0;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_save(&store, &p));
    mock_flash_fail_at = -1;
}

static void test_param_scan_crc_mismatch(void)
{
    mock_port_reset();
    SYN_ParamStore store;
    TestParams p = {.brightness = 100, .offset = 20, .mode = 1};

    syn_param_init(&store, 0, 2, sizeof(p));
    syn_param_save(&store, &p);

    /* Corrupt header CRC field in flash */
    SYN_ParamSlotHeader *hdr = (SYN_ParamSlotHeader *)mock_flash;
    hdr->crc ^= 0x1234;

    /* Re-init store. Init scans slots using data=NULL (verify_slot_crc, line 107) */
    SYN_ParamStore store2;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_init(&store2, 0, 2, sizeof(p)));
}

static void test_param_save_write_errors(void)
{
    mock_port_reset();
    SYN_ParamStore store;
    TestParams p = {.brightness = 100, .offset = 20, .mode = 1};
    syn_param_init(&store, 0, 2, sizeof(p));

    /* 1. Header write error (line 239) */
    mock_flash_write_fail_next = true;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_save(&store, &p));
    mock_port_reset();

    /* 2. First write succeeds, data write error (line 244) */
    syn_param_init(&store, 0, 2, sizeof(p));
    syn_param_save(&store, &p); /* next_seq becomes 2 */

    /* Advance slot to wrap sector */
    uint16_t slots = store.slots_per_sector;
    for (uint16_t i = 1; i < slots; i++) {
        syn_param_save(&store, &p);
    }
    /* Next save wraps sector and erases sector 1. Make erase fail (line 218) */
    mock_flash_fail_at = 1024;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_save(&store, &p));

    /* 3. Data write error (line 240) */
    mock_port_reset();
    syn_param_init(&store, 0, 2, sizeof(p));
    syn_param_save(&store, &p); /* first write at slot 0 (addr 0) */
    /* Second write goes to slot 1 (data at addr 40, hdr at addr 24). Set fail at 40 */
    mock_flash_fail_at = 40;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_save(&store, &p));

    /* 4. Read slot data flash read failure (line 96) */
    mock_port_reset();
    syn_param_init(&store, 0, 2, sizeof(p));
    syn_param_save(&store, &p);
    mock_flash_fail_at = 8;
    TestParams loaded;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_load(&store, &loaded));

    /* 5. verify_slot_crc flash read failure during init scan (line 62) */
    mock_port_reset();
    syn_param_init(&store, 0, 2, sizeof(p));
    syn_param_save(&store, &p);
    mock_flash_fail_at = 8;
    SYN_ParamStore store_scan;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_init(&store_scan, 0, 2, sizeof(p)));

    mock_port_reset();
}

static void test_param_power_loss_recovery(void)
{
    mock_port_reset();
    SYN_ParamStore store;
    TestParams p1 = {.brightness = 50, .offset = 5, .mode = 1};
    TestParams p2 = {.brightness = 99, .offset = 15, .mode = 2};

    syn_param_init(&store, 0, 2, sizeof(p1));
    syn_param_save(&store, &p1); /* Valid slot 0 */

    /* Simulate power cut during p2 save: data write at addr 40 fails midway, leaving header at 24
     * uncommitted (0xFF) */
    mock_flash_fail_at = 40; /* Fail data write */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_save(&store, &p2));

    /* Reset flash fail trigger for normal power cycle recovery */
    mock_flash_fail_at = -1;

    /* Power cycle: re-init store and verify slot 0 (p1) is loaded cleanly without corruption */
    SYN_ParamStore store_recovery;
    TEST_ASSERT_EQUAL(SYN_OK, syn_param_init(&store_recovery, 0, 2, sizeof(p1)));

    TestParams loaded;
    TEST_ASSERT_EQUAL(SYN_OK, syn_param_load(&store_recovery, &loaded));
    TEST_ASSERT_EQUAL_INT(50, loaded.brightness);
    TEST_ASSERT_EQUAL_INT(5, loaded.offset);

    mock_port_reset();
}

static void test_param_sequence_number_wrap(void)
{
    mock_port_reset();
    SYN_ParamStore store;
    TestParams p = {.brightness = 100, .offset = 20, .mode = 1};

    /* Initialize with blank flash */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_init(&store, 0, 2, sizeof(TestParams)));

    /* Set next_seq to 65534 so sequence numbers are contiguous: 65534 -> 65535 -> 0 -> 1 */
    store.next_seq = 65534;

    /* First write: writes seq=65534 at slot 0 */
    p.brightness = 1;
    TEST_ASSERT_EQUAL(SYN_OK, syn_param_save(&store, &p));
    TEST_ASSERT_EQUAL_UINT16(0, store.active_slot);
    TEST_ASSERT_EQUAL_UINT16(65535, store.next_seq);

    /* Second write: writes seq=65535 at slot 1 */
    p.brightness = 2;
    TEST_ASSERT_EQUAL(SYN_OK, syn_param_save(&store, &p));
    TEST_ASSERT_EQUAL_UINT16(1, store.active_slot);
    TEST_ASSERT_EQUAL_UINT16(0, store.next_seq); /* wrapped to 0! */

    /* Third write: with next_seq = 0, must advance to slot 2 and not erase active sector */
    p.brightness = 3;
    TEST_ASSERT_EQUAL(SYN_OK, syn_param_save(&store, &p));
    TEST_ASSERT_EQUAL_UINT16(2, store.active_slot);
    TEST_ASSERT_EQUAL_UINT16(1, store.next_seq);

    /* Fourth write: with next_seq = 1, must advance to slot 3 */
    p.brightness = 4;
    TEST_ASSERT_EQUAL(SYN_OK, syn_param_save(&store, &p));
    TEST_ASSERT_EQUAL_UINT16(3, store.active_slot);
    TEST_ASSERT_EQUAL_UINT16(2, store.next_seq);

    /* Power cycle: re-init store and ensure it finds latest seq (seq=1) at slot 3 */
    SYN_ParamStore store2;
    TEST_ASSERT_EQUAL(SYN_OK, syn_param_init(&store2, 0, 2, sizeof(TestParams)));
    TEST_ASSERT_EQUAL_UINT16(3, store2.active_slot);
    TEST_ASSERT_EQUAL_UINT16(2, store2.next_seq);

    TestParams loaded;
    TEST_ASSERT_EQUAL(SYN_OK, syn_param_load(&store2, &loaded));
    TEST_ASSERT_EQUAL_INT(4, loaded.brightness);
}

static void test_r5_param_regressions(void)
{
    mock_port_reset();
    SYN_ParamStore store;
    SYN_ParamStore uninit;
    memset(&uninit, 0, sizeof(uninit));
    TestParams p = {.brightness = 42, .offset = -3, .mode = 2};

    /* NULL / zero parameter checks */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_param_init(NULL, 0, 2, sizeof(p)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_param_init(&store, 0, 0, sizeof(p)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_param_init(&store, 0, 2, 0));

    /* R5-S11: 16-bit wraparound & divide-by-zero rejection */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_param_init(&store, 0, 2, 65530U));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_param_init(&store, 0, 2, 65520U));

    /* NULL & uninitialized guards on load / save / erase_all */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_param_load(NULL, &p));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_param_load(&uninit, &p));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_param_save(NULL, &p));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_param_save(&uninit, &p));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_param_erase_all(NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_param_erase_all(&uninit));

    /* Multi-chunk (> 32B) CRC verification + header read error path */
    uint8_t big_in[48];
    uint8_t big_out[48];
    for (size_t i = 0; i < sizeof(big_in); i++) {
        big_in[i] = (uint8_t)(i + 1U);
    }
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_init(&store, 0, 2, (uint16_t)sizeof(big_in)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_param_load(&store, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_param_save(&store, NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_param_save(&store, big_in));

    SYN_ParamStore store_reinit;
    TEST_ASSERT_EQUAL(SYN_OK, syn_param_init(&store_reinit, 0, 2, (uint16_t)sizeof(big_in)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_param_load(&store_reinit, big_out));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(big_in, big_out, sizeof(big_in));

    /* Fail slot header read on load, payload read on load (line 94), and payload read on init scan
     * (line 61) */
    mock_flash_fail_at = 0;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_load(&store_reinit, big_out));
    mock_flash_fail_at = (int)sizeof(SYN_ParamSlotHeader);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_load(&store_reinit, big_out));
    mock_flash_fail_at = (int)sizeof(SYN_ParamSlotHeader);
    SYN_ParamStore store_scan_fail;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_init(&store_scan_fail, 0, 2, (uint16_t)sizeof(big_in)));
    mock_port_reset();
}

void run_param_tests(void)
{
    RUN_TEST(test_param_store);
    RUN_TEST(test_param_data_too_large);
    RUN_TEST(test_param_sector_wrap);
    RUN_TEST(test_param_load_crc_corrupted_slot);
    RUN_TEST(test_param_corrupt_header_data_size);
    RUN_TEST(test_param_flash_erase_error_returns_failure);
    RUN_TEST(test_param_scan_crc_mismatch);
    RUN_TEST(test_param_save_write_errors);
    RUN_TEST(test_param_power_loss_recovery);
    RUN_TEST(test_param_sequence_number_wrap);
    RUN_TEST(test_r5_param_regressions);
}
