

# Struct SYN\_COMP



[**ClassList**](annotated.md) **>** [**SYN\_COMP**](structSYN__COMP.md)



_Analog Comparator handle. Caller allocates; zero heap._ 

* `#include <syn_comp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**comp\_id**](#variable-comp_id)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**enabled**](#variable-enabled)  <br> |
|  [**SYN\_COMP\_InvertingInput**](syn__port__comp_8h.md#enum-syn_comp_invertinginput) | [**inv\_in**](#variable-inv_in)  <br> |












































## Public Attributes Documentation




### variable comp\_id 

```C++
uint8_t SYN_COMP::comp_id;
```



Hardware comparator index 


        

<hr>



### variable enabled 

```C++
bool SYN_COMP::enabled;
```



Enable state flag 


        

<hr>



### variable inv\_in 

```C++
SYN_COMP_InvertingInput SYN_COMP::inv_in;
```



Configured reference input 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_comp.h`

