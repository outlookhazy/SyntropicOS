

# File syn\_bacnet.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_bacnet.h**](syn__bacnet_8h.md)

[Go to the source code of this file](syn__bacnet_8h_source.md)

_Zero-malloc, cleanroom BACnet MS/TP & APDU protocol stack (ISO 16484-5)._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_BACnet\_MSTP\_Frame**](structSYN__BACnet__MSTP__Frame.md) <br>_Decoded BACnet MS/TP Frame Structure._  |
| struct | [**SYN\_BACnet\_Node**](structSYN__BACnet__Node.md) <br>_BACnet MS/TP Node Context._  |
| struct | [**SYN\_BACnet\_Object**](structSYN__BACnet__Object.md) <br>_BACnet Object Instance Definition._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_BACnet\_MSTP\_FrameType**](#enum-syn_bacnet_mstp_frametype)  <br>_BACnet MS/TP Frame Types (ISO 16484-5 Clause 9.2)_  |
| enum  | [**SYN\_BACnet\_ObjectType**](#enum-syn_bacnet_objecttype)  <br>_BACnet Object Types (Clause 12)_  |
| enum  | [**SYN\_BACnet\_PropertyID**](#enum-syn_bacnet_propertyid)  <br>_BACnet Property Identifiers._  |
| enum  | [**SYN\_BACnet\_ServiceChoice**](#enum-syn_bacnet_servicechoice)  <br>_BACnet Service Choices (Clause 21)_  |




















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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_BACNET\_BROADCAST\_MAC**](syn__bacnet_8h.md#define-syn_bacnet_broadcast_mac)  `0xFFU`<br> |
| define  | [**SYN\_BACNET\_MAX\_OBJECTS**](syn__bacnet_8h.md#define-syn_bacnet_max_objects)  `16`<br> |
| define  | [**SYN\_BACNET\_MSTP\_PREAMBLE\_1**](syn__bacnet_8h.md#define-syn_bacnet_mstp_preamble_1)  `0x55U`<br> |
| define  | [**SYN\_BACNET\_MSTP\_PREAMBLE\_2**](syn__bacnet_8h.md#define-syn_bacnet_mstp_preamble_2)  `0xFFU`<br> |

## Detailed Description


Provides a lightweight, non-blocking BACnet MS/TP (Master-Slave/Token-Passing) frame encoder/decoder, CRC-8 / CRC-16 verifiers, APDU parser (Who-Is, I-Am, ReadProperty, WriteProperty), and static Object Database engine for embedded targets. 


    
## Public Types Documentation




### enum SYN\_BACnet\_MSTP\_FrameType 

_BACnet MS/TP Frame Types (ISO 16484-5 Clause 9.2)_ 
```C++
enum SYN_BACnet_MSTP_FrameType {
    SYN_BACNET_MSTP_FRAME_TOKEN = 0x00U,
    SYN_BACNET_MSTP_FRAME_POLL_FOR_MASTER = 0x01U,
    SYN_BACNET_MSTP_FRAME_REPLY_TO_POLL_FOR_MASTER = 0x02U,
    SYN_BACNET_MSTP_FRAME_TEST_REQUEST = 0x03U,
    SYN_BACNET_MSTP_FRAME_TEST_RESPONSE = 0x04U,
    SYN_BACNET_MSTP_FRAME_DATA_NOT_EXPECTING_REPLY = 0x05U,
    SYN_BACNET_MSTP_FRAME_DATA_EXPECTING_REPLY = 0x06U,
    SYN_BACNET_MSTP_FRAME_REPLY_POSTPONED = 0x07U
};
```




<hr>



### enum SYN\_BACnet\_ObjectType 

_BACnet Object Types (Clause 12)_ 
```C++
enum SYN_BACnet_ObjectType {
    SYN_BACNET_OBJ_ANALOG_INPUT = 0U,
    SYN_BACNET_OBJ_ANALOG_OUTPUT = 1U,
    SYN_BACNET_OBJ_BINARY_INPUT = 3U,
    SYN_BACNET_OBJ_BINARY_OUTPUT = 4U,
    SYN_BACNET_OBJ_DEVICE = 8U
};
```




<hr>



### enum SYN\_BACnet\_PropertyID 

_BACnet Property Identifiers._ 
```C++
enum SYN_BACnet_PropertyID {
    SYN_BACNET_PROP_PRESENT_VALUE = 85U,
    SYN_BACNET_PROP_STATUS_FLAGS = 111U,
    SYN_BACNET_PROP_OBJECT_IDENTIFIER = 75U,
    SYN_BACNET_PROP_OBJECT_NAME = 77U,
    SYN_BACNET_PROP_OBJECT_TYPE = 79U,
    SYN_BACNET_PROP_VENDOR_ID = 121U
};
```




<hr>



### enum SYN\_BACnet\_ServiceChoice 

_BACnet Service Choices (Clause 21)_ 
```C++
enum SYN_BACnet_ServiceChoice {
    SYN_BACNET_SERVICE_UNCONFIRMED_I_AM = 0U,
    SYN_BACNET_SERVICE_UNCONFIRMED_WHO_IS = 8U,
    SYN_BACNET_SERVICE_CONFIRMED_READ_PROPERTY = 12U,
    SYN_BACNET_SERVICE_CONFIRMED_WRITE_PROPERTY = 15U
};
```




<hr>
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
* `object_type` SYN\_BACnet\_ObjectType (AI, AO, BI, BO). 
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
## Macro Definition Documentation





### define SYN\_BACNET\_BROADCAST\_MAC 

```C++
#define SYN_BACNET_BROADCAST_MAC `0xFFU`
```



Broadcast MAC address (0xFF) 


        

<hr>



### define SYN\_BACNET\_MAX\_OBJECTS 

```C++
#define SYN_BACNET_MAX_OBJECTS `16`
```



Maximum supported BACnet objects per node instance 


        

<hr>



### define SYN\_BACNET\_MSTP\_PREAMBLE\_1 

```C++
#define SYN_BACNET_MSTP_PREAMBLE_1 `0x55U`
```



First MS/TP preamble byte (0x55) 


        

<hr>



### define SYN\_BACNET\_MSTP\_PREAMBLE\_2 

```C++
#define SYN_BACNET_MSTP_PREAMBLE_2 `0xFFU`
```



Second MS/TP preamble byte (0xFF) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_bacnet.h`

