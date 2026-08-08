

# File syn\_usb\_host.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_host.c**](syn__usb__host_8c.md)

[Go to the documentation of this file](syn__usb__host_8c.md)


```C++

#include "syntropic/drivers/syn_usb_host.h"

#include "port/syn_port_usb.h"
#include "port/syn_port_usb_host.h"

#include <string.h>

/* ── Internal helpers ──────────────────────────────────────────────────── */

static void build_setup(SYN_USB_SetupPacket *pkt, uint8_t bmRequestType, uint8_t bRequest,
                        uint16_t wValue, uint16_t wIndex, uint16_t wLength)
{
    pkt->bmRequestType = bmRequestType;
    pkt->bRequest = bRequest;
    pkt->wValue = wValue;
    pkt->wIndex = wIndex;
    pkt->wLength = wLength;
}

static void probe_class_drivers(SYN_USB_Host *host)
{
    const uint8_t *buf = host->enum_buf;
    uint16_t total = host->enum_buf_len;
    uint16_t pos = 0;

    while (pos + 2U <= total) {
        uint8_t bLength = buf[pos];
        uint8_t bDescType = buf[pos + 1U];

        if (bLength < 2U || (pos + bLength) > total) {
            break;
        }

        /* Interface Descriptor (bDescriptorType == 0x04, bLength >= 9) */
        if (bDescType == SYN_USB_DESC_TYPE_INTERFACE && bLength >= 9U) {
            uint8_t iface_class = buf[pos + 5U];
            uint8_t iface_sub = buf[pos + 6U];
            uint8_t iface_proto = buf[pos + 7U];

            /* Calculate length of this interface + subordinate descriptors */
            uint16_t iface_start = pos;
            uint16_t next = pos + bLength;
            while (next + 2U <= total) {
                uint8_t nLen = buf[next];
                uint8_t nType = buf[next + 1U];
                if (nLen < 2U || nType == SYN_USB_DESC_TYPE_INTERFACE) {
                    break;
                }
                next += nLen;
            }
            uint16_t iface_total_len = next - iface_start;

            /* Match against registered class drivers */
            for (uint8_t i = 0; i < host->class_count; i++) {
                SYN_USB_HostClassDriver *cls = &host->classes[i];
                if (cls->matched) {
                    continue;
                }
                bool class_match = (cls->class_code == 0xFFU) || (cls->class_code == iface_class);
                bool sub_match = (cls->subclass_code == 0xFFU) || (cls->subclass_code == iface_sub);
                bool proto_match =
                    (cls->protocol_code == 0xFFU) || (cls->protocol_code == iface_proto);

                if (class_match && sub_match && proto_match && cls->probe) {
                    if (cls->probe(cls->ctx, host->dev_info.dev_addr, &buf[iface_start],
                                   iface_total_len) == SYN_OK) {
                        cls->matched = true;
                        break; /* One driver per interface */
                    }
                }
            }
        }

        pos += bLength;
    }
}

/* ── Enumeration state machine ─────────────────────────────────────────── */

static SYN_Status enumerate_step(SYN_USB_Host *host)
{
    SYN_USB_SetupPacket pkt;

    /* If a transfer is pending, poll for completion */
    if (host->xfer_pending) {
        if (!syn_port_usb_host_xfer_done(0U)) {
            return SYN_BUSY;
        }
        host->xfer_pending = false;

        uint16_t actual = 0;
        SYN_Status xfer_st = syn_port_usb_host_xfer_result(0U, &actual);
        if (xfer_st != SYN_OK) {
            return SYN_ERROR;
        }
        host->enum_buf_len = actual;
    }

    switch (host->enum_step) {
    case SYN_USB_HOST_ENUM_GET_DEV8:
        /* Open pipe 0 to address 0, max_pkt 8 (minimum guaranteed) */
        syn_port_usb_host_pipe_open(0U, 0U, 0x00U, SYN_USB_EP_TYPE_CONTROL, 8U);
        /* GET_DESCRIPTOR(Device, 8 bytes) */
        build_setup(&pkt, 0x80U, SYN_USB_REQ_GET_DESCRIPTOR,
                    (uint16_t)(SYN_USB_DESC_TYPE_DEVICE << 8), 0U, 8U);
        syn_port_usb_host_submit_setup(0U, &pkt);
        syn_port_usb_host_submit_data(0U, host->enum_buf, 8U, true);
        host->xfer_pending = true;
        host->enum_step = SYN_USB_HOST_ENUM_SET_ADDR;
        return SYN_BUSY;

    case SYN_USB_HOST_ENUM_SET_ADDR: {
        /* Parse bMaxPacketSize0 from byte 7 of device descriptor */
        uint8_t max_pkt0 = host->enum_buf[7];
        if (max_pkt0 == 0U) {
            max_pkt0 = 8U;
        }
        host->dev_info.max_pkt_ep0 = max_pkt0;

        /* Re-open pipe 0 with actual max packet size */
        syn_port_usb_host_pipe_close(0U);
        syn_port_usb_host_pipe_open(0U, 0U, 0x00U, SYN_USB_EP_TYPE_CONTROL, max_pkt0);

        /* SET_ADDRESS */
        uint8_t addr = host->next_addr;
        if (addr > 127U) {
            addr = 1U;
        }
        host->next_addr = addr + 1U;
        host->dev_info.dev_addr = addr;

        build_setup(&pkt, 0x00U, SYN_USB_REQ_SET_ADDRESS, addr, 0U, 0U);
        syn_port_usb_host_submit_setup(0U, &pkt);
        host->xfer_pending = true;
        host->enum_step = SYN_USB_HOST_ENUM_GET_DEV_FULL;
        return SYN_BUSY;
    }

    case SYN_USB_HOST_ENUM_GET_DEV_FULL:
        /* Re-open pipe 0 to new address */
        syn_port_usb_host_pipe_close(0U);
        syn_port_usb_host_pipe_open(0U, host->dev_info.dev_addr, 0x00U, SYN_USB_EP_TYPE_CONTROL,
                                    host->dev_info.max_pkt_ep0);

        /* GET_DESCRIPTOR(Device, 18 bytes) */
        build_setup(&pkt, 0x80U, SYN_USB_REQ_GET_DESCRIPTOR,
                    (uint16_t)(SYN_USB_DESC_TYPE_DEVICE << 8), 0U, 18U);
        syn_port_usb_host_submit_setup(0U, &pkt);
        syn_port_usb_host_submit_data(0U, host->enum_buf, 18U, true);
        host->xfer_pending = true;
        host->enum_step = SYN_USB_HOST_ENUM_GET_CFG;
        return SYN_BUSY;

    case SYN_USB_HOST_ENUM_GET_CFG: {
        /* Parse full device descriptor */
        const uint8_t *d = host->enum_buf;
        host->dev_info.vid = (uint16_t)((uint16_t)d[9] << 8) | d[8];
        host->dev_info.pid = (uint16_t)((uint16_t)d[11] << 8) | d[10];
        host->dev_info.dev_class = d[4];
        host->dev_info.dev_subclass = d[5];
        host->dev_info.dev_protocol = d[6];
        host->dev_info.num_configs = d[17];

        /* GET_DESCRIPTOR(Configuration 0, up to enum_buf size) */
        build_setup(&pkt, 0x80U, SYN_USB_REQ_GET_DESCRIPTOR,
                    (uint16_t)(SYN_USB_DESC_TYPE_CONFIGURATION << 8), 0U,
                    SYN_USB_HOST_ENUM_BUF_SIZE);
        syn_port_usb_host_submit_setup(0U, &pkt);
        syn_port_usb_host_submit_data(0U, host->enum_buf, SYN_USB_HOST_ENUM_BUF_SIZE, true);
        host->xfer_pending = true;
        host->enum_step = SYN_USB_HOST_ENUM_SET_CFG;
        return SYN_BUSY;
    }

    case SYN_USB_HOST_ENUM_SET_CFG: {
        /* Config descriptor received. Parse bConfigurationValue (byte 5) */
        uint8_t cfg_val = 1U;
        if (host->enum_buf_len >= 6U) {
            cfg_val = host->enum_buf[5];
        }

        /* SET_CONFIGURATION */
        build_setup(&pkt, 0x00U, SYN_USB_REQ_SET_CONFIGURATION, cfg_val, 0U, 0U);
        syn_port_usb_host_submit_setup(0U, &pkt);
        host->xfer_pending = true;
        host->enum_step = SYN_USB_HOST_ENUM_CLASS_PROBE;
        return SYN_BUSY;
    }

    case SYN_USB_HOST_ENUM_CLASS_PROBE:
        /* Probe class drivers against interface descriptors */
        probe_class_drivers(host);
        host->enum_step = SYN_USB_HOST_ENUM_DONE;
        return SYN_OK;

    case SYN_USB_HOST_ENUM_DONE:
        return SYN_OK;

    default:
        return SYN_ERROR;
    }
}

/* ── Public API ────────────────────────────────────────────────────────── */

SYN_Status syn_usb_host_init(SYN_USB_Host *host)
{
    if (!host) {
        return SYN_INVALID_PARAM;
    }
    memset(host, 0, sizeof(*host));
    host->state = SYN_USB_HOST_STATE_DISCONNECTED;
    host->next_addr = 1U;
    return SYN_OK;
}

SYN_Status syn_usb_host_register_class(SYN_USB_Host *host, const SYN_USB_HostClassDriver *cls)
{
    if (!host || !cls) {
        return SYN_INVALID_PARAM;
    }
    if (host->class_count >= SYN_USB_HOST_MAX_CLASSES) {
        return SYN_ERROR;
    }
    host->classes[host->class_count] = *cls;
    host->classes[host->class_count].matched = false;
    host->class_count++;
    return SYN_OK;
}

SYN_Status syn_usb_host_process(SYN_USB_Host *host)
{
    if (!host) {
        return SYN_INVALID_PARAM;
    }

    switch (host->state) {
    case SYN_USB_HOST_STATE_DISCONNECTED:
        if (syn_port_usb_host_device_attached()) {
            syn_port_usb_host_vbus(true);
            host->state = SYN_USB_HOST_STATE_ATTACHED;
        }
        return SYN_OK;

    case SYN_USB_HOST_STATE_ATTACHED:
        syn_port_usb_host_vbus(true);
        syn_port_usb_host_bus_reset();
        host->enum_step = SYN_USB_HOST_ENUM_GET_DEV8;
        host->xfer_pending = false;
        host->enum_buf_len = 0;
        memset(&host->dev_info, 0, sizeof(host->dev_info));
        /* Clear matched flags */
        for (uint8_t i = 0; i < host->class_count; i++) {
            host->classes[i].matched = false;
        }
        host->state = SYN_USB_HOST_STATE_ENUMERATING;
        return SYN_OK;

    case SYN_USB_HOST_STATE_ENUMERATING: {
        SYN_Status st = enumerate_step(host);
        if (st == SYN_OK) {
            host->state = SYN_USB_HOST_STATE_READY;
        } else if (st == SYN_ERROR) {
            host->state = SYN_USB_HOST_STATE_ERROR;
        }
        return st;
    }

    case SYN_USB_HOST_STATE_READY:
        /* Check for detach */
        if (!syn_port_usb_host_device_attached()) {
            for (uint8_t i = 0; i < host->class_count; i++) {
                if (host->classes[i].matched && host->classes[i].disconnected) {
                    host->classes[i].disconnected(host->classes[i].ctx);
                }
                host->classes[i].matched = false;
            }
            syn_port_usb_host_pipe_close(0U);
            syn_port_usb_host_vbus(false);
            host->state = SYN_USB_HOST_STATE_DISCONNECTED;
            return SYN_OK;
        }
        /* Tick class drivers */
        for (uint8_t i = 0; i < host->class_count; i++) {
            if (host->classes[i].matched && host->classes[i].process) {
                host->classes[i].process(host->classes[i].ctx);
            }
        }
        return SYN_OK;

    case SYN_USB_HOST_STATE_ERROR:
        /* On error, check detach to allow re-enumeration */
        if (!syn_port_usb_host_device_attached()) {
            syn_port_usb_host_vbus(false);
            host->state = SYN_USB_HOST_STATE_DISCONNECTED;
        }
        return SYN_ERROR;

    default:
        return SYN_ERROR;
    }
}

bool syn_usb_host_is_ready(const SYN_USB_Host *host)
{
    return (host != NULL) && (host->state == SYN_USB_HOST_STATE_READY);
}

const SYN_USB_HostDevInfo *syn_usb_host_get_dev_info(const SYN_USB_Host *host)
{
    if (!host || host->state < SYN_USB_HOST_STATE_READY) {
        return NULL;
    }
    return &host->dev_info;
}
```


