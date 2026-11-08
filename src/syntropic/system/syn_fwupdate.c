#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_BOOT) || SYN_USE_BOOT

/**
 * @file syn_fwupdate.c
 * @brief Streaming firmware updater implementation.
 */

#include "../port/syn_port_flash.h"
#include "../util/syn_assert.h"
#include "../util/syn_crc.h"
#include "syn_fwimage.h"
#include "syn_fwupdate.h"

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
#include "../crypto/syn_ed25519.h"
#endif

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Flush the page buffer to flash, erasing any newly overlapped sectors first.
 * @param upd  Firmware update context.
 * @return SYN_OK on success.
 */
static SYN_Status flush_page(SYN_FwUpdate *upd)
{
    if (upd->page_buf_used == 0U) {
        return SYN_OK;
    }

    uint32_t addr = upd->data_addr + upd->bytes_written;
    uint32_t end_addr = addr + (uint32_t)upd->page_buf_used;

    while (upd->erased_up_to < end_addr) {
        uint32_t raw_sec = syn_port_flash_sector_size(upd->erased_up_to);
        uint32_t sector_size = (raw_sec > 0U) ? raw_sec : (uint32_t)upd->page_buf_used;
        uint32_t sector_start = upd->erased_up_to - (upd->erased_up_to % sector_size);
        SYN_Status erase_st = syn_port_flash_erase(sector_start);
        if (erase_st != SYN_OK) {
            return erase_st;
        }
        upd->erased_up_to = sector_start + sector_size;
    }

    SYN_Status st = syn_port_flash_write(addr, upd->page_buf, upd->page_buf_used);
    if (st != SYN_OK) {
        return st;
    }

    upd->bytes_written += upd->page_buf_used;
    upd->page_buf_used = 0U;

    return SYN_OK;
}

/**
 * @brief Append plaintext bytes into the page buffer and flush full pages to flash.
 * @param upd   Firmware update context.
 * @param data  Plaintext data bytes.
 * @param len   Length in bytes.
 * @return SYN_OK on success.
 */
static SYN_Status append_and_flush(SYN_FwUpdate *upd, const uint8_t *data, size_t len)
{
    size_t offset = 0U;
    while (offset < len) {
        size_t space = (size_t)(upd->page_buf_size - upd->page_buf_used);
        size_t to_copy = len - offset;
        if (to_copy > space) {
            to_copy = space;
        }

        memcpy(upd->page_buf + upd->page_buf_used, data + offset, to_copy);
        upd->page_buf_used += (uint16_t)to_copy;
        offset += to_copy;

        if (upd->page_buf_used >= upd->page_buf_size) {
            SYN_Status st = flush_page(upd);
            if (st != SYN_OK) {
                upd->error = true;
                return st;
            }
        }
    }
    return SYN_OK;
}

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
/**
 * @brief Volatile zeroize helper preventing dead-store elimination of keystream/plaintext buffers.
 * @param ptr  Buffer to clear.
 * @param len  Byte length to clear.
 */
static void fwupdate_secure_zero(void *ptr, size_t len)
{
    volatile uint8_t *p = (volatile uint8_t *)ptr;
    for (size_t i = 0U; i < len; i++) {
        p[i] = 0U;
    }
}

/**
 * @brief Update running GHASH and CTR keystream to decrypt a single ciphertext byte.
 * @param upd      Firmware update context.
 * @param ct_byte  Ciphertext byte.
 * @return Decrypted plaintext byte.
 */
static uint8_t gcm_decrypt_byte(SYN_FwUpdate *upd, uint8_t ct_byte)
{
    upd->gcm_partial_ct[upd->gcm_partial_used++] = ct_byte;
    if (upd->gcm_partial_used == 16U) {
        for (int j = 0; j < 16; j++) {
            upd->gcm_s[j] ^= upd->gcm_partial_ct[j];
        }
        syn_aes_ghash_mult(upd->gcm_s, upd->gcm_h, upd->gcm_s);
        upd->gcm_partial_used = 0U;
    }

    if (upd->gcm_stream_used == 0U || upd->gcm_stream_used == 16U) {
        syn_aes_encrypt_block(&upd->gcm_aes, upd->gcm_ctr, upd->gcm_stream_buf);
        for (int j = 15; j >= 12; j--) {
            upd->gcm_ctr[j]++;
            if (upd->gcm_ctr[j] != 0U) {
                break;
            }
        }
        upd->gcm_stream_used = 0U;
    }

    upd->gcm_total_bytes++;
    return (uint8_t)(ct_byte ^ upd->gcm_stream_buf[upd->gcm_stream_used++]);
}

