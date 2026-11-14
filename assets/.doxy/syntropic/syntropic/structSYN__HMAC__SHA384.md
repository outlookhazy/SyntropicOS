

# Struct SYN\_HMAC\_SHA384



[**ClassList**](annotated.md) **>** [**SYN\_HMAC\_SHA384**](structSYN__HMAC__SHA384.md)



_HMAC-SHA384 context — caller-owned._ 

* `#include <syn_sha512.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_SHA512**](structSYN__SHA512.md) | [**inner**](#variable-inner)  <br> |
|  uint8\_t | [**o\_key\_pad**](#variable-o_key_pad)  <br> |












































## Public Attributes Documentation




### variable inner 

```C++
SYN_SHA512 SYN_HMAC_SHA384::inner;
```



Inner hash context 


        

<hr>



### variable o\_key\_pad 

```C++
uint8_t SYN_HMAC_SHA384::o_key_pad[SYN_SHA512_BLOCK_SIZE];
```



Outer key pad (K ⊕ opad) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_sha512.h`

