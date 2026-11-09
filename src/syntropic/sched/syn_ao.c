#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_AO) || SYN_USE_AO

#if defined(SYN_USE_FSM) && !SYN_USE_FSM
#error "syn_ao requires SYN_USE_FSM=1 (table-driven FSM)"
#endif

#if defined(SYN_USE_SCHED) && !SYN_USE_SCHED
#error "syn_ao requires SYN_USE_SCHED=1"
#endif

/**
 * @file syn_ao.c
 * @brief Active Object framework implementation.
 */

#include "../port/syn_port_system.h"
#include "../sched/syn_sched.h"
#include "../util/syn_assert.h"
#include "syn_ao.h"

/** @cond INTERNAL */
static bool ao_dispatch_event(SYN_PT *pt, SYN_ActiveObject *ao, SYN_AO_Event ev)
{
    bool was_idle = PT_IS_IDLE(pt);
    if (was_idle) {
        pt->lc = 1U;
    }
    ao->last_event = ev;
    syn_fsm_dispatch(&ao->fsm, (SYN_FSM_Event)ev.sig);
    if (PT_IS_IDLE(pt)) {
        return true;
    }
    if (was_idle) {
        PT_INIT(pt);
    }
    return false;
}
/** @endcond */

/**
 * @brief Protothread entry for active objects -- dispatches queued events.
 * @param pt    Protothread state.
 * @param task  Owning scheduler task.
 * @return PT status.
 */
static SYN_PT_Status syn_ao_pt_run(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }

    SYN_ActiveObject *ao = (SYN_ActiveObject *)task->user_data;
    size_t burst_budget = syn_mailbox_pending(&ao->mailbox);
    if (burst_budget == 0U) {
        burst_budget = 1U;
    }

    PT_BEGIN(pt);

    for (;;) {
        /* Block task until mailbox is not empty */
        PT_BLOCK_CONDITION(pt, task, !syn_mailbox_empty(&ao->mailbox));

        SYN_AO_Event ev;
        if (syn_mailbox_receive(&ao->mailbox, &ev)) {
            if (ao_dispatch_event(pt, ao, ev)) {
                return PT_MORE;
            }
            burst_budget--;
            if (task->state != (uint8_t)SYN_TASK_READY) {
                PT_PASS(pt);
            }
        }
        if (!syn_mailbox_empty(&ao->mailbox) &&
            (burst_budget == 0U || syn_task_should_share(task))) {
            PT_CONTINUE(pt);
        }
    }

    PT_END(pt); /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */
}

void syn_ao_init(SYN_ActiveObject *ao, SYN_Task *task, const char *name, SYN_Prio priority,
                 const SYN_FSM_Transition *transitions, SYN_FSM_State initial_state,
                 void *mailbox_buf, size_t mailbox_cap)
{
    SYN_ASSERT(ao != NULL);
    SYN_ASSERT(task != NULL);
    SYN_ASSERT(mailbox_buf != NULL);
    SYN_ASSERT(mailbox_cap >= 2U);

    /* LCOV_EXCL_START: Defensive NULL/bounds check */
    if (ao == NULL || task == NULL || mailbox_buf == NULL || mailbox_cap < 2U) {
        return;
    }
    /* LCOV_EXCL_STOP */

    memset(&ao->last_event, 0, sizeof(ao->last_event));
    ao->task = task;

    /* Initialize event queue mailbox */
    syn_mailbox_init(&ao->mailbox, mailbox_buf, sizeof(SYN_AO_Event), mailbox_cap);

    /* Initialize FSM */
    syn_fsm_init(&ao->fsm, transitions, initial_state, name);
    syn_fsm_set_context(&ao->fsm, ao);

    /* Initialize bound scheduler task */
    syn_task_create(task, name, syn_ao_pt_run, priority, ao);
}

bool syn_ao_post(SYN_ActiveObject *ao, uint16_t sig, void *data)
{
    SYN_ASSERT(ao != NULL);
    if (ao == NULL) {
        return false; /* LCOV_EXCL_LINE: Defensive NULL check */
    }

    SYN_AO_Event ev;
    memset(&ev, 0, sizeof(ev));
    ev.sig = sig;
    ev.data = data;

    syn_port_enter_critical();
    bool success = syn_mailbox_post(&ao->mailbox, &ev);
    if (success && ao->task != NULL && ao->task->state == (uint8_t)SYN_TASK_BLOCKED) {
        syn_task_resume(ao->task);
    }
    syn_port_exit_critical();

    return success;
}

#endif /* SYN_USE_AO */
