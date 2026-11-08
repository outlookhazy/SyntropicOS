/**
 * @file test_fwupdate.c
 * @brief Tests for firmware image headers, streaming updater, and A/B boot.
 */

#include "mocks/mock_port.h"
#include "syntropic/port/syn_port_flash.h"
#include "syntropic/system/syn_fwboot.h"
#include "syntropic/system/syn_fwimage.h"
#include "syntropic/system/syn_fwupdate.h"
#include "syntropic/util/syn_crc.h"
#include "unity/unity.h"

#include <string.h>

/* Use the mock flash for all operations.
 * Layout: slot A at 0x0000, slot B at 0x0800 (within mock_flash[4096]).
 * Each slot is 2048 bytes (2 sectors of 1024). */
#define SLOT_A_ADDR 0x0000u
#define SLOT_B_ADDR 0x0800u
#define SLOT_SIZE (2048u - (uint32_t)MOCK_FLASH_SECTOR)

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
#define TEST_FWUPDATE_FINISH(u, crc, ver) syn_fwupdate_finish((u), (crc), NULL, NULL, (ver))
#else
#define TEST_FWUPDATE_FINISH(u, crc, ver) syn_fwupdate_finish((u), (crc), NULL, (ver))
#endif
#else
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
#define TEST_FWUPDATE_FINISH(u, crc, ver) syn_fwupdate_finish((u), (crc), NULL, (ver))
#else
#define TEST_FWUPDATE_FINISH(u, crc, ver) syn_fwupdate_finish((u), (crc), (ver))
#endif
#endif

/* ── Image header tests ─────────────────────────────────────────────────── */

void test_fwimage_seal_and_validate(void)
{
    SYN_FwImageHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = SYN_FW_MAGIC;
    hdr.version_code = 0x00010200; /* v1.2.0 */
    hdr.image_size = 512;
    hdr.image_crc = 0xDEADBEEF;
    hdr.state = SYN_FW_STATE_CONFIRMED;

    syn_fwimage_seal_header(&hdr);

    TEST_ASSERT_TRUE(syn_fwimage_header_valid(&hdr));
    TEST_ASSERT_TRUE(syn_fwimage_is_bootable(&hdr));
}

void test_fwimage_bad_magic(void)
{
    SYN_FwImageHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = 0x12345678; /* wrong */
    syn_fwimage_seal_header(&hdr);

    TEST_ASSERT_FALSE(syn_fwimage_header_valid(&hdr));
    TEST_ASSERT_FALSE(syn_fwimage_is_bootable(&hdr));
}

void test_fwimage_corrupted_crc(void)
{
    SYN_FwImageHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = SYN_FW_MAGIC;
    hdr.version_code = 0x00010000;
    hdr.state = SYN_FW_STATE_CONFIRMED;
    syn_fwimage_seal_header(&hdr);

    /* Corrupt the version */
    hdr.version_code = 0x00020000;
    TEST_ASSERT_FALSE(syn_fwimage_header_valid(&hdr));
}

void test_fwimage_bootable_states(void)
{
    SYN_FwImageHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = SYN_FW_MAGIC;

    /* CONFIRMED = bootable */
    hdr.state = SYN_FW_STATE_CONFIRMED;
    syn_fwimage_seal_header(&hdr);
    TEST_ASSERT_TRUE(syn_fwimage_is_bootable(&hdr));

    /* NEW = bootable */
    hdr.state = SYN_FW_STATE_NEW;
    syn_fwimage_seal_header(&hdr);
    TEST_ASSERT_TRUE(syn_fwimage_is_bootable(&hdr));

    /* TESTING = bootable */
    hdr.state = SYN_FW_STATE_TESTING;
    syn_fwimage_seal_header(&hdr);
    TEST_ASSERT_TRUE(syn_fwimage_is_bootable(&hdr));

    /* WRITING = not bootable */
    hdr.state = SYN_FW_STATE_WRITING;
    syn_fwimage_seal_header(&hdr);
    TEST_ASSERT_FALSE(syn_fwimage_is_bootable(&hdr));

    /* INVALID = not bootable */
    hdr.state = SYN_FW_STATE_INVALID;
    syn_fwimage_seal_header(&hdr);
    TEST_ASSERT_FALSE(syn_fwimage_is_bootable(&hdr));
}

/* ── Firmware update tests ──────────────────────────────────────────────── */

void test_fwupdate_basic(void)
{
    mock_port_reset();

    static uint8_t page_buf[64];
    SYN_FwUpdate upd;

    /* Create a small firmware image */
    uint8_t firmware[128];
    for (int i = 0; i < 128; i++)
        firmware[i] = (uint8_t)i;

    uint32_t expected_crc = syn_crc32(firmware, sizeof(firmware));

    SYN_Status st = syn_fwupdate_begin(&upd, SLOT_A_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf));
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_TRUE(syn_fwupdate_active(&upd));

    /* Write in chunks */
    st = syn_fwupdate_write(&upd, firmware, 50);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    st = syn_fwupdate_write(&upd, firmware + 50, 78);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    TEST_ASSERT_EQUAL(128, syn_fwupdate_progress(&upd) + upd.page_buf_used);

    st = TEST_FWUPDATE_FINISH(&upd, expected_crc, 0x00010200);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_FALSE(syn_fwupdate_active(&upd));

    /* Verify the header in flash */
    SYN_FwImageHeader hdr;
    syn_port_flash_read(SLOT_A_ADDR, &hdr, sizeof(hdr));
    TEST_ASSERT_TRUE(syn_fwimage_header_valid(&hdr));
    TEST_ASSERT_EQUAL(SYN_FW_STATE_NEW, hdr.state);
    TEST_ASSERT_EQUAL_HEX32(0x00010200, hdr.version_code);
    TEST_ASSERT_EQUAL(128, hdr.image_size);
    TEST_ASSERT_EQUAL_HEX32(expected_crc, hdr.image_crc);

    /* Verify the firmware payload was preserved in the payload sector */
    uint8_t readback[128] = {0};
    syn_port_flash_read(SLOT_A_ADDR + syn_fwimage_data_offset(SLOT_A_ADDR), readback,
                        sizeof(readback));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(firmware, readback, sizeof(firmware));
}

