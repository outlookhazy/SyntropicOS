/**
 * @file test_fwupdate_hmac.c
 * @brief Tests for HMAC-signed firmware verification.
 */

#include "mocks/mock_port.h"
#include "syntropic/crypto/syn_hmac.h"
#include "syntropic/port/syn_port_flash.h"
#include "syntropic/system/syn_fwimage.h"
#include "syntropic/system/syn_fwupdate.h"
#include "syntropic/util/syn_crc.h"
#include "unity/unity.h"

#include <string.h>

#define SLOT_ADDR 0x0000u
#define SLOT_SIZE (2048u - (uint32_t)MOCK_FLASH_SECTOR)

static uint8_t page_buf[256];
static SYN_FwUpdate upd;

/* ── Helper: compute CRC and HMAC of test data ─────────────────────────── */

static const uint8_t test_key[] = "SyntropicSecretKey1234567890ABCD"; /* 32 bytes */

static void compute_crc_hmac(const uint8_t *data, size_t len, uint32_t *out_crc,
                             uint8_t out_hmac[32])
{
    *out_crc = syn_crc32_final(syn_crc32_update(SYN_CRC32_INIT, data, len));

    SYN_HMAC_SHA256 ctx;
    syn_hmac_sha256_init(&ctx, test_key, sizeof(test_key));
    syn_hmac_sha256_update(&ctx, data, len);
    syn_hmac_sha256_final(&ctx, out_hmac);
}

/* ── Tests ──────────────────────────────────────────────────────────────── */

void test_hmac_roundtrip(void)
{
    /* Create test firmware */
    uint8_t firmware[128];
    memset(firmware, 0x42, sizeof(firmware));

    uint32_t crc;
    uint8_t hmac[32];
    compute_crc_hmac(firmware, sizeof(firmware), &crc, hmac);

    /* begin, then set key (begin zeroes the struct) */
    memset(&upd, 0, sizeof(upd));

    SYN_Status st = syn_fwupdate_begin(&upd, SLOT_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf));
    TEST_ASSERT_EQUAL(SYN_OK, st);

    syn_fwupdate_set_key(&upd, test_key, sizeof(test_key));

    st = syn_fwupdate_write(&upd, firmware, sizeof(firmware));
    TEST_ASSERT_EQUAL(SYN_OK, st);

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    st = syn_fwupdate_finish(&upd, crc, hmac, NULL, 0x00010200);
#else
    st = syn_fwupdate_finish(&upd, crc, hmac, 0x00010200);
#endif
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_FALSE(syn_fwupdate_active(&upd));

    /* Verify the header was written with HMAC */
    SYN_FwImageHeader hdr;
    memcpy(&hdr, mock_flash + SLOT_ADDR, sizeof(hdr));
    TEST_ASSERT_EQUAL(SYN_FW_MAGIC, hdr.magic);
    TEST_ASSERT_EQUAL(SYN_FW_STATE_NEW, hdr.state);
    TEST_ASSERT_EQUAL_MEMORY(hmac, hdr.image_hmac, 32);
}

void test_hmac_mismatch_fails(void)
{
    uint8_t firmware[64];
    memset(firmware, 0xAA, sizeof(firmware));

    uint32_t crc;
    uint8_t hmac[32];
    compute_crc_hmac(firmware, sizeof(firmware), &crc, hmac);

    /* Corrupt the HMAC */
    uint8_t bad_hmac[32];
    memcpy(bad_hmac, hmac, 32);
    bad_hmac[0] ^= 0xFF;

    memset(&upd, 0, sizeof(upd));
    syn_fwupdate_begin(&upd, SLOT_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf));
    syn_fwupdate_set_key(&upd, test_key, sizeof(test_key));
    syn_fwupdate_write(&upd, firmware, sizeof(firmware));

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    SYN_Status st = syn_fwupdate_finish(&upd, crc, bad_hmac, NULL, 0x00010200);
#else
    SYN_Status st = syn_fwupdate_finish(&upd, crc, bad_hmac, 0x00010200);
