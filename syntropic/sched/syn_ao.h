/**
 * @file syn_ao.h
 * @brief Active Object (Actor model) framework wrapper.
 *
 * Combines cooperative tasks, protothreads, mailboxes, and state machines
 * to enforce strict event-driven encapsulation.
 * @ingroup syn_sched
 */

#ifndef SYN_AO_H
#define SYN_AO_H

#include "../common/syn_defs.h"
#include "../sched/syn_task.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_mailbox.h"
#include "../util/syn_fsm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Active Object Event.
 */
typedef struct {
    uint16_t sig;   /**< Signal identifier                        */
    void    *data;  /**< Optional pointer to signal payload       */
} SYN_AO_Event;

/**
 * @brief Active Object structure.
 */
typedef struct {
    SYN_Task         task;     /**< Scheduler task handle                */
    SYN_PT           pt;       /**< Task protothread state               */
    SYN_Mailbox      mailbox;  /**< Event mailbox                        */
    SYN_FSM          fsm;      /**< State machine                        */
    SYN_AO_Event     last_event; /**< Most recent dispatched event (access in actions via ao->last_event) */
} SYN_ActiveObject;

/**
 * @brief Initialize an Active Object.
 *
 * @param ao             Active Object instance.
 * @param name           Name of the active object (used for task/logging).
 * @param transitions    Transition table for the internal state machine.
 * @param initial_state  Initial state of the state machine.
 * @param mailbox_buf    Buffer backing the mailbox event queue.
 * @param mailbox_cap    Capacity (number of elements) of the buffer.
 */
void syn_ao_init(SYN_ActiveObject *ao,
                 const char *name,
                 const SYN_FSM_Transition *transitions,
                 SYN_FSM_State initial_state,
                 void *mailbox_buf,
                 size_t mailbox_cap);

/**
 * @brief Post an event to the Active Object's mailbox.
 *
 * Thread-safe and ISR-safe. Protects the mailbox write using
 * critical sections.
 *
 * @param ao   Active Object instance.
 * @param sig  Signal identifier.
 * @param data Optional payload.
 * @return true if the event was successfully queued, false if queue is full.
 */
bool syn_ao_post(SYN_ActiveObject *ao, uint16_t sig, void *data);

#ifdef __cplusplus
}
#endif

#endif /* SYN_AO_H */
