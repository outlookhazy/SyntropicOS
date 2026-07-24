

# File syn\_dmx512.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_dmx512.c**](syn__dmx512_8c.md)

[Go to the documentation of this file](syn__dmx512_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_DMX512) || SYN_USE_DMX512

#include "syn_dmx512.h"
#include "../util/syn_assert.h"

#include <string.h>

void syn_dmx512_master_init(SYN_DMX512_Master *master, uint16_t num_channels)
{
    SYN_ASSERT(master != NULL);

    memset(master, 0, sizeof(*master));
    if (num_channels == 0 || num_channels > SYN_DMX512_MAX_CHANNELS) {
        num_channels = SYN_DMX512_MAX_CHANNELS;
    }
    master->num_channels = num_channels;
    master->start_code   = SYN_DMX512_START_CODE_NULL;
}

SYN_Status syn_dmx512_master_set_channel(SYN_DMX512_Master *master, uint16_t channel, uint8_t value)
{
    if (master == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (channel < 1 || channel > master->num_channels) {
        return SYN_INVALID_PARAM;
    }

    master->channels[channel - 1] = value;
    return SYN_OK;
}

uint8_t syn_dmx512_master_get_channel(const SYN_DMX512_Master *master, uint16_t channel)
{
    if (master == NULL || channel < 1 || channel > master->num_channels) {
        return 0;
    }
    return master->channels[channel - 1];
}

size_t syn_dmx512_master_build_frame(const SYN_DMX512_Master *master, uint8_t *out_buf, size_t max_len)
{
    if (master == NULL || out_buf == NULL || max_len < 1) {
        return 0;
    }

    size_t needed = 1 + master->num_channels;
    if (max_len < needed) {
        return 0;
    }

    out_buf[0] = master->start_code;
    memcpy(&out_buf[1], master->channels, master->num_channels);
    return needed;
}

void syn_dmx512_slave_init(SYN_DMX512_Slave *slave, uint16_t start_address, uint16_t footprint)
{
    SYN_ASSERT(slave != NULL);

    memset(slave, 0, sizeof(*slave));
    if (start_address < 1) start_address = 1;
    if (start_address > SYN_DMX512_MAX_CHANNELS) start_address = SYN_DMX512_MAX_CHANNELS;

    if (footprint < 1) footprint = 1;
    if ((uint32_t)start_address - 1U + footprint > SYN_DMX512_MAX_CHANNELS) {
        footprint = (uint16_t)(SYN_DMX512_MAX_CHANNELS - (start_address - 1U));
    }

    slave->start_address = start_address;
    slave->footprint     = footprint;
    slave->rx_state      = SYN_DMX512_STATE_WAIT_BREAK;
}

void syn_dmx512_slave_rx_break(SYN_DMX512_Slave *slave)
{
    if (slave == NULL) return;

    slave->rx_state     = SYN_DMX512_STATE_WAIT_START;
    slave->current_slot = 0;
}

void syn_dmx512_slave_rx_byte(SYN_DMX512_Slave *slave, uint8_t byte)
{
    if (slave == NULL) return;

    switch (slave->rx_state) {
    case SYN_DMX512_STATE_WAIT_START:
        slave->start_code = byte;
        if (byte == SYN_DMX512_START_CODE_NULL) {
            slave->rx_state     = SYN_DMX512_STATE_READ_SLOTS;
            slave->current_slot = 1; /* Slot 1 is first channel after Start Code */
        } else {
            slave->rx_state     = SYN_DMX512_STATE_WAIT_BREAK;
        }
        break;

    case SYN_DMX512_STATE_READ_SLOTS:
        if (slave->current_slot >= slave->start_address &&
            slave->current_slot < slave->start_address + slave->footprint) {
            uint16_t idx = slave->current_slot - slave->start_address;
            slave->rx_channels[idx] = byte;
        }

        slave->current_slot++;
        if (slave->current_slot > SYN_DMX512_MAX_CHANNELS) {
            slave->frame_complete = true;
            slave->rx_state       = SYN_DMX512_STATE_WAIT_BREAK;
        }
        break;

    case SYN_DMX512_STATE_WAIT_BREAK:
    default:
        break;
    }
}

uint8_t syn_dmx512_slave_get_channel(const SYN_DMX512_Slave *slave, uint16_t offset)
{
    if (slave == NULL || offset >= slave->footprint) {
        return 0;
    }
    return slave->rx_channels[offset];
}

bool syn_dmx512_slave_is_updated(SYN_DMX512_Slave *slave)
{
    if (slave == NULL) return false;

    if (slave->frame_complete) {
        slave->frame_complete = false;
        return true;
    }
    return false;
}

#endif /* SYN_USE_DMX512 */
```


