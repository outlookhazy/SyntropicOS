#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_SETTINGS) || SYN_USE_SETTINGS

/**
 * @file syn_settings.c
 * @brief Persistent settings manager with change detection and dual-bank support.
 */

#include "../util/syn_assert.h"
#include "../util/syn_crc.h"
#include "syn_settings.h"
#include "syn_vfs.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Compute CRC-16 CCITT checksum over a settings buffer.
 * @param data Settings buffer pointer.
 * @param size Size of settings buffer in bytes.
 * @return Computed 16-bit CRC.
 */
static uint16_t compute_crc(const void *data, uint16_t size)
{
    return syn_crc16_ccitt((const uint8_t *)data, size);
}

/**
 * @brief Initialize bank metadata and scan underlying store without writing defaults.
 * @param s            Settings bank instance.
 * @param flash_base   Flash base address for bank.
 * @param sector_count Number of sectors allocated to bank.
 * @param data         Shared RAM settings buffer.
 * @param data_size    Settings struct size in bytes.
 * @param defaults     Default settings struct in ROM.
 * @param has_valid    [out] Set to true if flash store contains a valid slot.
 * @return SYN_OK if bank geometry is valid, SYN_INVALID_PARAM otherwise.
 */
static SYN_Status probe_bank(SYN_Settings *s, uint32_t flash_base, uint8_t sector_count, void *data,
                             uint16_t data_size, const void *defaults, bool *has_valid)
{
    s->data = data;
    s->data_size = data_size;
    s->defaults = defaults;
    s->on_change = NULL;
    s->on_change_ctx = NULL;

    SYN_Status st = syn_param_init(&s->store, flash_base, sector_count, data_size);
    if (st == SYN_INVALID_PARAM) {
        *has_valid = false;
        return SYN_INVALID_PARAM;
    }

    *has_valid = (st == SYN_OK);
    return SYN_OK;
}

/**
 * @brief Select active dual-bank image, load RAM buffer, and synchronize next_seq.
 * @param db        Dual-bank settings instance.
 * @param valid_a   True if Bank A holds a valid slot.
 * @param valid_b   True if Bank B holds a valid slot.
 * @param data      Shared RAM settings buffer.
 * @param data_size Settings struct size in bytes.
 * @param defaults  Default settings struct in ROM.
 */
static void select_dual_bank(SYN_DualBankSettings *db, bool valid_a, bool valid_b, void *data,
                             uint16_t data_size, const void *defaults)
{
    uint16_t seq_a = (uint16_t)(db->bank_a.store.next_seq - 1U);
    uint16_t seq_b = (uint16_t)(db->bank_b.store.next_seq - 1U);

    if (valid_a && valid_b) {
        if ((int16_t)(seq_b - seq_a) > 0) {
            (void)syn_param_load(&db->bank_b.store, data);
            db->active_bank = 1U;
            db->bank_a.store.next_seq = (uint16_t)(seq_b + 1U);
            db->bank_b.store.next_seq = (uint16_t)(seq_b + 1U);
        } else {
            (void)syn_param_load(&db->bank_a.store, data);
            db->active_bank = 0U;
            db->bank_a.store.next_seq = (uint16_t)(seq_a + 1U);
            db->bank_b.store.next_seq = (uint16_t)(seq_a + 1U);
        }
    } else if (valid_b) {
        (void)syn_param_load(&db->bank_b.store, data);
        db->active_bank = 1U;
        db->bank_a.store.next_seq = (uint16_t)(seq_b + 1U);
        db->bank_b.store.next_seq = (uint16_t)(seq_b + 1U);
    } else if (valid_a) {
        (void)syn_param_load(&db->bank_a.store, data);
        db->active_bank = 0U;
        db->bank_a.store.next_seq = (uint16_t)(seq_a + 1U);
        db->bank_b.store.next_seq = (uint16_t)(seq_a + 1U);
    } else {
        memcpy(data, defaults, data_size);
        (void)syn_param_save(&db->bank_a.store, data);
        db->active_bank = 0U;
        db->bank_b.store.next_seq = db->bank_a.store.next_seq;
    }
}

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_settings_init(SYN_Settings *s, uint32_t flash_base, uint8_t sector_count, void *data,
                             uint16_t data_size, const void *defaults)
{
    if ((s == NULL) || (data == NULL) || (defaults == NULL) || (data_size == 0U)) {
        return SYN_INVALID_PARAM;
    }

    s->data = data;
    s->data_size = data_size;
    s->defaults = defaults;
    s->on_change = NULL;
    s->on_change_ctx = NULL;

    SYN_Status st = syn_param_init(&s->store, flash_base, sector_count, data_size);
    if (st == SYN_INVALID_PARAM) {
        return SYN_INVALID_PARAM;
    }

    if (st == SYN_OK) {
        st = syn_param_load(&s->store, data);
        if (st != SYN_OK) {
            memcpy(data, defaults, data_size);
        }
    } else {
        memcpy(data, defaults, data_size);
        SYN_Status save_st = syn_param_save(&s->store, data);
        if (save_st != SYN_OK) {
            return save_st;
        }
    }

    s->checksum = compute_crc(data, data_size);
    return SYN_OK;
}

SYN_Status syn_settings_save(SYN_Settings *s)
{
    if ((s == NULL) || (s->data == NULL)) {
        return SYN_INVALID_PARAM;
    }

    uint16_t new_crc = compute_crc(s->data, s->data_size);
    if (new_crc == s->checksum) {
        return SYN_OK;
    }

    SYN_Status st = syn_param_save(&s->store, s->data);
    if (st == SYN_OK) {
        s->checksum = new_crc;
        if (s->on_change != NULL) {
            s->on_change(s->data, s->on_change_ctx);
        }
    }

    return st;
}

