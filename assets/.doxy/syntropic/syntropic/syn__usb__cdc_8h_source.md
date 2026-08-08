

# File syn\_usb\_cdc.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_cdc.h**](syn__usb__cdc_8h.md)

[Go to the documentation of this file](syn__usb__cdc_8h.md)


```C++

#ifndef SYN_USB_CDC_H
#define SYN_USB_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_USB_CDC_SET_LINE_CODING 0x20U        
#define SYN_USB_CDC_GET_LINE_CODING 0x21U        
#define SYN_USB_CDC_SET_CONTROL_LINE_STATE 0x22U 
typedef struct {
    uint32_t baudrate; 
    uint8_t stop_bits; 
    uint8_t parity;    
    uint8_t data_bits; 
} SYN_USB_CDC_LineCoding;

typedef struct {
    uint8_t ep_in;                      
    uint8_t ep_out;                     
    uint8_t ep_cmd;                     
    uint8_t dev_address;                
    bool configured;                    
    SYN_USB_CDC_LineCoding line_coding; 
    uint8_t rx_buf[128];                
    uint16_t rx_len;                    
    uint8_t tx_buf[128];                
    uint16_t tx_len;                    
} SYN_USB_CDC;

SYN_Status syn_usb_cdc_init(SYN_USB_CDC *cdc);

SYN_Status syn_usb_cdc_register(SYN_USB_Device *dev, SYN_USB_CDC *cdc);

SYN_Status syn_usb_cdc_handle_setup(SYN_USB_CDC *cdc, const SYN_USB_SetupPacket *setup,
                                    uint8_t *resp, size_t *rlen);

SYN_Status syn_usb_cdc_write(SYN_USB_CDC *cdc, const void *data, size_t len);

SYN_Status syn_usb_cdc_read(SYN_USB_CDC *cdc, void *buf, size_t max_len, size_t *out_len);

bool syn_usb_cdc_rx_available(const SYN_USB_CDC *cdc);

bool syn_usb_cdc_tx_ready(const SYN_USB_CDC *cdc);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */
#include "syntropic/pt/syn_pt.h"

#define PT_USB_CDC_WAIT_RX(pt, cdc) PT_WAIT_UNTIL(pt, syn_usb_cdc_rx_available(cdc))

#define PT_USB_CDC_WAIT_TX_READY(pt, cdc) PT_WAIT_UNTIL(pt, syn_usb_cdc_tx_ready(cdc))

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_CDC_H */
```


