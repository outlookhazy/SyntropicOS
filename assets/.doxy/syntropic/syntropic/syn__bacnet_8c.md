

# File syn\_bacnet.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_bacnet.c**](syn__bacnet_8c.md)

[Go to the source code of this file](syn__bacnet_8c_source.md)

_Zero-malloc, cleanroom BACnet MS/TP & APDU protocol stack implementation._ 

* `#include "syn_bacnet.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_bacnet\_add\_object**](#function-syn_bacnet_add_object) ([**SYN\_BACnet\_Node**](structSYN__BACnet__Node.md) \* node, uint8\_t object\_type, uint32\_t instance\_id, float init\_value, const char \* name) <br>_Register an Object in the BACnet Node database._  |
|  uint16\_t | [**syn\_bacnet\_crc16**](#function-syn_bacnet_crc16) (const uint8\_t \* data, size\_t len) <br>_Calculate BACnet MS/TP Data CRC-16._  |
|  uint8\_t | [**syn\_bacnet\_crc8**](#function-syn_bacnet_crc8) (const uint8\_t \* data, size\_t len) <br>_Calculate BACnet MS/TP Header CRC-8._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_bacnet\_mstp\_decode\_frame**](#function-syn_bacnet_mstp_decode_frame) (const uint8\_t \* buf, size\_t len, [**SYN\_BACnet\_MSTP\_Frame**](structSYN__BACnet__MSTP__Frame.md) \* frame) <br>_Decode a raw byte buffer into a BACnet MS/TP Frame structure._  |
|  size\_t | [**syn\_bacnet\_mstp\_encode\_frame**](#function-syn_bacnet_mstp_encode_frame) (uint8\_t frame\_type, uint8\_t dest\_mac, uint8\_t src\_mac, const uint8\_t \* payload, uint16\_t payload\_len, uint8\_t \* out\_buf) <br>_Encode a complete BACnet MS/TP Frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_bacnet\_node\_init**](#function-syn_bacnet_node_init) ([**SYN\_BACnet\_Node**](structSYN__BACnet__Node.md) \* node, uint8\_t mac\_address, uint32\_t device\_id) <br>_Initialize a BACnet MS/TP Node context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_bacnet\_node\_process**](#function-syn_bacnet_node_process) ([**SYN\_BACnet\_Node**](structSYN__BACnet__Node.md) \* node, const [**SYN\_BACnet\_MSTP\_Frame**](structSYN__BACnet__MSTP__Frame.md) \* rx\_frame, [**SYN\_BACnet\_MSTP\_Frame**](structSYN__BACnet__MSTP__Frame.md) \* tx\_frame, [**bool**](syn__defs_8h.md#enum-bool) \* has\_tx) <br>_Process an incoming BACnet MS/TP frame and generate a response if required._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**bacnet\_encode\_iam**](#function-bacnet_encode_iam) (const [**SYN\_BACnet\_Node**](structSYN__BACnet__Node.md) \* node, [**SYN\_BACnet\_MSTP\_Frame**](structSYN__BACnet__MSTP__Frame.md) \* tx\_frame, [**bool**](syn__defs_8h.md#enum-bool) \* has\_tx) <br>_Encode an Unconfirmed I-Am broadcast frame with 4-byte BACnetObjectIdentifier._  |
|  [**SYN\_BACnet\_Object**](structSYN__BACnet__Object.md) \* | [**bacnet\_find\_object**](#function-bacnet_find_object) ([**SYN\_BACnet\_Node**](structSYN__BACnet__Node.md) \* node, uint16\_t obj\_type, uint32\_t instance\_id) <br>_Look up a registered BACnet object by type and instance identifier._  |
|  float | [**bacnet\_get\_be\_float**](#function-bacnet_get_be_float) (const uint8\_t in) <br>_Decode a big-endian IEEE-754 single-precision float._  |
|  void | [**bacnet\_handle\_confirmed\_req**](#function-bacnet_handle_confirmed_req) ([**SYN\_BACnet\_Node**](structSYN__BACnet__Node.md) \* node, const [**SYN\_BACnet\_MSTP\_Frame**](structSYN__BACnet__MSTP__Frame.md) \* rx\_frame, [**SYN\_BACnet\_MSTP\_Frame**](structSYN__BACnet__MSTP__Frame.md) \* tx\_frame, [**bool**](syn__defs_8h.md#enum-bool) \* has\_tx) <br>_Dispatch unicast Confirmed-Request APDU (ignoring broadcast Confirmed-Request)._  |
|  void | [**bacnet\_handle\_read\_prop**](#function-bacnet_handle_read_prop) ([**SYN\_BACnet\_Node**](structSYN__BACnet__Node.md) \* node, const [**SYN\_BACnet\_MSTP\_Frame**](structSYN__BACnet__MSTP__Frame.md) \* rx\_frame, uint8\_t invoke\_id, [**SYN\_BACnet\_MSTP\_Frame**](structSYN__BACnet__MSTP__Frame.md) \* tx\_frame, [**bool**](syn__defs_8h.md#enum-bool) \* has\_tx) <br>_Handle ConfirmedReadProperty APDU and populate Complex-ACK response._  |
|  void | [**bacnet\_handle\_write\_prop**](#function-bacnet_handle_write_prop) ([**SYN\_BACnet\_Node**](structSYN__BACnet__Node.md) \* node, const [**SYN\_BACnet\_MSTP\_Frame**](structSYN__BACnet__MSTP__Frame.md) \* rx\_frame, uint8\_t invoke\_id, [**SYN\_BACnet\_MSTP\_Frame**](structSYN__BACnet__MSTP__Frame.md) \* tx\_frame, [**bool**](syn__defs_8h.md#enum-bool) \* has\_tx) <br>_Handle ConfirmedWriteProperty APDU and populate Simple-ACK response._  |
|  void | [**bacnet\_parse\_obj\_id**](#function-bacnet_parse_obj_id) (const [**SYN\_BACnet\_MSTP\_Frame**](structSYN__BACnet__MSTP__Frame.md) \* rx\_frame, uint16\_t \* out\_type, uint32\_t \* out\_instance) <br>_Extract BACnet object type and instance number from a confirmed request APDU._  |
|  void | [**bacnet\_put\_be\_float**](#function-bacnet_put_be_float) (uint8\_t out, float val) <br>_Encode an IEEE-754 single-precision float in big-endian byte order._  |


























## Public Functions Documentation




### function syn\_bacnet\_add\_object 

_Register an Object in the BACnet Node database._ 
```C++
SYN_Status syn_bacnet_add_object (
    SYN_BACnet_Node * node,
    uint8_t object_type,
    uint32_t instance_id,
    float init_value,
    const char * name
) 
```





**Parameters:**


* `node` Node handle. 
* `object_type` [**SYN\_BACnet\_ObjectType**](syn__bacnet_8h.md#enum-syn_bacnet_objecttype) (AI, AO, BI, BO). 
* `instance_id` Instance ID. 
* `init_value` Initial Present Value. 
* `name` Object Name string. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if database full. 





        

<hr>



### function syn\_bacnet\_crc16 

_Calculate BACnet MS/TP Data CRC-16._ 
```C++
uint16_t syn_bacnet_crc16 (
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `data` Data buffer. 
* `len` Byte count. 



**Returns:**

CRC-16 checksum value. 





        

<hr>



### function syn\_bacnet\_crc8 

_Calculate BACnet MS/TP Header CRC-8._ 
```C++
uint8_t syn_bacnet_crc8 (
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `data` Data buffer. 
* `len` Byte count. 



**Returns:**

CRC-8 checksum value. 





        

<hr>



### function syn\_bacnet\_mstp\_decode\_frame 

_Decode a raw byte buffer into a BACnet MS/TP Frame structure._ 
```C++
bool syn_bacnet_mstp_decode_frame (
    const uint8_t * buf,
    size_t len,
    SYN_BACnet_MSTP_Frame * frame
) 
```





**Parameters:**


* `buf` Input raw frame byte array. 
* `len` Input byte count. 
* `frame` [out] Decoded MS/TP Frame structure. 



**Returns:**

true if valid MS/TP frame header and CRC-8/CRC-16 match. 





        

<hr>



### function syn\_bacnet\_mstp\_encode\_frame 

_Encode a complete BACnet MS/TP Frame._ 
```C++
size_t syn_bacnet_mstp_encode_frame (
    uint8_t frame_type,
    uint8_t dest_mac,
    uint8_t src_mac,
    const uint8_t * payload,
    uint16_t payload_len,
    uint8_t * out_buf
) 
```





**Parameters:**


* `frame_type` Frame Type enum. 
* `dest_mac` Destination MAC address. 
* `src_mac` Source MAC address. 
* `payload` Payload buffer (can be NULL if payload\_len == 0). 
* `payload_len` Payload byte count. 
* `out_buf` Destination output buffer (must be &gt;= payload\_len + 10 bytes). 



**Returns:**

Total encoded frame byte count. 





        

<hr>



### function syn\_bacnet\_node\_init 

_Initialize a BACnet MS/TP Node context._ 
```C++
SYN_Status syn_bacnet_node_init (
    SYN_BACnet_Node * node,
    uint8_t mac_address,
    uint32_t device_id
) 
```





**Parameters:**


* `node` Node handle. 
* `mac_address` Local MAC address (0..127). 
* `device_id` BACnet Device Instance ID (0..4194302). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_bacnet\_node\_process 

_Process an incoming BACnet MS/TP frame and generate a response if required._ 
```C++
SYN_Status syn_bacnet_node_process (
    SYN_BACnet_Node * node,
    const SYN_BACnet_MSTP_Frame * rx_frame,
    SYN_BACnet_MSTP_Frame * tx_frame,
    bool * has_tx
) 
```





**Parameters:**


* `node` Node context. 
* `rx_frame` Incoming received frame. 
* `tx_frame` [out] Outgoing response frame structure. 
* `has_tx` [out] Set to true if a response frame must be transmitted. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Public Static Functions Documentation




### function bacnet\_encode\_iam 

_Encode an Unconfirmed I-Am broadcast frame with 4-byte BACnetObjectIdentifier._ 
```C++
static void bacnet_encode_iam (
    const SYN_BACnet_Node * node,
    SYN_BACnet_MSTP_Frame * tx_frame,
    bool * has_tx
) 
```





**Parameters:**


* `node` BACnet node instance. 
* `tx_frame` Output MS/TP broadcast frame. 
* `has_tx` Set to true when frame is populated. 




        

<hr>



### function bacnet\_find\_object 

_Look up a registered BACnet object by type and instance identifier._ 
```C++
static SYN_BACnet_Object * bacnet_find_object (
    SYN_BACnet_Node * node,
    uint16_t obj_type,
    uint32_t instance_id
) 
```





**Parameters:**


* `node` BACnet node instance. 
* `obj_type` Object type code. 
* `instance_id` Object instance identifier. 



**Returns:**

Matching object pointer, or NULL if not found. 





        

<hr>



### function bacnet\_get\_be\_float 

_Decode a big-endian IEEE-754 single-precision float._ 
```C++
static float bacnet_get_be_float (
    const uint8_t in
) 
```





**Parameters:**


* `in` 4-byte source buffer. 



**Returns:**

Decoded floating-point value. 





        

<hr>



### function bacnet\_handle\_confirmed\_req 

_Dispatch unicast Confirmed-Request APDU (ignoring broadcast Confirmed-Request)._ 
```C++
static void bacnet_handle_confirmed_req (
    SYN_BACnet_Node * node,
    const SYN_BACnet_MSTP_Frame * rx_frame,
    SYN_BACnet_MSTP_Frame * tx_frame,
    bool * has_tx
) 
```





**Parameters:**


* `node` BACnet node instance. 
* `rx_frame` Received MS/TP request frame. 
* `tx_frame` Output MS/TP response frame. 
* `has_tx` Set to true when response frame is populated. 




        

<hr>



### function bacnet\_handle\_read\_prop 

_Handle ConfirmedReadProperty APDU and populate Complex-ACK response._ 
```C++
static void bacnet_handle_read_prop (
    SYN_BACnet_Node * node,
    const SYN_BACnet_MSTP_Frame * rx_frame,
    uint8_t invoke_id,
    SYN_BACnet_MSTP_Frame * tx_frame,
    bool * has_tx
) 
```





**Parameters:**


* `node` BACnet node instance. 
* `rx_frame` Received MS/TP request frame. 
* `invoke_id` Request invoke identifier. 
* `tx_frame` Output MS/TP response frame. 
* `has_tx` Set to true when a response frame is populated. 




        

<hr>



### function bacnet\_handle\_write\_prop 

_Handle ConfirmedWriteProperty APDU and populate Simple-ACK response._ 
```C++
static void bacnet_handle_write_prop (
    SYN_BACnet_Node * node,
    const SYN_BACnet_MSTP_Frame * rx_frame,
    uint8_t invoke_id,
    SYN_BACnet_MSTP_Frame * tx_frame,
    bool * has_tx
) 
```





**Parameters:**


* `node` BACnet node instance. 
* `rx_frame` Received MS/TP request frame. 
* `invoke_id` Request invoke identifier. 
* `tx_frame` Output MS/TP response frame. 
* `has_tx` Set to true when a response frame is populated. 




        

<hr>



### function bacnet\_parse\_obj\_id 

_Extract BACnet object type and instance number from a confirmed request APDU._ 
```C++
static void bacnet_parse_obj_id (
    const SYN_BACnet_MSTP_Frame * rx_frame,
    uint16_t * out_type,
    uint32_t * out_instance
) 
```





**Parameters:**


* `rx_frame` Received MS/TP frame. 
* `out_type` Output 10-bit object type code. 
* `out_instance` Output 22-bit object instance identifier. 




        

<hr>



### function bacnet\_put\_be\_float 

_Encode an IEEE-754 single-precision float in big-endian byte order._ 
```C++
static void bacnet_put_be_float (
    uint8_t out,
    float val
) 
```





**Parameters:**


* `out` 4-byte destination buffer. 
* `val` Floating-point value to encode. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_bacnet.c`

