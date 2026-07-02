

# File syn\_sd.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_sd.h**](syn__sd_8h.md)

[Go to the source code of this file](syn__sd_8h_source.md)

_SD card SPI block driver._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_spi.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SD**](structSYN__SD.md) <br>_SD card driver handle (caller-owned, zero heap allocation)._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_SD\_Type**](#enum-syn_sd_type)  <br>_SD card type, detected automatically during_ [_**syn\_sd\_init()**_](syn__sd_8h.md#function-syn_sd_init) _._ |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sd\_init**](#function-syn_sd_init) ([**SYN\_SD**](structSYN__SD.md) \* sd, uint8\_t spi\_bus, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) cs) <br>_Initialize the SD card over SPI._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sd\_read**](#function-syn_sd_read) (const [**SYN\_SD**](structSYN__SD.md) \* sd, uint32\_t sector, uint8\_t \* buf) <br>_Read one 512-byte sector from the SD card (CMD17)._  |
|  uint32\_t | [**syn\_sd\_sectors**](#function-syn_sd_sectors) (const [**SYN\_SD**](structSYN__SD.md) \* sd) <br>_Return total sector count parsed from the CSD register._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sd\_sync**](#function-syn_sd_sync) (const [**SYN\_SD**](structSYN__SD.md) \* sd) <br>_Flush the write pipeline — wait until card is idle (CMD13)._  |
|  [**SYN\_SD\_Type**](syn__sd_8h.md#enum-syn_sd_type) | [**syn\_sd\_type**](#function-syn_sd_type) (const [**SYN\_SD**](structSYN__SD.md) \* sd) <br>_Return the detected card type._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sd\_write**](#function-syn_sd_write) (const [**SYN\_SD**](structSYN__SD.md) \* sd, uint32\_t sector, const uint8\_t \* buf) <br>_Write one 512-byte sector to the SD card (CMD24)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SD\_SECTOR\_SIZE**](syn__sd_8h.md#define-syn_sd_sector_size)  `512u`<br> |

## Detailed Description


Zero-allocation driver for SD/SDHC/SDXC cards over hardware SPI. Implements the SD simplified SPI spec: raw 512-byte sector read/write. Uses [**syn\_port\_spi.h**](syn__port__spi_8h.md) for all bus transfers; CS is managed by the driver.



```C++
static SYN_SD sd;
if (syn_sd_init(&sd, 0, MY_SD_CS_PIN) == SYN_OK) {
    uint8_t buf[512];
    syn_sd_read(&sd, 0, buf);
    syn_sd_write(&sd, 1, buf);
    syn_sd_sync(&sd);
}
```



The SPI bus must support Mode 0 (CPOL=0, CPHA=0). When tx\_buf is NULL, [**syn\_port\_spi\_transfer()**](syn__port__spi_8h.md#function-syn_port_spi_transfer) must drive MOSI HIGH (0xFF), as required by the SD simplified spec during receive phases. 


    
## Public Types Documentation




### enum SYN\_SD\_Type 

_SD card type, detected automatically during_ [_**syn\_sd\_init()**_](syn__sd_8h.md#function-syn_sd_init) _._
```C++
enum SYN_SD_Type {
    SYN_SD_UNKNOWN = 0,
    SYN_SD_SDSC = 1,
    SYN_SD_SDHC = 2
};
```




<hr>
## Public Functions Documentation




### function syn\_sd\_init 

_Initialize the SD card over SPI._ 
```C++
SYN_Status syn_sd_init (
    SYN_SD * sd,
    uint8_t spi_bus,
    SYN_GPIO_Pin cs
) 
```



Performs power-up clocking, CMD0/CMD8/ACMD41 init, type detection via CMD58, and CSD parsing for capacity.




**Parameters:**


* `sd` Pointer to a caller-owned [**SYN\_SD**](structSYN__SD.md) struct. 
* `spi_bus` SPI bus index passed to syn\_port\_spi\_\*. 
* `cs` Chip-select GPIO pin (active-low). 



**Returns:**

SYN\_OK on success, SYN\_ERROR if no card or init failed. 





        

<hr>



### function syn\_sd\_read 

_Read one 512-byte sector from the SD card (CMD17)._ 
```C++
SYN_Status syn_sd_read (
    const SYN_SD * sd,
    uint32_t sector,
    uint8_t * buf
) 
```





**Parameters:**


* `sd` Initialized SD handle. 
* `sector` Zero-based sector index. 
* `buf` Output buffer — caller must provide at least 512 bytes. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on timeout or card error. 





        

<hr>



### function syn\_sd\_sectors 

_Return total sector count parsed from the CSD register._ 
```C++
uint32_t syn_sd_sectors (
    const SYN_SD * sd
) 
```





**Parameters:**


* `sd` Initialized SD handle. 



**Returns:**

Number of 512-byte sectors on the card. 





        

<hr>



### function syn\_sd\_sync 

_Flush the write pipeline — wait until card is idle (CMD13)._ 
```C++
SYN_Status syn_sd_sync (
    const SYN_SD * sd
) 
```





**Parameters:**


* `sd` Initialized SD handle. 



**Returns:**

SYN\_OK if card is idle and error-free, SYN\_ERROR otherwise. 





        

<hr>



### function syn\_sd\_type 

_Return the detected card type._ 
```C++
SYN_SD_Type syn_sd_type (
    const SYN_SD * sd
) 
```





**Parameters:**


* `sd` Initialized SD handle. 



**Returns:**

SYN\_SD\_SDSC or SYN\_SD\_SDHC. 





        

<hr>



### function syn\_sd\_write 

_Write one 512-byte sector to the SD card (CMD24)._ 
```C++
SYN_Status syn_sd_write (
    const SYN_SD * sd,
    uint32_t sector,
    const uint8_t * buf
) 
```





**Parameters:**


* `sd` Initialized SD handle. 
* `sector` Zero-based sector index. 
* `buf` Data to write — must be exactly 512 bytes. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on card rejection or timeout. 





        

<hr>
## Macro Definition Documentation





### define SYN\_SD\_SECTOR\_SIZE 

```C++
#define SYN_SD_SECTOR_SIZE `512u`
```



Fixed sector size in bytes. All SD cards expose 512-byte sectors in SPI mode. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_sd.h`

