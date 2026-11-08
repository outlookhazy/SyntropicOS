/**
 * @file test_ota.c
 * @brief Comprehensive unit tests for Secure Streaming OTA Orchestrator (syn_ota).
 */

#include "mocks/mock_port.h"
#include "syntropic/crypto/syn_aes.h"
#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/crypto/syn_hmac.h"
#include "syntropic/crypto/syn_sha256.h"
#include "syntropic/port/syn_port_flash.h"
#include "syntropic/proto/syn_lwm2m.h"
#include "syntropic/system/syn_fwboot.h"
#include "syntropic/system/syn_fwimage.h"
#include "syntropic/system/syn_fwupdate.h"
#include "syntropic/system/syn_ota.h"
#include "syntropic/util/syn_crc.h"
#include "unity/unity.h"

#include <string.h>

#define TEST_SLOT_A_ADDR 0x0000U
#define TEST_SLOT_B_ADDR 0x0800U
#define TEST_SLOT_SIZE 2048U

static uint8_t s_page_buf[256];
static SYN_FwBootManager s_boot_mgr;
static SYN_OTA_Manager s_ota_mgr;

static void ota_test_setup(void)
{
    (void)memset(mock_flash, 0xFF, sizeof(mock_flash));
    mock_flash_fail_at = -1;
    mock_flash_write_fail_next = false;
    (void)syn_fwboot_init(&s_boot_mgr, TEST_SLOT_A_ADDR, TEST_SLOT_B_ADDR);
}

/* ── Initialization & Parameter Validation Tests ─────────────────────────── */

void test_ota_init_and_param_validation(void)
{
    ota_test_setup();
    SYN_OTA_Manager mgr;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_init(NULL, &s_boot_mgr, TEST_SLOT_SIZE, s_page_buf,
                                                      sizeof(s_page_buf)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ota_init(&mgr, NULL, TEST_SLOT_SIZE, s_page_buf, sizeof(s_page_buf)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ota_init(&mgr, &s_boot_mgr, TEST_SLOT_SIZE, NULL, sizeof(s_page_buf)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ota_init(&mgr, &s_boot_mgr, TEST_SLOT_SIZE, s_page_buf, 8U));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ota_init(&mgr, &s_boot_mgr, 16U, s_page_buf, sizeof(s_page_buf)));

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ota_init(&mgr, &s_boot_mgr, TEST_SLOT_SIZE, s_page_buf, sizeof(s_page_buf)));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_IDLE, syn_ota_get_state(&mgr));
    TEST_ASSERT_EQUAL(SYN_OTA_ERR_NONE, syn_ota_get_last_error(&mgr));

    TEST_ASSERT_EQUAL(SYN_OTA_STATE_ERROR, syn_ota_get_state(NULL));
    TEST_ASSERT_EQUAL(SYN_OTA_ERR_INVALID_PARAM, syn_ota_get_last_error(NULL));

    /* Slot configuration */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_set_target_slot(NULL, SYN_FW_SLOT_A));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_set_target_slot(&mgr, 5U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_set_target_slot(&mgr, SYN_FW_SLOT_A));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_set_target_slot(&mgr, SYN_FW_SLOT_B));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_set_target_slot(&mgr, SYN_OTA_SLOT_AUTO));

    /* Verification key config validation */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ota_set_verification_key(NULL, SYN_OTA_CRYPTO_NONE, NULL, 0U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_set_verification_key(&mgr, SYN_OTA_CRYPTO_NONE, NULL, 0U));

    /* Progress query */
    uint32_t wr = 0, tot = 0;
    uint8_t pct = 0;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_get_progress(NULL, &wr, &tot, &pct));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_get_progress(&mgr, &wr, &tot, &pct));
    TEST_ASSERT_EQUAL_UINT32(0, wr);
    TEST_ASSERT_EQUAL_UINT32(0, tot);
    TEST_ASSERT_EQUAL_UINT8(0, pct);

    /* Progress query with NULL out pointers */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_get_progress(&mgr, NULL, NULL, NULL));

    /* Progress query clamping to 100% */
    mgr.expected_total_sz = 50U;
    mgr.fw_upd.bytes_written = 100U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_get_progress(&mgr, &wr, &tot, &pct));
    TEST_ASSERT_EQUAL_UINT8(100U, pct);
}

