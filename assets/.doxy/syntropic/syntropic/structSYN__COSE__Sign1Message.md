

# Struct SYN\_COSE\_Sign1Message



[**ClassList**](annotated.md) **>** [**SYN\_COSE\_Sign1Message**](structSYN__COSE__Sign1Message.md)



_Parsed COSE\_Sign1 Message descriptor._ 

* `#include <syn_cose.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_COSE\_Algorithm**](syn__cose_8h.md#enum-syn_cose_algorithm) | [**alg**](#variable-alg)  <br> |
|  uint8\_t | [**kid**](#variable-kid)  <br> |
|  size\_t | [**kid\_len**](#variable-kid_len)  <br> |
|  const uint8\_t \* | [**payload**](#variable-payload)  <br> |
|  size\_t | [**payload\_len**](#variable-payload_len)  <br> |
|  uint8\_t | [**protected\_hdr**](#variable-protected_hdr)  <br> |
|  size\_t | [**protected\_hdr\_len**](#variable-protected_hdr_len)  <br> |
|  const uint8\_t \* | [**signature**](#variable-signature)  <br> |
|  size\_t | [**signature\_len**](#variable-signature_len)  <br> |












































## Public Attributes Documentation




### variable alg 

```C++
SYN_COSE_Algorithm SYN_COSE_Sign1Message::alg;
```



Signature algorithm 


        

<hr>



### variable kid 

```C++
uint8_t SYN_COSE_Sign1Message::kid[SYN_COSE_MAX_KID_LEN];
```



Key identifier 


        

<hr>



### variable kid\_len 

```C++
size_t SYN_COSE_Sign1Message::kid_len;
```



Key ID length 


        

<hr>



### variable payload 

```C++
const uint8_t* SYN_COSE_Sign1Message::payload;
```



Payload pointer 


        

<hr>



### variable payload\_len 

```C++
size_t SYN_COSE_Sign1Message::payload_len;
```



Payload length 


        

<hr>



### variable protected\_hdr 

```C++
uint8_t SYN_COSE_Sign1Message::protected_hdr[SYN_COSE_MAX_PROTECTED_LEN];
```



Serialized protected header 


        

<hr>



### variable protected\_hdr\_len 

```C++
size_t SYN_COSE_Sign1Message::protected_hdr_len;
```



Protected header length 


        

<hr>



### variable signature 

```C++
const uint8_t* SYN_COSE_Sign1Message::signature;
```



Signature bytes pointer 


        

<hr>



### variable signature\_len 

```C++
size_t SYN_COSE_Sign1Message::signature_len;
```



Signature length 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cose.h`

