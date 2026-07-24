

# Struct SYN\_Sequencer



[**ClassList**](annotated.md) **>** [**SYN\_Sequencer**](structSYN__Sequencer.md)



_Sequencer runtime context._ 

* `#include <syn_sequencer.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**current**](#variable-current)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**loop**](#variable-loop)  <br> |
|  uint16\_t | [**loop\_count**](#variable-loop_count)  <br> |
|  [**SYN\_SeqCompleteCallback**](syn__sequencer_8h.md#typedef-syn_seqcompletecallback) | [**on\_complete**](#variable-on_complete)  <br> |
|  void \* | [**on\_complete\_ctx**](#variable-on_complete_ctx)  <br> |
|  [**SYN\_SeqState**](syn__sequencer_8h.md#enum-syn_seqstate) | [**state**](#variable-state)  <br> |
|  uint16\_t | [**step\_count**](#variable-step_count)  <br> |
|  const [**SYN\_SeqStep**](structSYN__SeqStep.md) \* | [**steps**](#variable-steps)  <br> |
|  uint32\_t | [**wait\_start**](#variable-wait_start)  <br> |












































## Public Attributes Documentation




### variable current 

```C++
uint16_t SYN_Sequencer::current;
```



Current step index 
 


        

<hr>



### variable loop 

```C++
bool SYN_Sequencer::loop;
```



Auto-restart on completion? 
 


        

<hr>



### variable loop\_count 

```C++
uint16_t SYN_Sequencer::loop_count;
```



Times completed (for looping) 


        

<hr>



### variable on\_complete 

```C++
SYN_SeqCompleteCallback SYN_Sequencer::on_complete;
```



Callback function when sequence finishes 


        

<hr>



### variable on\_complete\_ctx 

```C++
void* SYN_Sequencer::on_complete_ctx;
```



Context data for completion callback 


        

<hr>



### variable state 

```C++
SYN_SeqState SYN_Sequencer::state;
```



Current sequencer operational state 


        

<hr>



### variable step\_count 

```C++
uint16_t SYN_Sequencer::step_count;
```



Total step count in sequence 


        

<hr>



### variable steps 

```C++
const SYN_SeqStep* SYN_Sequencer::steps;
```



Array of step descriptors to run 


        

<hr>



### variable wait\_start 

```C++
uint32_t SYN_Sequencer::wait_start;
```



Tick when delay started 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_sequencer.h`

