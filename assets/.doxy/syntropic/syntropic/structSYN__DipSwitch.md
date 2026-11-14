

# Struct SYN\_DipSwitch



[**ClassList**](annotated.md) **>** [**SYN\_DipSwitch**](structSYN__DipSwitch.md)



_DIP Switch Context._ 

* `#include <syn_dipswitch.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active\_low**](#variable-active_low)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**changed**](#variable-changed)  <br> |
|  uint8\_t | [**count**](#variable-count)  <br> |
|  uint32\_t | [**current\_value**](#variable-current_value)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**pins**](#variable-pins)  <br> |
|  uint32\_t | [**previous\_value**](#variable-previous_value)  <br> |












































## Public Attributes Documentation




### variable active\_low 

```C++
bool SYN_DipSwitch::active_low;
```



True if switch ON grounds input pin 


        

<hr>



### variable changed 

```C++
bool SYN_DipSwitch::changed;
```



True if value changed since last read 


        

<hr>



### variable count 

```C++
uint8_t SYN_DipSwitch::count;
```



Number of configured switch pins 


        

<hr>



### variable current\_value 

```C++
uint32_t SYN_DipSwitch::current_value;
```



Packed binary integer value 


        

<hr>



### variable pins 

```C++
SYN_GPIO_Pin SYN_DipSwitch::pins[SYN_DIPSWITCH_MAX_PINS];
```



Array of GPIO pins 


        

<hr>



### variable previous\_value 

```C++
uint32_t SYN_DipSwitch::previous_value;
```



Previously read value 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_dipswitch.h`

