/**
 * @file syn_pmbus.c
 * @brief PMBus Protocol Engine implementation.
 * @ingroup syn_protocol
 */

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#include "syn_pmbus.h"

#include <math.h>

#if !defined(SYN_USE_PMBUS) || SYN_USE_PMBUS

float syn_pmbus_linear11_to_float(uint16_t raw)
{
    int8_t exponent = (int8_t)((raw >> 11U) & 0x1FU);
    if ((exponent & 0x10) != 0) {
        exponent |= (int8_t)0xF0;
    }
    int16_t mantissa = (int16_t)(raw & 0x07FFU);
    if ((mantissa & 0x0400) != 0) {
        mantissa |= (int16_t)0xF800;
    }
    float f_mantissa = (float)mantissa;
    if (exponent >= 0) {
        return f_mantissa * (float)((uint32_t)1U << (uint8_t)exponent);
    }
    return f_mantissa / (float)((uint32_t)1U << (uint8_t)(-exponent));
}

uint16_t syn_pmbus_float_to_linear11(float val)
{
    if (isnan(val) || val == 0.0f) {
        return 0U;
    }
    if (isinf(val)) {
        return (val > 0.0f) ? 0x7BFFU : 0x7C00U;
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
    while (temp > -512.0f && temp < 512.0f && exponent > -16) {
        temp *= 2.0f;
        exponent--;
    }

    if (temp > 1023.0f) {
        temp = 1023.0f;
    } else if (temp < -1024.0f) {
        temp = -1024.0f;
    }

    int16_t mantissa = (int16_t)temp;
    uint16_t u_exp = (uint16_t)(((uint8_t)exponent) & 0x1FU);
    uint16_t u_mant = (uint16_t)(((uint16_t)mantissa) & 0x07FFU);
    return (uint16_t)((u_exp << 11U) | u_mant);
}

float syn_pmbus_linear16_to_float(uint16_t raw, uint8_t vout_mode)
{
    int8_t exponent = (int8_t)(vout_mode & 0x1FU);
    if ((exponent & 0x10) != 0) {
        exponent |= (int8_t)0xF0;
    }
    float f_mantissa = (float)raw;
    if (exponent >= 0) {
        return f_mantissa * (float)((uint32_t)1U << (uint8_t)exponent);
    }
    return f_mantissa / (float)((uint32_t)1U << (uint8_t)(-exponent));
}

uint16_t syn_pmbus_float_to_linear16(float val, uint8_t vout_mode)
{
    if (isnan(val)) {
        return 0U;
    }
    int8_t exponent = (int8_t)(vout_mode & 0x1FU);
    if ((exponent & 0x10) != 0) {
        exponent |= (int8_t)0xF0;
    }
    float scaled = val;
    if (exponent >= 0) {
        scaled /= (float)((uint32_t)1U << (uint8_t)exponent);
    } else {
        scaled *= (float)((uint32_t)1U << (uint8_t)(-exponent));
    }
    if (scaled < 0.0f) {
        scaled = 0.0f;
    }
    if (scaled > 65535.0f) {
        scaled = 65535.0f;
    }
    return (uint16_t)(scaled + 0.5f);
}

void syn_pmbus_encode_read_cmd(SYN_SMBUS_Packet *pkt, uint8_t slave_addr, uint8_t cmd, bool use_pec)
{
    if (pkt == NULL) {
        return;
    }
    pkt->slave_addr = slave_addr;
    pkt->command = cmd;
    pkt->proto = SYN_SMBUS_PROTO_READ_WORD;
    pkt->pec_enabled = use_pec;
    pkt->length = 0U;
}

#endif /* SYN_USE_PMBUS */
