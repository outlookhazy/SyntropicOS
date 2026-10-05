

# File syn\_usb\_host\_cdc.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_host\_cdc.h**](syn__usb__host__cdc_8h.md)

[Go to the source code of this file](syn__usb__host__cdc_8h_source.md)

_Zero-Heap USB 2.0 Host CDC ACM Class Driver._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/drivers/syn_usb_cdc.h"`
* `#include "syntropic/drivers/syn_usb_host.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include "syntropic/pt/syn_pt.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_host\_cdc\_init**](#function-syn_usb_host_cdc_init) ([**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) \* hcdc) <br>_Initialize USB Host CDC instance with defaults (115200 8N1)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_host\_cdc\_read**](#function-syn_usb_host_cdc_read) ([**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) \* hcdc, void \* buf, size\_t max\_len, size\_t \* out\_len) <br>_Read bytes from USB Host CDC RX buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_host\_cdc\_register**](#function-syn_usb_host_cdc_register) ([**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host, [**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) \* hcdc) <br>_Register USB Host CDC class driver with USB Host Core._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_host\_cdc\_rx\_available**](#function-syn_usb_host_cdc_rx_available) (const [**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) \* hcdc) <br>_Check if receive data is available._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_host\_cdc\_tx\_ready**](#function-syn_usb_host_cdc_tx_ready) (const [**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) \* hcdc) <br>_Check if transmit buffer is ready._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_host\_cdc\_write**](#function-syn_usb_host_cdc_write) ([**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) \* hcdc, const void \* data, size\_t len) <br>_Write bytes to USB Host CDC TX buffer._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_USB\_HOST\_CDC\_WAIT\_RX**](syn__usb__host__cdc_8h.md#define-pt_usb_host_cdc_wait_rx) (pt, hcdc) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, [**syn\_usb\_host\_cdc\_rx\_available**](syn__usb__host__cdc_8c.md#function-syn_usb_host_cdc_rx_available)(hcdc))`<br>_Block a protothread coroutine until Host CDC RX data is available._  |
| define  | [**PT\_USB\_HOST\_CDC\_WAIT\_TX\_READY**](syn__usb__host__cdc_8h.md#define-pt_usb_host_cdc_wait_tx_ready) (pt, hcdc) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, [**syn\_usb\_host\_cdc\_tx\_ready**](syn__usb__host__cdc_8c.md#function-syn_usb_host_cdc_tx_ready)(hcdc))`<br>_Block a protothread coroutine until Host CDC TX buffer is ready._  |
| define  | [**SYN\_USB\_HOST\_CDC\_BUF\_SIZE**](syn__usb__host__cdc_8h.md#define-syn_usb_host_cdc_buf_size)  `128U`<br> |

## Detailed Description


Connects to downstream USB CDC ACM devices (serial adapters, modems, other MCUs in Device mode). Provides read/write buffers and protothread coroutine macros. 


    
## Public Functions Documentation




### function syn\_usb\_host\_cdc\_init 

_Initialize USB Host CDC instance with defaults (115200 8N1)._ 
```C++
SYN_Status syn_usb_host_cdc_init (
    SYN_USB_HostCDC * hcdc
) 
```





**Parameters:**


* `hcdc` Pointer to USB Host CDC instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_host\_cdc\_read 

_Read bytes from USB Host CDC RX buffer._ 
```C++
SYN_Status syn_usb_host_cdc_read (
    SYN_USB_HostCDC * hcdc,
    void * buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `hcdc` Pointer to USB Host CDC instance. 
* `buf` Pointer to destination buffer. 
* `max_len` Max capacity. 
* `out_len` Pointer to receive read byte count. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_host\_cdc\_register 

_Register USB Host CDC class driver with USB Host Core._ 
```C++
SYN_Status syn_usb_host_cdc_register (
    SYN_USB_Host * host,
    SYN_USB_HostCDC * hcdc
) 
```





**Parameters:**


* `host` Pointer to USB Host instance. 
* `hcdc` Pointer to USB Host CDC instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_host\_cdc\_rx\_available 

_Check if receive data is available._ 
```C++
bool syn_usb_host_cdc_rx_available (
    const SYN_USB_HostCDC * hcdc
) 
```





**Parameters:**


* `hcdc` Pointer to USB Host CDC instance. 



**Returns:**

true if unread data is present. 





        

<hr>



### function syn\_usb\_host\_cdc\_tx\_ready 

_Check if transmit buffer is ready._ 
```C++
bool syn_usb_host_cdc_tx_ready (
    const SYN_USB_HostCDC * hcdc
) 
```





**Parameters:**


* `hcdc` Pointer to USB Host CDC instance. 



**Returns:**

true if transmit buffer is available. 





        

<hr>



### function syn\_usb\_host\_cdc\_write 

_Write bytes to USB Host CDC TX buffer._ 
```C++
SYN_Status syn_usb_host_cdc_write (
    SYN_USB_HostCDC * hcdc,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `hcdc` Pointer to USB Host CDC instance. 
* `data` Pointer to data bytes. 
* `len` Byte length. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Macro Definition Documentation





### define PT\_USB\_HOST\_CDC\_WAIT\_RX 

_Block a protothread coroutine until Host CDC RX data is available._ 
```C++
#define PT_USB_HOST_CDC_WAIT_RX (
    pt,
    hcdc
) `PT_WAIT_UNTIL (pt, syn_usb_host_cdc_rx_available (hcdc))`
```





**Parameters:**


* `pt` Protothread context. 
* `hcdc` Pointer to USB Host CDC instance. 




        

<hr>



### define PT\_USB\_HOST\_CDC\_WAIT\_TX\_READY 

_Block a protothread coroutine until Host CDC TX buffer is ready._ 
```C++
#define PT_USB_HOST_CDC_WAIT_TX_READY (
    pt,
    hcdc
) `PT_WAIT_UNTIL (pt, syn_usb_host_cdc_tx_ready (hcdc))`
```





**Parameters:**


* `pt` Protothread context. 
* `hcdc` Pointer to USB Host CDC instance. 




        

<hr>



### define SYN\_USB\_HOST\_CDC\_BUF\_SIZE 

```C++
#define SYN_USB_HOST_CDC_BUF_SIZE `128U`
```



RX/TX buffer size 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_host_cdc.h`

