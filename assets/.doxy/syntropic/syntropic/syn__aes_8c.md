

# File syn\_aes.c



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_aes.c**](syn__aes_8c.md)

[Go to the source code of this file](syn__aes_8c_source.md)

_Unified AES block cipher & AEAD implementation (128/192/256-bit keys, ECB, CBC, CTR, GCM)._ 

* `#include "syn_aes.h"`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**AES\_state2**](structAES__state2.md) <br>_Bitsliced AES state matrix (8 slices of 32-bit words)._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_cbc\_decrypt**](#function-syn_aes_cbc_decrypt) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t iv, const uint8\_t \* in, size\_t in\_len, uint8\_t \* out, size\_t out\_capacity, size\_t \* out\_len) <br>_Decrypt data using AES-CBC with PKCS#7 unpadding._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_cbc\_encrypt**](#function-syn_aes_cbc_encrypt) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t iv, const uint8\_t \* in, size\_t in\_len, uint8\_t \* out, size\_t out\_capacity, size\_t \* out\_len) <br>_Encrypt data using AES-CBC with PKCS#7 padding._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_ccm\_decrypt**](#function-syn_aes_ccm_decrypt) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t \* nonce, size\_t nonce\_len, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* in, size\_t in\_len, const uint8\_t \* tag, size\_t tag\_len, uint8\_t \* out) <br>_AES-CCM authenticated decryption and tag verification (NIST SP 800-38C / RFC 3610)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_ccm\_encrypt**](#function-syn_aes_ccm_encrypt) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t \* nonce, size\_t nonce\_len, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* in, size\_t in\_len, uint8\_t \* out, uint8\_t \* tag, size\_t tag\_len) <br>_AES-CCM authenticated encryption (NIST SP 800-38C / RFC 3610)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_ctr**](#function-syn_aes_ctr) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t nonce, const uint8\_t \* in, size\_t len, uint8\_t \* out) <br>_Encrypt/decrypt arbitrary length data using AES-CTR stream mode (NIST SP 800-38A)._  |
|  void | [**syn\_aes\_decrypt\_block**](#function-syn_aes_decrypt_block) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t in, uint8\_t out) <br>_Decrypt a single 16-byte block (ECB mode)._  |
|  void | [**syn\_aes\_encrypt\_block**](#function-syn_aes_encrypt_block) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t in, uint8\_t out) <br>_Encrypt a single 16-byte block (ECB mode)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_gcm\_decrypt**](#function-syn_aes_gcm_decrypt) (const [**SYN\_AES\_GCM\_Context**](structSYN__AES__GCM__Context.md) \* ctx, const uint8\_t \* nonce, size\_t nonce\_len, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* in, size\_t in\_len, uint8\_t \* out, const uint8\_t tag) <br>_AES-GCM authenticated decryption and tag verification (NIST SP 800-38D)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_gcm\_encrypt**](#function-syn_aes_gcm_encrypt) (const [**SYN\_AES\_GCM\_Context**](structSYN__AES__GCM__Context.md) \* ctx, const uint8\_t \* nonce, size\_t nonce\_len, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* in, size\_t in\_len, uint8\_t \* out, uint8\_t tag) <br>_AES-GCM authenticated encryption (NIST SP 800-38D)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_gcm\_init**](#function-syn_aes_gcm_init) ([**SYN\_AES\_GCM\_Context**](structSYN__AES__GCM__Context.md) \* ctx, const uint8\_t \* key, size\_t key\_len) <br>_Initialize AES-GCM AEAD context (expands key and computes GHASH subkey H)._  |
|  void | [**syn\_aes\_ghash\_mult**](#function-syn_aes_ghash_mult) (const uint8\_t x, const uint8\_t h, uint8\_t out) <br>_Multiply 16-byte block by GHASH subkey H in GF(2^128)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_init**](#function-syn_aes_init) ([**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t \* key, size\_t key\_len) <br>_Initialize AES context and perform key expansion for 128/192/256-bit key._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**AES\_decrypt**](#function-aes_decrypt) (const uint16\_t rounds, int nrounds, uint8\_t \* plain16, const uint8\_t \* cipher16) <br>_Bitsliced AES single-block decryption._  |
|  void | [**AES\_setup**](#function-aes_setup) (uint16\_t rounds, const uint8\_t \* key, int nkeywords, int nrounds) <br>_Expand AES key into bitsliced round keys._  |
|  void | [**AddRoundKey2**](#function-addroundkey2) ([**AES\_state2**](structAES__state2.md) \* s, const uint16\_t rk) <br>_XOR 16-bit round keys into dual 16-bit halves of state._  |
|  void | [**GetOneColumn**](#function-getonecolumn) ([**AES\_state2**](structAES__state2.md) \* s, const uint16\_t a, int c) <br>_Extract one column from round key into column 0 of destination state._  |
|  void | [**InvShiftRows2**](#function-invshiftrows2) ([**AES\_state2**](structAES__state2.md) \* s) <br>_Bitsliced InvShiftRows operation._  |
|  void | [**KeySetupColumnMix**](#function-keysetupcolumnmix) ([**AES\_state2**](structAES__state2.md) \* s, uint16\_t r, const uint16\_t a, int c1, int c2) <br>_Key schedule helper: mix column word into target round key._  |
|  void | [**KeySetupTransform**](#function-keysetuptransform) ([**AES\_state2**](structAES__state2.md) \* s, const [**AES\_state2**](structAES__state2.md) \* r) <br>_Key schedule helper: rotate rows one position upwards and XOR round constant._  |
|  void | [**LoadByte**](#function-loadbyte) (uint16\_t rk, uint8\_t byte, int r, int c) <br>_Load a single byte into bitsliced round key at row r, column c._  |
|  void | [**LoadBytes1**](#function-loadbytes1) ([**AES\_state2**](structAES__state2.md) \* s, const uint8\_t \* b0) <br>_Pack 1 16-byte block into bitsliced state (Block 0 in bits 0..15)._  |
|  void | [**LoadBytes2**](#function-loadbytes2) ([**AES\_state2**](structAES__state2.md) \* s, const uint8\_t \* b0, const uint8\_t \* b1) <br>_Pack 2 16-byte blocks into bitsliced state (Block 0 in 0..15, Block 1 in 16..31)._  |
|  void | [**MixColumns\_dec2**](#function-mixcolumns_dec2) ([**AES\_state2**](structAES__state2.md) \* s) <br>_Bitsliced InvMixColumns operation (inverse / decryption)._  |
|  void | [**MixColumns\_enc2**](#function-mixcolumns_enc2) ([**AES\_state2**](structAES__state2.md) \* s) <br>_Bitsliced MixColumns operation (forward / encryption) for 2 blocks simultaneously._  |
|  void | [**MultX**](#function-multx) ([**AES\_state2**](structAES__state2.md) \* s) <br>_Multiply all cells in bitsliced state s by x in GF(2^8)._  |
|  void | [**SaveBytes1**](#function-savebytes1) (uint8\_t \* b0, [**AES\_state2**](structAES__state2.md) \* s) <br>_Unpack bitsliced state into 1 16-byte block in place (from bits 0..15)._  |
|  void | [**SaveBytes2**](#function-savebytes2) (uint8\_t \* b0, uint8\_t \* b1, [**AES\_state2**](structAES__state2.md) \* s) <br>_Unpack bitsliced state into 2 16-byte blocks in place._  |
|  void | [**ShiftMixAdd2**](#function-shiftmixadd2) ([**AES\_state2**](structAES__state2.md) \* s, const uint16\_t rk) <br>_Fused register-resident ShiftRows + MixColumns + AddRoundKey for inner encryption rounds._  |
|  void | [**SubBytes\_dec2**](#function-subbytes_dec2) ([**AES\_state2**](structAES__state2.md) \* s) <br>_Bitsliced Boyar-Peralta depth-16 AES S-Box circuit (inverse / decryption)._  |
|  void | [**SubBytes\_enc2**](#function-subbytes_enc2) ([**AES\_state2**](structAES__state2.md) \* s) <br>_Bitsliced Boyar-Peralta depth-16 AES S-Box circuit (forward / encryption)._  |
|  void | [**aes\_ct\_encrypt\_core**](#function-aes_ct_encrypt_core) ([**AES\_state2**](structAES__state2.md) \* s, const uint16\_t rk, uint8\_t nr) <br>_Core constant-time bitsliced AES encryption routine serving 1 or 2 blocks._  |
|  void | [**aes\_encrypt\_two\_blocks**](#function-aes_encrypt_two_blocks) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t in0, const uint8\_t in1, uint8\_t out0, uint8\_t out1) <br>_Encrypt 2 16-byte blocks in parallel using bitsliced engine (or port offload)._  |
|  uint32\_t | [**aes\_load\_be32**](#function-aes_load_be32) (const uint8\_t p) <br>_Load a 32-bit big-endian word from 4 bytes._  |
|  void | [**aes\_ortho32**](#function-aes_ortho32) (uint32\_t q) <br>_Self-inverse 3-stage SWAR SWAPMOVE bit-matrix orthogonal transpose._  |
|  uint32\_t | [**aes\_rotr16\_12\_2**](#function-aes_rotr16_12_2) (uint32\_t v) <br>_Rotate dual 16-bit halves right by 12 bits (left by 4 bits) simultaneously (SWAR)._  |
|  uint32\_t | [**aes\_rotr16\_4\_2**](#function-aes_rotr16_4_2) (uint32\_t v) <br>_Rotate dual 16-bit halves right by 4 bits simultaneously (SWAR)._  |
|  uint32\_t | [**aes\_rotr16\_8\_2**](#function-aes_rotr16_8_2) (uint32\_t v) <br>_Rotate dual 16-bit halves by 8 bits simultaneously (SWAR byte swap)._  |
|  uint32\_t | [**aes\_shift\_row\_slice**](#function-aes_shift_row_slice) (uint32\_t v) <br>_Bitsliced ShiftRows permutation on a single dual-block slice word._  |
|  void | [**aes\_store\_be32**](#function-aes_store_be32) (uint8\_t p, uint32\_t w) <br>_Store a 32-bit big-endian word to 4 bytes._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**ccm\_compute\_mac**](#function-ccm_compute_mac) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t \* nonce, size\_t nonce\_len, size\_t L, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* data, size\_t data\_len, size\_t tag\_len, uint8\_t mac) <br>_Compute NIST SP 800-38C / RFC 3610 CBC-MAC tag over header and payload._  |
|  void | [**ccm\_format\_ctr**](#function-ccm_format_ctr) (uint8\_t a, const uint8\_t \* nonce, size\_t nonce\_len, size\_t L, uint64\_t counter) <br>_Format counter block Ai for AES-CCM CTR mode._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ccm\_lengths\_valid**](#function-ccm_lengths_valid) (size\_t nonce\_len, size\_t aad\_len, size\_t in\_len, size\_t tag\_len) <br>_Validate AES-CCM nonce, tag, AAD, and payload length bounds (RFC 3610)._  |
|  void | [**ccm\_mac\_feed**](#function-ccm_mac_feed) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, uint8\_t mac, uint8\_t \* blk, size\_t \* blk\_len, const uint8\_t \* data, size\_t len) <br>_Feed data into AES-CCM CBC-MAC accumulator._  |
|  void | [**ccm\_mac\_pad\_zero**](#function-ccm_mac_pad_zero) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, uint8\_t mac, uint8\_t \* blk, size\_t \* blk\_len) <br>_Pad partial block with zeros and finish block encryption in CBC-MAC._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ccm\_params\_valid**](#function-ccm_params_valid) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t \* nonce, size\_t nonce\_len, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* in, size\_t in\_len, const uint8\_t \* out, const uint8\_t \* tag, size\_t tag\_len) <br>_Validate AES-CCM encrypt/decrypt pointer and length parameters._  |
|  void | [**gcm\_compute\_j0**](#function-gcm_compute_j0) (const [**SYN\_AES\_GCM\_Context**](structSYN__AES__GCM__Context.md) \* ctx, const uint8\_t \* nonce, size\_t nonce\_len, uint8\_t j0) <br>_Compute initial J0 counter block for GCM._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**gcm\_params\_valid**](#function-gcm_params_valid) (const [**SYN\_AES\_GCM\_Context**](structSYN__AES__GCM__Context.md) \* ctx, const uint8\_t \* nonce, size\_t nonce\_len, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* in, size\_t in\_len, const uint8\_t \* out, const uint8\_t \* tag) <br>_Validate AES-GCM encrypt/decrypt parameters and NIST SP 800-38D length bounds._  |
|  uint64\_t | [**ghash\_bmul32**](#function-ghash_bmul32) (uint32\_t x, uint32\_t y) <br>_Constant-time 32x32-&gt;64 carryless polynomial multiplication in GF(2)[X]._  |
|  void | [**ghash\_bmul64**](#function-ghash_bmul64) (uint32\_t x0, uint32\_t x1, uint32\_t y0, uint32\_t y1, uint32\_t r) <br>_Multiply two 64-bit polynomials (2 words each) in GF(2)[X] via 3-mul Karatsuba._  |
|  void | [**ghash\_mult\_bit**](#function-ghash_mult_bit) (const uint8\_t x, const uint8\_t y, uint8\_t out) <br>_Constant-time GHASH field multiplication in GF(2^128)._  |
|  void | [**ghash\_mult\_reduce**](#function-ghash_mult_reduce) (uint32\_t a, const uint32\_t b) <br>_Multiply two 128-bit polynomials in GF(2^128) via 9-mul Karatsuba and reduce in place._  |
|  void | [**ghash\_process\_data**](#function-ghash_process_data) (const [**SYN\_AES\_GCM\_Context**](structSYN__AES__GCM__Context.md) \* ctx, const uint8\_t \* data, size\_t len, uint32\_t yw) <br>_Process data buffer through GHASH using 32-bit word accumulator._  |
|  uint32\_t | [**ghash\_rev32**](#function-ghash_rev32) (uint32\_t x) <br>_Bit-reverse a 32-bit word in constant time._  |
|  void | [**ghash\_step**](#function-ghash_step) (const [**SYN\_AES\_GCM\_Context**](structSYN__AES__GCM__Context.md) \* ctx, const uint8\_t block, uint32\_t yw) <br>_One GHASH accumulation step: yw = (yw ^ block) \* H in GF(2^128)._  |
|  void | [**inc128**](#function-inc128) (uint8\_t counter) <br>_Increment 128-bit big-endian counter._  |
|  void | [**inc32**](#function-inc32) (uint8\_t block) <br>_Increment rightmost 32 bits of 128-bit counter._  |
|  void | [**put\_be64**](#function-put_be64) (uint8\_t out, uint64\_t val) <br>_Write 64-bit value big-endian._  |
|  void | [**syn\_secure\_zero**](#function-syn_secure_zero) (void \* v, size\_t n) <br>_Zeroize sensitive stack buffers without compiler dead-store elimination._  |
|  void | [**syn\_secure\_zero\_u32**](#function-syn_secure_zero_u32) (uint32\_t \* v, size\_t words) <br>_Zeroize sensitive 32-bit aligned buffers using 32-bit stores._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SWAPMOVE**](syn__aes_8c.md#define-swapmove) (a, b, mask, shift) `/* multi line expression */`<br>_Exchange masked bit-planes between two 32-bit words separated by shift bits._  |

## Public Functions Documentation




### function syn\_aes\_cbc\_decrypt 

_Decrypt data using AES-CBC with PKCS#7 unpadding._ 
```C++
SYN_Status syn_aes_cbc_decrypt (
    const SYN_AES_Context * ctx,
    const uint8_t iv,
    const uint8_t * in,
    size_t in_len,
    uint8_t * out,
    size_t out_capacity,
    size_t * out_len
) 
```





**Note:**

`out` must not alias `in`. On invalid PKCS#7 padding, `out` is zeroized across all decrypted blocks before returning SYN\_INVALID\_PARAM.




**Parameters:**


* `ctx` Initialized AES context. 
* `iv` 16-byte initialization vector. 
* `in` Ciphertext buffer (must be non-empty multiple of 16 bytes). 
* `in_len` Ciphertext length in bytes. 
* `out` Plaintext output buffer (must not alias `in`). 
* `out_capacity` Capacity of plaintext output buffer. 
* `out_len` Number of plaintext bytes written. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on invalid padding or parameters. 





        

<hr>



### function syn\_aes\_cbc\_encrypt 

_Encrypt data using AES-CBC with PKCS#7 padding._ 
```C++
SYN_Status syn_aes_cbc_encrypt (
    const SYN_AES_Context * ctx,
    const uint8_t iv,
    const uint8_t * in,
    size_t in_len,
    uint8_t * out,
    size_t out_capacity,
    size_t * out_len
) 
```





**Parameters:**


* `ctx` Initialized AES context. 
* `iv` 16-byte initialization vector. 
* `in` Plaintext buffer (may be NULL if in\_len is 0). 
* `in_len` Plaintext length in bytes. 
* `out` Ciphertext output buffer. 
* `out_capacity` Maximum capacity of output buffer (must be &gt;= in\_len + PKCS#7 pad). 
* `out_len` Number of ciphertext bytes written. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on failure. 





        

<hr>



### function syn\_aes\_ccm\_decrypt 

_AES-CCM authenticated decryption and tag verification (NIST SP 800-38C / RFC 3610)._ 
```C++
SYN_Status syn_aes_ccm_decrypt (
    const SYN_AES_Context * ctx,
    const uint8_t * nonce,
    size_t nonce_len,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * in,
    size_t in_len,
    const uint8_t * tag,
    size_t tag_len,
    uint8_t * out
) 
```



Decrypts ciphertext in CTR mode and verifies CBC-MAC authentication tag in constant time. If verification fails, plaintext buffer is zeroed (if out != in) and SYN\_ERROR is returned.




**Parameters:**


* `ctx` Initialized AES context. 
* `nonce` Nonce buffer (length must be 7..13 bytes). 
* `nonce_len` Nonce length in bytes (7 to 13). 
* `aad` Additional authenticated data (may be NULL if aad\_len is 0). 
* `aad_len` AAD length in bytes. 
* `in` Ciphertext buffer to decrypt (may be NULL if in\_len is 0). 
* `in_len` Ciphertext length in bytes. 
* `tag` Authentication tag to verify against. 
* `tag_len` Length of authentication tag in bytes (4, 6, 8, 10, 12, 14, or 16). 
* `out` Plaintext output buffer (must be at least in\_len bytes). 



**Returns:**

SYN\_OK on success, SYN\_ERROR on tag mismatch, or SYN\_INVALID\_PARAM on invalid params. 





        

<hr>



### function syn\_aes\_ccm\_encrypt 

_AES-CCM authenticated encryption (NIST SP 800-38C / RFC 3610)._ 
```C++
SYN_Status syn_aes_ccm_encrypt (
    const SYN_AES_Context * ctx,
    const uint8_t * nonce,
    size_t nonce_len,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * in,
    size_t in_len,
    uint8_t * out,
    uint8_t * tag,
    size_t tag_len
) 
```



Computes CBC-MAC authentication tag and encrypts payload in CTR mode.




**Parameters:**


* `ctx` Initialized AES context. 
* `nonce` Nonce buffer (length must be 7..13 bytes). 
* `nonce_len` Nonce length in bytes (7 to 13). 
* `aad` Additional authenticated data (may be NULL if aad\_len is 0). 
* `aad_len` AAD length in bytes. 
* `in` Plaintext buffer to encrypt (may be NULL if in\_len is 0). 
* `in_len` Plaintext length in bytes. 
* `out` Ciphertext output buffer (must be at least in\_len bytes). 
* `tag` Authentication tag output buffer. 
* `tag_len` Length of authentication tag in bytes (4, 6, 8, 10, 12, 14, or 16). 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on invalid parameters. 





        

<hr>



### function syn\_aes\_ctr 

_Encrypt/decrypt arbitrary length data using AES-CTR stream mode (NIST SP 800-38A)._ 
```C++
SYN_Status syn_aes_ctr (
    const SYN_AES_Context * ctx,
    const uint8_t nonce,
    const uint8_t * in,
    size_t len,
    uint8_t * out
) 
```





**Parameters:**


* `ctx` Initialized AES context. 
* `nonce` 16-byte initial counter block (incremented as 128-bit big-endian). 
* `in` Input data buffer (plaintext for encrypt, ciphertext for decrypt). 
* `len` Length of input and output data in bytes. 
* `out` Output data buffer. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on NULL pointers. 





        

<hr>



### function syn\_aes\_decrypt\_block 

_Decrypt a single 16-byte block (ECB mode)._ 
```C++
void syn_aes_decrypt_block (
    const SYN_AES_Context * ctx,
    const uint8_t in,
    uint8_t out
) 
```





**Parameters:**


* `ctx` Initialized AES context. 
* `in` 16-byte ciphertext input block. 
* `out` 16-byte plaintext output block. 




        

<hr>



### function syn\_aes\_encrypt\_block 

_Encrypt a single 16-byte block (ECB mode)._ 
```C++
void syn_aes_encrypt_block (
    const SYN_AES_Context * ctx,
    const uint8_t in,
    uint8_t out
) 
```





**Parameters:**


* `ctx` Initialized AES context. 
* `in` 16-byte plaintext input block. 
* `out` 16-byte ciphertext output block. 




        

<hr>



### function syn\_aes\_gcm\_decrypt 

_AES-GCM authenticated decryption and tag verification (NIST SP 800-38D)._ 
```C++
SYN_Status syn_aes_gcm_decrypt (
    const SYN_AES_GCM_Context * ctx,
    const uint8_t * nonce,
    size_t nonce_len,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * in,
    size_t in_len,
    uint8_t * out,
    const uint8_t tag
) 
```



Decrypts ciphertext and verifies the authentication tag in constant time. If authentication fails, plaintext output is zeroed (when out != in) and SYN\_ERROR is returned.




**Parameters:**


* `ctx` Initialized AES-GCM context. 
* `nonce` Initialization vector / nonce buffer. 
* `nonce_len` Nonce length in bytes. 
* `aad` Additional authenticated data (may be NULL if aad\_len is 0). 
* `aad_len` AAD length in bytes. 
* `in` Ciphertext buffer to decrypt (may be NULL if in\_len is 0). 
* `in_len` Ciphertext length in bytes. 
* `out` Plaintext output buffer (must be at least in\_len bytes). 
* `tag` 16-byte expected authentication tag to verify against. 



**Returns:**

SYN\_OK on successful verification, SYN\_ERROR on tag mismatch, or SYN\_INVALID\_PARAM on invalid parameters. 





        

<hr>



### function syn\_aes\_gcm\_encrypt 

_AES-GCM authenticated encryption (NIST SP 800-38D)._ 
```C++
SYN_Status syn_aes_gcm_encrypt (
    const SYN_AES_GCM_Context * ctx,
    const uint8_t * nonce,
    size_t nonce_len,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * in,
    size_t in_len,
    uint8_t * out,
    uint8_t tag
) 
```





**Parameters:**


* `ctx` Initialized AES-GCM context. 
* `nonce` Initialization vector / nonce buffer. 
* `nonce_len` Nonce length in bytes (standard is 12 bytes). 
* `aad` Additional authenticated data (may be NULL if aad\_len is 0). 
* `aad_len` AAD length in bytes. 
* `in` Plaintext buffer to encrypt (may be NULL if in\_len is 0). 
* `in_len` Plaintext length in bytes. 
* `out` Ciphertext output buffer (must be at least in\_len bytes). 
* `tag` 16-byte authentication tag output buffer. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on NULL pointers or invalid params. 





        

<hr>



### function syn\_aes\_gcm\_init 

_Initialize AES-GCM AEAD context (expands key and computes GHASH subkey H)._ 
```C++
SYN_Status syn_aes_gcm_init (
    SYN_AES_GCM_Context * ctx,
    const uint8_t * key,
    size_t key_len
) 
```





**Parameters:**


* `ctx` Pointer to GCM context to initialize. 
* `key` Secret key buffer. 
* `key_len` Length of secret key in bytes (16, 24, or 32). 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on failure. 





        

<hr>



### function syn\_aes\_ghash\_mult 

_Multiply 16-byte block by GHASH subkey H in GF(2^128)._ 
```C++
void syn_aes_ghash_mult (
    const uint8_t x,
    const uint8_t h,
    uint8_t out
) 
```





**Parameters:**


* `x` Input 16-byte field element. 
* `h` Input 16-byte GHASH subkey H. 
* `out` Output 16-byte product block. 




        

<hr>



### function syn\_aes\_init 

_Initialize AES context and perform key expansion for 128/192/256-bit key._ 
```C++
SYN_Status syn_aes_init (
    SYN_AES_Context * ctx,
    const uint8_t * key,
    size_t key_len
) 
```





**Parameters:**


* `ctx` Pointer to AES context to initialize. 
* `key` Secret key buffer. 
* `key_len` Length of secret key in bytes (16, 24, or 32). 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on invalid key size or NULL pointers. 





        

<hr>
## Public Static Functions Documentation




### function AES\_decrypt 

_Bitsliced AES single-block decryption._ 
```C++
static void AES_decrypt (
    const uint16_t rounds,
    int nrounds,
    uint8_t * plain16,
    const uint8_t * cipher16
) 
```





**Parameters:**


* `rounds` Bitsliced round keys. 
* `nrounds` Number of rounds. 
* `plain16` Output 16-byte plaintext block. 
* `cipher16` Input 16-byte ciphertext block. 




        

<hr>



### function AES\_setup 

_Expand AES key into bitsliced round keys._ 
```C++
static void AES_setup (
    uint16_t rounds,
    const uint8_t * key,
    int nkeywords,
    int nrounds
) 
```





**Parameters:**


* `rounds` Target array of (nrounds + 1) bitsliced round key slice arrays. 
* `key` Input key bytes. 
* `nkeywords` Number of 32-bit key words (4, 6, or 8). 
* `nrounds` Number of rounds (10, 12, or 14). 




        

<hr>



### function AddRoundKey2 

_XOR 16-bit round keys into dual 16-bit halves of state._ 
```C++
static inline void AddRoundKey2 (
    AES_state2 * s,
    const uint16_t rk
) 
```





**Parameters:**


* `s` Cipher state. 
* `rk` 8-element 16-bit round key array. 




        

<hr>



### function GetOneColumn 

_Extract one column from round key into column 0 of destination state._ 
```C++
static void GetOneColumn (
    AES_state2 * s,
    const uint16_t a,
    int c
) 
```





**Parameters:**


* `s` Destination column state. 
* `a` Source round key slice array. 
* `c` Source column index (0..3). 




        

<hr>



### function InvShiftRows2 

_Bitsliced InvShiftRows operation._ 
```C++
static inline void InvShiftRows2 (
    AES_state2 * s
) 
```





**Parameters:**


* `s` Bitsliced state pointer. 




        

<hr>



### function KeySetupColumnMix 

_Key schedule helper: mix column word into target round key._ 
```C++
static void KeySetupColumnMix (
    AES_state2 * s,
    uint16_t r,
    const uint16_t a,
    int c1,
    int c2
) 
```





**Parameters:**


* `s` Accumulated column state. 
* `r` Target round key state. 
* `a` Source preceding round key state. 
* `c1` Target column index (0..3). 
* `c2` Source column index (0..3). 




        

<hr>



### function KeySetupTransform 

_Key schedule helper: rotate rows one position upwards and XOR round constant._ 
```C++
static void KeySetupTransform (
    AES_state2 * s,
    const AES_state2 * r
) 
```





**Parameters:**


* `s` Destination column state. 
* `r` Round constant column state. 




        

<hr>



### function LoadByte 

_Load a single byte into bitsliced round key at row r, column c._ 
```C++
static void LoadByte (
    uint16_t rk,
    uint8_t byte,
    int r,
    int c
) 
```





**Parameters:**


* `rk` Target round key slice array. 
* `byte` Input byte. 
* `r` Row index (0..3). 
* `c` Column index (0..3). 




        

<hr>



### function LoadBytes1 

_Pack 1 16-byte block into bitsliced state (Block 0 in bits 0..15)._ 
```C++
static void LoadBytes1 (
    AES_state2 * s,
    const uint8_t * b0
) 
```





**Parameters:**


* `s` Destination bitsliced state. 
* `b0` 16-byte input block. 




        

<hr>



### function LoadBytes2 

_Pack 2 16-byte blocks into bitsliced state (Block 0 in 0..15, Block 1 in 16..31)._ 
```C++
static void LoadBytes2 (
    AES_state2 * s,
    const uint8_t * b0,
    const uint8_t * b1
) 
```





**Parameters:**


* `s` Destination bitsliced state. 
* `b0` First 16-byte input block. 
* `b1` Second 16-byte input block. 




        

<hr>



### function MixColumns\_dec2 

_Bitsliced InvMixColumns operation (inverse / decryption)._ 
```C++
static inline void MixColumns_dec2 (
    AES_state2 * s
) 
```





**Parameters:**


* `s` Bitsliced state pointer. 




        

<hr>



### function MixColumns\_enc2 

_Bitsliced MixColumns operation (forward / encryption) for 2 blocks simultaneously._ 
```C++
static inline void MixColumns_enc2 (
    AES_state2 * s
) 
```





**Parameters:**


* `s` Bitsliced state pointer. 




        

<hr>



### function MultX 

_Multiply all cells in bitsliced state s by x in GF(2^8)._ 
```C++
static void MultX (
    AES_state2 * s
) 
```





**Parameters:**


* `s` Bitsliced state pointer. 




        

<hr>



### function SaveBytes1 

_Unpack bitsliced state into 1 16-byte block in place (from bits 0..15)._ 
```C++
static void SaveBytes1 (
    uint8_t * b0,
    AES_state2 * s
) 
```





**Parameters:**


* `b0` Destination 16-byte block. 
* `s` Bitsliced state (transposed in place before caller zeroizes it). 




        

<hr>



### function SaveBytes2 

_Unpack bitsliced state into 2 16-byte blocks in place._ 
```C++
static void SaveBytes2 (
    uint8_t * b0,
    uint8_t * b1,
    AES_state2 * s
) 
```





**Parameters:**


* `b0` Destination first 16-byte block. 
* `b1` Destination second 16-byte block. 
* `s` Bitsliced state (transposed in place before caller zeroizes it). 




        

<hr>



### function ShiftMixAdd2 

_Fused register-resident ShiftRows + MixColumns + AddRoundKey for inner encryption rounds._ 
```C++
static inline void ShiftMixAdd2 (
    AES_state2 * s,
    const uint16_t rk
) 
```





**Parameters:**


* `s` Cipher state. 
* `rk` 8-element 16-bit round key array. 




        

<hr>



### function SubBytes\_dec2 

_Bitsliced Boyar-Peralta depth-16 AES S-Box circuit (inverse / decryption)._ 
```C++
static void SubBytes_dec2 (
    AES_state2 * s
) 
```





**Parameters:**


* `s` State containing 8 bit-slice planes. 




        

<hr>



### function SubBytes\_enc2 

_Bitsliced Boyar-Peralta depth-16 AES S-Box circuit (forward / encryption)._ 
```C++
static void SubBytes_enc2 (
    AES_state2 * s
) 
```



Kept non-inline to avoid stack spilling and excessive register pressure on ARM Cortex-M4.




**Parameters:**


* `s` State containing 8 bit-slice planes. 




        

<hr>



### function aes\_ct\_encrypt\_core 

_Core constant-time bitsliced AES encryption routine serving 1 or 2 blocks._ 
```C++
static void aes_ct_encrypt_core (
    AES_state2 * s,
    const uint16_t rk,
    uint8_t nr
) 
```





**Parameters:**


* `s` Cipher state. 
* `rk` Round keys array. 
* `nr` Number of rounds (10, 12, or 14). 




        

<hr>



### function aes\_encrypt\_two\_blocks 

_Encrypt 2 16-byte blocks in parallel using bitsliced engine (or port offload)._ 
```C++
static void aes_encrypt_two_blocks (
    const SYN_AES_Context * ctx,
    const uint8_t in0,
    const uint8_t in1,
    uint8_t out0,
    uint8_t out1
) 
```





**Parameters:**


* `ctx` Initialized AES context. 
* `in0` First 16-byte plaintext block. 
* `in1` Second 16-byte plaintext block. 
* `out0` First 16-byte ciphertext block. 
* `out1` Second 16-byte ciphertext block. 




        

<hr>



### function aes\_load\_be32 

_Load a 32-bit big-endian word from 4 bytes._ 
```C++
static inline uint32_t aes_load_be32 (
    const uint8_t p
) 
```





**Parameters:**


* `p` 4-byte input buffer. 



**Returns:**

Big-endian 32-bit word. 





        

<hr>



### function aes\_ortho32 

_Self-inverse 3-stage SWAR SWAPMOVE bit-matrix orthogonal transpose._ 
```C++
static void aes_ortho32 (
    uint32_t q
) 
```



Swaps word coordinate (r0, c1, c0) with bit coordinate (b2, b1, b0), keeping block index (bit 4) and row high bit (bit 3) in place. Transposing packed byte words yields bitsliced state slices; transposing bitslices yields packed byte words.




**Parameters:**


* `q` Array of 8 32-bit words. 




        

<hr>



### function aes\_rotr16\_12\_2 

_Rotate dual 16-bit halves right by 12 bits (left by 4 bits) simultaneously (SWAR)._ 
```C++
static inline uint32_t aes_rotr16_12_2 (
    uint32_t v
) 
```





**Parameters:**


* `v` 32-bit input word. 



**Returns:**

32-bit word with both 16-bit halves rotated right by 12 bits. 





        

<hr>



### function aes\_rotr16\_4\_2 

_Rotate dual 16-bit halves right by 4 bits simultaneously (SWAR)._ 
```C++
static inline uint32_t aes_rotr16_4_2 (
    uint32_t v
) 
```





**Parameters:**


* `v` 32-bit input word. 



**Returns:**

32-bit word with both 16-bit halves rotated right by 4 bits. 





        

<hr>



### function aes\_rotr16\_8\_2 

_Rotate dual 16-bit halves by 8 bits simultaneously (SWAR byte swap)._ 
```C++
static inline uint32_t aes_rotr16_8_2 (
    uint32_t v
) 
```





**Parameters:**


* `v` 32-bit input word. 



**Returns:**

32-bit word with bytes swapped in each 16-bit half. 





        

<hr>



### function aes\_shift\_row\_slice 

_Bitsliced ShiftRows permutation on a single dual-block slice word._ 
```C++
static inline uint32_t aes_shift_row_slice (
    uint32_t v
) 
```





**Parameters:**


* `v` 32-bit dual-block bit-slice word. 



**Returns:**

Permuted 32-bit bit-slice word. 





        

<hr>



### function aes\_store\_be32 

_Store a 32-bit big-endian word to 4 bytes._ 
```C++
static inline void aes_store_be32 (
    uint8_t p,
    uint32_t w
) 
```





**Parameters:**


* `p` 4-byte output buffer. 
* `w` 32-bit word. 




        

<hr>



### function ccm\_compute\_mac 

_Compute NIST SP 800-38C / RFC 3610 CBC-MAC tag over header and payload._ 
```C++
static SYN_Status ccm_compute_mac (
    const SYN_AES_Context * ctx,
    const uint8_t * nonce,
    size_t nonce_len,
    size_t L,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * data,
    size_t data_len,
    size_t tag_len,
    uint8_t mac
) 
```





**Parameters:**


* `ctx` AES context. 
* `nonce` Nonce buffer. 
* `nonce_len` Nonce length in bytes. 
* `L` Length parameter (15 - nonce\_len). 
* `aad` Associated data buffer. 
* `aad_len` Associated data length in bytes. 
* `data` Payload data buffer. 
* `data_len` Payload data length in bytes. 
* `tag_len` Desired MAC tag length in bytes. 
* `mac` 16-byte raw CBC-MAC output buffer. 



**Returns:**

[**SYN\_Status**](syn__defs_8h.md#enum-syn_status) SYN\_OK on success, error code otherwise. 





        

<hr>



### function ccm\_format\_ctr 

_Format counter block Ai for AES-CCM CTR mode._ 
```C++
static void ccm_format_ctr (
    uint8_t a,
    const uint8_t * nonce,
    size_t nonce_len,
    size_t L,
    uint64_t counter
) 
```





**Parameters:**


* `a` 16-byte formatted counter output block. 
* `nonce` Pointer to nonce buffer. 
* `nonce_len` Length of nonce in bytes. 
* `L` Length parameter (15 - nonce\_len). 
* `counter` Big-endian counter integer. 




        

<hr>



### function ccm\_lengths\_valid 

_Validate AES-CCM nonce, tag, AAD, and payload length bounds (RFC 3610)._ 
```C++
static bool ccm_lengths_valid (
    size_t nonce_len,
    size_t aad_len,
    size_t in_len,
    size_t tag_len
) 
```





**Parameters:**


* `nonce_len` Nonce length in bytes. 
* `aad_len` Associated data length in bytes. 
* `in_len` Payload length in bytes. 
* `tag_len` Tag length in bytes. 



**Returns:**

true if lengths are within specification limits, false otherwise. 





        

<hr>



### function ccm\_mac\_feed 

_Feed data into AES-CCM CBC-MAC accumulator._ 
```C++
static void ccm_mac_feed (
    const SYN_AES_Context * ctx,
    uint8_t mac,
    uint8_t * blk,
    size_t * blk_len,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `ctx` AES context. 
* `mac` Running CBC-MAC accumulator block. 
* `blk` Partial block staging buffer. 
* `blk_len` Pointer to length of valid bytes in staging buffer. 
* `data` Input byte buffer to feed into MAC. 
* `len` Number of bytes in input data buffer. 




        

<hr>



### function ccm\_mac\_pad\_zero 

_Pad partial block with zeros and finish block encryption in CBC-MAC._ 
```C++
static void ccm_mac_pad_zero (
    const SYN_AES_Context * ctx,
    uint8_t mac,
    uint8_t * blk,
    size_t * blk_len
) 
```





**Parameters:**


* `ctx` AES context. 
* `mac` Running CBC-MAC accumulator block. 
* `blk` Partial block staging buffer. 
* `blk_len` Pointer to length of valid bytes in staging buffer. 




        

<hr>



### function ccm\_params\_valid 

_Validate AES-CCM encrypt/decrypt pointer and length parameters._ 
```C++
static bool ccm_params_valid (
    const SYN_AES_Context * ctx,
    const uint8_t * nonce,
    size_t nonce_len,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * in,
    size_t in_len,
    const uint8_t * out,
    const uint8_t * tag,
    size_t tag_len
) 
```





**Parameters:**


* `ctx` AES context. 
* `nonce` Nonce buffer. 
* `nonce_len` Nonce length in bytes. 
* `aad` Associated data buffer. 
* `aad_len` Associated data length in bytes. 
* `in` Input payload buffer. 
* `in_len` Input payload length in bytes. 
* `out` Output payload buffer. 
* `tag` Authentication tag buffer. 
* `tag_len` Authentication tag length in bytes. 



**Returns:**

true if all parameters are valid, false otherwise. 





        

<hr>



### function gcm\_compute\_j0 

_Compute initial J0 counter block for GCM._ 
```C++
static void gcm_compute_j0 (
    const SYN_AES_GCM_Context * ctx,
    const uint8_t * nonce,
    size_t nonce_len,
    uint8_t j0
) 
```





**Parameters:**


* `ctx` GCM context. 
* `nonce` Nonce buffer. 
* `nonce_len` Nonce length in bytes. 
* `j0` Output 16-byte J0 block. 




        

<hr>



### function gcm\_params\_valid 

_Validate AES-GCM encrypt/decrypt parameters and NIST SP 800-38D length bounds._ 
```C++
static bool gcm_params_valid (
    const SYN_AES_GCM_Context * ctx,
    const uint8_t * nonce,
    size_t nonce_len,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * in,
    size_t in_len,
    const uint8_t * out,
    const uint8_t * tag
) 
```





**Parameters:**


* `ctx` GCM context. 
* `nonce` Initialization vector buffer. 
* `nonce_len` Nonce length in bytes. 
* `aad` Additional authenticated data buffer. 
* `aad_len` AAD length in bytes. 
* `in` Input payload buffer. 
* `in_len` Input payload length in bytes. 
* `out` Output payload buffer. 
* `tag` Authentication tag buffer. 



**Returns:**

true if all parameters are valid, false otherwise. 





        

<hr>



### function ghash\_bmul32 

_Constant-time 32x32-&gt;64 carryless polynomial multiplication in GF(2)[X]._ 
```C++
static inline uint64_t ghash_bmul32 (
    uint32_t x,
    uint32_t y
) 
```



Keeps operands as 32-bit integers so 32-bit ARM Cortex-M4 generates single-cycle UMULL instructions rather than multi-word 64x64 multiply helper loops.




**Parameters:**


* `x` First 32-bit polynomial operand. 
* `y` Second 32-bit polynomial operand. 



**Returns:**

64-bit carryless product in GF(2)[X]. 





        

<hr>



### function ghash\_bmul64 

_Multiply two 64-bit polynomials (2 words each) in GF(2)[X] via 3-mul Karatsuba._ 
```C++
static void ghash_bmul64 (
    uint32_t x0,
    uint32_t x1,
    uint32_t y0,
    uint32_t y1,
    uint32_t r
) 
```



Kept non-inline so GCC does not inline 9 copies of 16-UMULL ghash\_bmul32 into one stack frame.




**Parameters:**


* `x0` Low 32 bits of X. 
* `x1` High 32 bits of X. 
* `y0` Low 32 bits of Y. 
* `y1` High 32 bits of Y. 
* `r` Output 4-word product array. 




        

<hr>



### function ghash\_mult\_bit 

_Constant-time GHASH field multiplication in GF(2^128)._ 
```C++
static void ghash_mult_bit (
    const uint8_t x,
    const uint8_t y,
    uint8_t out
) 
```





**Parameters:**


* `x` First 16-byte block. 
* `y` Second 16-byte block. 
* `out` Output 16-byte block product. 




        

<hr>



### function ghash\_mult\_reduce 

_Multiply two 128-bit polynomials in GF(2^128) via 9-mul Karatsuba and reduce in place._ 
```C++
static void ghash_mult_reduce (
    uint32_t a,
    const uint32_t b
) 
```





**Parameters:**


* `a` First 128-bit operand in reversed words; receives reduced 128-bit product. 
* `b` Second 128-bit operand in reversed words. 




        

<hr>



### function ghash\_process\_data 

_Process data buffer through GHASH using 32-bit word accumulator._ 
```C++
static void ghash_process_data (
    const SYN_AES_GCM_Context * ctx,
    const uint8_t * data,
    size_t len,
    uint32_t yw
) 
```





**Parameters:**


* `ctx` GCM context. 
* `data` Input data buffer. 
* `len` Data length in bytes. 
* `yw` In/out 4-word GHASH state accumulator. 




        

<hr>



### function ghash\_rev32 

_Bit-reverse a 32-bit word in constant time._ 
```C++
static inline uint32_t ghash_rev32 (
    uint32_t x
) 
```





**Parameters:**


* `x` Input 32-bit word. 



**Returns:**

Bit-reversed 32-bit word. 





        

<hr>



### function ghash\_step 

_One GHASH accumulation step: yw = (yw ^ block) \* H in GF(2^128)._ 
```C++
static void ghash_step (
    const SYN_AES_GCM_Context * ctx,
    const uint8_t block,
    uint32_t yw
) 
```





**Parameters:**


* `ctx` GCM context. 
* `block` 16-byte input data block. 
* `yw` In/out GHASH accumulator in reversed 32-bit words. 




        

<hr>



### function inc128 

_Increment 128-bit big-endian counter._ 
```C++
static void inc128 (
    uint8_t counter
) 
```





**Parameters:**


* `counter` 16-byte counter block to increment in place. 




        

<hr>



### function inc32 

_Increment rightmost 32 bits of 128-bit counter._ 
```C++
static void inc32 (
    uint8_t block
) 
```





**Parameters:**


* `block` 16-byte block whose last 4 bytes are incremented. 




        

<hr>



### function put\_be64 

_Write 64-bit value big-endian._ 
```C++
static void put_be64 (
    uint8_t out,
    uint64_t val
) 
```





**Parameters:**


* `out` 8-byte buffer. 
* `val` 64-bit integer. 




        

<hr>



### function syn\_secure\_zero 

_Zeroize sensitive stack buffers without compiler dead-store elimination._ 
```C++
static void syn_secure_zero (
    void * v,
    size_t n
) 
```





**Parameters:**


* `v` Buffer pointer. 
* `n` Number of bytes to zeroize. 




        

<hr>



### function syn\_secure\_zero\_u32 

_Zeroize sensitive 32-bit aligned buffers using 32-bit stores._ 
```C++
static void syn_secure_zero_u32 (
    uint32_t * v,
    size_t words
) 
```





**Parameters:**


* `v` Buffer pointer. 
* `words` Number of 32-bit words to zeroize. 




        

<hr>
## Macro Definition Documentation





### define SWAPMOVE 

_Exchange masked bit-planes between two 32-bit words separated by shift bits._ 
```C++
#define SWAPMOVE (
    a,
    b,
    mask,
    shift
) `/* multi line expression */`
```





**Parameters:**


* `a` First 32-bit word (high bit-planes). 
* `b` Second 32-bit word (low bit-planes). 
* `mask` Bitmask selecting low bit-plane positions. 
* `shift` Bit distance between swapped bit-planes. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_aes.c`

