

# File syn\_ethercat.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ethercat.h**](syn__ethercat_8h.md)

[Go to the source code of this file](syn__ethercat_8h_source.md)

_EtherCAT (IEEE 802.3 EtherType 0x88A4) Bare-Metal Protocol Stack._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "syn_canopen.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_EcatCoEHeader**](structSYN__EcatCoEHeader.md) <br>_CoE (CANoverEtherCAT) Mailbox Header._  |
| struct | [**SYN\_EcatDatagram**](structSYN__EcatDatagram.md) <br>_EtherCAT Datagram Header (10 bytes header + 2 bytes WKC = 12 bytes overhead)_  |
| struct | [**SYN\_EcatHeader**](structSYN__EcatHeader.md) <br>_Raw EtherCAT Header (2 bytes)_  |
| struct | [**SYN\_EcatNode**](structSYN__EcatNode.md) <br>_EtherCAT Node Descriptor._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_EcatCmd**](#enum-syn_ecatcmd)  <br> |
| enum  | [**SYN\_EcatCoEType**](#enum-syn_ecatcoetype)  <br> |
| enum  | [**SYN\_EcatState**](#enum-syn_ecatstate)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_ecat\_build\_datagram\_frame**](#function-syn_ecat_build_datagram_frame) (uint8\_t \* buf, size\_t buf\_len, const [**SYN\_EcatDatagram**](structSYN__EcatDatagram.md) \* datagram, const uint8\_t \* data, uint16\_t data\_len) <br>_Build an EtherCAT Frame containing one or more datagrams._  |
|  size\_t | [**syn\_ecat\_coe\_encode\_sdo\_download**](#function-syn_ecat_coe_encode_sdo_download) (uint8\_t \* buf, size\_t buf\_len, uint16\_t index, uint8\_t subindex, const void \* data, size\_t data\_len) <br>_Encode a CoE SDO Download (Write) request into a mailbox buffer._  |
|  size\_t | [**syn\_ecat\_coe\_encode\_sdo\_upload**](#function-syn_ecat_coe_encode_sdo_upload) (uint8\_t \* buf, size\_t buf\_len, uint16\_t index, uint8\_t subindex) <br>_Encode a CoE SDO Upload (Read) request into a mailbox buffer._  |
|  void | [**syn\_ecat\_init**](#function-syn_ecat_init) ([**SYN\_EcatNode**](structSYN__EcatNode.md) \* node, uint16\_t station\_addr, [**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) \* od) <br>_Initialize an EtherCAT Node instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ecat\_parse\_frame**](#function-syn_ecat_parse_frame) ([**SYN\_EcatNode**](structSYN__EcatNode.md) \* node, const uint8\_t \* frame, size\_t len, uint16\_t \* wkc) <br>_Parse and process a received raw EtherCAT frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ecat\_set\_state**](#function-syn_ecat_set_state) ([**SYN\_EcatNode**](structSYN__EcatNode.md) \* node, [**SYN\_EcatState**](syn__ethercat_8h.md#enum-syn_ecatstate) new\_state) <br>_Request an EtherCAT State Machine (ESM) state transition._  |
|  void | [**syn\_ecat\_update**](#function-syn_ecat_update) ([**SYN\_EcatNode**](structSYN__EcatNode.md) \* node) <br>_Run ESM state machine step._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_ETHERCAT\_ETHERTYPE**](syn__ethercat_8h.md#define-syn_ethercat_ethertype)  `0x88A4`<br>_EtherCAT IEEE 802.3 Ethernet Frame Type._  |
| define  | [**SYN\_ETHERCAT\_TYPE\_DATAGRAM**](syn__ethercat_8h.md#define-syn_ethercat_type_datagram)  `1`<br>_EtherCAT Header Type (1 = EtherCAT Datagrams)_  |

## Detailed Description


Implements EtherCAT Frame packing, Datagram commands (APRD, APWR, APRW, FPRD, FPWR, FPRW, BRD, BWR, LRD, LWR, LRW), Working Counter (WKC) accounting, EtherCAT State Machine (ESM: INIT, PREOP, BOOT, SAFEOP, OP), and CoE (CAN application protocol over EtherCAT) Mailbox SDO & PDO handling.


Designed for zero heap allocation and direct L2 raw Ethernet frame processing. 


    
## Public Types Documentation




### enum SYN\_EcatCmd 

```C++
enum SYN_EcatCmd {
    SYN_ECAT_CMD_NOP = 0,
    SYN_ECAT_CMD_APRD = 1,
    SYN_ECAT_CMD_APWR = 2,
    SYN_ECAT_CMD_APRW = 3,
    SYN_ECAT_CMD_FPRD = 4,
    SYN_ECAT_CMD_FPWR = 5,
    SYN_ECAT_CMD_FPRW = 6,
    SYN_ECAT_CMD_BRD = 7,
    SYN_ECAT_CMD_BWR = 8,
    SYN_ECAT_CMD_BRW = 9,
    SYN_ECAT_CMD_LRD = 10,
    SYN_ECAT_CMD_LWR = 11,
    SYN_ECAT_CMD_LRW = 12,
    SYN_ECAT_CMD_ARMW = 13,
    SYN_ECAT_CMD_FRMW = 14
};
```



EtherCAT Datagram Commands Enum. 


        

<hr>



### enum SYN\_EcatCoEType 

```C++
enum SYN_EcatCoEType {
    SYN_ECAT_COE_TYPE_EMERGENCY = 1,
    SYN_ECAT_COE_TYPE_SDO_REQ = 2,
    SYN_ECAT_COE_TYPE_SDO_RESP = 3,
    SYN_ECAT_COE_TYPE_RXPDO = 4,
    SYN_ECAT_COE_TYPE_TXPDO = 5,
    SYN_ECAT_COE_TYPE_SDO_INFO = 6
};
```



CAN Application Protocol over EtherCAT (CoE) Service Types. 


        

<hr>



### enum SYN\_EcatState 

```C++
enum SYN_EcatState {
    SYN_ECAT_STATE_NONE = 0x00,
    SYN_ECAT_STATE_INIT = 0x01,
    SYN_ECAT_STATE_PREOP = 0x02,
    SYN_ECAT_STATE_BOOT = 0x03,
    SYN_ECAT_STATE_SAFEOP = 0x04,
    SYN_ECAT_STATE_OP = 0x08
};
```



EtherCAT State Machine (ESM) States. 


        

<hr>
## Public Functions Documentation




### function syn\_ecat\_build\_datagram\_frame 

_Build an EtherCAT Frame containing one or more datagrams._ 
```C++
size_t syn_ecat_build_datagram_frame (
    uint8_t * buf,
    size_t buf_len,
    const SYN_EcatDatagram * datagram,
    const uint8_t * data,
    uint16_t data_len
) 
```





**Parameters:**


* `buf` Output frame buffer (must include Ethernet header space if needed). 
* `buf_len` Capacity of output buffer. 
* `datagram` Datagram header descriptor. 
* `data` Payload bytes to write into datagram. 
* `data_len` Payload length. 



**Returns:**

Total EtherCAT frame length in bytes, or 0 on error. 





        

<hr>



### function syn\_ecat\_coe\_encode\_sdo\_download 

_Encode a CoE SDO Download (Write) request into a mailbox buffer._ 
```C++
size_t syn_ecat_coe_encode_sdo_download (
    uint8_t * buf,
    size_t buf_len,
    uint16_t index,
    uint8_t subindex,
    const void * data,
    size_t data_len
) 
```





**Parameters:**


* `buf` Output buffer. 
* `buf_len` Capacity of output buffer. 
* `index` Object Dictionary Index (0x0000 - 0xFFFF). 
* `subindex` Subindex (0x00 - 0xFF). 
* `data` Data bytes to write. 
* `data_len` Data length (1-4 bytes for expedited SDO). 



**Returns:**

Encoded CoE Mailbox packet length in bytes, or 0 on error. 





        

<hr>



### function syn\_ecat\_coe\_encode\_sdo\_upload 

_Encode a CoE SDO Upload (Read) request into a mailbox buffer._ 
```C++
size_t syn_ecat_coe_encode_sdo_upload (
    uint8_t * buf,
    size_t buf_len,
    uint16_t index,
    uint8_t subindex
) 
```





**Parameters:**


* `buf` Output buffer. 
* `buf_len` Capacity of output buffer. 
* `index` Object Dictionary Index (0x0000 - 0xFFFF). 
* `subindex` Subindex (0x00 - 0xFF). 



**Returns:**

Encoded CoE Mailbox packet length in bytes, or 0 on error. 





        

<hr>



### function syn\_ecat\_init 

_Initialize an EtherCAT Node instance._ 
```C++
void syn_ecat_init (
    SYN_EcatNode * node,
    uint16_t station_addr,
    SYN_CANOpenNode * od
) 
```





**Parameters:**


* `node` EtherCAT node instance. 
* `station_addr` Configured station address. 
* `od` Optional CANopen Object Dictionary binding for CoE. 




        

<hr>



### function syn\_ecat\_parse\_frame 

_Parse and process a received raw EtherCAT frame._ 
```C++
SYN_Status syn_ecat_parse_frame (
    SYN_EcatNode * node,
    const uint8_t * frame,
    size_t len,
    uint16_t * wkc
) 
```





**Parameters:**


* `node` EtherCAT node instance. 
* `frame` Raw received frame bytes (after Ethernet header). 
* `len` Frame length in bytes. 
* `wkc` [out] Extracted Working Counter sum. 



**Returns:**

SYN\_OK on successful parsing and WKC validation. 





        

<hr>



### function syn\_ecat\_set\_state 

_Request an EtherCAT State Machine (ESM) state transition._ 
```C++
SYN_Status syn_ecat_set_state (
    SYN_EcatNode * node,
    SYN_EcatState new_state
) 
```





**Parameters:**


* `node` EtherCAT node instance. 
* `new_state` Requested state (INIT, PREOP, BOOT, SAFEOP, OP). 



**Returns:**

SYN\_OK if state transition request is valid. 





        

<hr>



### function syn\_ecat\_update 

_Run ESM state machine step._ 
```C++
void syn_ecat_update (
    SYN_EcatNode * node
) 
```





**Parameters:**


* `node` EtherCAT node instance. 




        

<hr>
## Macro Definition Documentation





### define SYN\_ETHERCAT\_ETHERTYPE 

_EtherCAT IEEE 802.3 Ethernet Frame Type._ 
```C++
#define SYN_ETHERCAT_ETHERTYPE `0x88A4`
```




<hr>



### define SYN\_ETHERCAT\_TYPE\_DATAGRAM 

_EtherCAT Header Type (1 = EtherCAT Datagrams)_ 
```C++
#define SYN_ETHERCAT_TYPE_DATAGRAM `1`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ethercat.h`

