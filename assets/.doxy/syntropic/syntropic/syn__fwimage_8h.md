

# File syn\_fwimage.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_fwimage.h**](syn__fwimage_8h.md)

[Go to the source code of this file](syn__fwimage_8h_source.md)

_Firmware image header — standardized format for OTA slots._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_flash.h"`
* `#include "../util/syn_crc.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_FwImageHeader**](structSYN__FwImageHeader.md) <br>_Firmware image header — stored at the start of each flash slot._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_FwState**](#enum-syn_fwstate)  <br>_Firmware image lifecycle state._  |






















## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**syn\_fwimage\_data\_offset**](#function-syn_fwimage_data_offset) (uint32\_t slot\_addr) <br>_Compute the byte offset from slot start where firmware payload begins._  |
|  void | [**syn\_fwimage\_encode\_auth\_meta**](#function-syn_fwimage_encode_auth_meta) (uint32\_t version\_code, uint32\_t image\_size, uint8\_t out\_meta) <br>_Encode canonical 12-byte firmware metadata for cryptographic binding._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_fwimage\_header\_valid**](#function-syn_fwimage_header_valid) (const [**SYN\_FwImageHeader**](structSYN__FwImageHeader.md) \* hdr) <br>_Validate a firmware image header._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_fwimage\_is\_bootable**](#function-syn_fwimage_is_bootable) (const [**SYN\_FwImageHeader**](structSYN__FwImageHeader.md) \* hdr) <br>_Check if a slot contains a bootable image._  |
|  void | [**syn\_fwimage\_seal\_header**](#function-syn_fwimage_seal_header) ([**SYN\_FwImageHeader**](structSYN__FwImageHeader.md) \* hdr) <br>_Compute the header CRC and write it into the header._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_FWIMAGE\_AUTH\_META\_SIZE**](syn__fwimage_8h.md#define-syn_fwimage_auth_meta_size)  `12U`<br>_Byte size of canonical authenticated firmware metadata (magic \|\| version \|\| size)._  |
| define  | [**SYN\_FW\_MAGIC**](syn__fwimage_8h.md#define-syn_fw_magic)  `0x53594E46u /\* "SYNF" in little-endian \*/`<br>_Magic number identifying a SyntropicOS firmware image header ("SYNF")._  |

## Detailed Description


Each firmware slot (A or B) has this header at a known flash offset. The bootloader and OTA updater use it to decide which image to boot, whether an update succeeded, and when to rollback.


Layout in flash: 
```C++
[SYN_FwImageHeader (Sector 0)] [firmware binary at hdr_offset (image_size bytes)]
```





**
**


* OTA begins: header.state = WRITING
* OTA complete: header.state = NEW (CRC verified)
* First boot: bootloader sets state = TESTING
* App healthy: app calls confirm → state = CONFIRMED
* App crashes: bootloader sees TESTING + unhealthy → state = INVALID, rollback 







    
## Public Types Documentation




### enum SYN\_FwState 

_Firmware image lifecycle state._ 
```C++
enum SYN_FwState {
    SYN_FW_STATE_EMPTY = 0xFF,
    SYN_FW_STATE_WRITING = 0x01,
    SYN_FW_STATE_NEW = 0x02,
    SYN_FW_STATE_TESTING = 0x03,
    SYN_FW_STATE_CONFIRMED = 0x04,
    SYN_FW_STATE_INVALID = 0x00
};
```




<hr>
## Public Static Functions Documentation




### function syn\_fwimage\_data\_offset 

_Compute the byte offset from slot start where firmware payload begins._ 
```C++
static inline uint32_t syn_fwimage_data_offset (
    uint32_t slot_addr
) 
```



Reserves one full flash erase sector (at least sizeof(SYN\_FwImageHeader)) at the beginning of each slot so header state rewrites never erase payload.




**Parameters:**


* `slot_addr` Flash base address of the slot. 



**Returns:**

Payload byte offset from `slot_addr`. 





        

<hr>



### function syn\_fwimage\_encode\_auth\_meta 

_Encode canonical 12-byte firmware metadata for cryptographic binding._ 
```C++
static inline void syn_fwimage_encode_auth_meta (
    uint32_t version_code,
    uint32_t image_size,
    uint8_t out_meta
) 
```



Encodes little-endian `SYN_FW_MAGIC (4B) || version_code (4B) || image_size (4B)`. Appended to HMAC-SHA256 and Ed25519 message streams and supplied as AES-GCM AAD so header version or payload length tampering cannot bypass verification.




**Parameters:**


* `version_code` Packed firmware version code. 
* `image_size` Firmware binary payload size in bytes (excluding header). 
* `out_meta` Output 12-byte metadata buffer. 




        

<hr>



### function syn\_fwimage\_header\_valid 

_Validate a firmware image header._ 
```C++
static inline bool syn_fwimage_header_valid (
    const SYN_FwImageHeader * hdr
) 
```



Checks magic number and header CRC. Does NOT verify the image data CRC (use syn\_fwimage\_verify\_data for that).




**Parameters:**


* `hdr` Header to validate. 



**Returns:**

true if header is valid. 





        

<hr>



### function syn\_fwimage\_is\_bootable 

_Check if a slot contains a bootable image._ 
```C++
static inline bool syn_fwimage_is_bootable (
    const SYN_FwImageHeader * hdr
) 
```



An image is bootable if the header is valid and state is NEW, TESTING, or CONFIRMED.




**Parameters:**


* `hdr` Header to check. 



**Returns:**

true if bootable. 





        

<hr>



### function syn\_fwimage\_seal\_header 

_Compute the header CRC and write it into the header._ 
```C++
static inline void syn_fwimage_seal_header (
    SYN_FwImageHeader * hdr
) 
```



Call after setting all other fields.




**Parameters:**


* `hdr` Header to seal. 




        

<hr>
## Macro Definition Documentation





### define SYN\_FWIMAGE\_AUTH\_META\_SIZE 

_Byte size of canonical authenticated firmware metadata (magic \|\| version \|\| size)._ 
```C++
#define SYN_FWIMAGE_AUTH_META_SIZE `12U`
```




<hr>



### define SYN\_FW\_MAGIC 

_Magic number identifying a SyntropicOS firmware image header ("SYNF")._ 
```C++
#define SYN_FW_MAGIC `0x53594E46u /* "SYNF" in little-endian */`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_fwimage.h`

