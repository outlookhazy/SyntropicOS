

# Struct SYN\_USB\_Device



[**ClassList**](annotated.md) **>** [**SYN\_USB\_Device**](structSYN__USB__Device.md)



[More...](#detailed-description)

* `#include <syn_usb.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**class\_count**](#variable-class_count)  <br> |
|  [**SYN\_USB\_ClassDriver**](structSYN__USB__ClassDriver.md) | [**classes**](#variable-classes)  <br> |
|  uint8\_t | [**config\_buf**](#variable-config_buf)  <br> |
|  uint16\_t | [**config\_buf\_used**](#variable-config_buf_used)  <br> |
|  const uint8\_t \* | [**config\_desc**](#variable-config_desc)  <br> |
|  uint16\_t | [**config\_desc\_len**](#variable-config_desc_len)  <br> |
|  uint8\_t | [**config\_value**](#variable-config_value)  <br> |
|  uint8\_t | [**dev\_address**](#variable-dev_address)  <br> |
|  const uint8\_t \* | [**device\_desc**](#variable-device_desc)  <br> |
|  uint8\_t | [**ep0\_buf**](#variable-ep0_buf)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**raw\_config\_override**](#variable-raw_config_override)  <br> |
|  uint8\_t | [**state**](#variable-state)  <br> |
|  uint8\_t | [**string\_desc\_count**](#variable-string_desc_count)  <br> |
|  const uint8\_t \* | [**string\_descs**](#variable-string_descs)  <br> |












































## Detailed Description


USB Device Instance Context 


    
## Public Attributes Documentation




### variable class\_count 

```C++
uint8_t SYN_USB_Device::class_count;
```



Number of registered class drivers 


        

<hr>



### variable classes 

```C++
SYN_USB_ClassDriver SYN_USB_Device::classes[SYN_USB_MAX_CLASSES];
```



Class driver registry array 


        

<hr>



### variable config\_buf 

```C++
uint8_t SYN_USB_Device::config_buf[SYN_USB_MAX_CONFIG_DESC];
```



Auto-assembled config descriptor buffer 


        

<hr>



### variable config\_buf\_used 

```C++
uint16_t SYN_USB_Device::config_buf_used;
```



Length of auto-assembled config descriptor 


        

<hr>



### variable config\_desc 

```C++
const uint8_t* SYN_USB_Device::config_desc;
```



Active configuration descriptor pointer 


        

<hr>



### variable config\_desc\_len 

```C++
uint16_t SYN_USB_Device::config_desc_len;
```



Configuration descriptor length 


        

<hr>



### variable config\_value 

```C++
uint8_t SYN_USB_Device::config_value;
```



Active configuration value 


        

<hr>



### variable dev\_address 

```C++
uint8_t SYN_USB_Device::dev_address;
```



Assigned device bus address 


        

<hr>



### variable device\_desc 

```C++
const uint8_t* SYN_USB_Device::device_desc;
```



Pointer to 18-byte Device Descriptor 


        

<hr>



### variable ep0\_buf 

```C++
uint8_t SYN_USB_Device::ep0_buf[SYN_USB_EP0_BUF_SIZE];
```



Internal EP0 control buffer 


        

<hr>



### variable raw\_config\_override 

```C++
bool SYN_USB_Device::raw_config_override;
```



True if raw config descriptor set 


        

<hr>



### variable state 

```C++
uint8_t SYN_USB_Device::state;
```



USB Device State (SYN\_USB\_STATE\_\*) 


        

<hr>



### variable string\_desc\_count 

```C++
uint8_t SYN_USB_Device::string_desc_count;
```



String descriptor count 


        

<hr>



### variable string\_descs 

```C++
const uint8_t* SYN_USB_Device::string_descs[SYN_USB_MAX_STRING_DESCS];
```



String descriptor pointers 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb.h`

