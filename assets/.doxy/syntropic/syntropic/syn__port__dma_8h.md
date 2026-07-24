

# File syn\_port\_dma.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_dma.h**](syn__port__dma_8h.md)

[Go to the source code of this file](syn__port__dma_8h_source.md)

_Hardware port abstraction interface for DMA controller._ 

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_PortDmaTransfer**](structSYN__PortDmaTransfer.md) <br>_Hardware DMA transfer hardware request parameters._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_DMA\_Direction**](#enum-syn_dma_direction)  <br>_DMA transfer direction._  |
| enum  | [**SYN\_DMA\_Size**](#enum-syn_dma_size)  <br>_DMA transfer data element size._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_port\_cache\_clean**](#function-syn_port_cache_clean) (const void \* addr, size\_t len) <br>_Flush/clean data cache for source RAM region before DMA read._  |
|  void | [**syn\_port\_cache\_invalidate**](#function-syn_port_cache_invalidate) (void \* addr, size\_t len) <br>_Invalidate data cache for destination RAM region after DMA write._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_port\_dma\_is\_busy**](#function-syn_port_dma_is_busy) (uint8\_t channel\_id) <br>_Check if hardware DMA channel is active._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_dma\_start**](#function-syn_port_dma_start) (const [**SYN\_PortDmaTransfer**](structSYN__PortDmaTransfer.md) \* xfer) <br>_Trigger hardware DMA channel start._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_dma\_stop**](#function-syn_port_dma_stop) (uint8\_t channel\_id) <br>_Abort an active hardware DMA channel._  |




























## Public Types Documentation




### enum SYN\_DMA\_Direction 

_DMA transfer direction._ 
```C++
enum SYN_DMA_Direction {
    SYN_DMA_DIR_MEM_TO_MEM = 0,
    SYN_DMA_DIR_MEM_TO_PERIPH = 1,
    SYN_DMA_DIR_PERIPH_TO_MEM = 2
};
```




<hr>



### enum SYN\_DMA\_Size 

_DMA transfer data element size._ 
```C++
enum SYN_DMA_Size {
    SYN_DMA_SIZE_8BIT = 1,
    SYN_DMA_SIZE_16BIT = 2,
    SYN_DMA_SIZE_32BIT = 4
};
```




<hr>
## Public Functions Documentation




### function syn\_port\_cache\_clean 

_Flush/clean data cache for source RAM region before DMA read._ 
```C++
void syn_port_cache_clean (
    const void * addr,
    size_t len
) 
```





**Parameters:**


* `addr` Start address. 
* `len` Byte length. 




        

<hr>



### function syn\_port\_cache\_invalidate 

_Invalidate data cache for destination RAM region after DMA write._ 
```C++
void syn_port_cache_invalidate (
    void * addr,
    size_t len
) 
```





**Parameters:**


* `addr` Start address. 
* `len` Byte length. 




        

<hr>



### function syn\_port\_dma\_is\_busy 

_Check if hardware DMA channel is active._ 
```C++
bool syn_port_dma_is_busy (
    uint8_t channel_id
) 
```





**Parameters:**


* `channel_id` Hardware DMA channel index. 



**Returns:**

true if channel is actively transferring. 





        

<hr>



### function syn\_port\_dma\_start 

_Trigger hardware DMA channel start._ 
```C++
SYN_Status syn_port_dma_start (
    const SYN_PortDmaTransfer * xfer
) 
```





**Parameters:**


* `xfer` Transfer configuration. 



**Returns:**

SYN\_OK on success, error code otherwise. 





        

<hr>



### function syn\_port\_dma\_stop 

_Abort an active hardware DMA channel._ 
```C++
SYN_Status syn_port_dma_stop (
    uint8_t channel_id
) 
```





**Parameters:**


* `channel_id` Hardware DMA channel index. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_dma.h`

