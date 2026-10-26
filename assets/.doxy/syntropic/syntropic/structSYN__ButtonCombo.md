

# Struct SYN\_ButtonCombo



[**ClassList**](annotated.md) **>** [**SYN\_ButtonCombo**](structSYN__ButtonCombo.md)



_Combination button descriptor for simultaneous multi-button presses._ 

* `#include <syn_button.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  const [**SYN\_Button**](structSYN__Button.md) \*\* | [**buttons**](#variable-buttons)  <br> |
|  size\_t | [**count**](#variable-count)  <br> |
|  uint8\_t | [**events**](#variable-events)  <br> |
|  [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) | [**on\_combo**](#variable-on_combo)  <br> |
|  void \* | [**on\_combo\_ctx**](#variable-on_combo_ctx)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_ButtonCombo::active;
```



True while combination is active 
 


        

<hr>



### variable buttons 

```C++
const SYN_Button** SYN_ButtonCombo::buttons;
```



Array of pointers to monitored buttons 


        

<hr>



### variable count 

```C++
size_t SYN_ButtonCombo::count;
```



Number of buttons in combination 
 


        

<hr>



### variable events 

```C++
uint8_t SYN_ButtonCombo::events;
```



Pending events 
 


        

<hr>



### variable on\_combo 

```C++
SYN_ButtonCallback SYN_ButtonCombo::on_combo;
```



Callback when all buttons are held 
 


        

<hr>



### variable on\_combo\_ctx 

```C++
void* SYN_ButtonCombo::on_combo_ctx;
```



Callback context 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_button.h`

