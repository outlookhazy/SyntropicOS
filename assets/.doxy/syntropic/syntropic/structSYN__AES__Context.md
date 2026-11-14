

# Struct SYN\_AES\_Context



[**ClassList**](annotated.md) **>** [**SYN\_AES\_Context**](structSYN__AES__Context.md)



_AES context containing expanded round keys._ 

* `#include <syn_aes.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**nr**](#variable-nr)  <br> |
|  uint16\_t | [**rk\_slice**](#variable-rk_slice)  <br> |












































## Public Attributes Documentation




### variable nr 

```C++
uint8_t SYN_AES_Context::nr;
```



Number of rounds (10, 12, or 14) 


        

<hr>



### variable rk\_slice 

```C++
uint16_t SYN_AES_Context::rk_slice[SYN_AES_MAX_ROUNDS+1U][8];
```



Bitsliced round keys 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_aes.h`

