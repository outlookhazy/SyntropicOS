

# File syn\_usb\_hid\_mouse.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_hid\_mouse.h**](syn__usb__hid__mouse_8h.md)

[Go to the documentation of this file](syn__usb__hid__mouse_8h.md)


```C++

#ifndef SYN_USB_HID_MOUSE_H
#define SYN_USB_HID_MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb_hid.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_USB_HID_MOUSE_BTN_LEFT (1U << 0)
#define SYN_USB_HID_MOUSE_BTN_RIGHT (1U << 1)
#define SYN_USB_HID_MOUSE_BTN_MIDDLE (1U << 2)

typedef struct SYN_PACKED {
    uint8_t buttons; 
    int8_t x;        
    int8_t y;        
    int8_t wheel;    
} SYN_USB_HID_MouseReport;

extern const uint8_t SYN_USB_HID_MOUSE_REPORT_DESC[54];

SYN_Status syn_usb_hid_mouse_send(SYN_USB_HID *hid, uint8_t buttons, int8_t x, int8_t y,
                                  int8_t wheel);

SYN_Status syn_usb_hid_mouse_move(SYN_USB_HID *hid, int8_t x, int8_t y);

SYN_Status syn_usb_hid_mouse_click(SYN_USB_HID *hid, uint8_t buttons);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_HID_MOUSE_H */
```


