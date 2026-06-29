/**
 * @file syn_fsm.h
 * @brief Lightweight table-driven finite state machine.
 *
 * Define states and events as enums, then describe transitions in a
 * const table. The FSM dispatches events, calls transition actions,
 * and optionally logs every transition via the logging module.
 *
 * @par Usage
 * @code
 *   enum { ST_IDLE, ST_RUNNING, ST_ERROR };
 *   enum { EV_START, EV_STOP, EV_FAULT };
 *
 *   void on_start(void *ctx) { motor_enable(); }
 *   void on_stop(void *ctx)  { motor_disable(); }
 *
 *   static const SYN_FSM_Transition table[] = {
 *       { ST_IDLE,    EV_START, ST_RUNNING, NULL, on_start },
 *       { ST_RUNNING, EV_STOP,  ST_IDLE,    NULL, on_stop  },
 *       { ST_RUNNING, EV_FAULT, ST_ERROR,   NULL, NULL     },
 *       SYN_FSM_END
 *   };
 *
 *   static SYN_FSM fsm;
 *   syn_fsm_init(&fsm, table, ST_IDLE, "motor");
 *   syn_fsm_dispatch(&fsm, EV_START);
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_FSM_H
#define SYN_FSM_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Types ──────────────────────────────────────────────────────────────── */

typedef int16_t SYN_FSM_State;  /**< FSM state type.  */
typedef int16_t SYN_FSM_Event;  /**< FSM event type.  */

/** Sentinel value marking the end of a transition table. */
#define SYN_FSM_STATE_NONE   ((SYN_FSM_State)-1)

/**
 * @brief Guard function — return true to allow the transition.
 * @param ctx  User context.
 * @return true to allow, false to block.
 */
typedef bool (*SYN_FSM_Guard)(void *ctx);

/**
 * @brief Action function — called when a transition fires.
 * @param ctx  User context.
 */
typedef void (*SYN_FSM_Action)(void *ctx);

/* ── Transition descriptor ──────────────────────────────────────────────── */

/** @brief Transition descriptor — (from, event) → to, with optional guard/action. */
typedef struct {
    SYN_FSM_State   from;     /**< Source state                          */
    SYN_FSM_Event   event;    /**< Triggering event                      */
    SYN_FSM_State   to;       /**< Destination state                     */
    SYN_FSM_Guard   guard;    /**< Guard function (NULL = always allow)  */
    SYN_FSM_Action  action;   /**< Transition action (NULL = none)       */
} SYN_FSM_Transition;

/** Table terminator. Place at the end of your transition array. */
#define SYN_FSM_END  { SYN_FSM_STATE_NONE, 0, 0, NULL, NULL }

/* ── State descriptor (optional) ────────────────────────────────────────── */

/**
 * @brief Per-state entry/exit actions.
 *
 * Optional — pass NULL to syn_fsm_init() if you don't need them.
 */
/** @brief Per-state entry/exit action descriptor. */
typedef struct {
    SYN_FSM_State   state;    /**< State this applies to                 */
    SYN_FSM_Action  on_enter; /**< Called on state entry (or NULL)       */
    SYN_FSM_Action  on_exit;  /**< Called on state exit (or NULL)        */
} SYN_FSM_StateDesc;

/** State descriptor table terminator. */
#define SYN_FSM_STATE_END  { SYN_FSM_STATE_NONE, NULL, NULL }

/* ── FSM instance ───────────────────────────────────────────────────────── */

/** @brief FSM instance — transition table, state descriptors, current state. */
typedef struct {
    const SYN_FSM_Transition  *transitions;  /**< Transition table          */
    const SYN_FSM_StateDesc   *state_descs;  /**< Optional (may be NULL)  */
    const char * const         *state_names;  /**< Optional (may be NULL)  */
    const char                 *tag;           /**< Log tag                  */
    SYN_FSM_State              current;       /**< Current state            */
    void                       *ctx;           /**< User context for actions */
} SYN_FSM;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the FSM.
 *
 * @param fsm          FSM instance.
 * @param transitions  Transition table (terminated by SYN_FSM_END).
 * @param initial      Initial state.
 * @param tag          Log tag for transition logging (e.g., "motor").
 */
void syn_fsm_init(SYN_FSM *fsm,
                   const SYN_FSM_Transition *transitions,
                   SYN_FSM_State initial,
                   const char *tag);

/**
 * @brief Set optional state descriptors (entry/exit actions).
 *
 * @param fsm    FSM instance.
 * @param descs  State descriptor table (terminated by SYN_FSM_STATE_END).
 */
void syn_fsm_set_state_descs(SYN_FSM *fsm, const SYN_FSM_StateDesc *descs);

/**
 * @brief Set optional state name strings for debug/logging.
 *
 * @param fsm    FSM instance.
 * @param names  Array of state name strings, indexed by state value.
 *               Must cover all state values used in the transition table.
 */
void syn_fsm_set_state_names(SYN_FSM *fsm, const char * const *names);

/**
 * @brief Set the user context pointer passed to guards and actions.
 * @param fsm  FSM instance.
 * @param ctx  User context.
 */
void syn_fsm_set_context(SYN_FSM *fsm, void *ctx);

/**
 * @brief Dispatch an event.
 *
 * Scans the transition table for a matching (current_state, event) pair.
 * If a guard exists and returns false, the transition is skipped and the
 * next matching row is tried. On a successful match:
 *   1. Exit action for the current state (if state_descs set)
 *   2. Transition action
 *   3. State change
 *   4. Entry action for the new state (if state_descs set)
 *
 * @param fsm    FSM instance.
 * @param event  Event to dispatch.
 * @return true if a transition was taken, false if no match found.
 */
bool syn_fsm_dispatch(SYN_FSM *fsm, SYN_FSM_Event event);

/**
 * @brief Get the current state.
 * @param fsm  FSM instance.
 * @return Current state.
 */
static inline SYN_FSM_State syn_fsm_state(const SYN_FSM *fsm)
{
    return fsm->current;
}

/**
 * @brief Check if the FSM is in a specific state.
 * @param fsm  FSM instance.
 * @param st   State to check.
 * @return true if in that state.
 */
static inline bool syn_fsm_in_state(const SYN_FSM *fsm, SYN_FSM_State st)
{
    return fsm->current == st;
}

/**
 * @brief Force-set the FSM to a specific state.
 *
 * Fires exit action for the old state and entry action for the new state
 * (if state_descs are set). No transition action is called.
 *
 * @param fsm    FSM instance.
 * @param state  New state.
 */
void syn_fsm_set_state(SYN_FSM *fsm, SYN_FSM_State state);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FSM_H */
