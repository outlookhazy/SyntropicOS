

# Struct SYN\_COSE\_Encrypt0Message



[**ClassList**](annotated.md) **>** [**SYN\_COSE\_Encrypt0Message**](structSYN__COSE__Encrypt0Message.md)



_Parsed COSE\_Encrypt0 Message descriptor._ 

* `#include <syn_cose.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_COSE\_Algorithm**](syn__cose_8h.md#enum-syn_cose_algorithm) | [**alg**](#variable-alg)  <br> |
|  const uint8\_t \* | [**ciphertext**](#variable-ciphertext)  <br> |
|  size\_t | [**ciphertext\_len**](#variable-ciphertext_len)  <br> |
|  uint8\_t | [**iv**](#variable-iv)  <br> |
|  size\_t | [**iv\_len**](#variable-iv_len)  <br> |
|  uint8\_t | [**kid**](#variable-kid)  <br> |
|  size\_t | [**kid\_len**](#variable-kid_len)  <br> |
|  uint8\_t | [**protected\_hdr**](#variable-protected_hdr)  <br> |
|  size\_t | [**protected\_hdr\_len**](#variable-protected_hdr_len)  <br> |












































## Public Attributes Documentation




### variable alg 

```C++
SYN_COSE_Algorithm SYN_COSE_Encrypt0Message::alg;
```



AEAD algorithm 


        

<hr>



### variable ciphertext 

```C++
const uint8_t* SYN_COSE_Encrypt0Message::ciphertext;
```



Ciphertext + Tag pointer 


        

<hr>



### variable ciphertext\_len 

```C++
size_t SYN_COSE_Encrypt0Message::ciphertext_len;
```



Ciphertext length 


        

<hr>



### variable iv 

```C++
uint8_t SYN_COSE_Encrypt0Message::iv[SYN_COSE_MAX_IV_LEN];
```



IV / Nonce 


        

<hr>



### variable iv\_len 

```C++
size_t SYN_COSE_Encrypt0Message::iv_len;
```



IV length 


        

<hr>



### variable kid 

```C++
uint8_t SYN_COSE_Encrypt0Message::kid[SYN_COSE_MAX_KID_LEN];
```



Key identifier 


        

<hr>



### variable kid\_len 

```C++
size_t SYN_COSE_Encrypt0Message::kid_len;
```



Key ID length 


        

<hr>



### variable protected\_hdr 

```C++
uint8_t SYN_COSE_Encrypt0Message::protected_hdr[SYN_COSE_MAX_PROTECTED_LEN];
```



Serialized protected header 


        

<hr>



### variable protected\_hdr\_len 

```C++
size_t SYN_COSE_Encrypt0Message::protected_hdr_len;
```



Protected header length 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cose.h`

