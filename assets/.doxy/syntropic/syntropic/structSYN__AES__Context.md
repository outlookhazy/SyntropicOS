

# Struct SYN\_AES\_Context



[**ClassList**](annotated.md) **>** [**SYN\_AES\_Context**](structSYN__AES__Context.md)



_AES context containing expanded round keys._ 

* `#include <syn_aes.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**nr**](#variable-nr)  <br> |
|  uint8\_t | [**round\_keys**](#variable-round_keys)  <br> |












































## Public Attributes Documentation




### variable nr 

```C++
uint8_t SYN_AES_Context::nr;
```



Number of rounds (10, 12, or 14) 


        

<hr>



### variable round\_keys 

```C++
uint8_t SYN_AES_Context::round_keys[SYN_AES_MAX_EXPANDED_KEY];
```



Expanded round key schedule 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_aes.h`

