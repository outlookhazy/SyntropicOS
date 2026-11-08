/**
 * @file syn_port_usb.h
 * @brief USB HAL hardware port interface contract.
 */

#ifndef SYN_PORT_USB_H
#define SYN_PORT_USB_H

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** USB Endpoint Types per USB Spec */
#define SYN_USB_EP_TYPE_CONTROL 0U /**< Control Endpoint (0) */
#define SYN_USB_EP_TYPE_ISOC 1U    /**< Isochronous Endpoint (1) */
#define SYN_USB_EP_TYPE_BULK 2U    /**< Bulk Endpoint (2) */
#define SYN_USB_EP_TYPE_INTR 3U    /**< Interrupt Endpoint (3) */

/**
 * @brief Initialize low-level USB peripheral hardware.
 *
 * @return SYN_OK on success.
 */
SYN_Status syn_port_usb_init(void);

/**
 * @brief Signal physical USB connection/attach (e.g. enable D+ pullup).
 *
 * @return SYN_OK on success.
 */
SYN_Status syn_port_usb_connect(void);

/**
 * @brief Signal physical USB disconnect/detach (e.g. disable D+ pullup).
 *
 * @return SYN_OK on success.
 */
SYN_Status syn_port_usb_disconnect(void);

/**
 * @brief Assign USB device address in hardware controller.
 *
 * @param addr assigned address (1..127).
 * @return SYN_OK on success.
 */
SYN_Status syn_port_usb_set_address(uint8_t addr);

/**
 * @brief Open and configure a hardware endpoint.
 *
 * @param ep_addr Bit 7: Direction (0=OUT, 1=IN), Bits 3:0: Endpoint number.
 * @param ep_type Endpoint type (SYN_USB_EP_TYPE_*).
 * @param max_pkt Maximum packet size in bytes.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_usb_ep_open(uint8_t ep_addr, uint8_t ep_type, uint16_t max_pkt);

/**
 * @brief Close a hardware endpoint.
 *
 * @param ep_addr Endpoint address.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_usb_ep_close(uint8_t ep_addr);

/**
 * @brief Write packet to an IN endpoint FIFO/buffer.
 *
 * @param ep_addr Endpoint address.
 * @param data Pointer to payload bytes.
 * @param len Byte length.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_usb_ep_write(uint8_t ep_addr, const void *data, uint16_t len);

/**
 * @brief Read packet from an OUT endpoint FIFO/buffer.
 *
 * @param ep_addr Endpoint address.
 * @param buf Destination buffer.
 * @param max_len Max capacity.
 * @param out_len Pointer to receive read byte count.
 * @return SYN_OK on success, SYN_BUSY if no packet available.
 */
SYN_Status syn_port_usb_ep_read(uint8_t ep_addr, void *buf, uint16_t max_len, uint16_t *out_len);

/**
 * @brief Signal STALL condition on an endpoint.
 *
 * @param ep_addr Endpoint address.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_usb_ep_stall(uint8_t ep_addr);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_USB_H */
