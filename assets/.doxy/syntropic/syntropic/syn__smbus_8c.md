

# File syn\_smbus.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_smbus.c**](syn__smbus_8c.md)

[Go to the source code of this file](syn__smbus_8c_source.md)

_SMBus Protocol Engine implementation._ 

* `#include "syn_smbus.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_smbus\_calc\_pec**](#function-syn_smbus_calc_pec) (uint8\_t init\_crc, const uint8\_t \* data, size\_t len) <br>_Calculate SMBus Packet Error Checking (PEC) CRC-8 byte. Polynomial: x^8 + x^2 + x + 1 (0x07), Initial value: 0x00._  |
|  uint8\_t | [**syn\_smbus\_calc\_transaction\_pec**](#function-syn_smbus_calc_transaction_pec) (const [**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* req\_pkt, const uint8\_t \* rx\_data, size\_t rx\_data\_len) <br>_Calculate full SMBus 2.0 transaction PEC CRC-8 across write header, repeated-start, and response payload._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_smbus\_decode\_packet**](#function-syn_smbus_decode_packet) ([**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, const uint8\_t \* rx\_buf, size\_t rx\_len, [**SYN\_SMBUS\_Protocol**](syn__smbus_8h.md#enum-syn_smbus_protocol) proto, [**bool**](syn__defs_8h.md#enum-bool) has\_pec) <br>_Decode an SMBus byte stream into an SMBus packet structure._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_smbus\_encode\_packet**](#function-syn_smbus_encode_packet) (const [**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, uint8\_t \* tx\_buf, size\_t buf\_size, size\_t \* out\_len) <br>_Encode an SMBus packet into a byte stream for transmission._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**smbus\_calc\_call\_pec**](#function-smbus_calc_call_pec) (const [**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* req\_pkt, uint8\_t addr\_w, uint8\_t addr\_r) <br>_Compute running PEC seed over request header/payload for Process Call transactions._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**smbus\_encode\_proc\_call**](#function-smbus_encode_proc_call) (const [**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, uint8\_t addr\_w, uint8\_t \* tx\_buf, size\_t buf\_size, size\_t \* out\_idx) <br>_Encode master write phase for SMBus Process Call or Block Process Call._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**smbus\_encode\_read\_or\_call**](#function-smbus_encode_read_or_call) (const [**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, uint8\_t addr\_w, uint8\_t addr\_r, uint8\_t \* tx\_buf, size\_t buf\_size, size\_t \* out\_idx) <br>_Encode SMBus Quick, Receive Byte, Read, or Process Call request phase._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**smbus\_encode\_write**](#function-smbus_encode_write) (const [**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, uint8\_t addr\_w, uint8\_t \* tx\_buf, size\_t buf\_size, size\_t pec\_extra, size\_t \* out\_idx) <br>_Encode SMBus write-direction packet bytes prior to optional PEC byte._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**smbus\_has\_request\_pec**](#function-smbus_has_request_pec) ([**SYN\_SMBUS\_Protocol**](syn__smbus_8h.md#enum-syn_smbus_protocol) proto) <br>_Check whether an SMBus transaction appends PEC directly to the master request phase._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**smbus\_match\_addr\_w**](#function-smbus_match_addr_w) (uint8\_t slave\_addr, uint8\_t wire\_addr\_w) <br>_Check whether wire address byte matches expected 7-bit slave write address._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**smbus\_parse\_block**](#function-smbus_parse_block) (const [**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, const uint8\_t \* rx\_buf, size\_t data\_len, size\_t \* payload\_off, uint8\_t \* payload\_len, [**bool**](syn__defs_8h.md#enum-bool) \* is\_full) <br>_Locate variable-length SMBus block payload in full-wire or raw-response layout._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**smbus\_parse\_bpc\_block**](#function-smbus_parse_bpc_block) (uint8\_t slave\_addr, const uint8\_t \* rx\_buf, size\_t data\_len, size\_t \* payload\_off, uint8\_t \* payload\_len) <br>_Parse full-wire SMBus Block Process Call buffer and locate read-phase block payload._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**smbus\_parse\_full\_block**](#function-smbus_parse_full_block) (const [**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, const uint8\_t \* rx\_buf, size\_t data\_len, size\_t \* payload\_off, uint8\_t \* payload\_len) <br>_Match full-wire SMBus block framing before raw count+payload fallback._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**smbus\_parse\_rx\_payload**](#function-smbus_parse_rx_payload) (const [**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, const uint8\_t \* rx\_buf, size\_t data\_len, [**bool**](syn__defs_8h.md#enum-bool) has\_pec, size\_t \* payload\_off, uint8\_t \* payload\_len, [**bool**](syn__defs_8h.md#enum-bool) \* is\_full) <br>_Dispatch SMBus payload framing parser by transaction protocol family._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**smbus\_parse\_u16**](#function-smbus_parse_u16) ([**SYN\_SMBUS\_Protocol**](syn__smbus_8h.md#enum-syn_smbus_protocol) proto, size\_t data\_len, [**bool**](syn__defs_8h.md#enum-bool) has\_pec, size\_t \* payload\_off, uint8\_t \* payload\_len, [**bool**](syn__defs_8h.md#enum-bool) \* is\_full) <br>_Locate 2-byte SMBus word payload in either raw-response or full-wire buffer layout._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**smbus\_parse\_u8**](#function-smbus_parse_u8) ([**SYN\_SMBUS\_Protocol**](syn__smbus_8h.md#enum-syn_smbus_protocol) proto, size\_t data\_len, [**bool**](syn__defs_8h.md#enum-bool) has\_pec, size\_t \* payload\_off, uint8\_t \* payload\_len, [**bool**](syn__defs_8h.md#enum-bool) \* is\_full) <br>_Locate 1-byte SMBus payload in either raw-response or full-wire buffer layout._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**smbus\_verify\_rx\_pec**](#function-smbus_verify_rx_pec) ([**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, const uint8\_t \* rx\_buf, size\_t rx\_len, size\_t data\_len, [**bool**](syn__defs_8h.md#enum-bool) has\_pec, [**bool**](syn__defs_8h.md#enum-bool) is\_full) <br>_Verify trailing SMBus PEC byte on decoded packet when PEC checking is enabled._  |


























## Public Functions Documentation




### function syn\_smbus\_calc\_pec 

_Calculate SMBus Packet Error Checking (PEC) CRC-8 byte. Polynomial: x^8 + x^2 + x + 1 (0x07), Initial value: 0x00._ 
```C++
uint8_t syn_smbus_calc_pec (
    uint8_t init_crc,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `init_crc` Running CRC value (0x00 for fresh calculation). 
* `data` Pointer to data buffer. 
* `len` Data length in bytes. 



**Returns:**

Computed PEC byte. 





        

<hr>



### function syn\_smbus\_calc\_transaction\_pec 

_Calculate full SMBus 2.0 transaction PEC CRC-8 across write header, repeated-start, and response payload._ 
```C++
uint8_t syn_smbus_calc_transaction_pec (
    const SYN_SMBUS_Packet * req_pkt,
    const uint8_t * rx_data,
    size_t rx_data_len
) 
```





**Parameters:**


* `req_pkt` Pointer to request packet holding slave\_addr, command, proto, and write data. 
* `rx_data` Pointer to slave read-phase response bytes (excluding trailing PEC byte). 
* `rx_data_len` Number of response payload bytes in rx\_data. 



**Returns:**

Computed transaction PEC byte. 





        

<hr>



### function syn\_smbus\_decode\_packet 

_Decode an SMBus byte stream into an SMBus packet structure._ 
```C++
SYN_Status syn_smbus_decode_packet (
    SYN_SMBUS_Packet * pkt,
    const uint8_t * rx_buf,
    size_t rx_len,
    SYN_SMBUS_Protocol proto,
    bool has_pec
) 
```





**Parameters:**


* `pkt` Pointer to destination packet structure. 
* `rx_buf` Input buffer containing received bytes. 
* `rx_len` Length of received byte stream. 
* `proto` Protocol format expected. 
* `has_pec` True if PEC byte is appended at end of rx\_buf. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if PEC mismatch, or SYN\_INVALID\_PARAM. 





        

<hr>



### function syn\_smbus\_encode\_packet 

_Encode an SMBus packet into a byte stream for transmission._ 
```C++
SYN_Status syn_smbus_encode_packet (
    const SYN_SMBUS_Packet * pkt,
    uint8_t * tx_buf,
    size_t buf_size,
    size_t * out_len
) 
```





**Parameters:**


* `pkt` Pointer to packet structure. 
* `tx_buf` Output buffer. 
* `buf_size` Size of output buffer. 
* `out_len` Pointer to receive encoded byte count. 



**Returns:**

SYN\_OK on success, or error status. 





        

<hr>
## Public Static Functions Documentation




### function smbus\_calc\_call\_pec 

_Compute running PEC seed over request header/payload for Process Call transactions._ 
```C++
static uint8_t smbus_calc_call_pec (
    const SYN_SMBUS_Packet * req_pkt,
    uint8_t addr_w,
    uint8_t addr_r
) 
```





**Parameters:**


* `req_pkt` Request SMBus packet descriptor. 
* `addr_w` 8-bit shifted slave write address byte. 
* `addr_r` 8-bit shifted slave read address byte. 



**Returns:**

Running CRC-8 PEC value prior to slave read data bytes. 





        

<hr>



### function smbus\_encode\_proc\_call 

_Encode master write phase for SMBus Process Call or Block Process Call._ 
```C++
static SYN_Status smbus_encode_proc_call (
    const SYN_SMBUS_Packet * pkt,
    uint8_t addr_w,
    uint8_t * tx_buf,
    size_t buf_size,
    size_t * out_idx
) 
```





**Parameters:**


* `pkt` SMBus packet descriptor. 
* `addr_w` 8-bit shifted slave write address byte. 
* `tx_buf` Output transmit buffer. 
* `buf_size` Capacity of tx\_buf in bytes. 
* `out_idx` Output encoded byte length. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if tx\_buf is too small. 





        

<hr>



### function smbus\_encode\_read\_or\_call 

_Encode SMBus Quick, Receive Byte, Read, or Process Call request phase._ 
```C++
static SYN_Status smbus_encode_read_or_call (
    const SYN_SMBUS_Packet * pkt,
    uint8_t addr_w,
    uint8_t addr_r,
    uint8_t * tx_buf,
    size_t buf_size,
    size_t * out_idx
) 
```





**Parameters:**


* `pkt` SMBus packet descriptor. 
* `addr_w` 8-bit shifted slave write address byte. 
* `addr_r` 8-bit shifted slave read address byte. 
* `tx_buf` Output transmit buffer. 
* `buf_size` Capacity of tx\_buf in bytes. 
* `out_idx` Output encoded byte length. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on buffer overflow, SYN\_INVALID\_PARAM on unknown protocol. 





        

<hr>



### function smbus\_encode\_write 

_Encode SMBus write-direction packet bytes prior to optional PEC byte._ 
```C++
static SYN_Status smbus_encode_write (
    const SYN_SMBUS_Packet * pkt,
    uint8_t addr_w,
    uint8_t * tx_buf,
    size_t buf_size,
    size_t pec_extra,
    size_t * out_idx
) 
```





**Parameters:**


* `pkt` SMBus packet descriptor. 
* `addr_w` 8-bit shifted slave write address byte. 
* `tx_buf` Output transmit buffer. 
* `buf_size` Capacity of tx\_buf in bytes. 
* `pec_extra` Additional byte count reserved for PEC (0 or 1). 
* `out_idx` Output encoded byte length before PEC. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if tx\_buf is too small. 





        

<hr>



### function smbus\_has\_request\_pec 

_Check whether an SMBus transaction appends PEC directly to the master request phase._ 
```C++
static bool smbus_has_request_pec (
    SYN_SMBUS_Protocol proto
) 
```





**Parameters:**


* `proto` SMBus transaction protocol type. 



**Returns:**

true for write-only protocols with request PEC, false otherwise. 





        

<hr>



### function smbus\_match\_addr\_w 

_Check whether wire address byte matches expected 7-bit slave write address._ 
```C++
static bool smbus_match_addr_w (
    uint8_t slave_addr,
    uint8_t wire_addr_w
) 
```





**Parameters:**


* `slave_addr` Expected 7-bit slave address (0 if wildcard/unspecified). 
* `wire_addr_w` First byte on SMBus wire. 



**Returns:**

true if wire\_addr\_w is a valid matching write address byte, false otherwise. 





        

<hr>



### function smbus\_parse\_block 

_Locate variable-length SMBus block payload in full-wire or raw-response layout._ 
```C++
static SYN_Status smbus_parse_block (
    const SYN_SMBUS_Packet * pkt,
    const uint8_t * rx_buf,
    size_t data_len,
    size_t * payload_off,
    uint8_t * payload_len,
    bool * is_full
) 
```





**Parameters:**


* `pkt` SMBus packet context providing protocol and expected slave address. 
* `rx_buf` Received wire or response buffer. 
* `data_len` Buffer length excluding trailing PEC byte. 
* `payload_off` Output byte offset of block payload bytes. 
* `payload_len` Output block byte count. 
* `is_full` Set true if rx\_buf contains full wire framing, false if count+payload only. 



**Returns:**

SYN\_OK on valid block framing, SYN\_INVALID\_PARAM otherwise. 





        

<hr>



### function smbus\_parse\_bpc\_block 

_Parse full-wire SMBus Block Process Call buffer and locate read-phase block payload._ 
```C++
static bool smbus_parse_bpc_block (
    uint8_t slave_addr,
    const uint8_t * rx_buf,
    size_t data_len,
    size_t * payload_off,
    uint8_t * payload_len
) 
```





**Parameters:**


* `slave_addr` Expected 7-bit slave address (0 if wildcard/unspecified). 
* `rx_buf` Received wire buffer. 
* `data_len` Buffer length excluding trailing PEC byte. 
* `payload_off` Output byte offset of response block data. 
* `payload_len` Output response block byte count. 



**Returns:**

true if buffer matches Block Process Call framing, false otherwise. 





        

<hr>



### function smbus\_parse\_full\_block 

_Match full-wire SMBus block framing before raw count+payload fallback._ 
```C++
static bool smbus_parse_full_block (
    const SYN_SMBUS_Packet * pkt,
    const uint8_t * rx_buf,
    size_t data_len,
    size_t * payload_off,
    uint8_t * payload_len
) 
```





**Parameters:**


* `pkt` SMBus packet context providing protocol and expected slave address. 
* `rx_buf` Received wire buffer. 
* `data_len` Buffer length excluding trailing PEC byte. 
* `payload_off` Output byte offset of block payload bytes. 
* `payload_len` Output block byte count. 



**Returns:**

true if rx\_buf matches full-wire block framing for pkt-&gt;proto, false otherwise. 





        

<hr>



### function smbus\_parse\_rx\_payload 

_Dispatch SMBus payload framing parser by transaction protocol family._ 
```C++
static SYN_Status smbus_parse_rx_payload (
    const SYN_SMBUS_Packet * pkt,
    const uint8_t * rx_buf,
    size_t data_len,
    bool has_pec,
    size_t * payload_off,
    uint8_t * payload_len,
    bool * is_full
) 
```





**Parameters:**


* `pkt` SMBus packet context providing protocol and expected slave address. 
* `rx_buf` Received wire or response buffer. 
* `data_len` Buffer length excluding trailing PEC byte. 
* `has_pec` Whether PEC byte is present in rx\_buf. 
* `payload_off` Output byte offset of payload within rx\_buf. 
* `payload_len` Output payload byte count. 
* `is_full` Set true if rx\_buf contains full wire framing, false if payload-only. 



**Returns:**

SYN\_OK on valid framing, SYN\_INVALID\_PARAM otherwise. 





        

<hr>



### function smbus\_parse\_u16 

_Locate 2-byte SMBus word payload in either raw-response or full-wire buffer layout._ 
```C++
static SYN_Status smbus_parse_u16 (
    SYN_SMBUS_Protocol proto,
    size_t data_len,
    bool has_pec,
    size_t * payload_off,
    uint8_t * payload_len,
    bool * is_full
) 
```





**Parameters:**


* `proto` SMBus transaction protocol type. 
* `data_len` Buffer length excluding trailing PEC byte. 
* `has_pec` Whether PEC byte is present in rx\_buf. 
* `payload_off` Output byte offset of 2-byte word payload. 
* `payload_len` Output payload byte length (2). 
* `is_full` Set true if rx\_buf contains full wire framing, false if payload-only. 



**Returns:**

SYN\_OK on valid framing, SYN\_INVALID\_PARAM otherwise. 





        

<hr>



### function smbus\_parse\_u8 

_Locate 1-byte SMBus payload in either raw-response or full-wire buffer layout._ 
```C++
static SYN_Status smbus_parse_u8 (
    SYN_SMBUS_Protocol proto,
    size_t data_len,
    bool has_pec,
    size_t * payload_off,
    uint8_t * payload_len,
    bool * is_full
) 
```





**Parameters:**


* `proto` SMBus transaction protocol type. 
* `data_len` Buffer length excluding trailing PEC byte. 
* `has_pec` Whether PEC byte is present in rx\_buf. 
* `payload_off` Output byte offset of 1-byte payload. 
* `payload_len` Output payload byte length (1). 
* `is_full` Set true if rx\_buf contains full wire framing, false if payload-only. 



**Returns:**

SYN\_OK on valid framing, SYN\_INVALID\_PARAM otherwise. 





        

<hr>



### function smbus\_verify\_rx\_pec 

_Verify trailing SMBus PEC byte on decoded packet when PEC checking is enabled._ 
```C++
static SYN_Status smbus_verify_rx_pec (
    SYN_SMBUS_Packet * pkt,
    const uint8_t * rx_buf,
    size_t rx_len,
    size_t data_len,
    bool has_pec,
    bool is_full
) 
```





**Parameters:**


* `pkt` Decoded SMBus packet structure (pec and pec\_valid updated). 
* `rx_buf` Received buffer including trailing PEC byte when enabled. 
* `rx_len` Total length of rx\_buf in bytes. 
* `data_len` Length of rx\_buf excluding PEC byte. 
* `has_pec` Whether PEC verification is enabled. 
* `is_full` Whether rx\_buf includes full wire address/command framing. 



**Returns:**

SYN\_OK if PEC is disabled or valid, SYN\_ERROR on PEC mismatch. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_smbus.c`

