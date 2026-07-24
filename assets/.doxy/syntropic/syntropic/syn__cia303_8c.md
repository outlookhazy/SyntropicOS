

# File syn\_cia303.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cia303.c**](syn__cia303_8c.md)

[Go to the source code of this file](syn__cia303_8c_source.md)

_CANopen CiA 303-3 Indicator & Status LED implementation._ 

* `#include "syn_cia303.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_cia303\_init**](#function-syn_cia303_init) ([**SYN\_CiA303\_Indicator**](structSYN__CiA303__Indicator.md) \* ind, [**SYN\_LED**](structSYN__LED.md) \* run\_led, [**SYN\_LED**](structSYN__LED.md) \* err\_led) <br>_Initialize CiA 303-3 Indicator controller._  |
|  void | [**syn\_cia303\_set\_error\_state**](#function-syn_cia303_set_error_state) ([**SYN\_CiA303\_Indicator**](structSYN__CiA303__Indicator.md) \* ind, [**SYN\_CiA303\_ErrState**](syn__cia303_8h.md#enum-syn_cia303_errstate) err\_state) <br>_Update Red ERROR LED state._  |
|  void | [**syn\_cia303\_set\_nmt\_state**](#function-syn_cia303_set_nmt_state) ([**SYN\_CiA303\_Indicator**](structSYN__CiA303__Indicator.md) \* ind, uint8\_t nmt\_state) <br>_Update RUN LED state according to CANopen NMT state._  |
|  void | [**syn\_cia303\_step**](#function-syn_cia303_step) ([**SYN\_CiA303\_Indicator**](structSYN__CiA303__Indicator.md) \* ind) <br>_Step indicator LEDs (call periodically or from main loop)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_CANOPEN\_NMT\_OPERATIONAL**](syn__cia303_8c.md#define-syn_canopen_nmt_operational)  `0x05U`<br> |
| define  | [**SYN\_CANOPEN\_NMT\_PRE\_OPERATIONAL**](syn__cia303_8c.md#define-syn_canopen_nmt_pre_operational)  `0x7FU`<br> |
| define  | [**SYN\_CANOPEN\_NMT\_STOPPED**](syn__cia303_8c.md#define-syn_canopen_nmt_stopped)  `0x04U`<br> |

## Public Functions Documentation




### function syn\_cia303\_init 

_Initialize CiA 303-3 Indicator controller._ 
```C++
void syn_cia303_init (
    SYN_CiA303_Indicator * ind,
    SYN_LED * run_led,
    SYN_LED * err_led
) 
```





**Parameters:**


* `ind` Indicator instance. 
* `run_led` Green RUN LED instance (initialized). 
* `err_led` Red ERROR LED instance (initialized). 




        

<hr>



### function syn\_cia303\_set\_error\_state 

_Update Red ERROR LED state._ 
```C++
void syn_cia303_set_error_state (
    SYN_CiA303_Indicator * ind,
    SYN_CiA303_ErrState err_state
) 
```





**Parameters:**


* `ind` Indicator instance. 
* `err_state` CiA 303-3 Error state enum. 




        

<hr>



### function syn\_cia303\_set\_nmt\_state 

_Update RUN LED state according to CANopen NMT state._ 
```C++
void syn_cia303_set_nmt_state (
    SYN_CiA303_Indicator * ind,
    uint8_t nmt_state
) 
```





**Parameters:**


* `ind` Indicator instance. 
* `nmt_state` CANopen NMT state byte (e.g. Pre-Op, Op, Stopped). 




        

<hr>



### function syn\_cia303\_step 

_Step indicator LEDs (call periodically or from main loop)._ 
```C++
void syn_cia303_step (
    SYN_CiA303_Indicator * ind
) 
```





**Parameters:**


* `ind` Indicator instance. 




        

<hr>
## Macro Definition Documentation





### define SYN\_CANOPEN\_NMT\_OPERATIONAL 

```C++
#define SYN_CANOPEN_NMT_OPERATIONAL `0x05U`
```




<hr>



### define SYN\_CANOPEN\_NMT\_PRE\_OPERATIONAL 

```C++
#define SYN_CANOPEN_NMT_PRE_OPERATIONAL `0x7FU`
```




<hr>



### define SYN\_CANOPEN\_NMT\_STOPPED 

```C++
#define SYN_CANOPEN_NMT_STOPPED `0x04U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia303.c`

