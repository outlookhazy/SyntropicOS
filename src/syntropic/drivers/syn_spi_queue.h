/**
 * @file syn_spi_queue.h
 * @brief Non-blocking SPI transaction queue driver with CS management & parameter switching.
 * @ingroup syn_drivers
 */

#ifndef SYN_SPI_QUEUE_H
#define SYN_SPI_QUEUE_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"
#include "../port/syn_port_spi.h"
#include "syn_spi.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum pending transactions per SPI queue instance */
#ifndef SYN_SPI_QUEUE_MAX_DEPTH
#define SYN_SPI_QUEUE_MAX_DEPTH 16
#endif

/**
 * @brief SPI queue completion callback signature.
 * @param bus       SPI bus index.
 * @param result    SYN_OK on success, error status code on failure.
 * @param user_data Context pointer passed with transaction descriptor.
 */
typedef void (*SYN_SPI_Queue_Callback)(uint8_t bus, SYN_Status result, void *user_data);

/**
 * @brief SPI Transaction Descriptor.
 */
typedef struct {
    uint8_t bus;                     /**< SPI bus index */
    SYN_GPIO_Pin cs_pin;             /**< Target Chip Select GPIO pin */
    SYN_SPI_Mode mode;               /**< Target SPI mode (0-3) */
    uint32_t baudrate_hz;            /**< Target baud rate in Hz (0 = default 1 MHz) */
    bool keep_cs_active;             /**< If true, leaves CS low upon transfer completion */
    const uint8_t *tx_data;          /**< TX buffer (NULL → send dummy 0xFF) */
    uint8_t *rx_data;                /**< RX buffer (NULL → discard RX) */
    size_t len;                      /**< Transfer byte count */
    SYN_SPI_Queue_Callback callback; /**< Completion callback */
    void *user_data;                 /**< User context pointer */
} SYN_SPI_Transaction;

/**
 * @brief SPI Transaction Queue instance handle.
 */
typedef struct {
    SYN_SPI_Transaction ring[SYN_SPI_QUEUE_MAX_DEPTH]; /**< Transaction ring buffer */
    uint32_t configured_baud_hz;                       /**< Last configured clock in Hz */
    uint16_t head;                                     /**< Head index (pop position) */
    uint16_t tail;                                     /**< Tail index (push position) */
    uint16_t count;                                    /**< Current enqueued count */
    uint8_t bus;                                       /**< Target SPI bus index */
    uint8_t configured_bus;                            /**< Last configured bus index */
    uint8_t configured_mode;                           /**< Last configured SPI mode */
    bool bus_configured;                               /**< True once bus initialized */
    bool active;                                       /**< Transaction in progress */
    bool initialized;                                  /**< Initialization flag */
} SYN_SPI_Queue;

/**
 * @brief Initialize an SPI transaction queue for a bus.
 * @param q   Pointer to queue handle.
 * @param bus Target SPI bus index.
 * @return SYN_OK on success, SYN_INVALID_PARAM if q is NULL.
 */
SYN_Status syn_spi_queue_init(SYN_SPI_Queue *q, uint8_t bus);

/**
 * @brief Enqueue an SPI transaction for non-blocking execution.
 * @param q  Pointer to initialized queue handle.
 * @param tx Transaction descriptor.
 * @return SYN_OK if enqueued, SYN_BUSY if queue is full, SYN_INVALID_PARAM on invalid inputs.
 */
SYN_Status syn_spi_queue_enqueue(SYN_SPI_Queue *q, const SYN_SPI_Transaction *tx);

/**
 * @brief Process queue state machine (triggers pending transaction if idle).
 * @param q Pointer to initialized queue handle.
 * @return SYN_OK on success.
 */
SYN_Status syn_spi_queue_process(SYN_SPI_Queue *q);

/**
 * @brief Cancel all pending transactions in queue.
 * @param q Pointer to initialized queue handle.
 * @return SYN_OK on success.
 */
SYN_Status syn_spi_queue_cancel_all(SYN_SPI_Queue *q);

/**
 * @brief Query current pending transaction count.
 * @param q Pointer to queue handle.
 * @return Number of queued items (0 if NULL or empty).
 */
size_t syn_spi_queue_count(const SYN_SPI_Queue *q);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SPI_QUEUE_H */
