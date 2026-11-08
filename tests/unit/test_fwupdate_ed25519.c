/**
 * @file test_fwupdate_ed25519.c
 * @brief Tests for Ed25519-signed firmware verification (RFC 8032).
 */

#include "mocks/mock_port.h"
#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/port/syn_port_flash.h"
#include "syntropic/system/syn_fwboot.h"
#include "syntropic/system/syn_fwimage.h"
#include "syntropic/system/syn_fwupdate.h"
#include "syntropic/util/syn_crc.h"
#include "unity/unity.h"

#include <string.h>

#define SLOT_ADDR 0x0000u
#define SLOT_B_ADDR 0x0800u
#define SLOT_SIZE (2048u - (uint32_t)MOCK_FLASH_SECTOR)

static uint8_t page_buf[256];
static SYN_FwUpdate upd;

static bool sign_fw_ed25519(const uint8_t *fw, size_t fw_len, uint32_t version_code,
                            const uint8_t seckey[32], const uint8_t pubkey[32], uint8_t sig[64])
{
    uint8_t msg[512U + SYN_FWIMAGE_AUTH_META_SIZE];
    if (fw_len > 512U) {
        return false;
    }
    memcpy(msg, fw, fw_len);
    syn_fwimage_encode_auth_meta(version_code, (uint32_t)fw_len, msg + fw_len);
    return syn_ed25519_sign(msg, fw_len + SYN_FWIMAGE_AUTH_META_SIZE, seckey, pubkey, sig);
}

void test_ed25519_fwupdate_roundtrip(void)
{
    /* Generate Ed25519 keypair */
    uint8_t seed[32] = {0xAA, 0xBB, 0xCC, 0xDD, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                        0x88, 0x99, 0x00, 0xFF, 0xEE, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC,
                        0xDE, 0xF0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t pubkey[32], seckey[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(pubkey, seckey, seed));

    /* Create firmware payload */
    uint8_t firmware[512];
    for (size_t i = 0; i < sizeof(firmware); i++) {
        firmware[i] = (uint8_t)(i ^ 0x5A);
    }

    uint32_t crc = syn_crc32_final(syn_crc32_update(SYN_CRC32_INIT, firmware, sizeof(firmware)));
    uint8_t sig[64];
    TEST_ASSERT_TRUE(sign_fw_ed25519(firmware, sizeof(firmware), 0x00020000U, seckey, pubkey, sig));

    /* Begin update and configure public key */
    memset(&upd, 0, sizeof(upd));
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_fwupdate_begin(&upd, SLOT_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf)));

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    syn_fwupdate_set_public_key(&upd, pubkey);
#endif

    /* Stream firmware in chunks */
    for (size_t offset = 0; offset < sizeof(firmware); offset += 128) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_fwupdate_write(&upd, firmware + offset, 128));
    }

    /* Finalize update with signature */
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_fwupdate_finish(&upd, crc, NULL, sig, 0x00020000));
#else
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_fwupdate_finish(&upd, crc, NULL, 0x00020000));
#endif
#else
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_fwupdate_finish(&upd, crc, sig, 0x00020000));
#else
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_fwupdate_finish(&upd, crc, 0x00020000));
#endif
#endif

    TEST_ASSERT_FALSE(syn_fwupdate_active(&upd));

    /* Verify slot header in flash */
    SYN_FwImageHeader hdr;
    memcpy(&hdr, mock_flash + SLOT_ADDR, sizeof(hdr));
    TEST_ASSERT_EQUAL(SYN_FW_MAGIC, hdr.magic);
    TEST_ASSERT_EQUAL(SYN_FW_STATE_NEW, hdr.state);
    TEST_ASSERT_TRUE(syn_fwimage_header_valid(&hdr));

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    TEST_ASSERT_EQUAL_MEMORY(sig, hdr.image_sig, 64);
    TEST_ASSERT_TRUE(syn_fwimage_verify_signature(&hdr, SLOT_ADDR, pubkey));
#endif
}

