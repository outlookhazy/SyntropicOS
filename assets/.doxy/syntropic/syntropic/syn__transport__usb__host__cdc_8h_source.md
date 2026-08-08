

# File syn\_transport\_usb\_host\_cdc.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_transport\_usb\_host\_cdc.h**](syn__transport__usb__host__cdc_8h.md)

[Go to the documentation of this file](syn__transport__usb__host__cdc_8h.md)


```C++

#ifndef SYN_TRANSPORT_USB_HOST_CDC_H
#define SYN_TRANSPORT_USB_HOST_CDC_H

#include "syntropic/drivers/syn_usb_host_cdc.h"
#include "syntropic/net/syn_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

void syn_transport_from_usb_host_cdc(SYN_Transport *t, SYN_USB_HostCDC *hcdc);

#ifdef __cplusplus
}
#endif

#endif /* SYN_TRANSPORT_USB_HOST_CDC_H */
```


