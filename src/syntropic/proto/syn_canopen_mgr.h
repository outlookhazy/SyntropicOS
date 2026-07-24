/**
 * @file syn_canopen_mgr.h
 * @brief CANopen CiA 302 Network Manager, SDO Client & NMT Master.
 *
 * Provides a lightweight, non-blocking CANopen master engine capable of
 * transmitting NMT node control commands, executing expedited/segmented SDO
 * reads & writes to remote slave nodes, and monitoring network heartbeats.
 */

#ifndef SYN_CANOPEN_MGR_H
#define SYN_CANOPEN_MGR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"
#include "syn_canopen.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief SDO Client Transfer States */
typedef enum {
    SYN_SDO_CLIENT_STATE_IDLE = 0,
    SYN_SDO_CLIENT_STATE_WAIT_READ,
    SYN_SDO_CLIENT_STATE_WAIT_WRITE,
    SYN_SDO_CLIENT_STATE_SUCCESS,
    SYN_SDO_CLIENT_STATE_ERROR
} SYN_SDOClientState;

/** @brief SDO Client Transaction Handle */
typedef struct {
    uint8_t            target_node;   /**< Target slave Node-ID (1..127) */
    uint16_t           index;         /**< Target OD Index */
    uint8_t            subindex;      /**< Target OD Subindex */
    SYN_SDOClientState state;         /**< Current SDO state */
    uint32_t           abort_code;    /**< SDO abort code if error occurred */
    uint8_t            data[8];       /**< Read/Write data buffer */
    size_t             data_len;      /**< Length of payload data */
    uint16_t           timeout_ms;    /**< Transaction timeout timer */
} SYN_CANOpenSDOClient;

/** @brief Remote Node Heartbeat Monitor Entry */
typedef struct {
    uint8_t  node_id;       /**< Remote node-ID */
    uint8_t  nmt_state;     /**< Last known NMT state */
    uint16_t timer_ms;      /**< Elapsed time since last heartbeat */
    bool     online;        /**< True if node is active and within timeout */
} SYN_CANOpenNodeMonitor;

/** @brief CANopen Manager Master State Handle */
typedef struct {
    SYN_CANOpenSDOClient  sdo_client;     /**< SDO client instance */
    SYN_CANOpenNodeMonitor nodes[128];    /**< Node tracking table (1..127) */
} SYN_CANOpenManager;

/**
 * @brief Initialize CANopen Manager master instance.
 * @param mgr Pointer to manager handle.
 */
void syn_canopen_mgr_init(SYN_CANOpenManager *mgr);

/**
 * @brief Build an NMT Master command frame.
 * @param frame Output CAN frame buffer.
 * @param target_node Target node ID (1..127 or 0 for broadcast).
 * @param nmt_cmd Command (SYN_CANOPEN_NMT_CMD_*).
 */
void syn_canopen_mgr_build_nmt(SYN_CAN_Frame *frame, uint8_t target_node, uint8_t nmt_cmd);

/**
 * @brief Initiate an SDO Read request to a remote slave node.
 * @param mgr Pointer to manager handle.
 * @param frame Output CAN request frame buffer.
 * @param node_id Target slave Node-ID.
 * @param index Target Object Dictionary Index.
 * @param subindex Target Object Dictionary Subindex.
 * @return SYN_OK if initiated, SYN_BUSY if SDO client is busy.
 */
SYN_Status syn_canopen_mgr_sdo_read_init(SYN_CANOpenManager *mgr, SYN_CAN_Frame *frame,
                                          uint8_t node_id, uint16_t index, uint8_t subindex);

/**
 * @brief Initiate an SDO Write request to a remote slave node.
 * @param mgr Pointer to manager handle.
 * @param frame Output CAN request frame buffer.
 * @param node_id Target slave Node-ID.
 * @param index Target Object Dictionary Index.
 * @param subindex Target Object Dictionary Subindex.
 * @param data Pointer to payload bytes to write.
 * @param len Data length (1..4 bytes for expedited write).
 * @return SYN_OK if initiated, SYN_BUSY if busy.
 */
SYN_Status syn_canopen_mgr_sdo_write_init(SYN_CANOpenManager *mgr, SYN_CAN_Frame *frame,
                                           uint8_t node_id, uint16_t index, uint8_t subindex,
                                           const void *data, size_t len);

/**
 * @brief Process an incoming CAN frame for Manager processing (Heartbeats, SDO responses).
 * @param mgr Pointer to manager handle.
 * @param frame Incoming CAN frame.
 */
void syn_canopen_mgr_process_frame(SYN_CANOpenManager *mgr, const SYN_CAN_Frame *frame);

/**
 * @brief Advance CANopen Manager periodic timer ticks.
 * @param mgr Pointer to manager handle.
 * @param delta_ms Milliseconds elapsed since last call.
 */
void syn_canopen_mgr_step(SYN_CANOpenManager *mgr, uint16_t delta_ms);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CANOPEN_MGR_H */
