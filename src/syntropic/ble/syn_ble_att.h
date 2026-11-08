/**
 * @file syn_ble_att.h
 * @brief Zero-Heap Attribute Protocol (ATT) Encoding and Decoding.
 */

#ifndef SYN_BLE_ATT_H
#define SYN_BLE_ATT_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @name ATT Opcodes per Bluetooth Core Spec Vol 3, Part F */
/**@{*/
#define SYN_BLE_ATT_OP_ERROR_RSP 0x01U              /**< Error Response */
#define SYN_BLE_ATT_OP_EXCHANGE_MTU_REQ 0x02U       /**< Exchange MTU Request */
#define SYN_BLE_ATT_OP_EXCHANGE_MTU_RSP 0x03U       /**< Exchange MTU Response */
#define SYN_BLE_ATT_OP_FIND_INFO_REQ 0x04U          /**< Find Information Request */
#define SYN_BLE_ATT_OP_FIND_INFO_RSP 0x05U          /**< Find Information Response */
#define SYN_BLE_ATT_OP_FIND_BY_TYPE_VALUE_REQ 0x06U /**< Find By Type Value Request */
#define SYN_BLE_ATT_OP_FIND_BY_TYPE_VALUE_RSP 0x07U /**< Find By Type Value Response */
#define SYN_BLE_ATT_OP_READ_BY_TYPE_REQ 0x08U       /**< Read By Type Request */
#define SYN_BLE_ATT_OP_READ_BY_TYPE_RSP 0x09U       /**< Read By Type Response */
#define SYN_BLE_ATT_OP_READ_REQ 0x0AU               /**< Read Request */
#define SYN_BLE_ATT_OP_READ_RSP 0x0BU               /**< Read Response */
#define SYN_BLE_ATT_OP_READ_BLOB_REQ 0x0CU          /**< Read Blob Request */
#define SYN_BLE_ATT_OP_READ_BLOB_RSP 0x0DU          /**< Read Blob Response */
#define SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_REQ 0x10U /**< Read By Group Type Request */
#define SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_RSP 0x11U /**< Read By Group Type Response */
#define SYN_BLE_ATT_OP_WRITE_REQ 0x12U              /**< Write Request */
#define SYN_BLE_ATT_OP_WRITE_RSP 0x13U              /**< Write Response */
#define SYN_BLE_ATT_OP_WRITE_CMD 0x52U              /**< Write Command */
#define SYN_BLE_ATT_OP_HANDLE_VAL_NOTIF 0x1BU       /**< Handle Value Notification */
#define SYN_BLE_ATT_OP_HANDLE_VAL_IND 0x1DU         /**< Handle Value Indication */
#define SYN_BLE_ATT_OP_HANDLE_VAL_CONF 0x1EU        /**< Handle Value Confirmation */
/**@}*/

/** @name ATT Error Codes */
/**@{*/
#define SYN_BLE_ATT_ERR_INVALID_HANDLE 0x01U         /**< Invalid Handle */
#define SYN_BLE_ATT_ERR_READ_NOT_PERMITTED 0x02U     /**< Read Not Permitted */
#define SYN_BLE_ATT_ERR_WRITE_NOT_PERMITTED 0x03U    /**< Write Not Permitted */
#define SYN_BLE_ATT_ERR_INVALID_PDU 0x04U            /**< Invalid PDU */
#define SYN_BLE_ATT_ERR_INSUFFICIENT_AUTH 0x05U      /**< Insufficient Authentication */
#define SYN_BLE_ATT_ERR_REQ_NOT_SUPPORTED 0x06U      /**< Request Not Supported */
#define SYN_BLE_ATT_ERR_INVALID_OFFSET 0x07U         /**< Invalid Offset */
#define SYN_BLE_ATT_ERR_INSUFFICIENT_AUTHOR 0x08U    /**< Insufficient Authorization */
#define SYN_BLE_ATT_ERR_PREPARE_QUEUE_FULL 0x09U     /**< Prepare Queue Full */
#define SYN_BLE_ATT_ERR_ATTR_NOT_FOUND 0x0AU         /**< Attribute Not Found */
#define SYN_BLE_ATT_ERR_ATTR_NOT_LONG 0x0BU          /**< Attribute Not Long */
#define SYN_BLE_ATT_ERR_INSUFFICIENT_KEY_SIZE 0x0CU  /**< Insufficient Key Size */
#define SYN_BLE_ATT_ERR_INVALID_ATTR_LEN 0x0DU       /**< Invalid Attribute Value Length */
#define SYN_BLE_ATT_ERR_UNLIKELY_ERROR 0x0EU         /**< Unlikely Error */
#define SYN_BLE_ATT_ERR_INSUFFICIENT_ENCRYPT 0x0FU   /**< Insufficient Encryption */
#define SYN_BLE_ATT_ERR_UNSUPPORTED_GROUP_TYPE 0x10U /**< Unsupported Group Type */
#define SYN_BLE_ATT_ERR_INSUFFICIENT_RESOURCES 0x11U /**< Insufficient Resources */
/**@}*/

