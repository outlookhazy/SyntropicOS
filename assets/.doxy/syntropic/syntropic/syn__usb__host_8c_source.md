

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

static uint16_t calc_iface_span(const uint8_t *buf, uint16_t total, uint16_t iface_start)
{
    uint8_t iface_class = buf[iface_start + 5U];
    uint16_t next = (uint16_t)(iface_start + buf[iface_start]);

    while ((next + 2U) <= total) {
        uint8_t n_len = buf[next];
        uint8_t n_type = buf[next + 1U];
        if ((n_len < 2U) || ((next + n_len) > total)) {
            break;
        }
        if (n_type == SYN_USB_DESC_TYPE_INTERFACE) {
            bool is_sub_cdc_data =
                (iface_class == 0x02U) && (n_len >= 9U) && (buf[next + 5U] == 0x0AU);
            if (!is_sub_cdc_data) {
                break;
            }
        }
        next = (uint16_t)(next + n_len);
    }

    return (uint16_t)(next - iface_start);
}

static void match_iface_drivers(SYN_USB_Host *host, const uint8_t *iface_desc, uint16_t iface_len)
{
    uint8_t iface_class = iface_desc[5];
    uint8_t iface_sub = iface_desc[6];
    uint8_t iface_proto = iface_desc[7];

    for (uint8_t i = 0U; i < host->class_count; i++) {
        SYN_USB_HostClassDriver *cls = &host->classes[i];
        if (cls->matched || (cls->probe == NULL)) {
            continue;
        }
        bool class_ok = (cls->class_code == 0xFFU) || (cls->class_code == iface_class);
        bool sub_ok = (cls->subclass_code == 0xFFU) || (cls->subclass_code == iface_sub);
        bool proto_ok = (cls->protocol_code == 0xFFU) || (cls->protocol_code == iface_proto);

        if (class_ok && sub_ok && proto_ok &&
            (cls->probe(cls->ctx, host->dev_info.dev_addr, iface_desc, iface_len) == SYN_OK)) {
            cls->matched = true;
            break;
        }
    }
}

static void probe_class_drivers(SYN_USB_Host *host)
{
    const uint8_t *buf = host->enum_buf;
    uint16_t total = host->enum_buf_len;
    uint16_t pos = 0U;

    while ((pos + 2U) <= total) {
        uint8_t b_len = buf[pos];
        uint8_t b_type = buf[pos + 1U];

        if ((b_len < 2U) || ((pos + b_len) > total)) {
            break;
        }

        if ((b_type == SYN_USB_DESC_TYPE_INTERFACE) && (b_len >= 9U)) {
            uint16_t span = calc_iface_span(buf, total, pos);
            match_iface_drivers(host, &buf[pos], span);
        }

        pos = (uint16_t)(pos + b_len);
    }
}

static SYN_Status enum_poll_xfer(SYN_USB_Host *host)
{
    if (!host->xfer_pending) {
        return SYN_OK;
    }
    if (!syn_port_usb_host_xfer_done(0U)) {
        return SYN_BUSY;
    }

    host->xfer_pending = false;
    uint16_t actual = 0U;
    if (syn_port_usb_host_xfer_result(0U, &actual) != SYN_OK) {
        return SYN_ERROR;
    }

    if ((host->enum_step != SYN_USB_HOST_ENUM_GET_DEV_FULL) &&
        (host->enum_step != SYN_USB_HOST_ENUM_CLASS_PROBE)) {
        host->enum_buf_len = actual;
    }

    return SYN_OK;
}

