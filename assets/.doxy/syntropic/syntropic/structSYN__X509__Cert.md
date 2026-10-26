

# Struct SYN\_X509\_Cert



[**ClassList**](annotated.md) **>** [**SYN\_X509\_Cert**](structSYN__X509__Cert.md)



[More...](#detailed-description)

* `#include <syn_x509.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_ca**](#variable-is_ca)  <br> |
|  char | [**issuer\_cn**](#variable-issuer_cn)  <br> |
|  uint8\_t | [**pubkey**](#variable-pubkey)  <br> |
|  [**SYN\_X509\_Algo**](syn__x509_8h.md#enum-syn_x509_algo) | [**pubkey\_algo**](#variable-pubkey_algo)  <br> |
|  size\_t | [**pubkey\_len**](#variable-pubkey_len)  <br> |
|  const uint8\_t \* | [**serial**](#variable-serial)  <br> |
|  size\_t | [**serial\_len**](#variable-serial_len)  <br> |
|  [**SYN\_X509\_Algo**](syn__x509_8h.md#enum-syn_x509_algo) | [**sig\_algo**](#variable-sig_algo)  <br> |
|  uint8\_t | [**signature**](#variable-signature)  <br> |
|  size\_t | [**signature\_len**](#variable-signature_len)  <br> |
|  char | [**subject\_cn**](#variable-subject_cn)  <br> |
|  const uint8\_t \* | [**tbs\_bytes**](#variable-tbs_bytes)  <br> |
|  size\_t | [**tbs\_len**](#variable-tbs_len)  <br> |
|  uint32\_t | [**version**](#variable-version)  <br> |












































## Detailed Description


Parsed X.509 Certificate Data Structure. 


    
## Public Attributes Documentation




### variable is\_ca 

```C++
bool SYN_X509_Cert::is_ca;
```



BasicConstraints: cA = TRUE 


        

<hr>



### variable issuer\_cn 

```C++
char SYN_X509_Cert::issuer_cn[SYN_X509_MAX_NAME_LEN];
```



Issuer Common Name (CN) string 


        

<hr>



### variable pubkey 

```C++
uint8_t SYN_X509_Cert::pubkey[SYN_X509_PUBKEY_MAX_LEN];
```



Extracted raw public key bytes 


        

<hr>



### variable pubkey\_algo 

```C++
SYN_X509_Algo SYN_X509_Cert::pubkey_algo;
```



Subject public key algorithm 


        

<hr>



### variable pubkey\_len 

```C++
size_t SYN_X509_Cert::pubkey_len;
```



Raw public key length 


        

<hr>



### variable serial 

```C++
const uint8_t* SYN_X509_Cert::serial;
```



Serial number bytes 


        

<hr>



### variable serial\_len 

```C++
size_t SYN_X509_Cert::serial_len;
```



Serial number length 


        

<hr>



### variable sig\_algo 

```C++
SYN_X509_Algo SYN_X509_Cert::sig_algo;
```



Signature algorithm 


        

<hr>



### variable signature 

```C++
uint8_t SYN_X509_Cert::signature[SYN_X509_SIG_MAX_LEN];
```



Signature payload bytes 


        

<hr>



### variable signature\_len 

```C++
size_t SYN_X509_Cert::signature_len;
```



Signature length 


        

<hr>



### variable subject\_cn 

```C++
char SYN_X509_Cert::subject_cn[SYN_X509_MAX_NAME_LEN];
```



Subject Common Name (CN) string 


        

<hr>



### variable tbs\_bytes 

```C++
const uint8_t* SYN_X509_Cert::tbs_bytes;
```



TBSCertificate raw payload for signature checking 


        

<hr>



### variable tbs\_len 

```C++
size_t SYN_X509_Cert::tbs_len;
```



TBSCertificate length 


        

<hr>



### variable version 

```C++
uint32_t SYN_X509_Cert::version;
```



Certificate version (1, 2, or 3) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_x509.h`

