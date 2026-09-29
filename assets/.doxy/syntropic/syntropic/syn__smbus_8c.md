

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
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_smbus\_decode\_packet**](#function-syn_smbus_decode_packet) ([**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, const uint8\_t \* rx\_buf, size\_t rx\_len, [**SYN\_SMBUS\_Protocol**](syn__smbus_8h.md#enum-syn_smbus_protocol) proto, [**bool**](syn__defs_8h.md#enum-bool) has\_pec) <br>_Decode an SMBus byte stream into an SMBus packet structure._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_smbus\_encode\_packet**](#function-syn_smbus_encode_packet) (const [**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, uint8\_t \* tx\_buf, size\_t buf\_size, size\_t \* out\_len) <br>_Encode an SMBus packet into a byte stream for transmission._  |




























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

SYN\_OK on success, SYN\_ERR\_INVALID\_CHECKSUM if PEC mismatch, or SYN\_ERR\_INVALID\_PARAM. 





        

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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_smbus.c`

