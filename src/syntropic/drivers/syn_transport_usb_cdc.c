/**
 * @file syn_transport_usb_cdc.c
 * @brief USB CDC Transport Binding Implementation.
 * @ingroup syn_net
 */

#include "syntropic/drivers/syn_transport_usb_cdc.h"

#include <string.h>

/**
 * @brief Transmit packet via USB CDC transport.
 *
 * @param data Packet payload buffer.
 * @param len Packet length.
 * @param ctx Pointer to SYN_USB_CDC instance context.
 * @return true on success.
 */
static bool cdc_transport_send(const uint8_t *data, size_t len, void *ctx)
{
    SYN_USB_CDC *cdc = (SYN_USB_CDC *)ctx;
    if ((cdc == NULL) || (data == NULL) || (len == 0U)) {
        return false;
    }
    return (syn_usb_cdc_write(cdc, data, len) == SYN_OK);
}

/**
 * @brief Receive packet via USB CDC transport.
 *
 * @param data Buffer to receive payload.
 * @param max_len Capacity.
 * @param out_len Pointer to receive read byte count.
 * @param ctx Pointer to SYN_USB_CDC instance context.
 * @return true if packet read successfully.
 */
static bool cdc_transport_recv(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    SYN_USB_CDC *cdc = (SYN_USB_CDC *)ctx;
    if ((cdc == NULL) || (data == NULL) || (out_len == NULL)) {
        return false;
    }
    SYN_Status st = syn_usb_cdc_read(cdc, data, max_len, out_len);
    return (st == SYN_OK) && (*out_len > 0U);
}

/**
 * @brief Check whether unread bytes are queued in USB CDC receive buffer.
 *
 * @param ctx Pointer to SYN_USB_CDC instance context.
 * @return true when unread RX bytes are available.
 */
static bool cdc_transport_has_data(const void *ctx)
{
    return syn_usb_cdc_rx_available((const SYN_USB_CDC *)ctx);
}

void syn_transport_from_usb_cdc(SYN_Transport *t, SYN_USB_CDC *cdc)
{
    if (t == NULL) {
        return;
    }
    memset(t, 0, sizeof(*t));
    if (cdc != NULL) {
        t->send = cdc_transport_send;
        t->recv = cdc_transport_recv;
        t->has_data = cdc_transport_has_data;
        t->ctx = cdc;
    }
}
