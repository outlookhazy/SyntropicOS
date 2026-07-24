

# File syn\_fwupdate.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_fwupdate.c**](syn__fwupdate_8c.md)

[Go to the source code of this file](syn__fwupdate_8c_source.md)

_Streaming firmware updater implementation._ 

* `#include "syn_fwupdate.h"`
* `#include "syn_fwimage.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_crc.h"`
* `#include "../port/syn_port_flash.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_fwupdate\_abort**](#function-syn_fwupdate_abort) ([**SYN\_FwUpdate**](structSYN__FwUpdate.md) \* upd) <br>_Abort the update._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fwupdate\_begin**](#function-syn_fwupdate_begin) ([**SYN\_FwUpdate**](structSYN__FwUpdate.md) \* upd, uint32\_t slot\_addr, uint32\_t max\_size, uint8\_t \* page\_buf, uint16\_t page\_buf\_size) <br>_Begin a firmware update._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fwupdate\_finish**](#function-syn_fwupdate_finish) ([**SYN\_FwUpdate**](structSYN__FwUpdate.md) \* upd, uint32\_t expected\_crc, uint32\_t version\_code) <br>_Finalize the update._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fwupdate\_write**](#function-syn_fwupdate_write) ([**SYN\_FwUpdate**](structSYN__FwUpdate.md) \* upd, const uint8\_t \* data, size\_t len) <br>_Write a chunk of firmware data._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**flush\_page**](#function-flush_page) ([**SYN\_FwUpdate**](structSYN__FwUpdate.md) \* upd) <br>_Flush the page buffer to flash._  |


























## Public Functions Documentation




### function syn\_fwupdate\_abort 

_Abort the update._ 
```C++
void syn_fwupdate_abort (
    SYN_FwUpdate * upd
) 
```



Marks the slot as INVALID and cleans up.




**Parameters:**


* `upd` Updater instance. 




        

<hr>



### function syn\_fwupdate\_begin 

_Begin a firmware update._ 
```C++
SYN_Status syn_fwupdate_begin (
    SYN_FwUpdate * upd,
    uint32_t slot_addr,
    uint32_t max_size,
    uint8_t * page_buf,
    uint16_t page_buf_size
) 
```



Erases the first sector of the target slot and prepares for writing. The image header is written at slot\_addr; image data starts at slot\_addr + sizeof(SYN\_FwImageHeader).




**Parameters:**


* `upd` Updater instance. 
* `slot_addr` Flash base address of the target slot. 
* `max_size` Maximum image size in bytes (excl. header). 
* `page_buf` Caller-provided page buffer. 
* `page_buf_size` Buffer size (should match flash write granularity). 



**Returns:**

SYN\_OK on success, SYN\_ERROR on flash erase failure. 





        

<hr>



### function syn\_fwupdate\_finish 

_Finalize the update._ 
```C++
SYN_Status syn_fwupdate_finish (
    SYN_FwUpdate * upd,
    uint32_t expected_crc,
    uint32_t version_code
) 
```



Flushes any remaining buffered data, verifies the CRC-32 matches the expected value, and writes the image header with state = NEW.




**Parameters:**


* `upd` Updater instance. 
* `expected_crc` Expected CRC-32 of the full image. 
* `version_code` Version code for the new image. 



**Returns:**

SYN\_OK if verification passes and header written, SYN\_ERROR on mismatch or flash failure. 





        

<hr>



### function syn\_fwupdate\_write 

_Write a chunk of firmware data._ 
```C++
SYN_Status syn_fwupdate_write (
    SYN_FwUpdate * upd,
    const uint8_t * data,
    size_t len
) 
```



Data is buffered until a full page is ready, then flushed to flash. Automatically erases sectors as needed.




**Parameters:**


* `upd` Updater instance. 
* `data` Firmware data chunk. 
* `len` Chunk length. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on flash write failure or overflow. 





        

<hr>
## Public Static Functions Documentation




### function flush\_page 

_Flush the page buffer to flash._ 
```C++
static SYN_Status flush_page (
    SYN_FwUpdate * upd
) 
```





**Parameters:**


* `upd` Firmware update context. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_fwupdate.c`

