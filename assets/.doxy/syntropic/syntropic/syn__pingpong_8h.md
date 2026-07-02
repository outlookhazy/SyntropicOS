

# File syn\_pingpong.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_pingpong.h**](syn__pingpong_8h.md)

[Go to the source code of this file](syn__pingpong_8h_source.md)

_Ping-pong (double) buffer — header-only, zero-copy DMA pattern._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_PingPong**](structSYN__PingPong.md) <br>_Ping-pong (double) buffer — zero-copy DMA pattern._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**syn\_pingpong\_active**](#function-syn_pingpong_active) (const [**SYN\_PingPong**](structSYN__PingPong.md) \* pp) <br>_Get pointer to the active (being-filled) buffer._  |
|  void | [**syn\_pingpong\_consume**](#function-syn_pingpong_consume) ([**SYN\_PingPong**](structSYN__PingPong.md) \* pp) <br>_Mark the ready buffer as consumed. Call after processing._  |
|  void | [**syn\_pingpong\_init**](#function-syn_pingpong_init) ([**SYN\_PingPong**](structSYN__PingPong.md) \* pp, uint8\_t \* buf\_a, uint8\_t \* buf\_b, size\_t size) <br>_Initialize with two user-provided buffers of equal size._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_pingpong\_ready**](#function-syn_pingpong_ready) (const [**SYN\_PingPong**](structSYN__PingPong.md) \* pp) <br>_Check if the ready buffer has unprocessed data._  |
|  uint8\_t \* | [**syn\_pingpong\_ready\_buf**](#function-syn_pingpong_ready_buf) (const [**SYN\_PingPong**](structSYN__PingPong.md) \* pp) <br>_Get pointer to the ready (available-for-reading) buffer._  |
|  size\_t | [**syn\_pingpong\_size**](#function-syn_pingpong_size) (const [**SYN\_PingPong**](structSYN__PingPong.md) \* pp) <br>_Get the buffer size._  |
|  void | [**syn\_pingpong\_swap**](#function-syn_pingpong_swap) ([**SYN\_PingPong**](structSYN__PingPong.md) \* pp) <br>_Swap buffers. Call from ISR/DMA complete._  |


























## Detailed Description


One buffer is "active" (being filled by DMA/ISR), the other is "ready" (available for processing). Call swap() when the active buffer is full to exchange them atomically.




**
**


```C++
uint8_t pool_a[256], pool_b[256];
SYN_PingPong pp;
syn_pingpong_init(&pp, pool_a, pool_b, 256);

// DMA writes into active buffer:
uint8_t *dma_buf = syn_pingpong_active(&pp);
start_dma(dma_buf, 256);

// DMA complete ISR:
syn_pingpong_swap(&pp);

// Main loop processes the ready buffer:
if (syn_pingpong_ready(&pp)) {
    uint8_t *data = syn_pingpong_ready_buf(&pp);
    process(data, 256);
    syn_pingpong_consume(&pp);
}
```
 





    
## Public Static Functions Documentation




### function syn\_pingpong\_active 

_Get pointer to the active (being-filled) buffer._ 
```C++
static inline uint8_t * syn_pingpong_active (
    const SYN_PingPong * pp
) 
```





**Parameters:**


* `pp` Ping-pong instance. 



**Returns:**

Active buffer pointer. 





        

<hr>



### function syn\_pingpong\_consume 

_Mark the ready buffer as consumed. Call after processing._ 
```C++
static inline void syn_pingpong_consume (
    SYN_PingPong * pp
) 
```





**Parameters:**


* `pp` Ping-pong instance. 




        

<hr>



### function syn\_pingpong\_init 

_Initialize with two user-provided buffers of equal size._ 
```C++
static inline void syn_pingpong_init (
    SYN_PingPong * pp,
    uint8_t * buf_a,
    uint8_t * buf_b,
    size_t size
) 
```





**Parameters:**


* `pp` Ping-pong instance. 
* `buf_a` First buffer. 
* `buf_b` Second buffer. 
* `size` Size of each buffer in bytes. 




        

<hr>



### function syn\_pingpong\_ready 

_Check if the ready buffer has unprocessed data._ 
```C++
static inline bool syn_pingpong_ready (
    const SYN_PingPong * pp
) 
```





**Parameters:**


* `pp` Ping-pong instance. 



**Returns:**

true if data is ready. 





        

<hr>



### function syn\_pingpong\_ready\_buf 

_Get pointer to the ready (available-for-reading) buffer._ 
```C++
static inline uint8_t * syn_pingpong_ready_buf (
    const SYN_PingPong * pp
) 
```





**Parameters:**


* `pp` Ping-pong instance. 



**Returns:**

Ready buffer pointer. 





        

<hr>



### function syn\_pingpong\_size 

_Get the buffer size._ 
```C++
static inline size_t syn_pingpong_size (
    const SYN_PingPong * pp
) 
```





**Parameters:**


* `pp` Ping-pong instance. 



**Returns:**

Buffer size in bytes. 





        

<hr>



### function syn\_pingpong\_swap 

_Swap buffers. Call from ISR/DMA complete._ 
```C++
static inline void syn_pingpong_swap (
    SYN_PingPong * pp
) 
```



The previously-active buffer becomes ready for processing. The previously-ready buffer becomes the new active buffer.




**Parameters:**


* `pp` Ping-pong instance. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_pingpong.h`

