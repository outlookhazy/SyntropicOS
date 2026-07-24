

# File syn\_modbus\_tcp.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_modbus\_tcp.c**](syn__modbus__tcp_8c.md)

[Go to the source code of this file](syn__modbus__tcp_8c_source.md)

_Modbus TCP ADU (MBAP) framing implementation._ 

* `#include "syn_modbus_tcp.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_pack.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_mbap\_decode\_header**](#function-syn_mbap_decode_header) (const uint8\_t \* buf, [**SYN\_MBAP\_Header**](structSYN__MBAP__Header.md) \* hdr) <br>_Decode an MBAP header from a byte buffer._  |
|  void | [**syn\_mbap\_encode\_header**](#function-syn_mbap_encode_header) (const [**SYN\_MBAP\_Header**](structSYN__MBAP__Header.md) \* hdr, uint8\_t \* buf) <br>_Encode an MBAP header into a byte buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_modbus\_tcp\_build\_client\_adu**](#function-syn_modbus_tcp_build_client_adu) (uint16\_t transaction\_id, uint8\_t unit\_id, const uint8\_t \* pdu, uint16\_t pdu\_len, uint8\_t \* req\_adu, uint16\_t req\_max, uint16\_t \* req\_len) <br>_Build a Modbus TCP Master request ADU._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_modbus\_tcp\_process\_slave**](#function-syn_modbus_tcp_process_slave) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb, const uint8\_t \* req\_adu, uint16\_t req\_len, uint8\_t \* resp\_adu, uint16\_t resp\_max, uint16\_t \* resp\_len) <br>_Process an incoming Modbus TCP ADU frame on a Modbus Slave instance._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**get\_response\_pdu\_len**](#function-get_response_pdu_len) (const uint8\_t \* buf) <br> |


























## Public Functions Documentation




### function syn\_mbap\_decode\_header 

_Decode an MBAP header from a byte buffer._ 
```C++
bool syn_mbap_decode_header (
    const uint8_t * buf,
    SYN_MBAP_Header * hdr
) 
```





**Parameters:**


* `buf` Input byte buffer (must be at least 7 bytes). 
* `hdr` Pointer to destination MBAP header structure. 



**Returns:**

true if valid Modbus protocol ID (0x0000). 





        

<hr>



### function syn\_mbap\_encode\_header 

_Encode an MBAP header into a byte buffer._ 
```C++
void syn_mbap_encode_header (
    const SYN_MBAP_Header * hdr,
    uint8_t * buf
) 
```





**Parameters:**


* `hdr` Pointer to MBAP header structure. 
* `buf` Output byte buffer (must be at least 7 bytes). 




        

<hr>



### function syn\_modbus\_tcp\_build\_client\_adu 

_Build a Modbus TCP Master request ADU._ 
```C++
SYN_Status syn_modbus_tcp_build_client_adu (
    uint16_t transaction_id,
    uint8_t unit_id,
    const uint8_t * pdu,
    uint16_t pdu_len,
    uint8_t * req_adu,
    uint16_t req_max,
    uint16_t * req_len
) 
```





**Parameters:**


* `transaction_id` Transaction sequence number. 
* `unit_id` Target Unit ID (slave address). 
* `pdu` Raw PDU bytes (Function Code + Data). 
* `pdu_len` PDU length in bytes. 
* `req_adu` Output buffer for complete Modbus TCP ADU. 
* `req_max` Output buffer capacity. 
* `req_len` Out: Total ADU length (7 + pdu\_len). 



**Returns:**

SYN\_OK on success, SYN\_NO\_MEMORY if buffer too small. 





        

<hr>



### function syn\_modbus\_tcp\_process\_slave 

_Process an incoming Modbus TCP ADU frame on a Modbus Slave instance._ 
```C++
bool syn_modbus_tcp_process_slave (
    SYN_Modbus * mb,
    const uint8_t * req_adu,
    uint16_t req_len,
    uint8_t * resp_adu,
    uint16_t resp_max,
    uint16_t * resp_len
) 
```



Takes a full Modbus TCP ADU (MBAP header + PDU), processes the PDU using the Modbus slave engine, and builds the corresponding Modbus TCP response ADU.




**Parameters:**


* `mb` Modbus RTU/PDU slave instance. 
* `req_adu` Input Modbus TCP request ADU buffer. 
* `req_len` Length of request ADU in bytes. 
* `resp_adu` Output Modbus TCP response ADU buffer. 
* `resp_max` Capacity of response ADU buffer. 
* `resp_len` Out: Length of generated response ADU in bytes. 



**Returns:**

true if response ADU generated, false if no response (or invalid ADU). 





        

<hr>
## Public Static Functions Documentation




### function get\_response\_pdu\_len 

```C++
static uint16_t get_response_pdu_len (
    const uint8_t * buf
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_modbus_tcp.c`

