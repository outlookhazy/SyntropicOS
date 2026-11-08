/**
 * @file syn_ble_gatt.h
 * @brief Zero-Heap Generic Attribute Profile (GATT) Server Engine.
 */

#ifndef SYN_BLE_GATT_H
#define SYN_BLE_GATT_H

#include "../common/syn_defs.h"
#include "syn_ble_att.h"
#include "syn_ble_l2cap.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @name Common 16-bit UUID Definitions */
/**@{*/
#define SYN_BLE_UUID_PRIMARY_SERVICE 0x2800U   /**< Primary Service Declaration UUID */
#define SYN_BLE_UUID_SECONDARY_SERVICE 0x2801U /**< Secondary Service Declaration UUID */
#define SYN_BLE_UUID_CHARACTERISTIC 0x2803U    /**< Characteristic Declaration UUID */
#define SYN_BLE_UUID_CCCD 0x2902U /**< Client Characteristic Configuration Descriptor UUID */
/**@}*/

/** @name Characteristic Property Flags */
/**@{*/
#define SYN_BLE_PROP_READ 0x02U      /**< Read Property */
#define SYN_BLE_PROP_WRITE_CMD 0x04U /**< Write Command (Without Response) Property */
#define SYN_BLE_PROP_WRITE_NO_RSP \
    SYN_BLE_PROP_WRITE_CMD          /**< Alias for Write Without Response Property */
#define SYN_BLE_PROP_WRITE 0x08U    /**< Write Request Property */
#define SYN_BLE_PROP_NOTIFY 0x10U   /**< Notify Property */
#define SYN_BLE_PROP_INDICATE 0x20U /**< Indicate Property */
/**@}*/

#ifndef SYN_BLE_MAX_ATTRIBUTES
#define SYN_BLE_MAX_ATTRIBUTES 32U /**< Maximum GATT attribute table capacity */
#endif

#ifndef SYN_BLE_MAX_CCCDS
#define SYN_BLE_MAX_CCCDS 8U /**< Maximum independent CCCD descriptors per GATT server */
#endif

#define SYN_BLE_GATT_MAX_ATTR_LEN 64U /**< Maximum single-attribute scratch buffer size */

typedef struct SYN_BLE_GATT_s SYN_BLE_GATT;

/** Attribute Read Callback */
typedef SYN_Status (*SYN_BLE_GATT_ReadCb)(SYN_BLE_GATT *gatt, uint16_t conn_handle,
                                          uint16_t attr_handle, uint8_t *val, uint16_t *val_len,
                                          void *user_data);

/** Attribute Write Callback */
typedef SYN_Status (*SYN_BLE_GATT_WriteCb)(SYN_BLE_GATT *gatt, uint16_t conn_handle,
                                           uint16_t attr_handle, const uint8_t *val,
                                           uint16_t val_len, void *user_data);

/** Attribute Table Entry */
typedef struct {
    uint16_t handle;               /**< Attribute 16-bit handle */
    uint16_t uuid16;               /**< Attribute 16-bit UUID */
    uint8_t permissions;           /**< Property/permission bitmask */
    SYN_BLE_GATT_ReadCb read_cb;   /**< Dynamic read callback */
    SYN_BLE_GATT_WriteCb write_cb; /**< Dynamic write callback */
    const uint8_t *static_val;     /**< Static value buffer pointer */
    uint16_t static_val_len;       /**< Static value length */
} SYN_BLE_GATT_Attr;

/** GATT Server Instance Context */
struct SYN_BLE_GATT_s {
    const SYN_BLE_GATT_Attr *table;                                 /**< Attribute table pointer */
    uint16_t table_len;                                             /**< Entry count */
    uint16_t cccd_vals[SYN_BLE_MAX_CONNECTIONS][SYN_BLE_MAX_CCCDS]; /**< CCCD state per conn/slot */
    void *user_data;                                                /**< User context pointer */
};

/**
 * @brief Initialize GATT Server context.
 * @param gatt Context handle pointer.
 * @param table Attribute table array pointer.
 * @param table_len Count of attribute entries.
 * @param user_data User context pointer.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_gatt_init(SYN_BLE_GATT *gatt, const SYN_BLE_GATT_Attr *table, uint16_t table_len,
                             void *user_data);

/**
 * @brief Process incoming ATT PDU received over L2CAP channel 0x0004.
 * @param gatt Context handle pointer.
 * @param conn_handle Connection handle.
 * @param att_pdu Received ATT PDU buffer.
 * @param pdu_len PDU length.
 * @param resp_buf Buffer to store outbound ATT Response PDU.
 * @param resp_len Pointer to store response size.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_gatt_process_att_pdu(SYN_BLE_GATT *gatt, uint16_t conn_handle,
                                        const uint8_t *att_pdu, uint16_t pdu_len, uint8_t *resp_buf,
                                        uint16_t *resp_len);

/**
 * @brief Encode Outbound GATT Notification for attribute.
 * @param gatt Context handle pointer.
 * @param conn_handle Target connection handle.
 * @param attr_handle Target characteristic value attribute handle.
 * @param val Value buffer pointer.
 * @param val_len Value byte length.
 * @param tx_buf Output buffer for HCI ACL payload.
 * @param tx_len Output size pointer.
 * @return SYN_OK on success.
 */
SYN_Status syn_ble_gatt_notify(SYN_BLE_GATT *gatt, uint16_t conn_handle, uint16_t attr_handle,
                               const uint8_t *val, uint16_t val_len, uint8_t *tx_buf,
                               uint16_t *tx_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BLE_GATT_H */
