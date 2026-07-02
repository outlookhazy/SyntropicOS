

# File syn\_sd.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_sd.c**](syn__sd_8c.md)

[Go to the source code of this file](syn__sd_8c_source.md)

_SD card SPI block driver implementation._ [More...](#detailed-description)

* `#include "syn_sd.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sd\_init**](#function-syn_sd_init) ([**SYN\_SD**](structSYN__SD.md) \* sd, uint8\_t spi\_bus, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) cs) <br>_Initialize the SD card over SPI._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sd\_read**](#function-syn_sd_read) (const [**SYN\_SD**](structSYN__SD.md) \* sd, uint32\_t sector, uint8\_t \* buf) <br>_Read one 512-byte sector from the SD card (CMD17)._  |
|  uint32\_t | [**syn\_sd\_sectors**](#function-syn_sd_sectors) (const [**SYN\_SD**](structSYN__SD.md) \* sd) <br>_Return total sector count parsed from the CSD register._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sd\_sync**](#function-syn_sd_sync) (const [**SYN\_SD**](structSYN__SD.md) \* sd) <br>_Flush the write pipeline — wait until card is idle (CMD13)._  |
|  [**SYN\_SD\_Type**](syn__sd_8h.md#enum-syn_sd_type) | [**syn\_sd\_type**](#function-syn_sd_type) (const [**SYN\_SD**](structSYN__SD.md) \* sd) <br>_Return the detected card type._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sd\_write**](#function-syn_sd_write) (const [**SYN\_SD**](structSYN__SD.md) \* sd, uint32\_t sector, const uint8\_t \* buf) <br>_Write one 512-byte sector to the SD card (CMD24)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**sd\_cmd**](#function-sd_cmd) (const [**SYN\_SD**](structSYN__SD.md) \* sd, uint8\_t cmd, uint32\_t arg, uint8\_t crc) <br>_Send a 6-byte SD command and poll for R1 response._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**sd\_read\_csd**](#function-sd_read_csd) ([**SYN\_SD**](structSYN__SD.md) \* sd) <br>_Read the CSD register and parse sector count._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**sd\_wait\_ready**](#function-sd_wait_ready) (const [**SYN\_SD**](structSYN__SD.md) \* sd) <br>_Wait until the SD card is no longer busy (MISO = 0xFF)._  |
|  uint8\_t | [**sd\_xfer**](#function-sd_xfer) (const [**SYN\_SD**](structSYN__SD.md) \* sd, uint8\_t out) <br>_Transfer one byte over SPI and return the received byte._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SD\_ACMD41**](syn__sd_8c.md#define-sd_acmd41)  `41u`<br> |
| define  | [**SD\_ACMD41\_RETRIES**](syn__sd_8c.md#define-sd_acmd41_retries)  `1000u`<br> |
| define  | [**SD\_BUSY\_RETRIES**](syn__sd_8c.md#define-sd_busy_retries)  `2000u`<br> |
| define  | [**SD\_CMD0**](syn__sd_8c.md#define-sd_cmd0)  `0u`<br> |
| define  | [**SD\_CMD13**](syn__sd_8c.md#define-sd_cmd13)  `13u`<br> |
| define  | [**SD\_CMD16**](syn__sd_8c.md#define-sd_cmd16)  `16u`<br> |
| define  | [**SD\_CMD17**](syn__sd_8c.md#define-sd_cmd17)  `17u`<br> |
| define  | [**SD\_CMD24**](syn__sd_8c.md#define-sd_cmd24)  `24u`<br> |
| define  | [**SD\_CMD55**](syn__sd_8c.md#define-sd_cmd55)  `55u`<br> |
| define  | [**SD\_CMD58**](syn__sd_8c.md#define-sd_cmd58)  `58u`<br> |
| define  | [**SD\_CMD8**](syn__sd_8c.md#define-sd_cmd8)  `8u`<br> |
| define  | [**SD\_CMD9**](syn__sd_8c.md#define-sd_cmd9)  `9u`<br> |
| define  | [**SD\_R1\_ERR\_MSK**](syn__sd_8c.md#define-sd_r1_err_msk)  `0xFEu`<br> |
| define  | [**SD\_R1\_IDLE**](syn__sd_8c.md#define-sd_r1_idle)  `0x01u`<br> |
| define  | [**SD\_R1\_ILLCMD**](syn__sd_8c.md#define-sd_r1_illcmd)  `0x04u`<br> |
| define  | [**SD\_R1\_POLL\_RETRIES**](syn__sd_8c.md#define-sd_r1_poll_retries)  `8u`<br> |
| define  | [**SD\_R1\_READY**](syn__sd_8c.md#define-sd_r1_ready)  `0x00u`<br> |
| define  | [**SD\_R1\_TIMEOUT**](syn__sd_8c.md#define-sd_r1_timeout)  `0xFFu`<br> |
| define  | [**SD\_TOKEN\_ACCEPTED**](syn__sd_8c.md#define-sd_token_accepted)  `0x05u`<br> |
| define  | [**SD\_TOKEN\_RETRIES**](syn__sd_8c.md#define-sd_token_retries)  `2000u`<br> |
| define  | [**SD\_TOKEN\_START**](syn__sd_8c.md#define-sd_token_start)  `0xFEu`<br> |

## Detailed Description


Implements the SD simplified SPI spec:
* CMD0 (GO\_IDLE\_STATE) reset to SPI mode
* CMD8 (SEND\_IF\_COND) SDHC-capable detection
* CMD9 (SEND\_CSD) capacity via CSD v1/v2 parse
* CMD13 (SEND\_STATUS) sync / flush
* CMD16 (SET\_BLOCKLEN) SDSC: force 512-byte blocks
* CMD17 (READ\_SINGLE\_BLOCK)
* CMD24 (WRITE\_BLOCK)
* CMD55 (APP\_CMD) ACMD prefix
* CMD58 (READ\_OCR) card type confirmation
* ACMD41 (SD\_SEND\_OP\_COND) card initialization




CRC: CMD0/CMD8 use hardcoded CRC7 constants. All other commands and data blocks use 0xFF dummy bytes (CRC mode off by default in SPI). 


    
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
## Public Static Functions Documentation




### function sd\_cmd 

_Send a 6-byte SD command and poll for R1 response._ 
```C++
static uint8_t sd_cmd (
    const SYN_SD * sd,
    uint8_t cmd,
    uint32_t arg,
    uint8_t crc
) 
```





**Parameters:**


* `sd` SD card instance. 
* `cmd` Command index (e.g. CMD0). 
* `arg` 32-bit argument. 
* `crc` CRC byte. 



**Returns:**

R1 response byte. 





        

<hr>



### function sd\_read\_csd 

_Read the CSD register and parse sector count._ 
```C++
static SYN_Status sd_read_csd (
    SYN_SD * sd
) 
```





**Parameters:**


* `sd` SD card instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function sd\_wait\_ready 

_Wait until the SD card is no longer busy (MISO = 0xFF)._ 
```C++
static bool sd_wait_ready (
    const SYN_SD * sd
) 
```





**Parameters:**


* `sd` SD card instance. 



**Returns:**

true if card became ready within timeout. 





        

<hr>



### function sd\_xfer 

_Transfer one byte over SPI and return the received byte._ 
```C++
static uint8_t sd_xfer (
    const SYN_SD * sd,
    uint8_t out
) 
```





**Parameters:**


* `sd` SD card instance. 
* `out` Byte to send. 



**Returns:**

Received byte. 





        

<hr>
## Macro Definition Documentation





### define SD\_ACMD41 

```C++
#define SD_ACMD41 `41u`
```



SD\_SEND\_OP\_COND (app) 
 


        

<hr>



### define SD\_ACMD41\_RETRIES 

```C++
#define SD_ACMD41_RETRIES `1000u`
```



Max ACMD41 init retries 
 


        

<hr>



### define SD\_BUSY\_RETRIES 

```C++
#define SD_BUSY_RETRIES `2000u`
```



Max busy wait retries 
 


        

<hr>



### define SD\_CMD0 

```C++
#define SD_CMD0 `0u`
```



GO\_IDLE\_STATE 
 


        

<hr>



### define SD\_CMD13 

```C++
#define SD_CMD13 `13u`
```



SEND\_STATUS 
 


        

<hr>



### define SD\_CMD16 

```C++
#define SD_CMD16 `16u`
```



SET\_BLOCKLEN 
 


        

<hr>



### define SD\_CMD17 

```C++
#define SD_CMD17 `17u`
```



READ\_SINGLE\_BLOCK 
 


        

<hr>



### define SD\_CMD24 

```C++
#define SD_CMD24 `24u`
```



WRITE\_BLOCK 
 


        

<hr>



### define SD\_CMD55 

```C++
#define SD_CMD55 `55u`
```



APP\_CMD prefix 
 


        

<hr>



### define SD\_CMD58 

```C++
#define SD_CMD58 `58u`
```



READ\_OCR 
 


        

<hr>



### define SD\_CMD8 

```C++
#define SD_CMD8 `8u`
```



SEND\_IF\_COND 
 


        

<hr>



### define SD\_CMD9 

```C++
#define SD_CMD9 `9u`
```



SEND\_CSD 
 


        

<hr>



### define SD\_R1\_ERR\_MSK 

```C++
#define SD_R1_ERR_MSK `0xFEu`
```



Any bit other than IDLE = error 
 


        

<hr>



### define SD\_R1\_IDLE 

```C++
#define SD_R1_IDLE `0x01u`
```



Card in idle state during init 
 


        

<hr>



### define SD\_R1\_ILLCMD 

```C++
#define SD_R1_ILLCMD `0x04u`
```



Illegal command (SDSC: no CMD8 support) 


        

<hr>



### define SD\_R1\_POLL\_RETRIES 

```C++
#define SD_R1_POLL_RETRIES `8u`
```



Max R1 polling attempts 
 


        

<hr>



### define SD\_R1\_READY 

```C++
#define SD_R1_READY `0x00u`
```



No errors, card ready 
 


        

<hr>



### define SD\_R1\_TIMEOUT 

```C++
#define SD_R1_TIMEOUT `0xFFu`
```



No response from card 
 


        

<hr>



### define SD\_TOKEN\_ACCEPTED 

```C++
#define SD_TOKEN_ACCEPTED `0x05u`
```



Write data response: (xxx0\_0101) 
 


        

<hr>



### define SD\_TOKEN\_RETRIES 

```C++
#define SD_TOKEN_RETRIES `2000u`
```



Max token wait retries 
 


        

<hr>



### define SD\_TOKEN\_START 

```C++
#define SD_TOKEN_START `0xFEu`
```



Start block for single read/write 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_sd.c`

