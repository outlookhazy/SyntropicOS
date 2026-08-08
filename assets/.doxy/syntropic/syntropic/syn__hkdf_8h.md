

# File syn\_hkdf.h



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_hkdf.h**](syn__hkdf_8h.md)

[Go to the source code of this file](syn__hkdf_8h_source.md)

_HMAC-based Extract-and-Expand Key Derivation Function (HKDF-SHA256, RFC 5869) & TLS 1.3 HKDF-Expand-Label._ 

* `#include "syntropic/util/syn_hmac.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hkdf**](#function-syn_hkdf) (const uint8\_t \* salt, size\_t salt\_len, const uint8\_t \* ikm, size\_t ikm\_len, const uint8\_t \* info, size\_t info\_len, uint8\_t \* okm\_out, size\_t okm\_len) <br>_Complete HKDF-SHA256 (Extract then Expand)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hkdf\_expand**](#function-syn_hkdf_expand) (const uint8\_t \* prk, size\_t prk\_len, const uint8\_t \* info, size\_t info\_len, uint8\_t \* okm\_out, size\_t okm\_len) <br>_HKDF-SHA256 Expand step (RFC 5869 Section 2.3)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hkdf\_expand\_label**](#function-syn_hkdf_expand_label) (const uint8\_t \* secret, size\_t secret\_len, const char \* label, size\_t label\_len, const uint8\_t \* context, size\_t context\_len, uint8\_t \* out, size\_t out\_len) <br>_TLS 1.3 HKDF-Expand-Label (RFC 8446 Section 7.1)._  |
|  void | [**syn\_hkdf\_extract**](#function-syn_hkdf_extract) (const uint8\_t \* salt, size\_t salt\_len, const uint8\_t \* ikm, size\_t ikm\_len, uint8\_t prk\_out) <br>_HKDF-SHA256 Extract step (RFC 5869 Section 2.2)._  |




























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

_TLS 1.3 HKDF-Expand-Label (RFC 8446 Section 7.1)._ 
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



Derives a key using the TLS 1.3 formatting: HKDF-Expand-Label(Secret, Label, Context, Length) = HKDF-Expand(Secret, HkdfLabel, Length)


Where HkdfLabel = struct { uint16 length = Length; opaque label&lt;7..255&gt; = "tls13 " + Label; opaque context&lt;0..255&gt; = Context; }




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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_hkdf.h`

