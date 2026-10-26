

# Struct SYN\_BLAKE2s



[**ClassList**](annotated.md) **>** [**SYN\_BLAKE2s**](structSYN__BLAKE2s.md)



_BLAKE2s hash context — caller-owned._ [More...](#detailed-description)

* `#include <syn_blake2s.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**buf**](#variable-buf)  <br> |
|  uint8\_t | [**buflen**](#variable-buflen)  <br> |
|  uint32\_t | [**h**](#variable-h)  <br> |
|  uint8\_t | [**outlen**](#variable-outlen)  <br> |
|  uint32\_t | [**t**](#variable-t)  <br> |












































## Detailed Description


Typical size: ~120 bytes on a 32-bit target. 


    
## Public Attributes Documentation




### variable buf 

```C++
uint8_t SYN_BLAKE2s::buf[SYN_BLAKE2S_BLOCK_SIZE];
```



Partial block buffer 
 


        

<hr>



### variable buflen 

```C++
uint8_t SYN_BLAKE2s::buflen;
```



Bytes in buffer (0–64) 
 


        

<hr>



### variable h 

```C++
uint32_t SYN_BLAKE2s::h[8];
```



Running hash state 
 


        

<hr>



### variable outlen 

```C++
uint8_t SYN_BLAKE2s::outlen;
```



Desired output length 
 


        

<hr>



### variable t 

```C++
uint32_t SYN_BLAKE2s::t[2];
```



Byte counter (low, high) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_blake2s.h`

