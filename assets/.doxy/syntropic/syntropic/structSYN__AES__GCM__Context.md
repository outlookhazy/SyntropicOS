

# Struct SYN\_AES\_GCM\_Context



[**ClassList**](annotated.md) **>** [**SYN\_AES\_GCM\_Context**](structSYN__AES__GCM__Context.md)



_AES-GCM AEAD context — wraps AES key schedule and GHASH subkey/tables._ 

* `#include <syn_aes.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_AES\_Context**](structSYN__AES__Context.md) | [**aes**](#variable-aes)  <br> |
|  uint8\_t | [**h**](#variable-h)  <br> |












































## Public Attributes Documentation




### variable aes 

```C++
SYN_AES_Context SYN_AES_GCM_Context::aes;
```



Base AES cipher context 


        

<hr>



### variable h 

```C++
uint8_t SYN_AES_GCM_Context::h[SYN_AES_BLOCK_SIZE];
```



GHASH subkey H = AES(K, 0) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_aes.h`

