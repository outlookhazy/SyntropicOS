/**
 * @file syn_transport_usb_host_cdc.c
 * @brief USB Host CDC Transport Binding Implementation.
 */

#include "syntropic/drivers/syn_transport_usb_host_cdc.h"

#include <string.h>

/**
 * @brief Internal transport send wrapper for USB Host CDC.
 * @param data Packet data buffer.
 * @param len Byte length.
 * @param ctx Context pointer (SYN_USB_HostCDC).
 * @return true on success.
 */
static bool host_cdc_transport_send(const uint8_t *data, size_t len, void *ctx)
{
    SYN_USB_HostCDC *hcdc = (SYN_USB_HostCDC *)ctx;
    if ((hcdc == NULL) || (data == NULL) || (len == 0U)) {
        return false;
    }
    return (syn_usb_host_cdc_write(hcdc, data, len) == SYN_OK);
}

/**
 * @brief Internal transport receive wrapper for USB Host CDC.
 * @param data Output data buffer.
 * @param max_len Capacity.
 * @param out_len Pointer to receive read byte count.
 * @param ctx Context pointer (SYN_USB_HostCDC).
 * @return true when at least one byte was read.
 */
static bool host_cdc_transport_recv(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    SYN_USB_HostCDC *hcdc = (SYN_USB_HostCDC *)ctx;
    if ((hcdc == NULL) || (data == NULL) || (out_len == NULL)) {
        return false;
    }
    SYN_Status st = syn_usb_host_cdc_read(hcdc, data, max_len, out_len);
    return (st == SYN_OK) && (*out_len > 0U);
}

/**
 * @brief Check whether unread bytes are queued in USB Host CDC receive buffer.
 * @param ctx Context pointer (SYN_USB_HostCDC).
 * @return true when unread RX bytes are available.
 */
static bool host_cdc_transport_has_data(const void *ctx)
{
    return syn_usb_host_cdc_rx_available((const SYN_USB_HostCDC *)ctx);
}

void syn_transport_from_usb_host_cdc(SYN_Transport *t, SYN_USB_HostCDC *hcdc)
{
    if (t == NULL) {
        return;
    }
    memset(t, 0, sizeof(*t));
    if (hcdc != NULL) {
        t->send = host_cdc_transport_send;
        t->recv = host_cdc_transport_recv;
        t->has_data = host_cdc_transport_has_data;
        t->ctx = hcdc;
    }
}
