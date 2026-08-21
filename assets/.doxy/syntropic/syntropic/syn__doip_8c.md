

# File syn\_doip.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_doip.c**](syn__doip_8c.md)

[Go to the source code of this file](syn__doip_8c_source.md)

_SyntropicOS ISO 13400-2 Diagnostic over IP (DoIP) Implementation._ 

* `#include "syntropic/proto/syn_doip.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_doip\_encode\_header**](#function-syn_doip_encode_header) (const [**SYN\_DoIP\_Header**](structSYN__DoIP__Header.md) \* hdr, uint8\_t \* buf, uint16\_t max\_len) <br>_Encode 8-byte DoIP Header into target buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_doip\_init**](#function-syn_doip_init) ([**SYN\_DoIP\_Server**](structSYN__DoIP__Server.md) \* server, uint16\_t logical\_address) <br>_Initialize DoIP Server instance._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_doip\_parse\_header**](#function-syn_doip_parse_header) (const uint8\_t \* buf, uint16\_t len, [**SYN\_DoIP\_Header**](structSYN__DoIP__Header.md) \* hdr) <br>_Parse 8-byte DoIP Header from raw buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_doip\_process\_msg**](#function-syn_doip_process_msg) ([**SYN\_DoIP\_Server**](structSYN__DoIP__Server.md) \* server, [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* uds, const uint8\_t \* rx\_buf, uint16\_t rx\_len, uint8\_t \* tx\_buf, uint16\_t max\_tx\_len, uint16\_t \* tx\_len) <br>_Process incoming DoIP message (UDP or TCP) and route UDS requests to syn\_uds._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_doip\_set\_identifiers**](#function-syn_doip_set_identifiers) ([**SYN\_DoIP\_Server**](structSYN__DoIP__Server.md) \* server, const uint8\_t vin, const uint8\_t eid, const uint8\_t gid) <br>_Set VIN, EID, and GID parameters for vehicle identification responses._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**build\_generic\_nack**](#function-build_generic_nack) (uint8\_t nack\_code, uint8\_t \* tx\_buf, uint16\_t max\_tx, uint16\_t \* tx\_len) <br>_Build DoIP Generic NACK response._  |
|  uint16\_t | [**peek\_u16\_be**](#function-peek_u16_be) (const uint8\_t \* buf) <br>_Unpack 16-bit unsigned integer from big-endian byte array._  |
|  uint32\_t | [**peek\_u32\_be**](#function-peek_u32_be) (const uint8\_t \* buf) <br>_Unpack 32-bit unsigned integer from big-endian byte array._  |
|  void | [**poke\_u16\_be**](#function-poke_u16_be) (uint16\_t val, uint8\_t \* buf) <br>_Pack 16-bit unsigned integer into big-endian byte array._  |
|  void | [**poke\_u32\_be**](#function-poke_u32_be) (uint32\_t val, uint8\_t \* buf) <br>_Pack 32-bit unsigned integer into big-endian byte array._  |


























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
## Public Static Functions Documentation




### function build\_generic\_nack 

_Build DoIP Generic NACK response._ 
```C++
static bool build_generic_nack (
    uint8_t nack_code,
    uint8_t * tx_buf,
    uint16_t max_tx,
    uint16_t * tx_len
) 
```





**Parameters:**


* `nack_code` NACK error code. 
* `tx_buf` Target output buffer. 
* `max_tx` Capacity of tx\_buf. 
* `tx_len` Output length pointer. 



**Returns:**

true on success, false if buffer is too small. 





        

<hr>



### function peek\_u16\_be 

_Unpack 16-bit unsigned integer from big-endian byte array._ 
```C++
static inline uint16_t peek_u16_be (
    const uint8_t * buf
) 
```





**Parameters:**


* `buf` Input byte buffer. 



**Returns:**

16-bit integer value. 





        

<hr>



### function peek\_u32\_be 

_Unpack 32-bit unsigned integer from big-endian byte array._ 
```C++
static inline uint32_t peek_u32_be (
    const uint8_t * buf
) 
```





**Parameters:**


* `buf` Input byte buffer. 



**Returns:**

32-bit integer value. 





        

<hr>



### function poke\_u16\_be 

_Pack 16-bit unsigned integer into big-endian byte array._ 
```C++
static inline void poke_u16_be (
    uint16_t val,
    uint8_t * buf
) 
```





**Parameters:**


* `val` 16-bit integer value. 
* `buf` Target byte buffer. 




        

<hr>



### function poke\_u32\_be 

_Pack 32-bit unsigned integer into big-endian byte array._ 
```C++
static inline void poke_u32_be (
    uint32_t val,
    uint8_t * buf
) 
```





**Parameters:**


* `val` 32-bit integer value. 
* `buf` Target byte buffer. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_doip.c`

