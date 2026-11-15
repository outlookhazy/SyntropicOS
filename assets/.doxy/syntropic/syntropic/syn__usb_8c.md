

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
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_alloc\_ep**](#function-syn_usb_alloc_ep) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, uint8\_t preferred\_ep, uint8\_t \* assigned\_ep) <br>_Allocate a non-colliding endpoint address on a USB device._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_init**](#function-syn_usb_init) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, const uint8\_t \* device\_desc) <br>_Initialize USB Device Core with standard 18-byte Device Descriptor._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_is\_configured**](#function-syn_usb_is_configured) (const [**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev) <br>_Check if USB device is in CONFIGURED state._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_process\_setup**](#function-syn_usb_process_setup) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, const [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* pkt, uint8\_t \* resp, uint16\_t resp\_cap, uint16\_t \* rlen) <br>_Process Control Setup packet from Host (EP0)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_register\_class**](#function-syn_usb_register_class) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, const [**SYN\_USB\_ClassDriver**](structSYN__USB__ClassDriver.md) \* cls, const uint8\_t \* iface\_desc, uint16\_t iface\_desc\_len) <br>_Register a class driver and append its interface descriptors._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_set\_raw\_config\_desc**](#function-syn_usb_set_raw_config_desc) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, const uint8\_t \* desc, uint16\_t len) <br>_Set raw configuration descriptor pointer (overrides auto-assembly)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_set\_string\_desc**](#function-syn_usb_set_string_desc) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, uint8\_t index, const uint8\_t \* desc) <br>_Set string descriptor pointer for a given index._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**usb\_apply\_configuration**](#function-usb_apply_configuration) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, uint8\_t config\_val) <br>_Apply SET\_CONFIGURATION state transition and notify registered class drivers._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**usb\_handle\_get\_descriptor**](#function-usb_handle_get_descriptor) (const [**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, const [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* pkt, uint8\_t \* resp, uint16\_t resp\_cap, uint16\_t \* rlen) <br>_Handle standard USB GET\_DESCRIPTOR setup requests on EP0._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**usb\_is\_class\_routed\_setup**](#function-usb_is_class_routed_setup) (const [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* pkt) <br>_Check whether an EP0 Setup packet should be dispatched to a registered class driver._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**usb\_patch\_class\_desc**](#function-usb_patch_class_desc) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, uint8\_t \* buf, uint16\_t len, uint8\_t base\_iface) <br>_Walk a class descriptor block and patch interface/endpoint numbers._  |
|  void | [**usb\_patch\_cs\_iface**](#function-usb_patch_cs_iface) (uint8\_t \* d, uint8\_t b\_len, uint8\_t cur\_class, int16\_t delta) <br>_Patch class-specific functional descriptors referencing interface indices._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**usb\_patch\_single\_desc**](#function-usb_patch_single_desc) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, uint8\_t \* d, uint8\_t b\_len, int16\_t delta, uint8\_t \* cur\_class) <br>_Patch a single descriptor entry for dynamic interface and endpoint assignment._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**usb\_route\_class\_setup**](#function-usb_route_class_setup) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, const [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* pkt, uint8\_t \* resp, uint16\_t resp\_cap, uint16\_t \* rlen, [**bool**](syn__defs_8h.md#enum-bool) \* handled) <br>_Route class-specific or interface-targeted setup packets to matching driver._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**usb\_write\_resp**](#function-usb_write_resp) (uint8\_t \* resp, uint16\_t resp\_cap, const uint8\_t \* src, uint16\_t len, uint16\_t \* rlen) <br>_Copy an EP0 IN stage payload into_ `resp` _after checking_`resp_cap` _._ |


























## Public Static Attributes Documentation




### variable CONFIG\_HEADER\_TEMPLATE 

```C++
const uint8_t CONFIG_HEADER_TEMPLATE[9];
```



Standard Configuration Descriptor Header Template (9 bytes) 


        

<hr>
## Public Functions Documentation




### function syn\_usb\_alloc\_ep 

_Allocate a non-colliding endpoint address on a USB device._ 
```C++
SYN_Status syn_usb_alloc_ep (
    SYN_USB_Device * dev,
    uint8_t preferred_ep,
    uint8_t * assigned_ep
) 
```



Uses `preferred_ep` when free, or assigns the lowest available endpoint number (1..15) in the same direction.




**Parameters:**


* `dev` Pointer to USB device context. 
* `preferred_ep` Preferred endpoint address (bit 7 = direction, bits 3..0 = number). 
* `assigned_ep` Output pointer receiving assigned endpoint address. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL, SYN\_ERROR if endpoints exhausted. 





        

<hr>



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
    uint16_t resp_cap,
    uint16_t * rlen
) 
```





**Parameters:**


* `dev` Pointer to USB device context. 
* `pkt` Pointer to received Setup Packet. 
* `resp` Output buffer for data stage response. 
* `resp_cap` Capacity of `resp` in bytes. 
* `rlen` Output pointer to receive response byte length. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if unsupported or response exceeds `resp_cap`. 





        

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
## Public Static Functions Documentation




### function usb\_apply\_configuration 

_Apply SET\_CONFIGURATION state transition and notify registered class drivers._ 
```C++
static void usb_apply_configuration (
    SYN_USB_Device * dev,
    uint8_t config_val
) 
```





**Parameters:**


* `dev` Pointer to USB device context. 
* `config_val` Configuration value from wValue[7:0]. 




        

<hr>



### function usb\_handle\_get\_descriptor 

_Handle standard USB GET\_DESCRIPTOR setup requests on EP0._ 
```C++
static SYN_Status usb_handle_get_descriptor (
    const SYN_USB_Device * dev,
    const SYN_USB_SetupPacket * pkt,
    uint8_t * resp,
    uint16_t resp_cap,
    uint16_t * rlen
) 
```





**Parameters:**


* `dev` Pointer to USB device context. 
* `pkt` Setup packet pointer. 
* `resp` Response buffer. 
* `resp_cap` Capacity of `resp` in bytes. 
* `rlen` Output response length pointer. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if descriptor is unsupported/missing or exceeds `resp_cap`. 





        

<hr>



### function usb\_is\_class\_routed\_setup 

_Check whether an EP0 Setup packet should be dispatched to a registered class driver._ 
```C++
static bool usb_is_class_routed_setup (
    const SYN_USB_SetupPacket * pkt
) 
```



Routes Class-type requests (bmRequestType[6:5] == 1) and Standard Interface-targeted GET\_DESCRIPTOR requests (e.g. HID Report Descriptor 0x22 on bmRequestType == 0x81). Standard Device/Interface requests such as SET\_ADDRESS, SET\_CONFIGURATION, and GET\_STATUS stay in the USB Chapter 9 core.




**Parameters:**


* `pkt` Setup packet pointer. 



**Returns:**

true when class-level routing applies. 





        

<hr>



### function usb\_patch\_class\_desc 

_Walk a class descriptor block and patch interface/endpoint numbers._ 
```C++
static SYN_Status usb_patch_class_desc (
    SYN_USB_Device * dev,
    uint8_t * buf,
    uint16_t len,
    uint8_t base_iface
) 
```





**Parameters:**


* `dev` Pointer to USB device context. 
* `buf` Mutable descriptor slice inside config\_buf. 
* `len` Byte length of descriptor slice. 
* `base_iface` Assigned first interface number for this class. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if endpoint allocation fails. 





        

<hr>



### function usb\_patch\_cs\_iface 

_Patch class-specific functional descriptors referencing interface indices._ 
```C++
static void usb_patch_cs_iface (
    uint8_t * d,
    uint8_t b_len,
    uint8_t cur_class,
    int16_t delta
) 
```





**Parameters:**


* `d` Pointer to CS\_INTERFACE descriptor. 
* `b_len` Descriptor byte length. 
* `cur_class` Active parent interface bInterfaceClass. 
* `delta` Interface number offset from template base. 




        

<hr>



### function usb\_patch\_single\_desc 

_Patch a single descriptor entry for dynamic interface and endpoint assignment._ 
```C++
static SYN_Status usb_patch_single_desc (
    SYN_USB_Device * dev,
    uint8_t * d,
    uint8_t b_len,
    int16_t delta,
    uint8_t * cur_class
) 
```





**Parameters:**


* `dev` Pointer to USB device context. 
* `d` Pointer to descriptor start. 
* `b_len` Descriptor byte length. 
* `delta` Interface offset from template base. 
* `cur_class` In/out active interface class code. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if endpoints are exhausted. 





        

<hr>



### function usb\_route\_class\_setup 

_Route class-specific or interface-targeted setup packets to matching driver._ 
```C++
static SYN_Status usb_route_class_setup (
    SYN_USB_Device * dev,
    const SYN_USB_SetupPacket * pkt,
    uint8_t * resp,
    uint16_t resp_cap,
    uint16_t * rlen,
    bool * handled
) 
```





**Parameters:**


* `dev` Pointer to USB device context. 
* `pkt` Setup packet pointer. 
* `resp` Response buffer. 
* `resp_cap` Capacity of `resp` in bytes. 
* `rlen` Output response length pointer. 
* `handled` Output flag set true when a matching class setup handler handled pkt. 



**Returns:**

Status returned by the matched class handler. 





        

<hr>



### function usb\_write\_resp 

_Copy an EP0 IN stage payload into_ `resp` _after checking_`resp_cap` _._
```C++
static SYN_Status usb_write_resp (
    uint8_t * resp,
    uint16_t resp_cap,
    const uint8_t * src,
    uint16_t len,
    uint16_t * rlen
) 
```





**Parameters:**


* `resp` Destination response buffer. 
* `resp_cap` Capacity of `resp` in bytes. 
* `src` Source payload bytes. 
* `len` Payload byte length. 
* `rlen` Output response length pointer. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if `len` exceeds `resp_cap`. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb.c`

