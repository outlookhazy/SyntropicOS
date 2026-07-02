

# File syn\_crc.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_crc.h**](syn__crc_8h.md)

[Go to the source code of this file](syn__crc_8h_source.md)

_CRC calculation for communication protocols._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_crc16\_ccitt\_update**](#function-syn_crc16_ccitt_update) (uint16\_t crc, const void \* data, size\_t len) <br>_Update CRC-16 CCITT with a block of data._  |
|  uint16\_t | [**syn\_crc16\_modbus\_update**](#function-syn_crc16_modbus_update) (uint16\_t crc, const void \* data, size\_t len) <br>_Update CRC-16 Modbus with a block of data._  |
|  uint32\_t | [**syn\_crc32\_update**](#function-syn_crc32_update) (uint32\_t crc, const void \* data, size\_t len) <br>_Update CRC-32 with a block of data._  |
|  uint8\_t | [**syn\_crc8\_update**](#function-syn_crc8_update) (uint8\_t crc, const void \* data, size\_t len) <br>_Update CRC-8 with a block of data._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_crc16\_ccitt**](#function-syn_crc16_ccitt) (const void \* data, size\_t len) <br>_Compute CRC-16 CCITT of a complete buffer._  |
|  uint16\_t | [**syn\_crc16\_modbus**](#function-syn_crc16_modbus) (const void \* data, size\_t len) <br>_Compute CRC-16 Modbus of a complete buffer._  |
|  uint32\_t | [**syn\_crc32**](#function-syn_crc32) (const void \* data, size\_t len) <br>_Compute CRC-32 of a complete buffer._  |
|  uint32\_t | [**syn\_crc32\_final**](#function-syn_crc32_final) (uint32\_t crc) <br>_Finalize CRC-32 (XOR with 0xFFFFFFFF)._  |
|  uint8\_t | [**syn\_crc8**](#function-syn_crc8) (const void \* data, size\_t len) <br>_Compute CRC-8 of a complete buffer._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_CRC16\_CCITT\_INIT**](syn__crc_8h.md#define-syn_crc16_ccitt_init)  `0xFFFFu`<br>_CRC-16 CCITT initial value._  |
| define  | [**SYN\_CRC16\_MODBUS\_INIT**](syn__crc_8h.md#define-syn_crc16_modbus_init)  `0xFFFFu`<br>_CRC-16 Modbus initial value._  |
| define  | [**SYN\_CRC32\_INIT**](syn__crc_8h.md#define-syn_crc32_init)  `0xFFFFFFFFu`<br>_CRC-32 initial value._  |
| define  | [**SYN\_CRC8\_INIT**](syn__crc_8h.md#define-syn_crc8_init)  `0x00u`<br>_CRC-8 initial value._  |
| define  | [**SYN\_CRC\_USE\_TABLE**](syn__crc_8h.md#define-syn_crc_use_table)  `1`<br> |

## Detailed Description


Provides CRC-8, CRC-16 (CCITT, Modbus), and CRC-32 (Ethernet). Configure SYN\_CRC\_USE\_TABLE to trade ROM for speed: 1 = 256-entry lookup table (fast, ~256–1024 bytes ROM per variant) 0 = bit-by-bit computation (small, slower)


All variants support incremental (streaming) computation.




**
**


```C++
uint16_t crc = syn_crc16_ccitt(data, len);

// Incremental:
uint16_t crc = SYN_CRC16_CCITT_INIT;
crc = syn_crc16_ccitt_update(crc, chunk1, len1);
crc = syn_crc16_ccitt_update(crc, chunk2, len2);
```
 





    
## Public Functions Documentation




### function syn\_crc16\_ccitt\_update 

_Update CRC-16 CCITT with a block of data._ 
```C++
uint16_t syn_crc16_ccitt_update (
    uint16_t crc,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `crc` Running CRC value (start with SYN\_CRC16\_CCITT\_INIT). 
* `data` Data buffer. 
* `len` Length in bytes. 



**Returns:**

Updated CRC-16. 





        

<hr>



### function syn\_crc16\_modbus\_update 

_Update CRC-16 Modbus with a block of data._ 
```C++
uint16_t syn_crc16_modbus_update (
    uint16_t crc,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `crc` Running CRC value (start with SYN\_CRC16\_MODBUS\_INIT). 
* `data` Data buffer. 
* `len` Length in bytes. 



**Returns:**

Updated CRC-16. 





        

<hr>



### function syn\_crc32\_update 

_Update CRC-32 with a block of data._ 
```C++
uint32_t syn_crc32_update (
    uint32_t crc,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `crc` Running CRC value (start with SYN\_CRC32\_INIT). 
* `data` Data buffer. 
* `len` Length in bytes. 



**Returns:**

Updated CRC-32 (call syn\_crc32\_final to finalize). 





        

<hr>



### function syn\_crc8\_update 

_Update CRC-8 with a block of data._ 
```C++
uint8_t syn_crc8_update (
    uint8_t crc,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `crc` Running CRC value (start with SYN\_CRC8\_INIT). 
* `data` Data buffer. 
* `len` Length in bytes. 



**Returns:**

Updated CRC-8. 





        

<hr>
## Public Static Functions Documentation




### function syn\_crc16\_ccitt 

_Compute CRC-16 CCITT of a complete buffer._ 
```C++
static inline uint16_t syn_crc16_ccitt (
    const void * data,
    size_t len
) 
```





**Parameters:**


* `data` Data buffer. 
* `len` Length in bytes. 



**Returns:**

CRC-16 CCITT value. 





        

<hr>



### function syn\_crc16\_modbus 

_Compute CRC-16 Modbus of a complete buffer._ 
```C++
static inline uint16_t syn_crc16_modbus (
    const void * data,
    size_t len
) 
```





**Parameters:**


* `data` Data buffer. 
* `len` Length in bytes. 



**Returns:**

CRC-16 Modbus value. 





        

<hr>



### function syn\_crc32 

_Compute CRC-32 of a complete buffer._ 
```C++
static inline uint32_t syn_crc32 (
    const void * data,
    size_t len
) 
```





**Parameters:**


* `data` Data buffer. 
* `len` Length in bytes. 



**Returns:**

CRC-32 value. 





        

<hr>



### function syn\_crc32\_final 

_Finalize CRC-32 (XOR with 0xFFFFFFFF)._ 
```C++
static inline uint32_t syn_crc32_final (
    uint32_t crc
) 
```





**Parameters:**


* `crc` Running CRC-32 value. 



**Returns:**

Final CRC-32. 





        

<hr>



### function syn\_crc8 

_Compute CRC-8 of a complete buffer._ 
```C++
static inline uint8_t syn_crc8 (
    const void * data,
    size_t len
) 
```





**Parameters:**


* `data` Data buffer. 
* `len` Length in bytes. 



**Returns:**

CRC-8 value. 





        

<hr>
## Macro Definition Documentation





### define SYN\_CRC16\_CCITT\_INIT 

_CRC-16 CCITT initial value._ 
```C++
#define SYN_CRC16_CCITT_INIT `0xFFFFu`
```




<hr>



### define SYN\_CRC16\_MODBUS\_INIT 

_CRC-16 Modbus initial value._ 
```C++
#define SYN_CRC16_MODBUS_INIT `0xFFFFu`
```




<hr>



### define SYN\_CRC32\_INIT 

_CRC-32 initial value._ 
```C++
#define SYN_CRC32_INIT `0xFFFFFFFFu`
```




<hr>



### define SYN\_CRC8\_INIT 

_CRC-8 initial value._ 
```C++
#define SYN_CRC8_INIT `0x00u`
```




<hr>



### define SYN\_CRC\_USE\_TABLE 

```C++
#define SYN_CRC_USE_TABLE `1`
```



Set to 1 for lookup-table CRC (fast), 0 for bit-by-bit (small). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_crc.h`

