#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_FSM) || SYN_USE_FSM

/**
 * @file syn_fsm.c
 * @brief Finite state machine implementation.
 */

#include "syn_fsm.h"
#include "syn_assert.h"
#include "../log/syn_log.h"

#include <string.h>

/* ── Internal helpers ───────────────────────────────────────────────────── */

/**
 * @brief Find the state descriptor for a given state.
 * @param fsm    FSM instance.
 * @param state  State to look up.
 * @return Descriptor, or NULL if not found.
 */
static const SYN_FSM_StateDesc *find_state_desc(const SYN_FSM *fsm,
                                                  SYN_FSM_State state)
{
    if (fsm->state_descs == NULL) return NULL;

    const SYN_FSM_StateDesc *d = fsm->state_descs;
    while (d->state != SYN_FSM_STATE_NONE) {
        if (d->state == state) return d;
        d++;
    }
    return NULL;
}

/**
 * @brief Invoke the on_exit callback for a state.
 * @param fsm    FSM instance.
 * @param state  State being exited.
 */
static void fire_exit(const SYN_FSM *fsm, SYN_FSM_State state)
{
    const SYN_FSM_StateDesc *d = find_state_desc(fsm, state);
    if (d != NULL && d->on_exit != NULL) {
        d->on_exit(fsm->ctx);
    }
}

/**
 * @brief Invoke the on_enter callback for a state.
 * @param fsm    FSM instance.
 * @param state  State being entered.
 */
static void fire_enter(const SYN_FSM *fsm, SYN_FSM_State state)
{
    const SYN_FSM_StateDesc *d = find_state_desc(fsm, state);
    if (d != NULL && d->on_enter != NULL) {
        d->on_enter(fsm->ctx);
    }
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_fsm_init(SYN_FSM *fsm,
                   const SYN_FSM_Transition *transitions,
                   SYN_FSM_State initial,
                   const char *tag)
{
    SYN_ASSERT(fsm != NULL);
    SYN_ASSERT(transitions != NULL);

    memset(fsm, 0, sizeof(*fsm));
    fsm->transitions = transitions;
    fsm->current     = initial;
    fsm->tag         = tag;
}

void syn_fsm_set_state_descs(SYN_FSM *fsm, const SYN_FSM_StateDesc *descs)
{
    SYN_ASSERT(fsm != NULL);
    fsm->state_descs = descs;
}

void syn_fsm_set_state_names(SYN_FSM *fsm, const char * const *names)
{
    SYN_ASSERT(fsm != NULL);
    fsm->state_names = names;
}

void syn_fsm_set_context(SYN_FSM *fsm, void *ctx)
{
    SYN_ASSERT(fsm != NULL);
    fsm->ctx = ctx;
}

bool syn_fsm_dispatch(SYN_FSM *fsm, SYN_FSM_Event event)
{
    SYN_ASSERT(fsm != NULL);

    const SYN_FSM_Transition *t = fsm->transitions;

    while (t->from != SYN_FSM_STATE_NONE) {
        if (t->from == fsm->current && t->event == event) {
            /* Check guard */
            if (t->guard != NULL && !t->guard(fsm->ctx)) {
                t++;
                continue; /* try next matching row */
            }

            SYN_FSM_State old_state = fsm->current;

            /* Exit old state */
            fire_exit(fsm, old_state);

            /* Transition action */
            if (t->action != NULL) {
                t->action(fsm->ctx);
            }

            /* Change state */
            fsm->current = t->to;

            /* Log transition */
#if !defined(SYN_USE_LOG) || SYN_USE_LOG
            if (fsm->tag != NULL) {
                if (fsm->state_names != NULL) {
                    SYN_LOG_D(fsm->tag, "%s -> %s (evt %d)",
                               fsm->state_names[old_state],
                               fsm->state_names[t->to],
                               (int)event);
                } else {
                    SYN_LOG_D(fsm->tag, "S%d -> S%d (evt %d)",
                               (int)old_state, (int)t->to, (int)event);
                }
            }
#endif
            /* Enter new state */
            fire_enter(fsm, t->to);

            return true;
        }
        t++;
    }

    return false; /* no matching transition */
}

void syn_fsm_set_state(SYN_FSM *fsm, SYN_FSM_State state)
{
    SYN_ASSERT(fsm != NULL);

    if (fsm->current != state) {
        fire_exit(fsm, fsm->current);
        fsm->current = state;
        fire_enter(fsm, state);
    }
}

#endif /* SYN_USE_FSM */
