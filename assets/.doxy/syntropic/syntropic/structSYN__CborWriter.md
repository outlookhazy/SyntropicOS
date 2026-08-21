

# Struct SYN\_CborWriter



[**ClassList**](annotated.md) **>** [**SYN\_CborWriter**](structSYN__CborWriter.md)



_CBOR encoder state. Caller-allocated; zero heap._ 

* `#include <syn_cbor_write.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**buf**](#variable-buf)  <br> |
|  size\_t | [**cap**](#variable-cap)  <br> |
|  size\_t | [**len**](#variable-len)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**overflow**](#variable-overflow)  <br> |












































## Public Attributes Documentation




### variable buf 

```C++
uint8_t* SYN_CborWriter::buf;
```



Output buffer 
 


        

<hr>



### variable cap 

```C++
size_t SYN_CborWriter::cap;
```



Buffer capacity in bytes 
 


        

<hr>



### variable len 

```C++
size_t SYN_CborWriter::len;
```



Bytes encoded so far 
 


        

<hr>



### variable overflow 

```C++
bool SYN_CborWriter::overflow;
```



Set if buffer capacity exceeded 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_cbor_write.h`

