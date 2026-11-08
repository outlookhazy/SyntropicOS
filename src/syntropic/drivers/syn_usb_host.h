/**
 * @file syn_usb_host.h
 * @brief Zero-Heap USB 2.0 Host Core Protocol Engine.
 *
 * Manages device attach/detach detection, bus reset, standard
 * enumeration (GET_DESCRIPTOR, SET_ADDRESS, SET_CONFIGURATION),
 * and dispatches matched interfaces to registered host class drivers.
 */

#ifndef SYN_USB_HOST_H
#define SYN_USB_HOST_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb.h"
#include "syntropic/pt/syn_pt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_USB_HOST_MAX_CLASSES
#define SYN_USB_HOST_MAX_CLASSES 4U /**< Maximum registered host class drivers */
#endif

#ifndef SYN_USB_HOST_MAX_PIPES
#define SYN_USB_HOST_MAX_PIPES 4U /**< Maximum host pipes (EP0 + data pipes) */
#endif

#ifndef SYN_USB_HOST_ENUM_BUF_SIZE
#define SYN_USB_HOST_ENUM_BUF_SIZE 256U /**< Enumeration descriptor scratch buffer */
#endif

/** USB Host States */
#define SYN_USB_HOST_STATE_DISCONNECTED 0U /**< No device attached */
#define SYN_USB_HOST_STATE_ATTACHED 1U     /**< Device physically present */
#define SYN_USB_HOST_STATE_RESET 2U        /**< Bus reset in progress */
#define SYN_USB_HOST_STATE_ENUMERATING 3U  /**< Reading descriptors & assigning address */
#define SYN_USB_HOST_STATE_READY 4U        /**< Device enumerated and class active */
#define SYN_USB_HOST_STATE_ERROR 5U        /**< Enumeration or transfer error */

/** Enumeration sub-steps (internal to ENUMERATING state) */
#define SYN_USB_HOST_ENUM_GET_DEV8 0U     /**< Read first 8 bytes of Device Desc */
#define SYN_USB_HOST_ENUM_SET_ADDR 1U     /**< Assign bus address */
#define SYN_USB_HOST_ENUM_GET_DEV_FULL 2U /**< Read full 18-byte Device Desc */
#define SYN_USB_HOST_ENUM_GET_CFG 3U      /**< Read Configuration Descriptor */
#define SYN_USB_HOST_ENUM_SET_CFG 4U      /**< Issue SET_CONFIGURATION */
#define SYN_USB_HOST_ENUM_CLASS_PROBE 5U  /**< Match & probe class drivers */
#define SYN_USB_HOST_ENUM_DONE 6U         /**< Enumeration complete */

/** Cached device information from enumeration */
typedef struct {
    uint8_t dev_addr;     /**< Assigned USB bus address */
    uint16_t vid;         /**< Vendor ID */
    uint16_t pid;         /**< Product ID */
    uint8_t dev_class;    /**< Device class code */
    uint8_t dev_subclass; /**< Device subclass code */
    uint8_t dev_protocol; /**< Device protocol code */
    uint8_t max_pkt_ep0;  /**< EP0 maximum packet size */
    uint8_t num_configs;  /**< Number of configurations */
} SYN_USB_HostDevInfo;

/** Host class driver registration entry */
typedef struct {
    uint8_t class_code;    /**< bInterfaceClass to match (0xFF = any) */
    uint8_t subclass_code; /**< bInterfaceSubClass to match (0xFF = any) */
    uint8_t protocol_code; /**< bInterfaceProtocol to match (0xFF = any) */
    void *ctx;             /**< Driver context pointer */
    bool matched;          /**< True if this driver matched during probe */

    /**
     * @brief Called when a matching interface is found during enumeration.
     *
     * @param ctx       Driver context.
     * @param dev_addr  Device bus address.
     * @param iface_desc Pointer to interface descriptor bytes.
     * @param len       Length of interface descriptor + subordinate descriptors.
     * @return SYN_OK to claim the interface.
     */
    SYN_Status (*probe)(void *ctx, uint8_t dev_addr, const uint8_t *iface_desc, uint16_t len);

    /**
     * @brief Called when the device is detached.
     *
     * @param ctx Driver context.
     */
    void (*disconnected)(void *ctx);

    /**
     * @brief Called each tick while in READY state for class-specific polling.
     *
     * @param ctx Driver context.
     */
    void (*process)(void *ctx);
} SYN_USB_HostClassDriver;

/** USB Host Instance Context */
typedef struct {
    uint8_t state;       /**< Host state (SYN_USB_HOST_STATE_*) */
    uint8_t enum_step;   /**< Enumeration sub-step */
    uint8_t next_addr;   /**< Next device address to assign (1..127) */
    uint8_t class_count; /**< Number of registered class drivers */

    bool xfer_pending; /**< True if a HAL transfer is in-flight */

    SYN_USB_HostDevInfo dev_info; /**< Cached device descriptor info */

    uint8_t enum_buf[SYN_USB_HOST_ENUM_BUF_SIZE]; /**< Descriptor scratch buffer */
    uint16_t enum_buf_len;                        /**< Valid bytes in enum_buf */

    SYN_USB_HostClassDriver classes[SYN_USB_HOST_MAX_CLASSES]; /**< Class driver registry */
} SYN_USB_Host;

/**
 * @brief Initialize USB Host Core.
 *
 * @param host Pointer to USB Host instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_host_init(SYN_USB_Host *host);

/**
 * @brief Register a host class driver for interface matching.
 *
 * @param host Pointer to USB Host instance.
 * @param cls  Host class driver vtable.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_host_register_class(SYN_USB_Host *host, const SYN_USB_HostClassDriver *cls);

/**
 * @brief Process USB Host state machine (call each scheduler tick).
 *
 * Drives attach detection, bus reset, enumeration, class probing,
 * and detach handling. Non-blocking.
 *
 * @param host Pointer to USB Host instance.
 * @return SYN_OK on success, SYN_BUSY if transfer pending.
 */
SYN_Status syn_usb_host_process(SYN_USB_Host *host);

/**
 * @brief Check if USB Host has a device enumerated and ready.
 *
 * @param host Pointer to USB Host instance.
 * @return true if host state is READY.
 */
bool syn_usb_host_is_ready(const SYN_USB_Host *host);

/**
 * @brief Get cached device information from last enumeration.
 *
 * @param host Pointer to USB Host instance.
 * @return Pointer to device info struct, or NULL if no device.
 */
const SYN_USB_HostDevInfo *syn_usb_host_get_dev_info(const SYN_USB_Host *host);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */

/**
 * @brief Block a protothread coroutine until a USB device is enumerated and ready.
 *
 * @param pt   Protothread context.
 * @param host Pointer to USB Host instance.
 */
#define PT_USB_HOST_WAIT_READY(pt, host) PT_WAIT_UNTIL(pt, syn_usb_host_is_ready(host))

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_HOST_H */
