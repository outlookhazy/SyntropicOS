

# File syn\_sequencer.h



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_sequencer.h**](syn__sequencer_8h.md)

[Go to the source code of this file](syn__sequencer_8h_source.md)

_Timed action sequencer — choreograph steps with delays._ [More...](#detailed-description)

* `#include "../port/syn_port_system.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include <stddef.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SeqStep**](structSYN__SeqStep.md) <br> |
| struct | [**SYN\_Sequencer**](structSYN__Sequencer.md) <br>_Sequencer runtime context._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_SeqAction**](#typedef-syn_seqaction)  <br> |
| typedef void(\* | [**SYN\_SeqCompleteCallback**](#typedef-syn_seqcompletecallback)  <br>_Completion callback function signature._  |
| enum  | [**SYN\_SeqState**](#enum-syn_seqstate)  <br>_Action sequencer execution states._  |




















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
|  uint16\_t | [**syn\_seq\_current\_step**](#function-syn_seq_current_step) (const [**SYN\_Sequencer**](structSYN__Sequencer.md) \* seq) <br>_Get current step index._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_seq\_is\_done**](#function-syn_seq_is_done) (const [**SYN\_Sequencer**](structSYN__Sequencer.md) \* seq) <br>_Check if done._  |
|  uint16\_t | [**syn\_seq\_loops**](#function-syn_seq_loops) (const [**SYN\_Sequencer**](structSYN__Sequencer.md) \* seq) <br>_Get loop iteration count._  |


























## Detailed Description


Execute a sequence of actions with configurable delays between them. Non-blocking — call `update()` from your main loop.




**
**


```C++
void relay_on(void *ctx) { syn_gpio_write(RELAY_PIN, SYN_GPIO_HIGH); }
void relay_off(void *ctx) { syn_gpio_write(RELAY_PIN, SYN_GPIO_LOW); }
void read_sensor(void *ctx) { *(int*)ctx = adc_read(); }

static const SYN_SeqStep steps[] = {
    { relay_on,     NULL,  0 },      // step 0: relay on, no delay
    { NULL,         NULL,  200 },     // step 1: wait 200ms
    { read_sensor,  &val,  0 },      // step 2: read sensor
    { relay_off,    NULL,  100 },     // step 3: relay off, then wait 100ms
};

SYN_Sequencer seq;
syn_seq_init(&seq, steps, 4);
syn_seq_start(&seq);

while (!syn_seq_is_done(&seq)) {
    syn_seq_update(&seq);
}
```
 





    
## Public Types Documentation




### typedef SYN\_SeqAction 

```C++
typedef void(* SYN_SeqAction) (void *ctx);
```



Action function called for each step. 


        

<hr>



### typedef SYN\_SeqCompleteCallback 

_Completion callback function signature._ 
```C++
typedef void(* SYN_SeqCompleteCallback) (struct SYN_Sequencer *seq, void *ctx);
```





**Parameters:**


* `seq` Pointer to the completed sequencer context. 
* `ctx` User context associated with the completion handler. 




        

<hr>



### enum SYN\_SeqState 

_Action sequencer execution states._ 
```C++
enum SYN_SeqState {
    SYN_SEQ_IDLE = 0,
    SYN_SEQ_RUNNING = 1,
    SYN_SEQ_WAITING = 2,
    SYN_SEQ_DONE = 3
};
```




<hr>
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




### function syn\_seq\_current\_step 

_Get current step index._ 
```C++
static inline uint16_t syn_seq_current_step (
    const SYN_Sequencer * seq
) 
```





**Parameters:**


* `seq` Sequencer instance. 



**Returns:**

Index of the currently executing/waiting step. 





        

<hr>



### function syn\_seq\_is\_done 

_Check if done._ 
```C++
static inline bool syn_seq_is_done (
    const SYN_Sequencer * seq
) 
```





**Parameters:**


* `seq` Sequencer instance. 



**Returns:**

True if in IDLE or DONE state. 





        

<hr>



### function syn\_seq\_loops 

_Get loop iteration count._ 
```C++
static inline uint16_t syn_seq_loops (
    const SYN_Sequencer * seq
) 
```





**Parameters:**


* `seq` Sequencer instance. 



**Returns:**

Number of times the sequence has finished (if looping is enabled). 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_sequencer.h`

