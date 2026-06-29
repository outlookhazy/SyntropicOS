/**
 * @file syn_rate_limit.h
 * @brief Token bucket rate limiter — header-only.
 *
 * Throttle operations to N-per-second. Classic use cases: limit log
 * output, throttle retransmissions, cap sensor poll rate.
 *
 * @par Usage
 * @code
 *   SYN_RateLimit rl;
 *   syn_rate_limit_init(&rl, 10, 1000);  // 10 events per 1000ms
 *
 *   if (syn_rate_limit_allow(&rl)) {
 *       send_packet();
 *   }
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_RATE_LIMIT_H
#define SYN_RATE_LIMIT_H

#include "../port/syn_port_system.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Token bucket rate limiter instance. */
typedef struct {
    uint32_t  tokens;       /**< Current token count                     */
    uint32_t  max_tokens;   /**< Maximum (burst) capacity                */
    uint32_t  interval_ms;  /**< Refill interval (tokens refill to max)  */
    uint32_t  last_refill;  /**< Tick of last refill                     */
} SYN_RateLimit;

/**
 * @brief Initialize a rate limiter.
 *
 * @param rl           Rate limiter instance.
 * @param max_tokens   Maximum tokens (burst capacity).
 * @param interval_ms  Interval over which tokens refill to max.
 */
static inline void syn_rate_limit_init(SYN_RateLimit *rl,
                                        uint32_t max_tokens,
                                        uint32_t interval_ms)
{
    rl->tokens      = max_tokens;
    rl->max_tokens  = max_tokens;
    rl->interval_ms = interval_ms;
    rl->last_refill = syn_port_get_tick_ms();
}

/**
 * @brief Try to consume one token. Returns true if allowed.
 * @param rl  Rate limiter.
 * @return true if a token was consumed, false if exhausted.
 */
static inline bool syn_rate_limit_allow(SYN_RateLimit *rl)
{
    uint32_t now = syn_port_get_tick_ms();
    uint32_t elapsed = now - rl->last_refill;

    /* Refill tokens based on elapsed time */
    if (elapsed >= rl->interval_ms) {
        uint32_t refills = elapsed / rl->interval_ms;
        rl->tokens = rl->max_tokens; /* full refill */
        rl->last_refill += refills * rl->interval_ms;
    } else if (rl->interval_ms > 0) {
        /* Partial refill: proportional token recovery */
        uint32_t new_tokens = (rl->max_tokens * elapsed) / rl->interval_ms;
        if (new_tokens > 0) {
            rl->tokens += new_tokens;
            if (rl->tokens > rl->max_tokens) rl->tokens = rl->max_tokens;
            rl->last_refill = now;
        }
    }

    if (rl->tokens > 0) {
        rl->tokens--;
        return true;
    }
    return false;
}

/**
 * @brief Check remaining tokens without consuming.
 * @param rl  Rate limiter.
 * @return Remaining tokens.
 */
static inline uint32_t syn_rate_limit_remaining(const SYN_RateLimit *rl)
{
    return rl->tokens;
}

/**
 * @brief Reset the rate limiter to full capacity.
 * @param rl  Rate limiter.
 */
static inline void syn_rate_limit_reset(SYN_RateLimit *rl)
{
    rl->tokens      = rl->max_tokens;
    rl->last_refill = syn_port_get_tick_ms();
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_RATE_LIMIT_H */
