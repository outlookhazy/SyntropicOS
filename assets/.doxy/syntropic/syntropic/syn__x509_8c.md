

# File syn\_x509.c



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_x509.c**](syn__x509_8c.md)

[Go to the source code of this file](syn__x509_8c_source.md)

_Zero-Heap X.509 v3 Certificate & Chain Parser and Validator implementation._ 

* `#include "syntropic/crypto/syn_x509.h"`
* `#include "syntropic/crypto/syn_ed25519.h"`
* `#include "syntropic/crypto/syn_p256.h"`
* `#include "syntropic/crypto/syn_sha256.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_x509\_parse**](#function-syn_x509_parse) (const uint8\_t \* der, size\_t der\_len, [**SYN\_X509\_Cert**](structSYN__X509__Cert.md) \* cert\_out) <br>_Parse a raw DER-encoded X.509 certificate._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_x509\_validate\_chain**](#function-syn_x509_validate_chain) (const [**SYN\_X509\_Cert**](structSYN__X509__Cert.md) \* cert, const [**SYN\_X509\_Cert**](structSYN__X509__Cert.md) \* root\_ca, const char \* expected\_cn) <br>_Validate a certificate chain back to a trusted Root CA certificate._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_x509\_verify\_signature**](#function-syn_x509_verify_signature) (const [**SYN\_X509\_Cert**](structSYN__X509__Cert.md) \* cert, const uint8\_t \* issuer\_pubkey, size\_t issuer\_pubkey\_len, [**SYN\_X509\_Algo**](syn__x509_8h.md#enum-syn_x509_algo) algo) <br>_Verify certificate signature against an issuer's public key._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_x509.c`

