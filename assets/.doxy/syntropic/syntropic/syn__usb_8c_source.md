

# File syn\_usb.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb.c**](syn__usb_8c.md)

[Go to the documentation of this file](syn__usb_8c.md)


```C++

#include "syntropic/drivers/syn_usb.h"

#include <string.h>

static const uint8_t CONFIG_HEADER_TEMPLATE[9] = {
    0x09,       /* bLength */
    0x02,       /* bDescriptorType = Configuration */
    0x09, 0x00, /* wTotalLength (updated dynamically) */
    0x00,       /* bNumInterfaces (updated dynamically) */
    0x01,       /* bConfigurationValue = 1 */
    0x00,       /* iConfiguration = 0 */
    0xC0,       /* bmAttributes = Self-Powered */
    0x32        /* bMaxPower = 100mA */
};

SYN_Status syn_usb_init(SYN_USB_Device *dev, const uint8_t *device_desc)
{
    if (!dev || !device_desc) {
        return SYN_INVALID_PARAM;
    }

    memset(dev, 0, sizeof(*dev));
    dev->state = SYN_USB_STATE_DEFAULT;
    dev->device_desc = device_desc;

    /* Initialize auto-assembly configuration buffer with header template */
    memcpy(dev->config_buf, CONFIG_HEADER_TEMPLATE, sizeof(CONFIG_HEADER_TEMPLATE));
    dev->config_buf_used = sizeof(CONFIG_HEADER_TEMPLATE);
    dev->config_desc = dev->config_buf;
    dev->config_desc_len = dev->config_buf_used;

    return SYN_OK;
}

SYN_Status syn_usb_register_class(SYN_USB_Device *dev, const SYN_USB_ClassDriver *cls,
                                  const uint8_t *iface_desc, uint16_t iface_desc_len)
{
    if (!dev || !cls) {
        return SYN_INVALID_PARAM;
    }

    if (dev->class_count >= SYN_USB_MAX_CLASSES) {
        return SYN_ERROR;
    }

    dev->classes[dev->class_count] = *cls;
    dev->class_count++;

    if (iface_desc && iface_desc_len > 0 && !dev->raw_config_override) {
        if (dev->config_buf_used + iface_desc_len > SYN_USB_MAX_CONFIG_DESC) {
            return SYN_ERROR;
        }

        memcpy(&dev->config_buf[dev->config_buf_used], iface_desc, iface_desc_len);
        dev->config_buf_used += iface_desc_len;

        /* Update Total Length (bytes 2..3) and Num Interfaces (byte 4) in header */
        dev->config_buf[2] = (uint8_t)(dev->config_buf_used & 0xFF);
        dev->config_buf[3] = (uint8_t)((dev->config_buf_used >> 8) & 0xFF);
        dev->config_buf[4] += cls->iface_count;

        dev->config_desc_len = dev->config_buf_used;
    }

    return SYN_OK;
}

SYN_Status syn_usb_set_string_desc(SYN_USB_Device *dev, uint8_t index, const uint8_t *desc)
{
    if (!dev || !desc) {
        return SYN_INVALID_PARAM;
    }

    if (index >= SYN_USB_MAX_STRING_DESCS) {
        return SYN_INVALID_PARAM;
    }

    dev->string_descs[index] = desc;
    if (index >= dev->string_desc_count) {
        dev->string_desc_count = index + 1U;
    }

    return SYN_OK;
}

SYN_Status syn_usb_set_raw_config_desc(SYN_USB_Device *dev, const uint8_t *desc, uint16_t len)
{
    if (!dev || !desc || len < 9U) {
        return SYN_INVALID_PARAM;
    }

    dev->config_desc = desc;
    dev->config_desc_len = len;
    dev->raw_config_override = true;

    return SYN_OK;
}

SYN_Status syn_usb_process_setup(SYN_USB_Device *dev, const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                                 uint16_t *rlen)
{
    if (!dev || !pkt || !resp || !rlen) {
        return SYN_INVALID_PARAM;
    }

    *rlen = 0;
    uint8_t req_type = pkt->bmRequestType & 0x60U;  /* Bits 6..5: Type (0=Standard, 1=Class) */
    uint8_t recipient = pkt->bmRequestType & 0x1FU; /* Bits 4..0: Recipient */

    /* Handle Class / Interface Specific Requests */
    if (req_type == 0x20U || recipient == 0x01U) {
        uint8_t iface = (uint8_t)(pkt->wIndex & 0xFF);
        for (uint8_t i = 0U; i < dev->class_count; i++) {
            SYN_USB_ClassDriver *cls = &dev->classes[i];
            if (iface >= cls->iface_start && iface < (cls->iface_start + cls->iface_count)) {
                if (cls->setup) {
                    return cls->setup(cls->ctx, pkt, resp, rlen);
                }
            }
        }
    }

    /* Standard Device Requests */
    switch (pkt->bRequest) {
    case SYN_USB_REQ_SET_ADDRESS:
        dev->dev_address = (uint8_t)(pkt->wValue & 0x7FU);
        if (dev->dev_address != 0U) {
            dev->state = SYN_USB_STATE_ADDRESS;
        } else {
            dev->state = SYN_USB_STATE_DEFAULT;
        }
        return SYN_OK;

    case SYN_USB_REQ_SET_CONFIGURATION:
        dev->config_value = (uint8_t)(pkt->wValue & 0xFFU);
        if (dev->config_value != 0U) {
            dev->state = SYN_USB_STATE_CONFIGURED;
        } else {
            dev->state = SYN_USB_STATE_ADDRESS;
        }
        for (uint8_t i = 0U; i < dev->class_count; i++) {
            if (dev->classes[i].configured) {
                dev->classes[i].configured(dev->classes[i].ctx, dev->config_value);
            }
        }
        return SYN_OK;

    case SYN_USB_REQ_GET_CONFIGURATION:
        resp[0] = dev->config_value;
        *rlen = 1U;
        return SYN_OK;

    case SYN_USB_REQ_GET_STATUS:
        resp[0] = 0x01U; /* Self-powered */
        resp[1] = 0x00U;
        *rlen = 2U;
        return SYN_OK;

    case SYN_USB_REQ_GET_DESCRIPTOR: {
        uint8_t desc_type = (uint8_t)(pkt->wValue >> 8);
        uint8_t desc_idx = (uint8_t)(pkt->wValue & 0xFF);

        if (desc_type == SYN_USB_DESC_TYPE_DEVICE) {
            uint16_t len = (pkt->wLength < 18U) ? pkt->wLength : 18U;
            memcpy(resp, dev->device_desc, len);
            *rlen = len;
            return SYN_OK;
        } else if (desc_type == SYN_USB_DESC_TYPE_CONFIGURATION) {
            uint16_t len =
                (pkt->wLength < dev->config_desc_len) ? pkt->wLength : dev->config_desc_len;
            memcpy(resp, dev->config_desc, len);
            *rlen = len;
            return SYN_OK;
        } else if (desc_type == SYN_USB_DESC_TYPE_STRING) {
            if (desc_idx < SYN_USB_MAX_STRING_DESCS && dev->string_descs[desc_idx]) {
                const uint8_t *sdesc = dev->string_descs[desc_idx];
                uint16_t slen = sdesc[0];
                if (pkt->wLength < slen) {
                    slen = pkt->wLength;
                }
                memcpy(resp, sdesc, slen);
                *rlen = slen;
                return SYN_OK;
            }
        }
        return SYN_ERROR;
    }

    default:
        return SYN_OK;
    }
}

bool syn_usb_is_configured(const SYN_USB_Device *dev)
{
    return (dev != NULL) && (dev->state == SYN_USB_STATE_CONFIGURED);
}
```


