

# File syn\_encoder.c



[**FileList**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_encoder.c**](syn__encoder_8c.md)

[Go to the source code of this file](syn__encoder_8c_source.md)

_Quadrature rotary encoder implementation._ [More...](#detailed-description)

* `#include "syn_encoder.h"`
* `#include "../drivers/syn_gpio.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const int8\_t | [**quad\_table**](#variable-quad_table)   = `/* multi line expression */`<br>_Quadrature state transition table._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  int32\_t | [**syn\_encoder\_get\_delta**](#function-syn_encoder_get_delta) ([**SYN\_Encoder**](structSYN__Encoder.md) \* enc) <br>_Get accumulated delta since last call and reset it._  |
|  void | [**syn\_encoder\_init**](#function-syn_encoder_init) ([**SYN\_Encoder**](structSYN__Encoder.md) \* enc, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin\_a, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin\_b) <br>_Initialize encoder._  |
|  void | [**syn\_encoder\_set\_stats**](#function-syn_encoder_set_stats) ([**SYN\_Encoder**](structSYN__Encoder.md) \* enc, [**SYN\_Signal**](structSYN__Signal.md) \* stats) <br>_Attach a signal statistics window for velocity tracking._  |
|  void | [**syn\_encoder\_set\_steps\_per\_detent**](#function-syn_encoder_set_steps_per_detent) ([**SYN\_Encoder**](structSYN__Encoder.md) \* enc, uint8\_t spd) <br>_Set steps-per-detent (default = 1)._  |
|  void | [**syn\_encoder\_update**](#function-syn_encoder_update) ([**SYN\_Encoder**](structSYN__Encoder.md) \* enc) <br>_Sample pins and update position._  |




























## Detailed Description


Uses a 2-bit state machine with a lookup table for direction decoding. Handles all four quadrature transitions correctly with no missed steps. 


    
## Public Static Attributes Documentation




### variable quad\_table 

_Quadrature state transition table._ 
```C++
const int8_t quad_table[16];
```



16 entries: (old\_state &lt;&lt; 2) \| new\_state. Index = (old\_A&lt;&lt;3 \| old\_B&lt;&lt;2 \| new\_A&lt;&lt;1 \| new\_B). Value: +1 for clockwise, -1 for counter-clockwise, 0 for invalid/no-change. 


        

<hr>
## Public Functions Documentation




### function syn\_encoder\_get\_delta 

_Get accumulated delta since last call and reset it._ 
```C++
int32_t syn_encoder_get_delta (
    SYN_Encoder * enc
) 
```





**Parameters:**


* `enc` Encoder instance. 



**Returns:**

Number of steps (positive = CW, negative = CCW). 





        

<hr>



### function syn\_encoder\_init 

_Initialize encoder._ 
```C++
void syn_encoder_init (
    SYN_Encoder * enc,
    SYN_GPIO_Pin pin_a,
    SYN_GPIO_Pin pin_b
) 
```





**Parameters:**


* `enc` Encoder instance. 
* `pin_a` Channel A GPIO. 
* `pin_b` Channel B GPIO. 




        

<hr>



### function syn\_encoder\_set\_stats 

_Attach a signal statistics window for velocity tracking._ 
```C++
void syn_encoder_set_stats (
    SYN_Encoder * enc,
    SYN_Signal * stats
) 
```



Each call to [**syn\_encoder\_get\_delta()**](syn__encoder_8h.md#function-syn_encoder_get_delta) pushes the delta value into the signal window, giving you velocity min/max/mean/variance.




**Parameters:**


* `enc` Encoder. 
* `stats` Initialized [**SYN\_Signal**](structSYN__Signal.md), or NULL to detach. 




        

<hr>



### function syn\_encoder\_set\_steps\_per\_detent 

_Set steps-per-detent (default = 1)._ 
```C++
void syn_encoder_set_steps_per_detent (
    SYN_Encoder * enc,
    uint8_t spd
) 
```



Many mechanical encoders generate 2 or 4 state changes per physical detent. Setting this to 4 means `position` and `delta` only increment once per detent rather than once per state change.




**Parameters:**


* `enc` Encoder instance. 
* `spd` Steps per detent (1, 2, or 4). 




        

<hr>



### function syn\_encoder\_update 

_Sample pins and update position._ 
```C++
void syn_encoder_update (
    SYN_Encoder * enc
) 
```



Call from a timer ISR or fast polling loop. Must be called at ≥4× the encoder's maximum rotation speed.




**Parameters:**


* `enc` Encoder instance. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_encoder.c`