/**
 * @brief Decrypt and stream an AES-GCM ciphertext chunk into flash.
 * @param upd   Firmware update context.
 * @param data  Ciphertext chunk.
 * @param len   Chunk length in bytes.
 * @return SYN_OK on success.
 */
static SYN_Status gcm_write_stream(SYN_FwUpdate *upd, const uint8_t *data, size_t len)
{
    uint8_t dec_chunk[256] = {0};
    size_t ct_offset = 0U;
    while (ct_offset < len) {
        size_t chunk_len = len - ct_offset;
        if (chunk_len > sizeof(dec_chunk)) {
            chunk_len = sizeof(dec_chunk);
        }
        for (size_t i = 0U; i < chunk_len; i++) {
            dec_chunk[i] = gcm_decrypt_byte(upd, data[ct_offset + i]);
        }

        upd->crc_state = syn_crc32_update(upd->crc_state, dec_chunk, chunk_len);
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
        if (upd->key_set) {
            syn_hmac_sha256_update(&upd->hmac_ctx, dec_chunk, chunk_len);
        }
#endif
        SYN_Status st = append_and_flush(upd, dec_chunk, chunk_len);
        if (st != SYN_OK) {
            fwupdate_secure_zero(dec_chunk, sizeof(dec_chunk));
            return st;
        }
        ct_offset += chunk_len;
    }
    fwupdate_secure_zero(dec_chunk, sizeof(dec_chunk));
    return SYN_OK;
}
#endif

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
/**
 * @brief Verify computed HMAC-SHA256 when an HMAC key is configured (fail closed on NULL).
 * @param upd            Firmware update context.
 * @param computed_hmac  32-byte computed HMAC digest.
 * @param expected_hmac  Expected 32-byte HMAC digest (required if upd->key_set is true).
 * @return true if valid or HMAC not configured, false on missing/mismatched digest.
 */
static bool verify_fw_hmac(const SYN_FwUpdate *upd, const uint8_t computed_hmac[32],
                           const uint8_t *expected_hmac)
{
    if (!upd->key_set) {
        return true;
    }
    if (expected_hmac == NULL) {
        return false;
    }
    uint8_t diff = 0U;
    for (size_t i = 0U; i < 32U; i++) {
        diff |= (uint8_t)(computed_hmac[i] ^ expected_hmac[i]);
    }
    return (diff == 0U);
}
#endif

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
/**
 * @brief Verify Ed25519 signature when a public key is configured (fail closed on NULL).
 * @param upd           Firmware update context.
 * @param version_code  Image version code.
 * @param computed_crc  Computed CRC-32 over image payload.
 * @param expected_sig  Expected 64-byte signature (required if upd->pubkey_set is true).
 * @return true if valid or Ed25519 not configured, false on missing/invalid signature.
 */
