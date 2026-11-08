/**
 * @file syn_usb.c
 * @brief Zero-Heap Modular USB 2.0 Device Core Protocol Engine Implementation.
 */

#include "syntropic/drivers/syn_usb.h"

#include <string.h>

/** Standard Configuration Descriptor Header Template (9 bytes) */
static const uint8_t CONFIG_HEADER_TEMPLATE[9] = {
    0x09U,        /* bLength */
    0x02U,        /* bDescriptorType = Configuration */
    0x09U, 0x00U, /* wTotalLength (updated dynamically) */
    0x00U,        /* bNumInterfaces (updated dynamically) */
    0x01U,        /* bConfigurationValue = 1 */
    0x00U,        /* iConfiguration = 0 */
    0xC0U,        /* bmAttributes = Self-Powered */
    0x32U         /* bMaxPower = 100mA */
};

/**
 * @brief Patch class-specific functional descriptors referencing interface indices.
 * @param d         Pointer to CS_INTERFACE descriptor.
 * @param b_len     Descriptor byte length.
 * @param cur_class Active parent interface bInterfaceClass.
 * @param delta     Interface number offset from template base.
 */
static void usb_patch_cs_iface(uint8_t *d, uint8_t b_len, uint8_t cur_class, int16_t delta)
{
    if ((cur_class == 0x02U) && (b_len >= 5U)) {
        if (d[2] == 0x01U) {
            d[4] = (uint8_t)((int16_t)d[4] + delta);
        } else if (d[2] == 0x06U) {
            d[3] = (uint8_t)((int16_t)d[3] + delta);
            d[4] = (uint8_t)((int16_t)d[4] + delta);
        }
    } else if ((cur_class == 0x01U) && (b_len >= 9U) && (d[2] == 0x01U)) {
        d[8] = (uint8_t)((int16_t)d[8] + delta);
    }
}

/**
 * @brief Patch a single descriptor entry for dynamic interface and endpoint assignment.
 * @param dev        Pointer to USB device context.
 * @param d          Pointer to descriptor start.
 * @param b_len      Descriptor byte length.
 * @param delta      Interface offset from template base.
 * @param cur_class  In/out active interface class code.
 * @return SYN_OK on success, SYN_ERROR if endpoints are exhausted.
 */
static SYN_Status usb_patch_single_desc(SYN_USB_Device *dev, uint8_t *d, uint8_t b_len,
                                        int16_t delta, uint8_t *cur_class)
{
    uint8_t b_type = d[1];

    if ((b_type == 0x0BU) && (b_len >= 8U)) {
        d[2] = (uint8_t)((int16_t)d[2] + delta);
    } else if ((b_type == SYN_USB_DESC_TYPE_INTERFACE) && (b_len >= 9U)) {
        d[2] = (uint8_t)((int16_t)d[2] + delta);
        *cur_class = d[5];
    } else if (b_type == 0x24U) {
        usb_patch_cs_iface(d, b_len, *cur_class, delta);
    } else if ((b_type == SYN_USB_DESC_TYPE_ENDPOINT) && (b_len >= 7U)) {
        return syn_usb_alloc_ep(dev, d[2], &d[2]);
    }

    return SYN_OK;
}

/**
 * @brief Walk a class descriptor block and patch interface/endpoint numbers.
 * @param dev        Pointer to USB device context.
 * @param buf        Mutable descriptor slice inside config_buf.
 * @param len        Byte length of descriptor slice.
 * @param base_iface Assigned first interface number for this class.
 * @return SYN_OK on success, SYN_ERROR if endpoint allocation fails.
 */
