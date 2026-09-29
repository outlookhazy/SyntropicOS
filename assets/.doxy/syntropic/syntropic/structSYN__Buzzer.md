

# Struct SYN\_Buzzer



[**ClassList**](annotated.md) **>** [**SYN\_Buzzer**](structSYN__Buzzer.md)



_Non-blocking piezo buzzer context._ 

* `#include <syn_buzzer.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active\_high**](#variable-active_high)  <br> |
|  uint32\_t | [**duration\_ms**](#variable-duration_ms)  <br> |
|  uint32\_t | [**elapsed\_ms**](#variable-elapsed_ms)  <br> |
|  uint32\_t | [**freq\_hz**](#variable-freq_hz)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_playing**](#variable-is_playing)  <br> |
|  size\_t | [**pattern\_count**](#variable-pattern_count)  <br> |
|  const uint16\_t \* | [**pattern\_durs**](#variable-pattern_durs)  <br> |
|  const uint16\_t \* | [**pattern\_freqs**](#variable-pattern_freqs)  <br> |
|  size\_t | [**pattern\_index**](#variable-pattern_index)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**pin**](#variable-pin)  <br> |












































## Public Attributes Documentation




### variable active\_high 

```C++
bool SYN_Buzzer::active_high;
```



True if logic HIGH activates buzzer 


        

<hr>



### variable duration\_ms 

```C++
uint32_t SYN_Buzzer::duration_ms;
```



Current tone duration in milliseconds 


        

<hr>



### variable elapsed\_ms 

```C++
uint32_t SYN_Buzzer::elapsed_ms;
```



Time elapsed for current tone 


        

<hr>



### variable freq\_hz 

```C++
uint32_t SYN_Buzzer::freq_hz;
```



Current tone frequency in Hz 


        

<hr>



### variable is\_playing 

```C++
bool SYN_Buzzer::is_playing;
```



True if currently outputting tone 


        

<hr>



### variable pattern\_count 

```C++
size_t SYN_Buzzer::pattern_count;
```



Number of notes in pattern 


        

<hr>



### variable pattern\_durs 

```C++
const uint16_t* SYN_Buzzer::pattern_durs;
```



Sequence of durations for melody/pattern 


        

<hr>



### variable pattern\_freqs 

```C++
const uint16_t* SYN_Buzzer::pattern_freqs;
```



Sequence of frequencies for melody/pattern 


        

<hr>



### variable pattern\_index 

```C++
size_t SYN_Buzzer::pattern_index;
```



Active note index 


        

<hr>



### variable pin 

```C++
SYN_GPIO_Pin SYN_Buzzer::pin;
```



GPIO Pin identifier 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/output/syn_buzzer.h`

