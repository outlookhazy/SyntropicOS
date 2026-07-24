

# File syn\_dma.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_dma.c**](syn__dma_8c.md)

[Go to the source code of this file](syn__dma_8c_source.md)

_Bare-Metal Safe DMA Transaction Engine implementation._ 

* `#include "syn_dma.h"`
* `#include "../util/syn_assert.h"`
* `#include "../common/syn_compiler.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_DMA\_State**](syn__dma_8h.md#enum-syn_dma_state) | [**syn\_dma\_get\_state**](#function-syn_dma_get_state) (const [**SYN\_DMA**](structSYN__DMA.md) \* dma) <br>_Get current state of DMA channel._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dma\_init**](#function-syn_dma_init) ([**SYN\_DMA**](structSYN__DMA.md) \* dma, const [**SYN\_DMA\_Config**](structSYN__DMA__Config.md) \* cfg) <br>_Initialize a DMA channel object._  |
|  void | [**syn\_dma\_isr\_handler**](#function-syn_dma_isr_handler) ([**SYN\_DMA**](structSYN__DMA.md) \* dma, [**SYN\_DMA\_Event**](syn__dma_8h.md#enum-syn_dma_event) event) <br>_Hardware ISR event handler entry point._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dma\_start**](#function-syn_dma_start) ([**SYN\_DMA**](structSYN__DMA.md) \* dma, const void \* src, void \* dst, size\_t count) <br>_Submit and trigger a DMA transfer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dma\_stop**](#function-syn_dma_stop) ([**SYN\_DMA**](structSYN__DMA.md) \* dma) <br>_Stop/abort an active DMA transfer._  |




























## Public Functions Documentation




### function syn\_dma\_get\_state 

_Get current state of DMA channel._ 
```C++
SYN_DMA_State syn_dma_get_state (
    const SYN_DMA * dma
) 
```





**Parameters:**


* `dma` Pointer to DMA channel. 



**Returns:**

Current SYN\_DMA\_State. 





        

<hr>



### function syn\_dma\_init 

_Initialize a DMA channel object._ 
```C++
SYN_Status syn_dma_init (
    SYN_DMA * dma,
    const SYN_DMA_Config * cfg
) 
```





**Parameters:**


* `dma` Pointer to DMA instance. 
* `cfg` Configuration structure. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM if dma/cfg is NULL. 





        

<hr>



### function syn\_dma\_isr\_handler 

_Hardware ISR event handler entry point._ 
```C++
void syn_dma_isr_handler (
    SYN_DMA * dma,
    SYN_DMA_Event event
) 
```



Call this function from the MCU hardware DMA interrupt handler when a channel fires a half-transfer, complete, or error interrupt.




**Parameters:**


* `dma` Pointer to DMA channel. 
* `event` Event bitmask (SYN\_DMA\_EVENT\_\*). 




        

<hr>



### function syn\_dma\_start 

_Submit and trigger a DMA transfer._ 
```C++
SYN_Status syn_dma_start (
    SYN_DMA * dma,
    const void * src,
    void * dst,
    size_t count
) 
```



Verifies address alignment against data element size (8/16/32-bit), cleans D-cache for source region, invalidates D-cache for destination region, and launches hardware transfer.




**Parameters:**


* `dma` Pointer to initialized DMA channel instance. 
* `src` Source buffer address. 
* `dst` Destination buffer address. 
* `count` Number of elements to transfer (must be &gt; 0). 



**Returns:**

SYN\_OK on success. 




**Returns:**

SYN\_INVALID\_PARAM if buffer address is unaligned to data\_size or parameters invalid. 




**Returns:**

SYN\_BUSY if channel is actively transferring. 





        

<hr>



### function syn\_dma\_stop 

_Stop/abort an active DMA transfer._ 
```C++
SYN_Status syn_dma_stop (
    SYN_DMA * dma
) 
```





**Parameters:**


* `dma` Pointer to DMA channel. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_dma.c`

