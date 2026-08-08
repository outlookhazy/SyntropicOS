

# Struct SYN\_JsonReader



[**ClassList**](annotated.md) **>** [**SYN\_JsonReader**](structSYN__JsonReader.md)



_JSON reader — token array + parse state._ 

* `#include <syn_json_read.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**token\_count**](#variable-token_count)  <br> |
|  [**SYN\_JsonToken**](structSYN__JsonToken.md) | [**tokens**](#variable-tokens)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**valid**](#variable-valid)  <br> |












































## Public Attributes Documentation




### variable token\_count 

```C++
size_t SYN_JsonReader::token_count;
```



Number of tokens parsed 
 


        

<hr>



### variable tokens 

```C++
SYN_JsonToken SYN_JsonReader::tokens[SYN_JSON_MAX_TOKENS];
```



Parsed tokens 
 


        

<hr>



### variable valid 

```C++
bool SYN_JsonReader::valid;
```



Parse succeeded 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_json_read.h`

