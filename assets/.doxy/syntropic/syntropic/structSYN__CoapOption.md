

# Struct SYN\_CoapOption



[**ClassList**](annotated.md) **>** [**SYN\_CoapOption**](structSYN__CoapOption.md)



_Single CoAP option (number + opaque value)._ 

* `#include <syn_coap.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**len**](#variable-len)  <br> |
|  uint16\_t | [**num**](#variable-num)  <br> |
|  const uint8\_t \* | [**val**](#variable-val)  <br> |












































## Public Attributes Documentation




### variable len 

```C++
size_t SYN_CoapOption::len;
```



Length of option value in bytes 
 


        

<hr>



### variable num 

```C++
uint16_t SYN_CoapOption::num;
```



Option number (e.g. COAP\_OPT\_URI\_PATH) 


        

<hr>



### variable val 

```C++
const uint8_t* SYN_CoapOption::val;
```



Pointer to option value bytes 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_coap.h`

