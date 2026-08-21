

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
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ecat\_decode\_coe\_sdo\_response**](#function-syn_ecat_decode_coe_sdo_response) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, size\_t rx\_len, void \* out\_data, size\_t max\_len, size\_t \* out\_len) <br>_Decode SDO Upload response payload from rx\_buf._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ecat\_decode\_cyclic**](#function-syn_ecat_decode_cyclic) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, size\_t rx\_len) <br>_Parse LRW cyclic process exchange response from rx\_buf and copy inputs to input\_image._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ecat\_decode\_read\_al\_status**](#function-syn_ecat_decode_read_al_status) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, size\_t rx\_len, [**SYN\_EcatState**](syn__ethercat_8h.md#enum-syn_ecatstate) \* state, uint16\_t \* status\_code) <br>_Decode Application Layer (AL) Status and Status Code from response._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ecat\_decode\_read\_reg**](#function-syn_ecat_decode_read_reg) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, size\_t rx\_len, void \* out\_data, uint16\_t len) <br>_Decode FPRD response datagram for register read._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ecat\_decode\_read\_sii**](#function-syn_ecat_decode_read_sii) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, size\_t rx\_len, uint32\_t \* out\_val) <br>_Decode 32-bit word from SII EEPROM read response._  |
|  uint8\_t | [**syn\_ecat\_decode\_scan\_bus**](#function-syn_ecat_decode_scan_bus) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, size\_t rx\_len) <br>_Decode slave count from scan\_bus response frame in rx\_buf._  |
|  size\_t | [**syn\_ecat\_encode\_assign\_addr**](#function-syn_ecat_encode_assign_addr) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint16\_t position, uint16\_t station\_addr) <br>_Encode Auto-Increment Write (APWR) to assign station address._  |
|  size\_t | [**syn\_ecat\_encode\_coe\_sdo\_read**](#function-syn_ecat_encode_coe_sdo_read) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint16\_t station\_addr, uint16\_t index, uint8\_t subindex) <br>_Encode a complete Mailbox-wrapped CoE SDO Upload request frame into tx\_buf._  |
|  size\_t | [**syn\_ecat\_encode\_coe\_sdo\_write**](#function-syn_ecat_encode_coe_sdo_write) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint16\_t station\_addr, uint16\_t index, uint8\_t subindex, const void \* data, size\_t data\_len) <br>_Encode a complete Mailbox-wrapped CoE SDO Download request frame into tx\_buf._  |
|  size\_t | [**syn\_ecat\_encode\_cyclic**](#function-syn_ecat_encode_cyclic) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m) <br>_Encode LRW (Logical Read Write) cyclic process exchange frame into tx\_buf._  |
|  size\_t | [**syn\_ecat\_encode\_dc\_configure**](#function-syn_ecat_encode_dc_configure) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint16\_t station\_addr, uint32\_t sync0\_cycle\_ns, int32\_t sync0\_shift\_ns) <br>_Encode FPWR to configure DC SYNC0 cycle time and shift for a slave._  |
|  size\_t | [**syn\_ecat\_encode\_dc\_read\_system\_time**](#function-syn_ecat_encode_dc_read_system_time) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m) <br>_Encode Auto-Increment Read Multiple Write (ARMW) for DC System Time propagation._  |
|  size\_t | [**syn\_ecat\_encode\_read\_al\_status**](#function-syn_ecat_encode_read_al_status) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint16\_t station\_addr) <br>_Encode Configured Read (FPRD) to read slave Application Layer (AL) Status._  |
|  size\_t | [**syn\_ecat\_encode\_read\_reg**](#function-syn_ecat_encode_read_reg) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint16\_t station\_addr, uint16\_t reg\_addr, uint16\_t len) <br>_Encode FPRD to read register_ `reg_addr` _from slave_`station_addr` _._ |
|  size\_t | [**syn\_ecat\_encode\_read\_sii**](#function-syn_ecat_encode_read_sii) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint16\_t station\_addr, uint16\_t word\_offset) <br>_Encode Configured Read (FPRD) to read SII EEPROM word._  |
|  size\_t | [**syn\_ecat\_encode\_scan\_bus**](#function-syn_ecat_encode_scan_bus) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m) <br>_Encode Broadcast Read (BRD) to scan bus slave count into master tx\_buf._  |
|  size\_t | [**syn\_ecat\_encode\_set\_al\_control**](#function-syn_ecat_encode_set_al_control) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint16\_t station\_addr, [**SYN\_EcatState**](syn__ethercat_8h.md#enum-syn_ecatstate) state) <br>_Encode Configured Write (FPWR) to set slave Application Layer (AL) Control register._  |
|  size\_t | [**syn\_ecat\_encode\_write\_fmmu**](#function-syn_ecat_encode_write_fmmu) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint16\_t station\_addr, uint8\_t fmmu\_index, const [**SYN\_EcatFMMUConfig**](structSYN__EcatFMMUConfig.md) \* cfg) <br>_Encode Configured Write (FPWR) to configure an FMMU register block._  |
|  size\_t | [**syn\_ecat\_encode\_write\_reg**](#function-syn_ecat_encode_write_reg) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint16\_t station\_addr, uint16\_t reg\_addr, const void \* data, uint16\_t len) <br>_Encode FPWR to write register_ `reg_addr` _to slave_`station_addr` _._ |
|  size\_t | [**syn\_ecat\_encode\_write\_sm**](#function-syn_ecat_encode_write_sm) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint16\_t station\_addr, uint8\_t sm\_index, const [**SYN\_EcatSMConfig**](structSYN__EcatSMConfig.md) \* cfg) <br>_Encode Configured Write (FPWR) to configure a SyncManager register block._  |
|  size\_t | [**syn\_ecat\_frame\_add\_datagram**](#function-syn_ecat_frame_add_datagram) (uint8\_t \* buf, size\_t buf\_size, [**SYN\_EcatCmd**](syn__ethercat_8h.md#enum-syn_ecatcmd) cmd, uint8\_t idx, uint32\_t addr, const uint8\_t \* data, uint16\_t data\_len, [**bool**](syn__defs_8h.md#enum-bool) circ) <br>_Add a datagram payload to an in-progress EtherCAT frame._  |
|  void | [**syn\_ecat\_frame\_begin**](#function-syn_ecat_frame_begin) (uint8\_t \* buf, size\_t buf\_size) <br>_Initialize an EtherCAT frame buffer header._  |
|  size\_t | [**syn\_ecat\_frame\_finalize**](#function-syn_ecat_frame_finalize) (uint8\_t \* buf) <br>_Finalize EtherCAT frame header length field._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ecat\_frame\_parse\_next**](#function-syn_ecat_frame_parse_next) (const uint8\_t \* frame, size\_t frame\_len, size\_t \* offset, [**SYN\_EcatDatagramResult**](structSYN__EcatDatagramResult.md) \* out) <br>_Parse the next datagram from an EtherCAT frame buffer._  |
|  void | [**syn\_ecat\_init**](#function-syn_ecat_init) ([**SYN\_EcatNode**](structSYN__EcatNode.md) \* node, uint16\_t station\_addr, [**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) \* od) <br>_Initialize an EtherCAT Node instance._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_ecat\_master\_cyclic\_task**](#function-syn_ecat_master_cyclic_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m) <br>_Protothread Task: Execute single cyclic process exchange cycle._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_ecat\_master\_discover\_pdo\_mapping\_task**](#function-syn_ecat_master_discover_pdo_mapping_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint16\_t station\_addr, uint16\_t \* out\_rx\_bytes, uint16\_t \* out\_tx\_bytes) <br>_Protothread Task: Read CoE Object Dictionary 0x1C12/0x1C13 to calculate slave Rx/Tx PDO byte lengths._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ecat\_master\_init**](#function-syn_ecat_master_init) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint8\_t \* tx\_buf, uint16\_t tx\_buf\_size, uint8\_t \* rx\_buf, uint16\_t rx\_buf\_size, uint8\_t \* output\_image, uint16\_t output\_size, uint8\_t \* input\_image, uint16\_t input\_size) <br>_Initialize EtherCAT Master instance context._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ecat\_master\_pop\_tx\_frame**](#function-syn_ecat_master_pop_tx_frame) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, const uint8\_t \*\* out\_frame, size\_t \* out\_len) <br>_Pop pending transmit frame for Ethernet PHY driver._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_ecat\_master\_read\_sii\_task**](#function-syn_ecat_master_read_sii_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint16\_t station\_addr, uint16\_t word\_offset, uint32\_t \* out\_data) <br>_Protothread Task: Read 32-bit word from slave SII EEPROM via hardware status polling._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_ecat\_master\_scan\_task**](#function-syn_ecat_master_scan_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m) <br>_Protothread Task: Scan EtherCAT bus and assign station addresses._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_ecat\_master\_sdo\_read\_task**](#function-syn_ecat_master_sdo_read_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint16\_t station\_addr, uint16\_t index, uint8\_t subindex, void \* out\_data, size\_t max\_len, size\_t \* out\_len) <br>_Protothread Task: CoE SDO read from slave Object Dictionary with SM1 polling._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ecat\_master\_set\_rx\_frame**](#function-syn_ecat_master_set_rx_frame) ([**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, const uint8\_t \* frame, size\_t len) <br>_Feed received Ethernet frame into Master context._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_ecat\_master\_transition\_task**](#function-syn_ecat_master_transition_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, [**SYN\_EcatState**](syn__ethercat_8h.md#enum-syn_ecatstate) target\_state) <br>_Protothread Task: Drive EtherCAT slaves through state transition._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ecat\_parse\_frame**](#function-syn_ecat_parse_frame) ([**SYN\_EcatNode**](structSYN__EcatNode.md) \* node, const uint8\_t \* frame, size\_t len, uint16\_t \* wkc) <br>_Parse and process a received raw EtherCAT frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ecat\_set\_state**](#function-syn_ecat_set_state) ([**SYN\_EcatNode**](structSYN__EcatNode.md) \* node, [**SYN\_EcatState**](syn__ethercat_8h.md#enum-syn_ecatstate) new\_state) <br>_Request an EtherCAT State Machine (ESM) state transition._  |
|  void | [**syn\_ecat\_update**](#function-syn_ecat_update) ([**SYN\_EcatNode**](structSYN__EcatNode.md) \* node) <br>_Run ESM state machine step._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**load16\_le**](#function-load16_le) (const uint8\_t \* p) <br>_Load 16-bit unsigned integer from little-endian byte order._  |
|  uint32\_t | [**load32\_le**](#function-load32_le) (const uint8\_t \* p) <br>_Load 32-bit unsigned integer from little-endian byte order._  |
|  void | [**store16\_le**](#function-store16_le) (uint8\_t \* p, uint16\_t val) <br>_Store 16-bit unsigned integer in little-endian byte order._  |
|  void | [**store32\_le**](#function-store32_le) (uint8\_t \* p, uint32\_t val) <br>_Store 32-bit unsigned integer in little-endian byte order._  |
|  uint16\_t | [**syn\_ecat\_get\_slave\_station\_addr**](#function-syn_ecat_get_slave_station_addr) (const [**SYN\_EcatMaster**](structSYN__EcatMaster.md) \* m, uint8\_t idx) <br>_Get configured station address for slave index._  |


























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



### function syn\_ecat\_decode\_coe\_sdo\_response 

_Decode SDO Upload response payload from rx\_buf._ 
```C++
SYN_Status syn_ecat_decode_coe_sdo_response (
    SYN_EcatMaster * m,
    size_t rx_len,
    void * out_data,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `rx_len` Received frame length. 
* `out_data` Output destination buffer. 
* `max_len` Maximum output buffer capacity. 
* `out_len` Pointer to receive read byte count. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ecat\_decode\_cyclic 

_Parse LRW cyclic process exchange response from rx\_buf and copy inputs to input\_image._ 
```C++
SYN_Status syn_ecat_decode_cyclic (
    SYN_EcatMaster * m,
    size_t rx_len
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `rx_len` Received frame byte length. 



**Returns:**

SYN\_OK if WKC &gt;= wkc\_expected, SYN\_ERROR on WKC mismatch or parsing error. 





        

<hr>



### function syn\_ecat\_decode\_read\_al\_status 

_Decode Application Layer (AL) Status and Status Code from response._ 
```C++
SYN_Status syn_ecat_decode_read_al_status (
    SYN_EcatMaster * m,
    size_t rx_len,
    SYN_EcatState * state,
    uint16_t * status_code
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `rx_len` Received frame length. 
* `state` [out] Output location for current ESM state. 
* `status_code` [out] Output location for AL status error code. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ecat\_decode\_read\_reg 

_Decode FPRD response datagram for register read._ 
```C++
SYN_Status syn_ecat_decode_read_reg (
    SYN_EcatMaster * m,
    size_t rx_len,
    void * out_data,
    uint16_t len
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `rx_len` Received Ethernet frame length. 
* `out_data` Destination buffer for register data. 
* `len` Expected byte length. 



**Returns:**

SYN\_OK on success, error code otherwise. 





        

<hr>



### function syn\_ecat\_decode\_read\_sii 

_Decode 32-bit word from SII EEPROM read response._ 
```C++
SYN_Status syn_ecat_decode_read_sii (
    SYN_EcatMaster * m,
    size_t rx_len,
    uint32_t * out_val
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `rx_len` Received frame length. 
* `out_val` Pointer to receive parsed 32-bit SII value. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ecat\_decode\_scan\_bus 

_Decode slave count from scan\_bus response frame in rx\_buf._ 
```C++
uint8_t syn_ecat_decode_scan_bus (
    SYN_EcatMaster * m,
    size_t rx_len
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `rx_len` Received frame byte length. 



**Returns:**

Discovered slave count. 





        

<hr>



### function syn\_ecat\_encode\_assign\_addr 

_Encode Auto-Increment Write (APWR) to assign station address._ 
```C++
size_t syn_ecat_encode_assign_addr (
    SYN_EcatMaster * m,
    uint16_t position,
    uint16_t station_addr
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `position` Ring position index (0..slave\_count-1). 
* `station_addr` Configured station address to set (e.g. 0x1001). 



**Returns:**

Transmit frame length in bytes, or 0 on error. 





        

<hr>



### function syn\_ecat\_encode\_coe\_sdo\_read 

_Encode a complete Mailbox-wrapped CoE SDO Upload request frame into tx\_buf._ 
```C++
size_t syn_ecat_encode_coe_sdo_read (
    SYN_EcatMaster * m,
    uint16_t station_addr,
    uint16_t index,
    uint8_t subindex
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `station_addr` Target slave station address. 
* `index` Object Dictionary Index. 
* `subindex` Subindex. 



**Returns:**

Transmit frame byte length, or 0 on error. 





        

<hr>



### function syn\_ecat\_encode\_coe\_sdo\_write 

_Encode a complete Mailbox-wrapped CoE SDO Download request frame into tx\_buf._ 
```C++
size_t syn_ecat_encode_coe_sdo_write (
    SYN_EcatMaster * m,
    uint16_t station_addr,
    uint16_t index,
    uint8_t subindex,
    const void * data,
    size_t data_len
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `station_addr` Target slave station address. 
* `index` Object Dictionary Index. 
* `subindex` Subindex. 
* `data` Pointer to payload bytes (&lt;=4 bytes for expedited SDO). 
* `data_len` Payload byte length. 



**Returns:**

Transmit frame byte length, or 0 on error. 





        

<hr>



### function syn\_ecat\_encode\_cyclic 

_Encode LRW (Logical Read Write) cyclic process exchange frame into tx\_buf._ 
```C++
size_t syn_ecat_encode_cyclic (
    SYN_EcatMaster * m
) 
```





**Parameters:**


* `m` Pointer to master handle. 



**Returns:**

Transmit frame length in bytes, or 0 on error. 





        

<hr>



### function syn\_ecat\_encode\_dc\_configure 

_Encode FPWR to configure DC SYNC0 cycle time and shift for a slave._ 
```C++
size_t syn_ecat_encode_dc_configure (
    SYN_EcatMaster * m,
    uint16_t station_addr,
    uint32_t sync0_cycle_ns,
    int32_t sync0_shift_ns
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `station_addr` Target slave station address. 
* `sync0_cycle_ns` SYNC0 cycle time in nanoseconds. 
* `sync0_shift_ns` SYNC0 shift time in nanoseconds. 



**Returns:**

Transmit frame length in bytes, or 0 on error. 





        

<hr>



### function syn\_ecat\_encode\_dc\_read\_system\_time 

_Encode Auto-Increment Read Multiple Write (ARMW) for DC System Time propagation._ 
```C++
size_t syn_ecat_encode_dc_read_system_time (
    SYN_EcatMaster * m
) 
```





**Parameters:**


* `m` Pointer to master handle. 



**Returns:**

Transmit frame length in bytes, or 0 on error. 





        

<hr>



### function syn\_ecat\_encode\_read\_al\_status 

_Encode Configured Read (FPRD) to read slave Application Layer (AL) Status._ 
```C++
size_t syn_ecat_encode_read_al_status (
    SYN_EcatMaster * m,
    uint16_t station_addr
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `station_addr` Target slave station address. 



**Returns:**

Transmit frame length in bytes, or 0 on error. 





        

<hr>



### function syn\_ecat\_encode\_read\_reg 

_Encode FPRD to read register_ `reg_addr` _from slave_`station_addr` _._
```C++
size_t syn_ecat_encode_read_reg (
    SYN_EcatMaster * m,
    uint16_t station_addr,
    uint16_t reg_addr,
    uint16_t len
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `station_addr` Configured station address of slave. 
* `reg_addr` Register address to read. 
* `len` Number of bytes to read. 



**Returns:**

Encoded Ethernet frame length. 





        

<hr>



### function syn\_ecat\_encode\_read\_sii 

_Encode Configured Read (FPRD) to read SII EEPROM word._ 
```C++
size_t syn_ecat_encode_read_sii (
    SYN_EcatMaster * m,
    uint16_t station_addr,
    uint16_t word_offset
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `station_addr` Target slave station address. 
* `word_offset` SII EEPROM word offset. 



**Returns:**

Transmit frame length in bytes, or 0 on error. 





        

<hr>



### function syn\_ecat\_encode\_scan\_bus 

_Encode Broadcast Read (BRD) to scan bus slave count into master tx\_buf._ 
```C++
size_t syn_ecat_encode_scan_bus (
    SYN_EcatMaster * m
) 
```





**Parameters:**


* `m` Pointer to master handle. 



**Returns:**

Transmit frame length in bytes, or 0 on error. 





        

<hr>



### function syn\_ecat\_encode\_set\_al\_control 

_Encode Configured Write (FPWR) to set slave Application Layer (AL) Control register._ 
```C++
size_t syn_ecat_encode_set_al_control (
    SYN_EcatMaster * m,
    uint16_t station_addr,
    SYN_EcatState state
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `station_addr` Target slave station address. 
* `state` Requested ESM State (INIT, PREOP, SAFEOP, OP). 



**Returns:**

Transmit frame length in bytes, or 0 on error. 





        

<hr>



### function syn\_ecat\_encode\_write\_fmmu 

_Encode Configured Write (FPWR) to configure an FMMU register block._ 
```C++
size_t syn_ecat_encode_write_fmmu (
    SYN_EcatMaster * m,
    uint16_t station_addr,
    uint8_t fmmu_index,
    const SYN_EcatFMMUConfig * cfg
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `station_addr` Target slave station address. 
* `fmmu_index` FMMU index (0..3). 
* `cfg` Pointer to FMMU configuration settings. 



**Returns:**

Transmit frame length in bytes, or 0 on error. 





        

<hr>



### function syn\_ecat\_encode\_write\_reg 

_Encode FPWR to write register_ `reg_addr` _to slave_`station_addr` _._
```C++
size_t syn_ecat_encode_write_reg (
    SYN_EcatMaster * m,
    uint16_t station_addr,
    uint16_t reg_addr,
    const void * data,
    uint16_t len
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `station_addr` Configured station address of slave. 
* `reg_addr` Register address to write. 
* `data` Pointer to data bytes to write. 
* `len` Number of bytes to write. 



**Returns:**

Encoded Ethernet frame length. 





        

<hr>



### function syn\_ecat\_encode\_write\_sm 

_Encode Configured Write (FPWR) to configure a SyncManager register block._ 
```C++
size_t syn_ecat_encode_write_sm (
    SYN_EcatMaster * m,
    uint16_t station_addr,
    uint8_t sm_index,
    const SYN_EcatSMConfig * cfg
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `station_addr` Target slave station address. 
* `sm_index` SyncManager index (0..3). 
* `cfg` Pointer to SyncManager configuration settings. 



**Returns:**

Transmit frame length in bytes, or 0 on error. 





        

<hr>



### function syn\_ecat\_frame\_add\_datagram 

_Add a datagram payload to an in-progress EtherCAT frame._ 
```C++
size_t syn_ecat_frame_add_datagram (
    uint8_t * buf,
    size_t buf_size,
    SYN_EcatCmd cmd,
    uint8_t idx,
    uint32_t addr,
    const uint8_t * data,
    uint16_t data_len,
    bool circ
) 
```





**Parameters:**


* `buf` Target output buffer. 
* `buf_size` Total capacity of output buffer. 
* `cmd` Datagram command type. 
* `idx` Datagram index. 
* `addr` 32-bit address field. 
* `data` Payload data bytes (or NULL if data\_len is 0). 
* `data_len` Payload data length in bytes. 
* `circ` True if circulating datagram flag set. 



**Returns:**

Output byte offset after added datagram, or 0 on error / insufficient space. 





        

<hr>



### function syn\_ecat\_frame\_begin 

_Initialize an EtherCAT frame buffer header._ 
```C++
void syn_ecat_frame_begin (
    uint8_t * buf,
    size_t buf_size
) 
```





**Parameters:**


* `buf` Target output buffer. 
* `buf_size` Total capacity of output buffer. 




        

<hr>



### function syn\_ecat\_frame\_finalize 

_Finalize EtherCAT frame header length field._ 
```C++
size_t syn_ecat_frame_finalize (
    uint8_t * buf
) 
```





**Parameters:**


* `buf` Target output buffer. 



**Returns:**

Total frame size in bytes (including 2-byte EtherCAT header). 





        

<hr>



### function syn\_ecat\_frame\_parse\_next 

_Parse the next datagram from an EtherCAT frame buffer._ 
```C++
bool syn_ecat_frame_parse_next (
    const uint8_t * frame,
    size_t frame_len,
    size_t * offset,
    SYN_EcatDatagramResult * out
) 
```





**Parameters:**


* `frame` Pointer to raw EtherCAT frame bytes (starts with 2-byte EtherCAT header). 
* `frame_len` Total byte length of frame buffer. 
* `offset` In/out byte offset into frame buffer. Set \*offset = 2 to start. 
* `out` [out] Result struct to receive parsed datagram details. 



**Returns:**

true if datagram parsed successfully, false if no more datagrams or malformed. 





        

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



### function syn\_ecat\_master\_cyclic\_task 

_Protothread Task: Execute single cyclic process exchange cycle._ 
```C++
SYN_PT_Status syn_ecat_master_cyclic_task (
    SYN_PT * pt,
    SYN_EcatMaster * m
) 
```





**Parameters:**


* `pt` Protothread state handle. 
* `m` Pointer to master handle. 



**Returns:**

PT\_WAITING if waiting for frames, PT\_ENDED when cycle complete. 





        

<hr>



### function syn\_ecat\_master\_discover\_pdo\_mapping\_task 

_Protothread Task: Read CoE Object Dictionary 0x1C12/0x1C13 to calculate slave Rx/Tx PDO byte lengths._ 
```C++
SYN_PT_Status syn_ecat_master_discover_pdo_mapping_task (
    SYN_PT * pt,
    SYN_EcatMaster * m,
    uint16_t station_addr,
    uint16_t * out_rx_bytes,
    uint16_t * out_tx_bytes
) 
```





**Parameters:**


* `pt` Protothread context pointer. 
* `m` Pointer to master handle. 
* `station_addr` Configured station address of slave. 
* `out_rx_bytes` Destination pointer for Rx PDO byte size. 
* `out_tx_bytes` Destination pointer for Tx PDO byte size. 



**Returns:**

PT\_WAITING if waiting, PT\_ENDED when complete. 





        

<hr>



### function syn\_ecat\_master\_init 

_Initialize EtherCAT Master instance context._ 
```C++
SYN_Status syn_ecat_master_init (
    SYN_EcatMaster * m,
    uint8_t * tx_buf,
    uint16_t tx_buf_size,
    uint8_t * rx_buf,
    uint16_t rx_buf_size,
    uint8_t * output_image,
    uint16_t output_size,
    uint8_t * input_image,
    uint16_t input_size
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `tx_buf` Pointer to static transmit frame buffer. 
* `tx_buf_size` Capacity of transmit frame buffer. 
* `rx_buf` Pointer to static receive frame buffer. 
* `rx_buf_size` Capacity of receive frame buffer. 
* `output_image` Output process image buffer pointer. 
* `output_size` Output process image byte size. 
* `input_image` Input process image buffer pointer. 
* `input_size` Input process image byte size. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if parameters null/invalid. 





        

<hr>



### function syn\_ecat\_master\_pop\_tx\_frame 

_Pop pending transmit frame for Ethernet PHY driver._ 
```C++
bool syn_ecat_master_pop_tx_frame (
    SYN_EcatMaster * m,
    const uint8_t ** out_frame,
    size_t * out_len
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `out_frame` [out] Pointer to receive frame location. 
* `out_len` [out] Pointer to receive frame length. 



**Returns:**

true if frame was popped, false if no frame pending. 





        

<hr>



### function syn\_ecat\_master\_read\_sii\_task 

_Protothread Task: Read 32-bit word from slave SII EEPROM via hardware status polling._ 
```C++
SYN_PT_Status syn_ecat_master_read_sii_task (
    SYN_PT * pt,
    SYN_EcatMaster * m,
    uint16_t station_addr,
    uint16_t word_offset,
    uint32_t * out_data
) 
```





**Parameters:**


* `pt` Protothread context pointer. 
* `m` Pointer to master handle. 
* `station_addr` Configured station address of slave. 
* `word_offset` 16-bit word offset in SII EEPROM. 
* `out_data` Destination pointer for 32-bit word. 



**Returns:**

PT\_WAITING if waiting, PT\_ENDED when complete. 





        

<hr>



### function syn\_ecat\_master\_scan\_task 

_Protothread Task: Scan EtherCAT bus and assign station addresses._ 
```C++
SYN_PT_Status syn_ecat_master_scan_task (
    SYN_PT * pt,
    SYN_EcatMaster * m
) 
```





**Parameters:**


* `pt` Protothread state handle. 
* `m` Pointer to master handle. 



**Returns:**

PT\_WAITING if waiting for frames, PT\_ENDED when scan complete. 





        

<hr>



### function syn\_ecat\_master\_sdo\_read\_task 

_Protothread Task: CoE SDO read from slave Object Dictionary with SM1 polling._ 
```C++
SYN_PT_Status syn_ecat_master_sdo_read_task (
    SYN_PT * pt,
    SYN_EcatMaster * m,
    uint16_t station_addr,
    uint16_t index,
    uint8_t subindex,
    void * out_data,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `pt` Protothread context pointer. 
* `m` Pointer to master handle. 
* `station_addr` Configured station address of slave. 
* `index` 16-bit Object Dictionary index. 
* `subindex` 8-bit sub-index. 
* `out_data` Destination buffer for SDO payload. 
* `max_len` Capacity of destination buffer. 
* `out_len` Destination pointer for actual payload length. 



**Returns:**

PT\_WAITING if waiting, PT\_ENDED when complete. 





        

<hr>



### function syn\_ecat\_master\_set\_rx\_frame 

_Feed received Ethernet frame into Master context._ 
```C++
SYN_Status syn_ecat_master_set_rx_frame (
    SYN_EcatMaster * m,
    const uint8_t * frame,
    size_t len
) 
```





**Parameters:**


* `m` Pointer to master handle. 
* `frame` Pointer to raw frame payload bytes. 
* `len` Frame length in bytes. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ecat\_master\_transition\_task 

_Protothread Task: Drive EtherCAT slaves through state transition._ 
```C++
SYN_PT_Status syn_ecat_master_transition_task (
    SYN_PT * pt,
    SYN_EcatMaster * m,
    SYN_EcatState target_state
) 
```





**Parameters:**


* `pt` Protothread state handle. 
* `m` Pointer to master handle. 
* `target_state` Target ESM state (INIT, PREOP, SAFEOP, OP). 



**Returns:**

PT\_WAITING if waiting for frames, PT\_ENDED when transition complete. 





        

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



### function load32\_le 

_Load 32-bit unsigned integer from little-endian byte order._ 
```C++
static inline uint32_t load32_le (
    const uint8_t * p
) 
```





**Parameters:**


* `p` Pointer to input buffer. 



**Returns:**

32-bit integer value. 





        

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



### function syn\_ecat\_get\_slave\_station\_addr 

_Get configured station address for slave index._ 
```C++
static uint16_t syn_ecat_get_slave_station_addr (
    const SYN_EcatMaster * m,
    uint8_t idx
) 
```





**Parameters:**


* `m` Pointer to EtherCAT Master instance. 
* `idx` Slave index in master slave table. 



**Returns:**

Station address. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ethercat.c`

