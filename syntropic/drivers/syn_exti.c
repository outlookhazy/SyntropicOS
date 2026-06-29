#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_EXTI) || SYN_USE_EXTI

/**
 * @file syn_exti.c
 * @brief GPIO interrupt dispatcher implementation.
 */

#include "syn_exti.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Callback table ─────────────────────────────────────────────────────── */

/** @brief EXTI callback table entry. */
typedef struct {
    SYN_GPIO_Pin       pin;     /**< Registered pin                  */
    SYN_EXTI_Callback  cb;      /**< User callback                   */
    void               *ctx;    /**< User context                    */
    bool                active; /**< true if interrupt is enabled     */
} EXTI_Entry;

static EXTI_Entry exti_table[SYN_EXTI_MAX_PINS];  /**< Registered callbacks.  */
static size_t     exti_count;                      /**< Number of registered.  */

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Find the table entry for a given pin.
 * @param pin  GPIO pin.
 * @return Pointer to entry, or NULL if not found.
 */
static EXTI_Entry *find_entry(SYN_GPIO_Pin pin)
{
    size_t i;
    for (i = 0; i < exti_count; i++) {
        if (exti_table[i].pin == pin) {
            return &exti_table[i];
        }
    }
    return NULL;
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_exti_init(void)
{
    memset(exti_table, 0, sizeof(exti_table));
    exti_count = 0;
}

SYN_Status syn_exti_register(SYN_GPIO_Pin pin, SYN_EXTI_Edge edge,
                                SYN_EXTI_Callback cb, void *ctx)
{
    SYN_ASSERT(cb != NULL);

    /* Check if already registered — update in place */
    EXTI_Entry *e = find_entry(pin);
    if (e != NULL) {
        e->cb     = cb;
        e->ctx    = ctx;
        e->active = true;
        syn_port_exti_configure(pin, edge);
        return SYN_OK;
    }

    /* New registration */
    if (exti_count >= SYN_EXTI_MAX_PINS) {
        return SYN_ERROR;
    }

    e = &exti_table[exti_count++];
    e->pin    = pin;
    e->cb     = cb;
    e->ctx    = ctx;
    e->active = true;

    syn_port_exti_configure(pin, edge);
    return SYN_OK;
}

void syn_exti_unregister(SYN_GPIO_Pin pin)
{
    EXTI_Entry *e = find_entry(pin);
    if (e != NULL) {
        syn_port_exti_disable(pin);
        e->active = false;
        e->cb     = NULL;
    }
}

void syn_exti_enable(SYN_GPIO_Pin pin)
{
    const EXTI_Entry *e = find_entry(pin);
    if (e != NULL && e->active) {
        syn_port_exti_enable(pin);
    }
}

void syn_exti_disable(SYN_GPIO_Pin pin)
{
    const EXTI_Entry *e = find_entry(pin);
    if (e != NULL) {
        syn_port_exti_disable(pin);
    }
}

void syn_exti_irq_handler(SYN_GPIO_Pin pin)
{
    EXTI_Entry *e = find_entry(pin);
    if (e != NULL && e->active && e->cb != NULL) {
        e->cb(e->pin, e->ctx);
    }
}

size_t syn_exti_count(void)
{
    return exti_count;
}

#endif /* SYN_USE_EXTI */
