/**
 * @file syn_usb_cdc.h
 * @brief Zero-Heap USB 2.0 CDC ACM Virtual COM Port Class Driver.
 *
 * USB CDC specifications:
 * - Interface 0: CDC Communication Class (Interrupt EP 0x82 / 8B).
 * - Interface 1: CDC Data Class (Bulk IN EP 0x81 / 64B, Bulk OUT EP 0x01 / 64B).
 */

#ifndef SYN_USB_CDC_H
#define SYN_USB_CDC_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb.h"
#include "syntropic/pt/syn_pt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_USB_CDC_SET_LINE_CODING 0x20U        /**< CDC Request Set Line Coding (0x20) */
#define SYN_USB_CDC_GET_LINE_CODING 0x21U        /**< CDC Request Get Line Coding (0x21) */
#define SYN_USB_CDC_SET_CONTROL_LINE_STATE 0x22U /**< CDC Request Set Control Line State (0x22) */

/** CDC Line Coding Config (Baud rate, Stop bits, Parity, Data bits). */
typedef struct {
    uint32_t baudrate; /**< Transmission baud rate in bits per second */
    uint8_t stop_bits; /**< Stop bits setting (0=1, 1=1.5, 2=2) */
    uint8_t parity;    /**< Parity setting (0=None, 1=Odd, 2=Even) */
    uint8_t data_bits; /**< Data bits count (5, 6, 7, 8, 16) */
} SYN_USB_CDC_LineCoding;

/** USB CDC Instance. */
typedef struct {
    uint8_t ep_in;                      /**< Bulk IN Endpoint address */
    uint8_t ep_out;                     /**< Bulk OUT Endpoint address */
    uint8_t ep_cmd;                     /**< Interrupt Command Endpoint address */
    uint8_t iface_num;                  /**< First assigned USB interface index */
    uint8_t dev_address;                /**< Assigned USB device bus address */
    bool configured;                    /**< True if USB device is in Configured state */
    SYN_USB_CDC_LineCoding line_coding; /**< Active CDC line coding configuration */
    uint8_t rx_buf[128];                /**< Internal RX ring/linear payload buffer */
    uint16_t rx_len;                    /**< Length of unread data in rx_buf */
    uint8_t tx_buf[128];                /**< Internal TX ring/linear payload buffer */
    uint16_t tx_len;                    /**< Length of pending data in tx_buf */
} SYN_USB_CDC;

/**
 * @brief Initialize USB CDC Class instance with defaults (115200 baud, 8N1).
 *
 * @param cdc Pointer to USB CDC instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_cdc_init(SYN_USB_CDC *cdc);

/**
 * @brief Register USB CDC ACM class driver with USB device core.
 *
 * @param dev Pointer to USB device core context.
 * @param cdc Pointer to USB CDC instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_cdc_register(SYN_USB_Device *dev, SYN_USB_CDC *cdc);

/**
 * @brief Handle Control Setup Request from host (EP0).
 *
 * @param cdc   Pointer to USB CDC instance.
 * @param setup Pointer to setup packet.
 * @param resp  Pointer to response buffer.
 * @param rlen  Pointer to receive response length.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_cdc_handle_setup(SYN_USB_CDC *cdc, const SYN_USB_SetupPacket *setup,
                                    uint8_t *resp, size_t *rlen);

/**
 * @brief Write bytes to USB CDC Bulk IN transmit buffer.
 *
 * @param cdc  Pointer to USB CDC instance.
 * @param data Pointer to data bytes.
 * @param len  Byte length.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_cdc_write(SYN_USB_CDC *cdc, const void *data, size_t len);

/**
 * @brief Read bytes from USB CDC Bulk OUT receive buffer.
 *
 * @param cdc     Pointer to USB CDC instance.
 * @param buf     Pointer to destination buffer.
 * @param max_len Max capacity.
 * @param out_len Pointer to receive read byte count.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_cdc_read(SYN_USB_CDC *cdc, void *buf, size_t max_len, size_t *out_len);

/**
 * @brief Check if receive data is available.
 *
 * @param cdc Pointer to USB CDC instance.
 * @return true if unread data is present in rx_buf.
 */
bool syn_usb_cdc_rx_available(const SYN_USB_CDC *cdc);

/**
 * @brief Check if transmit buffer is ready.
 *
 * @param cdc Pointer to USB CDC instance.
 * @return true if transmit buffer is available.
 */
bool syn_usb_cdc_tx_ready(const SYN_USB_CDC *cdc);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */

/**
 * @brief Block a protothread coroutine until CDC RX data is available.
 *
 * @param pt  Protothread context.
 * @param cdc Pointer to USB CDC instance.
 */
#define PT_USB_CDC_WAIT_RX(pt, cdc) PT_WAIT_UNTIL(pt, syn_usb_cdc_rx_available(cdc))

/**
 * @brief Block a protothread coroutine until CDC TX buffer is ready.
 *
 * @param pt  Protothread context.
 * @param cdc Pointer to USB CDC instance.
 */
#define PT_USB_CDC_WAIT_TX_READY(pt, cdc) PT_WAIT_UNTIL(pt, syn_usb_cdc_tx_ready(cdc))

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_CDC_H */
