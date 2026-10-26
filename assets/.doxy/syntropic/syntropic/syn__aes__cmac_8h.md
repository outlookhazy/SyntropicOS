

# File syn\_aes\_cmac.h



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_aes\_cmac.h**](syn__aes__cmac_8h.md)

[Go to the source code of this file](syn__aes__cmac_8h_source.md)

_AES-CMAC (Cipher-based Message Authentication Code, RFC 4493 / NIST SP 800-38B)._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_cmac**](#function-syn_aes_cmac) (const uint8\_t key, const uint8\_t \* msg, size\_t msg\_len, uint8\_t mac) <br>_Generate a 128-bit AES-CMAC authentication tag (RFC 4493)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_aes\_cmac\_verify**](#function-syn_aes_cmac_verify) (const uint8\_t key, const uint8\_t \* msg, size\_t msg\_len, const uint8\_t mac) <br>_Verify an AES-CMAC authentication tag in constant time (RFC 4493)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_AES\_CMAC\_BLOCK\_SIZE**](syn__aes__cmac_8h.md#define-syn_aes_cmac_block_size)  `16U`<br>_AES-CMAC cipher block size in bytes (16 bytes / 128 bits)._  |
| define  | [**SYN\_AES\_CMAC\_KEY\_SIZE**](syn__aes__cmac_8h.md#define-syn_aes_cmac_key_size)  `16U`<br>_AES-CMAC key size in bytes (16 bytes / 128 bits)._  |
| define  | [**SYN\_AES\_CMAC\_TAG\_SIZE**](syn__aes__cmac_8h.md#define-syn_aes_cmac_tag_size)  `16U`<br>_AES-CMAC generated authentication tag size in bytes (16 bytes / 128 bits)._  |

## Detailed Description


Constant-time, zero-heap message authentication code engine based on AES-128. Standard primitive for BLE Secure Connections, LoRaWAN, AUTOSAR SecOC, and COSE. 


    
## Public Functions Documentation




### function syn\_aes\_cmac 

_Generate a 128-bit AES-CMAC authentication tag (RFC 4493)._ 
```C++
SYN_Status syn_aes_cmac (
    const uint8_t key,
    const uint8_t * msg,
    size_t msg_len,
    uint8_t mac
) 
```





**Parameters:**


* `key` 16-byte AES-128 secret key. 
* `msg` Pointer to message to authenticate (may be NULL if msg\_len is 0). 
* `msg_len` Length of the message in bytes. 
* `mac` 16-byte buffer to receive the computed MAC tag. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on invalid/NULL parameters. 





        

<hr>



### function syn\_aes\_cmac\_verify 

_Verify an AES-CMAC authentication tag in constant time (RFC 4493)._ 
```C++
bool syn_aes_cmac_verify (
    const uint8_t key,
    const uint8_t * msg,
    size_t msg_len,
    const uint8_t mac
) 
```





**Parameters:**


* `key` 16-byte AES-128 secret key. 
* `msg` Pointer to message to verify (may be NULL if msg\_len is 0). 
* `msg_len` Length of the message in bytes. 
* `mac` 16-byte expected MAC tag to verify against. 



**Returns:**

true if valid and authentic, false on mismatch or invalid parameters. 





        

<hr>
## Macro Definition Documentation





### define SYN\_AES\_CMAC\_BLOCK\_SIZE 

_AES-CMAC cipher block size in bytes (16 bytes / 128 bits)._ 
```C++
#define SYN_AES_CMAC_BLOCK_SIZE `16U`
```




<hr>



### define SYN\_AES\_CMAC\_KEY\_SIZE 

_AES-CMAC key size in bytes (16 bytes / 128 bits)._ 
```C++
#define SYN_AES_CMAC_KEY_SIZE `16U`
```




<hr>



### define SYN\_AES\_CMAC\_TAG\_SIZE 

_AES-CMAC generated authentication tag size in bytes (16 bytes / 128 bits)._ 
```C++
#define SYN_AES_CMAC_TAG_SIZE `16U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_aes_cmac.h`

