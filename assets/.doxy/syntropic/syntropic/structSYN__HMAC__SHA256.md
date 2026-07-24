

# Struct SYN\_HMAC\_SHA256



[**ClassList**](annotated.md) **>** [**SYN\_HMAC\_SHA256**](structSYN__HMAC__SHA256.md)



_HMAC-SHA256 context — caller-owned._ [More...](#detailed-description)

* `#include <syn_hmac.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_SHA256**](structSYN__SHA256.md) | [**inner**](#variable-inner)  <br> |
|  uint8\_t | [**o\_key\_pad**](#variable-o_key_pad)  <br> |












































## Detailed Description


Contains the inner SHA-256 context (used during update) and the outer key pad (used during finalization). 


    
## Public Attributes Documentation




### variable inner 

```C++
SYN_SHA256 SYN_HMAC_SHA256::inner;
```



Inner hash context 
 


        

<hr>



### variable o\_key\_pad 

```C++
uint8_t SYN_HMAC_SHA256::o_key_pad[SYN_SHA256_BLOCK_SIZE];
```



Outer key pad (K ⊕ opad) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_hmac.h`