static bool verify_fw_ed25519(const SYN_FwUpdate *upd, uint32_t version_code, uint32_t computed_crc,
                              const uint8_t *expected_sig)
{
    if (!upd->pubkey_set) {
        return true;
    }
    if (expected_sig == NULL) {
        return false;
    }

    SYN_FwImageHeader test_hdr;
    memset(&test_hdr, 0, sizeof(test_hdr));
    test_hdr.magic = SYN_FW_MAGIC;
    test_hdr.version_code = version_code;
    test_hdr.image_size = upd->bytes_written;
    test_hdr.image_crc = computed_crc;
    test_hdr.state = SYN_FW_STATE_NEW;
    memcpy(test_hdr.image_sig, expected_sig, 64U);
    syn_fwimage_seal_header(&test_hdr);

    return syn_fwimage_verify_signature(&test_hdr, upd->slot_addr, upd->public_key);
}
#endif

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_fwupdate_begin(SYN_FwUpdate *upd, uint32_t slot_addr, uint32_t max_size,
                              uint8_t *page_buf, uint16_t page_buf_size)
{
    if (upd == NULL || page_buf == NULL || page_buf_size == 0U) {
        return SYN_INVALID_PARAM;
    }

    uint32_t hdr_offset = syn_fwimage_data_offset(slot_addr);

    memset(upd, 0, sizeof(*upd));
    upd->slot_addr = slot_addr;
    upd->data_addr = slot_addr + hdr_offset;
    upd->erased_up_to = upd->data_addr;
    upd->max_size = max_size;
    upd->page_buf = page_buf;
    upd->page_buf_size = page_buf_size;
    upd->crc_state = SYN_CRC32_INIT;
    upd->active = true;

    /* Erase the reserved header region at the start of the slot */
    uint32_t erased = 0U;
    while (erased < hdr_offset) {
        uint32_t raw_sec = syn_port_flash_sector_size(slot_addr + erased);
        uint32_t sec_sz = (raw_sec > 0U) ? raw_sec : hdr_offset;
        SYN_Status st = syn_port_flash_erase(slot_addr + erased);
        if (st != SYN_OK) {
            upd->error = true;
            upd->active = false;
            return st;
        }
        erased += sec_sz;
    }

    return SYN_OK;
}

SYN_Status syn_fwupdate_write(SYN_FwUpdate *upd, const uint8_t *data, size_t len)
{
    SYN_ASSERT(upd != NULL);
    if (!upd->active || upd->error) {
        return SYN_ERROR;
    }
    if (data == NULL || len == 0U) {
        return SYN_OK;
    }

    uint64_t buffered = (uint64_t)upd->bytes_written + (uint64_t)upd->page_buf_used;
    if (buffered > (uint64_t)upd->max_size || len > (size_t)((uint64_t)upd->max_size - buffered)) {
        upd->error = true;
        return SYN_ERROR;
    }

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM
    if (upd->gcm_key_set) {
        return gcm_write_stream(upd, data, len);
    }
#endif

    upd->crc_state = syn_crc32_update(upd->crc_state, data, len);

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    if (upd->key_set) {
        syn_hmac_sha256_update(&upd->hmac_ctx, data, len);
    }
#endif

    return append_and_flush(upd, data, len);
}

SYN_Status syn_fwupdate_finish(SYN_FwUpdate *upd, uint32_t expected_crc,
#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
                               const uint8_t *expected_hmac,
#endif
#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
                               const uint8_t *expected_sig,
#endif
                               uint32_t version_code)
{
    SYN_ASSERT(upd != NULL);
    if (!upd->active || upd->error) {
        return SYN_ERROR;
    }

    SYN_Status st = flush_page(upd);
    if (st != SYN_OK) {
        upd->error = true;
        return st;
    }

    uint32_t computed_crc = syn_crc32_final(upd->crc_state);

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    uint8_t computed_hmac[32] = {0};
    if (upd->key_set) {
        syn_hmac_sha256_final(&upd->hmac_ctx, computed_hmac);
    }
#endif

    if (computed_crc != expected_crc) {
        syn_fwupdate_abort(upd);
        return SYN_ERROR;
    }

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    if (!verify_fw_hmac(upd, computed_hmac, expected_hmac)) {
        syn_fwupdate_abort(upd);
        return SYN_ERROR;
    }
#endif

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    if (!verify_fw_ed25519(upd, version_code, computed_crc, expected_sig)) {
        syn_fwupdate_abort(upd);
        return SYN_ERROR;
    }
#endif

    SYN_FwImageHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = SYN_FW_MAGIC;
    hdr.version_code = version_code;
    hdr.image_size = upd->bytes_written;
    hdr.image_crc = computed_crc;
    hdr.state = SYN_FW_STATE_NEW;

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC
    if (upd->key_set) {
        memcpy(hdr.image_hmac, computed_hmac, 32U);
    }
#endif

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519
    if (upd->pubkey_set) {
        memcpy(hdr.image_sig, expected_sig, 64U);
    }
#endif

    syn_fwimage_seal_header(&hdr);

    st = syn_port_flash_write(upd->slot_addr, &hdr, sizeof(hdr));
    if (st != SYN_OK) {
        upd->error = true;
        syn_fwupdate_abort(upd);
        return st;
    }

    upd->active = false;
    return SYN_OK;
}

