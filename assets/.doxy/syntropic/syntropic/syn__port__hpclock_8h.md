

# File syn\_port\_hpclock.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_hpclock.h**](syn__port__hpclock_8h.md)

[Go to the source code of this file](syn__port__hpclock_8h_source.md)

_High-precision clock port interface — implement for your platform._ [More...](#detailed-description)

* `#include <stdint.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  volatile uint32\_t | [**syn\_hpclock\_msb**](#variable-syn_hpclock_msb)  <br>_Global overflow counter — incremented by the platform ISR._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**syn\_port\_hpclock\_freq\_hz**](#function-syn_port_hpclock_freq_hz) (void) <br>_Return the timer clock frequency in Hz._  |
|  volatile uint32\_t \* | [**syn\_port\_hpclock\_lsb\_ptr**](#function-syn_port_hpclock_lsb_ptr) (void) <br>_Return a pointer to the hardware timer count register._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_HPCLOCK\_OVERFLOW\_TICK**](syn__port__hpclock_8h.md#define-syn_hpclock_overflow_tick) () `([**syn\_hpclock\_msb**](syn__hpclock_8c.md#variable-syn_hpclock_msb)++)`<br>_Macro for the platform overflow ISR — one line, nothing else._  |

## Detailed Description


Provides the hardware timer counter register and overflow counter that the syn\_hpclock driver uses to construct 64-bit system-clock- precision timestamps.


### Platform implementation checklist




* Configure a free-running hardware timer at the system clock rate (or as fast as practical). For example, STM32 TIM2 in free-run mode, or AVR Timer1 with no prescaler.
* Define `syn_port_hpclock_lsb_ptr()` to return a pointer to the timer's count register.
* Enable the timer overflow interrupt and call `SYN_HPCLOCK_OVERFLOW_TICK()` from the ISR.
* Define `syn_port_hpclock_freq_hz()` to return the timer's clock frequency.
* **The overflow ISR MUST have the highest interrupt priority in the system.** The resolve algorithm assumes the overflow ISR can preempt any context that calls [**SYN\_HPCLOCK\_CAPTURE()**](syn__hpclock_8h.md#define-syn_hpclock_capture). If a higher-priority ISR captures a timestamp while the overflow ISR is pended, the resolved tick will be silently wrong by one full counter period. There is no runtime detection of this error.






**
**


```C++
// In your platform port file:
volatile uint32_t *syn_port_hpclock_lsb_ptr(void) {
    return &TIM2->CNT;
}

uint32_t syn_port_hpclock_freq_hz(void) {
    return 84000000UL;
}

// Priority 0 = highest on Cortex-M NVIC
void TIM2_IRQHandler(void) {
    SYN_HPCLOCK_OVERFLOW_TICK();
    LL_TIM_ClearFlag_UPDATE(TIM2);
}
```
 






    
## Public Attributes Documentation




### variable syn\_hpclock\_msb 

_Global overflow counter — incremented by the platform ISR._ 
```C++
volatile uint32_t syn_hpclock_msb;
```



Declared here so both the port ISR and the driver can access it. Defined in [**syn\_hpclock.c**](syn__hpclock_8c.md). 


        

<hr>
## Public Functions Documentation




### function syn\_port\_hpclock\_freq\_hz 

_Return the timer clock frequency in Hz._ 
```C++
uint32_t syn_port_hpclock_freq_hz (
    void
) 
```



Used for tick-to-nanosecond conversion. Must be a compile-time constant or a value that does not change after initialization.




**Returns:**

Timer frequency in Hz (e.g. 16000000 for 16 MHz). 





        

<hr>



### function syn\_port\_hpclock\_lsb\_ptr 

_Return a pointer to the hardware timer count register._ 
```C++
volatile uint32_t * syn_port_hpclock_lsb_ptr (
    void
) 
```



The returned pointer must be volatile and point to a memory-mapped register that increments at the rate returned by [**syn\_port\_hpclock\_freq\_hz()**](syn__port__hpclock_8h.md#function-syn_port_hpclock_freq_hz).




**Returns:**

Pointer to the timer counter register. 





        

<hr>
## Macro Definition Documentation





### define SYN\_HPCLOCK\_OVERFLOW\_TICK 

_Macro for the platform overflow ISR — one line, nothing else._ 
```C++
#define SYN_HPCLOCK_OVERFLOW_TICK (
    
) `( syn_hpclock_msb ++)`
```



Call this from your timer update/overflow interrupt handler. Do not add any other logic before this macro in the ISR. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_hpclock.h`