void test_fwupdate_multi_sector(void)
{
    mock_port_reset();

    static uint8_t page_buf[256];
    SYN_FwUpdate upd;

    static uint8_t fw_large[1500];
    memset(fw_large, 0x55, sizeof(fw_large));
    uint32_t expected_crc = syn_crc32(fw_large, sizeof(fw_large));

    SYN_Status st = syn_fwupdate_begin(&upd, SLOT_A_ADDR, 2048, page_buf, sizeof(page_buf));
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Write 1500 bytes in 256 byte chunks */
    for (size_t i = 0; i < sizeof(fw_large); i += 256) {
        size_t len = (sizeof(fw_large) - i > 256) ? 256 : (sizeof(fw_large) - i);
        TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_write(&upd, fw_large + i, len));
    }

    st = TEST_FWUPDATE_FINISH(&upd, expected_crc, 0x00020000);
    TEST_ASSERT_EQUAL(SYN_OK, st);
}

void test_fwupdate_crc_mismatch(void)
{
    mock_port_reset();

    static uint8_t page_buf[64];
    SYN_FwUpdate upd;

    uint8_t firmware[32];
    memset(firmware, 0xAA, sizeof(firmware));

    syn_fwupdate_begin(&upd, SLOT_A_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf));
    syn_fwupdate_write(&upd, firmware, sizeof(firmware));

    /* Provide wrong CRC */
    SYN_Status st = TEST_FWUPDATE_FINISH(&upd, 0xBADBAD00, 0x00010000);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);

    /* Slot should be marked INVALID */
    SYN_FwImageHeader hdr;
    syn_port_flash_read(SLOT_A_ADDR, &hdr, sizeof(hdr));
    TEST_ASSERT_EQUAL(SYN_FW_STATE_INVALID, hdr.state);
}

void test_fwupdate_abort(void)
{
    mock_port_reset();

    static uint8_t page_buf[64];
    SYN_FwUpdate upd;

    syn_fwupdate_begin(&upd, SLOT_A_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf));

    uint8_t chunk[16] = {0};
    syn_fwupdate_write(&upd, chunk, sizeof(chunk));

    syn_fwupdate_abort(&upd);

    TEST_ASSERT_FALSE(syn_fwupdate_active(&upd));

    SYN_FwImageHeader hdr;
    syn_port_flash_read(SLOT_A_ADDR, &hdr, sizeof(hdr));
    TEST_ASSERT_EQUAL(SYN_FW_STATE_INVALID, hdr.state);
}

/* ── A/B Boot tests ─────────────────────────────────────────────────────── */

/** Helper: write a valid image header to flash at the given address. */
static void write_test_header(uint32_t addr, uint8_t state, uint32_t version)
{
    SYN_FwImageHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = SYN_FW_MAGIC;
    hdr.version_code = version;
    hdr.image_size = 64;
    hdr.image_crc = 0xDEADBEEF;
    hdr.state = state;
    syn_fwimage_seal_header(&hdr);

    syn_port_flash_erase(addr);
    syn_port_flash_write(addr, &hdr, sizeof(hdr));
}

void test_fwboot_select_confirmed(void)
{
    mock_port_reset();

    /* Slot A: confirmed v1.0.0, Slot B: empty */
    write_test_header(SLOT_A_ADDR, SYN_FW_STATE_CONFIRMED, 0x00010000);

    SYN_FwBootManager mgr;
    syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR);

    uint8_t slot = syn_fwboot_select(&mgr, false);
    TEST_ASSERT_EQUAL(SYN_FW_SLOT_A, slot);
}

void test_fwboot_select_new_over_confirmed(void)
{
    mock_port_reset();

    /* Slot A: confirmed v1.0.0, Slot B: NEW v2.0.0 */
    write_test_header(SLOT_A_ADDR, SYN_FW_STATE_CONFIRMED, 0x00010000);
    write_test_header(SLOT_B_ADDR, SYN_FW_STATE_NEW, 0x00020000);

    SYN_FwBootManager mgr;
    syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR);

    uint8_t slot = syn_fwboot_select(&mgr, false);
    TEST_ASSERT_EQUAL(SYN_FW_SLOT_B, slot);

    /* After select, NEW should be promoted to TESTING */
    SYN_FwImageHeader hdr;
    syn_port_flash_read(SLOT_B_ADDR, &hdr, sizeof(hdr));
    TEST_ASSERT_EQUAL(SYN_FW_STATE_TESTING, hdr.state);
}

