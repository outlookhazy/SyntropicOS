

# File syn\_usb\_hid.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_hid.c**](syn__usb__hid_8c.md)

[Go to the documentation of this file](syn__usb__hid_8c.md)


```C++

#include "syntropic/drivers/syn_usb_hid.h"

#include <string.h>

static const uint8_t HID_INTERFACE_DESC[25] = {
    /* Interface 0: HID Class */
    0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00,
    /* HID Descriptor */
    0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22, 0x34, 0x00,
    /* Endpoint 0x83: Interrupt IN */
    0x07, 0x05, 0x83, 0x03, 0x40, 0x00, 0x0A};

static SYN_Status hid_class_setup(void *ctx, const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                                  uint16_t *rlen)
{
    SYN_USB_HID *hid = (SYN_USB_HID *)ctx;
    if (!hid || !pkt || !resp || !rlen) {
        return SYN_INVALID_PARAM;
    }

    *rlen = 0;

    switch (pkt->bRequest) {
    case SYN_USB_HID_REQ_GET_REPORT:
        if (hid->tx_len > 0U) {
            uint16_t copy_len = (pkt->wLength < hid->tx_len) ? pkt->wLength : hid->tx_len;
            memcpy(resp, hid->tx_buf, copy_len);
            *rlen = copy_len;
            return SYN_OK;
        }
        return SYN_OK;

    case SYN_USB_HID_REQ_SET_REPORT:
        return SYN_OK;

    case SYN_USB_HID_REQ_GET_IDLE:
        resp[0] = hid->idle_rate;
        *rlen = 1U;
        return SYN_OK;

    case SYN_USB_HID_REQ_SET_IDLE:
        hid->idle_rate = (uint8_t)(pkt->wValue >> 8);
        return SYN_OK;

    case SYN_USB_HID_REQ_GET_PROTOCOL:
        resp[0] = hid->active_protocol;
        *rlen = 1U;
        return SYN_OK;

    case SYN_USB_HID_REQ_SET_PROTOCOL:
        hid->active_protocol = (uint8_t)(pkt->wValue & 0xFFU);
        return SYN_OK;

    case SYN_USB_REQ_GET_DESCRIPTOR: {
        uint8_t desc_type = (uint8_t)(pkt->wValue >> 8);
        if (desc_type == 0x22U && hid->report_desc) { /* HID Report Descriptor */
            uint16_t len =
                (pkt->wLength < hid->report_desc_len) ? pkt->wLength : hid->report_desc_len;
            memcpy(resp, hid->report_desc, len);
            *rlen = len;
            return SYN_OK;
        }
        return SYN_ERROR;
    }

    default:
        return SYN_OK;
    }
}

SYN_Status syn_usb_hid_init(SYN_USB_HID *hid)
{
    if (!hid) {
        return SYN_INVALID_PARAM;
    }
    memset(hid, 0, sizeof(*hid));
    hid->ep_in = 0x83;
    hid->idle_rate = 0;
    hid->active_protocol = 1; /* 1 = Report Protocol */
    return SYN_OK;
}

SYN_Status syn_usb_hid_register(SYN_USB_Device *dev, SYN_USB_HID *hid, const uint8_t *report_desc,
                                uint16_t report_desc_len)
{
    if (!dev || !hid || !report_desc || report_desc_len == 0U) {
        return SYN_INVALID_PARAM;
    }

    hid->report_desc = report_desc;
    hid->report_desc_len = report_desc_len;

    /* Copy static interface desc template and dynamically update Report Desc Length (bytes 16..17)
     */
    uint8_t desc_buf[sizeof(HID_INTERFACE_DESC)];
    memcpy(desc_buf, HID_INTERFACE_DESC, sizeof(HID_INTERFACE_DESC));
    desc_buf[16] = (uint8_t)(report_desc_len & 0xFFU);
    desc_buf[17] = (uint8_t)((report_desc_len >> 8) & 0xFFU);

    SYN_USB_ClassDriver cls;
    memset(&cls, 0, sizeof(cls));
    cls.iface_start = dev->class_count; /* Interface number assignment */
    cls.iface_count = 1U;
    cls.ctx = hid;
    cls.setup = hid_class_setup;

    hid->iface_num = cls.iface_start;

    return syn_usb_register_class(dev, &cls, desc_buf, sizeof(desc_buf));
}

SYN_Status syn_usb_hid_send_report(SYN_USB_HID *hid, const void *report, size_t len)
{
    if (!hid || !report || len == 0U) {
        return SYN_INVALID_PARAM;
    }

    if (len > sizeof(hid->tx_buf)) {
        len = sizeof(hid->tx_buf);
    }

    memcpy(hid->tx_buf, report, len);
    hid->tx_len = (uint16_t)len;

    return SYN_OK;
}

SYN_Status syn_usb_hid_read_report(SYN_USB_HID *hid, void *buf, size_t max_len, size_t *out_len)
{
    if (!hid || !buf || !out_len) {
        return SYN_INVALID_PARAM;
    }

    if (hid->rx_len == 0U) {
        *out_len = 0U;
        return SYN_OK;
    }

    size_t copy_len = (hid->rx_len < max_len) ? hid->rx_len : max_len;
    memcpy(buf, hid->rx_buf, copy_len);
    *out_len = copy_len;

    if (copy_len < hid->rx_len) {
        memmove(hid->rx_buf, &hid->rx_buf[copy_len], hid->rx_len - copy_len);
        hid->rx_len -= (uint16_t)copy_len;
    } else {
        hid->rx_len = 0U;
    }

    return SYN_OK;
}

bool syn_usb_hid_report_available(const SYN_USB_HID *hid)
{
    return (hid != NULL) && (hid->rx_len > 0U);
}

bool syn_usb_hid_tx_ready(const SYN_USB_HID *hid)
{
    return (hid != NULL) && (hid->tx_len == 0U);
}
```