void test_ed25519_fwupdate_signature_mismatch_fails(void)
{
    uint8_t seed[32] = {0x11, 0x22, 0x33, 0x44};
    uint8_t pubkey[32], seckey[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(pubkey, seckey, seed));

    uint8_t firmware[128];
    memset(firmware, 0x33, sizeof(firmware));

    uint32_t crc = syn_crc32_final(syn_crc32_update(SYN_CRC32_INIT, firmware, sizeof(firmware)));
    uint8_t sig[64];
    TEST_ASSERT_TRUE(sign_fw_ed25519(firmware, sizeof(firmware), 0x00020000U, seckey, pubkey, sig));

    /* Corrupt signature */
    sig[0] ^= 0x01;

    memset(&upd, 0, sizeof(upd));
    syn_fwupdate_begin(&upd, SLOT_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf));
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    syn_fwupdate_set_public_key(&upd, pubkey);
#endif
    syn_fwupdate_write(&upd, firmware, sizeof(firmware));

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_fwupdate_finish(&upd, crc, NULL, sig, 0x00020000));
#else
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_fwupdate_finish(&upd, crc, NULL, 0x00020000));
#endif
#else
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_fwupdate_finish(&upd, crc, sig, 0x00020000));
#else
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_fwupdate_finish(&upd, crc, 0x00020000));
#endif
#endif

    /* Slot should be marked INVALID */
    SYN_FwImageHeader hdr;
    memcpy(&hdr, mock_flash + SLOT_ADDR, sizeof(hdr));
    TEST_ASSERT_EQUAL(SYN_FW_STATE_INVALID, hdr.state);
}

void test_ed25519_fwupdate_wrong_public_key_fails(void)
{
    uint8_t seed1[32] = {0x01}, seed2[32] = {0x02};
    uint8_t pub1[32], sec1[32], pub2[32], sec2[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(pub1, sec1, seed1));
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(pub2, sec2, seed2));

    uint8_t firmware[64];
    memset(firmware, 0x77, sizeof(firmware));
    uint32_t crc = syn_crc32_final(syn_crc32_update(SYN_CRC32_INIT, firmware, sizeof(firmware)));
    uint8_t sig[64];
    TEST_ASSERT_TRUE(sign_fw_ed25519(firmware, sizeof(firmware), 0x00020000U, sec1, pub1, sig));

    /* Configure with pubkey2 (wrong key) */
    memset(&upd, 0, sizeof(upd));
    syn_fwupdate_begin(&upd, SLOT_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf));
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    syn_fwupdate_set_public_key(&upd, pub2);
#endif
    syn_fwupdate_write(&upd, firmware, sizeof(firmware));

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_fwupdate_finish(&upd, crc, NULL, sig, 0x00020000));
#else
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_fwupdate_finish(&upd, crc, NULL, 0x00020000));
#endif
#else
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_fwupdate_finish(&upd, crc, sig, 0x00020000));
#else
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_fwupdate_finish(&upd, crc, 0x00020000));
#endif
#endif
}

void test_ed25519_fwupdate_boundary_and_null_checks(void)
{
    uint8_t dummy_pub[32] = {0};

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    TEST_ASSERT_FALSE(syn_fwimage_verify_signature(NULL, SLOT_ADDR, dummy_pub));

    SYN_FwImageHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = SYN_FW_MAGIC;
    hdr.image_size = 100;
    syn_fwimage_seal_header(&hdr);

    TEST_ASSERT_FALSE(syn_fwimage_verify_signature(&hdr, SLOT_ADDR, NULL));

    /* Invalid header CRC */
    hdr.header_crc ^= 0x01;
    TEST_ASSERT_FALSE(syn_fwimage_verify_signature(&hdr, SLOT_ADDR, dummy_pub));

    /* Ed25519 verify_hash null checks and invalid high bits */
    uint8_t dummy_sig[64] = {0};
    uint8_t dummy_h[64] = {0};
    TEST_ASSERT_FALSE(syn_ed25519_verify_hash(NULL, dummy_h, dummy_pub));
    TEST_ASSERT_FALSE(syn_ed25519_verify_hash(dummy_sig, NULL, dummy_pub));
    TEST_ASSERT_FALSE(syn_ed25519_verify_hash(dummy_sig, dummy_h, NULL));

    dummy_sig[63] = 0xE0; /* High bits set */
    TEST_ASSERT_FALSE(syn_ed25519_verify_hash(dummy_sig, dummy_h, dummy_pub));

    /* Flash read failure test */
    hdr.header_crc ^= 0x01; /* Restore valid CRC */
    mock_flash_fail_at = (int32_t)(SLOT_ADDR + syn_fwimage_data_offset(SLOT_ADDR));
    TEST_ASSERT_FALSE(syn_fwimage_verify_signature(&hdr, SLOT_ADDR, dummy_pub));
    mock_flash_fail_at = -1;
#endif
}

