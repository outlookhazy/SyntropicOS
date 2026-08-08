

# File syn\_encoder.h



[**FileList**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_encoder.h**](syn__encoder_8h.md)

[Go to the source code of this file](syn__encoder_8h_source.md)

_Quadrature rotary encoder decoder._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../drivers/syn_gpio.h"`
* `#include "../dsp/syn_signal.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Encoder**](structSYN__Encoder.md) <br>_Quadrature encoder instance — pins, position, delta._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_EncoderDir**](#enum-syn_encoderdir)  <br>_Encoder rotation direction._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  int32\_t | [**syn\_encoder\_get\_delta**](#function-syn_encoder_get_delta) ([**SYN\_Encoder**](structSYN__Encoder.md) \* enc) <br>_Get accumulated delta since last call and reset it._  |
|  void | [**syn\_encoder\_init**](#function-syn_encoder_init) ([**SYN\_Encoder**](structSYN__Encoder.md) \* enc, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin\_a, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin\_b) <br>_Initialize encoder._  |
|  void | [**syn\_encoder\_set\_stats**](#function-syn_encoder_set_stats) ([**SYN\_Encoder**](structSYN__Encoder.md) \* enc, [**SYN\_Signal**](structSYN__Signal.md) \* stats) <br>_Attach a signal statistics window for velocity tracking._  |
|  void | [**syn\_encoder\_set\_steps\_per\_detent**](#function-syn_encoder_set_steps_per_detent) ([**SYN\_Encoder**](structSYN__Encoder.md) \* enc, uint8\_t spd) <br>_Set steps-per-detent (default = 1)._  |
|  void | [**syn\_encoder\_update**](#function-syn_encoder_update) ([**SYN\_Encoder**](structSYN__Encoder.md) \* enc) <br>_Sample pins and update position._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_EncoderDir**](syn__encoder_8h.md#enum-syn_encoderdir) | [**syn\_encoder\_direction**](#function-syn_encoder_direction) (const [**SYN\_Encoder**](structSYN__Encoder.md) \* enc) <br>_Get last direction._  |
|  int32\_t | [**syn\_encoder\_position**](#function-syn_encoder_position) (const [**SYN\_Encoder**](structSYN__Encoder.md) \* enc) <br>_Get absolute position._  |
|  void | [**syn\_encoder\_set\_position**](#function-syn_encoder_set_position) ([**SYN\_Encoder**](structSYN__Encoder.md) \* enc, int32\_t pos) <br>_Set absolute position._  |


























## Detailed Description


Decodes A/B quadrature signals from rotary encoders. Supports:
* Direction detection (CW/CCW)
* Accumulated position
* Delta since last read
* Optional push-button (use syn\_button for debouncing)




Call `syn_encoder_update()` from a timer ISR or fast loop (≥4× the maximum encoder frequency).




**
**


```C++
SYN_Encoder enc;
syn_encoder_init(&enc, PIN_A, PIN_B);

// In timer ISR or fast loop:
syn_encoder_update(&enc);

// In main loop:
int32_t delta = syn_encoder_get_delta(&enc);
if (delta != 0) {
    volume += delta;
}
```
 





    
## Public Types Documentation




### enum SYN\_EncoderDir 

_Encoder rotation direction._ 
```C++
enum SYN_EncoderDir {
    SYN_ENCODER_NONE = 0,
    SYN_ENCODER_CW = 1,
    SYN_ENCODER_CCW = -1
};
```




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
## Public Static Functions Documentation




### function syn\_encoder\_direction 

_Get last direction._ 
```C++
static inline SYN_EncoderDir syn_encoder_direction (
    const SYN_Encoder * enc
) 
```





**Parameters:**


* `enc` Encoder. 



**Returns:**

Last detected direction. 





        

<hr>



### function syn\_encoder\_position 

_Get absolute position._ 
```C++
static inline int32_t syn_encoder_position (
    const SYN_Encoder * enc
) 
```





**Parameters:**


* `enc` Encoder. 



**Returns:**

Accumulated position. 





        

<hr>



### function syn\_encoder\_set\_position 

_Set absolute position._ 
```C++
static inline void syn_encoder_set_position (
    SYN_Encoder * enc,
    int32_t pos
) 
```





**Parameters:**


* `enc` Encoder. 
* `pos` New position value. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_encoder.h`

