/**
 * @file syn_backoff.h
 * @brief Exponential backoff helper with jitter (RFC 7252 style).
 * @ingroup syn_util
 */

#ifndef SYN_BACKOFF_H
#define SYN_BACKOFF_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Exponential backoff state. */
typedef struct {
    uint32_t min_ms;      /**< Initial timeout (ms)           */
    uint32_t max_ms;      /**< Maximum timeout (ms)           */
    uint32_t current_ms;  /**< Current timeout duration (ms) */
    uint8_t  attempts;    /**< Number of failed attempts      */
    uint8_t  max_attempts; /**< Maximum number of attempts    */
    uint8_t  factor;      /**< Multiplier (usually 2)         */
} SYN_Backoff;

/**
 * @brief Initialize backoff state.
 * @param b       Backoff context.
 * @param min_ms  Initial delay (e.g., 1000).
 * @param max_ms  Maximum delay (e.g., 60000).
 * @param factor  Exponential factor (usually 2).
 * @param max_attempts Maximum number of attempts.
 */
void syn_backoff_init(SYN_Backoff *b, uint32_t min_ms, uint32_t max_ms, uint8_t factor, uint8_t max_attempts);

/**
 * @brief Get the next delay in milliseconds.
 *
 * Each call increases the delay exponentially (up to max_ms).
 * Includes RFC 7252-style jitter (1.0x to 1.5x) to avoid synchronization.
 *
 * @param b  Backoff context.
 * @return Next delay duration in milliseconds.
 */
uint32_t syn_backoff_next_ms(SYN_Backoff *b);

/**
 * @brief Check if the maximum number of attempts has been reached.
 * @param b  Backoff context.
 * @return true if attempts >= max_attempts.
 */
bool syn_backoff_exhausted(const SYN_Backoff *b);

/**
 * @brief Reset the backoff state to the initial minimum delay.
 * @param b  Backoff context.
 */
void syn_backoff_reset(SYN_Backoff *b);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BACKOFF_H */
