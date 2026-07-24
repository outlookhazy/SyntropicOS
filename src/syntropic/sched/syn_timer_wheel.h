/**
 * @file syn_timer_wheel.h
 * @brief Hashed Timing Wheel Scheduler.
 * @ingroup syn_sched
 *
 * Implements an O(1) constant-time hashed timing wheel scheduler optimized for
 * low-power tickless Operation with hundreds of concurrent software timers.
 */

#ifndef SYN_TIMER_WHEEL_H
#define SYN_TIMER_WHEEL_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_TIMER_WHEEL_BUCKETS 64U

typedef void (*SYN_TimerWheelCb)(void *arg);

/**
 * @brief Timer Wheel Node Structure.
 */
typedef struct SYN_TimerWheelNode {
    struct SYN_TimerWheelNode *next; /**< Pointer to next node in bucket      */
    struct SYN_TimerWheelNode *prev; /**< Pointer to previous node in bucket  */
    uint32_t         expires_tick;   /**< Target expiration tick              */
    uint32_t         rotation_count; /**< Rounds remaining before firing     */
    SYN_TimerWheelCb cb;             /**< Callback function pointer           */
    void            *arg;            /**< Callback user argument              */
    bool             active;         /**< Timer active flag                   */
} SYN_TimerWheelNode;

/**
 * @brief Hashed Timing Wheel Descriptor.
 */
typedef struct {
    SYN_TimerWheelNode *buckets[SYN_TIMER_WHEEL_BUCKETS]; /**< Bucket list array */
    uint32_t current_tick;                                 /**< Current wheel tick */
} SYN_TimerWheel;

/**
 * @brief Initialize a timing wheel scheduler.
 * @param wheel Pointer to wheel descriptor.
 * @return SYN_OK on success.
 */
SYN_Status syn_timer_wheel_init(SYN_TimerWheel *wheel);

/**
 * @brief Schedule a timer in the timing wheel.
 * @param wheel Pointer to wheel descriptor.
 * @param node Pointer to timer node memory.
 * @param delay_ticks Delay in ticks before timer fires.
 * @param cb Expiration callback.
 * @param arg Callback user argument.
 * @return SYN_OK on success.
 */
SYN_Status syn_timer_wheel_add(SYN_TimerWheel *wheel, SYN_TimerWheelNode *node, uint32_t delay_ticks, SYN_TimerWheelCb cb, void *arg);

/**
 * @brief Cancel a scheduled timer node.
 * @param wheel Pointer to wheel descriptor.
 * @param node Pointer to timer node.
 * @return SYN_OK on success.
 */
SYN_Status syn_timer_wheel_cancel(SYN_TimerWheel *wheel, SYN_TimerWheelNode *node);

/**
 * @brief Advance the timing wheel by one tick and fire expired timers.
 * @param wheel Pointer to wheel descriptor.
 * @return Number of timers fired during this tick step.
 */
size_t syn_timer_wheel_step(SYN_TimerWheel *wheel);

#ifdef __cplusplus
}
#endif

#endif /* SYN_TIMER_WHEEL_H */