static SYN_Status usb_patch_class_desc(SYN_USB_Device *dev, uint8_t *buf, uint16_t len,
                                       uint8_t base_iface)
{
    bool have_base = false;
    int16_t delta = 0;
    uint8_t cur_class = 0U;
    uint16_t pos = 0U;

    while ((pos + 2U) <= len) {
        uint8_t b_len = buf[pos];
        uint8_t b_type = buf[pos + 1U];
        if ((b_len < 2U) || ((pos + b_len) > len)) {
            break;
        }
        if (!have_base && (((b_type == 0x0BU) && (b_len >= 8U)) ||
                           ((b_type == SYN_USB_DESC_TYPE_INTERFACE) && (b_len >= 9U)))) {
            delta = (int16_t)base_iface - (int16_t)buf[pos + 2U];
            have_base = true;
        }
        if (usb_patch_single_desc(dev, &buf[pos], b_len, delta, &cur_class) != SYN_OK) {
            return SYN_ERROR;
        }
        pos = (uint16_t)(pos + b_len);
    }

    return SYN_OK;
}

/**
 * @brief Route class-specific or interface-targeted setup packets to matching driver.
 * @param dev     Pointer to USB device context.
 * @param pkt     Setup packet pointer.
 * @param resp    Response buffer.
 * @param rlen    Output response length pointer.
 * @param handled Output flag set true when a matching class setup handler handled pkt.
 * @return Status returned by the matched class handler.
 */
static SYN_Status usb_route_class_setup(SYN_USB_Device *dev, const SYN_USB_SetupPacket *pkt,
                                        uint8_t *resp, uint16_t *rlen, bool *handled)
{
    uint8_t iface = (uint8_t)(pkt->wIndex & 0xFFU);
    *handled = false;

    for (uint8_t i = 0U; i < dev->class_count; i++) {
        SYN_USB_ClassDriver *cls = &dev->classes[i];
        if ((iface >= cls->iface_start) &&
            (iface < (uint8_t)(cls->iface_start + cls->iface_count)) && (cls->setup != NULL)) {
            *handled = true;
            return cls->setup(cls->ctx, pkt, resp, rlen);
        }
    }

    return SYN_OK;
}

/**
 * @brief Handle standard USB GET_DESCRIPTOR setup requests on EP0.
 * @param dev  Pointer to USB device context.
 * @param pkt  Setup packet pointer.
 * @param resp Response buffer.
 * @param rlen Output response length pointer.
 * @return SYN_OK on success, SYN_ERROR if descriptor is unsupported/missing.
 */
static SYN_Status usb_handle_get_descriptor(const SYN_USB_Device *dev,
                                            const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                                            uint16_t *rlen)
{
    uint8_t desc_type = (uint8_t)(pkt->wValue >> 8);
    uint8_t desc_idx = (uint8_t)(pkt->wValue & 0xFFU);

    if (desc_type == SYN_USB_DESC_TYPE_DEVICE) {
        uint16_t len = (pkt->wLength < 18U) ? pkt->wLength : 18U;
        memcpy(resp, dev->device_desc, len);
        *rlen = len;
        return SYN_OK;
    }

    if (desc_type == SYN_USB_DESC_TYPE_CONFIGURATION) {
        uint16_t len = (pkt->wLength < dev->config_desc_len) ? pkt->wLength : dev->config_desc_len;
        memcpy(resp, dev->config_desc, len);
        *rlen = len;
        return SYN_OK;
    }

    if ((desc_type == SYN_USB_DESC_TYPE_STRING) && (desc_idx < SYN_USB_MAX_STRING_DESCS) &&
        (dev->string_descs[desc_idx] != NULL)) {
        const uint8_t *sdesc = dev->string_descs[desc_idx];
        uint16_t slen = (pkt->wLength < sdesc[0]) ? pkt->wLength : (uint16_t)sdesc[0];
        memcpy(resp, sdesc, slen);
        *rlen = slen;
        return SYN_OK;
    }

    return SYN_ERROR;
}

SYN_Status syn_usb_init(SYN_USB_Device *dev, const uint8_t *device_desc)
{
    if ((dev == NULL) || (device_desc == NULL)) {
        return SYN_INVALID_PARAM;
    }

    memset(dev, 0, sizeof(*dev));
    dev->state = SYN_USB_STATE_DEFAULT;
    dev->device_desc = device_desc;

    memcpy(dev->config_buf, CONFIG_HEADER_TEMPLATE, sizeof(CONFIG_HEADER_TEMPLATE));
    dev->config_buf_used = (uint16_t)sizeof(CONFIG_HEADER_TEMPLATE);
    dev->config_desc = dev->config_buf;
    dev->config_desc_len = dev->config_buf_used;

    return SYN_OK;
}

