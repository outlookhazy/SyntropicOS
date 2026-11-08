/**
 * @file syn_usb.h
 * @brief Zero-Heap Modular USB 2.0 Device Core Protocol Engine.
 */

#ifndef SYN_USB_H
#define SYN_USB_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/pt/syn_pt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_USB_MAX_CLASSES
#define SYN_USB_MAX_CLASSES 4U /**< Maximum registered class drivers */
#endif

#ifndef SYN_USB_MAX_CONFIG_DESC
#define SYN_USB_MAX_CONFIG_DESC \
    256U /**< Maximum auto-assembled configuration descriptor buffer capacity */
#endif

#ifndef SYN_USB_MAX_STRING_DESCS
#define SYN_USB_MAX_STRING_DESCS 8U /**< Maximum string descriptor slots */
#endif

#define SYN_USB_EP0_BUF_SIZE \
    SYN_USB_MAX_CONFIG_DESC /**< EP0 control buffer size (fits full config descriptor) */

/** USB Device States per USB 2.0 Spec §9.1.1 */
#define SYN_USB_STATE_DEFAULT 0U    /**< USB Default state */
#define SYN_USB_STATE_ADDRESS 1U    /**< USB Address state */
#define SYN_USB_STATE_CONFIGURED 2U /**< USB Configured state */

/** Standard USB Request Codes per USB 2.0 Spec Table 9-4 */
#define SYN_USB_REQ_GET_STATUS 0x00U        /**< USB Request Get Status (0x00) */
#define SYN_USB_REQ_CLEAR_FEATURE 0x01U     /**< USB Request Clear Feature (0x01) */
#define SYN_USB_REQ_SET_FEATURE 0x03U       /**< USB Request Set Feature (0x03) */
#define SYN_USB_REQ_SET_ADDRESS 0x05U       /**< USB Request Set Address (0x05) */
#define SYN_USB_REQ_GET_DESCRIPTOR 0x06U    /**< USB Request Get Descriptor (0x06) */
#define SYN_USB_REQ_SET_DESCRIPTOR 0x07U    /**< USB Request Set Descriptor (0x07) */
#define SYN_USB_REQ_GET_CONFIGURATION 0x08U /**< USB Request Get Configuration (0x08) */
#define SYN_USB_REQ_SET_CONFIGURATION 0x09U /**< USB Request Set Configuration (0x09) */

/** Descriptor Types */
#define SYN_USB_DESC_TYPE_DEVICE 0x01U        /**< Device Descriptor Type (0x01) */
#define SYN_USB_DESC_TYPE_CONFIGURATION 0x02U /**< Configuration Descriptor Type (0x02) */
#define SYN_USB_DESC_TYPE_STRING 0x03U        /**< String Descriptor Type (0x03) */
#define SYN_USB_DESC_TYPE_INTERFACE 0x04U     /**< Interface Descriptor Type (0x04) */
#define SYN_USB_DESC_TYPE_ENDPOINT 0x05U      /**< Endpoint Descriptor Type (0x05) */

/** USB Setup Packet Format */
typedef struct {
    uint8_t bmRequestType; /**< Characteristics of request */
    uint8_t bRequest;      /**< Specific request code */
    uint16_t wValue;       /**< Word-sized parameter */
    uint16_t wIndex;       /**< Interface/endpoint index */
    uint16_t wLength;      /**< Transfer length */
} SYN_USB_SetupPacket;

/** USB Class Driver Registration Entry */
typedef struct {
    uint8_t iface_start; /**< First interface index */
    uint8_t iface_count; /**< Interface count */
    void *ctx;           /**< Driver context */

    SYN_Status (*setup)(void *ctx, const SYN_USB_SetupPacket *pkt, uint8_t *resp, uint16_t resp_cap,
                        uint16_t *rlen); /**< Class setup handler */
    void (*data_out)(void *ctx, uint8_t ep, const uint8_t *data,
                     uint16_t len);                      /**< Data OUT handler */
    void (*data_in)(void *ctx, uint8_t ep);              /**< Data IN handler */
    SYN_Status (*configured)(void *ctx, uint8_t config); /**< Configured state callback */
} SYN_USB_ClassDriver;

