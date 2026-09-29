

# File syn\_usb\_host\_cdc.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_host\_cdc.c**](syn__usb__host__cdc_8c.md)

[Go to the source code of this file](syn__usb__host__cdc_8c_source.md)

_Zero-Heap USB 2.0 Host CDC ACM Class Driver Implementation._ 

* `#include "syntropic/drivers/syn_usb_host_cdc.h"`
* `#include "port/syn_port_usb.h"`
* `#include "port/syn_port_usb_host.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_host\_cdc\_init**](#function-syn_usb_host_cdc_init) ([**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) \* hcdc) <br>_Initialize USB Host CDC instance with defaults (115200 8N1)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_host\_cdc\_read**](#function-syn_usb_host_cdc_read) ([**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) \* hcdc, void \* buf, size\_t max\_len, size\_t \* out\_len) <br>_Read bytes from USB Host CDC RX buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_host\_cdc\_register**](#function-syn_usb_host_cdc_register) ([**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host, [**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) \* hcdc) <br>_Register USB Host CDC class driver with USB Host Core._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_host\_cdc\_rx\_available**](#function-syn_usb_host_cdc_rx_available) (const [**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) \* hcdc) <br>_Check if receive data is available._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_host\_cdc\_tx\_ready**](#function-syn_usb_host_cdc_tx_ready) (const [**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) \* hcdc) <br>_Check if transmit buffer is ready._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_host\_cdc\_write**](#function-syn_usb_host_cdc_write) ([**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) \* hcdc, const void \* data, size\_t len) <br>_Write bytes to USB Host CDC TX buffer._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**host\_cdc\_disconnected**](#function-host_cdc_disconnected) (void \* ctx) <br>_Host class disconnected callback._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**host\_cdc\_probe**](#function-host_cdc_probe) (void \* ctx, uint8\_t dev\_addr, const uint8\_t \* iface\_desc, uint16\_t len) <br>_Host class probe callback for CDC ACM interfaces._  |
|  void | [**host\_cdc\_process**](#function-host_cdc_process) (void \* ctx) <br>_Host class process/tick callback._  |


























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
## Public Static Functions Documentation




### function host\_cdc\_disconnected 

_Host class disconnected callback._ 
```C++
static void host_cdc_disconnected (
    void * ctx
) 
```





**Parameters:**


* `ctx` Context pointer ([**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md)). 




        

<hr>



### function host\_cdc\_probe 

_Host class probe callback for CDC ACM interfaces._ 
```C++
static SYN_Status host_cdc_probe (
    void * ctx,
    uint8_t dev_addr,
    const uint8_t * iface_desc,
    uint16_t len
) 
```





**Parameters:**


* `ctx` Context pointer ([**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md)). 
* `dev_addr` Device bus address. 
* `iface_desc` Pointer to interface descriptor payload. 
* `len` Descriptor byte length. 



**Returns:**

SYN\_OK on successful claim. 





        

<hr>



### function host\_cdc\_process 

_Host class process/tick callback._ 
```C++
static void host_cdc_process (
    void * ctx
) 
```





**Parameters:**


* `ctx` Context pointer ([**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md)). 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_host_cdc.c`

