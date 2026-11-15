

# File syn\_port\_aes.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_aes.h**](syn__port__aes_8h.md)

[Go to the source code of this file](syn__port__aes_8h_source.md)

_AES / GCM hardware acceleration port interface._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stddef.h>`
* `#include <stdint.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_aes\_decrypt\_block**](#function-syn_port_aes_decrypt_block) (const uint8\_t \* round\_keys, uint8\_t nr, const uint8\_t in, uint8\_t out) <br>_Offload a single 16-byte block decryption to hardware._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_aes\_encrypt\_block**](#function-syn_port_aes_encrypt_block) (const uint8\_t \* round\_keys, uint8\_t nr, const uint8\_t in, uint8\_t out) <br>_Offload a single 16-byte block encryption to hardware._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_aes\_init**](#function-syn_port_aes_init) (void) <br>_Initialize the hardware AES/crypto accelerator._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_ghash\_mult**](#function-syn_port_ghash_mult) (const uint8\_t x, const uint8\_t h, uint8\_t out) <br>_Offload GHASH Galois field GF(2^128) multiplication to hardware._  |




























## Detailed Description


Implement these functions to offload AES block cipher and GHASH field multiplication to microcontroller hardware security / cryptographic accelerators (e.g. STM32 CRYP, ESP32 hardware AES).


Enable via SYN\_USE\_PORT\_AES in syn\_config.h. When disabled or returning SYN\_NOT\_IMPLEMENTED, SyntropicOS automatically falls back to software calculation. 


    
## Public Functions Documentation




### function syn\_port\_aes\_decrypt\_block 

_Offload a single 16-byte block decryption to hardware._ 
```C++
SYN_Status syn_port_aes_decrypt_block (
    const uint8_t * round_keys,
    uint8_t nr,
    const uint8_t in,
    uint8_t out
) 
```





**Parameters:**


* `round_keys` Pointer to expanded round keys. 
* `nr` Number of rounds (10, 12, or 14). 
* `in` Input 16-byte ciphertext block. 
* `out` Output 16-byte plaintext block. 



**Returns:**

SYN\_OK on success, SYN\_NOT\_IMPLEMENTED to use software AES. 





        

<hr>



### function syn\_port\_aes\_encrypt\_block 

_Offload a single 16-byte block encryption to hardware._ 
```C++
SYN_Status syn_port_aes_encrypt_block (
    const uint8_t * round_keys,
    uint8_t nr,
    const uint8_t in,
    uint8_t out
) 
```





**Parameters:**


* `round_keys` Pointer to expanded round keys. 
* `nr` Number of rounds (10, 12, or 14). 
* `in` Input 16-byte plaintext block. 
* `out` Output 16-byte ciphertext block. 



**Returns:**

SYN\_OK on success, SYN\_NOT\_IMPLEMENTED to use software AES. 





        

<hr>



### function syn\_port\_aes\_init 

_Initialize the hardware AES/crypto accelerator._ 
```C++
SYN_Status syn_port_aes_init (
    void
) 
```





**Returns:**

SYN\_OK on success, SYN\_NOT\_IMPLEMENTED if hardware crypto is not available. 





        

<hr>



### function syn\_port\_ghash\_mult 

_Offload GHASH Galois field GF(2^128) multiplication to hardware._ 
```C++
SYN_Status syn_port_ghash_mult (
    const uint8_t x,
    const uint8_t h,
    uint8_t out
) 
```





**Parameters:**


* `x` Input 16-byte block. 
* `h` Input 16-byte subkey block. 
* `out` Output 16-byte product block. 



**Returns:**

SYN\_OK on success, SYN\_NOT\_IMPLEMENTED to use software GHASH. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_aes.h`

