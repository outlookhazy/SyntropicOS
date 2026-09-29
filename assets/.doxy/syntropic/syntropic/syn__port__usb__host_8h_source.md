

# File syn\_port\_usb\_host.h

[**File List**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**syn\_port\_usb\_host.h**](syn__port__usb__host_8h.md)

[Go to the documentation of this file](syn__port__usb__host_8h.md)


```C++

#ifndef SYN_PORT_USB_HOST_H
#define SYN_PORT_USB_HOST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syn_port_usb.h"
#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

SYN_Status syn_port_usb_host_init(void);

SYN_Status syn_port_usb_host_vbus(bool enable);

SYN_Status syn_port_usb_host_bus_reset(void);

bool syn_port_usb_host_device_attached(void);

SYN_Status syn_port_usb_host_pipe_open(uint8_t pipe, uint8_t dev_addr, uint8_t ep_addr,
                                       uint8_t ep_type, uint16_t max_pkt);

SYN_Status syn_port_usb_host_pipe_close(uint8_t pipe);

SYN_Status syn_port_usb_host_submit_setup(uint8_t pipe, const SYN_USB_SetupPacket *pkt);

SYN_Status syn_port_usb_host_submit_data(uint8_t pipe, uint8_t *buf, uint16_t len, bool is_in);

bool syn_port_usb_host_xfer_done(uint8_t pipe);

SYN_Status syn_port_usb_host_xfer_result(uint8_t pipe, uint16_t *actual_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_USB_HOST_H */
```


