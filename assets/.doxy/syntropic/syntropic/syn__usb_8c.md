

# File syn\_usb.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb.c**](syn__usb_8c.md)

[Go to the source code of this file](syn__usb_8c_source.md)

_Zero-Heap Modular USB 2.0 Device Core Protocol Engine Implementation._ 

* `#include "syntropic/drivers/syn_usb.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t | [**CONFIG\_HEADER\_TEMPLATE**](#variable-config_header_template)   = `/* multi line expression */`<br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_init**](#function-syn_usb_init) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, const uint8\_t \* device\_desc) <br>_Initialize USB Device Core with standard 18-byte Device Descriptor._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_is\_configured**](#function-syn_usb_is_configured) (const [**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev) <br>_Check if USB device is in CONFIGURED state._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_process\_setup**](#function-syn_usb_process_setup) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, const [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* pkt, uint8\_t \* resp, uint16\_t \* rlen) <br>_Process Control Setup packet from Host (EP0)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_register\_class**](#function-syn_usb_register_class) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, const [**SYN\_USB\_ClassDriver**](structSYN__USB__ClassDriver.md) \* cls, const uint8\_t \* iface\_desc, uint16\_t iface\_desc\_len) <br>_Register a class driver and append its interface descriptors._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_set\_raw\_config\_desc**](#function-syn_usb_set_raw_config_desc) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, const uint8\_t \* desc, uint16\_t len) <br>_Set raw configuration descriptor pointer (overrides auto-assembly)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_set\_string\_desc**](#function-syn_usb_set_string_desc) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, uint8\_t index, const uint8\_t \* desc) <br>_Set string descriptor pointer for a given index._  |




























## Public Static Attributes Documentation




### variable CONFIG\_HEADER\_TEMPLATE 

```C++
const uint8_t CONFIG_HEADER_TEMPLATE[9];
```



Standard Configuration Descriptor Header Template (9 bytes) 


        

<hr>
## Public Functions Documentation




### function syn\_usb\_init 

_Initialize USB Device Core with standard 18-byte Device Descriptor._ 
```C++
SYN_Status syn_usb_init (
    SYN_USB_Device * dev,
    const uint8_t * device_desc
) 
```





**Parameters:**


* `dev` Pointer to USB device instance. 
* `device_desc` Pointer to 18-byte standard USB device descriptor. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_is\_configured 

_Check if USB device is in CONFIGURED state._ 
```C++
bool syn_usb_is_configured (
    const SYN_USB_Device * dev
) 
```





**Parameters:**


* `dev` Pointer to USB device context. 



**Returns:**

true if device is configured. 





        

<hr>



### function syn\_usb\_process\_setup 

_Process Control Setup packet from Host (EP0)._ 
```C++
SYN_Status syn_usb_process_setup (
    SYN_USB_Device * dev,
    const SYN_USB_SetupPacket * pkt,
    uint8_t * resp,
    uint16_t * rlen
) 
```





**Parameters:**


* `dev` Pointer to USB device context. 
* `pkt` Pointer to received Setup Packet. 
* `resp` Output buffer for data stage response. 
* `rlen` Output pointer to receive response byte length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_register\_class 

_Register a class driver and append its interface descriptors._ 
```C++
SYN_Status syn_usb_register_class (
    SYN_USB_Device * dev,
    const SYN_USB_ClassDriver * cls,
    const uint8_t * iface_desc,
    uint16_t iface_desc_len
) 
```





**Parameters:**


* `dev` Pointer to USB device context. 
* `cls` Class driver vtable and binding context. 
* `iface_desc` Pointer to class interface descriptor payload. 
* `iface_desc_len` Length of interface descriptor payload. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_set\_raw\_config\_desc 

_Set raw configuration descriptor pointer (overrides auto-assembly)._ 
```C++
SYN_Status syn_usb_set_raw_config_desc (
    SYN_USB_Device * dev,
    const uint8_t * desc,
    uint16_t len
) 
```





**Parameters:**


* `dev` Pointer to USB device context. 
* `desc` Pointer to raw configuration descriptor. 
* `len` Length in bytes. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_set\_string\_desc 

_Set string descriptor pointer for a given index._ 
```C++
SYN_Status syn_usb_set_string_desc (
    SYN_USB_Device * dev,
    uint8_t index,
    const uint8_t * desc
) 
```





**Parameters:**


* `dev` Pointer to USB device context. 
* `index` String index (0=Language ID, 1=Manufacturer, 2=Product, etc). 
* `desc` Pointer to UNICODE string descriptor. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb.c`

