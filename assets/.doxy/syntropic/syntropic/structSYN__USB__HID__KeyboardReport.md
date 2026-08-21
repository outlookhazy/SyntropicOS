

# Struct SYN\_USB\_HID\_KeyboardReport



[**ClassList**](annotated.md) **>** [**SYN\_USB\_HID\_KeyboardReport**](structSYN__USB__HID__KeyboardReport.md)



[More...](#detailed-description)

* `#include <syn_usb_hid_keyboard.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**keycode**](#variable-keycode)  <br> |
|  uint8\_t | [**modifier**](#variable-modifier)  <br> |
|  uint8\_t | [**reserved**](#variable-reserved)  <br> |












































## Detailed Description


Standard 8-byte USB HID Boot Keyboard IN Report structure 


    
## Public Attributes Documentation




### variable keycode 

```C++
uint8_t SYN_USB_HID_KeyboardReport::keycode[6];
```



Keycodes for up to 6 pressed keys 
 


        

<hr>



### variable modifier 

```C++
uint8_t SYN_USB_HID_KeyboardReport::modifier;
```



Modifier keys bitmask (CTRL, SHIFT, ALT, GUI) 


        

<hr>



### variable reserved 

```C++
uint8_t SYN_USB_HID_KeyboardReport::reserved;
```



Reserved byte (0x00) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_hid_keyboard.h`

