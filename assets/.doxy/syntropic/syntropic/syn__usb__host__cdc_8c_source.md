

# File syn\_usb\_host\_cdc.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_host\_cdc.c**](syn__usb__host__cdc_8c.md)

[Go to the documentation of this file](syn__usb__host__cdc_8c.md)


```C++

#include "syntropic/drivers/syn_usb_host_cdc.h"

#include "port/syn_port_usb.h"
#include "port/syn_port_usb_host.h"

#include <string.h>

static SYN_Status host_cdc_probe(void *ctx, uint8_t dev_addr, const uint8_t *iface_desc,
                                 uint16_t len)
{
    SYN_USB_HostCDC *hcdc = (SYN_USB_HostCDC *)ctx;
    if (!hcdc || !iface_desc || len < 9U) {
        return SYN_INVALID_PARAM;
    }

    hcdc->dev_addr = dev_addr;

    /* Parse endpoints from subordinate descriptors */
    uint16_t pos = 0;
    while (pos + 2U <= len) {
        uint8_t bLen = iface_desc[pos];
        uint8_t bType = iface_desc[pos + 1U];
        if (bLen < 2U || (pos + bLen) > len) {
            break;
        }

        if (bType == SYN_USB_DESC_TYPE_ENDPOINT && bLen >= 7U) {
            uint8_t ep_addr = iface_desc[pos + 2U];
            uint8_t ep_attr = iface_desc[pos + 3U];
            uint16_t max_pkt =
                (uint16_t)iface_desc[pos + 4U] | ((uint16_t)iface_desc[pos + 5U] << 8);

            /* Bulk Endpoint */
            if ((ep_attr & 0x03U) == SYN_USB_EP_TYPE_BULK) {
                if ((ep_addr & 0x80U) != 0U) { /* Bulk IN */
                    hcdc->ep_bulk_in = ep_addr;
                    hcdc->pipe_bulk_in = 1U;
                    syn_port_usb_host_pipe_open(hcdc->pipe_bulk_in, dev_addr, ep_addr,
                                                SYN_USB_EP_TYPE_BULK, max_pkt);
                } else { /* Bulk OUT */
                    hcdc->ep_bulk_out = ep_addr;
                    hcdc->pipe_bulk_out = 2U;
                    syn_port_usb_host_pipe_open(hcdc->pipe_bulk_out, dev_addr, ep_addr,
                                                SYN_USB_EP_TYPE_BULK, max_pkt);
                }
            }
        }
        pos += bLen;
    }

    hcdc->connected = true;
    return SYN_OK;
}

static void host_cdc_disconnected(void *ctx)
{
    SYN_USB_HostCDC *hcdc = (SYN_USB_HostCDC *)ctx;
    if (hcdc) {
        if (hcdc->pipe_bulk_in != 0U) {
            syn_port_usb_host_pipe_close(hcdc->pipe_bulk_in);
        }
        if (hcdc->pipe_bulk_out != 0U) {
            syn_port_usb_host_pipe_close(hcdc->pipe_bulk_out);
        }
        hcdc->connected = false;
        hcdc->rx_len = 0;
        hcdc->tx_len = 0;
    }
}

static void host_cdc_process(void *ctx)
{
    SYN_USB_HostCDC *hcdc = (SYN_USB_HostCDC *)ctx;
    if (!hcdc || !hcdc->connected) {
        return;
    }

    /* Process pending TX transfers if any */
    if (hcdc->tx_len > 0U && hcdc->pipe_bulk_out != 0U) {
        if (!syn_port_usb_host_xfer_done(hcdc->pipe_bulk_out)) {
            /* Transfer ongoing */
        } else {
            /* Submit TX data */
            syn_port_usb_host_submit_data(hcdc->pipe_bulk_out, hcdc->tx_buf, hcdc->tx_len, false);
            hcdc->tx_len = 0;
        }
    }

    /* Process RX transfers if buffer has space */
    if (hcdc->rx_len == 0U && hcdc->pipe_bulk_in != 0U) {
        if (syn_port_usb_host_xfer_done(hcdc->pipe_bulk_in)) {
            uint16_t read_bytes = 0;
            if (syn_port_usb_host_xfer_result(hcdc->pipe_bulk_in, &read_bytes) == SYN_OK &&
                read_bytes > 0U) {
                hcdc->rx_len = (read_bytes < sizeof(hcdc->rx_buf)) ? read_bytes
                                                                   : (uint16_t)sizeof(hcdc->rx_buf);
            } else {
                /* Trigger non-blocking IN submit */
                syn_port_usb_host_submit_data(hcdc->pipe_bulk_in, hcdc->rx_buf,
                                              sizeof(hcdc->rx_buf), true);
            }
        }
    }
}

SYN_Status syn_usb_host_cdc_init(SYN_USB_HostCDC *hcdc)
{
    if (!hcdc) {
        return SYN_INVALID_PARAM;
    }
    memset(hcdc, 0, sizeof(*hcdc));
    hcdc->line_coding.baudrate = 115200;
    hcdc->line_coding.stop_bits = 0;
    hcdc->line_coding.parity = 0;
    hcdc->line_coding.data_bits = 8;
    return SYN_OK;
}

SYN_Status syn_usb_host_cdc_register(SYN_USB_Host *host, SYN_USB_HostCDC *hcdc)
{
    if (!host || !hcdc) {
        return SYN_INVALID_PARAM;
    }

    SYN_USB_HostClassDriver cls;
    memset(&cls, 0, sizeof(cls));
    cls.class_code = 0x02U;    /* CDC Communication Class (or Data class 0x0A) */
    cls.subclass_code = 0xFFU; /* Any subclass */
    cls.protocol_code = 0xFFU; /* Any protocol */
    cls.ctx = hcdc;
    cls.probe = host_cdc_probe;
    cls.disconnected = host_cdc_disconnected;
    cls.process = host_cdc_process;

    return syn_usb_host_register_class(host, &cls);
}

SYN_Status syn_usb_host_cdc_write(SYN_USB_HostCDC *hcdc, const void *data, size_t len)
{
    if (!hcdc || !data || len == 0U) {
        return SYN_INVALID_PARAM;
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
    if (!hcdc || !buf || !out_len) {
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
        memmove(hcdc->rx_buf, &hcdc->rx_buf[available], hcdc->rx_len - available);
        hcdc->rx_len -= available;
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
    return (hcdc != NULL) && (hcdc->tx_len == 0U);
}
```


