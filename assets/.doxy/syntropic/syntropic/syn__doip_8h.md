

# File syn\_doip.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_doip.h**](syn__doip_8h.md)

[Go to the source code of this file](syn__doip_8h_source.md)

_SyntropicOS ISO 13400-2 Diagnostic over IP (DoIP) Protocol Stack._ [More...](#detailed-description)

* `#include "syntropic/proto/syn_uds.h"`
* `#include "syntropic/syntropic.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DoIP\_Header**](structSYN__DoIP__Header.md) <br>_ISO 13400-2 DoIP Header Structure (8 bytes)_  |
| struct | [**SYN\_DoIP\_Server**](structSYN__DoIP__Server.md) <br>_DoIP Server State Structure._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_doip\_encode\_header**](#function-syn_doip_encode_header) (const [**SYN\_DoIP\_Header**](structSYN__DoIP__Header.md) \* hdr, uint8\_t \* buf, uint16\_t max\_len) <br>_Encode 8-byte DoIP Header into target buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_doip\_init**](#function-syn_doip_init) ([**SYN\_DoIP\_Server**](structSYN__DoIP__Server.md) \* server, uint16\_t logical\_address) <br>_Initialize DoIP Server instance._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_doip\_parse\_header**](#function-syn_doip_parse_header) (const uint8\_t \* buf, uint16\_t len, [**SYN\_DoIP\_Header**](structSYN__DoIP__Header.md) \* hdr) <br>_Parse 8-byte DoIP Header from raw buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_doip\_process\_msg**](#function-syn_doip_process_msg) ([**SYN\_DoIP\_Server**](structSYN__DoIP__Server.md) \* server, [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* uds, const uint8\_t \* rx\_buf, uint16\_t rx\_len, uint8\_t \* tx\_buf, uint16\_t max\_tx\_len, uint16\_t \* tx\_len) <br>_Process incoming DoIP message (UDP or TCP) and route UDS requests to syn\_uds._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_doip\_set\_identifiers**](#function-syn_doip_set_identifiers) ([**SYN\_DoIP\_Server**](structSYN__DoIP__Server.md) \* server, const uint8\_t vin, const uint8\_t eid, const uint8\_t gid) <br>_Set VIN, EID, and GID parameters for vehicle identification responses._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_DOIP\_DEFAULT\_PORT**](syn__doip_8h.md#define-syn_doip_default_port)  `13400U`<br> |
| define  | [**SYN\_DOIP\_HEADER\_SIZE**](syn__doip_8h.md#define-syn_doip_header_size)  `8U`<br> |
| define  | [**SYN\_DOIP\_INVERSE\_PROTOCOL\_VERSION**](syn__doip_8h.md#define-syn_doip_inverse_protocol_version)  `0xFDU`<br> |
| define  | [**SYN\_DOIP\_NACK\_INCORRECT\_PATTERN**](syn__doip_8h.md#define-syn_doip_nack_incorrect_pattern)  `0x00U`<br> |
| define  | [**SYN\_DOIP\_NACK\_INVALID\_PAYLOAD\_LENGTH**](syn__doip_8h.md#define-syn_doip_nack_invalid_payload_length)  `0x04U`<br> |
| define  | [**SYN\_DOIP\_NACK\_MESSAGE\_TOO\_LARGE**](syn__doip_8h.md#define-syn_doip_nack_message_too_large)  `0x02U`<br> |
| define  | [**SYN\_DOIP\_NACK\_OUT\_OF\_MEMORY**](syn__doip_8h.md#define-syn_doip_nack_out_of_memory)  `0x03U`<br> |
| define  | [**SYN\_DOIP\_NACK\_UNKNOWN\_PAYLOAD\_TYPE**](syn__doip_8h.md#define-syn_doip_nack_unknown_payload_type)  `0x01U`<br> |
| define  | [**SYN\_DOIP\_PROTOCOL\_VERSION**](syn__doip_8h.md#define-syn_doip_protocol_version)  `0x02U`<br> |
| define  | [**SYN\_DOIP\_ROUTING\_DENIED\_DIFFERENT\_PORT**](syn__doip_8h.md#define-syn_doip_routing_denied_different_port)  `0x04U`<br> |
| define  | [**SYN\_DOIP\_ROUTING\_DENIED\_UNKNOWN\_TESTER**](syn__doip_8h.md#define-syn_doip_routing_denied_unknown_tester)  `0x02U`<br> |
| define  | [**SYN\_DOIP\_ROUTING\_SUCCESS**](syn__doip_8h.md#define-syn_doip_routing_success)  `0x00U`<br> |
| define  | [**SYN\_DOIP\_TYPE\_ALIVE\_CHECK\_REQ**](syn__doip_8h.md#define-syn_doip_type_alive_check_req)  `0x0007U`<br> |
| define  | [**SYN\_DOIP\_TYPE\_ALIVE\_CHECK\_RESP**](syn__doip_8h.md#define-syn_doip_type_alive_check_resp)  `0x0008U`<br> |
| define  | [**SYN\_DOIP\_TYPE\_DIAGNOSTIC\_MSG**](syn__doip_8h.md#define-syn_doip_type_diagnostic_msg)  `0x8001U`<br> |
| define  | [**SYN\_DOIP\_TYPE\_DIAGNOSTIC\_MSG\_ACK**](syn__doip_8h.md#define-syn_doip_type_diagnostic_msg_ack)  `0x8002U`<br> |
| define  | [**SYN\_DOIP\_TYPE\_DIAGNOSTIC\_MSG\_NACK**](syn__doip_8h.md#define-syn_doip_type_diagnostic_msg_nack)  `0x8003U`<br> |
| define  | [**SYN\_DOIP\_TYPE\_GENERIC\_NACK**](syn__doip_8h.md#define-syn_doip_type_generic_nack)  `0x0000U`<br> |
| define  | [**SYN\_DOIP\_TYPE\_ROUTING\_ACTIVATION\_REQ**](syn__doip_8h.md#define-syn_doip_type_routing_activation_req)  `0x0005U`<br> |
| define  | [**SYN\_DOIP\_TYPE\_ROUTING\_ACTIVATION\_RESP**](syn__doip_8h.md#define-syn_doip_type_routing_activation_resp)  `0x0006U`<br> |
| define  | [**SYN\_DOIP\_TYPE\_VEHICLE\_ANNOUNCEMENT**](syn__doip_8h.md#define-syn_doip_type_vehicle_announcement)  `0x0004U`<br> |
| define  | [**SYN\_DOIP\_TYPE\_VEHICLE\_ID\_REQ**](syn__doip_8h.md#define-syn_doip_type_vehicle_id_req)  `0x0001U`<br> |
| define  | [**SYN\_DOIP\_TYPE\_VEHICLE\_ID\_REQ\_EID**](syn__doip_8h.md#define-syn_doip_type_vehicle_id_req_eid)  `0x0002U`<br> |
| define  | [**SYN\_DOIP\_TYPE\_VEHICLE\_ID\_REQ\_VIN**](syn__doip_8h.md#define-syn_doip_type_vehicle_id_req_vin)  `0x0003U`<br> |

## Detailed Description


Implements lightweight, zero-malloc ISO 13400-2 DoIP header serialization, vehicle identification, routing activation, and UDS (ISO 14229-1) message routing over TCP/UDP. 


    
## Public Functions Documentation




### function syn\_doip\_encode\_header 

_Encode 8-byte DoIP Header into target buffer._ 
```C++
uint16_t syn_doip_encode_header (
    const SYN_DoIP_Header * hdr,
    uint8_t * buf,
    uint16_t max_len
) 
```





**Parameters:**


* `hdr` Input header structure. 
* `buf` Output buffer. 
* `max_len` Output buffer capacity (must be &gt;= 8). 



**Returns:**

Length written (8), or 0 on error. 





        

<hr>



### function syn\_doip\_init 

_Initialize DoIP Server instance._ 
```C++
bool syn_doip_init (
    SYN_DoIP_Server * server,
    uint16_t logical_address
) 
```





**Parameters:**


* `server` Pointer to DoIP server instance. 
* `logical_address` ECU logical address (e.g. 0x1001). 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_doip\_parse\_header 

_Parse 8-byte DoIP Header from raw buffer._ 
```C++
bool syn_doip_parse_header (
    const uint8_t * buf,
    uint16_t len,
    SYN_DoIP_Header * hdr
) 
```





**Parameters:**


* `buf` Input buffer. 
* `len` Input buffer length. 
* `hdr` Output header structure. 



**Returns:**

true if header is valid, false otherwise. 





        

<hr>



### function syn\_doip\_process\_msg 

_Process incoming DoIP message (UDP or TCP) and route UDS requests to syn\_uds._ 
```C++
bool syn_doip_process_msg (
    SYN_DoIP_Server * server,
    SYN_UDS_Server * uds,
    const uint8_t * rx_buf,
    uint16_t rx_len,
    uint8_t * tx_buf,
    uint16_t max_tx_len,
    uint16_t * tx_len
) 
```





**Parameters:**


* `server` Pointer to DoIP server instance. 
* `uds` Pointer to UDS server instance. 
* `rx_buf` Incoming raw DoIP packet. 
* `rx_len` Incoming packet length. 
* `tx_buf` Output buffer for DoIP response frame. 
* `max_tx_len` Maximum output buffer capacity. 
* `tx_len` Output pointer for written response length. 



**Returns:**

true if message was processed successfully, false on invalid request/null. 





        

<hr>



### function syn\_doip\_set\_identifiers 

_Set VIN, EID, and GID parameters for vehicle identification responses._ 
```C++
bool syn_doip_set_identifiers (
    SYN_DoIP_Server * server,
    const uint8_t vin,
    const uint8_t eid,
    const uint8_t gid
) 
```





**Parameters:**


* `server` Pointer to DoIP server. 
* `vin` 17-byte VIN buffer. 
* `eid` 6-byte EID (MAC) buffer. 
* `gid` 6-byte GID buffer. 



**Returns:**

true on success, false on NULL pointers. 





        

<hr>
## Macro Definition Documentation





### define SYN\_DOIP\_DEFAULT\_PORT 

```C++
#define SYN_DOIP_DEFAULT_PORT `13400U`
```



Standard DoIP TCP/UDP port 


        

<hr>



### define SYN\_DOIP\_HEADER\_SIZE 

```C++
#define SYN_DOIP_HEADER_SIZE `8U`
```



Fixed 8-byte DoIP header size 


        

<hr>



### define SYN\_DOIP\_INVERSE\_PROTOCOL\_VERSION 

```C++
#define SYN_DOIP_INVERSE_PROTOCOL_VERSION `0xFDU`
```



Bitwise inverse of protocol version 


        

<hr>



### define SYN\_DOIP\_NACK\_INCORRECT\_PATTERN 

```C++
#define SYN_DOIP_NACK_INCORRECT_PATTERN `0x00U`
```



ISO 13400-2 Generic NACK Codes Incorrect header pattern 


        

<hr>



### define SYN\_DOIP\_NACK\_INVALID\_PAYLOAD\_LENGTH 

```C++
#define SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH `0x04U`
```



Invalid payload length 


        

<hr>



### define SYN\_DOIP\_NACK\_MESSAGE\_TOO\_LARGE 

```C++
#define SYN_DOIP_NACK_MESSAGE_TOO_LARGE `0x02U`
```



Message payload too large 


        

<hr>



### define SYN\_DOIP\_NACK\_OUT\_OF\_MEMORY 

```C++
#define SYN_DOIP_NACK_OUT_OF_MEMORY `0x03U`
```



Out of memory 


        

<hr>



### define SYN\_DOIP\_NACK\_UNKNOWN\_PAYLOAD\_TYPE 

```C++
#define SYN_DOIP_NACK_UNKNOWN_PAYLOAD_TYPE `0x01U`
```



Unknown payload type 


        

<hr>



### define SYN\_DOIP\_PROTOCOL\_VERSION 

```C++
#define SYN_DOIP_PROTOCOL_VERSION `0x02U`
```



ISO 13400-2 Protocol Constants ISO 13400-2 protocol version 


        

<hr>



### define SYN\_DOIP\_ROUTING\_DENIED\_DIFFERENT\_PORT 

```C++
#define SYN_DOIP_ROUTING_DENIED_DIFFERENT_PORT `0x04U`
```



Denied: Invalid socket port 


        

<hr>



### define SYN\_DOIP\_ROUTING\_DENIED\_UNKNOWN\_TESTER 

```C++
#define SYN_DOIP_ROUTING_DENIED_UNKNOWN_TESTER `0x02U`
```



Denied: Unknown tester address 


        

<hr>



### define SYN\_DOIP\_ROUTING\_SUCCESS 

```C++
#define SYN_DOIP_ROUTING_SUCCESS `0x00U`
```



ISO 13400-2 Routing Activation Response Codes Routing activation successful 


        

<hr>



### define SYN\_DOIP\_TYPE\_ALIVE\_CHECK\_REQ 

```C++
#define SYN_DOIP_TYPE_ALIVE_CHECK_REQ `0x0007U`
```



Alive Check Request 


        

<hr>



### define SYN\_DOIP\_TYPE\_ALIVE\_CHECK\_RESP 

```C++
#define SYN_DOIP_TYPE_ALIVE_CHECK_RESP `0x0008U`
```



Alive Check Response 


        

<hr>



### define SYN\_DOIP\_TYPE\_DIAGNOSTIC\_MSG 

```C++
#define SYN_DOIP_TYPE_DIAGNOSTIC_MSG `0x8001U`
```



UDS Diagnostic Message 


        

<hr>



### define SYN\_DOIP\_TYPE\_DIAGNOSTIC\_MSG\_ACK 

```C++
#define SYN_DOIP_TYPE_DIAGNOSTIC_MSG_ACK `0x8002U`
```



Diagnostic Message Positive Ack 


        

<hr>



### define SYN\_DOIP\_TYPE\_DIAGNOSTIC\_MSG\_NACK 

```C++
#define SYN_DOIP_TYPE_DIAGNOSTIC_MSG_NACK `0x8003U`
```



Diagnostic Message Negative Ack 


        

<hr>



### define SYN\_DOIP\_TYPE\_GENERIC\_NACK 

```C++
#define SYN_DOIP_TYPE_GENERIC_NACK `0x0000U`
```



ISO 13400-2 Payload Types Generic NACK payload type 


        

<hr>



### define SYN\_DOIP\_TYPE\_ROUTING\_ACTIVATION\_REQ 

```C++
#define SYN_DOIP_TYPE_ROUTING_ACTIVATION_REQ `0x0005U`
```



Routing Activation Request 


        

<hr>



### define SYN\_DOIP\_TYPE\_ROUTING\_ACTIVATION\_RESP 

```C++
#define SYN_DOIP_TYPE_ROUTING_ACTIVATION_RESP `0x0006U`
```



Routing Activation Response 


        

<hr>



### define SYN\_DOIP\_TYPE\_VEHICLE\_ANNOUNCEMENT 

```C++
#define SYN_DOIP_TYPE_VEHICLE_ANNOUNCEMENT `0x0004U`
```



Vehicle Announcement Message 


        

<hr>



### define SYN\_DOIP\_TYPE\_VEHICLE\_ID\_REQ 

```C++
#define SYN_DOIP_TYPE_VEHICLE_ID_REQ `0x0001U`
```



Vehicle Identification Request 


        

<hr>



### define SYN\_DOIP\_TYPE\_VEHICLE\_ID\_REQ\_EID 

```C++
#define SYN_DOIP_TYPE_VEHICLE_ID_REQ_EID `0x0002U`
```



Vehicle ID Request with EID 


        

<hr>



### define SYN\_DOIP\_TYPE\_VEHICLE\_ID\_REQ\_VIN 

```C++
#define SYN_DOIP_TYPE_VEHICLE_ID_REQ_VIN `0x0003U`
```



Vehicle ID Request with VIN 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_doip.h`

