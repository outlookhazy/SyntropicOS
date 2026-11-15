

# File syn\_i2c\_queue.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_i2c\_queue.c**](syn__i2c__queue_8c.md)

[Go to the documentation of this file](syn__i2c__queue_8c.md)


```C++


#include "syn_i2c_queue.h"

#include "../port/syn_port_i2c.h"

#include <string.h>

#define SYN_I2C_QUEUE_DEFAULT_CLOCK_HZ 100000U

SYN_Status syn_i2c_queue_init(SYN_I2C_Queue *q, uint8_t bus)
{
    if (q == NULL) {
        return SYN_INVALID_PARAM;
    }

    memset(q, 0, sizeof(*q));
    q->bus = bus;
    q->initialized = true;

    return SYN_OK;
}

SYN_Status syn_i2c_queue_enqueue(SYN_I2C_Queue *q, const SYN_I2C_Transaction *tx)
{
    if ((q == NULL) || !q->initialized || (tx == NULL)) {
        return SYN_INVALID_PARAM;
    }

    if (q->count >= SYN_I2C_QUEUE_MAX_DEPTH) {
        return SYN_BUSY;
    }

    q->ring[q->tail] = *tx;
    q->tail = (uint16_t)((q->tail + 1U) % SYN_I2C_QUEUE_MAX_DEPTH);
    q->count++;

    return syn_i2c_queue_process(q);
}

SYN_Status syn_i2c_queue_process(SYN_I2C_Queue *q)
{
    if ((q == NULL) || !q->initialized) {
        return SYN_INVALID_PARAM;
    }

    if (q->active) {
        return SYN_OK;
    }

    while ((q->count > 0U) && !q->active) {
        SYN_I2C_Transaction tx = q->ring[q->head];
        q->head = (uint16_t)((q->head + 1U) % SYN_I2C_QUEUE_MAX_DEPTH);
        q->count--;
        q->active = true;

        uint8_t target_bus = (tx.bus != 0U) ? tx.bus : q->bus;
        uint32_t fallback_hz = (q->configured_clock_hz != 0U) ? q->configured_clock_hz
                                                              : SYN_I2C_QUEUE_DEFAULT_CLOCK_HZ;
        uint32_t speed_hz = (tx.clock_speed_hz != 0U) ? tx.clock_speed_hz : fallback_hz;

        SYN_Status status = SYN_OK;
        if (!q->bus_configured || (target_bus != q->configured_bus) ||
            (speed_hz != q->configured_clock_hz)) {
            status = syn_port_i2c_init(target_bus, speed_hz, 0U, 0U);
            if (status == SYN_OK) {
                q->bus_configured = true;
                q->configured_bus = target_bus;
                q->configured_clock_hz = speed_hz;
            }
        }

        if (status == SYN_OK) {
            status = syn_port_i2c_transfer(target_bus, tx.addr, tx.tx_data, tx.tx_len, tx.rx_data,
                                           tx.rx_len);
        }

        q->active = false;

        if (tx.callback != NULL) {
            tx.callback(target_bus, status, tx.user_data);
        }
    }

    return SYN_OK;
}

SYN_Status syn_i2c_queue_cancel_all(SYN_I2C_Queue *q)
{
    if ((q == NULL) || !q->initialized) {
        return SYN_INVALID_PARAM;
    }

    q->head = 0U;
    q->tail = 0U;
    q->count = 0U;
    q->active = false;

    return SYN_OK;
}

size_t syn_i2c_queue_count(const SYN_I2C_Queue *q)
{
    if ((q == NULL) || !q->initialized) {
        return 0U;
    }

    return (size_t)q->count;
}
```


