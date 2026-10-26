

# File syn\_usb\_hid\_keyboard.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_hid\_keyboard.c**](syn__usb__hid__keyboard_8c.md)

[Go to the source code of this file](syn__usb__hid__keyboard_8c_source.md)

_Zero-Heap USB HID Boot Keyboard Device Helper Module Implementation._ 

* `#include "syntropic/drivers/syn_usb_hid_keyboard.h"`
* `#include <string.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t | [**SYN\_USB\_HID\_KEYBOARD\_REPORT\_DESC**](#variable-syn_usb_hid_keyboard_report_desc)  <br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_keyboard\_press**](#function-syn_usb_hid_keyboard_press) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid, uint8\_t modifier, uint8\_t keycode) <br>_Send a single keystroke (press 1 key + optional modifier)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_keyboard\_release**](#function-syn_usb_hid_keyboard_release) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid) <br>_Send a keyboard release report (all keys released)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_keyboard\_send**](#function-syn_usb_hid_keyboard_send) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid, uint8\_t modifier, const uint8\_t keycodes) <br>_Send a USB HID Keyboard report containing active keycodes._  |




























## Public Attributes Documentation




### variable SYN\_USB\_HID\_KEYBOARD\_REPORT\_DESC 

```C++
const uint8_t SYN_USB_HID_KEYBOARD_REPORT_DESC[63];
```



Standard USB HID Keyboard Report Descriptor (63 Bytes) 


        

<hr>
## Public Functions Documentation




### function syn\_usb\_hid\_keyboard\_press 

_Send a single keystroke (press 1 key + optional modifier)._ 
```C++
SYN_Status syn_usb_hid_keyboard_press (
    SYN_USB_HID * hid,
    uint8_t modifier,
    uint8_t keycode
) 
```





**Parameters:**


* `hid` Pointer to USB HID instance. 
* `modifier` Modifier bitmask (CTRL, SHIFT, ALT, GUI). 
* `keycode` HID keycode (e.g. SYN\_USB\_HID\_KEY\_A). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_hid\_keyboard\_release 

_Send a keyboard release report (all keys released)._ 
```C++
SYN_Status syn_usb_hid_keyboard_release (
    SYN_USB_HID * hid
) 
```





**Parameters:**


* `hid` Pointer to USB HID instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_hid\_keyboard\_send 

_Send a USB HID Keyboard report containing active keycodes._ 
```C++
SYN_Status syn_usb_hid_keyboard_send (
    SYN_USB_HID * hid,
    uint8_t modifier,
    const uint8_t keycodes
) 
```





**Parameters:**


* `hid` Pointer to USB HID instance. 
* `modifier` Modifier bitmask (CTRL, SHIFT, ALT, GUI). 
* `keycodes` Array of up to 6 pressed keycodes (or NULL for no key). 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_hid_keyboard.c`

