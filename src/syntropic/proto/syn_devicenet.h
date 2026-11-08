/**
 * @file syn_devicenet.h
 * @brief ODVA DeviceNet (Common Industrial Protocol over CAN) Protocol Stack.
 *
 * Provides a zero-heap, lightweight DeviceNet slave node protocol stack including
 * Group 2 Predefined Master/Slave Connection Set, Duplicate MAC ID Check state
 * machine, CIP Object Model (Identity 0x01, DeviceNet 0x03, Assembly 0x04),
 * Polled I/O messaging, Explicit messaging, and QuickConnect™ hot-swapping.
 */

#ifndef SYN_DEVICENET_H
#define SYN_DEVICENET_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @name DeviceNet Configuration Constants */
/**@{*/
#define SYN_DEVICENET_MAX_MAC_ID 63U           /**< Maximum valid MAC ID */
#define SYN_DEVICENET_MAX_PAYLOAD 8U           /**< Maximum CAN frame payload size */
#define SYN_DEVICENET_DUP_MAC_TIMEOUT_MS 1000U /**< Duplicate MAC check timeout */
/**@}*/

/** @brief DeviceNet Baud Rates */
typedef enum {
    SYN_DEVICENET_BAUD_125K = 0U, /**< 125 kbps */
    SYN_DEVICENET_BAUD_250K = 1U, /**< 250 kbps */
    SYN_DEVICENET_BAUD_500K = 2U  /**< 500 kbps */
} SYN_DeviceNet_Baud;

/** @brief DeviceNet Node States */
typedef enum {
    SYN_DEVICENET_STATE_OFFLINE = 0U,       /**< Node offline */
    SYN_DEVICENET_STATE_DUP_MAC_CHECK = 1U, /**< Checking for duplicate MAC ID */
    SYN_DEVICENET_STATE_ONLINE = 2U,        /**< Node online */
    SYN_DEVICENET_STATE_BUS_OFF = 3U        /**< Node in bus-off error state */
} SYN_DeviceNet_State;

/** @brief CIP Identity Object (Class 0x01) Attributes */
typedef struct {
    uint16_t vendor_id;         /**< Vendor ID code */
    uint16_t device_type;       /**< Device type profile */
    uint16_t product_code;      /**< Product code */
    uint8_t rev_major;          /**< Major revision number */
    uint8_t rev_minor;          /**< Minor revision number */
    uint16_t status;            /**< Device status word */
    uint32_t serial_number;     /**< 32-bit device serial number */
    char product_name[32];      /**< ASCII product name string */
    bool quick_connect_enabled; /**< QuickConnect feature enabled flag */
} SYN_CIP_Identity;

/** @brief CIP DeviceNet Object (Class 0x03) Attributes */
typedef struct {
    uint8_t mac_id;               /**< Node MAC ID address */
    SYN_DeviceNet_Baud baud_rate; /**< Configured CAN baud rate */
    uint8_t bus_off_action;       /**< Bus-off reset action */
    uint8_t allocation_choice;    /**< Allocation choice byte */
    uint8_t master_mac_id;        /**< Master node MAC ID */
} SYN_CIP_DeviceNet;

/** @brief CIP Assembly Object (Class 0x04) Context */
typedef struct {
    uint8_t *input_buf;  /**< Target input assembly data buffer */
    uint8_t input_len;   /**< Input assembly buffer length */
    uint8_t *output_buf; /**< Target output assembly data buffer */
    uint8_t output_len;  /**< Output assembly buffer length */
} SYN_CIP_Assembly;

/** @brief DeviceNet Node Context Structure */
typedef struct {
    SYN_DeviceNet_State state;       /**< Current node state */
    SYN_CIP_Identity identity;       /**< CIP Identity object attributes */
    SYN_CIP_DeviceNet devicenet_obj; /**< CIP DeviceNet object attributes */
    SYN_CIP_Assembly assembly;       /**< CIP Assembly object context */
    uint32_t dup_mac_timer_ms;       /**< Duplicate MAC check timer in ms */
    bool dup_mac_failed;             /**< Duplicate MAC check failure flag */
    bool explicit_connected;         /**< Explicit connection active flag */
    bool polled_connected;           /**< Polled I/O connection active flag */
} SYN_DeviceNet_Node;

/**
 * @brief Initialize DeviceNet node instance context.
 *
 * @param node Pointer to DeviceNet node context.
 * @param mac_id Initial Node Address (0..63).
 * @param baud Network baud rate setting.
 * @return true on success, false if parameters invalid.
 */
bool syn_devicenet_init(SYN_DeviceNet_Node *node, uint8_t mac_id, SYN_DeviceNet_Baud baud);

/**
 * @brief Set CIP Identity serial number for DeviceNet node.
 * @param node Pointer to DeviceNet node context.
 * @param serial_number 32-bit serial number.
 */
void syn_devicenet_set_serial_number(SYN_DeviceNet_Node *node, uint32_t serial_number);

/**
 * @brief Configure Assembly Object (Class 0x04) input/output I/O buffers.
 *
 * @param node Pointer to DeviceNet node instance.
 * @param in_buf Pointer to input data buffer (sensor/status data to Master).
 * @param in_len Length of input buffer (max 8 bytes).
 * @param out_buf Pointer to output data buffer (actuator/cmd data from Master).
 * @param out_len Length of output buffer (max 8 bytes).
 * @return true on success, false if parameters invalid.
 */
bool syn_devicenet_set_assembly(SYN_DeviceNet_Node *node, uint8_t *in_buf, uint8_t in_len,
                                uint8_t *out_buf, uint8_t out_len);

/**
 * @brief Enable or disable QuickConnect™ fast startup mode.
 *
 * @param node Pointer to DeviceNet node instance.
 * @param enable True to enable QuickConnect feature.
 * @return true on success, false if node is NULL.
 */
bool syn_devicenet_set_quickconnect(SYN_DeviceNet_Node *node, bool enable);

/**
 * @brief Process incoming CAN message frame and generate optional response frame.
 *
 * @param node Pointer to DeviceNet node instance.
 * @param can_id Received 11-bit CAN Identifier.
 * @param data Received CAN payload bytes.
 * @param len Received CAN payload length (0..8).
 * @param tx_can_id Output pointer for response CAN ID.
 * @param tx_data Output buffer for response payload bytes (min 8 bytes capacity).
 * @param tx_len Output pointer for response payload length.
 * @return true if response CAN frame generated, false if no response required.
 */
bool syn_devicenet_on_can_rx(SYN_DeviceNet_Node *node, uint32_t can_id, const uint8_t *data,
                             uint8_t len, uint32_t *tx_can_id, uint8_t *tx_data, uint8_t *tx_len);

/**
 * @brief Periodic timer tick to service Duplicate MAC ID check & QuickConnect timers.
 *
 * @param node Pointer to DeviceNet node instance.
 * @param dt_ms Time elapsed in milliseconds since last poll call.
 */
void syn_devicenet_poll(SYN_DeviceNet_Node *node, uint32_t dt_ms);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DEVICENET_H */
