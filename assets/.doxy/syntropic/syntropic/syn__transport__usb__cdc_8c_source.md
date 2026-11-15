

# File syn\_transport\_usb\_cdc.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_transport\_usb\_cdc.c**](syn__transport__usb__cdc_8c.md)

[Go to the documentation of this file](syn__transport__usb__cdc_8c.md)


```C++


#include "syntropic/drivers/syn_transport_usb_cdc.h"

#include <string.h>

static bool cdc_transport_send(const uint8_t *data, size_t len, void *ctx)
{
    SYN_USB_CDC *cdc = (SYN_USB_CDC *)ctx;
    if ((cdc == NULL) || (data == NULL) || (len == 0U) || (len > sizeof(cdc->tx_buf))) {
        return false;
    }
    return (syn_usb_cdc_write(cdc, data, len) == SYN_OK);
}

static bool cdc_transport_recv(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    SYN_USB_CDC *cdc = (SYN_USB_CDC *)ctx;
    if ((cdc == NULL) || (data == NULL) || (out_len == NULL)) {
        return false;
    }
    SYN_Status st = syn_usb_cdc_read(cdc, data, max_len, out_len);
    return (st == SYN_OK) && (*out_len > 0U);
}

static bool cdc_transport_has_data(const void *ctx)
{
    return syn_usb_cdc_rx_available((const SYN_USB_CDC *)ctx);
}

void syn_transport_from_usb_cdc(SYN_Transport *t, SYN_USB_CDC *cdc)
{
    if (t == NULL) {
        return;
    }
    memset(t, 0, sizeof(*t));
    if (cdc != NULL) {
        t->send = cdc_transport_send;
        t->recv = cdc_transport_recv;
        t->has_data = cdc_transport_has_data;
        t->ctx = cdc;
    }
}
```