void test_fwboot_rollback(void)
{
    mock_port_reset();

    /* Slot A: CONFIRMED v1.0.0, Slot B: TESTING v2.0.0 (failed) */
    write_test_header(SLOT_A_ADDR, SYN_FW_STATE_CONFIRMED, 0x00010000);
    write_test_header(SLOT_B_ADDR, SYN_FW_STATE_TESTING, 0x00020000);

    SYN_FwBootManager mgr;
    syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR);

    /* Rollback: TESTING slot should be invalidated */
    uint8_t slot = syn_fwboot_select(&mgr, true);
    TEST_ASSERT_EQUAL(SYN_FW_SLOT_A, slot);

    /* Slot B should now be INVALID */
    SYN_FwImageHeader hdr;
    syn_port_flash_read(SLOT_B_ADDR, &hdr, sizeof(hdr));
    TEST_ASSERT_EQUAL(SYN_FW_STATE_INVALID, hdr.state);
}

void test_fwboot_confirm(void)
{
    mock_port_reset();

    /* Slot A: TESTING v2.0.0 */
    write_test_header(SLOT_A_ADDR, SYN_FW_STATE_TESTING, 0x00020000);

    SYN_FwBootManager mgr;
    syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR);
    syn_fwboot_select(&mgr, false);

    SYN_Status st = syn_fwboot_confirm(&mgr);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    SYN_FwImageHeader hdr;
    syn_port_flash_read(SLOT_A_ADDR, &hdr, sizeof(hdr));
    TEST_ASSERT_EQUAL(SYN_FW_STATE_CONFIRMED, hdr.state);
}

void test_fwboot_higher_version_wins(void)
{
    mock_port_reset();

    /* Both confirmed, slot B has higher version */
    write_test_header(SLOT_A_ADDR, SYN_FW_STATE_CONFIRMED, 0x00010000);
    write_test_header(SLOT_B_ADDR, SYN_FW_STATE_CONFIRMED, 0x00020000);

    SYN_FwBootManager mgr;
    syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR);

    uint8_t slot = syn_fwboot_select(&mgr, false);
    TEST_ASSERT_EQUAL(SYN_FW_SLOT_B, slot);
}

/* ── Runner ─────────────────────────────────────────────────────────────── */

/** flash_erase fails in begin — exercises lines 77-79 */
static void test_fwupdate_erase_fail(void)
{
    mock_port_reset();
    static uint8_t pbuf[64];
    SYN_FwUpdate upd;

    /* Make erase fail at SLOT_A_ADDR */
    mock_flash_fail_at = SLOT_A_ADDR;
    SYN_Status st = syn_fwupdate_begin(&upd, SLOT_A_ADDR, SLOT_SIZE, pbuf, sizeof(pbuf));
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_TRUE(upd.error);
    TEST_ASSERT_FALSE(upd.active);
}

/** flash_write header fails in finish */
static void test_fwupdate_write_header_fail(void)
{
    mock_port_reset();
    static uint8_t pbuf[64];
    SYN_FwUpdate upd;

    SYN_Status st = syn_fwupdate_begin(&upd, SLOT_A_ADDR, SLOT_SIZE, pbuf, sizeof(pbuf));
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Fail the header write in finish() */
    mock_flash_write_fail_next = true;
    uint32_t crc = syn_crc32_final(SYN_CRC32_INIT);
    st = TEST_FWUPDATE_FINISH(&upd, crc, 1);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_TRUE(upd.error);
    TEST_ASSERT_FALSE(upd.active);
}

/** flush_page write fails during write — exercises lines 128-129 */
static void test_fwupdate_write_flush_fail(void)
{
    mock_port_reset();
    static uint8_t pbuf[4]; /* small page buffer so it fills quickly */
    SYN_FwUpdate upd;

    SYN_Status st = syn_fwupdate_begin(&upd, SLOT_A_ADDR, SLOT_SIZE, pbuf, sizeof(pbuf));
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Fill exactly the page buffer to trigger a flush — then fail the write */
    mock_flash_write_fail_next = true;
    uint8_t data[4] = {0x01, 0x02, 0x03, 0x04};
    st = syn_fwupdate_write(&upd, data, sizeof(data));
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_TRUE(upd.error);
}

