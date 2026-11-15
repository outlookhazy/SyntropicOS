

# Struct SYN\_CDR\_Reader



[**ClassList**](annotated.md) **>** [**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md)



_CDR deserialization input buffer context._ 

* `#include <syn_xrce_dds.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t \* | [**data**](#variable-data)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**error**](#variable-error)  <br> |
|  size\_t | [**pos**](#variable-pos)  <br> |
|  size\_t | [**size**](#variable-size)  <br> |












































## Public Attributes Documentation




### variable data 

```C++
const uint8_t* SYN_CDR_Reader::data;
```



Input payload buffer 


        

<hr>



### variable error 

```C++
bool SYN_CDR_Reader::error;
```



Error flag on underflow/malformed 


        

<hr>



### variable pos 

```C++
size_t SYN_CDR_Reader::pos;
```



Current read offset 


        

<hr>



### variable size 

```C++
size_t SYN_CDR_Reader::size;
```



Total payload length 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_xrce_dds.h`