/* ── CRC-32 Basic OTA Flow ───────────────────────────────────────────────── */

void test_ota_crc32_basic_flow(void)
{
    ota_test_setup();
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_init(&s_ota_mgr, &s_boot_mgr, TEST_SLOT_SIZE, s_page_buf,
                                           sizeof(s_page_buf)));

    /* Prepare 256 bytes payload */
    uint8_t payload[256];
    for (size_t i = 0; i < sizeof(payload); i++) {
        payload[i] = (uint8_t)(i ^ 0x5A);
    }
    uint32_t payload_crc = syn_crc32(payload, sizeof(payload));

    /* Active slot is SLOT A (0), auto selection should target SLOT B */
    s_boot_mgr.active_slot = SYN_FW_SLOT_A;

    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(payload), 0x00010000U, payload_crc));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_DOWNLOADING, syn_ota_get_state(&s_ota_mgr));

    /* Write 4 chunks of 64 bytes */
    for (size_t c = 0; c < 4; c++) {
        uint32_t written = 0, total = 0;
        uint8_t pct = 0;
        TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, &payload[c * 64U], 64U));
        TEST_ASSERT_EQUAL(SYN_OK, syn_ota_get_progress(&s_ota_mgr, &written, &total, &pct));
        TEST_ASSERT_EQUAL_UINT32((uint32_t)((c + 1) * 64U), written);
        TEST_ASSERT_EQUAL_UINT32(256U, total);
        TEST_ASSERT_EQUAL_UINT8((uint8_t)((c + 1) * 25U), pct);
    }

    TEST_ASSERT_EQUAL(SYN_OTA_STATE_DOWNLOADED, syn_ota_get_state(&s_ota_mgr));

    /* Zero chunk should be no-op */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, payload, 0U));

    /* Finish update */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_finish(&s_ota_mgr, NULL, 0U));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_READY_TO_APPLY, syn_ota_get_state(&s_ota_mgr));

    /* Apply update */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_apply(&s_ota_mgr));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_APPLIED, syn_ota_get_state(&s_ota_mgr));

    /* Boot manager should see slot B as NEW */
    TEST_ASSERT_EQUAL(SYN_FW_STATE_NEW, s_boot_mgr.slot_hdr[1].state);
    TEST_ASSERT_EQUAL_UINT32(0x00010000U, s_boot_mgr.slot_hdr[1].version_code);
}

/* ── Boundary, Error & Abort Paths ───────────────────────────────────────── */

