

# Struct SYN\_JsonWriter



[**ClassList**](annotated.md) **>** [**SYN\_JsonWriter**](structSYN__JsonWriter.md)



_Streaming JSON writer — tracks nesting, commas, and overflow._ 

* `#include <syn_json_write.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  char \* | [**buf**](#variable-buf)  <br> |
|  size\_t | [**capacity**](#variable-capacity)  <br> |
|  uint8\_t | [**depth**](#variable-depth)  <br> |
|  size\_t | [**len**](#variable-len)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**needs\_comma**](#variable-needs_comma)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**overflow**](#variable-overflow)  <br> |












































## Public Attributes Documentation




### variable buf 

```C++
char* SYN_JsonWriter::buf;
```



Output buffer 
 


        

<hr>



### variable capacity 

```C++
size_t SYN_JsonWriter::capacity;
```



Buffer capacity 
 


        

<hr>



### variable depth 

```C++
uint8_t SYN_JsonWriter::depth;
```



Current nesting depth 
 


        

<hr>



### variable len 

```C++
size_t SYN_JsonWriter::len;
```



Bytes written so far 
 


        

<hr>



### variable needs\_comma 

```C++
bool SYN_JsonWriter::needs_comma;
```



Insert comma before next element 
 


        

<hr>



### variable overflow 

```C++
bool SYN_JsonWriter::overflow;
```



Set if buffer overflowed 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_json_write.h`

