

# File syn\_sequencer.c

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_sequencer.c**](syn__sequencer_8c.md)

[Go to the documentation of this file](syn__sequencer_8c.md)


```C++
#include "../common/syn_defs.h"

#if !defined(SYN_USE_SEQUENCER) || SYN_USE_SEQUENCER


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
    seq->updating = true;

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

static bool complete_sequence(SYN_Sequencer *seq, bool *out_done)
{
    uint16_t last_idx = (uint16_t)(seq->step_count - 1U);
    seq->loop_count++;
    if (seq->loop) {
        seq->current = 0;
        seq->state = SYN_SEQ_RUNNING;
    } else {
        seq->current = last_idx;
        seq->state = SYN_SEQ_DONE;
    }

    seq->cb_untouched = true;
    if (seq->on_complete != NULL) {
        seq->on_complete(seq, seq->on_complete_ctx);
    }
    bool untouched = seq->cb_untouched;
    seq->cb_untouched = false;
    seq->updating = true;

    if (untouched) {
        if (seq->loop) {
            seq->current = 0;
            seq->state = SYN_SEQ_RUNNING;
        } else {
            seq->current = last_idx;
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

static bool seq_is_runnable(const SYN_Sequencer *seq)
{
    return seq != NULL && seq->steps != NULL && !seq->updating && seq->state != SYN_SEQ_IDLE &&
           seq->state != SYN_SEQ_DONE;
}

static bool seq_advance_wait(SYN_Sequencer *seq)
{
    uint32_t raw_delay = seq->steps[seq->current].delay_ms;
    uint32_t target_delay = (raw_delay > 0x7FFFFFFFU) ? 0x7FFFFFFFU : raw_delay;
    uint32_t elapsed = syn_port_get_tick_ms() - seq->wait_start;
    if (elapsed < target_delay) {
        return false;
    }
    seq->current++;
    seq->state = SYN_SEQ_RUNNING;
    return true;
}

bool syn_seq_update(SYN_Sequencer *seq)
{
    if (!seq_is_runnable(seq)) {
        return false;
    }

    seq->updating = true;
    bool done = false;

    if (seq->state == SYN_SEQ_WAITING) {
        if (!seq_advance_wait(seq)) {
            seq->updating = false;
            return false;
        }
        if (seq->current >= seq->step_count && complete_sequence(seq, &done)) {
            seq->updating = false;
            return done;
        }
    }

    /* Execute steps (bounded per call; 0-delay loops complete 1 pass per update) */
    size_t executed = 0U;
    while (seq->state == SYN_SEQ_RUNNING && executed <= (size_t)seq->step_count) {
        execute_step(seq);
        executed++;

        if (seq->state == SYN_SEQ_RUNNING && seq->current >= seq->step_count) {
            if (complete_sequence(seq, &done)) {
                seq->updating = false;
                return done;
            }
            if (executed >= (size_t)seq->step_count) {
                break;
            }
        }
    }

    done = (seq->state == SYN_SEQ_DONE);
    seq->updating = false;
    return done;
}

#endif /* SYN_USE_SEQUENCER */
```