void test_ota_boundary_and_abort_paths(void)
{
    ota_test_setup();
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_init(&s_ota_mgr, &s_boot_mgr, TEST_SLOT_SIZE, s_page_buf,
                                           sizeof(s_page_buf)));

    /* Begin with NULL mgr or boot_mgr */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_begin(NULL, 100U, 1U, 0U));
    SYN_OTA_Manager mgr_no_boot = s_ota_mgr;
    mgr_no_boot.boot_mgr = NULL;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_begin(&mgr_no_boot, 100U, 1U, 0U));

    /* Begin with 0 size */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_begin(&s_ota_mgr, 0U, 1U, 0U));

    /* Begin exceeding slot capacity */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_begin(&s_ota_mgr, TEST_SLOT_SIZE, 1U, 0U));

    /* Begin with invalid slot */
    s_ota_mgr.target_slot = 4U;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ota_begin(&s_ota_mgr, 100U, 1U, 0U));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_ERROR, syn_ota_get_state(&s_ota_mgr));
    TEST_ASSERT_EQUAL(SYN_OTA_ERR_NO_FLASH_SLOT, syn_ota_get_last_error(&s_ota_mgr));

    /* Reset and start valid session */
    s_ota_mgr.target_slot = SYN_FW_SLOT_A;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, 100U, 1U, 0x12345678U));

    /* Cannot call begin while already downloading */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ota_begin(&s_ota_mgr, 100U, 1U, 0x12345678U));

    /* Cannot configure keys or slots while active */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ota_set_target_slot(&s_ota_mgr, SYN_FW_SLOT_B));
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_ota_set_verification_key(&s_ota_mgr, SYN_OTA_CRYPTO_NONE, NULL, 0U));
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_ota_set_aes_gcm_params(&s_ota_mgr, s_page_buf, 16U, s_page_buf, 12U));

    /* Writing NULL chunk */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_write_chunk(&s_ota_mgr, NULL, 10U));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_write_chunk(NULL, s_page_buf, 10U));

    /* Overflow chunk size */
    uint8_t dummy[150];
    (void)memset(dummy, 0xAA, sizeof(dummy));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ota_write_chunk(&s_ota_mgr, dummy, 120U));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_ERROR, syn_ota_get_state(&s_ota_mgr));
    TEST_ASSERT_EQUAL(SYN_OTA_ERR_OUT_OF_SPACE, syn_ota_get_last_error(&s_ota_mgr));

    /* Write chunk while in error state */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ota_write_chunk(&s_ota_mgr, dummy, 10U));

    /* Abort when null */
    syn_ota_abort(NULL, SYN_OTA_ERR_NONE);

    /* Abort active */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, 50U, 1U, 0U));
    syn_ota_abort(&s_ota_mgr, SYN_OTA_ERR_INTEGRITY_CHECK);
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_ERROR, syn_ota_get_state(&s_ota_mgr));
    TEST_ASSERT_EQUAL(SYN_OTA_ERR_INTEGRITY_CHECK, syn_ota_get_last_error(&s_ota_mgr));

    /* Premature finish when bytes written < expected */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, 50U, 1U, 0U));
    uint8_t short_chunk[20] = {0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, short_chunk, 20U));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ota_finish(&s_ota_mgr, NULL, 0U));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_ERROR, syn_ota_get_state(&s_ota_mgr));
    TEST_ASSERT_EQUAL(SYN_OTA_ERR_INTEGRITY_CHECK, syn_ota_get_last_error(&s_ota_mgr));

    /* Apply when in error state */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ota_apply(&s_ota_mgr));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_apply(NULL));

    /* Finish when NULL or idle */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_finish(NULL, NULL, 0U));
    SYN_OTA_Manager mgr_idle;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_init(&mgr_idle, &s_boot_mgr, TEST_SLOT_SIZE, s_page_buf,
                                           sizeof(s_page_buf)));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ota_finish(&mgr_idle, NULL, 0U));

    /* Manual slot selection targeting Slot A when active is B */
    s_boot_mgr.active_slot = SYN_FW_SLOT_B;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_set_target_slot(&mgr_idle, SYN_OTA_SLOT_AUTO));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&mgr_idle, 50U, 1U, 0U));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_DOWNLOADING, syn_ota_get_state(&mgr_idle));
}

/* ── HMAC-SHA256 Verification Flow ───────────────────────────────────────── */

