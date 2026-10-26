

# File syn\_transport\_usb\_host\_cdc.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_transport\_usb\_host\_cdc.c**](syn__transport__usb__host__cdc_8c.md)

[Go to the documentation of this file](syn__transport__usb__host__cdc_8c.md)


```C++

#include "syntropic/drivers/syn_transport_usb_host_cdc.h"

static bool host_cdc_transport_send(const uint8_t *data, size_t len, void *ctx)
{
    SYN_USB_HostCDC *hcdc = (SYN_USB_HostCDC *)ctx;
    if (!hcdc || !data || len == 0) {
        return false;
    }
    return (syn_usb_host_cdc_write(hcdc, data, len) == SYN_OK);
}

static bool host_cdc_transport_recv(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    SYN_USB_HostCDC *hcdc = (SYN_USB_HostCDC *)ctx;
    if (!hcdc || !data || !out_len) {
        return false;
    }
    return (syn_usb_host_cdc_read(hcdc, data, max_len, out_len) == SYN_OK);
}

void syn_transport_from_usb_host_cdc(SYN_Transport *t, SYN_USB_HostCDC *hcdc)
{
    if (!t) {
        return;
    }
    if (!hcdc) {
        t->send = NULL;
        t->recv = NULL;
        t->ctx = NULL;
        return;
    }
    t->send = host_cdc_transport_send;
    t->recv = host_cdc_transport_recv;
    t->ctx = hcdc;
}
```


