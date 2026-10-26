

# File syn\_smbus.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_smbus.h**](syn__smbus_8h.md)

[Go to the source code of this file](syn__smbus_8h_source.md)

_SMBus (System Management Bus 1.1 / 2.0 / 3.0) Protocol Engine._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) <br>_SMBus transaction packet structure._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_SMBUS\_Protocol**](#enum-syn_smbus_protocol)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_smbus\_calc\_pec**](#function-syn_smbus_calc_pec) (uint8\_t init\_crc, const uint8\_t \* data, size\_t len) <br>_Calculate SMBus Packet Error Checking (PEC) CRC-8 byte. Polynomial: x^8 + x^2 + x + 1 (0x07), Initial value: 0x00._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_smbus\_decode\_packet**](#function-syn_smbus_decode_packet) ([**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, const uint8\_t \* rx\_buf, size\_t rx\_len, [**SYN\_SMBUS\_Protocol**](syn__smbus_8h.md#enum-syn_smbus_protocol) proto, [**bool**](syn__defs_8h.md#enum-bool) has\_pec) <br>_Decode an SMBus byte stream into an SMBus packet structure._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_smbus\_encode\_packet**](#function-syn_smbus_encode_packet) (const [**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, uint8\_t \* tx\_buf, size\_t buf\_size, size\_t \* out\_len) <br>_Encode an SMBus packet into a byte stream for transmission._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SMBUS\_ADDR\_ALERT\_RESPONSE**](syn__smbus_8h.md#define-syn_smbus_addr_alert_response)  `0x0C`<br> |
| define  | [**SYN\_SMBUS\_ADDR\_DEFAULT**](syn__smbus_8h.md#define-syn_smbus_addr_default)  `0x61`<br> |
| define  | [**SYN\_SMBUS\_ADDR\_SMART\_BATTERY**](syn__smbus_8h.md#define-syn_smbus_addr_smart_battery)  `0x16`<br> |
| define  | [**SYN\_SMBUS\_ADDR\_SMART\_BATTERY\_CHARGER**](syn__smbus_8h.md#define-syn_smbus_addr_smart_battery_charger)  `0x12`<br> |
| define  | [**SYN\_SMBUS\_ADDR\_SMART\_BATTERY\_SELECT**](syn__smbus_8h.md#define-syn_smbus_addr_smart_battery_select)  `0x13`<br> |
| define  | [**SYN\_SMBUS\_BLOCK\_MAX\_LEN**](syn__smbus_8h.md#define-syn_smbus_block_max_len)  `32`<br> |
| define  | [**SYN\_SMBUS\_BUF\_MAX\_LEN**](syn__smbus_8h.md#define-syn_smbus_buf_max_len)  `260`<br> |

## Detailed Description


Provides non-blocking SMBus packet encoding, decoding, Packet Error Checking (PEC) CRC-8 calculations, transaction protocol builders, and ARP/alert definitions. 


    
## Public Types Documentation




### enum SYN\_SMBUS\_Protocol 

```C++
enum SYN_SMBUS_Protocol {
    SYN_SMBUS_PROTO_QUICK_READ = 0,
    SYN_SMBUS_PROTO_QUICK_WRITE,
    SYN_SMBUS_PROTO_SEND_BYTE,
    SYN_SMBUS_PROTO_RECEIVE_BYTE,
    SYN_SMBUS_PROTO_WRITE_BYTE,
    SYN_SMBUS_PROTO_READ_BYTE,
    SYN_SMBUS_PROTO_WRITE_WORD,
    SYN_SMBUS_PROTO_READ_WORD,
    SYN_SMBUS_PROTO_WRITE_BLOCK,
    SYN_SMBUS_PROTO_READ_BLOCK,
    SYN_SMBUS_PROTO_PROCESS_CALL,
    SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL
};
```



SMBus Transaction Protocol Types Enum. 


        

<hr>
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
## Macro Definition Documentation





### define SYN\_SMBUS\_ADDR\_ALERT\_RESPONSE 

```C++
#define SYN_SMBUS_ADDR_ALERT_RESPONSE `0x0C`
```



Alert Response Address (ARA) 


        

<hr>



### define SYN\_SMBUS\_ADDR\_DEFAULT 

```C++
#define SYN_SMBUS_ADDR_DEFAULT `0x61`
```



SMBus Default Device Address (ARP) 


        

<hr>



### define SYN\_SMBUS\_ADDR\_SMART\_BATTERY 

```C++
#define SYN_SMBUS_ADDR_SMART_BATTERY `0x16`
```



Smart Battery Data 


        

<hr>



### define SYN\_SMBUS\_ADDR\_SMART\_BATTERY\_CHARGER 

```C++
#define SYN_SMBUS_ADDR_SMART_BATTERY_CHARGER `0x12`
```



Smart Battery Charger 


        

<hr>



### define SYN\_SMBUS\_ADDR\_SMART\_BATTERY\_SELECT 

```C++
#define SYN_SMBUS_ADDR_SMART_BATTERY_SELECT `0x13`
```



Smart Battery Selector 


        

<hr>



### define SYN\_SMBUS\_BLOCK\_MAX\_LEN 

```C++
#define SYN_SMBUS_BLOCK_MAX_LEN `32`
```



SMBus 2.0 default block payload max length 


        

<hr>



### define SYN\_SMBUS\_BUF\_MAX\_LEN 

```C++
#define SYN_SMBUS_BUF_MAX_LEN `260`
```



Buffer payload max length for extended frames 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_smbus.h`

