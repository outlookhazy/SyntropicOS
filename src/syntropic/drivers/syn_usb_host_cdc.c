/**
 * @file syn_usb_host_cdc.c
 * @brief Zero-Heap USB 2.0 Host CDC ACM Class Driver Implementation.
 */

#include "syntropic/drivers/syn_usb_host_cdc.h"

#include "port/syn_port_usb.h"
#include "port/syn_port_usb_host.h"

#include <string.h>

/**
 * @brief Scan a descriptor slice and open Bulk IN/OUT pipes when bulk endpoints are found.
 * @param hcdc     Pointer to USB Host CDC instance.
 * @param dev_addr Device bus address.
 * @param buf      Descriptor byte buffer.
 * @param len      Byte length of @p buf.
 */
static void host_cdc_scan_endpoints(SYN_USB_HostCDC *hcdc, uint8_t dev_addr, const uint8_t *buf,
                                    uint16_t len)
{
    uint16_t pos = 0U;
    while ((pos + 2U) <= len) {
        uint8_t b_len = buf[pos];
        uint8_t b_type = buf[pos + 1U];
        if ((b_len < 2U) || ((pos + b_len) > len)) {
            break;
        }

        if ((b_type == SYN_USB_DESC_TYPE_ENDPOINT) && (b_len >= 7U) &&
            ((buf[pos + 3U] & 0x03U) == SYN_USB_EP_TYPE_BULK)) {
            uint8_t ep_addr = buf[pos + 2U];
            uint16_t max_pkt = (uint16_t)buf[pos + 4U] | ((uint16_t)buf[pos + 5U] << 8);
            if ((ep_addr & 0x80U) != 0U) {
                hcdc->ep_bulk_in = ep_addr;
                hcdc->pipe_bulk_in = 1U;
                (void)syn_port_usb_host_pipe_open(1U, dev_addr, ep_addr, SYN_USB_EP_TYPE_BULK,
                                                  max_pkt);
            } else {
                hcdc->ep_bulk_out = ep_addr;
                hcdc->pipe_bulk_out = 2U;
                (void)syn_port_usb_host_pipe_open(2U, dev_addr, ep_addr, SYN_USB_EP_TYPE_BULK,
                                                  max_pkt);
            }
        }
        pos = (uint16_t)(pos + b_len);
    }
}

/**
 * @brief Fallback scan of full configuration descriptor for a CDC Data (0x0A) interface.
 * @param hcdc     Pointer to USB Host CDC instance.
 * @param dev_addr Device bus address.
 */
static void host_cdc_scan_data_ifaces(SYN_USB_HostCDC *hcdc, uint8_t dev_addr)
{
    const uint8_t *buf = hcdc->host->enum_buf;
    uint16_t total = hcdc->host->enum_buf_len;
    uint16_t pos = 0U;
    bool in_cdc_data = false;

    while ((pos + 2U) <= total) {
        uint8_t b_len = buf[pos];
        uint8_t b_type = buf[pos + 1U];
        if ((b_len < 2U) || ((pos + b_len) > total)) {
            break;
        }
        if ((b_type == SYN_USB_DESC_TYPE_INTERFACE) && (b_len >= 9U)) {
            in_cdc_data = (buf[pos + 5U] == 0x0AU);
        } else if (in_cdc_data && (b_type == SYN_USB_DESC_TYPE_ENDPOINT) && (b_len >= 7U)) {
            host_cdc_scan_endpoints(hcdc, dev_addr, &buf[pos], b_len);
        }
        pos = (uint16_t)(pos + b_len);
    }
}

/**
 * @brief Host class probe callback for CDC ACM interfaces.
 * @param ctx Context pointer (SYN_USB_HostCDC).
 * @param dev_addr Device bus address.
 * @param iface_desc Pointer to interface descriptor payload.
 * @param len Descriptor byte length.
 * @return SYN_OK on successful claim, SYN_ERROR if bulk pipes are missing.
 */
