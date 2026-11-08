/**
 * @file syn_usb_host_cdc.h
 * @brief Zero-Heap USB 2.0 Host CDC ACM Class Driver.
 *
 * Connects to downstream USB CDC ACM devices (serial adapters,
 * modems, other MCUs in Device mode). Provides read/write buffers
 * and protothread coroutine macros.
 */

#ifndef SYN_USB_HOST_CDC_H
#define SYN_USB_HOST_CDC_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb_cdc.h"
#include "syntropic/drivers/syn_usb_host.h"
#include "syntropic/pt/syn_pt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_USB_HOST_CDC_BUF_SIZE
#define SYN_USB_HOST_CDC_BUF_SIZE 128U /**< RX/TX buffer size */
#endif

/** USB Host CDC Instance Context */
typedef struct {
    const SYN_USB_Host *host; /**< Bound USB Host Core context */
    uint8_t dev_addr;         /**< Connected device bus address */
    uint8_t pipe_bulk_in;     /**< Pipe index for Bulk IN */
    uint8_t pipe_bulk_out;    /**< Pipe index for Bulk OUT */
    uint8_t ep_bulk_in;       /**< Endpoint address for Bulk IN */
    uint8_t ep_bulk_out;      /**< Endpoint address for Bulk OUT */
    bool connected;           /**< True if probe succeeded and device active */

    SYN_USB_CDC_LineCoding line_coding; /**< Active line coding config */

    uint8_t rx_buf[SYN_USB_HOST_CDC_BUF_SIZE]; /**< RX data buffer */
    uint16_t rx_len;                           /**< Unread bytes in rx_buf */
    uint8_t tx_buf[SYN_USB_HOST_CDC_BUF_SIZE]; /**< TX data buffer */
    uint16_t tx_len;                           /**< Pending bytes in tx_buf */
} SYN_USB_HostCDC;

/**
 * @brief Initialize USB Host CDC instance with defaults (115200 8N1).
 *
 * @param hcdc Pointer to USB Host CDC instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_host_cdc_init(SYN_USB_HostCDC *hcdc);

/**
 * @brief Register USB Host CDC class driver with USB Host Core.
 *
 * @param host Pointer to USB Host instance.
 * @param hcdc Pointer to USB Host CDC instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_host_cdc_register(SYN_USB_Host *host, SYN_USB_HostCDC *hcdc);

/**
 * @brief Write bytes to USB Host CDC TX buffer.
 *
 * @param hcdc Pointer to USB Host CDC instance.
 * @param data Pointer to data bytes.
 * @param len  Byte length.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_host_cdc_write(SYN_USB_HostCDC *hcdc, const void *data, size_t len);

/**
 * @brief Read bytes from USB Host CDC RX buffer.
 *
 * @param hcdc    Pointer to USB Host CDC instance.
 * @param buf     Pointer to destination buffer.
 * @param max_len Max capacity.
 * @param out_len Pointer to receive read byte count.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_host_cdc_read(SYN_USB_HostCDC *hcdc, void *buf, size_t max_len, size_t *out_len);

/**
 * @brief Check if receive data is available.
 *
 * @param hcdc Pointer to USB Host CDC instance.
 * @return true if unread data is present.
 */
bool syn_usb_host_cdc_rx_available(const SYN_USB_HostCDC *hcdc);

/**
 * @brief Check if transmit buffer is ready.
 *
 * @param hcdc Pointer to USB Host CDC instance.
 * @return true if transmit buffer is available.
 */
bool syn_usb_host_cdc_tx_ready(const SYN_USB_HostCDC *hcdc);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */

/**
 * @brief Block a protothread coroutine until Host CDC RX data is available.
 *
 * @param pt   Protothread context.
 * @param hcdc Pointer to USB Host CDC instance.
 */
#define PT_USB_HOST_CDC_WAIT_RX(pt, hcdc) PT_WAIT_UNTIL(pt, syn_usb_host_cdc_rx_available(hcdc))

/**
 * @brief Block a protothread coroutine until Host CDC TX buffer is ready.
 *
 * @param pt   Protothread context.
 * @param hcdc Pointer to USB Host CDC instance.
 */
#define PT_USB_HOST_CDC_WAIT_TX_READY(pt, hcdc) PT_WAIT_UNTIL(pt, syn_usb_host_cdc_tx_ready(hcdc))

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_HOST_CDC_H */