#endif
    TEST_ASSERT_EQUAL(SYN_ERROR, st);

    /* Slot should be marked INVALID */
    SYN_FwImageHeader hdr;
    memcpy(&hdr, mock_flash + SLOT_ADDR, sizeof(hdr));
    TEST_ASSERT_EQUAL(SYN_FW_STATE_INVALID, hdr.state);
}

void test_hmac_null_skips_verification(void)
{
    /* R5-S03: With HMAC key configured, passing NULL for expected_hmac must fail closed. */
    uint8_t firmware[64];
    memset(firmware, 0xBB, sizeof(firmware));

    uint32_t crc = syn_crc32_final(syn_crc32_update(SYN_CRC32_INIT, firmware, sizeof(firmware)));

    memset(&upd, 0, sizeof(upd));
    syn_fwupdate_begin(&upd, SLOT_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf));
    syn_fwupdate_set_key(&upd, test_key, sizeof(test_key));
    syn_fwupdate_write(&upd, firmware, sizeof(firmware));

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    SYN_Status st = syn_fwupdate_finish(&upd, crc, NULL, NULL, 0x00010200);
#else
    SYN_Status st = syn_fwupdate_finish(&upd, crc, NULL, 0x00010200);
#endif
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_FALSE(syn_fwupdate_active(&upd));

    SYN_FwImageHeader hdr;
    memcpy(&hdr, mock_flash + SLOT_ADDR, sizeof(hdr));
    TEST_ASSERT_EQUAL(SYN_FW_STATE_INVALID, hdr.state);
}

void test_hmac_crc_only_without_key(void)
{
    /* When set_key is never called, HMAC is not computed. */
    uint8_t firmware[64];
    memset(firmware, 0xCC, sizeof(firmware));

    uint32_t crc = syn_crc32_final(syn_crc32_update(SYN_CRC32_INIT, firmware, sizeof(firmware)));

    memset(&upd, 0, sizeof(upd));
    /* No set_key call */
    syn_fwupdate_begin(&upd, SLOT_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf));
    syn_fwupdate_write(&upd, firmware, sizeof(firmware));

    /* Even with an expected_hmac provided, key_set is false so it passes */
    uint8_t any_hmac[32] = {0};
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    SYN_Status st = syn_fwupdate_finish(&upd, crc, any_hmac, NULL, 0x00010200);
#else
    SYN_Status st = syn_fwupdate_finish(&upd, crc, any_hmac, 0x00010200);
#endif
    TEST_ASSERT_EQUAL(SYN_OK, st);
}

void test_hmac_multi_chunk_write(void)
{
    /* Verify that HMAC is correct when data is written in multiple chunks. */
    uint8_t firmware[256];
    for (int i = 0; i < 256; i++)
        firmware[i] = (uint8_t)i;

    uint32_t crc;
    uint8_t hmac[32];
    compute_crc_hmac(firmware, sizeof(firmware), &crc, hmac);

    memset(&upd, 0, sizeof(upd));
    syn_fwupdate_begin(&upd, SLOT_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf));
    syn_fwupdate_set_key(&upd, test_key, sizeof(test_key));

    /* Write in 4 × 64-byte chunks */
    for (int i = 0; i < 4; i++) {
        syn_fwupdate_write(&upd, firmware + i * 64, 64);
    }

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    SYN_Status st = syn_fwupdate_finish(&upd, crc, hmac, NULL, 0x00010200);
#else
    SYN_Status st = syn_fwupdate_finish(&upd, crc, hmac, 0x00010200);
#endif
    TEST_ASSERT_EQUAL(SYN_OK, st);
}

/* ── Runner ─────────────────────────────────────────────────────────────── */

void run_fwupdate_hmac_tests(void)
{
    RUN_TEST(test_hmac_roundtrip);
    RUN_TEST(test_hmac_mismatch_fails);
    RUN_TEST(test_hmac_null_skips_verification);
    RUN_TEST(test_hmac_crc_only_without_key);
    RUN_TEST(test_hmac_multi_chunk_write);
}
