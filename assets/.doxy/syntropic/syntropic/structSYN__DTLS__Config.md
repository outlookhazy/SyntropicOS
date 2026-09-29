

# Struct SYN\_DTLS\_Config



[**ClassList**](annotated.md) **>** [**SYN\_DTLS\_Config**](structSYN__DTLS__Config.md)



[More...](#detailed-description)

* `#include <syn_dtls.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_DTLS\_CipherSuite**](syn__dtls_8h.md#enum-syn_dtls_ciphersuite) | [**cipher\_suite**](#variable-cipher_suite)  <br> |
|  const uint8\_t \* | [**client\_cert\_der**](#variable-client_cert_der)  <br> |
|  size\_t | [**client\_cert\_len**](#variable-client_cert_len)  <br> |
|  const uint8\_t \* | [**client\_privkey**](#variable-client_privkey)  <br> |
|  [**SYN\_DTLS\_AuthMode**](syn__dtls_8h.md#enum-syn_dtls_authmode) | [**mode**](#variable-mode)  <br> |
|  const uint8\_t \* | [**peer\_pubkey**](#variable-peer_pubkey)  <br> |
|  size\_t | [**peer\_pubkey\_len**](#variable-peer_pubkey_len)  <br> |
|  const uint8\_t \* | [**psk\_identity**](#variable-psk_identity)  <br> |
|  size\_t | [**psk\_identity\_len**](#variable-psk_identity_len)  <br> |
|  const uint8\_t \* | [**psk\_secret**](#variable-psk_secret)  <br> |
|  size\_t | [**psk\_secret\_len**](#variable-psk_secret_len)  <br> |
|  const [**SYN\_X509\_Cert**](structSYN__X509__Cert.md) \* | [**root\_ca**](#variable-root_ca)  <br> |
|  const char \* | [**server\_name**](#variable-server_name)  <br> |












































## Detailed Description


DTLS 1.3 Engine Configuration 


    
## Public Attributes Documentation




### variable cipher\_suite 

```C++
SYN_DTLS_CipherSuite SYN_DTLS_Config::cipher_suite;
```



Selected cipher suite 


        

<hr>



### variable client\_cert\_der 

```C++
const uint8_t* SYN_DTLS_Config::client_cert_der;
```



Client certificate DER 


        

<hr>



### variable client\_cert\_len 

```C++
size_t SYN_DTLS_Config::client_cert_len;
```



Client certificate length 


        

<hr>



### variable client\_privkey 

```C++
const uint8_t* SYN_DTLS_Config::client_privkey;
```



Client private key (32 bytes) 


        

<hr>



### variable mode 

```C++
SYN_DTLS_AuthMode SYN_DTLS_Config::mode;
```



PSK, RPK, X.509, or mTLS 


        

<hr>



### variable peer\_pubkey 

```C++
const uint8_t* SYN_DTLS_Config::peer_pubkey;
```



32-byte raw peer public key 


        

<hr>



### variable peer\_pubkey\_len 

```C++
size_t SYN_DTLS_Config::peer_pubkey_len;
```



Peer public key length 


        

<hr>



### variable psk\_identity 

```C++
const uint8_t* SYN_DTLS_Config::psk_identity;
```



PSK identity bytes 


        

<hr>



### variable psk\_identity\_len 

```C++
size_t SYN_DTLS_Config::psk_identity_len;
```



PSK identity length 


        

<hr>



### variable psk\_secret 

```C++
const uint8_t* SYN_DTLS_Config::psk_secret;
```



Pre-shared secret bytes 


        

<hr>



### variable psk\_secret\_len 

```C++
size_t SYN_DTLS_Config::psk_secret_len;
```



Secret length 


        

<hr>



### variable root\_ca 

```C++
const SYN_X509_Cert* SYN_DTLS_Config::root_ca;
```



Trusted Root CA cert 


        

<hr>



### variable server\_name 

```C++
const char* SYN_DTLS_Config::server_name;
```



SNI hostname (optional) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dtls.h`