void test_ota_hmac_sha256_verification_flow(void)
{
    ota_test_setup();
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_init(&s_ota_mgr, &s_boot_mgr, TEST_SLOT_SIZE, s_page_buf,
                                           sizeof(s_page_buf)));

    uint8_t hmac_key[32];
    for (size_t i = 0; i < sizeof(hmac_key); i++) {
        hmac_key[i] = (uint8_t)(i + 0x10);
    }

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_set_verification_key(
                                             &s_ota_mgr, SYN_OTA_CRYPTO_HMAC_SHA256, NULL, 32U));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_set_verification_key(
                                             &s_ota_mgr, SYN_OTA_CRYPTO_HMAC_SHA256, hmac_key, 0U));
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_ota_set_verification_key(&s_ota_mgr, SYN_OTA_CRYPTO_HMAC_SHA256, hmac_key, 64U));

    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_set_verification_key(&s_ota_mgr, SYN_OTA_CRYPTO_HMAC_SHA256,
                                                           hmac_key, sizeof(hmac_key)));

    uint8_t fw_data[128];
    for (size_t i = 0; i < sizeof(fw_data); i++) {
        fw_data[i] = (uint8_t)(i * 5U);
    }
    uint32_t fw_crc = syn_crc32(fw_data, sizeof(fw_data));

    uint8_t expected_hmac[32];
    SYN_HMAC_SHA256 hmac;
    syn_hmac_sha256_init(&hmac, hmac_key, sizeof(hmac_key));
    syn_hmac_sha256_update(&hmac, fw_data, sizeof(fw_data));
    syn_hmac_sha256_final(&hmac, expected_hmac);

    /* Test corrupted HMAC fails and aborts */
    s_boot_mgr.active_slot = SYN_FW_SLOT_A;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(fw_data), 0x00010500U, fw_crc));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, fw_data, sizeof(fw_data)));
    uint8_t bad_hmac[32];
    (void)memcpy(bad_hmac, expected_hmac, sizeof(bad_hmac));
    bad_hmac[0] ^= 0x55;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ota_finish(&s_ota_mgr, bad_hmac, sizeof(bad_hmac)));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_ERROR, syn_ota_get_state(&s_ota_mgr));
    TEST_ASSERT_EQUAL(SYN_OTA_ERR_INTEGRITY_CHECK, syn_ota_get_last_error(&s_ota_mgr));

    /* Fresh session with valid HMAC */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(fw_data), 0x00010500U, fw_crc));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, fw_data, sizeof(fw_data)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_finish(&s_ota_mgr, expected_hmac, sizeof(expected_hmac)));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_READY_TO_APPLY, syn_ota_get_state(&s_ota_mgr));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_apply(&s_ota_mgr));
}

/* ── Ed25519 Signature Verification Flow ─────────────────────────────────── */

void test_ota_ed25519_verification_flow(void)
{
    ota_test_setup();
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_init(&s_ota_mgr, &s_boot_mgr, TEST_SLOT_SIZE, s_page_buf,
                                           sizeof(s_page_buf)));

    uint8_t seed[32];
    uint8_t privkey[32], pubkey[32];
    for (size_t i = 0; i < 32; i++) {
        seed[i] = (uint8_t)(i + 1);
    }
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(pubkey, privkey, seed));

    /* Invalid key parameters */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ota_set_verification_key(&s_ota_mgr, SYN_OTA_CRYPTO_ED25519, NULL, 32U));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_set_verification_key(
                                             &s_ota_mgr, SYN_OTA_CRYPTO_ED25519, pubkey, 16U));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_set_verification_key(
                                             &s_ota_mgr, (SYN_OTA_CryptoMode)99, pubkey, 32U));

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ota_set_verification_key(&s_ota_mgr, SYN_OTA_CRYPTO_ED25519, pubkey, 32U));

    uint8_t fw_data[128];
    for (size_t i = 0; i < sizeof(fw_data); i++) {
        fw_data[i] = (uint8_t)(i * 3U);
    }
    uint32_t fw_crc = syn_crc32(fw_data, sizeof(fw_data));

    uint8_t sig[64];
    TEST_ASSERT_TRUE(syn_ed25519_sign(fw_data, sizeof(fw_data), privkey, pubkey, sig));

    /* Corrupt signature test */
    s_boot_mgr.active_slot = SYN_FW_SLOT_A;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(fw_data), 0x00020000U, fw_crc));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, fw_data, sizeof(fw_data)));
    uint8_t bad_sig[64];
    (void)memcpy(bad_sig, sig, sizeof(bad_sig));
    bad_sig[0] ^= 0xFF;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ota_finish(&s_ota_mgr, bad_sig, sizeof(bad_sig)));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_ERROR, syn_ota_get_state(&s_ota_mgr));
    TEST_ASSERT_EQUAL(SYN_OTA_ERR_INTEGRITY_CHECK, syn_ota_get_last_error(&s_ota_mgr));

    /* Fresh session with valid signature */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(fw_data), 0x00020000U, fw_crc));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, fw_data, sizeof(fw_data)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_finish(&s_ota_mgr, sig, sizeof(sig)));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_READY_TO_APPLY, syn_ota_get_state(&s_ota_mgr));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_apply(&s_ota_mgr));
}

