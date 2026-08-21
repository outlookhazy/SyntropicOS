

# File syn\_hkdf.c



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_hkdf.c**](syn__hkdf_8c.md)

[Go to the source code of this file](syn__hkdf_8c_source.md)

_HMAC-based Extract-and-Expand Key Derivation Function (HKDF-SHA256, RFC 5869)._ 

* `#include "syn_hkdf.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hkdf**](#function-syn_hkdf) (const uint8\_t \* salt, size\_t salt\_len, const uint8\_t \* ikm, size\_t ikm\_len, const uint8\_t \* info, size\_t info\_len, uint8\_t \* okm\_out, size\_t okm\_len) <br>_Complete HKDF-SHA256 (Extract then Expand)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hkdf\_expand**](#function-syn_hkdf_expand) (const uint8\_t \* prk, size\_t prk\_len, const uint8\_t \* info, size\_t info\_len, uint8\_t \* okm\_out, size\_t okm\_len) <br>_HKDF-SHA256 Expand step (RFC 5869 Section 2.3)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hkdf\_expand\_label**](#function-syn_hkdf_expand_label) (const uint8\_t \* secret, size\_t secret\_len, const char \* label, size\_t label\_len, const uint8\_t \* context, size\_t context\_len, uint8\_t \* out, size\_t out\_len) <br>_TLS 1.3 HKDF-Expand-Label with SHA-256 (RFC 8446 Section 7.1)._  |
|  void | [**syn\_hkdf\_extract**](#function-syn_hkdf_extract) (const uint8\_t \* salt, size\_t salt\_len, const uint8\_t \* ikm, size\_t ikm\_len, uint8\_t prk\_out) <br>_HKDF-SHA256 Extract step (RFC 5869 Section 2.2)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hkdf\_sha384**](#function-syn_hkdf_sha384) (const uint8\_t \* salt, size\_t salt\_len, const uint8\_t \* ikm, size\_t ikm\_len, const uint8\_t \* info, size\_t info\_len, uint8\_t \* okm\_out, size\_t okm\_len) <br>_Complete HKDF-SHA384 (Extract then Expand)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hkdf\_sha384\_expand**](#function-syn_hkdf_sha384_expand) (const uint8\_t \* prk, size\_t prk\_len, const uint8\_t \* info, size\_t info\_len, uint8\_t \* okm\_out, size\_t okm\_len) <br>_HKDF-SHA384 Expand step (RFC 5869 Section 2.3)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hkdf\_sha384\_expand\_label**](#function-syn_hkdf_sha384_expand_label) (const uint8\_t \* secret, size\_t secret\_len, const char \* label, size\_t label\_len, const uint8\_t \* context, size\_t context\_len, uint8\_t \* out, size\_t out\_len) <br>_TLS 1.3 HKDF-Expand-Label with SHA-384 (RFC 8446 Section 7.1)._  |
|  void | [**syn\_hkdf\_sha384\_extract**](#function-syn_hkdf_sha384_extract) (const uint8\_t \* salt, size\_t salt\_len, const uint8\_t \* ikm, size\_t ikm\_len, uint8\_t prk\_out) <br>_HKDF-SHA384 Extract step (RFC 5869 Section 2.2)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hkdf\_sha512**](#function-syn_hkdf_sha512) (const uint8\_t \* salt, size\_t salt\_len, const uint8\_t \* ikm, size\_t ikm\_len, const uint8\_t \* info, size\_t info\_len, uint8\_t \* okm\_out, size\_t okm\_len) <br>_Complete HKDF-SHA512 (Extract then Expand)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hkdf\_sha512\_expand**](#function-syn_hkdf_sha512_expand) (const uint8\_t \* prk, size\_t prk\_len, const uint8\_t \* info, size\_t info\_len, uint8\_t \* okm\_out, size\_t okm\_len) <br>_HKDF-SHA512 Expand step (RFC 5869 Section 2.3)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hkdf\_sha512\_expand\_label**](#function-syn_hkdf_sha512_expand_label) (const uint8\_t \* secret, size\_t secret\_len, const char \* label, size\_t label\_len, const uint8\_t \* context, size\_t context\_len, uint8\_t \* out, size\_t out\_len) <br>_TLS 1.3 HKDF-Expand-Label with SHA-512 (RFC 8446 Section 7.1)._  |
|  void | [**syn\_hkdf\_sha512\_extract**](#function-syn_hkdf_sha512_extract) (const uint8\_t \* salt, size\_t salt\_len, const uint8\_t \* ikm, size\_t ikm\_len, uint8\_t prk\_out) <br>_HKDF-SHA512 Extract step (RFC 5869 Section 2.2)._  |




























## Public Functions Documentation




### function syn\_hkdf 

_Complete HKDF-SHA256 (Extract then Expand)._ 
```C++
bool syn_hkdf (
    const uint8_t * salt,
    size_t salt_len,
    const uint8_t * ikm,
    size_t ikm_len,
    const uint8_t * info,
    size_t info_len,
    uint8_t * okm_out,
    size_t okm_len
) 
```





**Parameters:**


* `salt` Salt value (optional). 
* `salt_len` Salt length. 
* `ikm` Input keying material. 
* `ikm_len` IKM length. 
* `info` Context info (optional). 
* `info_len` Info length. 
* `okm_out` [out] Output keying material. 
* `okm_len` Desired output length. 



**Returns:**

true on success. 





        

<hr>



### function syn\_hkdf\_expand 

_HKDF-SHA256 Expand step (RFC 5869 Section 2.3)._ 
```C++
bool syn_hkdf_expand (
    const uint8_t * prk,
    size_t prk_len,
    const uint8_t * info,
    size_t info_len,
    uint8_t * okm_out,
    size_t okm_len
) 
```



OKM = HKDF-Expand(PRK, info, L)




**Parameters:**


* `prk` Pseudorandom Key (at least 32 bytes). 
* `prk_len` PRK length in bytes. 
* `info` Optional context and application specific information. 
* `info_len` Info length in bytes. 
* `okm_out` [out] Output keying material buffer. 
* `okm_len` Desired length of output keying material (max 255 \* 32 = 8160 bytes). 



**Returns:**

true on success, false on invalid parameters. 





        

<hr>



### function syn\_hkdf\_expand\_label 

_TLS 1.3 HKDF-Expand-Label with SHA-256 (RFC 8446 Section 7.1)._ 
```C++
bool syn_hkdf_expand_label (
    const uint8_t * secret,
    size_t secret_len,
    const char * label,
    size_t label_len,
    const uint8_t * context,
    size_t context_len,
    uint8_t * out,
    size_t out_len
) 
```





**Parameters:**


* `secret` Secret key (at least 32 bytes). 
* `secret_len` Secret length. 
* `label` Label string (e.g. "c hs traffic", "s hs traffic", "derived", etc.). 
* `label_len` Label length (excluding null terminator). 
* `context` Transcript hash context bytes (or NULL if 0-length). 
* `context_len` Context length (e.g. 32 bytes for SHA-256 transcript hash). 
* `out` [out] Output key buffer. 
* `out_len` Desired output key length. 



**Returns:**

true on success. 





        

<hr>



### function syn\_hkdf\_extract 

_HKDF-SHA256 Extract step (RFC 5869 Section 2.2)._ 
```C++
void syn_hkdf_extract (
    const uint8_t * salt,
    size_t salt_len,
    const uint8_t * ikm,
    size_t ikm_len,
    uint8_t prk_out
) 
```



PRK = HMAC-Hash(salt, IKM)




**Parameters:**


* `salt` Salt value (if NULL, a string of 32 zero bytes is used). 
* `salt_len` Salt length in bytes. 
* `ikm` Input keying material. 
* `ikm_len` IKM length in bytes. 
* `prk_out` [out] Output Pseudorandom Key buffer (must be at least 32 bytes). 




        

<hr>



### function syn\_hkdf\_sha384 

_Complete HKDF-SHA384 (Extract then Expand)._ 
```C++
bool syn_hkdf_sha384 (
    const uint8_t * salt,
    size_t salt_len,
    const uint8_t * ikm,
    size_t ikm_len,
    const uint8_t * info,
    size_t info_len,
    uint8_t * okm_out,
    size_t okm_len
) 
```





**Parameters:**


* `salt` Salt value (optional). 
* `salt_len` Salt length. 
* `ikm` Input keying material. 
* `ikm_len` IKM length. 
* `info` Context info (optional). 
* `info_len` Info length. 
* `okm_out` [out] Output keying material. 
* `okm_len` Desired output length. 



**Returns:**

true on success. 





        

<hr>



### function syn\_hkdf\_sha384\_expand 

_HKDF-SHA384 Expand step (RFC 5869 Section 2.3)._ 
```C++
bool syn_hkdf_sha384_expand (
    const uint8_t * prk,
    size_t prk_len,
    const uint8_t * info,
    size_t info_len,
    uint8_t * okm_out,
    size_t okm_len
) 
```





**Parameters:**


* `prk` Pseudorandom Key (at least 48 bytes). 
* `prk_len` PRK length in bytes. 
* `info` Optional context and application specific information. 
* `info_len` Info length in bytes. 
* `okm_out` [out] Output keying material buffer. 
* `okm_len` Desired length of output keying material (max 255 \* 48 = 12240 bytes). 



**Returns:**

true on success, false on invalid parameters. 





        

<hr>



### function syn\_hkdf\_sha384\_expand\_label 

_TLS 1.3 HKDF-Expand-Label with SHA-384 (RFC 8446 Section 7.1)._ 
```C++
bool syn_hkdf_sha384_expand_label (
    const uint8_t * secret,
    size_t secret_len,
    const char * label,
    size_t label_len,
    const uint8_t * context,
    size_t context_len,
    uint8_t * out,
    size_t out_len
) 
```





**Parameters:**


* `secret` Secret key (at least 48 bytes). 
* `secret_len` Secret length. 
* `label` Label string. 
* `label_len` Label length (excluding null terminator). 
* `context` Transcript hash context bytes (or NULL if 0-length). 
* `context_len` Context length (e.g. 48 bytes for SHA-384 transcript hash). 
* `out` [out] Output key buffer. 
* `out_len` Desired output key length. 



**Returns:**

true on success. 





        

<hr>



### function syn\_hkdf\_sha384\_extract 

_HKDF-SHA384 Extract step (RFC 5869 Section 2.2)._ 
```C++
void syn_hkdf_sha384_extract (
    const uint8_t * salt,
    size_t salt_len,
    const uint8_t * ikm,
    size_t ikm_len,
    uint8_t prk_out
) 
```





**Parameters:**


* `salt` Salt value (if NULL, a string of 48 zero bytes is used). 
* `salt_len` Salt length in bytes. 
* `ikm` Input keying material. 
* `ikm_len` IKM length in bytes. 
* `prk_out` [out] Output Pseudorandom Key buffer (must be at least 48 bytes). 




        

<hr>



### function syn\_hkdf\_sha512 

_Complete HKDF-SHA512 (Extract then Expand)._ 
```C++
bool syn_hkdf_sha512 (
    const uint8_t * salt,
    size_t salt_len,
    const uint8_t * ikm,
    size_t ikm_len,
    const uint8_t * info,
    size_t info_len,
    uint8_t * okm_out,
    size_t okm_len
) 
```





**Parameters:**


* `salt` Salt value (optional). 
* `salt_len` Salt length. 
* `ikm` Input keying material. 
* `ikm_len` IKM length. 
* `info` Context info (optional). 
* `info_len` Info length. 
* `okm_out` [out] Output keying material. 
* `okm_len` Desired output length. 



**Returns:**

true on success. 





        

<hr>



### function syn\_hkdf\_sha512\_expand 

_HKDF-SHA512 Expand step (RFC 5869 Section 2.3)._ 
```C++
bool syn_hkdf_sha512_expand (
    const uint8_t * prk,
    size_t prk_len,
    const uint8_t * info,
    size_t info_len,
    uint8_t * okm_out,
    size_t okm_len
) 
```





**Parameters:**


* `prk` Pseudorandom Key (at least 64 bytes). 
* `prk_len` PRK length in bytes. 
* `info` Optional context and application specific information. 
* `info_len` Info length in bytes. 
* `okm_out` [out] Output keying material buffer. 
* `okm_len` Desired length of output keying material (max 255 \* 64 = 16320 bytes). 



**Returns:**

true on success, false on invalid parameters. 





        

<hr>



### function syn\_hkdf\_sha512\_expand\_label 

_TLS 1.3 HKDF-Expand-Label with SHA-512 (RFC 8446 Section 7.1)._ 
```C++
bool syn_hkdf_sha512_expand_label (
    const uint8_t * secret,
    size_t secret_len,
    const char * label,
    size_t label_len,
    const uint8_t * context,
    size_t context_len,
    uint8_t * out,
    size_t out_len
) 
```





**Parameters:**


* `secret` Secret key (at least 64 bytes). 
* `secret_len` Secret length. 
* `label` Label string. 
* `label_len` Label length (excluding null terminator). 
* `context` Transcript hash context bytes (or NULL if 0-length). 
* `context_len` Context length. 
* `out` [out] Output key buffer. 
* `out_len` Desired output key length. 



**Returns:**

true on success. 





        

<hr>



### function syn\_hkdf\_sha512\_extract 

_HKDF-SHA512 Extract step (RFC 5869 Section 2.2)._ 
```C++
void syn_hkdf_sha512_extract (
    const uint8_t * salt,
    size_t salt_len,
    const uint8_t * ikm,
    size_t ikm_len,
    uint8_t prk_out
) 
```





**Parameters:**


* `salt` Salt value (if NULL, a string of 64 zero bytes is used). 
* `salt_len` Salt length in bytes. 
* `ikm` Input keying material. 
* `ikm_len` IKM length in bytes. 
* `prk_out` [out] Output Pseudorandom Key buffer (must be at least 64 bytes). 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_hkdf.c`

