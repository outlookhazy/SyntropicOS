

# File syn\_pmbus.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_pmbus.c**](syn__pmbus_8c.md)

[Go to the documentation of this file](syn__pmbus_8c.md)


```C++

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#include "syn_pmbus.h"

#include <math.h>

#if !defined(SYN_USE_PMBUS) || SYN_USE_PMBUS

float syn_pmbus_linear11_to_float(uint16_t raw)
{
    int8_t exponent = (int8_t)((raw >> 11) & 0x1F);
    if (exponent & 0x10) {
        exponent |= (int8_t)0xF0;
    }
    int16_t mantissa = (int16_t)(raw & 0x07FF);
    if (mantissa & 0x0400) {
        mantissa |= (int16_t)0xF800;
    }
    float f_mantissa = (float)mantissa;
    if (exponent >= 0) {
        return f_mantissa * (float)(1 << exponent);
    } else {
        return f_mantissa / (float)(1 << (-exponent));
    }
}

uint16_t syn_pmbus_float_to_linear11(float val)
{
    if (val == 0.0f) {
        return 0;
    }
    int8_t exponent = 0;
    float temp = val;

    while (temp >= 1023.0f && exponent < 15) {
        temp /= 2.0f;
        exponent++;
    }
    while (temp <= -1024.0f && exponent < 15) {
        temp /= 2.0f;
        exponent++;
    }
    while (temp > -512.0f && temp < 512.0f && temp != 0.0f && exponent > -16) {
        temp *= 2.0f;
        exponent--;
    }

    int16_t mantissa = (int16_t)temp;
    if (mantissa > 1023)
        mantissa = 1023;
    if (mantissa < -1024)
        mantissa = -1024;

    uint16_t u_exp = (uint16_t)(((uint8_t)exponent) & 0x1F);
    uint16_t u_mant = (uint16_t)(((uint16_t)mantissa) & 0x07FF);
    return (uint16_t)((u_exp << 11) | u_mant);
}

float syn_pmbus_linear16_to_float(uint16_t raw, uint8_t vout_mode)
{
    int8_t exponent = (int8_t)(vout_mode & 0x1F);
    if (exponent & 0x10) {
        exponent |= (int8_t)0xF0;
    }
    float f_mantissa = (float)raw;
    if (exponent >= 0) {
        return f_mantissa * (float)(1 << exponent);
    } else {
        return f_mantissa / (float)(1 << (-exponent));
    }
}

uint16_t syn_pmbus_float_to_linear16(float val, uint8_t vout_mode)
{
    int8_t exponent = (int8_t)(vout_mode & 0x1F);
    if (exponent & 0x10) {
        exponent |= (int8_t)0xF0;
    }
    float scaled = val;
    if (exponent >= 0) {
        scaled /= (float)(1 << exponent);
    } else {
        scaled *= (float)(1 << (-exponent));
    }
    if (scaled < 0.0f)
        scaled = 0.0f;
    if (scaled > 65535.0f)
        scaled = 65535.0f;
    return (uint16_t)(scaled + 0.5f);
}

void syn_pmbus_encode_read_cmd(SYN_SMBUS_Packet *pkt, uint8_t slave_addr, uint8_t cmd, bool use_pec)
{
    if (pkt == NULL)
        return;
    pkt->slave_addr = slave_addr;
    pkt->command = cmd;
    pkt->proto = SYN_SMBUS_PROTO_READ_WORD;
    pkt->pec_enabled = use_pec;
    pkt->length = 0;
}

#endif /* SYN_USE_PMBUS */
```


