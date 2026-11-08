/**
 * @file syn_ble_hci.h
 * @brief Zero-Heap Host Controller Interface (HCI) Driver (H:4 UART Framing).
 * @ingroup syn_ble
 *
 * Implements Bluetooth Core Specification Vol 4, Part A (H:4 Transport Layer).
 * Encapsulates HCI Commands, HCI Events, and ACL Data packets over UART/SPI.
 */

#ifndef SYN_BLE_HCI_H
#define SYN_BLE_HCI_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @name HCI Packet Indicator Types (H:4 Protocol) */
/**@{*/
#define SYN_BLE_HCI_PKT_CMD 0x01U /**< HCI Command Packet */
#define SYN_BLE_HCI_PKT_ACL 0x02U /**< HCI ACL Data Packet */
#define SYN_BLE_HCI_PKT_SCO 0x03U /**< HCI Synchronous Data Packet */
#define SYN_BLE_HCI_PKT_EVT 0x04U /**< HCI Event Packet */
#define SYN_BLE_HCI_PKT_ISO 0x05U /**< HCI ISO Data Packet */
/**@}*/

/** @name Common HCI OGF Codes */
/**@{*/
#define SYN_BLE_HCI_OGF_LINK_CONTROL 0x01U  /**< Link Control OGF */
#define SYN_BLE_HCI_OGF_INFO_PARAM 0x04U    /**< Informational Parameters OGF */
#define SYN_BLE_HCI_OGF_STATUS_PARAM 0x05U  /**< Status Parameters OGF */
#define SYN_BLE_HCI_OGF_LE_CONTROLLER 0x08U /**< LE Controller Commands OGF */
/**@}*/

/** Construct 16-bit HCI Opcode from OGF and OCF */
#define SYN_BLE_HCI_OPCODE(ogf, ocf) ((uint16_t)(((ogf) << 10) | ((ocf) & 0x03FFU)))

/** @name Common LE HCI Opcodes */
/**@{*/
#define SYN_BLE_HCI_OP_LE_SET_EVENT_MASK \
    SYN_BLE_HCI_OPCODE(0x08U, 0x0001U) /**< LE Set Event Mask Opcode */
#define SYN_BLE_HCI_OP_LE_READ_BUFFER_SIZE \
    SYN_BLE_HCI_OPCODE(0x08U, 0x0002U) /**< LE Read Buffer Size Opcode */
#define SYN_BLE_HCI_OP_LE_SET_RANDOM_ADDR \
    SYN_BLE_HCI_OPCODE(0x08U, 0x0005U) /**< LE Set Random Address Opcode */
#define SYN_BLE_HCI_OP_LE_SET_ADV_PARAM \
    SYN_BLE_HCI_OPCODE(0x08U, 0x0006U) /**< LE Set Advertising Parameters Opcode */
#define SYN_BLE_HCI_OP_LE_SET_ADV_DATA \
    SYN_BLE_HCI_OPCODE(0x08U, 0x0008U) /**< LE Set Advertising Data Opcode */
#define SYN_BLE_HCI_OP_LE_SET_SCAN_RESP_DATA \
    SYN_BLE_HCI_OPCODE(0x08U, 0x0009U) /**< LE Set Scan Response Data Opcode */
#define SYN_BLE_HCI_OP_LE_SET_ADV_ENABLE \
    SYN_BLE_HCI_OPCODE(0x08U, 0x000AU) /**< LE Set Advertising Enable Opcode */
#define SYN_BLE_HCI_OP_LE_SET_SCAN_PARAM \
    SYN_BLE_HCI_OPCODE(0x08U, 0x000BU) /**< LE Set Scan Parameters Opcode */
#define SYN_BLE_HCI_OP_LE_SET_SCAN_ENABLE \
    SYN_BLE_HCI_OPCODE(0x08U, 0x000CU) /**< LE Set Scan Enable Opcode */
#define SYN_BLE_HCI_OP_LE_CREATE_CONN \
    SYN_BLE_HCI_OPCODE(0x08U, 0x000DU) /**< LE Create Connection Opcode */
#define SYN_BLE_HCI_OP_LE_CREATE_CONN_CANCEL \
    SYN_BLE_HCI_OPCODE(0x08U, 0x000EU) /**< LE Create Connection Cancel Opcode */
#define SYN_BLE_HCI_OP_DISCONNECT SYN_BLE_HCI_OPCODE(0x01U, 0x0006U) /**< Disconnect Opcode */
/**@}*/

/** @name Common HCI Event Codes */
/**@{*/
#define SYN_BLE_HCI_EVT_DISCONN_COMPLETE 0x05U /**< Disconnection Complete Event */
#define SYN_BLE_HCI_EVT_CMD_COMPLETE 0x0EU     /**< Command Complete Event */
#define SYN_BLE_HCI_EVT_CMD_STATUS 0x0FU       /**< Command Status Event */
#define SYN_BLE_HCI_EVT_LE_META 0x3EU          /**< LE Meta Event */
/**@}*/

