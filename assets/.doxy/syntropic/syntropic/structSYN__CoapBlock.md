

# Struct SYN\_CoapBlock



[**ClassList**](annotated.md) **>** [**SYN\_CoapBlock**](structSYN__CoapBlock.md)



_CoAP Blockwise Transfer Parameter (RFC 7959)._ 

* `#include <syn_coap.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**more**](#variable-more)  <br> |
|  uint32\_t | [**num**](#variable-num)  <br> |
|  uint8\_t | [**szx**](#variable-szx)  <br> |












































## Public Attributes Documentation




### variable more 

```C++
bool SYN_CoapBlock::more;
```



More blocks follow (true/false) 
 


        

<hr>



### variable num 

```C++
uint32_t SYN_CoapBlock::num;
```



Block sequence number (0..1048575) 


        

<hr>



### variable szx 

```C++
uint8_t SYN_CoapBlock::szx;
```



Size exponent (0=16B, 1=32B, 2=64B, 3=128B, 4=256B, 5=512B, 6=1024B) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_coap.h`