/** flush_page write fails in finish — exercises lines 147-148 */
static void test_fwupdate_finish_flush_fail(void)
{
    mock_port_reset();
    static uint8_t pbuf[64];
    SYN_FwUpdate upd;

    SYN_Status st = syn_fwupdate_begin(&upd, SLOT_A_ADDR, SLOT_SIZE, pbuf, sizeof(pbuf));
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Write some data (doesn't fill page buffer yet) */
    uint8_t data[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
    st = syn_fwupdate_write(&upd, data, sizeof(data));
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Fail the flush write that happens inside finish() */
    mock_flash_write_fail_next = true;
    uint32_t crc = syn_crc32_final(syn_crc32_update(SYN_CRC32_INIT, data, sizeof(data)));
    st = TEST_FWUPDATE_FINISH(&upd, crc, 1);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_TRUE(upd.error);
}

/** write exceeds max_size — exercises lines 106-107 */
static void test_fwupdate_write_overflow(void)
{
    mock_port_reset();
    static uint8_t pbuf[64];
    SYN_FwUpdate upd;

    SYN_Status st = syn_fwupdate_begin(&upd, SLOT_A_ADDR, 32, pbuf, sizeof(pbuf));
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Write more than max_size (32) */
    uint8_t data[100];
    memset(data, 0xAB, sizeof(data));
    st = syn_fwupdate_write(&upd, data, sizeof(data));
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_TRUE(upd.error);
}

/** syn_fwboot_refresh — exercises lines 149-155 */
static void test_fwboot_refresh(void)
{
    SYN_FwBootManager mgr;
    syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR);
    SYN_Status st = syn_fwboot_refresh(&mgr);
    TEST_ASSERT_EQUAL(SYN_OK, st);
}
/** Write: sector-boundary erase fails — exercises lines 39-40 */
static void test_fwupdate_sector_erase_fail(void)
{
    mock_port_reset();
    static uint8_t pbuf[64];
    SYN_FwUpdate upd;

    SYN_Status st = syn_fwupdate_begin(&upd, SLOT_A_ADDR, 2048u, pbuf, sizeof(pbuf));
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Write enough to fill first payload sector [1024, 2048) */
    size_t fill = (size_t)MOCK_FLASH_SECTOR;
    uint8_t chunk[64];
    memset(chunk, 0xAA, sizeof(chunk));

    while (fill > 0) {
        size_t n = fill > sizeof(chunk) ? sizeof(chunk) : fill;
        st = syn_fwupdate_write(&upd, chunk, n);
        if (st != SYN_OK) {
            break;
        }
        fill -= n;
    }
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Next write crosses into sector at addr 2048; make erase fail there */
    mock_flash_fail_at = 2048;
    st = syn_fwupdate_write(&upd, chunk, sizeof(chunk));
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
}

static void test_fwupdate_parameter_and_state_guards(void)
{
    mock_port_reset();
    static uint8_t pbuf[64];
    SYN_FwUpdate upd;

    /* Write/finish when inactive or len==0 (lines 111, 113, 164) */
    memset(&upd, 0, sizeof(upd));
    upd.active = false;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwupdate_write(&upd, pbuf, 10));
    TEST_ASSERT_EQUAL(SYN_ERROR, TEST_FWUPDATE_FINISH(&upd, 0, 0));

    syn_fwupdate_begin(&upd, SLOT_A_ADDR, SLOT_SIZE, pbuf, sizeof(pbuf));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_write(&upd, NULL, 0));

    /* 3. Flash write error on final header in syn_fwupdate_finish (lines 235-237) */
    uint8_t fw[32] = {0};
    syn_fwupdate_write(&upd, fw, sizeof(fw));
    uint32_t crc = syn_crc32(fw, sizeof(fw));
    mock_flash_fail_at = SLOT_A_ADDR;
    TEST_ASSERT_EQUAL(SYN_ERROR, TEST_FWUPDATE_FINISH(&upd, crc, 0x00010000));
    mock_flash_fail_at = -1;

    /* 4. Flush page flash write failure (lines 47-49) */
    mock_port_reset();
    syn_fwupdate_begin(&upd, SLOT_A_ADDR, SLOT_SIZE, pbuf, sizeof(pbuf));
    uint8_t fw64[64] = {0};
    mock_flash_write_fail_next = true;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwupdate_write(&upd, fw64, sizeof(fw64)));

    /* Inactive abort test */
    memset(&upd, 0, sizeof(upd));
    syn_fwupdate_abort(&upd);

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    /* HMAC test with key */
    mock_port_reset();
    syn_fwupdate_begin(&upd, SLOT_A_ADDR, SLOT_SIZE, pbuf, sizeof(pbuf));
    uint8_t hmac_key[16] = {0x01, 0x02, 0x03};
    syn_fwupdate_set_key(&upd, hmac_key, sizeof(hmac_key));
    uint8_t chunk[16] = {0xAA};
    syn_fwupdate_write(&upd, chunk, sizeof(chunk));
    syn_fwupdate_abort(&upd);
#endif

    mock_port_reset();
}

