

# Struct SYN\_TLS\_Config



[**ClassList**](annotated.md) **>** [**SYN\_TLS\_Config**](structSYN__TLS__Config.md)



[More...](#detailed-description)

* `#include <syn_tls.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_TLS\_CipherSuite**](syn__tls_8h.md#enum-syn_tls_ciphersuite) | [**cipher\_suite**](#variable-cipher_suite)  <br> |
|  const uint8\_t \* | [**client\_cert\_der**](#variable-client_cert_der)  <br> |
|  size\_t | [**client\_cert\_len**](#variable-client_cert_len)  <br> |
|  const uint8\_t \* | [**client\_privkey**](#variable-client_privkey)  <br> |
|  [**SYN\_TLS\_AuthMode**](syn__tls_8h.md#enum-syn_tls_authmode) | [**mode**](#variable-mode)  <br> |
|  const uint8\_t \* | [**peer\_pubkey**](#variable-peer_pubkey)  <br> |
|  size\_t | [**peer\_pubkey\_len**](#variable-peer_pubkey_len)  <br> |
|  const uint8\_t \* | [**psk\_identity**](#variable-psk_identity)  <br> |
|  size\_t | [**psk\_identity\_len**](#variable-psk_identity_len)  <br> |
|  const uint8\_t \* | [**psk\_secret**](#variable-psk_secret)  <br> |
|  size\_t | [**psk\_secret\_len**](#variable-psk_secret_len)  <br> |
|  const [**SYN\_X509\_Cert**](structSYN__X509__Cert.md) \* | [**root\_ca**](#variable-root_ca)  <br> |
|  const char \* | [**server\_name**](#variable-server_name)  <br> |












































## Detailed Description


TLS 1.3 Engine Configuration 


    
## Public Attributes Documentation




### variable cipher\_suite 

```C++
SYN_TLS_CipherSuite SYN_TLS_Config::cipher_suite;
```



Selected TLS 1.3 cipher suite 


        

<hr>



### variable client\_cert\_der 

```C++
const uint8_t* SYN_TLS_Config::client_cert_der;
```



Client certificate DER bytes (for mTLS) 


        

<hr>



### variable client\_cert\_len 

```C++
size_t SYN_TLS_Config::client_cert_len;
```



Client certificate length 


        

<hr>



### variable client\_privkey 

```C++
const uint8_t* SYN_TLS_Config::client_privkey;
```



Client private key (32 bytes) 


        

<hr>



### variable mode 

```C++
SYN_TLS_AuthMode SYN_TLS_Config::mode;
```



PSK, RPK, X.509, or mTLS 


        

<hr>



### variable peer\_pubkey 

```C++
const uint8_t* SYN_TLS_Config::peer_pubkey;
```



32-byte raw peer X25519/Ed25519 public key 


        

<hr>



### variable peer\_pubkey\_len 

```C++
size_t SYN_TLS_Config::peer_pubkey_len;
```



Raw peer public key length 


        

<hr>



### variable psk\_identity 

```C++
const uint8_t* SYN_TLS_Config::psk_identity;
```



PSK identity bytes 


        

<hr>



### variable psk\_identity\_len 

```C++
size_t SYN_TLS_Config::psk_identity_len;
```



PSK identity length 


        

<hr>



### variable psk\_secret 

```C++
const uint8_t* SYN_TLS_Config::psk_secret;
```



32-byte pre-shared secret 


        

<hr>



### variable psk\_secret\_len 

```C++
size_t SYN_TLS_Config::psk_secret_len;
```



Secret length 


        

<hr>



### variable root\_ca 

```C++
const SYN_X509_Cert* SYN_TLS_Config::root_ca;
```



Trusted Root CA cert for server validation 


        

<hr>



### variable server\_name 

```C++
const char* SYN_TLS_Config::server_name;
```



SNI hostname (optional) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_tls.h`

