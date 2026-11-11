/**
 * @file syn_timer_wheel.c
 * @brief Hashed Timing Wheel Scheduler implementation.
 */

#include "syn_timer_wheel.h"

#include "../port/syn_port_spinlock.h"
#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"

#include <string.h>

/** @cond INTERNAL */
typedef char syn_timer_wheel_buckets_pow2_check[((SYN_TIMER_WHEEL_BUCKETS > 0U) &&
                                                 ((SYN_TIMER_WHEEL_BUCKETS &
                                                   (SYN_TIMER_WHEEL_BUCKETS - 1U)) == 0U))
                                                    ? 1
                                                    : -1];

typedef struct TimerWheelFrame {
    SYN_TimerWheel *wheel;
    SYN_TimerWheelNode *head;
    struct TimerWheelFrame *prev;
    struct TimerWheelFrame *next;
    bool aborted;
} TimerWheelFrame;

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE && defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1)
static volatile uint8_t s_frame_lock = 0U;
static TimerWheelFrame *s_step_frames = NULL;

static TimerWheelFrame **timer_wheel_frames_slot(void)
{
    return &s_step_frames;
}

static void timer_wheel_lock(void)
{
    syn_port_enter_critical();
    while (__atomic_test_and_set(&s_frame_lock, __ATOMIC_ACQUIRE)) {
        /* spin */
    }
}

static void timer_wheel_unlock(void)
{
    __atomic_clear(&s_frame_lock, __ATOMIC_RELEASE);
    syn_port_exit_critical();
}
#elif defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
static TimerWheelFrame *s_step_frames[4];

static TimerWheelFrame **timer_wheel_frames_slot(void)
{
    return &s_step_frames[syn_port_core_id() & 3U];
}

static void timer_wheel_lock(void)
{
    syn_port_enter_critical();
}

static void timer_wheel_unlock(void)
{
    syn_port_exit_critical();
}
#else
static TimerWheelFrame *s_step_frames = NULL;

static TimerWheelFrame **timer_wheel_frames_slot(void)
{
    return &s_step_frames;
}

static void timer_wheel_lock(void)
{
    syn_port_enter_critical();
}

static void timer_wheel_unlock(void)
{
    syn_port_exit_critical();
}
#endif

static void timer_wheel_push_frame(TimerWheelFrame *frame)
{
    TimerWheelFrame **slot = timer_wheel_frames_slot();
    frame->prev = NULL;
    frame->next = *slot;
    if (*slot != NULL) {
        (*slot)->prev = frame;
    }
    *slot = frame;
}

static void timer_wheel_unlink_frame(TimerWheelFrame *frame)
{
    TimerWheelFrame **slot = timer_wheel_frames_slot();
    if (frame->prev != NULL) {
        frame->prev->next = frame->next;
    } else {
        *slot = frame->next;
    }
    if (frame->next != NULL) {
        frame->next->prev = frame->prev;
    }
    frame->prev = NULL;
    frame->next = NULL;
}

static void timer_wheel_clear_list(SYN_TimerWheelNode *head)
{
    while (head != NULL) {
        SYN_TimerWheelNode *next = head->next;
        head->next = NULL;
        head->prev = NULL;
        head->active = false;
        head = next;
    }
}

static void timer_wheel_abort_active_steps(SYN_TimerWheel *wheel)
{
    bool was_stepping = false;
    for (TimerWheelFrame *f = *timer_wheel_frames_slot(); f != NULL; f = f->next) {
        if (f->wheel == wheel) {
            was_stepping = true;
            if (!f->aborted) {
                f->aborted = true;
                timer_wheel_clear_list(f->head);
                f->head = NULL;
            }
        }
    }
    if (was_stepping) {
        for (uint32_t i = 0U; i < SYN_TIMER_WHEEL_BUCKETS; i++) {
            timer_wheel_clear_list(wheel->buckets[i]);
        }
    }
}

static TimerWheelFrame *timer_wheel_find_step_frame(const SYN_TimerWheel *wheel,
                                                    const SYN_TimerWheelNode *root)
{
    for (TimerWheelFrame *f = *timer_wheel_frames_slot(); f != NULL; f = f->next) {
        if (!f->aborted && f->wheel == wheel && f->head == root) {
            return f;
        }
    }
    return NULL;
}

static void timer_wheel_restore_expired_head(SYN_TimerWheel *wheel)
{
    for (TimerWheelFrame *f = *timer_wheel_frames_slot(); f != NULL; f = f->next) {
        if (!f->aborted && f->wheel == wheel) {
            wheel->expired_head = f->head;
            return;
        }
    }
    wheel->expired_head = NULL;
}