static SYN_Status host_cdc_probe(void *ctx, uint8_t dev_addr, const uint8_t *iface_desc,
                                 uint16_t len)
{
    SYN_USB_HostCDC *hcdc = (SYN_USB_HostCDC *)ctx;
    if ((hcdc == NULL) || (iface_desc == NULL) || (len < 9U)) {
        return SYN_INVALID_PARAM;
    }

    hcdc->dev_addr = dev_addr;
    hcdc->ep_bulk_in = 0U;
    hcdc->ep_bulk_out = 0U;
    hcdc->pipe_bulk_in = 0U;
    hcdc->pipe_bulk_out = 0U;
    hcdc->connected = false;

    host_cdc_scan_endpoints(hcdc, dev_addr, iface_desc, len);
    if (((hcdc->pipe_bulk_in == 0U) || (hcdc->pipe_bulk_out == 0U)) && (hcdc->host != NULL)) {
        host_cdc_scan_data_ifaces(hcdc, dev_addr);
    }

    if ((hcdc->pipe_bulk_in == 0U) || (hcdc->pipe_bulk_out == 0U)) {
        if (hcdc->pipe_bulk_in != 0U) {
            (void)syn_port_usb_host_pipe_close(hcdc->pipe_bulk_in);
            hcdc->pipe_bulk_in = 0U;
        }
        if (hcdc->pipe_bulk_out != 0U) {
            (void)syn_port_usb_host_pipe_close(hcdc->pipe_bulk_out);
            hcdc->pipe_bulk_out = 0U;
        }
        return SYN_ERROR;
    }

    hcdc->connected = true;
    (void)syn_port_usb_host_submit_data(hcdc->pipe_bulk_in, hcdc->rx_buf,
                                        (uint16_t)sizeof(hcdc->rx_buf), true);
    return SYN_OK;
}

/**
 * @brief Host class disconnected callback.
 * @param ctx Context pointer (SYN_USB_HostCDC).
 */
static void host_cdc_disconnected(void *ctx)
{
    SYN_USB_HostCDC *hcdc = (SYN_USB_HostCDC *)ctx;
    if (hcdc != NULL) {
        if (hcdc->pipe_bulk_in != 0U) {
            (void)syn_port_usb_host_pipe_close(hcdc->pipe_bulk_in);
            hcdc->pipe_bulk_in = 0U;
        }
        if (hcdc->pipe_bulk_out != 0U) {
            (void)syn_port_usb_host_pipe_close(hcdc->pipe_bulk_out);
            hcdc->pipe_bulk_out = 0U;
        }
        hcdc->connected = false;
        hcdc->rx_len = 0U;
        hcdc->tx_len = 0U;
    }
}

/**
 * @brief Poll Bulk IN pipe and refill RX buffer when empty.
 * @param hcdc Pointer to USB Host CDC instance.
 */
static void host_cdc_process_rx(SYN_USB_HostCDC *hcdc)
{
    if ((hcdc->rx_len != 0U) || (hcdc->pipe_bulk_in == 0U) ||
        !syn_port_usb_host_xfer_done(hcdc->pipe_bulk_in)) {
        return;
    }

    uint16_t read_bytes = 0U;
    if ((syn_port_usb_host_xfer_result(hcdc->pipe_bulk_in, &read_bytes) == SYN_OK) &&
        (read_bytes > 0U)) {
        hcdc->rx_len =
            (read_bytes < sizeof(hcdc->rx_buf)) ? read_bytes : (uint16_t)sizeof(hcdc->rx_buf);
    } else {
        (void)syn_port_usb_host_submit_data(hcdc->pipe_bulk_in, hcdc->rx_buf,
                                            (uint16_t)sizeof(hcdc->rx_buf), true);
    }
}

/**
 * @brief Host class process/tick callback.
 * @param ctx Context pointer (SYN_USB_HostCDC).
 */
