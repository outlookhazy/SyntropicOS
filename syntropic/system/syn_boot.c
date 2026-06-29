#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_BOOT) || SYN_USE_BOOT

/**
 * @file syn_boot.c
 * @brief Boot manager implementation.
 */

#include "syn_boot.h"
#include "../util/syn_assert.h"
#include "../system/syn_errlog.h"

#include <string.h>

#define SYN_BOOT_ERR_CRASH_LOOP  0x0300  /**< Repeated crash detected.  */
#define SYN_BOOT_ERR_SAFE_MODE   0x0301  /**< Entered safe mode.        */

SYN_Status syn_boot_init(SYN_Boot *boot, SYN_ParamStore *store,
                           uint16_t safe_threshold)
{
    SYN_ASSERT(boot != NULL);
    SYN_ASSERT(store != NULL);

    memset(boot, 0, sizeof(*boot));
    boot->store          = store;
    boot->safe_threshold = safe_threshold;

    /* Try to load existing boot data */
    SYN_Status st = syn_param_load(store, &boot->data);

    if (st != SYN_OK) {
        /* First boot ever — initialize defaults */
        memset(&boot->data, 0, sizeof(boot->data));
        boot->data.boot_count  = 1;
        boot->data.fail_count  = 0;
        boot->data.was_healthy = 0;
        boot->data.last_reset  = SYN_RESET_UNKNOWN;

        syn_param_save(store, &boot->data);
        boot->initialized = true;
        return SYN_ERROR; /* first boot, no prior data */
    }

    /* Increment boot count */
    boot->data.boot_count++;

    /* Check if last boot was healthy */
    if (!boot->data.was_healthy) {
        boot->data.fail_count++;
    } else {
        boot->data.fail_count = 0;
    }

    /* Mark this boot as NOT yet healthy */
    boot->data.was_healthy = 0;

    /* Check for safe mode */
    if (safe_threshold > 0 && boot->data.fail_count >= safe_threshold) {
        boot->safe_mode = true;
    }

    /* Save updated boot data */
    syn_param_save(store, &boot->data);
    boot->initialized = true;
    boot->errlog      = NULL;

    /* Log crash-loop and safe-mode events if errlog is later attached.
     * Since errlog is typically initialized after boot, users should
     * call syn_boot_set_errlog() then syn_boot_log_events(). */

    return SYN_OK;
}

SYN_Status syn_boot_mark_healthy(SYN_Boot *boot)
{
    SYN_ASSERT(boot != NULL);
    SYN_ASSERT(boot->initialized);

    boot->data.was_healthy = 1;
    boot->data.fail_count  = 0;

    return syn_param_save(boot->store, &boot->data);
}

void syn_boot_set_reset_reason(SYN_Boot *boot, uint8_t reason)
{
    SYN_ASSERT(boot != NULL);
    boot->data.last_reset = reason;
}

SYN_Status syn_boot_clear_safe_mode(SYN_Boot *boot)
{
    SYN_ASSERT(boot != NULL);
    SYN_ASSERT(boot->initialized);

    boot->safe_mode        = false;
    boot->data.fail_count  = 0;
    boot->data.was_healthy = 1;

    return syn_param_save(boot->store, &boot->data);
}

void syn_boot_set_errlog(SYN_Boot *boot, SYN_ErrLog *errlog)
{
    SYN_ASSERT(boot != NULL);
    boot->errlog = errlog;
}

void syn_boot_log_events(SYN_Boot *boot)
{
    SYN_ASSERT(boot != NULL);
    if (boot->errlog == NULL) return;

    /* Log crash-loop if previous boot wasn't healthy */
    if (boot->data.fail_count > 0) {
        syn_errlog_record(boot->errlog, SYN_BOOT_ERR_CRASH_LOOP,
                           SYN_ERR_WARNING,
                           (uint32_t)boot->data.fail_count);
    }

    /* Log safe mode entry */
    if (boot->safe_mode) {
        syn_errlog_record(boot->errlog, SYN_BOOT_ERR_SAFE_MODE,
                           SYN_ERR_FATAL,
                           (uint32_t)boot->data.boot_count);
    }
}

#endif /* SYN_USE_BOOT */
