

# File syn\_settings.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_settings.c**](syn__settings_8c.md)

[Go to the documentation of this file](syn__settings_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_SETTINGS) || SYN_USE_SETTINGS

#include "../util/syn_assert.h"
#include "../util/syn_crc.h"
#include "syn_settings.h"
#include "syn_vfs.h"

#include <stdio.h>
#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

static uint16_t compute_crc(const void *data, uint16_t size)
{
    return syn_crc16_ccitt((const uint8_t *)data, size);
}

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_settings_init(SYN_Settings *s, uint32_t flash_base, uint8_t sector_count, void *data,
                             uint16_t data_size, const void *defaults)
{
    if (s == NULL || data == NULL || defaults == NULL || data_size == 0) {
        return SYN_INVALID_PARAM;
    }

    SYN_ASSERT(s != NULL);
    SYN_ASSERT(data != NULL);
    SYN_ASSERT(defaults != NULL);
    SYN_ASSERT(data_size > 0);

    s->data = data;
    s->data_size = data_size;
    s->defaults = defaults;
    s->on_change = NULL;
    s->on_change_ctx = NULL;

    /* Initialize the backing param store */
    SYN_Status st = syn_param_init(&s->store, flash_base, sector_count, data_size);

    if (st == SYN_OK) {
        /* Flash has valid data — load it */
        st = syn_param_load(&s->store, data);
        if (st != SYN_OK) {
            /* LCOV_EXCL_START: Load failed apply defaults fallback */
            memcpy(data, defaults, data_size);
            /* LCOV_EXCL_STOP */
        }
    } else {
        /* Flash is blank or corrupt — apply defaults and write them */
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
    if (s == NULL || s->data == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_ASSERT(s != NULL);

    uint16_t new_crc = compute_crc(s->data, s->data_size);

    /* Only write flash if data actually changed */
    if (new_crc == s->checksum) {
        return SYN_OK; /* No change — skip flash write */
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
    if (s == NULL || buf == NULL)
        return -1;
    if (len < s->data_size)
        return -2;

    memcpy(buf, s->data, s->data_size);
    return (int)s->data_size;
}

SYN_Status syn_settings_import(SYN_Settings *s, const void *buf, size_t len, bool save)
{
    if (s == NULL || buf == NULL || len != s->data_size)
        return SYN_INVALID_PARAM;

    memcpy(s->data, buf, s->data_size);
    if (save) {
        return syn_settings_save(s);
    }

    s->checksum = compute_crc(s->data, s->data_size);
    return SYN_OK;
}

SYN_Status syn_settings_export_vfs(const SYN_Settings *s, const char *filepath)
{
    if (s == NULL || filepath == NULL)
        return SYN_INVALID_PARAM;

    int fd = syn_vfs_open(filepath, SYN_O_WRONLY | SYN_O_CREAT | SYN_O_TRUNC);
    if (fd < 0)
        return SYN_ERROR;

    int written = syn_vfs_write(fd, s->data, s->data_size);
    syn_vfs_close(fd);

    if (written < 0 || (size_t)written != s->data_size)
        return SYN_ERROR; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */

    return SYN_OK;
}

SYN_Status syn_settings_import_vfs(SYN_Settings *s, const char *filepath, bool save)
{
    if (s == NULL || filepath == NULL)
        return SYN_INVALID_PARAM;

    int fd = syn_vfs_open(filepath, SYN_O_RDONLY);
    if (fd < 0)
        return SYN_ERROR;

    int read_bytes = syn_vfs_read(fd, s->data, s->data_size);
    syn_vfs_close(fd);

    if (read_bytes < 0 || (size_t)read_bytes != s->data_size)
        return SYN_ERROR; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */

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
    if (db == NULL || data == NULL || defaults == NULL || data_size == 0) {
        return SYN_INVALID_PARAM;
    }

    SYN_ASSERT(db != NULL);
    SYN_ASSERT(data != NULL);
    SYN_ASSERT(defaults != NULL);
    SYN_ASSERT(data_size > 0);

    memset(db, 0, sizeof(*db));

    SYN_Status st_a =
        syn_settings_init(&db->bank_a, flash_base_a, sector_count, data, data_size, defaults);
    SYN_Status st_b =
        syn_settings_init(&db->bank_b, flash_base_b, sector_count, data, data_size, defaults);

    if (st_a == SYN_OK && st_b == SYN_OK) {
        /* Both banks valid: compare sequence numbers to pick newest bank */
        uint16_t seq_a = (uint16_t)(db->bank_a.store.next_seq - 1U);
        uint16_t seq_b = (uint16_t)(db->bank_b.store.next_seq - 1U);
        if ((int16_t)(seq_b - seq_a) > 0) {
            db->active_bank = 1;
            /* data already contains Bank B */
        } else {
            db->active_bank = 0;
            (void)syn_param_load(&db->bank_a.store, data);
        }
    } else if (st_b == SYN_OK) {
        db->active_bank = 1;
        /* data already contains Bank B */
    } else if (st_a == SYN_OK) {
        db->active_bank = 0;
        (void)syn_param_load(&db->bank_a.store, data);
    } else {
        memcpy(data, defaults, data_size);
        (void)syn_settings_save(&db->bank_a);
        db->active_bank = 0;
    }

    SYN_Settings *active = (db->active_bank == 0) ? &db->bank_a : &db->bank_b;
    SYN_Settings *inactive = (db->active_bank == 0) ? &db->bank_b : &db->bank_a;
    active->checksum = compute_crc(data, data_size);
    inactive->checksum = active->checksum;
    db->active_crc32 = syn_crc32((const uint8_t *)data, data_size);

    return SYN_OK;
}

SYN_Status syn_settings_dual_bank_save(SYN_DualBankSettings *db)
{
    if (db == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_ASSERT(db != NULL);

    SYN_Settings *active = (db->active_bank == 0) ? &db->bank_a : &db->bank_b;
    SYN_Settings *inactive = (db->active_bank == 0) ? &db->bank_b : &db->bank_a;

    uint32_t current_crc = syn_crc32((const uint8_t *)active->data, active->data_size);
    if (current_crc == db->active_crc32) {
        return SYN_OK; /* Unchanged */
    }

    /* Write to inactive bank */
    SYN_Status st = syn_settings_save(inactive);
    if (st == SYN_OK) {
        /* Switch active bank */
        db->active_bank = (db->active_bank == 0) ? 1 : 0;
        db->active_crc32 = current_crc;
    }

    return st;
}

#endif /* SYN_USE_SETTINGS */
```


