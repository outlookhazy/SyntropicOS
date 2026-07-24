

# Struct SYN\_NetBufPool



[**ClassList**](annotated.md) **>** [**SYN\_NetBufPool**](structSYN__NetBufPool.md)



_Packet Buffer Pool Descriptor._ 

* `#include <syn_netbuf.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**buf\_count**](#variable-buf_count)  <br> |
|  size\_t | [**buf\_size**](#variable-buf_size)  <br> |
|  [**SYN\_NetBuf**](structSYN__NetBuf.md) \* | [**bufs**](#variable-bufs)  <br> |
|  size\_t | [**free\_count**](#variable-free_count)  <br> |
|  [**SYN\_NetBuf**](structSYN__NetBuf.md) \* | [**free\_list**](#variable-free_list)  <br> |
|  uint8\_t \* | [**raw\_mem**](#variable-raw_mem)  <br> |












































## Public Attributes Documentation




### variable buf\_count 

```C++
size_t SYN_NetBufPool::buf_count;
```



Total buffer count in pool 
 


        

<hr>



### variable buf\_size 

```C++
size_t SYN_NetBufPool::buf_size;
```



Raw storage block size in bytes 
 


        

<hr>



### variable bufs 

```C++
SYN_NetBuf* SYN_NetBufPool::bufs;
```



Array of buffer descriptors 
 


        

<hr>



### variable free\_count 

```C++
size_t SYN_NetBufPool::free_count;
```



Number of currently available buffers 
 


        

<hr>



### variable free\_list 

```C++
SYN_NetBuf* SYN_NetBufPool::free_list;
```



O(1) Freelist head pointer 
 


        

<hr>



### variable raw\_mem 

```C++
uint8_t* SYN_NetBufPool::raw_mem;
```



Array of raw data storage blocks 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_netbuf.h`