static SYN_Status enumerate_step(SYN_USB_Host *host)
{
    SYN_Status poll_st = enum_poll_xfer(host);
    if (poll_st != SYN_OK) {
        return poll_st;
    }

    SYN_USB_SetupPacket pkt;

    switch (host->enum_step) {
    case SYN_USB_HOST_ENUM_GET_DEV8:
        (void)syn_port_usb_host_pipe_open(0U, 0U, 0x00U, SYN_USB_EP_TYPE_CONTROL, 8U);
        build_setup(&pkt, 0x80U, SYN_USB_REQ_GET_DESCRIPTOR,
                    (uint16_t)(SYN_USB_DESC_TYPE_DEVICE << 8), 0U, 8U);
        (void)syn_port_usb_host_submit_setup(0U, &pkt);
        (void)syn_port_usb_host_submit_data(0U, host->enum_buf, 8U, true);
        host->xfer_pending = true;
        host->enum_step = SYN_USB_HOST_ENUM_SET_ADDR;
        return SYN_BUSY;

    case SYN_USB_HOST_ENUM_SET_ADDR: {
        uint8_t max_pkt0 = (host->enum_buf[7] == 0U) ? 8U : host->enum_buf[7];
        host->dev_info.max_pkt_ep0 = max_pkt0;
        (void)syn_port_usb_host_pipe_close(0U);
        (void)syn_port_usb_host_pipe_open(0U, 0U, 0x00U, SYN_USB_EP_TYPE_CONTROL, max_pkt0);

        uint8_t addr = (host->next_addr > 127U) ? 1U : host->next_addr;
        host->next_addr = (uint8_t)(addr + 1U);
        host->dev_info.dev_addr = addr;

        build_setup(&pkt, 0x00U, SYN_USB_REQ_SET_ADDRESS, addr, 0U, 0U);
        (void)syn_port_usb_host_submit_setup(0U, &pkt);
        host->xfer_pending = true;
        host->enum_step = SYN_USB_HOST_ENUM_GET_DEV_FULL;
        return SYN_BUSY;
    }

    case SYN_USB_HOST_ENUM_GET_DEV_FULL:
        (void)syn_port_usb_host_pipe_close(0U);
        (void)syn_port_usb_host_pipe_open(0U, host->dev_info.dev_addr, 0x00U,
                                          SYN_USB_EP_TYPE_CONTROL, host->dev_info.max_pkt_ep0);
        build_setup(&pkt, 0x80U, SYN_USB_REQ_GET_DESCRIPTOR,
                    (uint16_t)(SYN_USB_DESC_TYPE_DEVICE << 8), 0U, 18U);
        (void)syn_port_usb_host_submit_setup(0U, &pkt);
        (void)syn_port_usb_host_submit_data(0U, host->enum_buf, 18U, true);
        host->xfer_pending = true;
        host->enum_step = SYN_USB_HOST_ENUM_GET_CFG;
        return SYN_BUSY;

    case SYN_USB_HOST_ENUM_GET_CFG: {
        const uint8_t *d = host->enum_buf;
        host->dev_info.vid = (uint16_t)(((uint16_t)d[9] << 8) | d[8]);
        host->dev_info.pid = (uint16_t)(((uint16_t)d[11] << 8) | d[10]);
        host->dev_info.dev_class = d[4];
        host->dev_info.dev_subclass = d[5];
        host->dev_info.dev_protocol = d[6];
        host->dev_info.num_configs = d[17];

        build_setup(&pkt, 0x80U, SYN_USB_REQ_GET_DESCRIPTOR,
                    (uint16_t)(SYN_USB_DESC_TYPE_CONFIGURATION << 8), 0U,
                    SYN_USB_HOST_ENUM_BUF_SIZE);
        (void)syn_port_usb_host_submit_setup(0U, &pkt);
        (void)syn_port_usb_host_submit_data(0U, host->enum_buf, SYN_USB_HOST_ENUM_BUF_SIZE, true);
        host->xfer_pending = true;
        host->enum_step = SYN_USB_HOST_ENUM_SET_CFG;
        return SYN_BUSY;
    }

    case SYN_USB_HOST_ENUM_SET_CFG: {
        uint8_t cfg_val = (host->enum_buf_len >= 6U) ? host->enum_buf[5] : 1U;
        build_setup(&pkt, 0x00U, SYN_USB_REQ_SET_CONFIGURATION, cfg_val, 0U, 0U);
        (void)syn_port_usb_host_submit_setup(0U, &pkt);
        host->xfer_pending = true;
        host->enum_step = SYN_USB_HOST_ENUM_CLASS_PROBE;
        return SYN_BUSY;
    }

    case SYN_USB_HOST_ENUM_CLASS_PROBE:
        probe_class_drivers(host);
        host->enum_step = SYN_USB_HOST_ENUM_DONE;
        return SYN_OK;

    case SYN_USB_HOST_ENUM_DONE:
        return SYN_OK;

    default:
        return SYN_ERROR;
    }
}

static void host_process_ready(SYN_USB_Host *host)
{
    if (!syn_port_usb_host_device_attached()) {
        for (uint8_t i = 0U; i < host->class_count; i++) {
            if (host->classes[i].matched && (host->classes[i].disconnected != NULL)) {
                host->classes[i].disconnected(host->classes[i].ctx);
            }
            host->classes[i].matched = false;
        }
        (void)syn_port_usb_host_pipe_close(0U);
        (void)syn_port_usb_host_vbus(false);
        host->state = SYN_USB_HOST_STATE_DISCONNECTED;
        return;
    }

    for (uint8_t i = 0U; i < host->class_count; i++) {
        if (host->classes[i].matched && (host->classes[i].process != NULL)) {
            host->classes[i].process(host->classes[i].ctx);
        }
    }
}

/* ── Public API ────────────────────────────────────────────────────────── */

SYN_Status syn_usb_host_init(SYN_USB_Host *host)
{
    if (host == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(host, 0, sizeof(*host));
    host->state = SYN_USB_HOST_STATE_DISCONNECTED;
    host->next_addr = 1U;
    return SYN_OK;
}

SYN_Status syn_usb_host_register_class(SYN_USB_Host *host, const SYN_USB_HostClassDriver *cls)
{
    if ((host == NULL) || (cls == NULL)) {
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
    if (host == NULL) {
        return SYN_INVALID_PARAM;
    }

    switch (host->state) {
    case SYN_USB_HOST_STATE_DISCONNECTED:
        if (syn_port_usb_host_device_attached()) {
            (void)syn_port_usb_host_vbus(true);
            host->state = SYN_USB_HOST_STATE_ATTACHED;
        }
        return SYN_OK;

    case SYN_USB_HOST_STATE_ATTACHED:
        (void)syn_port_usb_host_vbus(true);
        (void)syn_port_usb_host_bus_reset();
        host->enum_step = SYN_USB_HOST_ENUM_GET_DEV8;
        host->xfer_pending = false;
        host->enum_buf_len = 0U;
        memset(&host->dev_info, 0, sizeof(host->dev_info));
        for (uint8_t i = 0U; i < host->class_count; i++) {
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
        host_process_ready(host);
        return SYN_OK;

    case SYN_USB_HOST_STATE_ERROR:
        if (!syn_port_usb_host_device_attached()) {
            (void)syn_port_usb_host_vbus(false);
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
    if ((host == NULL) || (host->state < SYN_USB_HOST_STATE_READY)) {
        return NULL;
    }
    return &host->dev_info;
}
```


