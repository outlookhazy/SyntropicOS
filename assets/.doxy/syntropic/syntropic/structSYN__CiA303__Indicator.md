

# Struct SYN\_CiA303\_Indicator



[**ClassList**](annotated.md) **>** [**SYN\_CiA303\_Indicator**](structSYN__CiA303__Indicator.md)



_CiA 303-3 Status Indicator Handle._ 

* `#include <syn_cia303.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_LED**](structSYN__LED.md) \* | [**err\_led**](#variable-err_led)  <br> |
|  [**SYN\_CiA303\_ErrState**](syn__cia303_8h.md#enum-syn_cia303_errstate) | [**err\_state**](#variable-err_state)  <br> |
|  [**SYN\_LED**](structSYN__LED.md) \* | [**run\_led**](#variable-run_led)  <br> |
|  [**SYN\_CiA303\_RunState**](syn__cia303_8h.md#enum-syn_cia303_runstate) | [**run\_state**](#variable-run_state)  <br> |












































## Public Attributes Documentation




### variable err\_led 

```C++
SYN_LED* SYN_CiA303_Indicator::err_led;
```



Red ERROR LED instance 
 


        

<hr>



### variable err\_state 

```C++
SYN_CiA303_ErrState SYN_CiA303_Indicator::err_state;
```



Active ERR state 
 


        

<hr>



### variable run\_led 

```C++
SYN_LED* SYN_CiA303_Indicator::run_led;
```



Green RUN LED instance 
 


        

<hr>



### variable run\_state 

```C++
SYN_CiA303_RunState SYN_CiA303_Indicator::run_state;
```



Active RUN state 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia303.h`

