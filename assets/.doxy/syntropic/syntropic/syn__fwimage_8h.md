

# File syn\_fwimage.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_fwimage.h**](syn__fwimage_8h.md)

[Go to the source code of this file](syn__fwimage_8h_source.md)

_Firmware image header — standardized format for OTA slots._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../util/syn_crc.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















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
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_fwimage\_header\_valid**](#function-syn_fwimage_header_valid) (const [**SYN\_FwImageHeader**](structSYN__FwImageHeader.md) \* hdr) <br>_Validate a firmware image header._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_fwimage\_is\_bootable**](#function-syn_fwimage_is_bootable) (const [**SYN\_FwImageHeader**](structSYN__FwImageHeader.md) \* hdr) <br>_Check if a slot contains a bootable image._  |
|  void | [**syn\_fwimage\_seal\_header**](#function-syn_fwimage_seal_header) ([**SYN\_FwImageHeader**](structSYN__FwImageHeader.md) \* hdr) <br>_Compute the header CRC and write it into the header._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_FW\_MAGIC**](syn__fwimage_8h.md#define-syn_fw_magic)  `0x53594E46u  /\* "SYNF" in little-endian \*/`<br>_Magic number identifying a SyntropicOS firmware image header ("SYNF")._  |

## Detailed Description


Each firmware slot (A or B) has this header at a known flash offset. The bootloader and OTA updater use it to decide which image to boot, whether an update succeeded, and when to rollback.


Layout in flash: 
```C++
[SYN_FwImageHeader (24 bytes)] [firmware binary (image_size bytes)]
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





### define SYN\_FW\_MAGIC 

_Magic number identifying a SyntropicOS firmware image header ("SYNF")._ 
```C++
#define SYN_FW_MAGIC `0x53594E46u  /* "SYNF" in little-endian */`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_fwimage.h`

