

# Struct SYN\_SeqStep



[**ClassList**](annotated.md) **>** [**SYN\_SeqStep**](structSYN__SeqStep.md)



[More...](#detailed-description)

* `#include <syn_sequencer.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_SeqAction**](syn__sequencer_8h.md#typedef-syn_seqaction) | [**action**](#variable-action)  <br> |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  uint32\_t | [**delay\_ms**](#variable-delay_ms)  <br> |












































## Detailed Description


Sequence step descriptor. 


    
## Public Attributes Documentation




### variable action 

```C++
SYN_SeqAction SYN_SeqStep::action;
```



Action to execute (NULL = delay only) 


        

<hr>



### variable ctx 

```C++
void* SYN_SeqStep::ctx;
```



Context for action 
 


        

<hr>



### variable delay\_ms 

```C++
uint32_t SYN_SeqStep::delay_ms;
```



Delay AFTER action before next step 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_sequencer.h`

