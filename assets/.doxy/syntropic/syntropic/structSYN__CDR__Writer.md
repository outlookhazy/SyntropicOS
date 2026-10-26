

# Struct SYN\_CDR\_Writer



[**ClassList**](annotated.md) **>** [**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md)



_CDR serialization output buffer context._ 

* `#include <syn_xrce_dds.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**data**](#variable-data)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**error**](#variable-error)  <br> |
|  size\_t | [**pos**](#variable-pos)  <br> |
|  size\_t | [**size**](#variable-size)  <br> |












































## Public Attributes Documentation




### variable data 

```C++
uint8_t* SYN_CDR_Writer::data;
```



Output buffer 


        

<hr>



### variable error 

```C++
bool SYN_CDR_Writer::error;
```



Error flag on overflow 


        

<hr>



### variable pos 

```C++
size_t SYN_CDR_Writer::pos;
```



Current write offset 


        

<hr>



### variable size 

```C++
size_t SYN_CDR_Writer::size;
```



Buffer capacity 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_xrce_dds.h`

