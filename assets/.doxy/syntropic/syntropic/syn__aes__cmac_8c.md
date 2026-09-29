

# File syn\_aes\_cmac.c



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_aes\_cmac.c**](syn__aes__cmac_8c.md)

[Go to the source code of this file](syn__aes__cmac_8c_source.md)

_AES-CMAC (Cipher-based Message Authentication Code, RFC 4493 / NIST SP 800-38B)._ 

* `#include "syn_aes.h"`
* `#include "syn_aes_cmac.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_cmac**](#function-syn_aes_cmac) (const uint8\_t key, const uint8\_t \* msg, size\_t msg\_len, uint8\_t mac) <br>_Generate a 128-bit AES-CMAC authentication tag (RFC 4493)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_aes\_cmac\_verify**](#function-syn_aes_cmac_verify) (const uint8\_t key, const uint8\_t \* msg, size\_t msg\_len, const uint8\_t mac) <br>_Verify an AES-CMAC authentication tag in constant time (RFC 4493)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**generate\_subkeys**](#function-generate_subkeys) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, uint8\_t k1, uint8\_t k2) <br>_Subkey generation algorithm per RFC 4493 section 2.3._  |
|  void | [**shl\_128**](#function-shl_128) (const uint8\_t in, uint8\_t out) <br>_Left shift a 128-bit block by 1 bit (Big-Endian)._  |


























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
## Public Static Functions Documentation




### function generate\_subkeys 

_Subkey generation algorithm per RFC 4493 section 2.3._ 
```C++
static void generate_subkeys (
    const SYN_AES_Context * ctx,
    uint8_t k1,
    uint8_t k2
) 
```





**Parameters:**


* `ctx` Initialized AES context. 
* `k1` First 128-bit subkey. 
* `k2` Second 128-bit subkey. 




        

<hr>



### function shl\_128 

_Left shift a 128-bit block by 1 bit (Big-Endian)._ 
```C++
static void shl_128 (
    const uint8_t in,
    uint8_t out
) 
```





**Parameters:**


* `in` 16-byte source block. 
* `out` 16-byte shifted output block. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_aes_cmac.c`

