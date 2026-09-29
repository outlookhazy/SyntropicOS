

# Struct SYN\_USB\_HID\_MouseReport



[**ClassList**](annotated.md) **>** [**SYN\_USB\_HID\_MouseReport**](structSYN__USB__HID__MouseReport.md)



[More...](#detailed-description)

* `#include <syn_usb_hid_mouse.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**buttons**](#variable-buttons)  <br> |
|  int8\_t | [**wheel**](#variable-wheel)  <br> |
|  int8\_t | [**x**](#variable-x)  <br> |
|  int8\_t | [**y**](#variable-y)  <br> |












































## Detailed Description


Standard 4-byte USB HID Boot Mouse IN Report structure 


    
## Public Attributes Documentation




### variable buttons 

```C++
uint8_t SYN_USB_HID_MouseReport::buttons;
```



Button bitmask (Left=0x01, Right=0x02, Middle=0x04) 


        

<hr>



### variable wheel 

```C++
int8_t SYN_USB_HID_MouseReport::wheel;
```



Relative vertical scroll wheel (-127 to +127) 
 


        

<hr>



### variable x 

```C++
int8_t SYN_USB_HID_MouseReport::x;
```



Relative X displacement (-127 to +127) 
 


        

<hr>



### variable y 

```C++
int8_t SYN_USB_HID_MouseReport::y;
```



Relative Y displacement (-127 to +127) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_hid_mouse.h`

