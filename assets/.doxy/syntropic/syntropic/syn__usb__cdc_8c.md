

# File syn\_usb\_cdc.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_cdc.c**](syn__usb__cdc_8c.md)

[Go to the source code of this file](syn__usb__cdc_8c_source.md)

_Zero-Heap USB 2.0 CDC ACM Virtual COM Port Class Engine Implementation._ 

* `#include "syntropic/drivers/syn_usb_cdc.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t | [**CDC\_INTERFACE\_DESC**](#variable-cdc_interface_desc)   = `/* multi line expression */`<br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_cdc\_handle\_setup**](#function-syn_usb_cdc_handle_setup) ([**SYN\_USB\_CDC**](structSYN__USB__CDC.md) \* cdc, const [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* setup, uint8\_t \* resp, size\_t \* rlen) <br>_Handle Control Setup Request from host (EP0)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_cdc\_init**](#function-syn_usb_cdc_init) ([**SYN\_USB\_CDC**](structSYN__USB__CDC.md) \* cdc) <br>_Initialize USB CDC Class instance with defaults (115200 baud, 8N1)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_cdc\_read**](#function-syn_usb_cdc_read) ([**SYN\_USB\_CDC**](structSYN__USB__CDC.md) \* cdc, void \* buf, size\_t max\_len, size\_t \* out\_len) <br>_Read bytes from USB CDC Bulk OUT receive buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_cdc\_register**](#function-syn_usb_cdc_register) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, [**SYN\_USB\_CDC**](structSYN__USB__CDC.md) \* cdc) <br>_Register USB CDC ACM class driver with USB device core._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_cdc\_rx\_available**](#function-syn_usb_cdc_rx_available) (const [**SYN\_USB\_CDC**](structSYN__USB__CDC.md) \* cdc) <br>_Check if receive data is available._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_cdc\_tx\_ready**](#function-syn_usb_cdc_tx_ready) (const [**SYN\_USB\_CDC**](structSYN__USB__CDC.md) \* cdc) <br>_Check if transmit buffer is ready._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_cdc\_write**](#function-syn_usb_cdc_write) ([**SYN\_USB\_CDC**](structSYN__USB__CDC.md) \* cdc, const void \* data, size\_t len) <br>_Write bytes to USB CDC Bulk IN transmit buffer._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**cdc\_class\_configured**](#function-cdc_class_configured) (void \* ctx, uint8\_t config) <br>_Configured callback wrapper for CDC class._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**cdc\_class\_setup**](#function-cdc_class_setup) (void \* ctx, const [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* pkt, uint8\_t \* resp, uint16\_t \* rlen) <br>_Setup request callback wrapper for CDC class requests._  |


























## Public Static Attributes Documentation




### variable CDC\_INTERFACE\_DESC 

```C++
const uint8_t CDC_INTERFACE_DESC[67];
```



Standard CDC Interface Descriptor Assembly (2 interfaces, 3 endpoints, 67 bytes) 


        

<hr>
## Public Functions Documentation




### function syn\_usb\_cdc\_handle\_setup 

_Handle Control Setup Request from host (EP0)._ 
```C++
SYN_Status syn_usb_cdc_handle_setup (
    SYN_USB_CDC * cdc,
    const SYN_USB_SetupPacket * setup,
    uint8_t * resp,
    size_t * rlen
) 
```





**Parameters:**


* `cdc` Pointer to USB CDC instance. 
* `setup` Pointer to setup packet. 
* `resp` Pointer to response buffer. 
* `rlen` Pointer to receive response length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_cdc\_init 

_Initialize USB CDC Class instance with defaults (115200 baud, 8N1)._ 
```C++
SYN_Status syn_usb_cdc_init (
    SYN_USB_CDC * cdc
) 
```





**Parameters:**


* `cdc` Pointer to USB CDC instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_cdc\_read 

_Read bytes from USB CDC Bulk OUT receive buffer._ 
```C++
SYN_Status syn_usb_cdc_read (
    SYN_USB_CDC * cdc,
    void * buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `cdc` Pointer to USB CDC instance. 
* `buf` Pointer to destination buffer. 
* `max_len` Max capacity. 
* `out_len` Pointer to receive read byte count. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_cdc\_register 

_Register USB CDC ACM class driver with USB device core._ 
```C++
SYN_Status syn_usb_cdc_register (
    SYN_USB_Device * dev,
    SYN_USB_CDC * cdc
) 
```





**Parameters:**


* `dev` Pointer to USB device core context. 
* `cdc` Pointer to USB CDC instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_cdc\_rx\_available 

_Check if receive data is available._ 
```C++
bool syn_usb_cdc_rx_available (
    const SYN_USB_CDC * cdc
) 
```





**Parameters:**


* `cdc` Pointer to USB CDC instance. 



**Returns:**

true if unread data is present in rx\_buf. 





        

<hr>



### function syn\_usb\_cdc\_tx\_ready 

_Check if transmit buffer is ready._ 
```C++
bool syn_usb_cdc_tx_ready (
    const SYN_USB_CDC * cdc
) 
```





**Parameters:**


* `cdc` Pointer to USB CDC instance. 



**Returns:**

true if transmit buffer is available. 





        

<hr>



### function syn\_usb\_cdc\_write 

_Write bytes to USB CDC Bulk IN transmit buffer._ 
```C++
SYN_Status syn_usb_cdc_write (
    SYN_USB_CDC * cdc,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `cdc` Pointer to USB CDC instance. 
* `data` Pointer to data bytes. 
* `len` Byte length. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Public Static Functions Documentation




### function cdc\_class\_configured 

_Configured callback wrapper for CDC class._ 
```C++
static SYN_Status cdc_class_configured (
    void * ctx,
    uint8_t config
) 
```





**Parameters:**


* `ctx` Pointer to [**SYN\_USB\_CDC**](structSYN__USB__CDC.md) instance. 
* `config` Configuration index (0 = unconfigured). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function cdc\_class\_setup 

_Setup request callback wrapper for CDC class requests._ 
```C++
static SYN_Status cdc_class_setup (
    void * ctx,
    const SYN_USB_SetupPacket * pkt,
    uint8_t * resp,
    uint16_t * rlen
) 
```





**Parameters:**


* `ctx` Pointer to [**SYN\_USB\_CDC**](structSYN__USB__CDC.md) instance. 
* `pkt` Pointer to received Setup Packet. 
* `resp` Output buffer for data stage response. 
* `rlen` Pointer to receive response length. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_cdc.c`