static SYN_Status timer_wheel_cancel_locked(SYN_TimerWheel *wheel, SYN_TimerWheelNode *node)
{
    if (!node->active) {
        return SYN_INVALID_PARAM;
    }

    uint32_t bucket_idx = node->expires_tick % SYN_TIMER_WHEEL_BUCKETS;
    const SYN_TimerWheelNode *root = node;
    while (root->prev != NULL) {
        root = root->prev;
    }

    TimerWheelFrame *owner_frame =
        (wheel->buckets[bucket_idx] == root) ? NULL : timer_wheel_find_step_frame(wheel, root);
    if (wheel->buckets[bucket_idx] != root && owner_frame == NULL) {
        return SYN_INVALID_PARAM;
    }

    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else if (owner_frame != NULL) {
        owner_frame->head = node->next;
        if (wheel->expired_head == node) {
            wheel->expired_head = node->next;
        }
    } else {
        wheel->buckets[bucket_idx] = node->next;
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    }

    node->next = NULL;
    node->prev = NULL;
    node->active = false;

    return SYN_OK;
}
/** @endcond */

SYN_Status syn_timer_wheel_init(SYN_TimerWheel *wheel)
{
    if (!wheel)
        return SYN_INVALID_PARAM;

    timer_wheel_lock();
    timer_wheel_abort_active_steps(wheel);
    memset(wheel, 0, sizeof(*wheel));
    timer_wheel_unlock();

    return SYN_OK;
}

SYN_Status syn_timer_wheel_add(SYN_TimerWheel *wheel, SYN_TimerWheelNode *node,
                               uint32_t delay_ticks, SYN_TimerWheelCb cb, void *arg)
{
    if (!wheel || !node || !cb || delay_ticks == 0)
        return SYN_INVALID_PARAM;

    timer_wheel_lock();
    if (node->active && timer_wheel_cancel_locked(wheel, node) != SYN_OK) {
        timer_wheel_unlock();
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
    timer_wheel_unlock();

    return SYN_OK;
}

SYN_Status syn_timer_wheel_cancel(SYN_TimerWheel *wheel, SYN_TimerWheelNode *node)
{
    if (!wheel || !node) {
        return SYN_INVALID_PARAM;
    }

    timer_wheel_lock();
    SYN_Status st = timer_wheel_cancel_locked(wheel, node);
    timer_wheel_unlock();

    return st;
}

/**
 * @brief Partition a bucket into still-rotating nodes and ready-to-fire nodes.
 * @param wheel        Timer wheel instance.
 * @param bucket_idx   Index of bucket to partition.
 * @param out_exp_head Output pointer receiving head of expired nodes list.
 */
static void timer_wheel_partition_bucket(SYN_TimerWheel *wheel, uint32_t bucket_idx,
                                         SYN_TimerWheelNode **out_exp_head)
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
}

size_t syn_timer_wheel_step(SYN_TimerWheel *wheel)
{
    if (!wheel) {
        return 0U;
    }

    timer_wheel_lock();
    wheel->current_tick++;
    uint32_t bucket_idx = wheel->current_tick % SYN_TIMER_WHEEL_BUCKETS;
    SYN_TimerWheelNode *exp_head = NULL;

    timer_wheel_partition_bucket(wheel, bucket_idx, &exp_head);
    if (exp_head == NULL) {
        timer_wheel_unlock();
        return 0U;
    }

    TimerWheelFrame frame;
    frame.wheel = wheel;
    frame.head = exp_head;
    frame.aborted = false;
    timer_wheel_push_frame(&frame);
    wheel->expired_head = frame.head;

    size_t fired_count = 0U;
    while (!frame.aborted && frame.head != NULL) {
        SYN_TimerWheelNode *curr = frame.head;
        SYN_TimerWheelCb cb = curr->cb;
        void *arg = curr->arg;

        frame.head = curr->next;
        if (frame.head != NULL) {
            frame.head->prev = NULL;
        }
        wheel->expired_head = frame.head;

        curr->next = NULL;
        curr->prev = NULL;
        curr->active = false;
        timer_wheel_unlock();

        cb(arg);
        fired_count++;

        timer_wheel_lock();
    }

    timer_wheel_unlink_frame(&frame);
    if (!frame.aborted) {
        timer_wheel_restore_expired_head(wheel);
    }
    timer_wheel_unlock();

    return fired_count;
}
