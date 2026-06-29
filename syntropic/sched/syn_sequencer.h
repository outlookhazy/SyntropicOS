/**
 * @file syn_sequencer.h
 * @brief Timed action sequencer — choreograph steps with delays.
 *
 * Execute a sequence of actions with configurable delays between them.
 * Non-blocking — call `update()` from your main loop.
 *
 * @par Usage
 * @code
 *   void relay_on(void *ctx) { syn_gpio_write(RELAY_PIN, SYN_GPIO_HIGH); }
 *   void relay_off(void *ctx) { syn_gpio_write(RELAY_PIN, SYN_GPIO_LOW); }
 *   void read_sensor(void *ctx) { *(int*)ctx = adc_read(); }
 *
 *   static const SYN_SeqStep steps[] = {
 *       { relay_on,     NULL,  0 },      // step 0: relay on, no delay
 *       { NULL,         NULL,  200 },     // step 1: wait 200ms
 *       { read_sensor,  &val,  0 },      // step 2: read sensor
 *       { relay_off,    NULL,  100 },     // step 3: relay off, then wait 100ms
 *   };
 *
 *   SYN_Sequencer seq;
 *   syn_seq_init(&seq, steps, 4);
 *   syn_seq_start(&seq);
 *
 *   while (!syn_seq_is_done(&seq)) {
 *       syn_seq_update(&seq);
 *   }
 * @endcode
 * @ingroup syn_sched
 */

#ifndef SYN_SEQUENCER_H
#define SYN_SEQUENCER_H

#include "../port/syn_port_system.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Step action ────────────────────────────────────────────────────────── */

/** Action function called for each step. */
typedef void (*SYN_SeqAction)(void *ctx);

/** Sequence step descriptor. */
typedef struct {
    SYN_SeqAction  action;     /**< Action to execute (NULL = delay only)*/
    void           *ctx;        /**< Context for action                   */
    uint32_t        delay_ms;   /**< Delay AFTER action before next step  */
} SYN_SeqStep;

/* ── Sequencer state ────────────────────────────────────────────────────── */

/**
 * @brief Action sequencer execution states.
 */
typedef enum {
    SYN_SEQ_IDLE     = 0,            /**< Sequence is inactive or stopped */
    SYN_SEQ_RUNNING  = 1,            /**< Actively executing actions */
    SYN_SEQ_WAITING  = 2,            /**< Waiting on step delay timer */
    SYN_SEQ_DONE     = 3,            /**< Sequence completed all steps */
} SYN_SeqState;

/* ── Completion callback ────────────────────────────────────────────────── */

struct SYN_Sequencer;

/**
 * @brief Completion callback function signature.
 *
 * @param seq Pointer to the completed sequencer context.
 * @param ctx User context associated with the completion handler.
 */
typedef void (*SYN_SeqCompleteCallback)(struct SYN_Sequencer *seq,
                                          void *ctx);

/**
 * @brief Sequencer runtime context.
 */
typedef struct SYN_Sequencer {
    const SYN_SeqStep  *steps;          /**< Array of step descriptors to run */
    uint16_t             step_count;    /**< Total step count in sequence */
    uint16_t             current;       /**< Current step index            */
    SYN_SeqState        state;         /**< Current sequencer operational state */
    uint32_t             wait_start;    /**< Tick when delay started       */
    bool                 loop;          /**< Auto-restart on completion?   */
    uint16_t             loop_count;    /**< Times completed (for looping) */

    SYN_SeqCompleteCallback on_complete; /**< Callback function when sequence finishes */
    void                    *on_complete_ctx; /**< Context data for completion callback */
} SYN_Sequencer;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize a sequencer.
 *
 * @param seq    Sequencer instance.
 * @param steps  Array of step descriptors (must persist).
 * @param count  Number of steps.
 */
void syn_seq_init(SYN_Sequencer *seq,
                   const SYN_SeqStep *steps, uint16_t count);

/**
 * @brief Set completion callback.
 *
 * @param seq Sequencer instance.
 * @param cb  Callback function invoked on sequence end.
 * @param ctx Callback context data pointer.
 */
void syn_seq_on_complete(SYN_Sequencer *seq,
                          SYN_SeqCompleteCallback cb, void *ctx);

/**
 * @brief Enable looping (auto-restart on completion).
 *
 * @param seq  Sequencer instance.
 * @param loop True to loop sequence indefinitely, false for single run.
 */
void syn_seq_set_loop(SYN_Sequencer *seq, bool loop);

/**
 * @brief Start the sequence from the beginning.
 *
 * @param seq Sequencer instance.
 */
void syn_seq_start(SYN_Sequencer *seq);

/**
 * @brief Stop the sequence.
 *
 * @param seq Sequencer instance.
 */
void syn_seq_stop(SYN_Sequencer *seq);

/**
 * @brief Update the sequencer — call from main loop.
 *
 * Executes actions and manages delays non-blockingly.
 *
 * @param seq Sequencer instance.
 * @return true if the sequence completed this call.
 */
bool syn_seq_update(SYN_Sequencer *seq);

/**
 * @brief Check if done.
 *
 * @param seq Sequencer instance.
 * @return True if in IDLE or DONE state.
 */
static inline bool syn_seq_is_done(const SYN_Sequencer *seq)
{
    return seq->state == SYN_SEQ_DONE || seq->state == SYN_SEQ_IDLE;
}

/**
 * @brief Get current step index.
 *
 * @param seq Sequencer instance.
 * @return Index of the currently executing/waiting step.
 */
static inline uint16_t syn_seq_current_step(const SYN_Sequencer *seq)
{
    return seq->current;
}

/**
 * @brief Get loop iteration count.
 *
 * @param seq Sequencer instance.
 * @return Number of times the sequence has finished (if looping is enabled).
 */
static inline uint16_t syn_seq_loops(const SYN_Sequencer *seq)
{
    return seq->loop_count;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_SEQUENCER_H */
