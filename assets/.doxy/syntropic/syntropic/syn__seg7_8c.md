

# File syn\_seg7.c



[**FileList**](files.md) **>** [**display**](dir_572b98f20fe1ab62e971e24ddc7f0bdb.md) **>** [**syn\_seg7.c**](syn__seg7_8c.md)

[Go to the source code of this file](syn__seg7_8c_source.md)

_7-Segment LED Display & Multi-Digit Array Driver._ 

* `#include "syn_seg7.h"`
* `#include "../util/syn_assert.h"`
* `#include <stdio.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_seg7\_clear**](#function-syn_seg7_clear) ([**SYN\_Seg7**](structSYN__Seg7.md) \* seg) <br>_Clear display (turn off all segments)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_seg7\_init**](#function-syn_seg7_init) ([**SYN\_Seg7**](structSYN__Seg7.md) \* seg, const [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) segments, const [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) \* digits, uint8\_t num\_digits, [**SYN\_Seg7Type**](syn__seg7_8h.md#enum-syn_seg7type) type) <br>_Initialize a 7-Segment Display instance._  |
|  void | [**syn\_seg7\_print\_float**](#function-syn_seg7_print_float) ([**SYN\_Seg7**](structSYN__Seg7.md) \* seg, float val, uint8\_t decimals) <br>_Display a floating-point number with specified decimal precision._  |
|  void | [**syn\_seg7\_print\_hex**](#function-syn_seg7_print_hex) ([**SYN\_Seg7**](structSYN__Seg7.md) \* seg, uint32\_t val) <br>_Display an integer in hexadecimal format._  |
|  void | [**syn\_seg7\_print\_int**](#function-syn_seg7_print_int) ([**SYN\_Seg7**](structSYN__Seg7.md) \* seg, int32\_t val) <br>_Display an integer number._  |
|  void | [**syn\_seg7\_print\_str**](#function-syn_seg7_print_str) ([**SYN\_Seg7**](structSYN__Seg7.md) \* seg, const char \* str) <br>_Display a text string (best-effort 7-segment ASCII mapping)._  |
|  void | [**syn\_seg7\_scan**](#function-syn_seg7_scan) ([**SYN\_Seg7**](structSYN__Seg7.md) \* seg) <br>_Non-blocking multiplex scan tick. Call periodically in main/scheduler loop._  |
|  void | [**syn\_seg7\_set\_digit\_raw**](#function-syn_seg7_set_digit_raw) ([**SYN\_Seg7**](structSYN__Seg7.md) \* seg, uint8\_t digit\_idx, uint8\_t seg\_mask) <br>_Set raw segment bitmask for a specific digit._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**char\_to\_seg7**](#function-char_to_seg7) (char c) <br>_Convert ASCII character to 7-segment bitmask._  |


























## Public Functions Documentation




### function syn\_seg7\_clear 

_Clear display (turn off all segments)._ 
```C++
void syn_seg7_clear (
    SYN_Seg7 * seg
) 
```





**Parameters:**


* `seg` 7-segment context. 




        

<hr>



### function syn\_seg7\_init 

_Initialize a 7-Segment Display instance._ 
```C++
SYN_Status syn_seg7_init (
    SYN_Seg7 * seg,
    const SYN_GPIO_Pin segments,
    const SYN_GPIO_Pin * digits,
    uint8_t num_digits,
    SYN_Seg7Type type
) 
```





**Parameters:**


* `seg` 7-segment context. 
* `segments` Array of 8 segment GPIO pins (A, B, C, D, E, F, G, DP). 
* `digits` Array of digit selector GPIO pins (up to 8). 
* `num_digits` Number of digits in the array (1 to 8). 
* `type` Wiring type (Common Cathode / Common Anode). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_seg7\_print\_float 

_Display a floating-point number with specified decimal precision._ 
```C++
void syn_seg7_print_float (
    SYN_Seg7 * seg,
    float val,
    uint8_t decimals
) 
```





**Parameters:**


* `seg` 7-segment context. 
* `val` Float value to display. 
* `decimals` Number of fractional digits after decimal point. 




        

<hr>



### function syn\_seg7\_print\_hex 

_Display an integer in hexadecimal format._ 
```C++
void syn_seg7_print_hex (
    SYN_Seg7 * seg,
    uint32_t val
) 
```





**Parameters:**


* `seg` 7-segment context. 
* `val` Unsigned integer to display in hex. 




        

<hr>



### function syn\_seg7\_print\_int 

_Display an integer number._ 
```C++
void syn_seg7_print_int (
    SYN_Seg7 * seg,
    int32_t val
) 
```





**Parameters:**


* `seg` 7-segment context. 
* `val` Signed integer value to display. 




        

<hr>



### function syn\_seg7\_print\_str 

_Display a text string (best-effort 7-segment ASCII mapping)._ 
```C++
void syn_seg7_print_str (
    SYN_Seg7 * seg,
    const char * str
) 
```





**Parameters:**


* `seg` 7-segment context. 
* `str` ASCII string. 




        

<hr>



### function syn\_seg7\_scan 

_Non-blocking multiplex scan tick. Call periodically in main/scheduler loop._ 
```C++
void syn_seg7_scan (
    SYN_Seg7 * seg
) 
```





**Parameters:**


* `seg` 7-segment context. 




        

<hr>



### function syn\_seg7\_set\_digit\_raw 

_Set raw segment bitmask for a specific digit._ 
```C++
void syn_seg7_set_digit_raw (
    SYN_Seg7 * seg,
    uint8_t digit_idx,
    uint8_t seg_mask
) 
```





**Parameters:**


* `seg` 7-segment context. 
* `digit_idx` Digit index (0 to num\_digits-1). 
* `seg_mask` Raw segment bitmask (Bit 0=A, Bit 1=B, ..., Bit 7=DP). 




        

<hr>
## Public Static Functions Documentation




### function char\_to\_seg7 

_Convert ASCII character to 7-segment bitmask._ 
```C++
static uint8_t char_to_seg7 (
    char c
) 
```





**Parameters:**


* `c` ASCII character. 



**Returns:**

7-segment segment bitmask. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/display/syn_seg7.c`

