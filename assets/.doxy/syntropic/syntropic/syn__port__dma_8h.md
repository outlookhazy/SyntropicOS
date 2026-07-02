

# File syn\_port\_dma.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_dma.h**](syn__port__dma_8h.md)

[Go to the source code of this file](syn__port__dma_8h_source.md)

_DMA port interface — implement these for your platform._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DMA\_Config**](structSYN__DMA__Config.md) <br>_DMA channel configuration — caller-owned._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_DMA\_Callback**](#typedef-syn_dma_callback)  <br>_DMA transfer completion callback._  |
| enum  | [**SYN\_DMA\_Dir**](#enum-syn_dma_dir)  <br>_DMA transfer direction._  |
| enum  | [**SYN\_DMA\_Width**](#enum-syn_dma_width)  <br>_DMA element data width._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_port\_dma\_busy**](#function-syn_port_dma_busy) (uint8\_t channel) <br>_Check if a DMA channel is currently transferring._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_dma\_init**](#function-syn_port_dma_init) (const [**SYN\_DMA\_Config**](structSYN__DMA__Config.md) \* cfg) <br>_Initialize a DMA channel._  |
|  size\_t | [**syn\_port\_dma\_remaining**](#function-syn_port_dma_remaining) (uint8\_t channel) <br>_Get the number of remaining elements in the current transfer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_dma\_start**](#function-syn_port_dma_start) (uint8\_t channel, const volatile void \* src, volatile void \* dst, size\_t count) <br>_Start a DMA transfer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_dma\_stop**](#function-syn_port_dma_stop) (uint8\_t channel) <br>_Stop a DMA transfer in progress._  |




























## Detailed Description


Provides a portable DMA abstraction for memory-to-peripheral, peripheral-to-memory, and memory-to-memory transfers. The completion callback fires from ISR context — use [**syn\_workqueue\_post()**](syn__workqueue_8c.md#function-syn_workqueue_post) to defer heavy processing to the main context.




**
**


```C++
static void on_dma_done(uint8_t ch, SYN_Status result, void *ctx) {
    // Transfer complete — process data or post to workqueue
}

SYN_DMA_Config cfg = {
    .channel   = 0,
    .direction = SYN_DMA_PERIPH_TO_MEM,
    .width     = SYN_DMA_WIDTH_16,
    .src_incr  = false,
    .dst_incr  = true,
    .callback  = on_dma_done,
    .user_data = NULL,
};
syn_port_dma_init(&cfg);
syn_port_dma_start(0, &ADC_DR, adc_buf, 256);
```
 





    
## Public Types Documentation




### typedef SYN\_DMA\_Callback 

_DMA transfer completion callback._ 
```C++
typedef void(* SYN_DMA_Callback) (uint8_t channel, SYN_Status result, void *ctx);
```



Called from ISR context when a transfer completes or encounters an error. Use [**syn\_workqueue\_post()**](syn__workqueue_8c.md#function-syn_workqueue_post) for non-trivial processing.




**Parameters:**


* `channel` DMA channel that completed. 
* `result` SYN\_OK on success, SYN\_ERROR on transfer error. 
* `ctx` User context provided in the config. 




        

<hr>



### enum SYN\_DMA\_Dir 

_DMA transfer direction._ 
```C++
enum SYN_DMA_Dir {
    SYN_DMA_MEM_TO_PERIPH = 0,
    SYN_DMA_PERIPH_TO_MEM = 1,
    SYN_DMA_MEM_TO_MEM = 2
};
```




<hr>



### enum SYN\_DMA\_Width 

_DMA element data width._ 
```C++
enum SYN_DMA_Width {
    SYN_DMA_WIDTH_8 = 1,
    SYN_DMA_WIDTH_16 = 2,
    SYN_DMA_WIDTH_32 = 4
};
```




<hr>
## Public Functions Documentation




### function syn\_port\_dma\_busy 

_Check if a DMA channel is currently transferring._ 
```C++
bool syn_port_dma_busy (
    uint8_t channel
) 
```





**Parameters:**


* `channel` DMA channel to query. 



**Returns:**

true if a transfer is in progress. 





        

<hr>



### function syn\_port\_dma\_init 

_Initialize a DMA channel._ 
```C++
SYN_Status syn_port_dma_init (
    const SYN_DMA_Config * cfg
) 
```





**Parameters:**


* `cfg` Channel configuration. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_dma\_remaining 

_Get the number of remaining elements in the current transfer._ 
```C++
size_t syn_port_dma_remaining (
    uint8_t channel
) 
```





**Parameters:**


* `channel` DMA channel to query. 



**Returns:**

Remaining element count, or 0 if idle. 





        

<hr>



### function syn\_port\_dma\_start 

_Start a DMA transfer._ 
```C++
SYN_Status syn_port_dma_start (
    uint8_t channel,
    const volatile void * src,
    volatile void * dst,
    size_t count
) 
```





**Parameters:**


* `channel` DMA channel number (must have been initialized). 
* `src` Source address (memory or peripheral register). 
* `dst` Destination address (memory or peripheral register). 
* `count` Number of elements (not bytes) to transfer. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if channel busy. 





        

<hr>



### function syn\_port\_dma\_stop 

_Stop a DMA transfer in progress._ 
```C++
SYN_Status syn_port_dma_stop (
    uint8_t channel
) 
```





**Parameters:**


* `channel` DMA channel to stop. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_dma.h`

