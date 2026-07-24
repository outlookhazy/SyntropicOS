/**
 * @file syn_cia401.c
 * @brief Implementation of CANopen CiA 401 Generic I/O Device Profile.
 */

#include "syn_cia401.h"
#include "../util/syn_assert.h"
#include <string.h>
#include <stdlib.h>

void syn_cia401_init(SYN_CiA401_Device *dev, uint8_t dig_in_bytes, uint8_t dig_out_bytes,
                     uint8_t analog_in_ch, uint8_t analog_out_ch)
{
    SYN_ASSERT(dev != NULL);
    memset(dev, 0, sizeof(*dev));

    dev->digital_in_count  = (dig_in_bytes > SYN_CIA401_MAX_DIGITAL_8BIT) ? SYN_CIA401_MAX_DIGITAL_8BIT : dig_in_bytes;
    dev->digital_out_count = (dig_out_bytes > SYN_CIA401_MAX_DIGITAL_8BIT) ? SYN_CIA401_MAX_DIGITAL_8BIT : dig_out_bytes;
    dev->analog_in_count   = (analog_in_ch > SYN_CIA401_MAX_ANALOG_16BIT) ? SYN_CIA401_MAX_ANALOG_16BIT : analog_in_ch;
    dev->analog_out_count  = (analog_out_ch > SYN_CIA401_MAX_ANALOG_16BIT) ? SYN_CIA401_MAX_ANALOG_16BIT : analog_out_ch;
}

size_t syn_cia401_populate_od(SYN_CiA401_Device *dev, SYN_CANOpenODEntry *od_table, size_t max_entries)
{
    SYN_ASSERT(dev != NULL && od_table != NULL);
    size_t count = 0;
    uint8_t i;

    /* 0x6000: Digital Inputs 8-bit */
    for (i = 0; i < dev->digital_in_count && count < max_entries; i++) {
        od_table[count].index      = SYN_CIA401_OD_DIGITAL_INPUT_8BIT;
        od_table[count].subindex   = i + 1;
        od_table[count].type       = SYN_CANOPEN_TYPE_U8;
        od_table[count].access     = SYN_CANOPEN_ACCESS_RO;
        od_table[count].data_ptr   = &dev->digital_in[i];
        od_table[count].size       = sizeof(uint8_t);
        count++;
    }

    /* 0x6200: Digital Outputs 8-bit */
    for (i = 0; i < dev->digital_out_count && count < max_entries; i++) {
        od_table[count].index      = SYN_CIA401_OD_DIGITAL_OUTPUT_8BIT;
        od_table[count].subindex   = i + 1;
        od_table[count].type       = SYN_CANOPEN_TYPE_U8;
        od_table[count].access     = SYN_CANOPEN_ACCESS_RW;
        od_table[count].data_ptr   = &dev->digital_out[i];
        od_table[count].size       = sizeof(uint8_t);
        count++;
    }

    /* 0x6401: Analog Inputs 16-bit */
    for (i = 0; i < dev->analog_in_count && count < max_entries; i++) {
        od_table[count].index      = SYN_CIA401_OD_ANALOG_INPUT_16BIT;
        od_table[count].subindex   = i + 1;
        od_table[count].type       = SYN_CANOPEN_TYPE_I16;
        od_table[count].access     = SYN_CANOPEN_ACCESS_RO;
        od_table[count].data_ptr   = &dev->analog_in[i];
        od_table[count].size       = sizeof(int16_t);
        count++;
    }

    /* 0x6411: Analog Outputs 16-bit */
    for (i = 0; i < dev->analog_out_count && count < max_entries; i++) {
        od_table[count].index      = SYN_CIA401_OD_ANALOG_OUTPUT_16BIT;
        od_table[count].subindex   = i + 1;
        od_table[count].type       = SYN_CANOPEN_TYPE_I16;
        od_table[count].access     = SYN_CANOPEN_ACCESS_RW;
        od_table[count].data_ptr   = &dev->analog_out[i];
        od_table[count].size       = sizeof(int16_t);
        count++;
    }

    /* 0x6421: Analog Input Interrupt Delta */
    for (i = 0; i < dev->analog_in_count && count < max_entries; i++) {
        od_table[count].index      = SYN_CIA401_OD_ANALOG_INPUT_DELTA;
        od_table[count].subindex   = i + 1;
        od_table[count].type       = SYN_CANOPEN_TYPE_I16;
        od_table[count].access     = SYN_CANOPEN_ACCESS_RW;
        od_table[count].data_ptr   = &dev->analog_delta[i];
        od_table[count].size       = sizeof(int16_t);
        count++;
    }

    return count;
}

bool syn_cia401_check_analog_delta(SYN_CiA401_Device *dev, uint8_t *channel)
{
    SYN_ASSERT(dev != NULL && channel != NULL);

    for (uint8_t i = 0; i < dev->analog_in_count; i++) {
        int16_t diff = dev->analog_in[i] - dev->prev_analog_in[i];
        if (diff < 0) diff = -diff;

        if (dev->analog_delta[i] > 0 && diff >= dev->analog_delta[i]) {
            dev->prev_analog_in[i] = dev->analog_in[i];
            *channel = i;
            return true;
        }
    }
    return false;
}
