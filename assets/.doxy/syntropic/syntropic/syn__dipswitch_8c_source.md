

# File syn\_dipswitch.c

[**File List**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_dipswitch.c**](syn__dipswitch_8c.md)

[Go to the documentation of this file](syn__dipswitch_8c.md)


```C++

#include "syn_dipswitch.h"

#include "../util/syn_assert.h"

#include <string.h>

SYN_Status syn_dipswitch_init(SYN_DipSwitch *ds, const SYN_GPIO_Pin *pins, uint8_t count,
                              bool active_low)
{
    SYN_ASSERT(ds != NULL);
    SYN_ASSERT(pins != NULL);
    SYN_ASSERT(count > 0 && count <= SYN_DIPSWITCH_MAX_PINS);

    memset(ds, 0, sizeof(*ds));
    ds->count = count;
    ds->active_low = active_low;
    memcpy(ds->pins, pins, sizeof(SYN_GPIO_Pin) * count);

    SYN_GPIO_Mode mode = active_low ? SYN_GPIO_INPUT_PULLUP : SYN_GPIO_INPUT_PULLDOWN;

    for (uint8_t i = 0; i < count; i++) {
        syn_port_gpio_init(ds->pins[i], mode);
    }

    syn_dipswitch_read(ds);
    ds->changed = false;

    return SYN_OK;
}

void syn_dipswitch_read(SYN_DipSwitch *ds)
{
    if (ds == NULL)
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */

    uint32_t val = 0;

    for (uint8_t i = 0; i < ds->count; i++) {
        bool raw_state = (syn_port_gpio_read(ds->pins[i]) == SYN_GPIO_HIGH);
        bool on = ds->active_low ? !raw_state : raw_state;
        if (on) {
            val |= (1u << i);
        }
    }

    if (val != ds->current_value) {
        ds->previous_value = ds->current_value;
        ds->current_value = val;
        ds->changed = true;
    } else {
        ds->changed = false;
    }
}

uint32_t syn_dipswitch_get_value(const SYN_DipSwitch *ds)
{
    return ds != NULL ? ds->current_value : 0;
}

bool syn_dipswitch_has_changed(SYN_DipSwitch *ds)
{
    return ds != NULL && ds->changed;
}
```


