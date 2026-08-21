

# File syn\_cose.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cose.c**](syn__cose_8c.md)

[Go to the source code of this file](syn__cose_8c_source.md)

_Zero-Heap CBOR Object Signing and Encryption (COSE - RFC 9052) implementation._ 

* `#include "syntropic/proto/syn_cose.h"`
* `#include "syntropic/util/syn_random.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cose\_encrypt0\_create**](#function-syn_cose_encrypt0_create) ([**SYN\_COSE\_Algorithm**](syn__cose_8h.md#enum-syn_cose_algorithm) alg, const uint8\_t \* key, const uint8\_t \* iv, size\_t iv\_len, const uint8\_t \* kid, size\_t kid\_len, const uint8\_t \* plaintext, size\_t plaintext\_len, const uint8\_t \* external\_aad, size\_t external\_aad\_len, uint8\_t \* out\_buf, size\_t out\_buf\_size, size\_t \* out\_len) <br>_Encrypt a payload into a COSE\_Encrypt0 (Tag 16) message using ChaCha20/Poly1305._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cose\_encrypt0\_decrypt**](#function-syn_cose_encrypt0_decrypt) (const uint8\_t \* msg, size\_t msg\_len, const uint8\_t \* key, const uint8\_t \* external\_aad, size\_t external\_aad\_len, uint8\_t \* out\_plaintext, size\_t out\_plaintext\_size, size\_t \* out\_plaintext\_len, [**SYN\_COSE\_Encrypt0Message**](structSYN__COSE__Encrypt0Message.md) \* parsed\_out) <br>_Decrypt and verify a COSE\_Encrypt0 message._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cose\_key\_decode**](#function-syn_cose_key_decode) (const uint8\_t \* buf, size\_t buf\_len, [**SYN\_COSE\_Key**](structSYN__COSE__Key.md) \* key\_out) <br>_Decode a CBOR-encoded COSE\_Key map into a_ [_**SYN\_COSE\_Key**_](structSYN__COSE__Key.md) _structure._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cose\_key\_encode**](#function-syn_cose_key_encode) (const [**SYN\_COSE\_Key**](structSYN__COSE__Key.md) \* key, uint8\_t \* out\_buf, size\_t out\_buf\_size, size\_t \* out\_len) <br>_Encode a COSE\_Key structure into CBOR format._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cose\_mac0\_create**](#function-syn_cose_mac0_create) ([**SYN\_COSE\_Algorithm**](syn__cose_8h.md#enum-syn_cose_algorithm) alg, const uint8\_t \* key, size\_t key\_len, const uint8\_t \* kid, size\_t kid\_len, const uint8\_t \* payload, size\_t payload\_len, const uint8\_t \* external\_aad, size\_t external\_aad\_len, uint8\_t \* out\_buf, size\_t out\_buf\_size, size\_t \* out\_len) <br>_Create a COSE\_Mac0 (Tag 17) message using HMAC-SHA256._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cose\_mac0\_verify**](#function-syn_cose_mac0_verify) (const uint8\_t \* msg, size\_t msg\_len, const uint8\_t \* key, size\_t key\_len, const uint8\_t \* external\_aad, size\_t external\_aad\_len, [**SYN\_COSE\_Mac0Message**](structSYN__COSE__Mac0Message.md) \* parsed\_out) <br>_Verify a COSE\_Mac0 message using HMAC-SHA256._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cose\_sign1\_create**](#function-syn_cose_sign1_create) ([**SYN\_COSE\_Algorithm**](syn__cose_8h.md#enum-syn_cose_algorithm) alg, const uint8\_t \* secret\_key, const uint8\_t \* public\_key, const uint8\_t \* kid, size\_t kid\_len, const uint8\_t \* payload, size\_t payload\_len, const uint8\_t \* external\_aad, size\_t external\_aad\_len, uint8\_t \* out\_buf, size\_t out\_buf\_size, size\_t \* out\_len) <br>_Create a COSE\_Sign1 (Tag 18) message signed with EdDSA (Ed25519) or ES256 (P-256)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cose\_sign1\_verify**](#function-syn_cose_sign1_verify) (const uint8\_t \* msg, size\_t msg\_len, const uint8\_t \* public\_key, size\_t public\_key\_len, const uint8\_t \* external\_aad, size\_t external\_aad\_len, [**SYN\_COSE\_Sign1Message**](structSYN__COSE__Sign1Message.md) \* parsed\_out) <br>_Parse and verify a COSE\_Sign1 message using the signer's public key._  |




























## Public Functions Documentation




### function syn\_cose\_encrypt0\_create 

_Encrypt a payload into a COSE\_Encrypt0 (Tag 16) message using ChaCha20/Poly1305._ 
```C++
SYN_Status syn_cose_encrypt0_create (
    SYN_COSE_Algorithm alg,
    const uint8_t * key,
    const uint8_t * iv,
    size_t iv_len,
    const uint8_t * kid,
    size_t kid_len,
    const uint8_t * plaintext,
    size_t plaintext_len,
    const uint8_t * external_aad,
    size_t external_aad_len,
    uint8_t * out_buf,
    size_t out_buf_size,
    size_t * out_len
) 
```





**Parameters:**


* `alg` Encryption algorithm (SYN\_COSE\_ALGO\_CHACHA20\_POLY1305). 
* `key` Symmetric key (32 bytes). 
* `iv` Nonce / IV (12 bytes for ChaCha20/Poly1305). 
* `iv_len` IV length. 
* `kid` Optional key identifier (NULL if unused). 
* `kid_len` Key identifier length. 
* `plaintext` Plaintext payload to encrypt. 
* `plaintext_len` Plaintext length. 
* `external_aad` Optional external associated data (NULL if unused). 
* `external_aad_len` External AAD length. 
* `out_buf` [out] Output buffer for encoded COSE\_Encrypt0 message. 
* `out_buf_size` Capacity of output buffer. 
* `out_len` [out] Number of bytes written to output buffer. 



**Returns:**

SYN\_Status SYN\_OK on success, error code otherwise. 





        

<hr>



### function syn\_cose\_encrypt0\_decrypt 

_Decrypt and verify a COSE\_Encrypt0 message._ 
```C++
SYN_Status syn_cose_encrypt0_decrypt (
    const uint8_t * msg,
    size_t msg_len,
    const uint8_t * key,
    const uint8_t * external_aad,
    size_t external_aad_len,
    uint8_t * out_plaintext,
    size_t out_plaintext_size,
    size_t * out_plaintext_len,
    SYN_COSE_Encrypt0Message * parsed_out
) 
```





**Parameters:**


* `msg` Raw COSE\_Encrypt0 CBOR encoded message. 
* `msg_len` Message length. 
* `key` Symmetric key (32 bytes). 
* `external_aad` Optional external associated data (NULL if unused). 
* `external_aad_len` External AAD length. 
* `out_plaintext` [out] Output buffer for decrypted plaintext. 
* `out_plaintext_size` Output buffer capacity. 
* `out_plaintext_len` [out] Number of plaintext bytes written. 
* `parsed_out` [out] Optional parsed message details (may be NULL). 



**Returns:**

SYN\_Status SYN\_OK if decrypted and authenticated, error code otherwise. 





        

<hr>



### function syn\_cose\_key\_decode 

_Decode a CBOR-encoded COSE\_Key map into a_ [_**SYN\_COSE\_Key**_](structSYN__COSE__Key.md) _structure._
```C++
SYN_Status syn_cose_key_decode (
    const uint8_t * buf,
    size_t buf_len,
    SYN_COSE_Key * key_out
) 
```





**Parameters:**


* `buf` Input CBOR buffer. 
* `buf_len` Length of CBOR buffer. 
* `key_out` [out] Decoded key descriptor. 



**Returns:**

SYN\_Status SYN\_OK on success, error code otherwise. 





        

<hr>



### function syn\_cose\_key\_encode 

_Encode a COSE\_Key structure into CBOR format._ 
```C++
SYN_Status syn_cose_key_encode (
    const SYN_COSE_Key * key,
    uint8_t * out_buf,
    size_t out_buf_size,
    size_t * out_len
) 
```





**Parameters:**


* `key` Pointer to populated COSE\_Key descriptor. 
* `out_buf` [out] Output buffer for encoded CBOR map. 
* `out_buf_size` Capacity of output buffer. 
* `out_len` [out] Number of bytes written. 



**Returns:**

SYN\_Status SYN\_OK on success, error code otherwise. 





        

<hr>



### function syn\_cose\_mac0\_create 

_Create a COSE\_Mac0 (Tag 17) message using HMAC-SHA256._ 
```C++
SYN_Status syn_cose_mac0_create (
    SYN_COSE_Algorithm alg,
    const uint8_t * key,
    size_t key_len,
    const uint8_t * kid,
    size_t kid_len,
    const uint8_t * payload,
    size_t payload_len,
    const uint8_t * external_aad,
    size_t external_aad_len,
    uint8_t * out_buf,
    size_t out_buf_size,
    size_t * out_len
) 
```





**Parameters:**


* `alg` MAC algorithm (SYN\_COSE\_ALGO\_HMAC\_256\_256 or SYN\_COSE\_ALGO\_HMAC\_256\_64). 
* `key` MAC key. 
* `key_len` Key length in bytes. 
* `kid` Optional key identifier (NULL if unused). 
* `kid_len` Key identifier length. 
* `payload` Message payload to authenticate. 
* `payload_len` Payload length. 
* `external_aad` Optional external associated data (NULL if unused). 
* `external_aad_len` External AAD length. 
* `out_buf` [out] Output buffer for encoded COSE\_Mac0 message. 
* `out_buf_size` Capacity of output buffer. 
* `out_len` [out] Number of bytes written to output buffer. 



**Returns:**

SYN\_Status SYN\_OK on success, error code otherwise. 





        

<hr>



### function syn\_cose\_mac0\_verify 

_Verify a COSE\_Mac0 message using HMAC-SHA256._ 
```C++
SYN_Status syn_cose_mac0_verify (
    const uint8_t * msg,
    size_t msg_len,
    const uint8_t * key,
    size_t key_len,
    const uint8_t * external_aad,
    size_t external_aad_len,
    SYN_COSE_Mac0Message * parsed_out
) 
```





**Parameters:**


* `msg` Raw COSE\_Mac0 CBOR encoded message. 
* `msg_len` Message length. 
* `key` MAC key. 
* `key_len` Key length in bytes. 
* `external_aad` Optional external associated data (NULL if unused). 
* `external_aad_len` External AAD length. 
* `parsed_out` [out] Optional parsed message details (may be NULL). 



**Returns:**

SYN\_Status SYN\_OK if authentication tag is valid, error code otherwise. 





        

<hr>



### function syn\_cose\_sign1\_create 

_Create a COSE\_Sign1 (Tag 18) message signed with EdDSA (Ed25519) or ES256 (P-256)._ 
```C++
SYN_Status syn_cose_sign1_create (
    SYN_COSE_Algorithm alg,
    const uint8_t * secret_key,
    const uint8_t * public_key,
    const uint8_t * kid,
    size_t kid_len,
    const uint8_t * payload,
    size_t payload_len,
    const uint8_t * external_aad,
    size_t external_aad_len,
    uint8_t * out_buf,
    size_t out_buf_size,
    size_t * out_len
) 
```





**Parameters:**


* `alg` Signature algorithm (SYN\_COSE\_ALGO\_EDDSA or SYN\_COSE\_ALGO\_ES256). 
* `secret_key` Secret key (32 bytes for Ed25519 / P-256 scalar). 
* `public_key` Public key (32 bytes for Ed25519, 64/65 bytes for P-256). 
* `kid` Optional key identifier (NULL if unused). 
* `kid_len` Key identifier length. 
* `payload` Message payload to sign. 
* `payload_len` Payload length. 
* `external_aad` Optional external associated data (NULL if unused). 
* `external_aad_len` External AAD length. 
* `out_buf` [out] Output buffer for encoded COSE\_Sign1 message. 
* `out_buf_size` Capacity of output buffer. 
* `out_len` [out] Number of bytes written to output buffer. 



**Returns:**

SYN\_Status SYN\_OK on success, error code otherwise. 





        

<hr>



### function syn\_cose\_sign1\_verify 

_Parse and verify a COSE\_Sign1 message using the signer's public key._ 
```C++
SYN_Status syn_cose_sign1_verify (
    const uint8_t * msg,
    size_t msg_len,
    const uint8_t * public_key,
    size_t public_key_len,
    const uint8_t * external_aad,
    size_t external_aad_len,
    SYN_COSE_Sign1Message * parsed_out
) 
```





**Parameters:**


* `msg` Raw COSE\_Sign1 CBOR encoded message. 
* `msg_len` Message length. 
* `public_key` Public key of the signer. 
* `public_key_len` Public key length (32 for Ed25519, 64/65 for P-256). 
* `external_aad` Optional external associated data (NULL if unused). 
* `external_aad_len` External AAD length. 
* `parsed_out` [out] Optional parsed message details (may be NULL). 



**Returns:**

SYN\_Status SYN\_OK if verified and valid, error code otherwise. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cose.c`

