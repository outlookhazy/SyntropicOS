

# File syn\_iolink.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_iolink.c**](syn__iolink_8c.md)

[Go to the source code of this file](syn__iolink_8c_source.md)

_IO-Link (IEC 61131-9) Single-Drop Digital Point-to-Point Protocol Engine Implementation._ 

* `#include "../port/syn_port_system.h"`
* `#include "syn_iolink.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_iolink\_calc\_checksum**](#function-syn_iolink_calc_checksum) (const uint8\_t \* data, size\_t len) <br>_Calculate standard IO-Link 6-bit CRC with parity._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_iolink\_device\_get\_pd\_out**](#function-syn_iolink_device_get_pd_out) (const [**SYN\_IOLink\_Device**](structSYN__IOLink__Device.md) \* dev, uint8\_t \* out\_buf, size\_t max\_len, size\_t \* out\_len) <br>_Get the device's latest actuator output process data written by master._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_iolink\_device\_init**](#function-syn_iolink_device_init) ([**SYN\_IOLink\_Device**](structSYN__IOLink__Device.md) \* dev, const [**SYN\_IOLink\_DeviceConfig**](structSYN__IOLink__DeviceConfig.md) \* cfg) <br>_Initialize IO-Link Device._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_iolink\_device\_process\_frame**](#function-syn_iolink_device_process_frame) ([**SYN\_IOLink\_Device**](structSYN__IOLink__Device.md) \* dev, const uint8\_t \* rx, size\_t rx\_len, uint8\_t \* tx, size\_t max\_tx, size\_t \* out\_len) <br>_Process an inbound M-Sequence master frame and generate the device response frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_iolink\_device\_set\_pd\_in**](#function-syn_iolink_device_set_pd_in) ([**SYN\_IOLink\_Device**](structSYN__IOLink__Device.md) \* dev, const uint8\_t \* data, size\_t len) <br>_Set the device's sensor input process data._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_iolink\_master\_exchange\_pd**](#function-syn_iolink_master_exchange_pd) ([**SYN\_IOLink\_Master**](structSYN__IOLink__Master.md) \* master, const uint8\_t \* pd\_out, size\_t out\_len, uint8\_t \* pd\_in, size\_t \* in\_len) <br>_Perform a cyclic Process Data exchange with connected device._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_iolink\_master\_init**](#function-syn_iolink_master_init) ([**SYN\_IOLink\_Master**](structSYN__IOLink__Master.md) \* master, const [**SYN\_IOLink\_MasterConfig**](structSYN__IOLink__MasterConfig.md) \* cfg) <br>_Initialize IO-Link Master._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_iolink\_master\_pt**](#function-syn_iolink_master_pt) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Protothread continuation function for_ `syn_sched` _._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_iolink\_master\_read\_isdu**](#function-syn_iolink_master_read_isdu) ([**SYN\_IOLink\_Master**](structSYN__IOLink__Master.md) \* master, uint16\_t index, uint8\_t subindex, uint8\_t \* data, size\_t max\_len, size\_t \* out\_len) <br>_Read an acyclic ISDU parameter from device._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_iolink\_master\_start**](#function-syn_iolink_master_start) ([**SYN\_IOLink\_Master**](structSYN__IOLink__Master.md) \* master) <br>_Trigger master startup handshake sequence on connected port._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_iolink\_master\_step**](#function-syn_iolink_master_step) ([**SYN\_IOLink\_Master**](structSYN__IOLink__Master.md) \* master, uint32\_t now\_ms) <br>_Step the IO-Link master state machine and cyclic communication loop._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_iolink\_master\_write\_isdu**](#function-syn_iolink_master_write_isdu) ([**SYN\_IOLink\_Master**](structSYN__IOLink__Master.md) \* master, uint16\_t index, uint8\_t subindex, const uint8\_t \* data, size\_t len) <br>_Write an acyclic ISDU parameter to device._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**master\_read\_direct\_param**](#function-master_read_direct_param) ([**SYN\_IOLink\_Master**](structSYN__IOLink__Master.md) \* master, uint8\_t addr, uint8\_t \* out\_val) <br>_Read single byte from Direct Parameter Page 1 on attached IO-Link device._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_IOLINK\_CRC6\_POLY**](syn__iolink_8c.md#define-syn_iolink_crc6_poly)  `0x3DU`<br>_IEC 61131-9 CRC-6 polynomial (x^6 + x^5 + x^4 + x^3 + x^2 + 1)_  |
| define  | [**SYN\_IOLINK\_CRC6\_SEED**](syn__iolink_8c.md#define-syn_iolink_crc6_seed)  `0x1FU`<br>_IEC 61131-9 CRC-6 initial seed value._  |
| define  | [**SYN\_IOLINK\_DEFAULT\_CYCLE\_MS**](syn__iolink_8c.md#define-syn_iolink_default_cycle_ms)  `5U`<br>_Default IO-Link master cycle time in milliseconds._  |

## Public Functions Documentation




### function syn\_iolink\_calc\_checksum 

_Calculate standard IO-Link 6-bit CRC with parity._ 
```C++
uint8_t syn_iolink_calc_checksum (
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `data` Data buffer to calculate checksum over. 
* `len` Data length in bytes. 



**Returns:**

8-bit Checksum byte (6-bit CRC in bits 0..5, parity in bits 6..7). 





        

<hr>



### function syn\_iolink\_device\_get\_pd\_out 

_Get the device's latest actuator output process data written by master._ 
```C++
SYN_Status syn_iolink_device_get_pd_out (
    const SYN_IOLink_Device * dev,
    uint8_t * out_buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `dev` Device instance. 
* `out_buf` Buffer receiving process data. 
* `max_len` Capacity of output buffer. 
* `out_len` Pointer receiving length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_iolink\_device\_init 

_Initialize IO-Link Device._ 
```C++
SYN_Status syn_iolink_device_init (
    SYN_IOLink_Device * dev,
    const SYN_IOLink_DeviceConfig * cfg
) 
```





**Parameters:**


* `dev` Device instance to initialize. 
* `cfg` Configuration parameters and parameter page values. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_iolink\_device\_process\_frame 

_Process an inbound M-Sequence master frame and generate the device response frame._ 
```C++
SYN_Status syn_iolink_device_process_frame (
    SYN_IOLink_Device * dev,
    const uint8_t * rx,
    size_t rx_len,
    uint8_t * tx,
    size_t max_tx,
    size_t * out_len
) 
```





**Parameters:**


* `dev` Device instance. 
* `rx` Master frame payload. 
* `rx_len` Master frame length. 
* `tx` Buffer receiving device response frame. 
* `max_tx` Capacity of response buffer. 
* `out_len` Pointer receiving response length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_iolink\_device\_set\_pd\_in 

_Set the device's sensor input process data._ 
```C++
SYN_Status syn_iolink_device_set_pd_in (
    SYN_IOLink_Device * dev,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `dev` Device instance. 
* `data` Input process data payload. 
* `len` Length of process data. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_iolink\_master\_exchange\_pd 

_Perform a cyclic Process Data exchange with connected device._ 
```C++
SYN_Status syn_iolink_master_exchange_pd (
    SYN_IOLink_Master * master,
    const uint8_t * pd_out,
    size_t out_len,
    uint8_t * pd_in,
    size_t * in_len
) 
```





**Parameters:**


* `master` Master instance. 
* `pd_out` Output process data to send to device (actuator command). 
* `out_len` Length of output process data. 
* `pd_in` Buffer receiving input process data from device (sensor measurement). 
* `in_len` Pointer receiving length of input process data. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_iolink\_master\_init 

_Initialize IO-Link Master._ 
```C++
SYN_Status syn_iolink_master_init (
    SYN_IOLink_Master * master,
    const SYN_IOLink_MasterConfig * cfg
) 
```





**Parameters:**


* `master` Master instance to initialize. 
* `cfg` Configuration parameters. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid argument. 





        

<hr>



### function syn\_iolink\_master\_pt 

_Protothread continuation function for_ `syn_sched` _._
```C++
SYN_PT_Status syn_iolink_master_pt (
    SYN_PT * pt,
    SYN_Task * task
) 
```





**Parameters:**


* `pt` Protothread continuation structure. 
* `task` Task handle containing [**SYN\_IOLink\_Master**](structSYN__IOLink__Master.md) in user\_data. 



**Returns:**

SYN\_PT\_Status continuation status. 





        

<hr>



### function syn\_iolink\_master\_read\_isdu 

_Read an acyclic ISDU parameter from device._ 
```C++
SYN_Status syn_iolink_master_read_isdu (
    SYN_IOLink_Master * master,
    uint16_t index,
    uint8_t subindex,
    uint8_t * data,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `master` Master instance. 
* `index` 16-bit ISDU Index. 
* `subindex` 8-bit ISDU Subindex. 
* `data` Buffer receiving parameter data. 
* `max_len` Capacity of parameter buffer. 
* `out_len` Pointer receiving parameter data length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_iolink\_master\_start 

_Trigger master startup handshake sequence on connected port._ 
```C++
SYN_Status syn_iolink_master_start (
    SYN_IOLink_Master * master
) 
```





**Parameters:**


* `master` Master instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_iolink\_master\_step 

_Step the IO-Link master state machine and cyclic communication loop._ 
```C++
SYN_Status syn_iolink_master_step (
    SYN_IOLink_Master * master,
    uint32_t now_ms
) 
```





**Parameters:**


* `master` Master instance. 
* `now_ms` Current timestamp in milliseconds. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_iolink\_master\_write\_isdu 

_Write an acyclic ISDU parameter to device._ 
```C++
SYN_Status syn_iolink_master_write_isdu (
    SYN_IOLink_Master * master,
    uint16_t index,
    uint8_t subindex,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `master` Master instance. 
* `index` 16-bit ISDU Index. 
* `subindex` 8-bit ISDU Subindex. 
* `data` Parameter data to write. 
* `len` Parameter data length. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Public Static Functions Documentation




### function master\_read\_direct\_param 

_Read single byte from Direct Parameter Page 1 on attached IO-Link device._ 
```C++
static SYN_Status master_read_direct_param (
    SYN_IOLink_Master * master,
    uint8_t addr,
    uint8_t * out_val
) 
```





**Parameters:**


* `master` Master instance pointer. 
* `addr` Parameter address (0x00..0x3F). 
* `out_val` Pointer to receive parameter byte. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on transport or checksum failure. 





        

<hr>
## Macro Definition Documentation





### define SYN\_IOLINK\_CRC6\_POLY 

_IEC 61131-9 CRC-6 polynomial (x^6 + x^5 + x^4 + x^3 + x^2 + 1)_ 
```C++
#define SYN_IOLINK_CRC6_POLY `0x3DU`
```




<hr>



### define SYN\_IOLINK\_CRC6\_SEED 

_IEC 61131-9 CRC-6 initial seed value._ 
```C++
#define SYN_IOLINK_CRC6_SEED `0x1FU`
```




<hr>



### define SYN\_IOLINK\_DEFAULT\_CYCLE\_MS 

_Default IO-Link master cycle time in milliseconds._ 
```C++
#define SYN_IOLINK_DEFAULT_CYCLE_MS `5U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_iolink.c`

