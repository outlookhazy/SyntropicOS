/**
 * @file syn_ble_l2cap.h
 * @brief Zero-Heap BLE Logical Link Control and Adaptation Protocol (L2CAP).
 */

#ifndef SYN_BLE_L2CAP_H
#define SYN_BLE_L2CAP_H

#include "../common/syn_defs.h"
#include "syn_ble_hci.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @name Fixed L2CAP Channel Identifiers (CIDs) */
/**@{*/
#define SYN_BLE_L2CAP_CID_ATT 0x0004U       /**< Attribute Protocol (ATT) Channel */
#define SYN_BLE_L2CAP_CID_SIGNALING 0x0005U /**< LE Signaling Channel */
#define SYN_BLE_L2CAP_CID_SMP 0x0006U       /**< Security Manager Protocol (SMP) Channel */
/**@}*/

#ifndef SYN_BLE_MAX_CONNECTIONS
#define SYN_BLE_MAX_CONNECTIONS 2U /**< Maximum active BLE connection handles */
#endif

#ifndef SYN_BLE_L2CAP_BUF_SIZE
#define SYN_BLE_L2CAP_BUF_SIZE 256U /**< Maximum reassembled L2CAP PDU size */
#endif

typedef struct SYN_BLE_L2CAP_s SYN_BLE_L2CAP;

/** L2CAP PDU Received Callback */
typedef void (*SYN_BLE_L2CAP_RxCb)(SYN_BLE_L2CAP *l2cap, uint16_t conn_handle, uint16_t cid,
                                   const uint8_t *payload, uint16_t len, void *user_data);

/** Connection State Entry */
typedef struct {
    uint16_t conn_handle;                   /**< Connection handle */
    uint8_t rx_buf[SYN_BLE_L2CAP_BUF_SIZE]; /**< Reassembly buffer */
    uint16_t rx_len;                        /**< Current accumulated bytes */
    uint16_t expected_len;                  /**< Total expected PDU size */
    uint16_t cid;                           /**< Target channel ID */
    bool active;                            /**< True if slot is active */
    bool reassembling;                      /**< True if currently reassembling */
} SYN_BLE_L2CAP_Connection;

/** L2CAP Engine Context */
struct SYN_BLE_L2CAP_s {
    SYN_BLE_L2CAP_Connection conns[SYN_BLE_MAX_CONNECTIONS]; /**< Connection table */
    SYN_BLE_L2CAP_RxCb rx_cb;                                /**< PDU RX callback */
    void *user_data;                                         /**< User context pointer */
};

/**
 * @brief Initialize L2CAP engine context.
 * @param l2cap Context handle pointer.
 * @param rx_cb PDU receive callback.
 * @param user_data User context pointer.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_l2cap_init(SYN_BLE_L2CAP *l2cap, SYN_BLE_L2CAP_RxCb rx_cb, void *user_data);

/**
 * @brief Register connection handle.
 * @param l2cap Context handle pointer.
 * @param conn_handle Connection handle assigned by HCI.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_l2cap_connect(SYN_BLE_L2CAP *l2cap, uint16_t conn_handle);

/**
 * @brief Unregister connection handle.
 * @param l2cap Context handle pointer.
 * @param conn_handle Connection handle assigned by HCI.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_l2cap_disconnect(SYN_BLE_L2CAP *l2cap, uint16_t conn_handle);

/**
 * @brief Process incoming ACL packet from HCI layer.
 *
 * If @p conn_handle is not yet registered via syn_ble_l2cap_connect(), an unused
 * connection slot is allocated automatically on the first start-fragment ACL frame
 * (returning SYN_ERROR when all SYN_BLE_MAX_CONNECTIONS slots are occupied).
 *
 * @param l2cap Context handle pointer.
 * @param conn_handle Connection handle.
 * @param pb_bc_flags Packet boundary/broadcast flags.
 * @param data ACL payload.
 * @param len Payload length.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_l2cap_process_acl(SYN_BLE_L2CAP *l2cap, uint16_t conn_handle,
                                     uint8_t pb_bc_flags, const uint8_t *data, uint16_t len);

/**
 * @brief Encode L2CAP PDU for transmission over HCI ACL.
 * @param conn_handle Connection handle.
 * @param cid Channel ID (e.g. SYN_BLE_L2CAP_CID_ATT).
 * @param payload PDU payload buffer.
 * @param len Payload length.
 * @param tx_buf Output buffer for HCI ACL frame.
 * @param tx_len Output size pointer.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_l2cap_encode_pdu(uint16_t conn_handle, uint16_t cid, const uint8_t *payload,
                                    uint16_t len, uint8_t *tx_buf, uint16_t *tx_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BLE_L2CAP_H */
