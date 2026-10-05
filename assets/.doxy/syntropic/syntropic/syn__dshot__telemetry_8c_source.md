

# File syn\_dshot\_telemetry.c

[**File List**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_dshot\_telemetry.c**](syn__dshot__telemetry_8c.md)

[Go to the documentation of this file](syn__dshot__telemetry_8c.md)


```C++

#include "syntropic/output/syn_dshot_telemetry.h"

#include "syntropic/output/syn_dshot.h"

static const int8_t gcr_to_nibble[32] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, 9,  10,
                                         11, -1, 13, 14, 15, -1, -1, 2,  3,  -1, 5,
                                         6,  7,  -1, 0,  8,  1,  -1, 4,  12, -1};

SYN_Status syn_dshot_decode_gcr_20bit(uint32_t gcr_20bit, uint16_t *payload_out)
{
    if (!payload_out) {
        return SYN_INVALID_PARAM;
    }

    uint8_t gcr0 = (uint8_t)((gcr_20bit >> 15U) & 0x1FU);
    uint8_t gcr1 = (uint8_t)((gcr_20bit >> 10U) & 0x1FU);
    uint8_t gcr2 = (uint8_t)((gcr_20bit >> 5U) & 0x1FU);
    uint8_t gcr3 = (uint8_t)(gcr_20bit & 0x1FU);

    int8_t n0 = gcr_to_nibble[gcr0];
    int8_t n1 = gcr_to_nibble[gcr1];
    int8_t n2 = gcr_to_nibble[gcr2];
    int8_t n3 = gcr_to_nibble[gcr3];

    if (n0 < 0 || n1 < 0 || n2 < 0 || n3 < 0) {
        return SYN_ERROR;
    }

    *payload_out =
        (uint16_t)(((uint8_t)n0 << 12U) | ((uint8_t)n1 << 8U) | ((uint8_t)n2 << 4U) | (uint8_t)n3);
    return SYN_OK;
}

SYN_Status syn_dshot_parse_telemetry(uint32_t gcr_20bit, uint8_t pole_pairs,
                                     SYN_DShot_Telemetry *telemetry)
{
    if (!telemetry) {
        return SYN_INVALID_PARAM;
    }

    telemetry->valid = false;
    telemetry->period_us = 0;
    telemetry->erpm = 0;
    telemetry->rpm = 0;

    uint16_t payload = 0;
    if (syn_dshot_decode_gcr_20bit(gcr_20bit, &payload) != SYN_OK) {
        return SYN_ERROR;
    }

    /* CRC check over payload[15:4] */
    uint16_t data_12bit = (payload >> 4U);
    uint8_t crc_rx = (uint8_t)(payload & 0x0FU);
    uint8_t crc_calc = syn_dshot_calc_crc(data_12bit);

    if (crc_rx != crc_calc) {
        return SYN_ERROR;
    }

    /* Unpack 12-bit eRPM period: 9-bit mantissa + 3-bit exponent */
    uint16_t mantissa = (data_12bit & 0x01FFU);
    uint8_t exponent = (uint8_t)((data_12bit >> 9U) & 0x07U);

    uint32_t period_us = ((uint32_t)mantissa) << exponent;
    telemetry->period_us = period_us;

    if (period_us > 0) {
        telemetry->erpm = 60000000U / period_us;
        uint8_t p = (pole_pairs == 0) ? 7U : pole_pairs;
        telemetry->rpm = telemetry->erpm / p;
    }

    telemetry->valid = true;
    return SYN_OK;
}
```


