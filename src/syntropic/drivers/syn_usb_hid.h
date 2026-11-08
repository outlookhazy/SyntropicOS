/**
 * @file syn_usb_hid.h
 * @brief Zero-Heap USB 2.0 Human Interface Device (HID) Class Driver.
 */

#ifndef SYN_USB_HID_H
#define SYN_USB_HID_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb.h"
#include "syntropic/pt/syn_pt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_USB_HID_MAX_REPORT_SIZE 64U /**< Maximum HID report size */

/** HID Class Specific Requests per USB HID Spec 1.11 §7.2 */
#define SYN_USB_HID_REQ_GET_REPORT 0x01U   /**< HID Request Get Report (0x01) */
#define SYN_USB_HID_REQ_GET_IDLE 0x02U     /**< HID Request Get Idle (0x02) */
#define SYN_USB_HID_REQ_GET_PROTOCOL 0x03U /**< HID Request Get Protocol (0x03) */
#define SYN_USB_HID_REQ_SET_REPORT 0x09U   /**< HID Request Set Report (0x09) */
#define SYN_USB_HID_REQ_SET_IDLE 0x0AU     /**< HID Request Set Idle (0x0A) */
#define SYN_USB_HID_REQ_SET_PROTOCOL 0x0BU /**< HID Request Set Protocol (0x0B) */

/** USB HID Instance Context */
typedef struct {
    uint8_t ep_in;                               /**< Interrupt IN Endpoint address */
    uint8_t ep_out;                              /**< Interrupt OUT Endpoint address */
    uint8_t iface_num;                           /**< Assigned interface index */
    uint8_t protocol;                            /**< Subclass protocol */
    const uint8_t *report_desc;                  /**< Pointer to HID report descriptor */
    uint16_t report_desc_len;                    /**< Length of HID report descriptor */
    uint8_t tx_buf[SYN_USB_HID_MAX_REPORT_SIZE]; /**< IN report payload buffer */
    uint16_t tx_len;                             /**< Pending IN report byte length */
    uint8_t rx_buf[SYN_USB_HID_MAX_REPORT_SIZE]; /**< OUT report payload buffer */
    uint16_t rx_len;                             /**< Unread OUT report byte length */
    uint8_t idle_rate;                           /**< Active HID idle rate */
    uint8_t active_protocol;                     /**< Active HID protocol */
    bool release_pending; /**< Auto-release mouse report queued for next IN completion */
} SYN_USB_HID;

/**
 * @brief Initialize USB HID Class Instance.
 *
 * @param hid Pointer to USB HID instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_hid_init(SYN_USB_HID *hid);

/**
 * @brief Register USB HID class driver with USB device core.
 *
 * @param dev Pointer to USB device core context.
 * @param hid Pointer to USB HID instance.
 * @param report_desc Pointer to static HID report descriptor.
 * @param report_desc_len Byte length of report descriptor.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_hid_register(SYN_USB_Device *dev, SYN_USB_HID *hid, const uint8_t *report_desc,
                                uint16_t report_desc_len);

/**
 * @brief Stage an IN report in the HID Interrupt IN transmit buffer.
 *
 * @note Port-decoupled staging contract: copies up to `SYN_USB_HID_MAX_REPORT_SIZE` (64 B)
 *       into `hid->tx_buf` and sets `hid->tx_len`. The platform USB ISR transmits
 *       `hid->tx_buf` on `hid->ep_in` and invokes the class `data_in` callback upon
 *       completion to clear `hid->tx_len` (or advance a queued auto-release report).
 *
 * @param hid Pointer to USB HID instance.
 * @param report Pointer to report data.
 * @param len Report byte length.
 * @return SYN_OK on success, SYN_BUSY if a staged report is still pending.
 */
SYN_Status syn_usb_hid_send_report(SYN_USB_HID *hid, const void *report, size_t len);

/**
 * @brief Read received OUT report from host.
 *
 * @param hid Pointer to USB HID instance.
 * @param buf Output buffer.
 * @param max_len Capacity.
 * @param out_len Pointer to receive read byte count.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_hid_read_report(SYN_USB_HID *hid, void *buf, size_t max_len, size_t *out_len);

/**
 * @brief Check if OUT report is available.
 *
 * @param hid Pointer to USB HID instance.
 * @return true if unread report data exists.
 */
bool syn_usb_hid_report_available(const SYN_USB_HID *hid);

/**
 * @brief Check if IN transmit buffer is ready.
 *
 * @param hid Pointer to USB HID instance.
 * @return true if transmit buffer is free.
 */
bool syn_usb_hid_tx_ready(const SYN_USB_HID *hid);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */

/**
 * @brief Block a protothread coroutine until an OUT report is available.
 *
 * @param pt  Protothread context.
 * @param hid Pointer to USB HID instance.
 */
#define PT_USB_HID_WAIT_RX(pt, hid) PT_WAIT_UNTIL(pt, syn_usb_hid_report_available(hid))

/**
 * @brief Block a protothread coroutine until the IN report transmit buffer is ready.
 *
 * @param pt  Protothread context.
 * @param hid Pointer to USB HID instance.
 */
#define PT_USB_HID_WAIT_TX_READY(pt, hid) PT_WAIT_UNTIL(pt, syn_usb_hid_tx_ready(hid))

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_HID_H */
