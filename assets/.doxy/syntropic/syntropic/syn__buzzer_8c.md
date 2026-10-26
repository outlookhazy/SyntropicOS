

# File syn\_buzzer.c



[**FileList**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_buzzer.c**](syn__buzzer_8c.md)

[Go to the source code of this file](syn__buzzer_8c_source.md)

_Non-blocking Piezo Buzzer & Tone Generator module._ 

* `#include "syn_buzzer.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_buzzer\_beep**](#function-syn_buzzer_beep) ([**SYN\_Buzzer**](structSYN__Buzzer.md) \* buz, uint32\_t freq\_hz, uint32\_t duration\_ms) <br>_Play a single tone non-blockingly._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_buzzer\_init**](#function-syn_buzzer_init) ([**SYN\_Buzzer**](structSYN__Buzzer.md) \* buz, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, [**bool**](syn__defs_8h.md#enum-bool) active\_high) <br>_Initialize a buzzer instance on a GPIO pin._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_buzzer\_is\_playing**](#function-syn_buzzer_is_playing) (const [**SYN\_Buzzer**](structSYN__Buzzer.md) \* buz) <br>_Check if buzzer is currently playing a tone or melody pattern._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_buzzer\_play\_pattern**](#function-syn_buzzer_play_pattern) ([**SYN\_Buzzer**](structSYN__Buzzer.md) \* buz, const uint16\_t \* freqs, const uint16\_t \* durs, size\_t count) <br>_Play a sequence of tones (melody/pattern) non-blockingly._  |
|  void | [**syn\_buzzer\_step**](#function-syn_buzzer_step) ([**SYN\_Buzzer**](structSYN__Buzzer.md) \* buz, uint32\_t dt\_ms) <br>_Update buzzer state machine. Call periodically in main/scheduler loop._  |
|  void | [**syn\_buzzer\_stop**](#function-syn_buzzer_stop) ([**SYN\_Buzzer**](structSYN__Buzzer.md) \* buz) <br>_Stop audio output immediately._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**set\_buzzer\_state**](#function-set_buzzer_state) ([**SYN\_Buzzer**](structSYN__Buzzer.md) \* buz, [**bool**](syn__defs_8h.md#enum-bool) state) <br>_Set hardware buzzer GPIO output state based on active polarity._  |


























## Public Functions Documentation




### function syn\_buzzer\_beep 

_Play a single tone non-blockingly._ 
```C++
SYN_Status syn_buzzer_beep (
    SYN_Buzzer * buz,
    uint32_t freq_hz,
    uint32_t duration_ms
) 
```





**Parameters:**


* `buz` Buzzer context. 
* `freq_hz` Frequency in Hz (0 = silent pause). 
* `duration_ms` Duration in milliseconds. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_buzzer\_init 

_Initialize a buzzer instance on a GPIO pin._ 
```C++
SYN_Status syn_buzzer_init (
    SYN_Buzzer * buz,
    SYN_GPIO_Pin pin,
    bool active_high
) 
```





**Parameters:**


* `buz` Buzzer context. 
* `pin` GPIO pin identifier. 
* `active_high` True if HIGH logic turns on buzzer. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_buzzer\_is\_playing 

_Check if buzzer is currently playing a tone or melody pattern._ 
```C++
bool syn_buzzer_is_playing (
    const SYN_Buzzer * buz
) 
```





**Parameters:**


* `buz` Buzzer context. 



**Returns:**

True if active. 





        

<hr>



### function syn\_buzzer\_play\_pattern 

_Play a sequence of tones (melody/pattern) non-blockingly._ 
```C++
SYN_Status syn_buzzer_play_pattern (
    SYN_Buzzer * buz,
    const uint16_t * freqs,
    const uint16_t * durs,
    size_t count
) 
```





**Parameters:**


* `buz` Buzzer context. 
* `freqs` Array of note frequencies in Hz. 
* `durs` Array of note durations in ms. 
* `count` Number of notes in the pattern. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_buzzer\_step 

_Update buzzer state machine. Call periodically in main/scheduler loop._ 
```C++
void syn_buzzer_step (
    SYN_Buzzer * buz,
    uint32_t dt_ms
) 
```





**Parameters:**


* `buz` Buzzer context. 
* `dt_ms` Milliseconds elapsed since last call. 




        

<hr>



### function syn\_buzzer\_stop 

_Stop audio output immediately._ 
```C++
void syn_buzzer_stop (
    SYN_Buzzer * buz
) 
```





**Parameters:**


* `buz` Buzzer context. 




        

<hr>
## Public Static Functions Documentation




### function set\_buzzer\_state 

_Set hardware buzzer GPIO output state based on active polarity._ 
```C++
static void set_buzzer_state (
    SYN_Buzzer * buz,
    bool state
) 
```





**Parameters:**


* `buz` Pointer to buzzer instance. 
* `state` Desired boolean output state (true = active tone, false = silence). 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/output/syn_buzzer.c`

