

# Struct SYN\_USB\_Host



[**ClassList**](annotated.md) **>** [**SYN\_USB\_Host**](structSYN__USB__Host.md)



[More...](#detailed-description)

* `#include <syn_usb_host.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**class\_count**](#variable-class_count)  <br> |
|  [**SYN\_USB\_HostClassDriver**](structSYN__USB__HostClassDriver.md) | [**classes**](#variable-classes)  <br> |
|  [**SYN\_USB\_HostDevInfo**](structSYN__USB__HostDevInfo.md) | [**dev\_info**](#variable-dev_info)  <br> |
|  uint8\_t | [**enum\_buf**](#variable-enum_buf)  <br> |
|  uint16\_t | [**enum\_buf\_len**](#variable-enum_buf_len)  <br> |
|  uint8\_t | [**enum\_step**](#variable-enum_step)  <br> |
|  uint8\_t | [**next\_addr**](#variable-next_addr)  <br> |
|  uint8\_t | [**state**](#variable-state)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**xfer\_pending**](#variable-xfer_pending)  <br> |












































## Detailed Description


USB Host Instance Context 


    
## Public Attributes Documentation




### variable class\_count 

```C++
uint8_t SYN_USB_Host::class_count;
```



Number of registered class drivers 


        

<hr>



### variable classes 

```C++
SYN_USB_HostClassDriver SYN_USB_Host::classes[SYN_USB_HOST_MAX_CLASSES];
```



Class driver registry 


        

<hr>



### variable dev\_info 

```C++
SYN_USB_HostDevInfo SYN_USB_Host::dev_info;
```



Cached device descriptor info 


        

<hr>



### variable enum\_buf 

```C++
uint8_t SYN_USB_Host::enum_buf[SYN_USB_HOST_ENUM_BUF_SIZE];
```



Descriptor scratch buffer 


        

<hr>



### variable enum\_buf\_len 

```C++
uint16_t SYN_USB_Host::enum_buf_len;
```



Valid bytes in enum\_buf 


        

<hr>



### variable enum\_step 

```C++
uint8_t SYN_USB_Host::enum_step;
```



Enumeration sub-step 


        

<hr>



### variable next\_addr 

```C++
uint8_t SYN_USB_Host::next_addr;
```



Next device address to assign (1..127) 


        

<hr>



### variable state 

```C++
uint8_t SYN_USB_Host::state;
```



Host state (SYN\_USB\_HOST\_STATE\_\*) 


        

<hr>



### variable xfer\_pending 

```C++
bool SYN_USB_Host::xfer_pending;
```



True if a HAL transfer is in-flight 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_host.h`

