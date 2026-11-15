

# Struct SYN\_Keypad



[**ClassList**](annotated.md) **>** [**SYN\_Keypad**](structSYN__Keypad.md)



_Matrix Keypad Context._ [More...](#detailed-description)

* `#include <syn_keypad.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  char | [**active\_key**](#variable-active_key)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**cols**](#variable-cols)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_pressed**](#variable-is_pressed)  <br> |
|  char | [**keymap**](#variable-keymap)  <br> |
|  char | [**last\_raw\_key**](#variable-last_raw_key)  <br> |
|  uint8\_t | [**num\_cols**](#variable-num_cols)  <br> |
|  uint8\_t | [**num\_rows**](#variable-num_rows)  <br> |
|  [**SYN\_KeypadCallback**](syn__keypad_8h.md#typedef-syn_keypadcallback) | [**on\_event**](#variable-on_event)  <br> |
|  uint32\_t | [**press\_count**](#variable-press_count)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**rows**](#variable-rows)  <br> |
|  void \* | [**user\_ctx**](#variable-user_ctx)  <br> |












































## Detailed Description


Keypad State Context. 


    
## Public Attributes Documentation




### variable active\_key 

```C++
char SYN_Keypad::active_key;
```



Currently pressed debounced key 


        

<hr>



### variable cols 

```C++
SYN_GPIO_Pin SYN_Keypad::cols[SYN_KEYPAD_MAX_COLS];
```



Array of column GPIO pins 


        

<hr>



### variable is\_pressed 

```C++
bool SYN_Keypad::is_pressed;
```



True if key currently down 


        

<hr>



### variable keymap 

```C++
char SYN_Keypad::keymap[SYN_KEYPAD_MAX_ROWS *SYN_KEYPAD_MAX_COLS];
```



Flat keymap character mapping 


        

<hr>



### variable last\_raw\_key 

```C++
char SYN_Keypad::last_raw_key;
```



Last scanned raw key 


        

<hr>



### variable num\_cols 

```C++
uint8_t SYN_Keypad::num_cols;
```



Number of active column pins 


        

<hr>



### variable num\_rows 

```C++
uint8_t SYN_Keypad::num_rows;
```



Number of active row pins 


        

<hr>



### variable on\_event 

```C++
SYN_KeypadCallback SYN_Keypad::on_event;
```



Key event callback function 


        

<hr>



### variable press\_count 

```C++
uint32_t SYN_Keypad::press_count;
```



Total keypress count 


        

<hr>



### variable rows 

```C++
SYN_GPIO_Pin SYN_Keypad::rows[SYN_KEYPAD_MAX_ROWS];
```



Array of row GPIO pins 


        

<hr>



### variable user\_ctx 

```C++
void* SYN_Keypad::user_ctx;
```



User callback context 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_keypad.h`

