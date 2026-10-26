

# Struct SYN\_USB\_SetupPacket



[**ClassList**](annotated.md) **>** [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md)



[More...](#detailed-description)

* `#include <syn_usb.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**bRequest**](#variable-brequest)  <br> |
|  uint8\_t | [**bmRequestType**](#variable-bmrequesttype)  <br> |
|  uint16\_t | [**wIndex**](#variable-windex)  <br> |
|  uint16\_t | [**wLength**](#variable-wlength)  <br> |
|  uint16\_t | [**wValue**](#variable-wvalue)  <br> |












































## Detailed Description


USB Setup Packet Format 


    
## Public Attributes Documentation




### variable bRequest 

```C++
uint8_t SYN_USB_SetupPacket::bRequest;
```



Specific request code 


        

<hr>



### variable bmRequestType 

```C++
uint8_t SYN_USB_SetupPacket::bmRequestType;
```



Characteristics of request 


        

<hr>



### variable wIndex 

```C++
uint16_t SYN_USB_SetupPacket::wIndex;
```



Interface/endpoint index 


        

<hr>



### variable wLength 

```C++
uint16_t SYN_USB_SetupPacket::wLength;
```



Transfer length 


        

<hr>



### variable wValue 

```C++
uint16_t SYN_USB_SetupPacket::wValue;
```



Word-sized parameter 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb.h`