/* ── AES-GCM Encrypted Firmware Update Flow ──────────────────────────────── */

void test_ota_aes_gcm_encrypted_flow(void)
{
    ota_test_setup();
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_init(&s_ota_mgr, &s_boot_mgr, TEST_SLOT_SIZE, s_page_buf,
                                           sizeof(s_page_buf)));

    uint8_t key[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
                       0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
                       0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};
    uint8_t iv[12] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB};

    /* Invalid AES-GCM params */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ota_set_aes_gcm_params(NULL, key, sizeof(key), iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ota_set_aes_gcm_params(&s_ota_mgr, NULL, sizeof(key), iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ota_set_aes_gcm_params(&s_ota_mgr, key, 10U, iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ota_set_aes_gcm_params(&s_ota_mgr, key, sizeof(key), iv, 8U));

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ota_set_aes_gcm_params(&s_ota_mgr, key, sizeof(key), iv, sizeof(iv)));

    uint8_t plaintext[128];
    for (size_t i = 0; i < sizeof(plaintext); i++) {
        plaintext[i] = (uint8_t)(i + 0x40);
    }
    uint8_t ciphertext[128];
    uint8_t tag[16];

    SYN_AES_GCM_Context gcm_ctx;
    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_gcm_init(&gcm_ctx, key, sizeof(key)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_gcm_encrypt(&gcm_ctx, iv, sizeof(iv), NULL, 0U, plaintext,
                                                  sizeof(plaintext), ciphertext, tag));

    s_boot_mgr.active_slot = SYN_FW_SLOT_A;

    /* Bad tag validation */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(ciphertext), 0x00030000U, 0U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, ciphertext, sizeof(ciphertext)));
    uint8_t bad_tag[16];
    (void)memcpy(bad_tag, tag, sizeof(tag));
    bad_tag[0] ^= 0x01;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ota_finish(&s_ota_mgr, bad_tag, sizeof(bad_tag)));

    /* Finish with invalid sig length */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(ciphertext), 0x00030000U, 0U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, ciphertext, sizeof(ciphertext)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_finish(&s_ota_mgr, tag, 8U));

    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(ciphertext), 0x00030000U, 0U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, ciphertext, sizeof(ciphertext)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_finish(&s_ota_mgr, NULL, 16U));

    /* Valid tag */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(ciphertext), 0x00030000U, 0U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, ciphertext, sizeof(ciphertext)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_finish(&s_ota_mgr, tag, sizeof(tag)));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_READY_TO_APPLY, syn_ota_get_state(&s_ota_mgr));

    /* Verify plaintext was decrypted in flash slot */
    uint8_t readback[128];
    uint32_t data_addr =
        s_ota_mgr.target_slot_addr + syn_fwimage_data_offset(s_ota_mgr.target_slot_addr);
    (void)syn_port_flash_read(data_addr, readback, sizeof(readback));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(plaintext, readback, sizeof(plaintext));
}

/* ── LwM2M Object 5 Synchronization Tests ────────────────────────────────── */