void syn_fwupdate_abort(SYN_FwUpdate *upd)
{
    SYN_ASSERT(upd != NULL);
    if (!upd->active) {
        return;
    }

    SYN_FwImageHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = SYN_FW_MAGIC;
    hdr.state = SYN_FW_STATE_INVALID;
    syn_fwimage_seal_header(&hdr);

    uint32_t hdr_span = syn_fwimage_data_offset(upd->slot_addr);
    uint32_t erased = 0U;
    while (erased < hdr_span) {
        uint32_t raw_sec = syn_port_flash_sector_size(upd->slot_addr + erased);
        uint32_t sec_sz = (raw_sec > 0U) ? raw_sec : hdr_span;
        (void)syn_port_flash_erase(upd->slot_addr + erased);
        erased += sec_sz;
    }
    (void)syn_port_flash_write(upd->slot_addr, &hdr, sizeof(hdr));

    upd->active = false;
    upd->error = true;
}

#if defined(SYN_FW_USE_HMAC) && SYN_FW_USE_HMAC

void syn_fwupdate_set_key(SYN_FwUpdate *upd, const void *key, size_t key_len)
{
    SYN_ASSERT(upd != NULL);
    SYN_ASSERT(key != NULL);
    syn_hmac_sha256_init(&upd->hmac_ctx, key, key_len);
    upd->key_set = true;
}

#endif /* SYN_FW_USE_HMAC */

#if defined(SYN_FW_USE_ED25519) && SYN_FW_USE_ED25519

void syn_fwupdate_set_public_key(SYN_FwUpdate *upd, const uint8_t *public_key)
{
    SYN_ASSERT(upd != NULL);
    SYN_ASSERT(public_key != NULL);
    memcpy(upd->public_key, public_key, 32);
    upd->pubkey_set = true;
}

bool syn_fwimage_verify_signature(const SYN_FwImageHeader *hdr, uint32_t slot_addr,
                                  const uint8_t *public_key)
{
    if (hdr == NULL || public_key == NULL || !syn_fwimage_header_valid(hdr)) {
        return false;
    }

    uint32_t data_addr = slot_addr + syn_fwimage_data_offset(slot_addr);
    uint32_t remaining = hdr->image_size;

    SYN_SHA512_Ctx hash_ctx;
    syn_sha512_init(&hash_ctx);
    syn_sha512_update(&hash_ctx, hdr->image_sig, 32U);
    syn_sha512_update(&hash_ctx, public_key, 32U);

    uint8_t chunk[256];
    uint32_t offset = 0;
    while (remaining > 0) {
        uint32_t to_read =
            (remaining > (uint32_t)sizeof(chunk)) ? (uint32_t)sizeof(chunk) : remaining;
        SYN_Status st = syn_port_flash_read(data_addr + offset, chunk, to_read);
        if (st != SYN_OK) {
            return false;
        }
        syn_sha512_update(&hash_ctx, chunk, to_read);
        offset += to_read;
        remaining -= to_read;
    }

    uint8_t h[64];
    syn_sha512_final(&hash_ctx, h);

    return syn_ed25519_verify_hash(hdr->image_sig, h, public_key);
}

#endif /* SYN_FW_USE_ED25519 */

#if defined(SYN_FW_USE_AES_GCM) && SYN_FW_USE_AES_GCM