SYN_Status syn_usb_alloc_ep(SYN_USB_Device *dev, uint8_t preferred_ep, uint8_t *assigned_ep)
{
    if ((dev == NULL) || (assigned_ep == NULL)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t dir = (uint8_t)(preferred_ep & 0x80U);
    uint8_t num = (uint8_t)(preferred_ep & 0x0FU);
    uint16_t *mask = (dir != 0U) ? &dev->ep_in_mask : &dev->ep_out_mask;

    if ((num >= 1U) && (num <= 15U) && ((*mask & (uint16_t)(1U << num)) == 0U)) {
        *mask |= (uint16_t)(1U << num);
        *assigned_ep = (uint8_t)(dir | num);
        return SYN_OK;
    }

    for (uint8_t n = 1U; n <= 15U; n++) {
        if ((*mask & (uint16_t)(1U << n)) == 0U) {
            *mask |= (uint16_t)(1U << n);
            *assigned_ep = (uint8_t)(dir | n);
            return SYN_OK;
        }
    }

    return SYN_ERROR;
}

SYN_Status syn_usb_register_class(SYN_USB_Device *dev, const SYN_USB_ClassDriver *cls,
                                  const uint8_t *iface_desc, uint16_t iface_desc_len)
{
    if ((dev == NULL) || (cls == NULL)) {
        return SYN_INVALID_PARAM;
    }

    if (dev->class_count >= SYN_USB_MAX_CLASSES) {
        return SYN_ERROR;
    }

    uint8_t base_iface =
        (cls->iface_start > dev->iface_count) ? cls->iface_start : dev->iface_count;

    if ((iface_desc != NULL) && (iface_desc_len > 0U) && !dev->raw_config_override) {
        if (((uint32_t)dev->config_buf_used + (uint32_t)iface_desc_len) > SYN_USB_MAX_CONFIG_DESC) {
            return SYN_ERROR;
        }

        uint16_t saved_in = dev->ep_in_mask;
        uint16_t saved_out = dev->ep_out_mask;
        uint8_t *dst = &dev->config_buf[dev->config_buf_used];
        memcpy(dst, iface_desc, iface_desc_len);

        if (usb_patch_class_desc(dev, dst, iface_desc_len, base_iface) != SYN_OK) {
            dev->ep_in_mask = saved_in;
            dev->ep_out_mask = saved_out;
            return SYN_ERROR;
        }

        dev->config_buf_used = (uint16_t)(dev->config_buf_used + iface_desc_len);
        dev->config_buf[2] = (uint8_t)(dev->config_buf_used & 0xFFU);
        dev->config_buf[3] = (uint8_t)((dev->config_buf_used >> 8) & 0xFFU);
        dev->config_buf[4] = (uint8_t)(dev->config_buf[4] + cls->iface_count);
        dev->config_desc_len = dev->config_buf_used;
    }

    dev->classes[dev->class_count] = *cls;
    dev->classes[dev->class_count].iface_start = base_iface;
    dev->class_count++;
    dev->iface_count = (uint8_t)(base_iface + cls->iface_count);

    return SYN_OK;
}

SYN_Status syn_usb_set_string_desc(SYN_USB_Device *dev, uint8_t index, const uint8_t *desc)
{
    if ((dev == NULL) || (desc == NULL) || (index >= SYN_USB_MAX_STRING_DESCS)) {
        return SYN_INVALID_PARAM;
    }

    dev->string_descs[index] = desc;
    if (index >= dev->string_desc_count) {
        dev->string_desc_count = (uint8_t)(index + 1U);
    }

    return SYN_OK;
}

SYN_Status syn_usb_set_raw_config_desc(SYN_USB_Device *dev, const uint8_t *desc, uint16_t len)
{
    if ((dev == NULL) || (desc == NULL) || (len < 9U)) {
        return SYN_INVALID_PARAM;
    }

    dev->config_desc = desc;
    dev->config_desc_len = len;
    dev->raw_config_override = true;

    return SYN_OK;
}

/**
 * @brief Check whether an EP0 Setup packet should be dispatched to a registered class driver.
 *
 * Routes Class-type requests (bmRequestType[6:5] == 1) and Standard Interface-targeted
 * GET_DESCRIPTOR requests (e.g. HID Report Descriptor 0x22 on bmRequestType == 0x81).
 * Standard Device/Interface requests such as SET_ADDRESS, SET_CONFIGURATION, and GET_STATUS
 * stay in the USB Chapter 9 core.
 *
 * @param pkt Setup packet pointer.
 * @return true when class-level routing applies.
 */
static bool usb_is_class_routed_setup(const SYN_USB_SetupPacket *pkt)
{
    uint8_t req_type = (uint8_t)(pkt->bmRequestType & 0x60U);
    uint8_t recipient = (uint8_t)(pkt->bmRequestType & 0x1FU);
    if (req_type == 0x20U) {
        return true;
    }
    return (req_type == 0x00U) && (recipient == 0x01U) &&
           (pkt->bRequest == SYN_USB_REQ_GET_DESCRIPTOR);
}

/**
 * @brief Apply SET_CONFIGURATION state transition and notify registered class drivers.
 * @param dev        Pointer to USB device context.
 * @param config_val Configuration value from wValue[7:0].
 */
static void usb_apply_configuration(SYN_USB_Device *dev, uint8_t config_val)
{
    dev->config_value = config_val;
    dev->state = (config_val != 0U) ? SYN_USB_STATE_CONFIGURED : SYN_USB_STATE_ADDRESS;
    for (uint8_t i = 0U; i < dev->class_count; i++) {
        if (dev->classes[i].configured != NULL) {
            (void)dev->classes[i].configured(dev->classes[i].ctx, config_val);
        }
    }
}

SYN_Status syn_usb_process_setup(SYN_USB_Device *dev, const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                                 uint16_t *rlen)
{
    if ((dev == NULL) || (pkt == NULL) || (resp == NULL) || (rlen == NULL)) {
        return SYN_INVALID_PARAM;
    }

    *rlen = 0U;
    uint8_t recipient = (uint8_t)(pkt->bmRequestType & 0x1FU);

    if (usb_is_class_routed_setup(pkt)) {
        bool handled = false;
        SYN_Status cls_st = usb_route_class_setup(dev, pkt, resp, rlen, &handled);
        if (handled) {
            return cls_st;
        }
    }

    switch (pkt->bRequest) {
    case SYN_USB_REQ_SET_ADDRESS:
        dev->dev_address = (uint8_t)(pkt->wValue & 0x7FU);
        dev->state = (dev->dev_address != 0U) ? SYN_USB_STATE_ADDRESS : SYN_USB_STATE_DEFAULT;
        return SYN_OK;

    case SYN_USB_REQ_SET_CONFIGURATION:
        usb_apply_configuration(dev, (uint8_t)(pkt->wValue & 0xFFU));
        return SYN_OK;

    case SYN_USB_REQ_GET_CONFIGURATION:
        resp[0] = dev->config_value;
        *rlen = 1U;
        return SYN_OK;

    case SYN_USB_REQ_GET_STATUS:
        resp[0] = (recipient == 0x00U) ? 0x01U : 0x00U;
        resp[1] = 0x00U;
        *rlen = 2U;
        return SYN_OK;

    case SYN_USB_REQ_GET_DESCRIPTOR:
        return usb_handle_get_descriptor(dev, pkt, resp, rlen);

    default:
        return SYN_OK;
    }
}

bool syn_usb_is_configured(const SYN_USB_Device *dev)
{
    return (dev != NULL) && (dev->state == SYN_USB_STATE_CONFIGURED);
}
