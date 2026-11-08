/**
 * @file syn_cannm.h
 * @brief AUTOSAR CAN Network Management (CanNm) Protocol.
 * @ingroup syn_protocol
 *
 * Implements AUTOSAR CAN Network Management non-blocking state machine for ECU
 * sleep/wake coordination over CAN bus.
 */

#ifndef SYN_CANNM_H
#define SYN_CANNM_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_CANNM) || SYN_USE_CANNM

#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup cannm_cbv AUTOSAR CAN NM Control Bit Vector (CBV) Flags
 *  @{ */
#define SYN_CANNM_CBV_REPEAT_MSG_REQ 0x01U    /**< Repeat Message Request */
#define SYN_CANNM_CBV_PNI_REQ 0x40U           /**< Partial Network Information */
#define SYN_CANNM_CBV_ACTIVE_WAKEUP_REQ 0x10U /**< Active Wakeup Bit */
/** @} */

/**
 * @brief AUTOSAR CAN Network Management States.
 */
typedef enum {
    SYN_CANNM_STATE_BUS_SLEEP = 0, /**< Low power bus sleep state */
    SYN_CANNM_STATE_PRE_BUS_SLEEP, /**< Prepare bus sleep state */
    SYN_CANNM_STATE_REPEAT_MSG,    /**< Repeat message state */
    SYN_CANNM_STATE_NORMAL_OP,     /**< Normal operation state */
    SYN_CANNM_STATE_READY_SLEEP    /**< Ready sleep state */
} SYN_CanNM_State;

/**
 * @brief AUTOSAR CAN Network Management Configuration.
 */
typedef struct {
    uint8_t node_id;             /**< Local ECU Node ID (e.g. 0x01) */
    uint32_t can_id_base;        /**< CAN NM ID Base (e.g. 0x400) */
    uint32_t can_id_mask;        /**< CAN NM ID Filter Mask (e.g. 0x7F0) */
    uint32_t msg_cycle_ms;       /**< Transmission period (default: 100ms) */
    uint32_t nm_timeout_ms;      /**< NM Timeout (default: 1000ms) */
    uint32_t wait_bus_sleep_ms;  /**< Wait Bus Sleep Timeout (default: 1500ms) */
    uint32_t repeat_msg_time_ms; /**< Repeat Message State Duration (default: 1600ms) */
} SYN_CanNM_Config;

/**
 * @brief AUTOSAR CAN Network Management Session Context.
 */
typedef struct {
    SYN_CanNM_Config config;    /**< Session timer & ID configuration */
    SYN_CanNM_State state;      /**< Current CAN NM FSM state */
    SYN_CanNM_State prev_state; /**< Previous CAN NM FSM state */

    bool node_comm_req;  /**< Local application network request */
    bool repeat_msg_req; /**< Repeat message request flag */
    bool active_wakeup;  /**< Active wakeup bit latched until BUS_SLEEP */

    uint32_t msg_cycle_timer;      /**< Message transmission period timer in ms */
    uint32_t timeout_timer;        /**< NM Timeout timer in ms */
    uint32_t wait_bus_sleep_timer; /**< Wait Bus Sleep timer in ms */
    uint32_t repeat_msg_timer;     /**< Repeat Message state duration timer in ms */

    uint8_t user_data[6];      /**< Local user data bytes */
    uint8_t rx_user_data[6];   /**< User data from last received NM message */
    uint8_t rx_source_node_id; /**< Node ID of last received NM message */
    uint8_t rx_cbv;            /**< CBV of last received NM message */
} SYN_CanNM_Session;

/**
 * @brief Initialize CAN NM Session.
 * @param session Session handle pointer
 * @param cfg Configuration structure (or NULL for defaults)
 */
void syn_cannm_init(SYN_CanNM_Session *session, const SYN_CanNM_Config *cfg);

/**
 * @brief Request network active operation (keep awake / active wakeup).
 * @param session Session handle pointer
 */
void syn_cannm_request_network(SYN_CanNM_Session *session);

/**
 * @brief Release network request (allow transition to sleep).
 * @param session Session handle pointer
 */
void syn_cannm_release_network(SYN_CanNM_Session *session);

/**
 * @brief Request Repeat Message state transition.
 * @param session Session handle pointer
 */
void syn_cannm_request_repeat_msg(SYN_CanNM_Session *session);

/**
 * @brief Set local NM user payload bytes (6 bytes max).
 * @param session Session handle pointer
 * @param data Data pointer
 * @param len Data length (0 to 6 bytes)
 */
void syn_cannm_set_user_data(SYN_CanNM_Session *session, const uint8_t *data, size_t len);

/**
 * @brief Process an incoming CAN frame for CAN NM filtering and state machine.
 * @param session Session handle pointer
 * @param frame Incoming CAN frame pointer
 * @return true if frame was a valid matching CAN NM PDU
 */
bool syn_cannm_process_rx_frame(SYN_CanNM_Session *session, const SYN_CAN_Frame *frame);

/**
 * @brief Step CAN NM state machine and timer tick.
 * @param session Session handle pointer
 * @param delta_ms Elapsed time in milliseconds
 * @param tx_frame Pointer to CAN frame structure to receive pending CAN NM message
 * @return true if a CAN frame is ready in tx_frame to be transmitted
 */
bool syn_cannm_step(SYN_CanNM_Session *session, uint32_t delta_ms, SYN_CAN_Frame *tx_frame);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_CANNM */
#endif /* SYN_CANNM_H */