bool syn_settings_changed(const SYN_Settings *s)
{
    SYN_ASSERT(s != NULL);
    uint16_t live_crc = compute_crc(s->data, s->data_size);
    return live_crc != s->checksum;
}

SYN_Status syn_settings_reset(SYN_Settings *s)
{
    SYN_ASSERT(s != NULL);
    memcpy(s->data, s->defaults, s->data_size);
    return syn_settings_save(s);
}

void syn_settings_on_change(SYN_Settings *s, SYN_SettingsChangeCallback cb, void *ctx)
{
    SYN_ASSERT(s != NULL);
    s->on_change = cb;
    s->on_change_ctx = ctx;
}

SYN_Status syn_settings_reload(SYN_Settings *s)
{
    SYN_ASSERT(s != NULL);
    SYN_Status st = syn_param_load(&s->store, s->data);
    if (st == SYN_OK) {
        s->checksum = compute_crc(s->data, s->data_size);
    }
    return st;
}

int syn_settings_export(const SYN_Settings *s, void *buf, size_t len)
{
    if ((s == NULL) || (buf == NULL)) {
        return -1;
    }
    if (len < s->data_size) {
        return -2;
    }

    memcpy(buf, s->data, s->data_size);
    return (int)s->data_size;
}

SYN_Status syn_settings_import(SYN_Settings *s, const void *buf, size_t len, bool save)
{
    if ((s == NULL) || (buf == NULL) || (len != s->data_size)) {
        return SYN_INVALID_PARAM;
    }

    memcpy(s->data, buf, s->data_size);
    if (save) {
        return syn_settings_save(s);
    }

    s->checksum = compute_crc(s->data, s->data_size);
    return SYN_OK;
}

SYN_Status syn_settings_export_vfs(const SYN_Settings *s, const char *filepath)
{
    if ((s == NULL) || (filepath == NULL)) {
        return SYN_INVALID_PARAM;
    }

    int fd = syn_vfs_open(filepath, SYN_O_WRONLY | SYN_O_CREAT | SYN_O_TRUNC);
    if (fd < 0) {
        return SYN_ERROR;
    }

    int written = syn_vfs_write(fd, s->data, s->data_size);
    syn_vfs_close(fd);

    if ((written < 0) || ((size_t)written != s->data_size)) {
        return SYN_ERROR;
    }

    return SYN_OK;
}

SYN_Status syn_settings_import_vfs(SYN_Settings *s, const char *filepath, bool save)
{
    if ((s == NULL) || (filepath == NULL)) {
        return SYN_INVALID_PARAM;
    }

    int fd = syn_vfs_open(filepath, SYN_O_RDONLY);
    if (fd < 0) {
        return SYN_ERROR;
    }

    int read_bytes = syn_vfs_read(fd, s->data, s->data_size);
    syn_vfs_close(fd);

    if ((read_bytes < 0) || ((size_t)read_bytes != s->data_size)) {
        return SYN_ERROR;
    }

    if (save) {
        return syn_settings_save(s);
    }

    s->checksum = compute_crc(s->data, s->data_size);
    return SYN_OK;
}

/* ── Dual-Bank Transactional Settings Implementation ─────────────────────── */

SYN_Status syn_settings_dual_bank_init(SYN_DualBankSettings *db, uint32_t flash_base_a,
                                       uint32_t flash_base_b, uint8_t sector_count, void *data,
                                       uint16_t data_size, const void *defaults)
{
    if ((db == NULL) || (data == NULL) || (defaults == NULL) || (data_size == 0U)) {
        return SYN_INVALID_PARAM;
    }

    memset(db, 0, sizeof(*db));

    bool valid_a = false;
    bool valid_b = false;
    if ((probe_bank(&db->bank_a, flash_base_a, sector_count, data, data_size, defaults, &valid_a) !=
         SYN_OK) ||
        (probe_bank(&db->bank_b, flash_base_b, sector_count, data, data_size, defaults, &valid_b) !=
         SYN_OK)) {
        return SYN_INVALID_PARAM;
    }

    select_dual_bank(db, valid_a, valid_b, data, data_size, defaults);

    uint16_t crc16 = compute_crc(data, data_size);
    db->bank_a.checksum = crc16;
    db->bank_b.checksum = crc16;
    db->active_crc32 = syn_crc32((const uint8_t *)data, data_size);

    return SYN_OK;
}

SYN_Status syn_settings_dual_bank_save(SYN_DualBankSettings *db)
{
    if (db == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_Settings *active = (db->active_bank == 0U) ? &db->bank_a : &db->bank_b;
    SYN_Settings *inactive = (db->active_bank == 0U) ? &db->bank_b : &db->bank_a;

    if (active->data == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint16_t new_crc16 = compute_crc(active->data, active->data_size);
    uint32_t new_crc32 = syn_crc32((const uint8_t *)active->data, active->data_size);
    if ((new_crc16 == active->checksum) && (new_crc32 == db->active_crc32)) {
        return SYN_OK;
    }

    SYN_Status st = syn_param_save(&inactive->store, inactive->data);
    if (st == SYN_OK) {
        inactive->checksum = new_crc16;
        active->checksum = new_crc16;
        active->store.next_seq = inactive->store.next_seq;
        db->active_bank = (db->active_bank == 0U) ? 1U : 0U;
        db->active_crc32 = new_crc32;
        if (inactive->on_change != NULL) {
            inactive->on_change(inactive->data, inactive->on_change_ctx);
        }
    }

    return st;
}

#endif /* SYN_USE_SETTINGS */