static void test_fwboot_testing_priority_and_confirm_failures(void)
{
    mock_port_reset();
    write_test_header(SLOT_A_ADDR, SYN_FW_STATE_TESTING, 0x00010000);

    SYN_FwBootManager mgr;
    syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR);

    /* Priority 1: TESTING slot selection (lines 94-97) */
    TEST_ASSERT_EQUAL(SYN_FW_SLOT_A, syn_fwboot_select(&mgr, false));

    /* Confirm failure when already confirmed (line 136) */
    write_test_header(SLOT_A_ADDR, SYN_FW_STATE_CONFIRMED, 0x00010000);
    syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR);
    syn_fwboot_select(&mgr, false);
    /* Confirm failure when active_slot == NONE (line 131) */
    mgr.active_slot = SYN_FW_SLOT_NONE;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwboot_confirm(&mgr));

    /* Flash erase failure branch in update_state_in_flash (line 48) */
    write_test_header(SLOT_A_ADDR, SYN_FW_STATE_TESTING, 0x00010000);
    syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR);
    syn_fwboot_select(&mgr, false);
    mock_flash_fail_at = SLOT_A_ADDR;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwboot_confirm(&mgr));
    mock_flash_fail_at = 0;
}

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
void test_fwupdate_aes_gcm_streaming_decryption_and_tamper(void)
{
    mock_port_reset();
    static uint8_t page_buf[256];
    SYN_FwUpdate upd;

    static const uint8_t key[32] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                                    0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
                                    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    static const uint8_t iv[12] = {0xCA, 0xFE, 0xBA, 0xBE, 0xFA, 0xCE,
                                   0xDB, 0xAD, 0xDE, 0xCA, 0xF8, 0x88};

    /* 1. Parameter guards on set_aes_gcm_key */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_fwupdate_set_aes_gcm_key(NULL, key, sizeof(key), iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_fwupdate_set_aes_gcm_key(&upd, NULL, sizeof(key), iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_fwupdate_set_aes_gcm_key(&upd, key, sizeof(key), NULL, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_fwupdate_set_aes_gcm_key(&upd, key, sizeof(key), iv, 11));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_fwupdate_set_aes_gcm_key(&upd, key, 15, iv, 12));

    /* 2. Create 300-byte plaintext test image */
    uint8_t plaintext[300];
    for (size_t i = 0; i < sizeof(plaintext); i++) {
        plaintext[i] = (uint8_t)(i ^ 0x5A);
    }

    uint8_t ciphertext[sizeof(plaintext)];
    uint8_t tag[16];
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_aes_gcm_encrypt(NULL, 0, 0, 0, 0, 0, 0, 0, 0)); /* null guard */

    SYN_AES_GCM_Context gcm_ctx;
    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_gcm_init(&gcm_ctx, key, sizeof(key)));
    uint8_t auth_meta_v2[SYN_FWIMAGE_AUTH_META_SIZE];
    syn_fwimage_encode_auth_meta(0x00020000U, (uint32_t)sizeof(plaintext), auth_meta_v2);
    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_gcm_encrypt(&gcm_ctx, iv, sizeof(iv), auth_meta_v2,
                                                  sizeof(auth_meta_v2), plaintext,
                                                  sizeof(plaintext), ciphertext, tag));

    /* 3. Begin updater on Slot B + test metadata bind parameter/state guards */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_fwupdate_bind_aes_gcm_meta(NULL, 0x00020000U, sizeof(plaintext)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_begin(&upd, SLOT_B_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf)));
    /* Bind before set_aes_gcm_key must fail */
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_fwupdate_bind_aes_gcm_meta(&upd, 0x00020000U, sizeof(plaintext)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_set_aes_gcm_key(&upd, key, sizeof(key), iv, sizeof(iv)));
    /* finish_gcm or write before bind_aes_gcm_meta must fail */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwupdate_finish_gcm(&upd, tag, 0x00020000U));
    SYN_FwUpdate upd_unbound;
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_begin(&upd_unbound, SLOT_A_ADDR, SLOT_SIZE, page_buf,
                                                 sizeof(page_buf)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_set_aes_gcm_key(&upd_unbound, key, sizeof(key), iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwupdate_write(&upd_unbound, ciphertext, 16U));
    TEST_ASSERT_TRUE(upd_unbound.error);

    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_bind_aes_gcm_meta(&upd, 0x00020000U, sizeof(plaintext)));
    /* Duplicate bind must fail */
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_fwupdate_bind_aes_gcm_meta(&upd, 0x00020000U, sizeof(plaintext)));

    /* 4. Stream ciphertext in irregular chunk sizes (17, 33, 100, 150) */
    size_t chunk_sizes[] = {17, 33, 100, 150};
    size_t offset = 0;
    for (size_t c = 0; c < sizeof(chunk_sizes) / sizeof(chunk_sizes[0]); c++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_write(&upd, ciphertext + offset, chunk_sizes[c]));
        offset += chunk_sizes[c];
    }
    TEST_ASSERT_EQUAL(sizeof(plaintext), offset);

    /* 5. Finish with valid tag -> must succeed */
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_finish_gcm(&upd, tag, 0x00020000U));
    TEST_ASSERT_FALSE(syn_fwupdate_active(&upd));

    /* 6. Verify written image header and decrypted payload in flash */
    SYN_FwImageHeader hdr;
    syn_port_flash_read(SLOT_B_ADDR, &hdr, sizeof(hdr));
    TEST_ASSERT_TRUE(syn_fwimage_header_valid(&hdr));
    TEST_ASSERT_EQUAL(0x00020000U, hdr.version_code);
    TEST_ASSERT_EQUAL(sizeof(plaintext), hdr.image_size);
    TEST_ASSERT_EQUAL(SYN_FW_STATE_NEW, hdr.state);

    uint8_t readback[sizeof(plaintext)];
    syn_port_flash_read(SLOT_B_ADDR + syn_fwimage_data_offset(SLOT_B_ADDR), readback,
                        sizeof(readback));
    TEST_ASSERT_EQUAL_MEMORY(plaintext, readback, sizeof(plaintext));

    /* 7. Test bad tag on separate update (Slot A) -> must fail and abort */
    SYN_FwUpdate upd_bad;
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_fwupdate_begin(&upd_bad, SLOT_A_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_set_aes_gcm_key(&upd_bad, key, sizeof(key), iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_bind_aes_gcm_meta(&upd_bad, 0x00020000U, sizeof(plaintext)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_write(&upd_bad, ciphertext, sizeof(ciphertext)));

    uint8_t bad_tag[16];
    memcpy(bad_tag, tag, 16);
    bad_tag[0] ^= 0x01;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwupdate_finish_gcm(&upd_bad, bad_tag, 0x00020000U));
    TEST_ASSERT_FALSE(syn_fwupdate_active(&upd_bad));

    /* 7b. SYS-03: Valid tag for v2 finished with tampered version v9 -> must fail and abort */
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_fwupdate_begin(&upd_bad, SLOT_A_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_set_aes_gcm_key(&upd_bad, key, sizeof(key), iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_bind_aes_gcm_meta(&upd_bad, 0x00020000U, sizeof(plaintext)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_write(&upd_bad, ciphertext, sizeof(ciphertext)));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwupdate_finish_gcm(&upd_bad, tag, 0x00090000U));
    TEST_ASSERT_FALSE(syn_fwupdate_active(&upd_bad));

    /* 7c. SYS-03: Truncated ciphertext stream (bytes_written != gcm_bound_size) -> must fail */
    uint8_t trunc_ct[64];
    uint8_t trunc_tag[16];
    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_gcm_encrypt(&gcm_ctx, iv, sizeof(iv), auth_meta_v2,
                                                  sizeof(auth_meta_v2), plaintext, sizeof(trunc_ct),
                                                  trunc_ct, trunc_tag));
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_fwupdate_begin(&upd_bad, SLOT_A_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_set_aes_gcm_key(&upd_bad, key, sizeof(key), iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_bind_aes_gcm_meta(&upd_bad, 0x00020000U, sizeof(plaintext)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_write(&upd_bad, trunc_ct, sizeof(trunc_ct)));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwupdate_finish_gcm(&upd_bad, trunc_tag, 0x00020000U));

    /* 8. HMAC + AES-GCM combined */
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    SYN_FwUpdate upd_hmac_gcm;
    uint8_t auth_meta_v3[SYN_FWIMAGE_AUTH_META_SIZE];
    uint8_t tag_v3[16];
    syn_fwimage_encode_auth_meta(0x00030000U, (uint32_t)sizeof(plaintext), auth_meta_v3);
    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_gcm_encrypt(&gcm_ctx, iv, sizeof(iv), auth_meta_v3,
                                                  sizeof(auth_meta_v3), plaintext,
                                                  sizeof(plaintext), ciphertext, tag_v3));
    mock_port_reset();
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_begin(&upd_hmac_gcm, SLOT_A_ADDR, SLOT_SIZE, page_buf,
                                                 sizeof(page_buf)));
    syn_fwupdate_set_key(&upd_hmac_gcm, key, 32);
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_fwupdate_set_aes_gcm_key(&upd_hmac_gcm, key, sizeof(key), iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_fwupdate_bind_aes_gcm_meta(&upd_hmac_gcm, 0x00030000U, sizeof(plaintext)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_write(&upd_hmac_gcm, ciphertext, sizeof(ciphertext)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_finish_gcm(&upd_hmac_gcm, tag_v3, 0x00030000U));

    uint8_t expected_gcm_hmac[32];
    SYN_HMAC_SHA256 gcm_hmac_ctx;
    syn_hmac_sha256_init(&gcm_hmac_ctx, key, 32);
    syn_hmac_sha256_update(&gcm_hmac_ctx, plaintext, sizeof(plaintext));
    syn_hmac_sha256_update(&gcm_hmac_ctx, auth_meta_v3, sizeof(auth_meta_v3));
    syn_hmac_sha256_final(&gcm_hmac_ctx, expected_gcm_hmac);
    syn_port_flash_read(SLOT_A_ADDR, &hdr, sizeof(hdr));
    TEST_ASSERT_EQUAL_MEMORY(expected_gcm_hmac, hdr.image_hmac, sizeof(expected_gcm_hmac));
#endif

    /* 9. Flash error during GCM write page flush */
    SYN_FwUpdate upd_fail;
    uint8_t auth_meta_v4[SYN_FWIMAGE_AUTH_META_SIZE];
    uint8_t tag_v4[16];
    syn_fwimage_encode_auth_meta(0x00040000U, (uint32_t)sizeof(plaintext), auth_meta_v4);
    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_gcm_encrypt(&gcm_ctx, iv, sizeof(iv), auth_meta_v4,
                                                  sizeof(auth_meta_v4), plaintext,
                                                  sizeof(plaintext), ciphertext, tag_v4));
    mock_port_reset();
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_fwupdate_begin(&upd_fail, SLOT_A_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_set_aes_gcm_key(&upd_fail, key, sizeof(key), iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_bind_aes_gcm_meta(&upd_fail, 0x00040000U, sizeof(plaintext)));
    mock_flash_fail_at = (int32_t)(SLOT_A_ADDR + syn_fwimage_data_offset(SLOT_A_ADDR));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwupdate_write(&upd_fail, ciphertext, sizeof(page_buf) + 16));
    mock_flash_fail_at = -1;

    /* 10. Flash error during finish_gcm page flush */
    mock_port_reset();
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_fwupdate_begin(&upd_fail, SLOT_A_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_set_aes_gcm_key(&upd_fail, key, sizeof(key), iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_bind_aes_gcm_meta(&upd_fail, 0x00040000U, sizeof(plaintext)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_write(&upd_fail, ciphertext, 64));
    mock_flash_fail_at = (int32_t)(SLOT_A_ADDR + syn_fwimage_data_offset(SLOT_A_ADDR));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwupdate_finish_gcm(&upd_fail, tag_v4, 0x00040000U));
    mock_flash_fail_at = -1;

    /* 11. Flash error during finish_gcm header write */
    mock_port_reset();
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_fwupdate_begin(&upd_fail, SLOT_A_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_set_aes_gcm_key(&upd_fail, key, sizeof(key), iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_bind_aes_gcm_meta(&upd_fail, 0x00040000U, sizeof(plaintext)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_write(&upd_fail, ciphertext, sizeof(ciphertext)));
    mock_flash_fail_at = SLOT_A_ADDR;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwupdate_finish_gcm(&upd_fail, tag_v4, 0x00040000U));
    TEST_ASSERT_FALSE(syn_fwupdate_active(&upd_fail));
    mock_flash_fail_at = -1;

    /* 12. Boundary guards on finish_gcm + AUD-M8 direct syn_fwupdate_finish rejection */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwupdate_finish_gcm(NULL, tag, 0x00020000U));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwupdate_finish_gcm(&upd, NULL, 0x00020000U));
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_fwupdate_finish_gcm(&upd, tag, 0x00020000U)); /* already inactive */

    SYN_FwUpdate upd_bypass;
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_begin(&upd_bypass, SLOT_A_ADDR, SLOT_SIZE, page_buf,
                                                 sizeof(page_buf)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_set_aes_gcm_key(&upd_bypass, key, sizeof(key), iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fwupdate_bind_aes_gcm_meta(&upd_bypass, 0x00020000U, sizeof(plaintext)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_write(&upd_bypass, ciphertext, sizeof(ciphertext)));
    TEST_ASSERT_EQUAL(
        SYN_ERROR,
        TEST_FWUPDATE_FINISH(&upd_bypass, syn_crc32(plaintext, sizeof(plaintext)), 0x00020000U));
    TEST_ASSERT_FALSE(syn_fwupdate_active(&upd_bypass));
}
#endif

static void test_r5_fwboot_and_fwupdate_regressions(void)
{
    mock_port_reset();
    static uint8_t unaligned_buf[73];
    SYN_FwUpdate upd;

    /* 1. Parameter guards on syn_fwupdate_begin */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_fwupdate_begin(NULL, SLOT_A_ADDR, SLOT_SIZE, unaligned_buf, 73U));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_fwupdate_begin(&upd, SLOT_A_ADDR, SLOT_SIZE, NULL, 73U));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_fwupdate_begin(&upd, SLOT_A_ADDR, SLOT_SIZE, unaligned_buf, 0U));

    /* 2. Dirty entire flash with 0xAA and stream multi-sector unaligned flushes */
    memset(mock_flash, 0xAA, sizeof(mock_flash));
    static uint8_t fw_img[1530];
    for (size_t i = 0U; i < sizeof(fw_img); i++) {
        fw_img[i] = (uint8_t)((i * 7U) ^ 0xC3U);
    }
    uint32_t fw_crc = syn_crc32(fw_img, sizeof(fw_img));

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_fwupdate_begin(&upd, SLOT_A_ADDR, 2048U, unaligned_buf, sizeof(unaligned_buf)));
    const size_t chunks[] = {19U, 88U, 311U, 700U, 412U};
    size_t pos = 0U;
    for (size_t c = 0U; c < sizeof(chunks) / sizeof(chunks[0]); c++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_write(&upd, fw_img + pos, chunks[c]));
        pos += chunks[c];
    }
    TEST_ASSERT_EQUAL(sizeof(fw_img), pos);
    TEST_ASSERT_EQUAL(SYN_OK, TEST_FWUPDATE_FINISH(&upd, fw_crc, 0x00030100U));

    static uint8_t readback[1530];
    memset(readback, 0, sizeof(readback));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_port_flash_read(SLOT_A_ADDR + syn_fwimage_data_offset(SLOT_A_ADDR),
                                          readback, sizeof(readback)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(fw_img, readback, sizeof(fw_img));

    /* 3. Verify payload survives select (NEW -> TESTING) and confirm (TESTING -> CONFIRMED) */
    memset(mock_flash, 0xAA, sizeof(mock_flash));
    uint8_t slot_fw[512];
    for (size_t i = 0U; i < sizeof(slot_fw); i++) {
        slot_fw[i] = (uint8_t)(0xFFU - (uint8_t)i);
    }
    uint32_t slot_crc = syn_crc32(slot_fw, sizeof(slot_fw));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_begin(&upd, SLOT_A_ADDR, SLOT_SIZE, unaligned_buf,
                                                 sizeof(unaligned_buf)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_write(&upd, slot_fw, sizeof(slot_fw)));
    TEST_ASSERT_EQUAL(SYN_OK, TEST_FWUPDATE_FINISH(&upd, slot_crc, 0x00040000U));

    SYN_FwBootManager mgr;
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR));
    TEST_ASSERT_EQUAL_UINT32(SLOT_A_ADDR, syn_fwboot_slot_addr(&mgr, SYN_FW_SLOT_A));
    TEST_ASSERT_EQUAL_UINT32(0U, syn_fwboot_slot_addr(&mgr, 2U));
    TEST_ASSERT_EQUAL_UINT32(0U, syn_fwboot_slot_addr(NULL, SYN_FW_SLOT_A));
    TEST_ASSERT_EQUAL_UINT32(SLOT_A_ADDR + MOCK_FLASH_SECTOR,
                             syn_fwboot_vector_addr(&mgr, SYN_FW_SLOT_A));
    TEST_ASSERT_EQUAL_UINT32(0U, syn_fwboot_vector_addr(&mgr, 2U));
    TEST_ASSERT_EQUAL_UINT32(0U, syn_fwboot_vector_addr(NULL, SYN_FW_SLOT_A));
    TEST_ASSERT_NOT_NULL(syn_fwboot_slot_header(&mgr, SYN_FW_SLOT_A));
    TEST_ASSERT_NULL(syn_fwboot_slot_header(&mgr, 2U));
    TEST_ASSERT_NULL(syn_fwboot_slot_header(NULL, SYN_FW_SLOT_A));
    TEST_ASSERT_EQUAL_UINT8(SYN_FW_SLOT_NONE, syn_fwboot_active_slot(NULL));

    TEST_ASSERT_EQUAL_UINT8(SYN_FW_SLOT_A, syn_fwboot_select(&mgr, false));
    TEST_ASSERT_EQUAL_UINT8(SYN_FW_SLOT_A, syn_fwboot_active_slot(&mgr));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwboot_confirm(&mgr));
    /* Confirming an already CONFIRMED slot fails cleanly */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwboot_confirm(&mgr));

    uint8_t verify_fw[512] = {0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_port_flash_read(syn_fwboot_vector_addr(&mgr, SYN_FW_SLOT_A),
                                                  verify_fw, sizeof(verify_fw)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(slot_fw, verify_fw, sizeof(slot_fw));
    TEST_ASSERT_EQUAL_HEX32(slot_crc, syn_crc32(verify_fw, sizeof(verify_fw)));

    /* 4. Flash I/O failure propagation in syn_fwboot_init, refresh, select, confirm */
    write_test_header(SLOT_A_ADDR, SYN_FW_STATE_CONFIRMED, 0x00010000U);
    write_test_header(SLOT_B_ADDR, SYN_FW_STATE_NEW, 0x00020000U);

    mock_flash_fail_at = (int32_t)SLOT_B_ADDR;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR));
    /* Slot B header read failed -> invalidated, so Slot A (CONFIRMED) is selected */
    TEST_ASSERT_EQUAL_UINT8(SYN_FW_SLOT_A, syn_fwboot_select(&mgr, false));

    mock_flash_fail_at = (int32_t)SLOT_A_ADDR;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwboot_refresh(&mgr));

    /* NEW -> TESTING write failure falls back to healthy CONFIRMED slot */
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR));
    mock_flash_write_fail_next = true;
    TEST_ASSERT_EQUAL_UINT8(SYN_FW_SLOT_A, syn_fwboot_select(&mgr, false));

    /* Confirm write failure returns SYN_ERROR and marks cached state INVALID */
    write_test_header(SLOT_B_ADDR, SYN_FW_STATE_TESTING, 0x00020000U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR));
    TEST_ASSERT_EQUAL_UINT8(SYN_FW_SLOT_B, syn_fwboot_select(&mgr, false));
    mock_flash_write_fail_next = true;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwboot_confirm(&mgr));
    TEST_ASSERT_EQUAL(SYN_FW_STATE_INVALID, mgr.slot_hdr[SYN_FW_SLOT_B].state);

    mock_port_reset();
}

