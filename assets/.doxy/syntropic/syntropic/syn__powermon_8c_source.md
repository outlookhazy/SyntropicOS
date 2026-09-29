

# File syn\_powermon.c

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_powermon.c**](syn__powermon_8c.md)

[Go to the documentation of this file](syn__powermon_8c.md)


```C++

#include "syn_powermon.h"

#include "../util/syn_assert.h"

#include <string.h>

SYN_Status syn_powermon_init(SYN_PowerMon *pm, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda, uint8_t i2c_addr,
                             float shunt_resistor_ohms, SYN_PowerMonType type)
{
    SYN_ASSERT(pm != NULL);
    SYN_ASSERT(shunt_resistor_ohms > 0.0001f);

    if (pm == NULL || shunt_resistor_ohms <= 0.0001f) {
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL and boundary check after
                                     SYN_ASSERT macro in release mode */
    }

    memset(pm, 0, sizeof(*pm));
    pm->type = type;
    pm->i2c_addr = i2c_addr;
    pm->shunt_resistor_ohms = shunt_resistor_ohms;

    syn_soft_i2c_init(&pm->i2c, scl, sda, 10);

    return SYN_OK;
}

void syn_powermon_feed_raw(SYN_PowerMon *pm, uint16_t raw_bus_v, float raw_shunt_mv)
{
    if (pm == NULL)
        return;

    if (pm->type == SYN_POWERMON_INA219) {
        /* INA219 Bus Voltage LSB = 4mV (Shifted right by 3 bits) */
        pm->bus_voltage_v = (float)((raw_bus_v >> 3) * 4) / 1000.0f;
    } else {
        /* INA226 Bus Voltage LSB = 1.25mV */
        pm->bus_voltage_v = (float)(raw_bus_v * 1.25f) / 1000.0f;
    }

    /* Current I = V_shunt / R_shunt */
    if (pm->shunt_resistor_ohms > 0.0f) {
        pm->shunt_current_ma = (raw_shunt_mv / pm->shunt_resistor_ohms);
    } else {
        pm->shunt_current_ma = 0.0f;
    }

    /* Power P = V * I */
    pm->power_mw = pm->bus_voltage_v * pm->shunt_current_ma;
}

float syn_powermon_get_bus_voltage(const SYN_PowerMon *pm)
{
    if (pm == NULL)
        return 0.0f;
    return pm->bus_voltage_v;
}

float syn_powermon_get_current_ma(const SYN_PowerMon *pm)
{
    if (pm == NULL)
        return 0.0f;
    return pm->shunt_current_ma;
}

float syn_powermon_get_power_mw(const SYN_PowerMon *pm)
{
    if (pm == NULL)
        return 0.0f;
    return pm->power_mw;
}
```


