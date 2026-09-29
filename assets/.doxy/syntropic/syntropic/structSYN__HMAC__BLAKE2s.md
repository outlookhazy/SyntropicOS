

# Struct SYN\_HMAC\_BLAKE2s



[**ClassList**](annotated.md) **>** [**SYN\_HMAC\_BLAKE2s**](structSYN__HMAC__BLAKE2s.md)



_HMAC-BLAKE2s context — caller-owned._ [More...](#detailed-description)

* `#include <syn_blake2s.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_BLAKE2s**](structSYN__BLAKE2s.md) | [**inner**](#variable-inner)  <br> |
|  uint8\_t | [**o\_key\_pad**](#variable-o_key_pad)  <br> |












































## Detailed Description


Used internally by HKDF for WireGuard key derivation. 


    
## Public Attributes Documentation




### variable inner 

```C++
SYN_BLAKE2s SYN_HMAC_BLAKE2s::inner;
```



Inner hash context 
 


        

<hr>



### variable o\_key\_pad 

```C++
uint8_t SYN_HMAC_BLAKE2s::o_key_pad[SYN_BLAKE2S_BLOCK_SIZE];
```



Outer key pad 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_blake2s.h`

