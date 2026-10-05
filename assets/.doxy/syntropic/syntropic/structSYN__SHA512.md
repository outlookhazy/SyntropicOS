

# Struct SYN\_SHA512



[**ClassList**](annotated.md) **>** [**SYN\_SHA512**](structSYN__SHA512.md)



_SHA-512 / SHA-384 hash context — caller-owned._ [More...](#detailed-description)

* `#include <syn_sha512.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**buf**](#variable-buf)  <br> |
|  uint32\_t | [**buf\_len**](#variable-buf_len)  <br> |
|  uint64\_t | [**state**](#variable-state)  <br> |
|  uint64\_t | [**total\_len\_hi**](#variable-total_len_hi)  <br> |
|  uint64\_t | [**total\_len\_lo**](#variable-total_len_lo)  <br> |












































## Detailed Description


Size: 216 bytes (8×8 state + 128 buffer + 4 buf\_len + 2×8 counters). 


    
## Public Attributes Documentation




### variable buf 

```C++
uint8_t SYN_SHA512::buf[SYN_SHA512_BLOCK_SIZE];
```



Partial 128-byte block buffer 
 


        

<hr>



### variable buf\_len 

```C++
uint32_t SYN_SHA512::buf_len;
```



Bytes in buffer (0–127) 
 


        

<hr>



### variable state 

```C++
uint64_t SYN_SHA512::state[8];
```



Running 64-bit hash state (H0–H7) 


        

<hr>



### variable total\_len\_hi 

```C++
uint64_t SYN_SHA512::total_len_hi;
```



Total message length in bytes, high 


        

<hr>



### variable total\_len\_lo 

```C++
uint64_t SYN_SHA512::total_len_lo;
```



Total message length in bytes, low 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_sha512.h`

