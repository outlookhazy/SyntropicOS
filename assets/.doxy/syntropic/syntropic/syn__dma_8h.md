

# File syn\_dma.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_dma.h**](syn__dma_8h.md)

[Go to the source code of this file](syn__dma_8h_source.md)

_Bare-Metal Safe DMA Transaction Engine._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_dma.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DMA**](structSYN__DMA.md) <br>_DMA Channel instance object._  |
| struct | [**SYN\_DMA\_Config**](structSYN__DMA__Config.md) <br>_Configuration parameters for a DMA channel._  |
| struct | [**SYN\_DMA\_RingBuf**](structSYN__DMA__RingBuf.md) <br>_Continuous circular DMA reception stream controller._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_DMA\_Callback**](#typedef-syn_dma_callback)  <br>_Callback for DMA transfer events._  |
| enum  | [**SYN\_DMA\_Event**](#enum-syn_dma_event)  <br>_DMA ISR event flags._  |
| enum  | [**SYN\_DMA\_State**](#enum-syn_dma_state)  <br>_DMA channel execution state._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_DMA\_State**](syn__dma_8h.md#enum-syn_dma_state) | [**syn\_dma\_get\_state**](#function-syn_dma_get_state) (const [**SYN\_DMA**](structSYN__DMA.md) \* dma) <br>_Get current state of DMA channel._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dma\_init**](#function-syn_dma_init) ([**SYN\_DMA**](structSYN__DMA.md) \* dma, const [**SYN\_DMA\_Config**](structSYN__DMA__Config.md) \* cfg) <br>_Initialize a DMA channel object._  |
|  void | [**syn\_dma\_isr\_handler**](#function-syn_dma_isr_handler) ([**SYN\_DMA**](structSYN__DMA.md) \* dma, [**SYN\_DMA\_Event**](syn__dma_8h.md#enum-syn_dma_event) event) <br>_Hardware ISR event handler entry point._  |
|  size\_t | [**syn\_dma\_ringbuf\_bytes\_available**](#function-syn_dma_ringbuf_bytes_available) (const [**SYN\_DMA\_RingBuf**](structSYN__DMA__RingBuf.md) \* r) <br>_Get total bytes available to read from the circular DMA stream._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dma\_ringbuf\_init**](#function-syn_dma_ringbuf_init) ([**SYN\_DMA\_RingBuf**](structSYN__DMA__RingBuf.md) \* r, [**SYN\_DMA**](structSYN__DMA.md) \* dma, uint8\_t \* buf, size\_t capacity) <br>_Initialize a circular DMA ring buffer receiver._  |
|  size\_t | [**syn\_dma\_ringbuf\_read**](#function-syn_dma_ringbuf_read) ([**SYN\_DMA\_RingBuf**](structSYN__DMA__RingBuf.md) \* r, uint8\_t \* dest, size\_t len) <br>_Read bytes from the circular DMA reception buffer into destination._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dma\_ringbuf\_start**](#function-syn_dma_ringbuf_start) ([**SYN\_DMA\_RingBuf**](structSYN__DMA__RingBuf.md) \* r, const void \* periph\_src) <br>_Start continuous circular DMA stream reception._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dma\_start**](#function-syn_dma_start) ([**SYN\_DMA**](structSYN__DMA.md) \* dma, const void \* src, void \* dst, size\_t count) <br>_Submit and trigger a DMA transfer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dma\_stop**](#function-syn_dma_stop) ([**SYN\_DMA**](structSYN__DMA.md) \* dma) <br>_Stop/abort an active DMA transfer._  |




























## Detailed Description


Provides safe DMA transaction submission with memory alignment verification, data cache maintenance, re-entrancy locking, and ISR event callbacks. 


    
## Public Types Documentation




### typedef SYN\_DMA\_Callback 

_Callback for DMA transfer events._ 
```C++
typedef void(* SYN_DMA_Callback) (struct SYN_DMA *dma, SYN_DMA_Event event, void *user_ctx);
```




<hr>



### enum SYN\_DMA\_Event 

_DMA ISR event flags._ 
```C++
enum SYN_DMA_Event {
    SYN_DMA_EVENT_HALF_COMPLETE = 0x01,
    SYN_DMA_EVENT_COMPLETE = 0x02,
    SYN_DMA_EVENT_ERROR = 0x04
};
```




<hr>



### enum SYN\_DMA\_State 

_DMA channel execution state._ 
```C++
enum SYN_DMA_State {
    SYN_DMA_STATE_IDLE = 0,
    SYN_DMA_STATE_BUSY = 1,
    SYN_DMA_STATE_COMPLETE = 2,
    SYN_DMA_STATE_ERROR = 3
};
```




<hr>
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



### function syn\_dma\_ringbuf\_bytes\_available 

_Get total bytes available to read from the circular DMA stream._ 
```C++
size_t syn_dma_ringbuf_bytes_available (
    const SYN_DMA_RingBuf * r
) 
```





**Parameters:**


* `r` Ring buffer instance. 



**Returns:**

Number of unread bytes. 





        

<hr>



### function syn\_dma\_ringbuf\_init 

_Initialize a circular DMA ring buffer receiver._ 
```C++
SYN_Status syn_dma_ringbuf_init (
    SYN_DMA_RingBuf * r,
    SYN_DMA * dma,
    uint8_t * buf,
    size_t capacity
) 
```





**Parameters:**


* `r` Ring buffer instance. 
* `dma` Initialized DMA channel instance. 
* `buf` Reception buffer (aligned to dma data size). 
* `capacity` Buffer capacity in bytes. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_dma\_ringbuf\_read 

_Read bytes from the circular DMA reception buffer into destination._ 
```C++
size_t syn_dma_ringbuf_read (
    SYN_DMA_RingBuf * r,
    uint8_t * dest,
    size_t len
) 
```





**Parameters:**


* `r` Ring buffer instance. 
* `dest` Output data buffer. 
* `len` Maximum bytes to read. 



**Returns:**

Actual bytes read. 





        

<hr>



### function syn\_dma\_ringbuf\_start 

_Start continuous circular DMA stream reception._ 
```C++
SYN_Status syn_dma_ringbuf_start (
    SYN_DMA_RingBuf * r,
    const void * periph_src
) 
```





**Parameters:**


* `r` Ring buffer instance. 
* `periph_src` Peripheral data register source address. 



**Returns:**

SYN\_OK on success. 





        

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
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_dma.h`