/**
 * @brief Verify syn_fwupdate_write prevents size_t wraparound on huge chunk lengths.
 */
static void test_tier_a_fwupdate_len_wraparound(void)
{
    mock_port_reset();
    static uint8_t pbuf[64];
    SYN_FwUpdate upd;
    uint8_t chunk[16] = {0x55U};

    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_begin(&upd, SLOT_A_ADDR, SLOT_SIZE, pbuf, sizeof(pbuf)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwupdate_write(&upd, chunk, sizeof(chunk)));

    /* SIZE_MAX length would wrap 32-bit (bytes_written + page_buf_used + len) */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_fwupdate_write(&upd, chunk, SIZE_MAX));
    TEST_ASSERT_TRUE(upd.error);
    mock_port_reset();
}

void run_fwupdate_tests(void)
{
    /* Image header */
    RUN_TEST(test_fwimage_seal_and_validate);
    RUN_TEST(test_fwimage_bad_magic);
    RUN_TEST(test_fwimage_corrupted_crc);
    RUN_TEST(test_fwimage_bootable_states);

    /* Streaming updater */
    RUN_TEST(test_fwupdate_basic);
    RUN_TEST(test_fwupdate_multi_sector);
    RUN_TEST(test_fwupdate_crc_mismatch);
    RUN_TEST(test_fwupdate_abort);

    /* A/B Boot */
    RUN_TEST(test_fwboot_select_confirmed);
    RUN_TEST(test_fwboot_select_new_over_confirmed);
    RUN_TEST(test_fwboot_rollback);
    RUN_TEST(test_fwboot_confirm);
    RUN_TEST(test_fwboot_higher_version_wins);
    RUN_TEST(test_fwboot_refresh);
    RUN_TEST(test_fwupdate_erase_fail);
    RUN_TEST(test_fwupdate_write_header_fail);
    RUN_TEST(test_fwupdate_write_overflow);
    RUN_TEST(test_fwupdate_write_flush_fail);
    RUN_TEST(test_fwupdate_finish_flush_fail);
    RUN_TEST(test_fwupdate_sector_erase_fail);
    RUN_TEST(test_fwupdate_parameter_and_state_guards);
    RUN_TEST(test_fwboot_testing_priority_and_confirm_failures);
    RUN_TEST(test_r5_fwboot_and_fwupdate_regressions);
#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
    RUN_TEST(test_fwupdate_aes_gcm_streaming_decryption_and_tamper);
#endif
    RUN_TEST(test_tier_a_fwupdate_len_wraparound);
}
