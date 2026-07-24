/**
 * @file syn_timer_wheel.c
 * @brief Hashed Timing Wheel Scheduler implementation.
 */

#include "syn_timer_wheel.h"
#include <string.h>

SYN_Status syn_timer_wheel_init(SYN_TimerWheel *wheel)
{
    if (!wheel) return SYN_INVALID_PARAM;

    memset(wheel, 0, sizeof(*wheel));
    wheel->current_tick = 0;

    return SYN_OK;
}

SYN_Status syn_timer_wheel_add(SYN_TimerWheel *wheel, SYN_TimerWheelNode *node, uint32_t delay_ticks, SYN_TimerWheelCb cb, void *arg)
{
    if (!wheel || !node || !cb || delay_ticks == 0) return SYN_INVALID_PARAM;

    if (node->active) {
        syn_timer_wheel_cancel(wheel, node);
    }

    uint32_t target_tick = wheel->current_tick + delay_ticks;
    uint32_t bucket_idx  = target_tick % SYN_TIMER_WHEEL_BUCKETS;
    uint32_t rotations   = delay_ticks / SYN_TIMER_WHEEL_BUCKETS;

    node->expires_tick   = target_tick;
    node->rotation_count = rotations;
    node->cb             = cb;
    node->arg            = arg;
    node->active         = true;
    node->next           = wheel->buckets[bucket_idx];
    node->prev           = NULL;

    if (wheel->buckets[bucket_idx]) {
        wheel->buckets[bucket_idx]->prev = node;
    }
    wheel->buckets[bucket_idx] = node;

    return SYN_OK;
}

SYN_Status syn_timer_wheel_cancel(SYN_TimerWheel *wheel, SYN_TimerWheelNode *node)
{
    if (!wheel || !node || !node->active) return SYN_INVALID_PARAM;

    uint32_t bucket_idx = node->expires_tick % SYN_TIMER_WHEEL_BUCKETS;

    if (node->prev) {
        node->prev->next = node->next;
    } else {
        wheel->buckets[bucket_idx] = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    }

    node->next   = NULL;
    node->prev   = NULL;
    node->active = false;

    return SYN_OK;
}

size_t syn_timer_wheel_step(SYN_TimerWheel *wheel)
{
    if (!wheel) return 0;

    wheel->current_tick++;
    uint32_t bucket_idx = wheel->current_tick % SYN_TIMER_WHEEL_BUCKETS;
    SYN_TimerWheelNode *curr = wheel->buckets[bucket_idx];
    size_t fired_count = 0;

    while (curr) {
        SYN_TimerWheelNode *next_node = curr->next;
        if (curr->rotation_count > 0) {
            curr->rotation_count--;
        } else {
            /* Unlink timer node */
            if (curr->prev) {
                curr->prev->next = curr->next;
            } else {
                wheel->buckets[bucket_idx] = curr->next;
            }
            if (curr->next) {
                curr->next->prev = curr->prev;
            }

            curr->next   = NULL;
            curr->prev   = NULL;
            curr->active = false;

            /* Invoke callback */
            if (curr->cb) {
                curr->cb(curr->arg);
                fired_count++;
            }
        }
        curr = next_node;
    }

    return fired_count;
}
