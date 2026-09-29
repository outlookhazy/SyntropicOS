

# File syn\_iolink.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_iolink.h**](syn__iolink_8h.md)

[Go to the source code of this file](syn__iolink_8h_source.md)

_IO-Link (IEC 61131-9) Single-Drop Digital Point-to-Point Master & Device Protocol Engine._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../net/syn_transport.h"`
* `#include "../pt/syn_pt.h"`
* `#include "../sched/syn_sched.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_IOLink\_Device**](structSYN__IOLink__Device.md) <br>_IO-Link Device instance context._  |
| struct | [**SYN\_IOLink\_DeviceConfig**](structSYN__IOLink__DeviceConfig.md) <br>_Device configuration descriptor._  |
| struct | [**SYN\_IOLink\_DirectParams**](structSYN__IOLink__DirectParams.md) <br>_Direct Parameter Page 1 structure._  |
| struct | [**SYN\_IOLink\_Master**](structSYN__IOLink__Master.md) <br>_IO-Link Master instance context._  |
| struct | [**SYN\_IOLink\_MasterConfig**](structSYN__IOLink__MasterConfig.md) <br>_Master configuration descriptor._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_IOLink\_Baud**](#enum-syn_iolink_baud)  <br>_IO-Link transmission speed baud rate._  |
| typedef [**SYN\_Status**](syn__defs_8h.md#enum-syn_status)(\* | [**SYN\_IOLink\_ISDU\_ReadCb**](#typedef-syn_iolink_isdu_readcb)  <br>_Callback for dynamic ISDU parameter read._  |
| typedef [**SYN\_Status**](syn__defs_8h.md#enum-syn_status)(\* | [**SYN\_IOLink\_ISDU\_WriteCb**](#typedef-syn_iolink_isdu_writecb)  <br>_Callback for dynamic ISDU parameter write._  |
| enum  | [**SYN\_IOLink\_MSeqType**](#enum-syn_iolink_mseqtype)  <br>_M-Sequence message type._  |
| enum  | [**SYN\_IOLink\_PortState**](#enum-syn_iolink_portstate)  <br>_Port operational state._  |




















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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_IOLINK\_MAX\_ISDU\_LEN**](syn__iolink_8h.md#define-syn_iolink_max_isdu_len)  `64U`<br> |
| define  | [**SYN\_IOLINK\_MAX\_PD\_LEN**](syn__iolink_8h.md#define-syn_iolink_max_pd_len)  `32U`<br> |
| define  | [**SYN\_IOLINK\_PAGE\_DEVICE\_ID\_1**](syn__iolink_8h.md#define-syn_iolink_page_device_id_1)  `0x08U`<br> |
| define  | [**SYN\_IOLINK\_PAGE\_DEVICE\_ID\_2**](syn__iolink_8h.md#define-syn_iolink_page_device_id_2)  `0x09U`<br> |
| define  | [**SYN\_IOLINK\_PAGE\_DEVICE\_ID\_3**](syn__iolink_8h.md#define-syn_iolink_page_device_id_3)  `0x0AU`<br> |
| define  | [**SYN\_IOLINK\_PAGE\_MASTER\_CYCLE\_TIME**](syn__iolink_8h.md#define-syn_iolink_page_master_cycle_time)  `0x00U`<br> |
| define  | [**SYN\_IOLINK\_PAGE\_MIN\_CYCLE\_TIME**](syn__iolink_8h.md#define-syn_iolink_page_min_cycle_time)  `0x01U`<br> |
| define  | [**SYN\_IOLINK\_PAGE\_MSEQ\_CAPABILITY**](syn__iolink_8h.md#define-syn_iolink_page_mseq_capability)  `0x02U`<br> |
| define  | [**SYN\_IOLINK\_PAGE\_PD\_IN\_LEN**](syn__iolink_8h.md#define-syn_iolink_page_pd_in_len)  `0x04U`<br> |
| define  | [**SYN\_IOLINK\_PAGE\_PD\_OUT\_LEN**](syn__iolink_8h.md#define-syn_iolink_page_pd_out_len)  `0x05U`<br> |
| define  | [**SYN\_IOLINK\_PAGE\_REVISION\_ID**](syn__iolink_8h.md#define-syn_iolink_page_revision_id)  `0x03U`<br> |
| define  | [**SYN\_IOLINK\_PAGE\_VENDOR\_ID\_1**](syn__iolink_8h.md#define-syn_iolink_page_vendor_id_1)  `0x06U`<br> |
| define  | [**SYN\_IOLINK\_PAGE\_VENDOR\_ID\_2**](syn__iolink_8h.md#define-syn_iolink_page_vendor_id_2)  `0x07U`<br> |
| define  | [**SYN\_IOLINK\_REV\_1\_0**](syn__iolink_8h.md#define-syn_iolink_rev_1_0)  `0x10U`<br> |
| define  | [**SYN\_IOLINK\_REV\_1\_1**](syn__iolink_8h.md#define-syn_iolink_rev_1_1)  `0x11U`<br> |

## Detailed Description


Implements a zero-heap, deterministic IO-Link protocol engine:
* Physical transmission rate configuration (COM1: 4.8k, COM2: 38.4k, COM3: 230.4k baud).
* M-Sequence frame encoding & parsing (Type 0, Type 1\_1, Type 1\_2, Type 2\_1, Type 2\_2, Type 2\_V) with 6-bit CRC.
* Direct Parameter Page 1 handling (Vendor ID, Device ID, Min Cycle Time, Revision ID, IO-Link profile).
* Acyclic ISDU (Indexed Service Data Unit) parameter read/write transactions.
* Non-blocking master polling step and cooperative protothread runner. 




    
## Public Types Documentation




### enum SYN\_IOLink\_Baud 

_IO-Link transmission speed baud rate._ 
```C++
enum SYN_IOLink_Baud {
    SYN_IOLINK_BAUD_COM1 = 0,
    SYN_IOLINK_BAUD_COM2 = 1,
    SYN_IOLINK_BAUD_COM3 = 2
};
```




<hr>



### typedef SYN\_IOLink\_ISDU\_ReadCb 

_Callback for dynamic ISDU parameter read._ 
```C++
typedef SYN_Status(* SYN_IOLink_ISDU_ReadCb) (uint16_t index, uint8_t subindex, uint8_t *out_data, size_t max_len, size_t *out_len, void *user_data);
```




<hr>



### typedef SYN\_IOLink\_ISDU\_WriteCb 

_Callback for dynamic ISDU parameter write._ 
```C++
typedef SYN_Status(* SYN_IOLink_ISDU_WriteCb) (uint16_t index, uint8_t subindex, const uint8_t *data, size_t len, void *user_data);
```




<hr>



### enum SYN\_IOLink\_MSeqType 

_M-Sequence message type._ 
```C++
enum SYN_IOLink_MSeqType {
    SYN_IOLINK_TYPE_0 = 0,
    SYN_IOLINK_TYPE_1_1 = 1,
    SYN_IOLINK_TYPE_1_2 = 2,
    SYN_IOLINK_TYPE_2_1 = 3,
    SYN_IOLINK_TYPE_2_2 = 4,
    SYN_IOLINK_TYPE_2_V = 5
};
```




<hr>



### enum SYN\_IOLink\_PortState 

_Port operational state._ 
```C++
enum SYN_IOLink_PortState {
    SYN_IOLINK_PORT_INACTIVE = 0,
    SYN_IOLINK_PORT_SIO,
    SYN_IOLINK_PORT_STARTUP,
    SYN_IOLINK_PORT_OPERATE,
    SYN_IOLINK_PORT_FAULT
};
```




<hr>
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
## Macro Definition Documentation





### define SYN\_IOLINK\_MAX\_ISDU\_LEN 

```C++
#define SYN_IOLINK_MAX_ISDU_LEN `64U`
```



Max ISDU payload length 


        

<hr>



### define SYN\_IOLINK\_MAX\_PD\_LEN 

```C++
#define SYN_IOLINK_MAX_PD_LEN `32U`
```



Max Process Data length in bytes 


        

<hr>



### define SYN\_IOLINK\_PAGE\_DEVICE\_ID\_1 

```C++
#define SYN_IOLINK_PAGE_DEVICE_ID_1 `0x08U`
```



Device ID MSB 


        

<hr>



### define SYN\_IOLINK\_PAGE\_DEVICE\_ID\_2 

```C++
#define SYN_IOLINK_PAGE_DEVICE_ID_2 `0x09U`
```



Device ID Mid 


        

<hr>



### define SYN\_IOLINK\_PAGE\_DEVICE\_ID\_3 

```C++
#define SYN_IOLINK_PAGE_DEVICE_ID_3 `0x0AU`
```



Device ID LSB 


        

<hr>



### define SYN\_IOLINK\_PAGE\_MASTER\_CYCLE\_TIME 

```C++
#define SYN_IOLINK_PAGE_MASTER_CYCLE_TIME `0x00U`
```



Master cycle time address 


        

<hr>



### define SYN\_IOLINK\_PAGE\_MIN\_CYCLE\_TIME 

```C++
#define SYN_IOLINK_PAGE_MIN_CYCLE_TIME `0x01U`
```



Min cycle time address 


        

<hr>



### define SYN\_IOLINK\_PAGE\_MSEQ\_CAPABILITY 

```C++
#define SYN_IOLINK_PAGE_MSEQ_CAPABILITY `0x02U`
```



M-Sequence capability 


        

<hr>



### define SYN\_IOLINK\_PAGE\_PD\_IN\_LEN 

```C++
#define SYN_IOLINK_PAGE_PD_IN_LEN `0x04U`
```



Process data in length 


        

<hr>



### define SYN\_IOLINK\_PAGE\_PD\_OUT\_LEN 

```C++
#define SYN_IOLINK_PAGE_PD_OUT_LEN `0x05U`
```



Process data out length 


        

<hr>



### define SYN\_IOLINK\_PAGE\_REVISION\_ID 

```C++
#define SYN_IOLINK_PAGE_REVISION_ID `0x03U`
```



Protocol revision ID (0x11 = v1.1) 


        

<hr>



### define SYN\_IOLINK\_PAGE\_VENDOR\_ID\_1 

```C++
#define SYN_IOLINK_PAGE_VENDOR_ID_1 `0x06U`
```



Vendor ID MSB 


        

<hr>



### define SYN\_IOLINK\_PAGE\_VENDOR\_ID\_2 

```C++
#define SYN_IOLINK_PAGE_VENDOR_ID_2 `0x07U`
```



Vendor ID LSB 


        

<hr>



### define SYN\_IOLINK\_REV\_1\_0 

```C++
#define SYN_IOLINK_REV_1_0 `0x10U`
```



IO-Link v1.0 


        

<hr>



### define SYN\_IOLINK\_REV\_1\_1 

```C++
#define SYN_IOLINK_REV_1_1 `0x11U`
```



IO-Link v1.1 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_iolink.h`