#ifndef SYN_BLE_ATT_MTU_DEFAULT
#define SYN_BLE_ATT_MTU_DEFAULT 23U /**< Default minimum ATT MTU size */
#endif

/** @name Find Information Response Format Codes */
/**@{*/
#define SYN_BLE_ATT_FIND_INFO_FMT_UUID16 0x01U  /**< 16-bit UUID handle-type pairs (4 bytes) */
#define SYN_BLE_ATT_FIND_INFO_FMT_UUID128 0x02U /**< 128-bit UUID handle-type pairs (18 bytes) */
/**@}*/

/**
 * @brief Encode ATT Error Response.
 * @param req_opcode Request Opcode.
 * @param handle Attribute Handle.
 * @param err_code Error Code.
 * @param buf Output Buffer.
 * @param len Output Length Pointer.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_att_encode_error_rsp(uint8_t req_opcode, uint16_t handle, uint8_t err_code,
                                        uint8_t *buf, uint16_t *len);

/**
 * @brief Encode ATT Exchange MTU Response.
 * @param server_mtu Server MTU.
 * @param buf Output Buffer.
 * @param len Output Length Pointer.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_att_encode_mtu_rsp(uint16_t server_mtu, uint8_t *buf, uint16_t *len);

/**
 * @brief Encode ATT Read Response.
 * @param val Value bytes.
 * @param val_len Value length.
 * @param buf Output Buffer.
 * @param len Output Length Pointer.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_att_encode_read_rsp(const uint8_t *val, uint16_t val_len, uint8_t *buf,
                                       uint16_t *len);

/**
 * @brief Encode ATT Write Response.
 * @param buf Output Buffer.
 * @param len Output Length Pointer.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_att_encode_write_rsp(uint8_t *buf, uint16_t *len);

/**
 * @brief Encode ATT Handle Value Notification.
 * @param handle Attribute Handle.
 * @param val Value bytes.
 * @param val_len Value length.
 * @param buf Output Buffer.
 * @param len Output Length Pointer.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_att_encode_notification(uint16_t handle, const uint8_t *val, uint16_t val_len,
                                           uint8_t *buf, uint16_t *len);

/**
 * @brief Encode ATT Find Information Response (0x05).
 * @param format Format code (SYN_BLE_ATT_FIND_INFO_FMT_UUID16 or UUID128).
 * @param info_data Packed handle-UUID pair list.
 * @param info_len Byte length of info_data.
 * @param buf Output buffer.
 * @param len Output length pointer.
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid arguments.
 */
SYN_Status syn_ble_att_encode_find_info_rsp(uint8_t format, const uint8_t *info_data,
                                            uint16_t info_len, uint8_t *buf, uint16_t *len);

/**
 * @brief Encode ATT Read By Type Response (0x09).
 * @param pair_len Length of each handle-value pair in bytes (minimum 3).
 * @param data_list Packed handle-value pairs.
 * @param list_len Total byte length of data_list.
 * @param buf Output buffer.
 * @param len Output length pointer.
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid arguments.
 */
SYN_Status syn_ble_att_encode_read_by_type_rsp(uint8_t pair_len, const uint8_t *data_list,
                                               uint16_t list_len, uint8_t *buf, uint16_t *len);

/**
 * @brief Encode ATT Read By Group Type Response (0x11).
 * @param data_len Length of each attribute data element in bytes (minimum 5).
 * @param data_list Packed attribute data elements.
 * @param list_len Total byte length of data_list.
 * @param buf Output buffer.
 * @param len Output length pointer.
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid arguments.
 */
SYN_Status syn_ble_att_encode_read_by_group_rsp(uint8_t data_len, const uint8_t *data_list,
                                                uint16_t list_len, uint8_t *buf, uint16_t *len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BLE_ATT_H */
