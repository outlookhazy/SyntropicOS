

# File syn\_x509.h



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_x509.h**](syn__x509_8h.md)

[Go to the source code of this file](syn__x509_8h_source.md)

_Zero-Heap X.509 v3 Certificate & Chain Parser and Validator._ 

* `#include "syntropic/crypto/syn_asn1.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_X509\_Cert**](structSYN__X509__Cert.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_X509\_Algo**](#enum-syn_x509_algo)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_x509\_parse**](#function-syn_x509_parse) (const uint8\_t \* der, size\_t der\_len, [**SYN\_X509\_Cert**](structSYN__X509__Cert.md) \* cert\_out) <br>_Parse a raw DER-encoded X.509 certificate._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_x509\_validate\_chain**](#function-syn_x509_validate_chain) (const [**SYN\_X509\_Cert**](structSYN__X509__Cert.md) \* cert, const [**SYN\_X509\_Cert**](structSYN__X509__Cert.md) \* root\_ca, const char \* expected\_cn) <br>_Validate a certificate chain back to a trusted Root CA certificate._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_x509\_verify\_signature**](#function-syn_x509_verify_signature) (const [**SYN\_X509\_Cert**](structSYN__X509__Cert.md) \* cert, const uint8\_t \* issuer\_pubkey, size\_t issuer\_pubkey\_len, [**SYN\_X509\_Algo**](syn__x509_8h.md#enum-syn_x509_algo) algo) <br>_Verify certificate signature against an issuer's public key._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_X509\_MAX\_NAME\_LEN**](syn__x509_8h.md#define-syn_x509_max_name_len)  `128U`<br>_Maximum CommonName string length (128)._  |
| define  | [**SYN\_X509\_PUBKEY\_MAX\_LEN**](syn__x509_8h.md#define-syn_x509_pubkey_max_len)  `128U`<br>_Maximum public key buffer length (128)._  |
| define  | [**SYN\_X509\_SIG\_MAX\_LEN**](syn__x509_8h.md#define-syn_x509_sig_max_len)  `256U`<br>_Maximum signature buffer length (256)._  |

## Public Types Documentation




### enum SYN\_X509\_Algo 

```C++
enum SYN_X509_Algo {
    SYN_X509_ALGO_UNKNOWN = 0,
    SYN_X509_ALGO_ED25519,
    SYN_X509_ALGO_ECDSA_P256,
    SYN_X509_ALGO_RSA_PSS
};
```



Public Key Algorithm Types in X.509 SubjectPublicKeyInfo 


        

<hr>
## Public Functions Documentation




### function syn\_x509\_parse 

_Parse a raw DER-encoded X.509 certificate._ 
```C++
bool syn_x509_parse (
    const uint8_t * der,
    size_t der_len,
    SYN_X509_Cert * cert_out
) 
```





**Parameters:**


* `der` Raw DER bytes of certificate. 
* `der_len` Length of DER bytes. 
* `cert_out` [out] Output parsed certificate struct. 



**Returns:**

true if valid X.509 certificate parsed successfully. 





        

<hr>



### function syn\_x509\_validate\_chain 

_Validate a certificate chain back to a trusted Root CA certificate._ 
```C++
bool syn_x509_validate_chain (
    const SYN_X509_Cert * cert,
    const SYN_X509_Cert * root_ca,
    const char * expected_cn
) 
```





**Parameters:**


* `cert` Leaf certificate. 
* `root_ca` Trusted Root CA certificate. 
* `expected_cn` Expected Server Name (SNI) to match against leaf CN/SAN. 



**Returns:**

true if chain is valid and trusted. 





        

<hr>



### function syn\_x509\_verify\_signature 

_Verify certificate signature against an issuer's public key._ 
```C++
bool syn_x509_verify_signature (
    const SYN_X509_Cert * cert,
    const uint8_t * issuer_pubkey,
    size_t issuer_pubkey_len,
    SYN_X509_Algo algo
) 
```





**Parameters:**


* `cert` Parsed child certificate to verify. 
* `issuer_pubkey` Issuer's public key. 
* `issuer_pubkey_len` Issuer's public key length. 
* `algo` Algorithm of issuer's public key. 



**Returns:**

true if signature is valid. 





        

<hr>
## Macro Definition Documentation





### define SYN\_X509\_MAX\_NAME\_LEN 

_Maximum CommonName string length (128)._ 
```C++
#define SYN_X509_MAX_NAME_LEN `128U`
```




<hr>



### define SYN\_X509\_PUBKEY\_MAX\_LEN 

_Maximum public key buffer length (128)._ 
```C++
#define SYN_X509_PUBKEY_MAX_LEN `128U`
```




<hr>



### define SYN\_X509\_SIG\_MAX\_LEN 

_Maximum signature buffer length (256)._ 
```C++
#define SYN_X509_SIG_MAX_LEN `256U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_x509.h`

