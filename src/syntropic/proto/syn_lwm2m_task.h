/**
 * @file syn_lwm2m_task.h
 * @brief Autonomous OMA LwM2M Client Supervisor Task & Engine.
 * @ingroup syn_proto
 *
 * Implements an event-driven cooperative client task for OMA LwM2M v1.1/v1.2:
 * - Autonomous Server Registration, 80% Lifetime Renewal, and Deregistration.
 * - Transport management over UDP or DTLS 1.3 (via SYN_CoapsClient).
 * - Downlink CoAP Request Dispatch (Read, Write, Execute, Discover).
 * - RFC 7641 Observe & Notify scheduler matching periodic pmax & value triggers.
 * - Standard Device Object (Obj 3) reboot/reset and Firmware Update (Obj 5) dispatch.
 */

#ifndef SYN_LWM2M_TASK_H
#define SYN_LWM2M_TASK_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/proto/syn_lwm2m.h"

#if (!defined(SYN_USE_LWM2M_TASK) || SYN_USE_LWM2M_TASK) && SYN_LWM2M_ENABLED && \
    (!defined(SYN_USE_PT) || SYN_USE_PT) && (!defined(SYN_USE_SCHED) || SYN_USE_SCHED)
/** @brief Non-zero when LwM2M autonomous task and scheduler dependencies are enabled. */
#define SYN_LWM2M_TASK_ENABLED 1
#else
/** @brief Non-zero when LwM2M autonomous task and scheduler dependencies are enabled. */
#define SYN_LWM2M_TASK_ENABLED 0
#endif

#if SYN_LWM2M_TASK_ENABLED

#include "syntropic/net/syn_coap.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Forward Declarations & Callbacks ────────────────────────────────────── */

struct SYN_LwM2M_Task;

/**
 * @brief Callback for Device Object 3 Reboot command (Res 4).
 * @param user_data User-defined context pointer.
 */
typedef void (*SYN_LwM2M_RebootCb)(void *user_data);

/**
 * @brief Callback for Device Object 3 Factory Reset command (Res 5).
 * @param user_data User-defined context pointer.
 */
typedef void (*SYN_LwM2M_FactoryResetCb)(void *user_data);

/**
 * @brief Callback for Firmware Update Object 5 Update command (Res 2).
 * @param package_uri Download URI / package identifier.
 * @param user_data   User-defined context pointer.
 */
typedef void (*SYN_LwM2M_FwUpdateCb)(const char *package_uri, void *user_data);

/**
 * @brief Callback on client registration state transition.
 * @param old_state Previous registration state.
 * @param new_state New registration state.
 * @param user_data User-defined context pointer.
 */
typedef void (*SYN_LwM2M_StateChangeCb)(SYN_LwM2M_ClientState old_state,
                                        SYN_LwM2M_ClientState new_state, void *user_data);

/* ── Configuration Structure ─────────────────────────────────────────────── */

/**
 * @brief Configuration parameters for LwM2M Client Supervisor Task.
 */
typedef struct {
    SYN_LwM2M_Client *client;          /**< Initialized LwM2M client instance */
    SYN_Transport *transport;          /**< Underlying datagram transport (UDP/custom) */
    SYN_CoapsClient *coaps_client;     /**< Optional DTLS 1.3 client context (or NULL) */
    uint32_t retry_backoff_ms;         /**< Retry backoff period on network failure (ms) */
    SYN_LwM2M_RebootCb on_reboot;      /**< Device reboot handler */
    SYN_LwM2M_FactoryResetCb on_reset; /**< Factory reset handler */
    SYN_LwM2M_FwUpdateCb on_fw_update; /**< Firmware update handler */
    SYN_LwM2M_StateChangeCb on_state;  /**< State change notification callback */
    void *user_data;                   /**< User data passed to callbacks */

    /* Scratch frame buffers */
    uint8_t *rx_buf;    /**< Caller-allocated RX frame scratch buffer */
    size_t rx_buf_size; /**< Size of RX frame buffer (recommended >= 512) */
    uint8_t *tx_buf;    /**< Caller-allocated TX frame scratch buffer */
    size_t tx_buf_size; /**< Size of TX frame buffer (recommended >= 512) */
} SYN_LwM2M_TaskConfig;

/* ── Task Context ────────────────────────────────────────────────────────── */

