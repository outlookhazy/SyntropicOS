

# File syn\_cia303.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cia303.h**](syn__cia303_8h.md)

[Go to the source code of this file](syn__cia303_8h_source.md)

_CANopen CiA 303-3 Indicator & Status LED Specification implementation._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../output/syn_led.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_CiA303\_Indicator**](structSYN__CiA303__Indicator.md) <br>_CiA 303-3 Status Indicator Handle._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_CiA303\_ErrState**](#enum-syn_cia303_errstate)  <br>_CiA 303-3 Red ERROR LED indicator states._  |
| enum  | [**SYN\_CiA303\_RunState**](#enum-syn_cia303_runstate)  <br>_CiA 303-3 Green RUN LED indicator states._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_cia303\_init**](#function-syn_cia303_init) ([**SYN\_CiA303\_Indicator**](structSYN__CiA303__Indicator.md) \* ind, [**SYN\_LED**](structSYN__LED.md) \* run\_led, [**SYN\_LED**](structSYN__LED.md) \* err\_led) <br>_Initialize CiA 303-3 Indicator controller._  |
|  void | [**syn\_cia303\_set\_error\_state**](#function-syn_cia303_set_error_state) ([**SYN\_CiA303\_Indicator**](structSYN__CiA303__Indicator.md) \* ind, [**SYN\_CiA303\_ErrState**](syn__cia303_8h.md#enum-syn_cia303_errstate) err\_state) <br>_Update Red ERROR LED state._  |
|  void | [**syn\_cia303\_set\_nmt\_state**](#function-syn_cia303_set_nmt_state) ([**SYN\_CiA303\_Indicator**](structSYN__CiA303__Indicator.md) \* ind, uint8\_t nmt\_state) <br>_Update RUN LED state according to CANopen NMT state._  |
|  void | [**syn\_cia303\_step**](#function-syn_cia303_step) ([**SYN\_CiA303\_Indicator**](structSYN__CiA303__Indicator.md) \* ind) <br>_Step indicator LEDs (call periodically or from main loop)._  |




























## Detailed Description


Provides standard CiA 303-3 Green "RUN" and Red "ERROR" LED indicator patterns for CANopen node status visualization, fully integrated with SyntropicOS's syn\_led controller and syn\_canopen\_mgr NMT engine. 


    
## Public Types Documentation




### enum SYN\_CiA303\_ErrState 

_CiA 303-3 Red ERROR LED indicator states._ 
```C++
enum SYN_CiA303_ErrState {
    SYN_CIA303_ERR_OFF = 0,
    SYN_CIA303_ERR_SINGLE_FLASH = 1,
    SYN_CIA303_ERR_DOUBLE_FLASH = 2,
    SYN_CIA303_ERR_TRIPLE_FLASH = 3,
    SYN_CIA303_ERR_SOLID_ON = 4
};
```




<hr>



### enum SYN\_CiA303\_RunState 

_CiA 303-3 Green RUN LED indicator states._ 
```C++
enum SYN_CiA303_RunState {
    SYN_CIA303_RUN_OFF = 0,
    SYN_CIA303_RUN_SINGLE_FLASH = 1,
    SYN_CIA303_RUN_BLINKING = 2,
    SYN_CIA303_RUN_SOLID_ON = 3
};
```




<hr>
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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia303.h`

