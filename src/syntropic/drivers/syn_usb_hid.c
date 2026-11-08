/**
 * @file syn_usb_hid.c
 * @brief Zero-Heap USB 2.0 Human Interface Device (HID) Class Driver Implementation.
 */

#include "syntropic/drivers/syn_usb_hid.h"

#include <string.h>

/** Standard HID Interface Descriptor Template (1 interface, 1 IN EP, 25 bytes) */
static const uint8_t HID_INTERFACE_DESC[25] = {
    /* Interface 0: HID Class */
    0x09U, 0x04U, 0x00U, 0x00U, 0x01U, 0x03U, 0x00U, 0x00U, 0x00U,
    /* HID Descriptor */
    0x09U, 0x21U, 0x11U, 0x01U, 0x00U, 0x01U, 0x22U, 0x34U, 0x00U,
    /* Endpoint 0x83: Interrupt IN */
    0x07U, 0x05U, 0x83U, 0x03U, 0x40U, 0x00U, 0x0AU};

/**
 * @brief Handle HID GET_DESCRIPTOR setup requests (0x22 Report Descriptor).
 *
 * @param hid  Pointer to SYN_USB_HID instance.
 * @param pkt  Setup packet pointer.
 * @param resp Response buffer.
 * @param rlen Output response length pointer.
 * @return SYN_OK on success, SYN_ERROR if unsupported.
 */
static SYN_Status hid_handle_get_descriptor(const SYN_USB_HID *hid, const SYN_USB_SetupPacket *pkt,
                                            uint8_t *resp, uint16_t *rlen)
{
    if (((uint8_t)(pkt->wValue >> 8) == 0x22U) && (hid->report_desc != NULL)) {
        uint16_t len = (pkt->wLength < hid->report_desc_len) ? pkt->wLength : hid->report_desc_len;
        memcpy(resp, hid->report_desc, len);
        *rlen = len;
        return SYN_OK;
    }
    return SYN_ERROR;
}

/**
 * @brief Class setup request dispatcher for HID class requests.
 *
 * @param ctx Pointer to SYN_USB_HID instance.
 * @param pkt Pointer to received Setup Packet.
 * @param resp Response buffer.
 * @param rlen Output pointer to receive response byte length.
 * @return SYN_OK on success.
 */
static SYN_Status hid_class_setup(void *ctx, const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                                  uint16_t *rlen)
{
    SYN_USB_HID *hid = (SYN_USB_HID *)ctx;
    if ((hid == NULL) || (pkt == NULL) || (resp == NULL) || (rlen == NULL)) {
        return SYN_INVALID_PARAM;
    }

    *rlen = 0U;

    switch (pkt->bRequest) {
    case SYN_USB_HID_REQ_GET_REPORT:
        if (hid->tx_len > 0U) {
            uint16_t copy_len = (pkt->wLength < hid->tx_len) ? pkt->wLength : hid->tx_len;
            memcpy(resp, hid->tx_buf, copy_len);
            *rlen = copy_len;
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

    case SYN_USB_REQ_GET_DESCRIPTOR:
        return hid_handle_get_descriptor(hid, pkt, resp, rlen);

    default:
        return SYN_OK;
    }
}

/**
 * @brief Data IN completion callback for HID Interrupt IN endpoint.
 *
 * @param ctx Pointer to SYN_USB_HID instance.
 * @param ep  Endpoint address.
 */
static void hid_class_data_in(void *ctx, uint8_t ep)
{
    SYN_USB_HID *hid = (SYN_USB_HID *)ctx;
    if ((hid != NULL) && (ep == hid->ep_in)) {
        hid->tx_len = 0U;
    }
}

SYN_Status syn_usb_hid_init(SYN_USB_HID *hid)
{
    if (hid == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(hid, 0, sizeof(*hid));
    hid->ep_in = 0x83U;
    hid->idle_rate = 0U;
    hid->active_protocol = 1U;
    return SYN_OK;
}

SYN_Status syn_usb_hid_register(SYN_USB_Device *dev, SYN_USB_HID *hid, const uint8_t *report_desc,
                                uint16_t report_desc_len)
{
    if ((dev == NULL) || (hid == NULL) || (report_desc == NULL) || (report_desc_len == 0U)) {
        return SYN_INVALID_PARAM;
    }

    hid->report_desc = report_desc;
    hid->report_desc_len = report_desc_len;

    uint8_t desc_buf[sizeof(HID_INTERFACE_DESC)];
    memcpy(desc_buf, HID_INTERFACE_DESC, sizeof(HID_INTERFACE_DESC));
    desc_buf[16] = (uint8_t)(report_desc_len & 0xFFU);
    desc_buf[17] = (uint8_t)((report_desc_len >> 8) & 0xFFU);
    desc_buf[20] = hid->ep_in;

    SYN_USB_ClassDriver cls;
    memset(&cls, 0, sizeof(cls));
    cls.iface_start = hid->iface_num;
    cls.iface_count = 1U;
    cls.ctx = hid;
    cls.setup = hid_class_setup;
    cls.data_in = hid_class_data_in;

    SYN_Status st = syn_usb_register_class(dev, &cls, desc_buf, (uint16_t)sizeof(desc_buf));
    if (st == SYN_OK) {
        hid->iface_num = dev->classes[dev->class_count - 1U].iface_start;
        if (!dev->raw_config_override) {
            uint16_t base = (uint16_t)(dev->config_buf_used - sizeof(desc_buf));
            hid->ep_in = dev->config_buf[base + 20U];
        }
    }
    return st;
}

SYN_Status syn_usb_hid_send_report(SYN_USB_HID *hid, const void *report, size_t len)
{
    if ((hid == NULL) || (report == NULL) || (len == 0U)) {
        return SYN_INVALID_PARAM;
    }

    if (hid->tx_len > 0U) {
        return SYN_BUSY;
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
    if ((hid == NULL) || (buf == NULL) || (out_len == NULL)) {
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
        memmove(hid->rx_buf, &hid->rx_buf[copy_len], (size_t)hid->rx_len - copy_len);
        hid->rx_len = (uint16_t)(hid->rx_len - (uint16_t)copy_len);
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
