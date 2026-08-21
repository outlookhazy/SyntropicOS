

# File syn\_transport\_usb\_cdc.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_transport\_usb\_cdc.c**](syn__transport__usb__cdc_8c.md)

[Go to the documentation of this file](syn__transport__usb__cdc_8c.md)


```C++

#include "syntropic/drivers/syn_transport_usb_cdc.h"

static bool cdc_transport_send(const uint8_t *data, size_t len, void *ctx)
{
    SYN_USB_CDC *cdc = (SYN_USB_CDC *)ctx;
    if (!cdc || !data || len == 0U) {
        return false;
    }
    return (syn_usb_cdc_write(cdc, data, len) == SYN_OK);
}

static bool cdc_transport_recv(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    SYN_USB_CDC *cdc = (SYN_USB_CDC *)ctx;
    if (!cdc || !data || !out_len) {
        return false;
    }
    SYN_Status st = syn_usb_cdc_read(cdc, data, max_len, out_len);
    return (st == SYN_OK && *out_len > 0U);
}

void syn_transport_from_usb_cdc(SYN_Transport *t, SYN_USB_CDC *cdc)
{
    if (!t) {
        return;
    }
    t->send = (cdc != NULL) ? cdc_transport_send : NULL;
    t->recv = (cdc != NULL) ? cdc_transport_recv : NULL;
    t->ctx = cdc;
}
```


