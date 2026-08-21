

# Struct SYN\_LIN\_Master



[**ClassList**](annotated.md) **>** [**SYN\_LIN\_Master**](structSYN__LIN__Master.md)



_LIN Master state machine handle._ 

* `#include <syn_lin.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**current\_slot**](#variable-current_slot)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**running**](#variable-running)  <br> |
|  size\_t | [**slot\_count**](#variable-slot_count)  <br> |
|  uint32\_t | [**slot\_timer**](#variable-slot_timer)  <br> |
|  const [**SYN\_LIN\_ScheduleSlot**](structSYN__LIN__ScheduleSlot.md) \* | [**slots**](#variable-slots)  <br> |












































## Public Attributes Documentation




### variable current\_slot 

```C++
size_t SYN_LIN_Master::current_slot;
```



Currently active slot index 


        

<hr>



### variable running 

```C++
bool SYN_LIN_Master::running;
```



Schedule table execution status 


        

<hr>



### variable slot\_count 

```C++
size_t SYN_LIN_Master::slot_count;
```



Total number of slots in schedule table 


        

<hr>



### variable slot\_timer 

```C++
uint32_t SYN_LIN_Master::slot_timer;
```



Millisecond timer accumulator 


        

<hr>



### variable slots 

```C++
const SYN_LIN_ScheduleSlot* SYN_LIN_Master::slots;
```



Pointer to schedule table array 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lin.h`

