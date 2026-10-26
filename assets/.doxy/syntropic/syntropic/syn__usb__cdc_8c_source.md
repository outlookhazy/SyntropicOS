

# File syn\_usb\_cdc.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_cdc.c**](syn__usb__cdc_8c.md)

[Go to the documentation of this file](syn__usb__cdc_8c.md)


```C++

#include "syntropic/drivers/syn_usb_cdc.h"

#include <string.h>

static const uint8_t CDC_INTERFACE_DESC[67] = {
    /* Interface Association Descriptor (IAD) */
    0x08, 0x0B, 0x00, 0x02, 0x02, 0x02, 0x01, 0x00,

    /* Interface 0: Communication Interface Class */
    0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00,
    /* Header Functional Descriptor */
    0x05, 0x24, 0x00, 0x10, 0x01,
    /* Call Management Functional Descriptor */
    0x05, 0x24, 0x01, 0x00, 0x01,
    /* ACM Functional Descriptor */
    0x04, 0x24, 0x02, 0x02,
    /* Union Functional Descriptor */
    0x05, 0x24, 0x06, 0x00, 0x01,
    /* Endpoint 0x82: Interrupt IN */
    0x07, 0x05, 0x82, 0x03, 0x08, 0x00, 0x10,

    /* Interface 1: Data Interface Class */
    0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x00,
    /* Endpoint 0x01: Bulk OUT */
    0x07, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00,
    /* Endpoint 0x81: Bulk IN */
    0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00};

static SYN_Status cdc_class_setup(void *ctx, const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                                  uint16_t *rlen)
{
    SYN_USB_CDC *cdc = (SYN_USB_CDC *)ctx;
    size_t size_rlen = 0;
    SYN_Status st = syn_usb_cdc_handle_setup(cdc, pkt, resp, &size_rlen);
    *rlen = (uint16_t)size_rlen;
    return st;
}

static SYN_Status cdc_class_configured(void *ctx, uint8_t config)
{
    SYN_USB_CDC *cdc = (SYN_USB_CDC *)ctx;
    if (cdc) {
        cdc->configured = (config != 0U);
    }
    return SYN_OK;
}

SYN_Status syn_usb_cdc_init(SYN_USB_CDC *cdc)
{
    if (!cdc) {
        return SYN_INVALID_PARAM;
    }
    memset(cdc, 0, sizeof(*cdc));

    cdc->ep_in = 0x81;
    cdc->ep_out = 0x01;
    cdc->ep_cmd = 0x82;
    cdc->configured = false;

    cdc->line_coding.baudrate = 115200;
    cdc->line_coding.stop_bits = 0; /* 1 Stop Bit */
    cdc->line_coding.parity = 0;    /* None */
    cdc->line_coding.data_bits = 8;

    return SYN_OK;
}

SYN_Status syn_usb_cdc_register(SYN_USB_Device *dev, SYN_USB_CDC *cdc)
{
    if (!dev || !cdc) {
        return SYN_INVALID_PARAM;
    }

    SYN_USB_ClassDriver cls;
    memset(&cls, 0, sizeof(cls));
    cls.iface_start = 0U;
    cls.iface_count = 2U;
    cls.ctx = cdc;
    cls.setup = cdc_class_setup;
    cls.configured = cdc_class_configured;

    return syn_usb_register_class(dev, &cls, CDC_INTERFACE_DESC, sizeof(CDC_INTERFACE_DESC));
}

SYN_Status syn_usb_cdc_handle_setup(SYN_USB_CDC *cdc, const SYN_USB_SetupPacket *setup,
                                    uint8_t *resp, size_t *rlen)
{
    if (!cdc || !setup || !resp || !rlen) {
        return SYN_INVALID_PARAM;
    }

    *rlen = 0;

    switch (setup->bRequest) {
    case SYN_USB_REQ_SET_ADDRESS:
        cdc->dev_address = (uint8_t)(setup->wValue & 0x7F);
        return SYN_OK;

    case SYN_USB_REQ_SET_CONFIGURATION:
        cdc->configured = (setup->wValue != 0);
        return SYN_OK;

    case SYN_USB_CDC_SET_LINE_CODING:
        /* Line coding data stage handler can update active parameters */
        return SYN_OK;

    case SYN_USB_CDC_GET_LINE_CODING:
        resp[0] = (uint8_t)(cdc->line_coding.baudrate & 0xFF);
        resp[1] = (uint8_t)((cdc->line_coding.baudrate >> 8) & 0xFF);
        resp[2] = (uint8_t)((cdc->line_coding.baudrate >> 16) & 0xFF);
        resp[3] = (uint8_t)((cdc->line_coding.baudrate >> 24) & 0xFF);
        resp[4] = cdc->line_coding.stop_bits;
        resp[5] = cdc->line_coding.parity;
        resp[6] = cdc->line_coding.data_bits;
        *rlen = 7;
        return SYN_OK;

    case SYN_USB_CDC_SET_CONTROL_LINE_STATE:
        return SYN_OK;

    default:
        return SYN_OK;
    }
}

SYN_Status syn_usb_cdc_write(SYN_USB_CDC *cdc, const void *data, size_t len)
{
    if (!cdc || !data || len == 0) {
        return SYN_INVALID_PARAM;
    }

    if (len > sizeof(cdc->tx_buf)) {
        len = sizeof(cdc->tx_buf);
    }

    memcpy(cdc->tx_buf, data, len);
    cdc->tx_len = (uint16_t)len;

    return SYN_OK;
}

SYN_Status syn_usb_cdc_read(SYN_USB_CDC *cdc, void *buf, size_t max_len, size_t *out_len)
{
    if (!cdc || !buf || !out_len) {
        return SYN_INVALID_PARAM;
    }

    uint16_t available = cdc->rx_len;
    if (available == 0) {
        *out_len = 0;
        return SYN_OK;
    }

    if (max_len < available) {
        available = (uint16_t)max_len;
    }

    memcpy(buf, cdc->rx_buf, available);
    *out_len = available;

    if (available < cdc->rx_len) {
        memmove(cdc->rx_buf, &cdc->rx_buf[available], cdc->rx_len - available);
        cdc->rx_len -= available;
    } else {
        cdc->rx_len = 0;
    }

    return SYN_OK;
}

bool syn_usb_cdc_rx_available(const SYN_USB_CDC *cdc)
{
    return (cdc != NULL) && (cdc->rx_len > 0U);
}

bool syn_usb_cdc_tx_ready(const SYN_USB_CDC *cdc)
{
    return (cdc != NULL) && (cdc->tx_len == 0U);
}
```


