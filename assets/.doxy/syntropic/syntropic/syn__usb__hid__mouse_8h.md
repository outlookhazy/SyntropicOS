

# File syn\_usb\_hid\_mouse.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_hid\_mouse.h**](syn__usb__hid__mouse_8h.md)

[Go to the source code of this file](syn__usb__hid__mouse_8h_source.md)

_Zero-Heap USB HID Boot Mouse Device Helper Module._ 

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/drivers/syn_usb_hid.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_USB\_HID\_MouseReport**](structSYN__USB__HID__MouseReport.md) <br> |






## Public Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t | [**SYN\_USB\_HID\_MOUSE\_REPORT\_DESC**](#variable-syn_usb_hid_mouse_report_desc)  <br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_mouse\_click**](#function-syn_usb_hid_mouse_click) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid, uint8\_t buttons) <br>_Send a USB HID Mouse click (press button + release)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_mouse\_move**](#function-syn_usb_hid_mouse_move) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid, int8\_t x, int8\_t y) <br>_Move the USB HID Mouse pointer relatively._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_mouse\_send**](#function-syn_usb_hid_mouse_send) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid, uint8\_t buttons, int8\_t x, int8\_t y, int8\_t wheel) <br>_Send a USB HID Mouse report with relative movement and buttons._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_USB\_HID\_MOUSE\_BTN\_LEFT**](syn__usb__hid__mouse_8h.md#define-syn_usb_hid_mouse_btn_left)  `(1U &lt;&lt; 0)`<br>_Left mouse button bitmask._  |
| define  | [**SYN\_USB\_HID\_MOUSE\_BTN\_MIDDLE**](syn__usb__hid__mouse_8h.md#define-syn_usb_hid_mouse_btn_middle)  `(1U &lt;&lt; 2)`<br>_Middle mouse button bitmask._  |
| define  | [**SYN\_USB\_HID\_MOUSE\_BTN\_RIGHT**](syn__usb__hid__mouse_8h.md#define-syn_usb_hid_mouse_btn_right)  `(1U &lt;&lt; 1)`<br>_Right mouse button bitmask._  |

## Public Attributes Documentation




### variable SYN\_USB\_HID\_MOUSE\_REPORT\_DESC 

```C++
const uint8_t SYN_USB_HID_MOUSE_REPORT_DESC[54];
```



Standard USB HID Mouse Report Descriptor (54 Bytes) 


        

<hr>
## Public Functions Documentation




### function syn\_usb\_hid\_mouse\_click 

_Send a USB HID Mouse click (press button + release)._ 
```C++
SYN_Status syn_usb_hid_mouse_click (
    SYN_USB_HID * hid,
    uint8_t buttons
) 
```





**Parameters:**


* `hid` Pointer to USB HID instance. 
* `buttons` Button bitmask (e.g. SYN\_USB\_HID\_MOUSE\_BTN\_LEFT). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_hid\_mouse\_move 

_Move the USB HID Mouse pointer relatively._ 
```C++
SYN_Status syn_usb_hid_mouse_move (
    SYN_USB_HID * hid,
    int8_t x,
    int8_t y
) 
```





**Parameters:**


* `hid` Pointer to USB HID instance. 
* `x` Relative X axis displacement. 
* `y` Relative Y axis displacement. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_hid\_mouse\_send 

_Send a USB HID Mouse report with relative movement and buttons._ 
```C++
SYN_Status syn_usb_hid_mouse_send (
    SYN_USB_HID * hid,
    uint8_t buttons,
    int8_t x,
    int8_t y,
    int8_t wheel
) 
```





**Parameters:**


* `hid` Pointer to USB HID instance. 
* `buttons` Button bitmask (Left=0x01, Right=0x02, Middle=0x04). 
* `x` Relative X axis displacement. 
* `y` Relative Y axis displacement. 
* `wheel` Relative vertical scroll wheel displacement. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Macro Definition Documentation





### define SYN\_USB\_HID\_MOUSE\_BTN\_LEFT 

_Left mouse button bitmask._ 
```C++
#define SYN_USB_HID_MOUSE_BTN_LEFT `(1U << 0)`
```



USB HID Mouse Button Bitmasks 


        

<hr>



### define SYN\_USB\_HID\_MOUSE\_BTN\_MIDDLE 

_Middle mouse button bitmask._ 
```C++
#define SYN_USB_HID_MOUSE_BTN_MIDDLE `(1U << 2)`
```




<hr>



### define SYN\_USB\_HID\_MOUSE\_BTN\_RIGHT 

_Right mouse button bitmask._ 
```C++
#define SYN_USB_HID_MOUSE_BTN_RIGHT `(1U << 1)`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_hid_mouse.h`

