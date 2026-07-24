

# File syn\_crc.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_crc.c**](syn__crc_8c.md)

[Go to the source code of this file](syn__crc_8c_source.md)

_CRC calculation implementations._ 

* `#include "syn_crc.h"`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const uint16\_t | [**crc16\_ccitt\_table**](#variable-crc16_ccitt_table)  <br>_CRC-16 CCITT lookup table._  |
|  const uint16\_t | [**crc16\_modbus\_table**](#variable-crc16_modbus_table)  <br>_CRC-16 Modbus lookup table._  |
|  const uint32\_t | [**crc32\_table**](#variable-crc32_table)  <br>_CRC-32 lookup table._  |
|  const uint8\_t | [**crc8\_table**](#variable-crc8_table)   = `/* multi line expression */`<br>_CRC-8 lookup table._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_crc16\_ccitt\_update**](#function-syn_crc16_ccitt_update) (uint16\_t crc, const void \* data, size\_t len) <br>_Update CRC-16 CCITT with a block of data._  |
|  uint16\_t | [**syn\_crc16\_modbus\_update**](#function-syn_crc16_modbus_update) (uint16\_t crc, const void \* data, size\_t len) <br>_Update CRC-16 Modbus with a block of data._  |
|  uint32\_t | [**syn\_crc32\_update**](#function-syn_crc32_update) (uint32\_t crc, const void \* data, size\_t len) <br>_Update CRC-32 with a block of data._  |
|  uint8\_t | [**syn\_crc8\_update**](#function-syn_crc8_update) (uint8\_t crc, const void \* data, size\_t len) <br>_Update CRC-8 with a block of data._  |




























## Public Static Attributes Documentation




### variable crc16\_ccitt\_table 

_CRC-16 CCITT lookup table._ 
```C++
const uint16_t crc16_ccitt_table[256];
```




<hr>



### variable crc16\_modbus\_table 

_CRC-16 Modbus lookup table._ 
```C++
const uint16_t crc16_modbus_table[256];
```




<hr>



### variable crc32\_table 

_CRC-32 lookup table._ 
```C++
const uint32_t crc32_table[256];
```




<hr>



### variable crc8\_table 

_CRC-8 lookup table._ 
```C++
const uint8_t crc8_table[256];
```




<hr>
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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_crc.c`

