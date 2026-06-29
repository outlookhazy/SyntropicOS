#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SEQUENCER) || SYN_USE_SEQUENCER

/**
 * @file syn_sequencer.c
 * @brief Timed action sequencer implementation.
 */

#include "syn_sequencer.h"
#include "../util/syn_assert.h"

#include <string.h>

void syn_seq_init(SYN_Sequencer *seq,
                   const SYN_SeqStep *steps, uint16_t count)
{
    SYN_ASSERT(seq != NULL);
    SYN_ASSERT(steps != NULL);
    SYN_ASSERT(count > 0);

    memset(seq, 0, sizeof(*seq));
    seq->steps      = steps;
    seq->step_count = count;
    seq->state      = SYN_SEQ_IDLE;
}

void syn_seq_on_complete(SYN_Sequencer *seq,
                          SYN_SeqCompleteCallback cb, void *ctx)
{
    SYN_ASSERT(seq != NULL);
    seq->on_complete     = cb;
    seq->on_complete_ctx = ctx;
}

void syn_seq_set_loop(SYN_Sequencer *seq, bool loop)
{
    SYN_ASSERT(seq != NULL);
    seq->loop = loop;
}

void syn_seq_start(SYN_Sequencer *seq)
{
    SYN_ASSERT(seq != NULL);
    seq->current = 0;
    seq->state   = SYN_SEQ_RUNNING;
}

void syn_seq_stop(SYN_Sequencer *seq)
{
    SYN_ASSERT(seq != NULL);
    seq->state = SYN_SEQ_IDLE;
}

/**
 * @brief Execute the current sequence step's action.
 * @param seq  Sequencer instance.
 */
static void execute_step(SYN_Sequencer *seq)
{
    const SYN_SeqStep *step = &seq->steps[seq->current];

    /* Execute action if present */
    if (step->action != NULL) {
        step->action(step->ctx);
    }

    /* Start delay if present */
    if (step->delay_ms > 0) {
        seq->wait_start = syn_port_get_tick_ms();
        seq->state = SYN_SEQ_WAITING;
    } else {
        /* No delay — advance immediately */
        seq->current++;
    }
}

bool syn_seq_update(SYN_Sequencer *seq)
{
    SYN_ASSERT(seq != NULL);

    if (seq->state == SYN_SEQ_IDLE || seq->state == SYN_SEQ_DONE) {
        return false;
    }

    if (seq->state == SYN_SEQ_WAITING) {
        uint32_t elapsed = syn_port_get_tick_ms() - seq->wait_start;
        if (elapsed < seq->steps[seq->current].delay_ms) {
            return false; /* still waiting */
        }
        /* Delay complete — advance */
        seq->current++;
        seq->state = SYN_SEQ_RUNNING;
    }

    /* Execute steps (may chain multiple zero-delay steps) */
    while (seq->state == SYN_SEQ_RUNNING) {
        if (seq->current >= seq->step_count) {
            /* Sequence complete */
            seq->loop_count++;

            if (seq->on_complete != NULL) {
                seq->on_complete(seq, seq->on_complete_ctx);
            }

            if (seq->loop) {
                seq->current = 0;
                /* Continue running */
            } else {
                seq->state = SYN_SEQ_DONE;
                return true;
            }
        }

        execute_step(seq);
    }

    return false;
}

#endif /* SYN_USE_SEQUENCER */