void test_ota_lwm2m_integration(void)
{
    ota_test_setup();
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_init(&s_ota_mgr, &s_boot_mgr, TEST_SLOT_SIZE, s_page_buf,
                                           sizeof(s_page_buf)));

    SYN_LwM2M_FirmwareContext fw_ctx;
    (void)memset(&fw_ctx, 0, sizeof(fw_ctx));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_bind_lwm2m(NULL, &fw_ctx));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_bind_lwm2m(&s_ota_mgr, &fw_ctx));

    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_STATE_IDLE, fw_ctx.state);
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_RESULT_DEFAULT, fw_ctx.result);

    uint8_t data[64];
    (void)memset(data, 0x55, sizeof(data));
    uint32_t data_crc = syn_crc32(data, sizeof(data));

    /* Begin download */
    s_boot_mgr.active_slot = SYN_FW_SLOT_A;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(data), 1U, data_crc));
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_STATE_DOWNLOADING, fw_ctx.state);

    /* Write full data */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, data, sizeof(data)));
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_STATE_DOWNLOADED, fw_ctx.state);

    /* Finish */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_finish(&s_ota_mgr, NULL, 0U));
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_STATE_DOWNLOADED, fw_ctx.state);

    /* Apply */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_apply(&s_ota_mgr));
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_STATE_UPDATING, fw_ctx.state);
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_RESULT_SUCCESS, fw_ctx.result);

    /* Abort error test */
    syn_ota_abort(&s_ota_mgr, SYN_OTA_ERR_INTEGRITY_CHECK);
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_STATE_IDLE, fw_ctx.state);
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_RESULT_INTEGRITY_FAIL, fw_ctx.result);

    /* Abort flash error test */
    syn_ota_abort(&s_ota_mgr, SYN_OTA_ERR_OUT_OF_SPACE);
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_RESULT_NO_FLASH, fw_ctx.result);

    /* Abort bad pkg test */
    syn_ota_abort(&s_ota_mgr, SYN_OTA_ERR_INVALID_PARAM);
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_RESULT_BAD_PKG_TYPE, fw_ctx.result);

    syn_ota_abort(&s_ota_mgr, SYN_OTA_ERR_UNSUPPORTED_CRYPTO);
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_RESULT_BAD_PKG_TYPE, fw_ctx.result);

    syn_ota_abort(&s_ota_mgr, SYN_OTA_ERR_NONE);
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_RESULT_DEFAULT, fw_ctx.result);

    syn_ota_abort(&s_ota_mgr, (SYN_OTA_ErrorCode)99);
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_RESULT_DEFAULT, fw_ctx.result);
}

/* ── Flash Fault Injection & Coverage Edges ──────────────────────────────── */

void test_ota_flash_faults_and_coverage(void)
{
    ota_test_setup();
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_init(&s_ota_mgr, &s_boot_mgr, TEST_SLOT_SIZE, s_page_buf,
                                           sizeof(s_page_buf)));

    /* Inject erase error */
    mock_flash_fail_at = (int32_t)TEST_SLOT_A_ADDR;
    s_ota_mgr.target_slot = SYN_FW_SLOT_A;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ota_begin(&s_ota_mgr, 64U, 1U, 0U));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_ERROR, syn_ota_get_state(&s_ota_mgr));
    TEST_ASSERT_EQUAL(SYN_OTA_ERR_FLASH_ERASE, syn_ota_get_last_error(&s_ota_mgr));

    /* Inject write error (full page buffer flush triggers flash write) */
    mock_flash_fail_at = -1;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, 256U, 1U, 0U));
    mock_flash_write_fail_next = true;
    uint8_t full_page[256];
    (void)memset(full_page, 0x11, sizeof(full_page));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ota_write_chunk(&s_ota_mgr, full_page, sizeof(full_page)));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_ERROR, syn_ota_get_state(&s_ota_mgr));
    TEST_ASSERT_EQUAL(SYN_OTA_ERR_FLASH_WRITE, syn_ota_get_last_error(&s_ota_mgr));

    /* Finalize cleanup so mock flash state never leaks */
    mock_flash_fail_at = -1;
    mock_flash_write_fail_next = false;
}

