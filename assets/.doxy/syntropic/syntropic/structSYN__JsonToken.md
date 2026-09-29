

# Struct SYN\_JsonToken



[**ClassList**](annotated.md) **>** [**SYN\_JsonToken**](structSYN__JsonToken.md)



_Parsed JSON token — key + value + type._ 

* `#include <syn_json_read.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**depth**](#variable-depth)  <br> |
|  int32\_t | [**int\_val**](#variable-int_val)  <br> |
|  const char \* | [**key**](#variable-key)  <br> |
|  [**SYN\_JsonType**](syn__json__read_8h.md#enum-syn_jsontype) | [**type**](#variable-type)  <br> |
|  const char \* | [**value**](#variable-value)  <br> |












































## Public Attributes Documentation




### variable depth 

```C++
uint8_t SYN_JsonToken::depth;
```



Nesting depth 
 


        

<hr>



### variable int\_val 

```C++
int32_t SYN_JsonToken::int_val;
```



Parsed integer (for numbers) 
 


        

<hr>



### variable key 

```C++
const char* SYN_JsonToken::key;
```



Key string (NULL for array elements) 
 


        

<hr>



### variable type 

```C++
SYN_JsonType SYN_JsonToken::type;
```



Value type 
 


        

<hr>



### variable value 

```C++
const char* SYN_JsonToken::value;
```



Value string (for string/number) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_json_read.h`

