

# File syn\_usb\_host\_cdc.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_host\_cdc.h**](syn__usb__host__cdc_8h.md)

[Go to the documentation of this file](syn__usb__host__cdc_8h.md)


```C++

#ifndef SYN_USB_HOST_CDC_H
#define SYN_USB_HOST_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb_cdc.h"
#include "syntropic/drivers/syn_usb_host.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef SYN_USB_HOST_CDC_BUF_SIZE
#define SYN_USB_HOST_CDC_BUF_SIZE 128U 
#endif

typedef struct {
    uint8_t dev_addr;      
    uint8_t pipe_bulk_in;  
    uint8_t pipe_bulk_out; 
    uint8_t ep_bulk_in;    
    uint8_t ep_bulk_out;   
    bool connected;        
    SYN_USB_CDC_LineCoding line_coding; 
    uint8_t rx_buf[SYN_USB_HOST_CDC_BUF_SIZE]; 
    uint16_t rx_len;                           
    uint8_t tx_buf[SYN_USB_HOST_CDC_BUF_SIZE]; 
    uint16_t tx_len;                           
} SYN_USB_HostCDC;

SYN_Status syn_usb_host_cdc_init(SYN_USB_HostCDC *hcdc);

SYN_Status syn_usb_host_cdc_register(SYN_USB_Host *host, SYN_USB_HostCDC *hcdc);

SYN_Status syn_usb_host_cdc_write(SYN_USB_HostCDC *hcdc, const void *data, size_t len);

SYN_Status syn_usb_host_cdc_read(SYN_USB_HostCDC *hcdc, void *buf, size_t max_len, size_t *out_len);

bool syn_usb_host_cdc_rx_available(const SYN_USB_HostCDC *hcdc);

bool syn_usb_host_cdc_tx_ready(const SYN_USB_HostCDC *hcdc);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */
#include "syntropic/pt/syn_pt.h"

#define PT_USB_HOST_CDC_WAIT_RX(pt, hcdc) PT_WAIT_UNTIL(pt, syn_usb_host_cdc_rx_available(hcdc))

#define PT_USB_HOST_CDC_WAIT_TX_READY(pt, hcdc) PT_WAIT_UNTIL(pt, syn_usb_host_cdc_tx_ready(hcdc))

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_HOST_CDC_H */
```


