

# Struct SYN\_NetBuf



[**ClassList**](annotated.md) **>** [**SYN\_NetBuf**](structSYN__NetBuf.md)



_Zero-Copy Packet Buffer Descriptor._ 

* `#include <syn_netbuf.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**data**](#variable-data)  <br> |
|  uint8\_t \* | [**end**](#variable-end)  <br> |
|  uint16\_t | [**flags**](#variable-flags)  <br> |
|  uint8\_t \* | [**head**](#variable-head)  <br> |
|  struct [**SYN\_NetBuf**](structSYN__NetBuf.md) \* | [**next\_free**](#variable-next_free)  <br> |
|  void \* | [**pool**](#variable-pool)  <br> |
|  volatile uint16\_t | [**ref\_count**](#variable-ref_count)  <br> |
|  uint8\_t \* | [**tail**](#variable-tail)  <br> |












































## Public Attributes Documentation




### variable data 

```C++
uint8_t* SYN_NetBuf::data;
```



Pointer to current payload start 
 


        

<hr>



### variable end 

```C++
uint8_t* SYN_NetBuf::end;
```



Pointer to end of raw memory buffer 
 


        

<hr>



### variable flags 

```C++
uint16_t SYN_NetBuf::flags;
```



Internal buffer flags 
 


        

<hr>



### variable head 

```C++
uint8_t* SYN_NetBuf::head;
```



Pointer to start of allocated raw memory 


        

<hr>



### variable next\_free 

```C++
struct SYN_NetBuf* SYN_NetBuf::next_free;
```



Pointer to next free buffer in pool 


        

<hr>



### variable pool 

```C++
void* SYN_NetBuf::pool;
```



Pointer back to owning pool 
 


        

<hr>



### variable ref\_count 

```C++
volatile uint16_t SYN_NetBuf::ref_count;
```



Reference counter 
 


        

<hr>



### variable tail 

```C++
uint8_t* SYN_NetBuf::tail;
```



Pointer to end of valid payload 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_netbuf.h`

