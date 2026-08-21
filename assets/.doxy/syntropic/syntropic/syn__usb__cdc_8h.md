

# File syn\_usb\_cdc.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_cdc.h**](syn__usb__cdc_8h.md)

[Go to the source code of this file](syn__usb__cdc_8h_source.md)

_Zero-Heap USB 2.0 CDC ACM Virtual COM Port Class Driver._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/drivers/syn_usb.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include "syntropic/pt/syn_pt.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_USB\_CDC**](structSYN__USB__CDC.md) <br> |
| struct | [**SYN\_USB\_CDC\_LineCoding**](structSYN__USB__CDC__LineCoding.md) <br> |






















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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_USB\_CDC\_WAIT\_RX**](syn__usb__cdc_8h.md#define-pt_usb_cdc_wait_rx) (pt, cdc) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, [**syn\_usb\_cdc\_rx\_available**](syn__usb__cdc_8c.md#function-syn_usb_cdc_rx_available)(cdc))`<br>_Block a protothread coroutine until CDC RX data is available._  |
| define  | [**PT\_USB\_CDC\_WAIT\_TX\_READY**](syn__usb__cdc_8h.md#define-pt_usb_cdc_wait_tx_ready) (pt, cdc) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, [**syn\_usb\_cdc\_tx\_ready**](syn__usb__cdc_8c.md#function-syn_usb_cdc_tx_ready)(cdc))`<br>_Block a protothread coroutine until CDC TX buffer is ready._  |
| define  | [**SYN\_USB\_CDC\_GET\_LINE\_CODING**](syn__usb__cdc_8h.md#define-syn_usb_cdc_get_line_coding)  `0x21U`<br> |
| define  | [**SYN\_USB\_CDC\_SET\_CONTROL\_LINE\_STATE**](syn__usb__cdc_8h.md#define-syn_usb_cdc_set_control_line_state)  `0x22U`<br> |
| define  | [**SYN\_USB\_CDC\_SET\_LINE\_CODING**](syn__usb__cdc_8h.md#define-syn_usb_cdc_set_line_coding)  `0x20U`<br> |

## Detailed Description


USB CDC specifications:
* Interface 0: CDC Communication Class (Interrupt EP 0x82 / 8B).
* Interface 1: CDC Data Class (Bulk IN EP 0x81 / 64B, Bulk OUT EP 0x01 / 64B). 




    
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
## Macro Definition Documentation





### define PT\_USB\_CDC\_WAIT\_RX 

_Block a protothread coroutine until CDC RX data is available._ 
```C++
#define PT_USB_CDC_WAIT_RX (
    pt,
    cdc
) `PT_WAIT_UNTIL (pt, syn_usb_cdc_rx_available (cdc))`
```





**Parameters:**


* `pt` Protothread context. 
* `cdc` Pointer to USB CDC instance. 




        

<hr>



### define PT\_USB\_CDC\_WAIT\_TX\_READY 

_Block a protothread coroutine until CDC TX buffer is ready._ 
```C++
#define PT_USB_CDC_WAIT_TX_READY (
    pt,
    cdc
) `PT_WAIT_UNTIL (pt, syn_usb_cdc_tx_ready (cdc))`
```





**Parameters:**


* `pt` Protothread context. 
* `cdc` Pointer to USB CDC instance. 




        

<hr>



### define SYN\_USB\_CDC\_GET\_LINE\_CODING 

```C++
#define SYN_USB_CDC_GET_LINE_CODING `0x21U`
```



CDC Request Get Line Coding (0x21) 


        

<hr>



### define SYN\_USB\_CDC\_SET\_CONTROL\_LINE\_STATE 

```C++
#define SYN_USB_CDC_SET_CONTROL_LINE_STATE `0x22U`
```



CDC Request Set Control Line State (0x22) 


        

<hr>



### define SYN\_USB\_CDC\_SET\_LINE\_CODING 

```C++
#define SYN_USB_CDC_SET_LINE_CODING `0x20U`
```



CDC Request Set Line Coding (0x20) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_cdc.h`