/** @brief Maximum queued dirty resource notifications per supervisor tick. */
#define SYN_LWM2M_MAX_DIRTY_QUEUE 8U

/**
 * @brief Queued dirty resource or object-instance entry.
 */
typedef struct {
    uint16_t obj_id;  /**< Changed Object ID */
    uint16_t inst_id; /**< Changed Instance ID */
    int32_t res_id;   /**< Changed Resource ID (-1 for entire instance) */
} SYN_LwM2M_DirtyEntry;

/**
 * @brief LwM2M Supervisor Task Context.
 */
typedef struct SYN_LwM2M_Task {
    SYN_LwM2M_TaskConfig cfg; /**< Task configuration */
    SYN_PT pt;                /**< Protothread state */

    uint16_t next_msg_id;    /**< Message ID sequence counter */
    uint16_t pending_msg_id; /**< Message ID of in-flight Register or Update request */
    uint32_t last_action_ms; /**< Timestamp of last transmitted request (ms) */
    uint32_t retry_count;    /**< Number of failed retry attempts */
    bool has_acted;          /**< True after initial action timestamp is recorded */
    bool force_update;       /**< Flag to trigger immediate Registration Update */
    bool force_deregister;   /**< Flag to request clean Deregistration */
    bool pending_notify[SYN_LWM2M_MAX_OBSERVERS]; /**< Per-observer latched dirty state */
    SYN_LwM2M_DirtyEntry dirty_queue[SYN_LWM2M_MAX_DIRTY_QUEUE]; /**< Pending dirty queue */
    size_t dirty_count; /**< Number of queued dirty entries */
} SYN_LwM2M_Task;

/* ── Public API ─────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the LwM2M Supervisor Task context.
 *
 * @param task Task instance to initialize.
 * @param cfg  Task configuration parameters.
 * @return SYN_OK on success, SYN_INVALID_PARAM on NULL or invalid arguments.
 */
SYN_Status syn_lwm2m_task_init(SYN_LwM2M_Task *task, const SYN_LwM2M_TaskConfig *cfg);

/**
 * @brief Advance the LwM2M Supervisor state machine by one step / tick.
 *
 * Handles DTLS handshake progress, registration renewal timers, receiving
 * incoming CoAP requests, dispatching responses, evaluating observation rules,
 * and firing notifications.
 *
 * @param task   Task instance.
 * @param now_ms Current system timestamp in milliseconds.
 * @return SYN_OK on normal operation, error code on fatal condition.
 */
SYN_Status syn_lwm2m_task_step(SYN_LwM2M_Task *task, uint32_t now_ms);

/**
 * @brief Protothread task entry function for scheduler integration (`syn_sched`).
 *
 * @param pt   Protothread continuation structure.
 * @param task Generic scheduler task descriptor containing SYN_LwM2M_Task in user_data.
 * @return SYN_PT_Status continuation status.
 */
SYN_PT_Status syn_lwm2m_task_pt(SYN_PT *pt, SYN_Task *task);

/**
 * @brief Trigger an immediate LwM2M Registration Update.
 *
 * @param task Task instance.
 * @return SYN_OK on success, SYN_INVALID_PARAM if task is NULL.
 */
SYN_Status syn_lwm2m_task_trigger_update(SYN_LwM2M_Task *task);

/**
 * @brief Request a graceful LwM2M Deregistration.
 *
 * @param task Task instance.
 * @return SYN_OK on success, SYN_INVALID_PARAM if task is NULL.
 */
SYN_Status syn_lwm2m_task_deregister(SYN_LwM2M_Task *task);

/**
 * @brief Mark an object instance or resource as modified to evaluate observe notifications.
 *
 * @param task    Task instance.
 * @param obj_id  Object ID that changed.
 * @param inst_id Instance ID that changed.
 * @param res_id  Resource ID that changed (-1 for entire instance).
 * @return SYN_OK on success, SYN_INVALID_PARAM if task is NULL.
 */
SYN_Status syn_lwm2m_task_notify_changed(SYN_LwM2M_Task *task, uint16_t obj_id, uint16_t inst_id,
                                         int32_t res_id);

#ifdef __cplusplus
}
#endif

#endif /* SYN_LWM2M_TASK_ENABLED */

#endif /* SYN_LWM2M_TASK_H */
