

# File syn\_dshot.c

[**File List**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_dshot.c**](syn__dshot_8c.md)

[Go to the documentation of this file](syn__dshot_8c.md)


```C++

#include "syntropic/output/syn_dshot.h"

uint8_t syn_dshot_calc_crc(uint16_t payload_12bit)
{
    /* DShot CRC = (payload ^ (payload >> 4) ^ (payload >> 8)) & 0x0F */
    uint32_t val = (uint32_t)payload_12bit;
    uint32_t crc = (val ^ (val >> 4U) ^ (val >> 8U)) & 0x0FU;
    return (uint8_t)crc;
}

SYN_Status syn_dshot_encode(uint16_t throttle, bool telemetry, SYN_DShot_Packet *packet)
{
    if (!packet) {
        return SYN_INVALID_PARAM;
    }

    if (throttle > 2047U) {
        throttle = 2047U;
    }

    uint16_t payload_12bit = (uint16_t)((throttle << 1U) | (telemetry ? 1U : 0U));
    uint8_t crc = syn_dshot_calc_crc(payload_12bit);
    uint16_t raw_frame = (uint16_t)((payload_12bit << 4U) | crc);

    packet->throttle = throttle;
    packet->telemetry = telemetry;
    packet->crc = crc;
    packet->raw_frame = raw_frame;

    return SYN_OK;
}

uint16_t syn_dshot_us_to_throttle(uint16_t us)
{
    if (us <= 1048U) {
        return 0U; /* Disarmed / Stop */
    }
    if (us >= 2000U) {
        return 2047U; /* Full Throttle */
    }

    /*
     * Linear mapping:
     * us range: [1048, 2000] -> span = 952
     * throttle range: [48, 2047] -> span = 1999
     * throttle = 48 + ((us - 1048) * 1999) / 952
     */
    uint32_t val = (uint32_t)(us - 1048U);
    uint32_t throttle = 48U + ((val * 1999U) / 952U);
    return (uint16_t)throttle;
}
```


