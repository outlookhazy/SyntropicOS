

# File syn\_ethercat.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ethercat.c**](syn__ethercat_8c.md)

[Go to the source code of this file](syn__ethercat_8c_source.md)

_Bare-metal EtherCAT Protocol Engine implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_ethercat.h"`
* `#include <string.h>`





































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


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**load16\_le**](#function-load16_le) (const uint8\_t \* p) <br>_Load 16-bit unsigned integer from little-endian byte order._  |
|  void | [**store16\_le**](#function-store16_le) (uint8\_t \* p, uint16\_t val) <br>_Store 16-bit unsigned integer in little-endian byte order._  |
|  void | [**store32\_le**](#function-store32_le) (uint8\_t \* p, uint32\_t val) <br>_Store 32-bit unsigned integer in little-endian byte order._  |


























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
## Public Static Functions Documentation




### function load16\_le 

_Load 16-bit unsigned integer from little-endian byte order._ 
```C++
static inline uint16_t load16_le (
    const uint8_t * p
) 
```





**Parameters:**


* `p` Pointer to input buffer. 



**Returns:**

16-bit integer value. 





        

<hr>



### function store16\_le 

_Store 16-bit unsigned integer in little-endian byte order._ 
```C++
static inline void store16_le (
    uint8_t * p,
    uint16_t val
) 
```





**Parameters:**


* `p` Pointer to output buffer. 
* `val` 16-bit integer value. 




        

<hr>



### function store32\_le 

_Store 32-bit unsigned integer in little-endian byte order._ 
```C++
static inline void store32_le (
    uint8_t * p,
    uint32_t val
) 
```





**Parameters:**


* `p` Pointer to output buffer. 
* `val` 32-bit integer value. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ethercat.c`

