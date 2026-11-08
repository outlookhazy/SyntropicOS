/**
 * @file syn_spi_queue.c
 * @brief Non-blocking SPI transaction queue driver implementation.
 * @ingroup syn_drivers
 */

#include "syn_spi_queue.h"

#include "../port/syn_port_gpio.h"
#include "../port/syn_port_spi.h"

#include <string.h>

/** @brief Default SPI clock frequency (1 MHz) when transaction baudrate_hz is 0. */
#define SYN_SPI_QUEUE_DEFAULT_BAUD_HZ 1000000U

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
    if ((q == NULL) || !q->initialized || (tx == NULL) || ((uint8_t)tx->mode > 3U)) {
        return SYN_INVALID_PARAM;
    }

    if (q->count >= SYN_SPI_QUEUE_MAX_DEPTH) {
        return SYN_BUSY;
    }

    q->ring[q->tail] = *tx;
    q->tail = (uint16_t)((q->tail + 1U) % SYN_SPI_QUEUE_MAX_DEPTH);
    q->count++;

    return syn_spi_queue_process(q);
}

SYN_Status syn_spi_queue_process(SYN_SPI_Queue *q)
{
    if ((q == NULL) || !q->initialized) {
        return SYN_INVALID_PARAM;
    }

    if (q->active) {
        return SYN_OK;
    }

    while ((q->count > 0U) && !q->active) {
        SYN_SPI_Transaction tx = q->ring[q->head];
        q->head = (uint16_t)((q->head + 1U) % SYN_SPI_QUEUE_MAX_DEPTH);
        q->count--;
        q->active = true;

        uint8_t target_bus = (tx.bus != 0U) ? tx.bus : q->bus;
        uint32_t baud = (tx.baudrate_hz != 0U) ? tx.baudrate_hz : SYN_SPI_QUEUE_DEFAULT_BAUD_HZ;
        uint8_t mode = (uint8_t)tx.mode;

        SYN_Status status = SYN_OK;
        if (!q->bus_configured || (target_bus != q->configured_bus) ||
            (baud != q->configured_baud_hz) || (mode != q->configured_mode)) {
            status = syn_port_spi_init(target_bus, baud, mode, 0U);
            if (status == SYN_OK) {
                q->bus_configured = true;
                q->configured_bus = target_bus;
                q->configured_baud_hz = baud;
                q->configured_mode = mode;
            }
        }

        if (status == SYN_OK) {
            syn_port_gpio_write(tx.cs_pin, SYN_GPIO_LOW);
            status = syn_port_spi_transfer(target_bus, tx.tx_data, tx.rx_data, tx.len);
            if (!tx.keep_cs_active) {
                syn_port_gpio_write(tx.cs_pin, SYN_GPIO_HIGH);
            }
        }

        q->active = false;

        if (tx.callback != NULL) {
            tx.callback(target_bus, status, tx.user_data);
        }
    }

    return SYN_OK;
}

SYN_Status syn_spi_queue_cancel_all(SYN_SPI_Queue *q)
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

size_t syn_spi_queue_count(const SYN_SPI_Queue *q)
{
    if ((q == NULL) || !q->initialized) {
        return 0U;
    }

    return (size_t)q->count;
}
