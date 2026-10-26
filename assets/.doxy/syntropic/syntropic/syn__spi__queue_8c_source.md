

# File syn\_spi\_queue.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_spi\_queue.c**](syn__spi__queue_8c.md)

[Go to the documentation of this file](syn__spi__queue_8c.md)


```C++

#include "syn_spi_queue.h"

#include "../port/syn_port_gpio.h"
#include "../port/syn_port_spi.h"

#include <string.h>

SYN_Status syn_spi_queue_init(SYN_SPI_Queue *q, uint8_t bus)
{
    if (q == NULL) {
        return SYN_INVALID_PARAM;
    }

    memset(q, 0, sizeof(*q));
    q->bus = bus;
    q->initialized = true;

    return SYN_OK;
}

SYN_Status syn_spi_queue_enqueue(SYN_SPI_Queue *q, const SYN_SPI_Transaction *tx)
{
    if (q == NULL || !q->initialized || tx == NULL) {
        return SYN_INVALID_PARAM;
    }

    if (q->count >= SYN_SPI_QUEUE_MAX_DEPTH) {
        return SYN_BUSY;
    }

    q->ring[q->tail] = *tx;
    q->tail = (uint16_t)((q->tail + 1) % SYN_SPI_QUEUE_MAX_DEPTH);
    q->count++;

    return syn_spi_queue_process(q);
}

SYN_Status syn_spi_queue_process(SYN_SPI_Queue *q)
{
    if (q == NULL || !q->initialized) {
        return SYN_INVALID_PARAM;
    }

    if (q->active) {
        return SYN_OK;
    }

    while (q->count > 0 && !q->active) {
        SYN_SPI_Transaction tx = q->ring[q->head];
        q->head = (uint16_t)((q->head + 1) % SYN_SPI_QUEUE_MAX_DEPTH);
        q->count--;
        q->active = true;

        /* Assert target CS pin */
        syn_port_gpio_write(tx.cs_pin, SYN_GPIO_LOW);

        /* Execute full duplex transfer */
        SYN_Status status = syn_port_spi_transfer(q->bus, tx.tx_data, tx.rx_data, tx.len);

        /* Deassert CS pin if keep_cs_active is false */
        if (!tx.keep_cs_active) {
            syn_port_gpio_write(tx.cs_pin, SYN_GPIO_HIGH);
        }

        q->active = false;

        if (tx.callback != NULL) {
            tx.callback(q->bus, status, tx.user_data);
        }
    }

    return SYN_OK;
}

SYN_Status syn_spi_queue_cancel_all(SYN_SPI_Queue *q)
{
    if (q == NULL || !q->initialized) {
        return SYN_INVALID_PARAM;
    }

    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->active = false;

    return SYN_OK;
}

size_t syn_spi_queue_count(const SYN_SPI_Queue *q)
{
    if (q == NULL || !q->initialized) {
        return 0;
    }

    return (size_t)q->count;
}
```


