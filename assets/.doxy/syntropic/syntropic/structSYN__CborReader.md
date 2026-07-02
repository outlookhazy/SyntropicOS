

# Struct SYN\_CborReader



[**ClassList**](annotated.md) **>** [**SYN\_CborReader**](structSYN__CborReader.md)



_CBOR decoder state. Caller-allocated; zero heap._ 

* `#include <syn_cbor_read.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t \* | [**buf**](#variable-buf)  <br> |
|  size\_t | [**len**](#variable-len)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ok**](#variable-ok)  <br> |
|  size\_t | [**pos**](#variable-pos)  <br> |












































## Public Attributes Documentation




### variable buf 

```C++
const uint8_t* SYN_CborReader::buf;
```



Input buffer 
 


        

<hr>



### variable len 

```C++
size_t SYN_CborReader::len;
```



Buffer length 
 


        

<hr>



### variable ok 

```C++
bool SYN_CborReader::ok;
```



false after any decode error 
 


        

<hr>



### variable pos 

```C++
size_t SYN_CborReader::pos;
```



Current read position 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_cbor_read.h`