/** USB Device Instance Context */
typedef struct {
    uint8_t state;        /**< USB Device State (SYN_USB_STATE_*) */
    uint8_t dev_address;  /**< Assigned device bus address */
    uint8_t config_value; /**< Active configuration value */
    uint8_t class_count;  /**< Number of registered class drivers */
    uint8_t iface_count;  /**< Total allocated USB interface count */
    uint16_t ep_in_mask;  /**< Bitmask of allocated IN endpoints (bits 1..15) */
    uint16_t ep_out_mask; /**< Bitmask of allocated OUT endpoints (bits 1..15) */

    uint8_t ep0_buf[SYN_USB_EP0_BUF_SIZE]; /**< Internal EP0 control buffer */

    const uint8_t *device_desc; /**< Pointer to 18-byte Device Descriptor */
    const uint8_t *config_desc; /**< Active configuration descriptor pointer */
    uint16_t config_desc_len;   /**< Configuration descriptor length */
    bool raw_config_override;   /**< True if raw config descriptor set */

    const uint8_t *string_descs[SYN_USB_MAX_STRING_DESCS]; /**< String descriptor pointers */
    uint8_t string_desc_count;                             /**< String descriptor count */

    uint8_t config_buf[SYN_USB_MAX_CONFIG_DESC]; /**< Auto-assembled config descriptor buffer */
    uint16_t config_buf_used;                    /**< Length of auto-assembled config descriptor */

    SYN_USB_ClassDriver classes[SYN_USB_MAX_CLASSES]; /**< Class driver registry array */
} SYN_USB_Device;

/**
 * @brief Initialize USB Device Core with standard 18-byte Device Descriptor.
 *
 * @param dev Pointer to USB device instance.
 * @param device_desc Pointer to 18-byte standard USB device descriptor.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_init(SYN_USB_Device *dev, const uint8_t *device_desc);

/**
 * @brief Allocate a non-colliding endpoint address on a USB device.
 *
 * Uses @p preferred_ep when free, or assigns the lowest available endpoint
 * number (1..15) in the same direction.
 *
 * @param dev          Pointer to USB device context.
 * @param preferred_ep Preferred endpoint address (bit 7 = direction, bits 3..0 = number).
 * @param assigned_ep  Output pointer receiving assigned endpoint address.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL, SYN_ERROR if endpoints exhausted.
 */
SYN_Status syn_usb_alloc_ep(SYN_USB_Device *dev, uint8_t preferred_ep, uint8_t *assigned_ep);

/**
 * @brief Register a class driver and append its interface descriptors.
 *
 * @param dev Pointer to USB device context.
 * @param cls Class driver vtable and binding context.
 * @param iface_desc Pointer to class interface descriptor payload.
 * @param iface_desc_len Length of interface descriptor payload.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_register_class(SYN_USB_Device *dev, const SYN_USB_ClassDriver *cls,
                                  const uint8_t *iface_desc, uint16_t iface_desc_len);

/**
 * @brief Set string descriptor pointer for a given index.
 *
 * @param dev Pointer to USB device context.
 * @param index String index (0=Language ID, 1=Manufacturer, 2=Product, etc).
 * @param desc Pointer to UNICODE string descriptor.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_set_string_desc(SYN_USB_Device *dev, uint8_t index, const uint8_t *desc);

/**
 * @brief Set raw configuration descriptor pointer (overrides auto-assembly).
 *
 * @param dev Pointer to USB device context.
 * @param desc Pointer to raw configuration descriptor.
 * @param len Length in bytes.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_set_raw_config_desc(SYN_USB_Device *dev, const uint8_t *desc, uint16_t len);

/**
 * @brief Process Control Setup packet from Host (EP0).
 *
 * @param dev      Pointer to USB device context.
 * @param pkt      Pointer to received Setup Packet.
 * @param resp     Output buffer for data stage response.
 * @param resp_cap Capacity of @p resp in bytes.
 * @param rlen     Output pointer to receive response byte length.
 * @return SYN_OK on success, SYN_ERROR if unsupported or response exceeds @p resp_cap.
 */
SYN_Status syn_usb_process_setup(SYN_USB_Device *dev, const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                                 uint16_t resp_cap, uint16_t *rlen);

/**
 * @brief Check if USB device is in CONFIGURED state.
 *
 * @param dev Pointer to USB device context.
 * @return true if device is configured.
 */
bool syn_usb_is_configured(const SYN_USB_Device *dev);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */

/**
 * @brief Block a protothread coroutine until USB device reaches CONFIGURED state.
 *
 * @param pt  Protothread context.
 * @param dev Pointer to USB device instance.
 */
#define PT_USB_WAIT_CONFIGURED(pt, dev) PT_WAIT_UNTIL(pt, syn_usb_is_configured(dev))

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_H */
