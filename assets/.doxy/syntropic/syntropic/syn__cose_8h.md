

# File syn\_cose.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cose.h**](syn__cose_8h.md)

[Go to the source code of this file](syn__cose_8h_source.md)

_Zero-Heap CBOR Object Signing and Encryption (COSE - RFC 9052 / RFC 8152)._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/crypto/syn_aes.h"`
* `#include "syntropic/crypto/syn_chacha20poly1305.h"`
* `#include "syntropic/crypto/syn_ed25519.h"`
* `#include "syntropic/crypto/syn_hmac.h"`
* `#include "syntropic/crypto/syn_p256.h"`
* `#include "syntropic/crypto/syn_sha256.h"`
* `#include "syntropic/util/syn_cbor_read.h"`
* `#include "syntropic/util/syn_cbor_write.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_COSE\_Encrypt0Message**](structSYN__COSE__Encrypt0Message.md) <br>_Parsed COSE\_Encrypt0 Message descriptor._  |
| struct | [**SYN\_COSE\_Key**](structSYN__COSE__Key.md) <br>_COSE\_Key representation structure (RFC 9052 / RFC 9053)._  |
| struct | [**SYN\_COSE\_Mac0Message**](structSYN__COSE__Mac0Message.md) <br>_Parsed COSE\_Mac0 Message descriptor (Tag 17)._  |
| struct | [**SYN\_COSE\_Sign1Message**](structSYN__COSE__Sign1Message.md) <br>_Parsed COSE\_Sign1 Message descriptor._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_COSE\_Algorithm**](#enum-syn_cose_algorithm)  <br>_COSE Algorithm Identifiers._  |
| enum  | [**SYN\_COSE\_Curve**](#enum-syn_cose_curve)  <br>_COSE Elliptic Curves (crv)_  |
| enum  | [**SYN\_COSE\_KeyType**](#enum-syn_cose_keytype)  <br>_COSE Key Types (kty)_  |




















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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_COSE\_HEADER\_ALG**](syn__cose_8h.md#define-syn_cose_header_alg)  `1`<br>_Header parameter label for algorithm identifier (1)._  |
| define  | [**SYN\_COSE\_HEADER\_CONTENT\_TYPE**](syn__cose_8h.md#define-syn_cose_header_content_type)  `3`<br>_Header parameter label for content type (3)._  |
| define  | [**SYN\_COSE\_HEADER\_CRIT**](syn__cose_8h.md#define-syn_cose_header_crit)  `2`<br>_Header parameter label for criticality (2)._  |
| define  | [**SYN\_COSE\_HEADER\_IV**](syn__cose_8h.md#define-syn_cose_header_iv)  `5`<br>_Header parameter label for initialization vector / nonce (5)._  |
| define  | [**SYN\_COSE\_HEADER\_KID**](syn__cose_8h.md#define-syn_cose_header_kid)  `4`<br>_Header parameter label for key identifier (4)._  |
| define  | [**SYN\_COSE\_HEADER\_PARTIAL\_IV**](syn__cose_8h.md#define-syn_cose_header_partial_iv)  `6`<br>_Header parameter label for partial IV (6)._  |
| define  | [**SYN\_COSE\_KEY\_ALG**](syn__cose_8h.md#define-syn_cose_key_alg)  `3`<br>_Key parameter label for key algorithm identifier (3)._  |
| define  | [**SYN\_COSE\_KEY\_CRV**](syn__cose_8h.md#define-syn_cose_key_crv)  `(-1)`<br>_Key parameter label for elliptic curve identifier (-1)._  |
| define  | [**SYN\_COSE\_KEY\_D**](syn__cose_8h.md#define-syn_cose_key_d)  `(-4)`<br>_Key parameter label for private key / secret exponent (-4)._  |
| define  | [**SYN\_COSE\_KEY\_K**](syn__cose_8h.md#define-syn_cose_key_k)  `(-1)`<br>_Key parameter label for symmetric key octet sequence (-1)._  |
| define  | [**SYN\_COSE\_KEY\_KID**](syn__cose_8h.md#define-syn_cose_key_kid)  `2`<br>_Key parameter label for key identification value (2)._  |
| define  | [**SYN\_COSE\_KEY\_KTY**](syn__cose_8h.md#define-syn_cose_key_kty)  `1`<br>_Key parameter label for identification of the key type (1)._  |
| define  | [**SYN\_COSE\_KEY\_X**](syn__cose_8h.md#define-syn_cose_key_x)  `(-2)`<br>_Key parameter label for public key x-coordinate or OKP public key (-2)._  |
| define  | [**SYN\_COSE\_KEY\_Y**](syn__cose_8h.md#define-syn_cose_key_y)  `(-3)`<br>_Key parameter label for public key y-coordinate (-3)._  |
| define  | [**SYN\_COSE\_MAX\_IV\_LEN**](syn__cose_8h.md#define-syn_cose_max_iv_len)  `16U`<br>_Maximum IV length (16 bytes)._  |
| define  | [**SYN\_COSE\_MAX\_KID\_LEN**](syn__cose_8h.md#define-syn_cose_max_kid_len)  `32U`<br>_Maximum key ID length (32 bytes)._  |
| define  | [**SYN\_COSE\_MAX\_PROTECTED\_LEN**](syn__cose_8h.md#define-syn_cose_max_protected_len)  `64U`<br>_Maximum protected header byte length (64 bytes)._  |
| define  | [**SYN\_COSE\_MAX\_SIG\_LEN**](syn__cose_8h.md#define-syn_cose_max_sig_len)  `64U`<br>_Maximum signature length in bytes (64 bytes for Ed25519 / P-256)._  |
| define  | [**SYN\_COSE\_MAX\_TAG\_LEN**](syn__cose_8h.md#define-syn_cose_max_tag_len)  `32U`<br>_Maximum MAC tag length in bytes (32 bytes for HMAC-256/256)._  |

## Detailed Description


Implements COSE\_Sign1 (Tag 18) and COSE\_Encrypt0 (Tag 16) for constrained IoT nodes. Supported Cryptographic Algorithms:
* EdDSA (-8): Pure C99 Ed25519 signature scheme (RFC 8032).
* ES256 (-7): ECDSA over NIST P-256 curve with SHA-256 (RFC 9053).
* ChaCha20/Poly1305 (24): Authenticated encryption with associated data (RFC 9053). 




    
## Public Types Documentation




### enum SYN\_COSE\_Algorithm 

_COSE Algorithm Identifiers._ 
```C++
enum SYN_COSE_Algorithm {
    SYN_COSE_ALGO_UNKNOWN = 0,
    SYN_COSE_ALGO_A128GCM = 1,
    SYN_COSE_ALGO_A192GCM = 2,
    SYN_COSE_ALGO_A256GCM = 3,
    SYN_COSE_ALGO_HMAC_256_64 = 4,
    SYN_COSE_ALGO_HMAC_256_256 = 5,
    SYN_COSE_ALGO_AES_CCM_16_64_128 =
        10,
    SYN_COSE_ALGO_AES_CCM_16_64_256 =
        11,
    SYN_COSE_ALGO_CHACHA20_POLY1305 = 24,
    SYN_COSE_ALGO_AES_CCM_16_128_128 =
        30,
    SYN_COSE_ALGO_AES_CCM_16_128_256 =
        31,
    SYN_COSE_ALGO_ES256 = -7,
    SYN_COSE_ALGO_EDDSA = -8
};
```




<hr>



### enum SYN\_COSE\_Curve 

_COSE Elliptic Curves (crv)_ 
```C++
enum SYN_COSE_Curve {
    SYN_COSE_CRV_RESERVED = 0,
    SYN_COSE_CRV_P256 = 1,
    SYN_COSE_CRV_ED25519 = 6
};
```




<hr>



### enum SYN\_COSE\_KeyType 

_COSE Key Types (kty)_ 
```C++
enum SYN_COSE_KeyType {
    SYN_COSE_KTY_RESERVED = 0,
    SYN_COSE_KTY_OKP = 1,
    SYN_COSE_KTY_EC2 = 2,
    SYN_COSE_KTY_SYMMETRIC = 4
};
```




<hr>
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
## Macro Definition Documentation





### define SYN\_COSE\_HEADER\_ALG 

_Header parameter label for algorithm identifier (1)._ 
```C++
#define SYN_COSE_HEADER_ALG `1`
```




<hr>



### define SYN\_COSE\_HEADER\_CONTENT\_TYPE 

_Header parameter label for content type (3)._ 
```C++
#define SYN_COSE_HEADER_CONTENT_TYPE `3`
```




<hr>



### define SYN\_COSE\_HEADER\_CRIT 

_Header parameter label for criticality (2)._ 
```C++
#define SYN_COSE_HEADER_CRIT `2`
```




<hr>



### define SYN\_COSE\_HEADER\_IV 

_Header parameter label for initialization vector / nonce (5)._ 
```C++
#define SYN_COSE_HEADER_IV `5`
```




<hr>



### define SYN\_COSE\_HEADER\_KID 

_Header parameter label for key identifier (4)._ 
```C++
#define SYN_COSE_HEADER_KID `4`
```




<hr>



### define SYN\_COSE\_HEADER\_PARTIAL\_IV 

_Header parameter label for partial IV (6)._ 
```C++
#define SYN_COSE_HEADER_PARTIAL_IV `6`
```




<hr>



### define SYN\_COSE\_KEY\_ALG 

_Key parameter label for key algorithm identifier (3)._ 
```C++
#define SYN_COSE_KEY_ALG `3`
```




<hr>



### define SYN\_COSE\_KEY\_CRV 

_Key parameter label for elliptic curve identifier (-1)._ 
```C++
#define SYN_COSE_KEY_CRV `(-1)`
```




<hr>



### define SYN\_COSE\_KEY\_D 

_Key parameter label for private key / secret exponent (-4)._ 
```C++
#define SYN_COSE_KEY_D `(-4)`
```




<hr>



### define SYN\_COSE\_KEY\_K 

_Key parameter label for symmetric key octet sequence (-1)._ 
```C++
#define SYN_COSE_KEY_K `(-1)`
```




<hr>



### define SYN\_COSE\_KEY\_KID 

_Key parameter label for key identification value (2)._ 
```C++
#define SYN_COSE_KEY_KID `2`
```




<hr>



### define SYN\_COSE\_KEY\_KTY 

_Key parameter label for identification of the key type (1)._ 
```C++
#define SYN_COSE_KEY_KTY `1`
```




<hr>



### define SYN\_COSE\_KEY\_X 

_Key parameter label for public key x-coordinate or OKP public key (-2)._ 
```C++
#define SYN_COSE_KEY_X `(-2)`
```




<hr>



### define SYN\_COSE\_KEY\_Y 

_Key parameter label for public key y-coordinate (-3)._ 
```C++
#define SYN_COSE_KEY_Y `(-3)`
```




<hr>



### define SYN\_COSE\_MAX\_IV\_LEN 

_Maximum IV length (16 bytes)._ 
```C++
#define SYN_COSE_MAX_IV_LEN `16U`
```




<hr>



### define SYN\_COSE\_MAX\_KID\_LEN 

_Maximum key ID length (32 bytes)._ 
```C++
#define SYN_COSE_MAX_KID_LEN `32U`
```




<hr>



### define SYN\_COSE\_MAX\_PROTECTED\_LEN 

_Maximum protected header byte length (64 bytes)._ 
```C++
#define SYN_COSE_MAX_PROTECTED_LEN `64U`
```




<hr>



### define SYN\_COSE\_MAX\_SIG\_LEN 

_Maximum signature length in bytes (64 bytes for Ed25519 / P-256)._ 
```C++
#define SYN_COSE_MAX_SIG_LEN `64U`
```




<hr>



### define SYN\_COSE\_MAX\_TAG\_LEN 

_Maximum MAC tag length in bytes (32 bytes for HMAC-256/256)._ 
```C++
#define SYN_COSE_MAX_TAG_LEN `32U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cose.h`