SYN_Status syn_fwupdate_set_aes_gcm_key(SYN_FwUpdate *upd, const uint8_t *key, size_t key_len,
                                        const uint8_t *iv, size_t iv_len)
{
    if (upd == NULL || key == NULL || iv == NULL || iv_len != 12U) {
        return SYN_INVALID_PARAM;
    }
    if (key_len != 16U && key_len != 24U && key_len != 32U) {
        return SYN_INVALID_PARAM;
    }

    (void)syn_aes_init(&upd->gcm_aes, key, key_len);

    /* Compute H = AES_K(0^128) */
    uint8_t zero[16] = {0};
    syn_aes_encrypt_block(&upd->gcm_aes, zero, upd->gcm_h);

    /* Construct J0 = IV || 0x00000001 */
    memcpy(upd->gcm_j0, iv, 12);
    upd->gcm_j0[12] = 0x00;
    upd->gcm_j0[13] = 0x00;
    upd->gcm_j0[14] = 0x00;
    upd->gcm_j0[15] = 0x01;

    /* CTR counter starts at J0 + 1 */
    memcpy(upd->gcm_ctr, upd->gcm_j0, 16);
    upd->gcm_ctr[15] = 0x02;

    memset(upd->gcm_s, 0, 16);
    memset(upd->gcm_partial_ct, 0, 16);
    upd->gcm_partial_used = 0;
    memset(upd->gcm_stream_buf, 0, 16);
    upd->gcm_stream_used = 0;
    upd->gcm_total_bytes = 0;
    upd->gcm_key_set = true;

    return SYN_OK;
}

SYN_Status syn_fwupdate_finish_gcm(SYN_FwUpdate *upd, const uint8_t expected_tag[16],
                                   uint32_t version_code)
{
    if (upd == NULL || expected_tag == NULL || !upd->active || upd->error || !upd->gcm_key_set) {
        return SYN_ERROR;
    }

    /* Flush remaining page buffer to flash */
    SYN_Status st = flush_page(upd);
    if (st != SYN_OK) {
        upd->error = true;
        return st;
    }

    /* Finalize partial GHASH block if any */
    if (upd->gcm_partial_used > 0) {
        for (uint8_t i = upd->gcm_partial_used; i < 16U; i++) {
            upd->gcm_partial_ct[i] = 0;
        }
        for (int j = 0; j < 16; j++) {
            upd->gcm_s[j] ^= upd->gcm_partial_ct[j];
        }
        syn_aes_ghash_mult(upd->gcm_s, upd->gcm_h, upd->gcm_s);
        upd->gcm_partial_used = 0;
    }

    /* Length block: [len(AAD)*8 = 0 (64-bit)] || [len(CT)*8 (64-bit)] */
    uint8_t len_block[16] = {0};
    uint64_t ct_bits = upd->gcm_total_bytes * 8ULL;
    for (int i = 0; i < 8; i++) {
        len_block[15 - i] = (uint8_t)((ct_bits >> (i * 8)) & 0xFF);
    }
    for (int j = 0; j < 16; j++) {
        upd->gcm_s[j] ^= len_block[j];
    }
    syn_aes_ghash_mult(upd->gcm_s, upd->gcm_h, upd->gcm_s);

    /* Encrypt J0 for tag mask */
    uint8_t tag_mask[16];
    syn_aes_encrypt_block(&upd->gcm_aes, upd->gcm_j0, tag_mask);

    uint8_t computed_tag[16];
    uint8_t diff = 0;
    for (int j = 0; j < 16; j++) {
        computed_tag[j] = (uint8_t)(upd->gcm_s[j] ^ tag_mask[j]);
        diff |= (uint8_t)(computed_tag[j] ^ expected_tag[j]);
    }
    fwupdate_secure_zero(tag_mask, sizeof(tag_mask));
    fwupdate_secure_zero(computed_tag, sizeof(computed_tag));

    if (diff != 0) {
        syn_fwupdate_abort(upd);
        return SYN_ERROR;
    }

    /* Write final image header */
    SYN_FwImageHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = SYN_FW_MAGIC;
    hdr.version_code = version_code;
    hdr.image_size = upd->bytes_written;
    hdr.image_crc = syn_crc32_final(upd->crc_state);
    hdr.state = SYN_FW_STATE_NEW;

    syn_fwimage_seal_header(&hdr);

    st = syn_port_flash_write(upd->slot_addr, &hdr, sizeof(hdr));
    if (st != SYN_OK) {
        upd->error = true;
        syn_fwupdate_abort(upd);
        return st;
    }

    upd->active = false;
    return SYN_OK;
}

#endif /* SYN_FW_USE_AES_GCM */

#endif /* SYN_USE_BOOT */