static void test_r5_ed25519_null_sig_and_boot_confirm_intact(void)
{
    mock_port_reset();
    uint8_t seed[32] = {0x99, 0x88, 0x77, 0x66};
    uint8_t pubkey[32], seckey[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(pubkey, seckey, seed));

    uint8_t firmware[256];
    for (size_t i = 0U; i < sizeof(firmware); i++) {
        firmware[i] = (uint8_t)(i ^ 0xA5U);
    }
    uint32_t crc = syn_crc32(firmware, sizeof(firmware));

    /* 1. R5-S03: pubkey_set + expected_sig == NULL must fail closed */
    memset(&upd, 0, sizeof(upd));
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_fwupdate_begin(&upd, SLOT_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf)));
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    syn_fwupdate_set_public_key(&upd, pubkey);
#endif
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_fwupdate_write(&upd, firmware, sizeof(firmware)));
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_fwupdate_finish(&upd, crc, NULL, NULL, 0x00020100U));
#endif
#elif defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_fwupdate_finish(&upd, crc, NULL, 0x00020100U));
#endif
    SYN_FwImageHeader bad_hdr;
    memcpy(&bad_hdr, mock_flash + SLOT_ADDR, sizeof(bad_hdr));
    TEST_ASSERT_EQUAL(SYN_FW_STATE_INVALID, bad_hdr.state);

    /* 2. R5-S01: Ed25519 signature + payload CRC remain intact across select + confirm */
    uint8_t sig[64];
    TEST_ASSERT_TRUE(sign_fw_ed25519(firmware, sizeof(firmware), 0x00020200U, seckey, pubkey, sig));
    memset(mock_flash, 0xAA, sizeof(mock_flash));
    memset(&upd, 0, sizeof(upd));
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_fwupdate_begin(&upd, SLOT_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf)));
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    syn_fwupdate_set_public_key(&upd, pubkey);
#endif
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_fwupdate_write(&upd, firmware, sizeof(firmware)));
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_fwupdate_finish(&upd, crc, NULL, sig, 0x00020200U));
#endif
#elif defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_fwupdate_finish(&upd, crc, sig, 0x00020200U));
#endif

    SYN_FwBootManager mgr;
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwboot_init(&mgr, SLOT_ADDR, SLOT_B_ADDR));
    TEST_ASSERT_EQUAL_UINT8(SYN_FW_SLOT_A, syn_fwboot_select(&mgr, false));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fwboot_confirm(&mgr));

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    const SYN_FwImageHeader *confirmed_hdr = syn_fwboot_slot_header(&mgr, SYN_FW_SLOT_A);
    TEST_ASSERT_NOT_NULL(confirmed_hdr);
    TEST_ASSERT_EQUAL(SYN_FW_STATE_CONFIRMED, confirmed_hdr->state);
    TEST_ASSERT_TRUE(syn_fwimage_verify_signature(confirmed_hdr, SLOT_ADDR, pubkey));

    /* SYS-03: Mutating version_code or image_size and re-sealing header_crc must fail Ed25519 */
    SYN_FwImageHeader tampered_hdr = *confirmed_hdr;
    tampered_hdr.version_code = 0x00010000U;
    syn_fwimage_seal_header(&tampered_hdr);
    TEST_ASSERT_TRUE(syn_fwimage_header_valid(&tampered_hdr));
    TEST_ASSERT_FALSE(syn_fwimage_verify_signature(&tampered_hdr, SLOT_ADDR, pubkey));

    /* Also reject mismatched version_code at syn_fwupdate_finish time */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_fwupdate_begin(&upd, SLOT_B_ADDR, SLOT_SIZE, page_buf, sizeof(page_buf)));
    syn_fwupdate_set_public_key(&upd, pubkey);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_fwupdate_write(&upd, firmware, sizeof(firmware)));
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_fwupdate_finish(&upd, crc, NULL, sig, 0x00090000U));
#else
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_fwupdate_finish(&upd, crc, sig, 0x00090000U));
#endif
#endif

    mock_port_reset();
}

void run_fwupdate_ed25519_tests(void)
{
    RUN_TEST(test_ed25519_fwupdate_roundtrip);
    RUN_TEST(test_ed25519_fwupdate_signature_mismatch_fails);
    RUN_TEST(test_ed25519_fwupdate_wrong_public_key_fails);
    RUN_TEST(test_ed25519_fwupdate_boundary_and_null_checks);
    RUN_TEST(test_r5_ed25519_null_sig_and_boot_confirm_intact);
}
