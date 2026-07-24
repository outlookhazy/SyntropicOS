

# File syn\_port\_system.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_system.h**](syn__port__system_8h.md)

[Go to the source code of this file](syn__port__system_8h_source.md)

_System-level port interface — functions the user must implement._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../common/syn_compiler.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_port\_delay\_ms**](#function-syn_port_delay_ms) (uint32\_t ms) <br>_Blocking delay for the specified number of milliseconds._  |
|  void | [**syn\_port\_enter\_critical**](#function-syn_port_enter_critical) (void) <br>_Enter a critical section (disable interrupts)._  |
|  void | [**syn\_port\_exit\_critical**](#function-syn_port_exit_critical) (void) <br>_Exit a critical section (re-enable interrupts)._  |
|  uint32\_t | [**syn\_port\_get\_tick\_ms**](#function-syn_port_get_tick_ms) (void) <br>_Return the current system tick in milliseconds._  |
|  [**SYN\_NORETURN**](syn__compiler_8h.md#define-syn_noreturn) void | [**syn\_port\_system\_reset**](#function-syn_port_system_reset) (void) <br>_Perform a system reset._  |




























## Detailed Description


Provides critical-section management, a millisecond tick source, delay, and system reset. These are required by several SyntropicOS modules. 


    
## Public Functions Documentation




### function syn\_port\_delay\_ms 

_Blocking delay for the specified number of milliseconds._ 
```C++
void syn_port_delay_ms (
    uint32_t ms
) 
```



The implementation may busy-wait or yield to an RTOS. The delay must be at least `ms` milliseconds.




**Parameters:**


* `ms` Number of milliseconds to delay. 




        

<hr>



### function syn\_port\_enter\_critical 

_Enter a critical section (disable interrupts)._ 
```C++
void syn_port_enter_critical (
    void
) 
```



Calls may be nested; the implementation must track nesting depth and only re-enable interrupts when the outermost critical section exits. 


        

<hr>



### function syn\_port\_exit\_critical 

_Exit a critical section (re-enable interrupts)._ 
```C++
void syn_port_exit_critical (
    void
) 
```



Must be called once for each corresponding [**syn\_port\_enter\_critical()**](syn__port__system_8h.md#function-syn_port_enter_critical). 


        

<hr>



### function syn\_port\_get\_tick\_ms 

_Return the current system tick in milliseconds._ 
```C++
uint32_t syn_port_get_tick_ms (
    void
) 
```



This value must be monotonically increasing and should wrap naturally at UINT32\_MAX. Typical sources: SysTick, a hardware timer, or an RTOS tick.




**Returns:**

Milliseconds since system start (or last wrap). 





        

<hr>



### function syn\_port\_system\_reset 

_Perform a system reset._ 
```C++
SYN_NORETURN void syn_port_system_reset (
    void
) 
```



This function should not return. On Cortex-M, this is typically NVIC\_SystemReset(). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_system.h`

