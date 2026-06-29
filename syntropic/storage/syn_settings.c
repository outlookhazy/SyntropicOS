#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SETTINGS) || SYN_USE_SETTINGS

#include "syn_settings.h"
#include "../util/syn_assert.h"
#include "../util/syn_crc.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

static uint16_t compute_crc(const void *data, uint16_t size)
{
    return syn_crc16_ccitt((const uint8_t *)data, size);
}

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_settings_init(SYN_Settings *s,
                               uint32_t flash_base, uint8_t sector_count,
                               void *data, uint16_t data_size,
                               const void *defaults)
{
    SYN_ASSERT(s != NULL);
    SYN_ASSERT(data != NULL);
    SYN_ASSERT(defaults != NULL);
    SYN_ASSERT(data_size > 0);

    s->data     = data;
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
            /* Load failed — apply defaults */
            memcpy(data, defaults, data_size);
        }
    } else {
        /* Flash is blank or corrupt — apply defaults and write them */
        memcpy(data, defaults, data_size);
        syn_param_save(&s->store, data);
    }

    s->checksum = compute_crc(data, data_size);
    return SYN_OK;
}

SYN_Status syn_settings_save(SYN_Settings *s)
{
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

void syn_settings_on_change(SYN_Settings *s,
                              SYN_SettingsChangeCallback cb, void *ctx)
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

#endif /* SYN_USE_SETTINGS */
