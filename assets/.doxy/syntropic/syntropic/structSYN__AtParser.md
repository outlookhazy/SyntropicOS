

# Struct SYN\_AtParser



[**ClassList**](annotated.md) **>** [**SYN\_AtParser**](structSYN__AtParser.md)



_AT Command Parser context._ 

* `#include <syn_at_parser.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**cme\_error\_code**](#variable-cme_error_code)  <br> |
|  [**SYN\_AtRespType**](syn__at__parser_8h.md#enum-syn_atresptype) | [**last\_resp**](#variable-last_resp)  <br> |
|  char \* | [**line\_buf**](#variable-line_buf)  <br> |
|  size\_t | [**line\_buf\_size**](#variable-line_buf_size)  <br> |
|  size\_t | [**line\_len**](#variable-line_len)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**prompt\_detected**](#variable-prompt_detected)  <br> |












































## Public Attributes Documentation




### variable cme\_error\_code 

```C++
int SYN_AtParser::cme_error_code;
```



Parsed CME/CMS error code (if last\_resp == SYN\_AT\_RESP\_CME\_ERROR) 


        

<hr>



### variable last\_resp 

```C++
SYN_AtRespType SYN_AtParser::last_resp;
```



Result of last decoded feed byte 


        

<hr>



### variable line\_buf 

```C++
char* SYN_AtParser::line_buf;
```



Caller-owned line buffer 


        

<hr>



### variable line\_buf\_size 

```C++
size_t SYN_AtParser::line_buf_size;
```



Capacity of line\_buf 


        

<hr>



### variable line\_len 

```C++
size_t SYN_AtParser::line_len;
```



Current accumulated line length 


        

<hr>



### variable prompt\_detected 

```C++
bool SYN_AtParser::prompt_detected;
```



true if '&gt;' prompt was encountered 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_at_parser.h`

