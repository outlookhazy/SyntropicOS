

# Struct SYN\_RingBuf



[**ClassList**](annotated.md) **>** [**SYN\_RingBuf**](structSYN__RingBuf.md)



_Ring buffer control structure._ [More...](#detailed-description)

* `#include <syn_ringbuf.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**buf**](#variable-buf)  <br> |
|  volatile size\_t | [**head**](#variable-head)  <br> |
|  size\_t | [**size**](#variable-size)  <br> |
|  volatile size\_t | [**tail**](#variable-tail)  <br> |












































## Detailed Description


All fields are considered private. Use the API functions below. 


    
## Public Attributes Documentation




### variable buf 

```C++
uint8_t* SYN_RingBuf::buf;
```



Pointer to caller-provided backing array 


        

<hr>



### variable head 

```C++
volatile size_t SYN_RingBuf::head;
```



Write index (producer advances this) 


        

<hr>



### variable size 

```C++
size_t SYN_RingBuf::size;
```



Total size of backing array in bytes 


        

<hr>



### variable tail 

```C++
volatile size_t SYN_RingBuf::tail;
```



Read index (consumer advances this) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_ringbuf.h`

