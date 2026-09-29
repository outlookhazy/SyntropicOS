

# File syn\_usb.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb.h**](syn__usb_8h.md)

[Go to the source code of this file](syn__usb_8h_source.md)

_Zero-Heap Modular USB 2.0 Device Core Protocol Engine._ 

* `#include "syntropic/common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include "syntropic/pt/syn_pt.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_USB\_ClassDriver**](structSYN__USB__ClassDriver.md) <br> |
| struct | [**SYN\_USB\_Device**](structSYN__USB__Device.md) <br> |
| struct | [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_init**](#function-syn_usb_init) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, const uint8\_t \* device\_desc) <br>_Initialize USB Device Core with standard 18-byte Device Descriptor._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_is\_configured**](#function-syn_usb_is_configured) (const [**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev) <br>_Check if USB device is in CONFIGURED state._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_process\_setup**](#function-syn_usb_process_setup) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, const [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* pkt, uint8\_t \* resp, uint16\_t \* rlen) <br>_Process Control Setup packet from Host (EP0)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_register\_class**](#function-syn_usb_register_class) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, const [**SYN\_USB\_ClassDriver**](structSYN__USB__ClassDriver.md) \* cls, const uint8\_t \* iface\_desc, uint16\_t iface\_desc\_len) <br>_Register a class driver and append its interface descriptors._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_set\_raw\_config\_desc**](#function-syn_usb_set_raw_config_desc) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, const uint8\_t \* desc, uint16\_t len) <br>_Set raw configuration descriptor pointer (overrides auto-assembly)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_set\_string\_desc**](#function-syn_usb_set_string_desc) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, uint8\_t index, const uint8\_t \* desc) <br>_Set string descriptor pointer for a given index._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_USB\_WAIT\_CONFIGURED**](syn__usb_8h.md#define-pt_usb_wait_configured) (pt, dev) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, [**syn\_usb\_is\_configured**](syn__usb_8c.md#function-syn_usb_is_configured)(dev))`<br>_Block a protothread coroutine until USB device reaches CONFIGURED state._  |
| define  | [**SYN\_USB\_DESC\_TYPE\_CONFIGURATION**](syn__usb_8h.md#define-syn_usb_desc_type_configuration)  `0x02U`<br> |
| define  | [**SYN\_USB\_DESC\_TYPE\_DEVICE**](syn__usb_8h.md#define-syn_usb_desc_type_device)  `0x01U`<br> |
| define  | [**SYN\_USB\_DESC\_TYPE\_ENDPOINT**](syn__usb_8h.md#define-syn_usb_desc_type_endpoint)  `0x05U`<br> |
| define  | [**SYN\_USB\_DESC\_TYPE\_INTERFACE**](syn__usb_8h.md#define-syn_usb_desc_type_interface)  `0x04U`<br> |
| define  | [**SYN\_USB\_DESC\_TYPE\_STRING**](syn__usb_8h.md#define-syn_usb_desc_type_string)  `0x03U`<br> |
| define  | [**SYN\_USB\_EP0\_BUF\_SIZE**](syn__usb_8h.md#define-syn_usb_ep0_buf_size)  `64U`<br> |
| define  | [**SYN\_USB\_MAX\_CLASSES**](syn__usb_8h.md#define-syn_usb_max_classes)  `4U`<br> |
| define  | [**SYN\_USB\_MAX\_CONFIG\_DESC**](syn__usb_8h.md#define-syn_usb_max_config_desc)  `256U`<br> |
| define  | [**SYN\_USB\_MAX\_STRING\_DESCS**](syn__usb_8h.md#define-syn_usb_max_string_descs)  `8U`<br> |
| define  | [**SYN\_USB\_REQ\_CLEAR\_FEATURE**](syn__usb_8h.md#define-syn_usb_req_clear_feature)  `0x01U`<br> |
| define  | [**SYN\_USB\_REQ\_GET\_CONFIGURATION**](syn__usb_8h.md#define-syn_usb_req_get_configuration)  `0x08U`<br> |
| define  | [**SYN\_USB\_REQ\_GET\_DESCRIPTOR**](syn__usb_8h.md#define-syn_usb_req_get_descriptor)  `0x06U`<br> |
| define  | [**SYN\_USB\_REQ\_GET\_STATUS**](syn__usb_8h.md#define-syn_usb_req_get_status)  `0x00U`<br> |
| define  | [**SYN\_USB\_REQ\_SET\_ADDRESS**](syn__usb_8h.md#define-syn_usb_req_set_address)  `0x05U`<br> |
| define  | [**SYN\_USB\_REQ\_SET\_CONFIGURATION**](syn__usb_8h.md#define-syn_usb_req_set_configuration)  `0x09U`<br> |
| define  | [**SYN\_USB\_REQ\_SET\_DESCRIPTOR**](syn__usb_8h.md#define-syn_usb_req_set_descriptor)  `0x07U`<br> |
| define  | [**SYN\_USB\_REQ\_SET\_FEATURE**](syn__usb_8h.md#define-syn_usb_req_set_feature)  `0x03U`<br> |
| define  | [**SYN\_USB\_STATE\_ADDRESS**](syn__usb_8h.md#define-syn_usb_state_address)  `1U`<br> |
| define  | [**SYN\_USB\_STATE\_CONFIGURED**](syn__usb_8h.md#define-syn_usb_state_configured)  `2U`<br> |
| define  | [**SYN\_USB\_STATE\_DEFAULT**](syn__usb_8h.md#define-syn_usb_state_default)  `0U`<br> |

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
## Macro Definition Documentation





### define PT\_USB\_WAIT\_CONFIGURED 

_Block a protothread coroutine until USB device reaches CONFIGURED state._ 
```C++
#define PT_USB_WAIT_CONFIGURED (
    pt,
    dev
) `PT_WAIT_UNTIL (pt, syn_usb_is_configured (dev))`
```





**Parameters:**


* `pt` Protothread context. 
* `dev` Pointer to USB device instance. 




        

<hr>



### define SYN\_USB\_DESC\_TYPE\_CONFIGURATION 

```C++
#define SYN_USB_DESC_TYPE_CONFIGURATION `0x02U`
```



Configuration Descriptor Type (0x02) 


        

<hr>



### define SYN\_USB\_DESC\_TYPE\_DEVICE 

```C++
#define SYN_USB_DESC_TYPE_DEVICE `0x01U`
```



Descriptor Types Device Descriptor Type (0x01) 


        

<hr>



### define SYN\_USB\_DESC\_TYPE\_ENDPOINT 

```C++
#define SYN_USB_DESC_TYPE_ENDPOINT `0x05U`
```



Endpoint Descriptor Type (0x05) 


        

<hr>



### define SYN\_USB\_DESC\_TYPE\_INTERFACE 

```C++
#define SYN_USB_DESC_TYPE_INTERFACE `0x04U`
```



Interface Descriptor Type (0x04) 


        

<hr>



### define SYN\_USB\_DESC\_TYPE\_STRING 

```C++
#define SYN_USB_DESC_TYPE_STRING `0x03U`
```



String Descriptor Type (0x03) 


        

<hr>



### define SYN\_USB\_EP0\_BUF\_SIZE 

```C++
#define SYN_USB_EP0_BUF_SIZE `64U`
```



EP0 control buffer size 


        

<hr>



### define SYN\_USB\_MAX\_CLASSES 

```C++
#define SYN_USB_MAX_CLASSES `4U`
```



Maximum registered class drivers 


        

<hr>



### define SYN\_USB\_MAX\_CONFIG\_DESC 

```C++
#define SYN_USB_MAX_CONFIG_DESC `256U`
```



Maximum auto-assembled configuration descriptor buffer capacity 


        

<hr>



### define SYN\_USB\_MAX\_STRING\_DESCS 

```C++
#define SYN_USB_MAX_STRING_DESCS `8U`
```



Maximum string descriptor slots 


        

<hr>



### define SYN\_USB\_REQ\_CLEAR\_FEATURE 

```C++
#define SYN_USB_REQ_CLEAR_FEATURE `0x01U`
```



USB Request Clear Feature (0x01) 


        

<hr>



### define SYN\_USB\_REQ\_GET\_CONFIGURATION 

```C++
#define SYN_USB_REQ_GET_CONFIGURATION `0x08U`
```



USB Request Get Configuration (0x08) 


        

<hr>



### define SYN\_USB\_REQ\_GET\_DESCRIPTOR 

```C++
#define SYN_USB_REQ_GET_DESCRIPTOR `0x06U`
```



USB Request Get Descriptor (0x06) 


        

<hr>



### define SYN\_USB\_REQ\_GET\_STATUS 

```C++
#define SYN_USB_REQ_GET_STATUS `0x00U`
```



Standard USB Request Codes per USB 2.0 Spec Table 9-4 USB Request Get Status (0x00) 


        

<hr>



### define SYN\_USB\_REQ\_SET\_ADDRESS 

```C++
#define SYN_USB_REQ_SET_ADDRESS `0x05U`
```



USB Request Set Address (0x05) 


        

<hr>



### define SYN\_USB\_REQ\_SET\_CONFIGURATION 

```C++
#define SYN_USB_REQ_SET_CONFIGURATION `0x09U`
```



USB Request Set Configuration (0x09) 


        

<hr>



### define SYN\_USB\_REQ\_SET\_DESCRIPTOR 

```C++
#define SYN_USB_REQ_SET_DESCRIPTOR `0x07U`
```



USB Request Set Descriptor (0x07) 


        

<hr>



### define SYN\_USB\_REQ\_SET\_FEATURE 

```C++
#define SYN_USB_REQ_SET_FEATURE `0x03U`
```



USB Request Set Feature (0x03) 


        

<hr>



### define SYN\_USB\_STATE\_ADDRESS 

```C++
#define SYN_USB_STATE_ADDRESS `1U`
```



USB Address state 


        

<hr>



### define SYN\_USB\_STATE\_CONFIGURED 

```C++
#define SYN_USB_STATE_CONFIGURED `2U`
```



USB Configured state 


        

<hr>



### define SYN\_USB\_STATE\_DEFAULT 

```C++
#define SYN_USB_STATE_DEFAULT `0U`
```



USB Device States per USB 2.0 Spec §9.1.1 USB Default state 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb.h`