static void host_cdc_process(void *ctx)
{
    SYN_USB_HostCDC *hcdc = (SYN_USB_HostCDC *)ctx;
    if ((hcdc == NULL) || !hcdc->connected) {
        return;
    }

    if ((hcdc->tx_len > 0U) && (hcdc->pipe_bulk_out != 0U) &&
        syn_port_usb_host_xfer_done(hcdc->pipe_bulk_out)) {
        (void)syn_port_usb_host_submit_data(hcdc->pipe_bulk_out, hcdc->tx_buf, hcdc->tx_len, false);
        hcdc->tx_len = 0U;
    }

    host_cdc_process_rx(hcdc);
}

SYN_Status syn_usb_host_cdc_init(SYN_USB_HostCDC *hcdc)
{
    if (hcdc == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(hcdc, 0, sizeof(*hcdc));
    hcdc->line_coding.baudrate = 115200U;
    hcdc->line_coding.stop_bits = 0U;
    hcdc->line_coding.parity = 0U;
    hcdc->line_coding.data_bits = 8U;
    return SYN_OK;
}

SYN_Status syn_usb_host_cdc_register(SYN_USB_Host *host, SYN_USB_HostCDC *hcdc)
{
    if ((host == NULL) || (hcdc == NULL)) {
        return SYN_INVALID_PARAM;
    }

    hcdc->host = host;

    SYN_USB_HostClassDriver cls;
    memset(&cls, 0, sizeof(cls));
    cls.class_code = 0x02U;
    cls.subclass_code = 0xFFU;
    cls.protocol_code = 0xFFU;
    cls.ctx = hcdc;
    cls.probe = host_cdc_probe;
    cls.disconnected = host_cdc_disconnected;
    cls.process = host_cdc_process;

    return syn_usb_host_register_class(host, &cls);
}

SYN_Status syn_usb_host_cdc_write(SYN_USB_HostCDC *hcdc, const void *data, size_t len)
{
    if ((hcdc == NULL) || (data == NULL) || (len == 0U)) {
        return SYN_INVALID_PARAM;
    }

    if (!syn_usb_host_cdc_tx_ready(hcdc)) {
        return SYN_BUSY;
    }

    if (len > sizeof(hcdc->tx_buf)) {
        len = sizeof(hcdc->tx_buf);
    }

    memcpy(hcdc->tx_buf, data, len);
    hcdc->tx_len = (uint16_t)len;

    return SYN_OK;
}

SYN_Status syn_usb_host_cdc_read(SYN_USB_HostCDC *hcdc, void *buf, size_t max_len, size_t *out_len)
{
    if ((hcdc == NULL) || (buf == NULL) || (out_len == NULL)) {
        return SYN_INVALID_PARAM;
    }

    uint16_t available = hcdc->rx_len;
    if (available == 0U) {
        *out_len = 0U;
        return SYN_OK;
    }

    if (max_len < available) {
        available = (uint16_t)max_len;
    }

    memcpy(buf, hcdc->rx_buf, available);
    *out_len = available;

    if (available < hcdc->rx_len) {
        memmove(hcdc->rx_buf, &hcdc->rx_buf[available], (size_t)(hcdc->rx_len - available));
        hcdc->rx_len = (uint16_t)(hcdc->rx_len - available);
    } else {
        hcdc->rx_len = 0U;
    }

    return SYN_OK;
}

bool syn_usb_host_cdc_rx_available(const SYN_USB_HostCDC *hcdc)
{
    return (hcdc != NULL) && (hcdc->rx_len > 0U);
}

bool syn_usb_host_cdc_tx_ready(const SYN_USB_HostCDC *hcdc)
{
    if ((hcdc == NULL) || (hcdc->tx_len != 0U)) {
        return false;
    }
    return (hcdc->pipe_bulk_out == 0U) || syn_port_usb_host_xfer_done(hcdc->pipe_bulk_out);
}
