

# File syn\_port\_system.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_system.h**](syn__port__system_8h.md)

[Go to the source code of this file](syn__port__system_8h_source.md)

_System-level port interface — functions the user must implement._ [More...](#detailed-description)

* `#include "../common/syn_compiler.h"`
* `#include "../common/syn_defs.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_port\_delay\_ms**](#function-syn_port_delay_ms) (uint32\_t ms) <br>_Blocking delay for the specified number of milliseconds._  |
|  void | [**syn\_port\_enter\_critical**](#function-syn_port_enter_critical) (void) <br>_Enter a critical section (disable interrupts)._  |
|  void | [**syn\_port\_exit\_critical**](#function-syn_port_exit_critical) (void) <br>_Exit a critical section (re-enable interrupts)._  |
|  uint32\_t | [**syn\_port\_get\_tick\_ms**](#function-syn_port_get_tick_ms) (void) <br>_Return the current system tick in milliseconds._  |
|  uint32\_t | [**syn\_port\_get\_tick\_us**](#function-syn_port_get_tick_us) (void) <br>_Return the current system tick in microseconds._  |
|  void | [**syn\_port\_nvic\_enable\_irq**](#function-syn_port_nvic_enable_irq) (uint8\_t irq\_num) <br>_Enable a specific interrupt line in the hardware interrupt controller._  |
|  void | [**syn\_port\_nvic\_set\_priority**](#function-syn_port_nvic_set_priority) (uint8\_t irq\_num, uint8\_t preempt\_prio, uint8\_t sub\_prio) <br>_Configure interrupt preemption and subpriority in hardware controller._  |
|  [**SYN\_NORETURN**](syn__compiler_8h.md#define-syn_noreturn) void | [**syn\_port\_system\_reset**](#function-syn_port_system_reset) (void) <br>_Perform a system reset._  |




























## Detailed Description


Provides critical-section management, a millisecond tick source, delay, interrupt priority management, and system reset. 


    
## Public Functions Documentation




### function syn\_port\_delay\_ms 

_Blocking delay for the specified number of milliseconds._ 
```C++
void syn_port_delay_ms (
    uint32_t ms
) 
```





**Parameters:**


* `ms` Delay duration in milliseconds. 




        

<hr>



### function syn\_port\_enter\_critical 

_Enter a critical section (disable interrupts)._ 
```C++
void syn_port_enter_critical (
    void
) 
```




<hr>



### function syn\_port\_exit\_critical 

_Exit a critical section (re-enable interrupts)._ 
```C++
void syn_port_exit_critical (
    void
) 
```




<hr>



### function syn\_port\_get\_tick\_ms 

_Return the current system tick in milliseconds._ 
```C++
uint32_t syn_port_get_tick_ms (
    void
) 
```





**Returns:**

Current system tick count in milliseconds. 





        

<hr>



### function syn\_port\_get\_tick\_us 

_Return the current system tick in microseconds._ 
```C++
uint32_t syn_port_get_tick_us (
    void
) 
```





**Returns:**

Current system tick count in microseconds. 





        

<hr>



### function syn\_port\_nvic\_enable\_irq 

_Enable a specific interrupt line in the hardware interrupt controller._ 
```C++
void syn_port_nvic_enable_irq (
    uint8_t irq_num
) 
```





**Parameters:**


* `irq_num` Interrupt vector number to enable. 




        

<hr>



### function syn\_port\_nvic\_set\_priority 

_Configure interrupt preemption and subpriority in hardware controller._ 
```C++
void syn_port_nvic_set_priority (
    uint8_t irq_num,
    uint8_t preempt_prio,
    uint8_t sub_prio
) 
```





**Parameters:**


* `irq_num` Interrupt vector number. 
* `preempt_prio` Preemption priority (0 = highest). 
* `sub_prio` Subpriority level (0 = highest). 




        

<hr>



### function syn\_port\_system\_reset 

_Perform a system reset._ 
```C++
SYN_NORETURN void syn_port_system_reset (
    void
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_system.h`

