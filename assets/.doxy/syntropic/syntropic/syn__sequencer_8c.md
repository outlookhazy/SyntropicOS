

# File syn\_sequencer.c



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_sequencer.c**](syn__sequencer_8c.md)

[Go to the source code of this file](syn__sequencer_8c_source.md)

_Timed action sequencer implementation._ 

* `#include "syn_sequencer.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_seq\_init**](#function-syn_seq_init) ([**SYN\_Sequencer**](structSYN__Sequencer.md) \* seq, const [**SYN\_SeqStep**](structSYN__SeqStep.md) \* steps, uint16\_t count) <br>_Initialize a sequencer._  |
|  void | [**syn\_seq\_on\_complete**](#function-syn_seq_on_complete) ([**SYN\_Sequencer**](structSYN__Sequencer.md) \* seq, [**SYN\_SeqCompleteCallback**](syn__sequencer_8h.md#typedef-syn_seqcompletecallback) cb, void \* ctx) <br>_Set completion callback._  |
|  void | [**syn\_seq\_set\_loop**](#function-syn_seq_set_loop) ([**SYN\_Sequencer**](structSYN__Sequencer.md) \* seq, [**bool**](syn__defs_8h.md#enum-bool) loop) <br>_Enable looping (auto-restart on completion)._  |
|  void | [**syn\_seq\_start**](#function-syn_seq_start) ([**SYN\_Sequencer**](structSYN__Sequencer.md) \* seq) <br>_Start the sequence from the beginning._  |
|  void | [**syn\_seq\_stop**](#function-syn_seq_stop) ([**SYN\_Sequencer**](structSYN__Sequencer.md) \* seq) <br>_Stop the sequence._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_seq\_update**](#function-syn_seq_update) ([**SYN\_Sequencer**](structSYN__Sequencer.md) \* seq) <br>_Update the sequencer — call from main loop._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**execute\_step**](#function-execute_step) ([**SYN\_Sequencer**](structSYN__Sequencer.md) \* seq) <br>_Execute the current sequence step's action._  |


























## Public Functions Documentation




### function syn\_seq\_init 

_Initialize a sequencer._ 
```C++
void syn_seq_init (
    SYN_Sequencer * seq,
    const SYN_SeqStep * steps,
    uint16_t count
) 
```





**Parameters:**


* `seq` Sequencer instance. 
* `steps` Array of step descriptors (must persist). 
* `count` Number of steps. 




        

<hr>



### function syn\_seq\_on\_complete 

_Set completion callback._ 
```C++
void syn_seq_on_complete (
    SYN_Sequencer * seq,
    SYN_SeqCompleteCallback cb,
    void * ctx
) 
```





**Parameters:**


* `seq` Sequencer instance. 
* `cb` Callback function invoked on sequence end. 
* `ctx` Callback context data pointer. 




        

<hr>



### function syn\_seq\_set\_loop 

_Enable looping (auto-restart on completion)._ 
```C++
void syn_seq_set_loop (
    SYN_Sequencer * seq,
    bool loop
) 
```





**Parameters:**


* `seq` Sequencer instance. 
* `loop` True to loop sequence indefinitely, false for single run. 




        

<hr>



### function syn\_seq\_start 

_Start the sequence from the beginning._ 
```C++
void syn_seq_start (
    SYN_Sequencer * seq
) 
```





**Parameters:**


* `seq` Sequencer instance. 




        

<hr>



### function syn\_seq\_stop 

_Stop the sequence._ 
```C++
void syn_seq_stop (
    SYN_Sequencer * seq
) 
```





**Parameters:**


* `seq` Sequencer instance. 




        

<hr>



### function syn\_seq\_update 

_Update the sequencer — call from main loop._ 
```C++
bool syn_seq_update (
    SYN_Sequencer * seq
) 
```



Executes actions and manages delays non-blockingly.




**Parameters:**


* `seq` Sequencer instance. 



**Returns:**

true if the sequence completed this call. 





        

<hr>
## Public Static Functions Documentation




### function execute\_step 

_Execute the current sequence step's action._ 
```C++
static void execute_step (
    SYN_Sequencer * seq
) 
```





**Parameters:**


* `seq` Sequencer instance. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_sequencer.c`

