

# File syn\_port\_usb.h



[**FileList**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**syn\_port\_usb.h**](syn__port__usb_8h.md)

[Go to the source code of this file](syn__port__usb_8h_source.md)

_USB HAL hardware port interface contract._ 

* `#include "syntropic/common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_connect**](#function-syn_port_usb_connect) (void) <br>_Signal physical USB connection/attach (e.g. enable D+ pullup)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_disconnect**](#function-syn_port_usb_disconnect) (void) <br>_Signal physical USB disconnect/detach (e.g. disable D+ pullup)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_ep\_close**](#function-syn_port_usb_ep_close) (uint8\_t ep\_addr) <br>_Close a hardware endpoint._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_ep\_open**](#function-syn_port_usb_ep_open) (uint8\_t ep\_addr, uint8\_t ep\_type, uint16\_t max\_pkt) <br>_Open and configure a hardware endpoint._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_ep\_read**](#function-syn_port_usb_ep_read) (uint8\_t ep\_addr, void \* buf, uint16\_t max\_len, uint16\_t \* out\_len) <br>_Read packet from an OUT endpoint FIFO/buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_ep\_stall**](#function-syn_port_usb_ep_stall) (uint8\_t ep\_addr) <br>_Signal STALL condition on an endpoint._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_ep\_write**](#function-syn_port_usb_ep_write) (uint8\_t ep\_addr, const void \* data, uint16\_t len) <br>_Write packet to an IN endpoint FIFO/buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_init**](#function-syn_port_usb_init) (void) <br>_Initialize low-level USB peripheral hardware._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_set\_address**](#function-syn_port_usb_set_address) (uint8\_t addr) <br>_Assign USB device address in hardware controller._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_USB\_EP\_TYPE\_BULK**](syn__port__usb_8h.md#define-syn_usb_ep_type_bulk)  `2U`<br> |
| define  | [**SYN\_USB\_EP\_TYPE\_CONTROL**](syn__port__usb_8h.md#define-syn_usb_ep_type_control)  `0U`<br> |
| define  | [**SYN\_USB\_EP\_TYPE\_INTR**](syn__port__usb_8h.md#define-syn_usb_ep_type_intr)  `3U`<br> |
| define  | [**SYN\_USB\_EP\_TYPE\_ISOC**](syn__port__usb_8h.md#define-syn_usb_ep_type_isoc)  `1U`<br> |

## Public Functions Documentation




### function syn\_port\_usb\_connect 

_Signal physical USB connection/attach (e.g. enable D+ pullup)._ 
```C++
SYN_Status syn_port_usb_connect (
    void
) 
```





**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_usb\_disconnect 

_Signal physical USB disconnect/detach (e.g. disable D+ pullup)._ 
```C++
SYN_Status syn_port_usb_disconnect (
    void
) 
```





**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_usb\_ep\_close 

_Close a hardware endpoint._ 
```C++
SYN_Status syn_port_usb_ep_close (
    uint8_t ep_addr
) 
```





**Parameters:**


* `ep_addr` Endpoint address. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_usb\_ep\_open 

_Open and configure a hardware endpoint._ 
```C++
SYN_Status syn_port_usb_ep_open (
    uint8_t ep_addr,
    uint8_t ep_type,
    uint16_t max_pkt
) 
```





**Parameters:**


* `ep_addr` Bit 7: Direction (0=OUT, 1=IN), Bits 3:0: Endpoint number. 
* `ep_type` Endpoint type (SYN\_USB\_EP\_TYPE\_\*). 
* `max_pkt` Maximum packet size in bytes. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_usb\_ep\_read 

_Read packet from an OUT endpoint FIFO/buffer._ 
```C++
SYN_Status syn_port_usb_ep_read (
    uint8_t ep_addr,
    void * buf,
    uint16_t max_len,
    uint16_t * out_len
) 
```





**Parameters:**


* `ep_addr` Endpoint address. 
* `buf` Destination buffer. 
* `max_len` Max capacity. 
* `out_len` Pointer to receive read byte count. 



**Returns:**

SYN\_OK on success, SYN\_BUSY if no packet available. 





        

<hr>



### function syn\_port\_usb\_ep\_stall 

_Signal STALL condition on an endpoint._ 
```C++
SYN_Status syn_port_usb_ep_stall (
    uint8_t ep_addr
) 
```





**Parameters:**


* `ep_addr` Endpoint address. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_usb\_ep\_write 

_Write packet to an IN endpoint FIFO/buffer._ 
```C++
SYN_Status syn_port_usb_ep_write (
    uint8_t ep_addr,
    const void * data,
    uint16_t len
) 
```





**Parameters:**


* `ep_addr` Endpoint address. 
* `data` Pointer to payload bytes. 
* `len` Byte length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_usb\_init 

_Initialize low-level USB peripheral hardware._ 
```C++
SYN_Status syn_port_usb_init (
    void
) 
```





**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_usb\_set\_address 

_Assign USB device address in hardware controller._ 
```C++
SYN_Status syn_port_usb_set_address (
    uint8_t addr
) 
```





**Parameters:**


* `addr` assigned address (1..127). 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Macro Definition Documentation





### define SYN\_USB\_EP\_TYPE\_BULK 

```C++
#define SYN_USB_EP_TYPE_BULK `2U`
```



Bulk Endpoint (2) 


        

<hr>



### define SYN\_USB\_EP\_TYPE\_CONTROL 

```C++
#define SYN_USB_EP_TYPE_CONTROL `0U`
```



USB Endpoint Types per USB Spec Control Endpoint (0) 


        

<hr>



### define SYN\_USB\_EP\_TYPE\_INTR 

```C++
#define SYN_USB_EP_TYPE_INTR `3U`
```



Interrupt Endpoint (3) 


        

<hr>



### define SYN\_USB\_EP\_TYPE\_ISOC 

```C++
#define SYN_USB_EP_TYPE_ISOC `1U`
```



Isochronous Endpoint (1) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/port/syn_port_usb.h`

