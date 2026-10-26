

# File syn\_port\_usb.h

[**File List**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**syn\_port\_usb.h**](syn__port__usb_8h.md)

[Go to the documentation of this file](syn__port__usb_8h.md)


```C++

#ifndef SYN_PORT_USB_H
#define SYN_PORT_USB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_USB_EP_TYPE_CONTROL 0U 
#define SYN_USB_EP_TYPE_ISOC 1U    
#define SYN_USB_EP_TYPE_BULK 2U    
#define SYN_USB_EP_TYPE_INTR 3U    
SYN_Status syn_port_usb_init(void);

SYN_Status syn_port_usb_connect(void);

SYN_Status syn_port_usb_disconnect(void);

SYN_Status syn_port_usb_set_address(uint8_t addr);

SYN_Status syn_port_usb_ep_open(uint8_t ep_addr, uint8_t ep_type, uint16_t max_pkt);

SYN_Status syn_port_usb_ep_close(uint8_t ep_addr);

SYN_Status syn_port_usb_ep_write(uint8_t ep_addr, const void *data, uint16_t len);

SYN_Status syn_port_usb_ep_read(uint8_t ep_addr, void *buf, uint16_t max_len, uint16_t *out_len);

SYN_Status syn_port_usb_ep_stall(uint8_t ep_addr);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_USB_H */
```


