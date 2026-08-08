

# Struct SYN\_SHA256



[**ClassList**](annotated.md) **>** [**SYN\_SHA256**](structSYN__SHA256.md)



_SHA-256 hash context — caller-owned._ [More...](#detailed-description)

* `#include <syn_sha256.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**buf**](#variable-buf)  <br> |
|  uint32\_t | [**buf\_len**](#variable-buf_len)  <br> |
|  uint32\_t | [**state**](#variable-state)  <br> |
|  uint32\_t | [**total\_len\_hi**](#variable-total_len_hi)  <br> |
|  uint32\_t | [**total\_len\_lo**](#variable-total_len_lo)  <br> |












































## Detailed Description


Typical size: 112 bytes (8×4 state + 64 buffer + 2×4 counters). 


    
## Public Attributes Documentation




### variable buf 

```C++
uint8_t SYN_SHA256::buf[SYN_SHA256_BLOCK_SIZE];
```



Partial block buffer 
 


        

<hr>



### variable buf\_len 

```C++
uint32_t SYN_SHA256::buf_len;
```



Bytes in buffer (0–63) 
 


        

<hr>



### variable state 

```C++
uint32_t SYN_SHA256::state[8];
```



Running hash state (H0–H7) 


        

<hr>



### variable total\_len\_hi 

```C++
uint32_t SYN_SHA256::total_len_hi;
```



Total message length, high 


        

<hr>



### variable total\_len\_lo 

```C++
uint32_t SYN_SHA256::total_len_lo;
```



Total message length, low 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_sha256.h`

