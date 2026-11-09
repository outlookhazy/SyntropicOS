#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_SEQUENCER) || SYN_USE_SEQUENCER

/**
 * @file syn_sequencer.c
 * @brief Timed action sequencer implementation.
 */

#include "../util/syn_assert.h"
#include "syn_sequencer.h"

#include <string.h>

void syn_seq_init(SYN_Sequencer *seq, const SYN_SeqStep *steps, uint16_t count)
{
    SYN_ASSERT(seq != NULL);
    SYN_ASSERT(steps != NULL);
    SYN_ASSERT(count > 0);

    if (seq == NULL || steps == NULL ||
        count == 0) { /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */
        return;       /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }

    memset(seq, 0, sizeof(*seq));
    seq->steps = steps;
    seq->step_count = count;
    seq->state = SYN_SEQ_IDLE;
}

void syn_seq_on_complete(SYN_Sequencer *seq, SYN_SeqCompleteCallback cb, void *ctx)
{
    SYN_ASSERT(seq != NULL);

    if (seq == NULL) { /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */
        return;        /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }
    seq->on_complete = cb;
    seq->on_complete_ctx = ctx;
}

void syn_seq_set_loop(SYN_Sequencer *seq, bool loop)
{
    SYN_ASSERT(seq != NULL);

    if (seq == NULL) { /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */
        return;        /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }
    seq->loop = loop;
}

void syn_seq_start(SYN_Sequencer *seq)
{
    SYN_ASSERT(seq != NULL);

    if (seq == NULL) { /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */
        return;        /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }
    seq->current = 0;
    seq->cb_untouched = false;
    seq->state = SYN_SEQ_RUNNING;
}

void syn_seq_stop(SYN_Sequencer *seq)
{
    SYN_ASSERT(seq != NULL);

    if (seq == NULL) { /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */
        return;        /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }
    seq->cb_untouched = false;
    seq->state = SYN_SEQ_IDLE;
}

/**
 * @brief Execute the current sequence step's action.
 * @param seq  Sequencer instance.
 */
static void execute_step(SYN_Sequencer *seq)
{
    const SYN_SeqStep *step = &seq->steps[seq->current];

    seq->cb_untouched = true;
    /* Execute action if present */
    if (step->action != NULL) {
        step->action(step->ctx);
    }
    bool untouched = seq->cb_untouched;
    seq->cb_untouched = false;

    /* Respect init/start/stop/nested-update changes requested inside action callback */
    if (!untouched) {
        return;
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

/** @cond INTERNAL */
static bool complete_sequence(SYN_Sequencer *seq, bool *out_done)
{
    seq->loop_count++;
    if (seq->loop) {
        seq->current = 0;
        seq->state = SYN_SEQ_RUNNING;
    } else {
        seq->state = SYN_SEQ_DONE;
    }

    seq->cb_untouched = true;
    if (seq->on_complete != NULL) {
        seq->on_complete(seq, seq->on_complete_ctx);
    }
    bool untouched = seq->cb_untouched;
    seq->cb_untouched = false;

    if (untouched) {
        if (seq->loop) {
            seq->current = 0;
            seq->state = SYN_SEQ_RUNNING;
        } else {
            seq->state = SYN_SEQ_DONE;
        }
    }

    if (seq->state == SYN_SEQ_DONE) {
        *out_done = true;
        return true;
    }
    if (seq->state != SYN_SEQ_RUNNING) {
        *out_done = false;
        return true;
    }
    return false;
}
/** @endcond */

bool syn_seq_update(SYN_Sequencer *seq)
{
    if (seq == NULL || seq->steps == NULL) {
        return false;
    }
    SYN_ASSERT(seq != NULL);

    if (seq->state == SYN_SEQ_IDLE || seq->state == SYN_SEQ_DONE) {
        return false;
    }

    if (seq->state == SYN_SEQ_WAITING) {
        uint32_t raw_delay = seq->steps[seq->current].delay_ms;
        uint32_t target_delay = (raw_delay > 0x7FFFFFFFU) ? 0x7FFFFFFFU : raw_delay;
        uint32_t elapsed = syn_port_get_tick_ms() - seq->wait_start;
        if (elapsed < target_delay) {
            return false; /* still waiting */
        }
        /* Delay complete — advance */
        seq->current++;
        seq->state = SYN_SEQ_RUNNING;
    }

    /* Execute steps (limit execution per tick to step_count + 1 to prevent infinite loops on
     * 0-delay loop sequences while allowing completion check) */
    size_t max_exec = (size_t)seq->step_count + 1;
    size_t executed = 0;
    while (seq->state == SYN_SEQ_RUNNING && executed < max_exec) {
        if (seq->current >= seq->step_count) {
            bool done = false;
            if (complete_sequence(seq, &done)) {
                return done;
            }
        }

        execute_step(seq);
        executed++;
    }

    return false;
}

#endif /* SYN_USE_SEQUENCER */
