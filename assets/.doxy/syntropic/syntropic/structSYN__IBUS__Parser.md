

# Struct SYN\_IBUS\_Parser



[**ClassList**](annotated.md) **>** [**SYN\_IBUS\_Parser**](structSYN__IBUS__Parser.md)



[More...](#detailed-description)

* `#include <syn_ibus.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**buf**](#variable-buf)  <br> |
|  uint32\_t | [**checksum\_errors**](#variable-checksum_errors)  <br> |
|  uint32\_t | [**frames\_received**](#variable-frames_received)  <br> |
|  uint8\_t | [**idx**](#variable-idx)  <br> |
|  [**SYN\_IBUS\_Frame**](structSYN__IBUS__Frame.md) | [**last\_frame**](#variable-last_frame)  <br> |












































## Detailed Description


IBUS Streaming Parser Instance. 


    
## Public Attributes Documentation




### variable buf 

```C++
uint8_t SYN_IBUS_Parser::buf[SYN_IBUS_FRAME_SIZE];
```



Frame assembly buffer 


        

<hr>



### variable checksum\_errors 

```C++
uint32_t SYN_IBUS_Parser::checksum_errors;
```



Count of checksum errors 


        

<hr>



### variable frames\_received 

```C++
uint32_t SYN_IBUS_Parser::frames_received;
```



Total valid IBUS frames decoded 


        

<hr>



### variable idx 

```C++
uint8_t SYN_IBUS_Parser::idx;
```



Current write index 


        

<hr>



### variable last\_frame 

```C++
SYN_IBUS_Frame SYN_IBUS_Parser::last_frame;
```



Last successfully decoded IBUS frame 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ibus.h`