/** @name Common LE Meta Event Subcodes */
/**@{*/
#define SYN_BLE_HCI_LE_SUBEVT_CONN_COMPLETE 0x01U /**< LE Connection Complete Subevent */
#define SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT 0x02U    /**< LE Advertising Report Subevent */
#define SYN_BLE_HCI_LE_SUBEVT_CONN_UPDATE_COMPLETE \
    0x03U /**< LE Connection Update Complete Subevent */
/**@}*/

#ifndef SYN_BLE_HCI_RX_BUF_SIZE
#define SYN_BLE_HCI_RX_BUF_SIZE 260U /**< HCI RX packet capacity (4B ACL + 256B L2CAP) */
#endif

#ifndef SYN_BLE_HCI_TX_BUF_SIZE
#define SYN_BLE_HCI_TX_BUF_SIZE 261U /**< HCI TX packet capacity (1B H:4 + 4B ACL + 256B L2CAP) */
#endif

typedef struct SYN_BLE_HCI_s SYN_BLE_HCI;

/** Callback function pointer for received HCI Events */
typedef void (*SYN_BLE_HCI_EventCb)(SYN_BLE_HCI *hci, uint8_t evt_code, const uint8_t *payload,
                                    uint8_t len, void *user_data);

/** Callback function pointer for received ACL Data */
typedef void (*SYN_BLE_HCI_AclCb)(SYN_BLE_HCI *hci, uint16_t conn_handle, uint8_t pb_bc_flags,
                                  const uint8_t *data, uint16_t len, void *user_data);

/** HCI Driver Configuration */
typedef struct {
    SYN_BLE_HCI_EventCb evt_cb; /**< Event callback */
    SYN_BLE_HCI_AclCb acl_cb;   /**< ACL Data callback */
    void *user_data;            /**< User context pointer */
} SYN_BLE_HCI_Config;

/** HCI Driver Context State */
struct SYN_BLE_HCI_s {
    SYN_BLE_HCI_Config cfg;                  /**< Driver configuration */
    uint8_t rx_buf[SYN_BLE_HCI_RX_BUF_SIZE]; /**< RX framing buffer */
    uint16_t rx_idx;                         /**< Accumulated byte count */
    uint16_t rx_target_len;                  /**< Expected frame target size */
    uint8_t rx_state;                        /**< Current state machine phase */
    uint8_t rx_pkt_type;                     /**< Current packet type (EVT/ACL) */
    uint16_t last_cmd_opcode;                /**< Last completed command opcode */
    uint8_t last_cmd_status;                 /**< Last completed command status */
    bool cmd_pending;                        /**< True if awaiting command response */
};

/**
 * @brief Initialize HCI Driver Context.
 * @param hci Driver handle pointer.
 * @param cfg Driver configuration struct.
 * @return SYN_OK on success, SYN_INVALID_PARAM if null.
 */
SYN_Status syn_ble_hci_init(SYN_BLE_HCI *hci, const SYN_BLE_HCI_Config *cfg);

/**
 * @brief Send HCI Command Packet.
 * @param opcode 16-bit HCI Command Opcode.
 * @param params Pointer to parameter payload (or NULL if len is 0).
 * @param param_len Parameter payload length in bytes.
 * @param tx_buf Buffer to store framed HCI packet (minimum 4 + param_len bytes).
 * @param tx_len Pointer to store resulting framed size.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_hci_encode_command(uint16_t opcode, const uint8_t *params, uint8_t param_len,
                                      uint8_t *tx_buf, uint16_t *tx_len);

/**
 * @brief Encode HCI ACL Data Packet.
 * @param conn_handle 12-bit Connection Handle.
 * @param pb_flags Packet Boundary flags (2 bits).
 * @param bc_flags Broadcast flags (2 bits).
 * @param data Data payload bytes.
 * @param len Data payload length in bytes.
 * @param tx_buf Buffer to store framed ACL packet (minimum 5 + len bytes).
 * @param tx_len Pointer to store resulting framed size.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_hci_encode_acl(uint16_t conn_handle, uint8_t pb_flags, uint8_t bc_flags,
                                  const uint8_t *data, uint16_t len, uint8_t *tx_buf,
                                  uint16_t *tx_len);

/**
 * @brief Feed raw bytes from UART/SPI into HCI receiver state machine.
 * @param hci Driver handle pointer.
 * @param byte Single received byte from H:4 transport.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_hci_rx_byte(SYN_BLE_HCI *hci, uint8_t byte);

/**
 * @brief Feed raw buffer into HCI receiver state machine.
 * @param hci Driver handle pointer.
 * @param buf Data buffer pointer.
 * @param len Data buffer length.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_hci_rx_buf(SYN_BLE_HCI *hci, const uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BLE_HCI_H */
