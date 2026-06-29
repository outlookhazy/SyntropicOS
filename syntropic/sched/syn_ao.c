#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_AO) || SYN_USE_AO

#if defined(SYN_USE_FSM) && !SYN_USE_FSM
  #error "syn_ao requires SYN_USE_FSM=1 (table-driven FSM)"
#endif

/**
 * @file syn_ao.c
 * @brief Active Object framework implementation.
 */

#include "syn_ao.h"
#include "../sched/syn_sched.h"
#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"

/**
 * @brief Protothread entry for active objects -- dispatches queued events.
 * @param pt    Protothread state.
 * @param task  Owning scheduler task.
 * @return PT status.
 */
static SYN_PT_Status syn_ao_pt_run(SYN_PT *pt, SYN_Task *task)
{
    SYN_ActiveObject *ao = (SYN_ActiveObject *)task->user_data;
    SYN_ASSERT(ao != NULL);

    PT_BEGIN(pt);

    for (;;) {
        /* Yield until mailbox is not empty */
        PT_WAIT_UNTIL(pt, !syn_mailbox_empty(&ao->mailbox));

        SYN_AO_Event ev;
        bool has_msg = syn_mailbox_receive(&ao->mailbox, &ev);
        if (has_msg) {
            ao->last_event = ev;
            syn_fsm_dispatch(&ao->fsm, (SYN_FSM_Event)ev.sig);
        }
    }

    PT_END(pt);
}

void syn_ao_init(SYN_ActiveObject *ao,
                 const char *name,
                 const SYN_FSM_Transition *transitions,
                 SYN_FSM_State initial_state,
                 void *mailbox_buf,
                 size_t mailbox_cap)
{
    SYN_ASSERT(ao != NULL);
    SYN_ASSERT(mailbox_buf != NULL);
    SYN_ASSERT(mailbox_cap > 0);

    /* Initialize event queue mailbox */
    syn_mailbox_init(&ao->mailbox, mailbox_buf, sizeof(SYN_AO_Event), mailbox_cap);

    /* Initialize FSM */
    syn_fsm_init(&ao->fsm, transitions, initial_state, name);
    syn_fsm_set_context(&ao->fsm, ao);

    /* Initialize protothread and task */
    PT_INIT(&ao->pt);
    syn_task_create(&ao->task, name, syn_ao_pt_run, 0, ao);
}

bool syn_ao_post(SYN_ActiveObject *ao, uint16_t sig, void *data)
{
    SYN_ASSERT(ao != NULL);
    SYN_AO_Event ev = { .sig = sig, .data = data };

    syn_port_enter_critical();
    bool success = syn_mailbox_post(&ao->mailbox, &ev);
    syn_port_exit_critical();

    return success;
}

#endif /* SYN_USE_AO */
