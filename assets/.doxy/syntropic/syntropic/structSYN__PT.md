

# Struct SYN\_PT



[**ClassList**](annotated.md) **>** [**SYN\_PT**](structSYN__PT.md)



_Protothread control block._ [More...](#detailed-description)

* `#include <syn_pt.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**lc**](#variable-lc)  <br> |












































## Detailed Description


Stores the line-continuation used to resume the coroutine. Costs exactly 2 bytes of RAM. 


    
## Public Attributes Documentation




### variable lc 

```C++
uint16_t SYN_PT::lc;
```



Line continuation — stores **LINE** at last yield 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/pt/syn_pt.h`

