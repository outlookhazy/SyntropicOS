

# File syn\_port\_usb\_host.h



[**FileList**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**syn\_port\_usb\_host.h**](syn__port__usb__host_8h.md)

[Go to the source code of this file](syn__port__usb__host_8h_source.md)

_USB Host HAL hardware port interface contract._ [More...](#detailed-description)

* `#include "syn_port_usb.h"`
* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/drivers/syn_usb.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_host\_bus\_reset**](#function-syn_port_usb_host_bus_reset) (void) <br>_Issue USB bus reset (10ms SE0 signaling)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_port\_usb\_host\_device\_attached**](#function-syn_port_usb_host_device_attached) (void) <br>_Check if a downstream USB device is physically attached._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_host\_init**](#function-syn_port_usb_host_init) (void) <br>_Initialize USB Host controller hardware._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_host\_pipe\_close**](#function-syn_port_usb_host_pipe_close) (uint8\_t pipe) <br>_Close a host pipe._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_host\_pipe\_open**](#function-syn_port_usb_host_pipe_open) (uint8\_t pipe, uint8\_t dev\_addr, uint8\_t ep\_addr, uint8\_t ep\_type, uint16\_t max\_pkt) <br>_Open a host pipe to a device endpoint._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_host\_submit\_data**](#function-syn_port_usb_host_submit_data) (uint8\_t pipe, uint8\_t \* buf, uint16\_t len, [**bool**](syn__defs_8h.md#enum-bool) is\_in) <br>_Submit a data stage transfer on a pipe (non-blocking)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_host\_submit\_setup**](#function-syn_port_usb_host_submit_setup) (uint8\_t pipe, const [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* pkt) <br>_Submit a control setup packet on a pipe (non-blocking)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_host\_vbus**](#function-syn_port_usb_host_vbus) ([**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable or disable VBUS 5V power supply to downstream port._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_port\_usb\_host\_xfer\_done**](#function-syn_port_usb_host_xfer_done) (uint8\_t pipe) <br>_Check if a submitted transfer has completed._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_usb\_host\_xfer\_result**](#function-syn_port_usb_host_xfer_result) (uint8\_t pipe, uint16\_t \* actual\_len) <br>_Get the result of a completed transfer._  |




























## Detailed Description


Provides the hardware abstraction for USB Host controllers. Transfer model is non-blocking: submit a transfer, poll for completion, then read the result. Fits cooperative protothreads. 


    
## Public Functions Documentation




### function syn\_port\_usb\_host\_bus\_reset 

_Issue USB bus reset (10ms SE0 signaling)._ 
```C++
SYN_Status syn_port_usb_host_bus_reset (
    void
) 
```





**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_usb\_host\_device\_attached 

_Check if a downstream USB device is physically attached._ 
```C++
bool syn_port_usb_host_device_attached (
    void
) 
```





**Returns:**

true if device detected on the port. 





        

<hr>



### function syn\_port\_usb\_host\_init 

_Initialize USB Host controller hardware._ 
```C++
SYN_Status syn_port_usb_host_init (
    void
) 
```





**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_usb\_host\_pipe\_close 

_Close a host pipe._ 
```C++
SYN_Status syn_port_usb_host_pipe_close (
    uint8_t pipe
) 
```





**Parameters:**


* `pipe` Pipe index. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_usb\_host\_pipe\_open 

_Open a host pipe to a device endpoint._ 
```C++
SYN_Status syn_port_usb_host_pipe_open (
    uint8_t pipe,
    uint8_t dev_addr,
    uint8_t ep_addr,
    uint8_t ep_type,
    uint16_t max_pkt
) 
```





**Parameters:**


* `pipe` Pipe index (0..SYN\_USB\_HOST\_MAX\_PIPES-1). 
* `dev_addr` Target device bus address (0 during initial enum). 
* `ep_addr` Target endpoint address (bit 7 = direction). 
* `ep_type` Endpoint type (SYN\_USB\_EP\_TYPE\_\*). 
* `max_pkt` Maximum packet size. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_usb\_host\_submit\_data 

_Submit a data stage transfer on a pipe (non-blocking)._ 
```C++
SYN_Status syn_port_usb_host_submit_data (
    uint8_t pipe,
    uint8_t * buf,
    uint16_t len,
    bool is_in
) 
```





**Parameters:**


* `pipe` Pipe index. 
* `buf` Data buffer (IN: filled on completion, OUT: sent). 
* `len` Byte length. 
* `is_in` true for IN (device-to-host), false for OUT. 



**Returns:**

SYN\_OK if submitted. 





        

<hr>



### function syn\_port\_usb\_host\_submit\_setup 

_Submit a control setup packet on a pipe (non-blocking)._ 
```C++
SYN_Status syn_port_usb_host_submit_setup (
    uint8_t pipe,
    const SYN_USB_SetupPacket * pkt
) 
```





**Parameters:**


* `pipe` Pipe index (must be open as Control type). 
* `pkt` Pointer to 8-byte USB Setup Packet. 



**Returns:**

SYN\_OK if submitted. 





        

<hr>



### function syn\_port\_usb\_host\_vbus 

_Enable or disable VBUS 5V power supply to downstream port._ 
```C++
SYN_Status syn_port_usb_host_vbus (
    bool enable
) 
```





**Parameters:**


* `enable` true to power on, false to power off. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_usb\_host\_xfer\_done 

_Check if a submitted transfer has completed._ 
```C++
bool syn_port_usb_host_xfer_done (
    uint8_t pipe
) 
```





**Parameters:**


* `pipe` Pipe index. 



**Returns:**

true if the last submitted transfer is complete. 





        

<hr>



### function syn\_port\_usb\_host\_xfer\_result 

_Get the result of a completed transfer._ 
```C++
SYN_Status syn_port_usb_host_xfer_result (
    uint8_t pipe,
    uint16_t * actual_len
) 
```





**Parameters:**


* `pipe` Pipe index. 
* `actual_len` [out] Actual bytes transferred. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on transfer error. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/port/syn_port_usb_host.h`

