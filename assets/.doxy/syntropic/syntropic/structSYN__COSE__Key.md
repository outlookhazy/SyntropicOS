

# Struct SYN\_COSE\_Key



[**ClassList**](annotated.md) **>** [**SYN\_COSE\_Key**](structSYN__COSE__Key.md)



_COSE\_Key representation structure (RFC 9052 / RFC 9053)._ 

* `#include <syn_cose.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_COSE\_Algorithm**](syn__cose_8h.md#enum-syn_cose_algorithm) | [**alg**](#variable-alg)  <br> |
|  [**SYN\_COSE\_Curve**](syn__cose_8h.md#enum-syn_cose_curve) | [**crv**](#variable-crv)  <br> |
|  uint8\_t | [**kid**](#variable-kid)  <br> |
|  size\_t | [**kid\_len**](#variable-kid_len)  <br> |
|  [**SYN\_COSE\_KeyType**](syn__cose_8h.md#enum-syn_cose_keytype) | [**kty**](#variable-kty)  <br> |
|  uint8\_t | [**priv\_d**](#variable-priv_d)  <br> |
|  size\_t | [**priv\_d\_len**](#variable-priv_d_len)  <br> |
|  uint8\_t | [**pub\_x**](#variable-pub_x)  <br> |
|  size\_t | [**pub\_x\_len**](#variable-pub_x_len)  <br> |
|  uint8\_t | [**pub\_y**](#variable-pub_y)  <br> |
|  size\_t | [**pub\_y\_len**](#variable-pub_y_len)  <br> |












































## Public Attributes Documentation




### variable alg 

```C++
SYN_COSE_Algorithm SYN_COSE_Key::alg;
```



Algorithm identifier (0 if unused) 


        

<hr>



### variable crv 

```C++
SYN_COSE_Curve SYN_COSE_Key::crv;
```



Curve identifier (for OKP / EC2) 


        

<hr>



### variable kid 

```C++
uint8_t SYN_COSE_Key::kid[SYN_COSE_MAX_KID_LEN];
```



Key identifier 


        

<hr>



### variable kid\_len 

```C++
size_t SYN_COSE_Key::kid_len;
```



Key ID length 


        

<hr>



### variable kty 

```C++
SYN_COSE_KeyType SYN_COSE_Key::kty;
```



Key Type (OKP, EC2, Symmetric) 


        

<hr>



### variable priv\_d 

```C++
uint8_t SYN_COSE_Key::priv_d[32];
```



Private key scalar / Symmetric key (32 bytes) 


        

<hr>



### variable priv\_d\_len 

```C++
size_t SYN_COSE_Key::priv_d_len;
```



Private / Symmetric key length 


        

<hr>



### variable pub\_x 

```C++
uint8_t SYN_COSE_Key::pub_x[32];
```



Public key / x-coordinate (32 bytes) 


        

<hr>



### variable pub\_x\_len 

```C++
size_t SYN_COSE_Key::pub_x_len;
```



Public key x length 


        

<hr>



### variable pub\_y 

```C++
uint8_t SYN_COSE_Key::pub_y[32];
```



Public key y-coordinate (32 bytes for EC2) 


        

<hr>



### variable pub\_y\_len 

```C++
size_t SYN_COSE_Key::pub_y_len;
```



Public key y length 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cose.h`