static void test_r5_ota_regressions(void)
{
    ota_test_setup();

    /* 1. R5-S01: slot_size <= sector size rejected by syn_ota_init; payload > slot - sector
     * rejected */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_init(&s_ota_mgr, &s_boot_mgr, MOCK_FLASH_SECTOR,
                                                      s_page_buf, sizeof(s_page_buf)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_init(&s_ota_mgr, &s_boot_mgr, TEST_SLOT_SIZE, s_page_buf,
                                           sizeof(s_page_buf)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ota_begin(&s_ota_mgr, (TEST_SLOT_SIZE - MOCK_FLASH_SECTOR) + 1U, 1U, 0U));
    TEST_ASSERT_EQUAL(SYN_OTA_ERR_OUT_OF_SPACE, syn_ota_get_last_error(&s_ota_mgr));

    /* 2. R5-S03: HMAC & Ed25519 fail closed on NULL or truncated sig_len in syn_ota_finish */
    uint8_t key32[32] = {0x11, 0x22, 0x33, 0x44};
    uint8_t fw[64];
    memset(fw, 0x7A, sizeof(fw));
    uint32_t fw_crc = syn_crc32(fw, sizeof(fw));

    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_set_verification_key(&s_ota_mgr, SYN_OTA_CRYPTO_HMAC_SHA256,
                                                           key32, sizeof(key32)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(fw), 2U, fw_crc));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, fw, sizeof(fw)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_finish(&s_ota_mgr, NULL, 32U));

    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(fw), 2U, fw_crc));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, fw, sizeof(fw)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_finish(&s_ota_mgr, key32, 16U));

    uint8_t pubkey[32], privkey[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(pubkey, privkey, key32));
    uint8_t sig[64];
    TEST_ASSERT_TRUE(syn_ed25519_sign(fw, sizeof(fw), privkey, pubkey, sig));

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ota_set_verification_key(&s_ota_mgr, SYN_OTA_CRYPTO_ED25519, pubkey, 32U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(fw), 3U, fw_crc));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, fw, sizeof(fw)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_finish(&s_ota_mgr, NULL, 64U));

    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(fw), 3U, fw_crc));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, fw, sizeof(fw)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ota_finish(&s_ota_mgr, sig, 32U));

    /* 3. Complete valid Ed25519 OTA -> APPLIED, then start second OTA directly from APPLIED! */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(fw), 3U, fw_crc));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, fw, sizeof(fw)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_finish(&s_ota_mgr, sig, sizeof(sig)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_apply(&s_ota_mgr));
    TEST_ASSERT_EQUAL(SYN_OTA_STATE_APPLIED, syn_ota_get_state(&s_ota_mgr));

    uint8_t iv[12] = {0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_set_target_slot(&s_ota_mgr, SYN_FW_SLOT_A));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ota_set_aes_gcm_params(&s_ota_mgr, key32, sizeof(key32), iv, sizeof(iv)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ota_set_verification_key(&s_ota_mgr, SYN_OTA_CRYPTO_NONE, NULL, 0U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_begin(&s_ota_mgr, sizeof(fw), 4U, fw_crc));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_write_chunk(&s_ota_mgr, fw, sizeof(fw)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ota_finish(&s_ota_mgr, NULL, 0U));
}

void run_ota_tests(void)
{
    RUN_TEST(test_ota_init_and_param_validation);
    RUN_TEST(test_ota_crc32_basic_flow);
    RUN_TEST(test_ota_boundary_and_abort_paths);
    RUN_TEST(test_ota_hmac_sha256_verification_flow);
    RUN_TEST(test_ota_ed25519_verification_flow);
    RUN_TEST(test_ota_aes_gcm_encrypted_flow);
    RUN_TEST(test_ota_lwm2m_integration);
    RUN_TEST(test_ota_flash_faults_and_coverage);
    RUN_TEST(test_r5_ota_regressions);
}
