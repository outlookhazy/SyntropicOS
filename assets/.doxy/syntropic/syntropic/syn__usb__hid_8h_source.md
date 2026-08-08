

# File syn\_usb\_hid.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_hid.h**](syn__usb__hid_8h.md)

[Go to the documentation of this file](syn__usb__hid_8h.md)


```C++

#ifndef SYN_USB_HID_H
#define SYN_USB_HID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_USB_HID_MAX_REPORT_SIZE 64U 
#define SYN_USB_HID_REQ_GET_REPORT 0x01U   
#define SYN_USB_HID_REQ_GET_IDLE 0x02U     
#define SYN_USB_HID_REQ_GET_PROTOCOL 0x03U 
#define SYN_USB_HID_REQ_SET_REPORT 0x09U   
#define SYN_USB_HID_REQ_SET_IDLE 0x0AU     
#define SYN_USB_HID_REQ_SET_PROTOCOL 0x0BU 
typedef struct {
    uint8_t ep_in;                               
    uint8_t ep_out;                              
    uint8_t iface_num;                           
    uint8_t protocol;                            
    const uint8_t *report_desc;                  
    uint16_t report_desc_len;                    
    uint8_t tx_buf[SYN_USB_HID_MAX_REPORT_SIZE]; 
    uint16_t tx_len;                             
    uint8_t rx_buf[SYN_USB_HID_MAX_REPORT_SIZE]; 
    uint16_t rx_len;                             
    uint8_t idle_rate;                           
    uint8_t active_protocol;                     
} SYN_USB_HID;

SYN_Status syn_usb_hid_init(SYN_USB_HID *hid);

SYN_Status syn_usb_hid_register(SYN_USB_Device *dev, SYN_USB_HID *hid, const uint8_t *report_desc,
                                uint16_t report_desc_len);

SYN_Status syn_usb_hid_send_report(SYN_USB_HID *hid, const void *report, size_t len);

SYN_Status syn_usb_hid_read_report(SYN_USB_HID *hid, void *buf, size_t max_len, size_t *out_len);

bool syn_usb_hid_report_available(const SYN_USB_HID *hid);

bool syn_usb_hid_tx_ready(const SYN_USB_HID *hid);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */
#include "syntropic/pt/syn_pt.h"

#define PT_USB_HID_WAIT_RX(pt, hid) PT_WAIT_UNTIL(pt, syn_usb_hid_report_available(hid))

#define PT_USB_HID_WAIT_TX_READY(pt, hid) PT_WAIT_UNTIL(pt, syn_usb_hid_tx_ready(hid))

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_HID_H */
```


