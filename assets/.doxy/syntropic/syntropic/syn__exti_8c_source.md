

# File syn\_exti.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_exti.c**](syn__exti_8c.md)

[Go to the documentation of this file](syn__exti_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_EXTI) || SYN_USE_EXTI

#include "syn_exti.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Callback table ─────────────────────────────────────────────────────── */

typedef struct {
    SYN_GPIO_Pin       pin;     
    SYN_EXTI_Callback  cb;      
    void               *ctx;    
    bool                active; 
} EXTI_Entry;

static EXTI_Entry exti_table[SYN_EXTI_MAX_PINS];  
static size_t     exti_count;                      
/* ── Helpers ────────────────────────────────────────────────────────────── */

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
```


