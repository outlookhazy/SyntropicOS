/**
 * @file syn_timer_wheel.c
 * @brief Hashed Timing Wheel Scheduler implementation.
 */

#include "syn_timer_wheel.h"

#include "../util/syn_assert.h"

#include <string.h>

SYN_Status syn_timer_wheel_init(SYN_TimerWheel *wheel)
{
    if (!wheel)
        return SYN_INVALID_PARAM;

    memset(wheel, 0, sizeof(*wheel));
    wheel->current_tick = 0;

    return SYN_OK;
}

SYN_Status syn_timer_wheel_add(SYN_TimerWheel *wheel, SYN_TimerWheelNode *node,
                               uint32_t delay_ticks, SYN_TimerWheelCb cb, void *arg)
{
    if (!wheel || !node || !cb || delay_ticks == 0)
        return SYN_INVALID_PARAM;

    if (node->active && syn_timer_wheel_cancel(wheel, node) != SYN_OK) {
        return SYN_INVALID_PARAM;
    }

    uint32_t target_tick = wheel->current_tick + delay_ticks;
    uint32_t bucket_idx = target_tick % SYN_TIMER_WHEEL_BUCKETS;
    uint32_t rotations = (delay_ticks - 1U) / SYN_TIMER_WHEEL_BUCKETS;

    node->expires_tick = target_tick;
    node->rotation_count = rotations;
    node->cb = cb;
    node->arg = arg;
    node->active = true;
    node->next = wheel->buckets[bucket_idx];
    node->prev = NULL;

    if (wheel->buckets[bucket_idx]) {
        wheel->buckets[bucket_idx]->prev = node;
    }
    wheel->buckets[bucket_idx] = node;

    return SYN_OK;
}

SYN_Status syn_timer_wheel_cancel(SYN_TimerWheel *wheel, SYN_TimerWheelNode *node)
{
    if (!wheel || !node || !node->active) {
        return SYN_INVALID_PARAM;
    }

    uint32_t bucket_idx = node->expires_tick % SYN_TIMER_WHEEL_BUCKETS;
    const SYN_TimerWheelNode *root = node;
    while (root->prev != NULL) {
        root = root->prev;
    }
    if (wheel->buckets[bucket_idx] != root && wheel->expired_head != root) {
        return SYN_INVALID_PARAM;
    }

    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else if (wheel->buckets[bucket_idx] == node) {
        wheel->buckets[bucket_idx] = node->next;
    } else {
        wheel->expired_head = node->next;
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    }

    node->next = NULL;
    node->prev = NULL;
    node->active = false;

    return SYN_OK;
}

/**
 * @brief Partition a bucket into still-rotating nodes and ready-to-fire nodes.
 * @param wheel        Timer wheel instance.
 * @param bucket_idx   Index of bucket to partition.
 * @param out_exp_head Output pointer receiving head of expired nodes list.
 * @param out_exp_tail Output pointer receiving tail of expired nodes list.
 */
static void timer_wheel_partition_bucket(SYN_TimerWheel *wheel, uint32_t bucket_idx,
                                         SYN_TimerWheelNode **out_exp_head,
                                         SYN_TimerWheelNode **out_exp_tail)
{
    SYN_TimerWheelNode *curr = wheel->buckets[bucket_idx];
    SYN_TimerWheelNode *keep_head = NULL;
    SYN_TimerWheelNode *keep_tail = NULL;
    SYN_TimerWheelNode *exp_head = NULL;
    SYN_TimerWheelNode *exp_tail = NULL;

    while (curr != NULL) {
        SYN_TimerWheelNode *next_node = curr->next;
        curr->prev = NULL;
        curr->next = NULL;

        if (curr->rotation_count > 0U) {
            curr->rotation_count--;
            if (keep_tail != NULL) {
                keep_tail->next = curr;
                curr->prev = keep_tail;
            } else {
                keep_head = curr;
            }
            keep_tail = curr;
        } else {
            if (exp_tail != NULL) {
                exp_tail->next = curr;
                curr->prev = exp_tail;
            } else {
                exp_head = curr;
            }
            exp_tail = curr;
        }
        curr = next_node;
    }

    wheel->buckets[bucket_idx] = keep_head;
    *out_exp_head = exp_head;
    *out_exp_tail = exp_tail;
}

size_t syn_timer_wheel_step(SYN_TimerWheel *wheel)
{
    if (!wheel) {
        return 0U;
    }

    wheel->current_tick++;
    uint32_t bucket_idx = wheel->current_tick % SYN_TIMER_WHEEL_BUCKETS;
    SYN_TimerWheelNode *exp_head = NULL;
    SYN_TimerWheelNode *exp_tail = NULL;

    timer_wheel_partition_bucket(wheel, bucket_idx, &exp_head, &exp_tail);
    if (exp_head == NULL) {
        return 0U;
    }

    SYN_TimerWheelNode barrier;
    memset(&barrier, 0, sizeof(barrier));
    barrier.next = wheel->expired_head;
    if (wheel->expired_head != NULL) {
        wheel->expired_head->prev = &barrier;
    }
    barrier.prev = exp_tail;
    exp_tail->next = &barrier;
    wheel->expired_head = exp_head;

    size_t fired_count = 0U;
    while (wheel->expired_head != NULL && wheel->expired_head != &barrier) {
        SYN_TimerWheelNode *curr = wheel->expired_head;
        wheel->expired_head = curr->next;
        wheel->expired_head->prev = NULL;

        curr->next = NULL;
        curr->prev = NULL;
        curr->active = false;

        curr->cb(curr->arg);
        fired_count++;
    }

    if (wheel->expired_head == &barrier) {
        wheel->expired_head = barrier.next;
        if (wheel->expired_head != NULL) {
            wheel->expired_head->prev = NULL;
        }
    }

    return fired_count;
}
