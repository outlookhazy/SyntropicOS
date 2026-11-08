/**
 * @file syn_i2c_queue.h
 * @brief Non-blocking I2C transaction queue driver for multi-client bus access.
 * @ingroup syn_drivers
 */

#ifndef SYN_I2C_QUEUE_H
#define SYN_I2C_QUEUE_H

#include "../common/syn_defs.h"
#include "../port/syn_port_i2c.h"
#include "../port/syn_port_i2c_async.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum pending transactions per I2C queue instance */
#ifndef SYN_I2C_QUEUE_MAX_DEPTH
#define SYN_I2C_QUEUE_MAX_DEPTH 16
#endif

/**
 * @brief I2C queue completion callback signature.
 * @param bus       I2C bus index.
 * @param result    SYN_OK on success, error status code on failure.
 * @param user_data Context pointer passed with transaction descriptor.
 */
typedef void (*SYN_I2C_Queue_Callback)(uint8_t bus, SYN_Status result, void *user_data);

/**
 * @brief I2C Transaction Descriptor.
 */
typedef struct {
    uint8_t bus;                     /**< I2C bus index */
    uint16_t addr;                   /**< 7-bit target slave address */
    uint32_t clock_speed_hz;         /**< Target clock frequency in Hz (0 = default 100 kHz) */
    const uint8_t *tx_data;          /**< TX buffer (NULL if read-only) */
    size_t tx_len;                   /**< TX byte count */
    uint8_t *rx_data;                /**< RX buffer (NULL if write-only) */
    size_t rx_len;                   /**< RX byte count */
    SYN_I2C_Queue_Callback callback; /**< Completion callback */
    void *user_data;                 /**< User context pointer */
} SYN_I2C_Transaction;

/**
 * @brief I2C Transaction Queue instance handle.
 */
typedef struct {
    SYN_I2C_Transaction ring[SYN_I2C_QUEUE_MAX_DEPTH]; /**< Transaction ring buffer */
    uint32_t configured_clock_hz;                      /**< Last configured bus speed in Hz */
    uint16_t head;                                     /**< Head index (pop position) */
    uint16_t tail;                                     /**< Tail index (push position) */
    uint16_t count;                                    /**< Current enqueued count */
    uint8_t bus;                                       /**< Target I2C bus index */
    uint8_t configured_bus;                            /**< Last configured bus index */
    bool bus_configured;                               /**< True once bus clock initialized */
    bool active;                                       /**< Transaction in progress */
    bool initialized;                                  /**< Initialization flag */
} SYN_I2C_Queue;

/**
 * @brief Initialize an I2C transaction queue for a bus.
 * @param q   Pointer to queue handle.
 * @param bus Target I2C bus index.
 * @return SYN_OK on success, SYN_INVALID_PARAM if q is NULL.
 */
SYN_Status syn_i2c_queue_init(SYN_I2C_Queue *q, uint8_t bus);

/**
 * @brief Enqueue a transaction for non-blocking execution.
 * @param q  Pointer to initialized queue handle.
 * @param tx Transaction descriptor.
 * @return SYN_OK if enqueued, SYN_BUSY if queue is full, SYN_INVALID_PARAM on invalid inputs.
 */
SYN_Status syn_i2c_queue_enqueue(SYN_I2C_Queue *q, const SYN_I2C_Transaction *tx);

/**
 * @brief Process queue state machine (triggers pending transaction if idle).
 * @param q Pointer to initialized queue handle.
 * @return SYN_OK on success.
 */
SYN_Status syn_i2c_queue_process(SYN_I2C_Queue *q);

/**
 * @brief Cancel all pending transactions in queue.
 * @param q Pointer to initialized queue handle.
 * @return SYN_OK on success.
 */
SYN_Status syn_i2c_queue_cancel_all(SYN_I2C_Queue *q);

/**
 * @brief Query current pending transaction count.
 * @param q Pointer to queue handle.
 * @return Number of queued items (0 if NULL or empty).
 */
size_t syn_i2c_queue_count(const SYN_I2C_Queue *q);

#ifdef __cplusplus
}
#endif

#endif /* SYN_I2C_QUEUE_H */
