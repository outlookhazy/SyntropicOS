

# File syn\_netbuf.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_netbuf.h**](syn__netbuf_8h.md)

[Go to the source code of this file](syn__netbuf_8h_source.md)

_Zero-Copy Packet Buffer Pool._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_NetBuf**](structSYN__NetBuf.md) <br>_Zero-Copy Packet Buffer Descriptor._  |
| struct | [**SYN\_NetBufPool**](structSYN__NetBufPool.md) <br>_Packet Buffer Pool Descriptor._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_NetBuf**](structSYN__NetBuf.md) \* | [**syn\_netbuf\_alloc**](#function-syn_netbuf_alloc) ([**SYN\_NetBufPool**](structSYN__NetBufPool.md) \* pool, size\_t headroom) <br>_Allocate a packet buffer from the pool._  |
|  void | [**syn\_netbuf\_free**](#function-syn_netbuf_free) ([**SYN\_NetBufPool**](structSYN__NetBufPool.md) \* pool, [**SYN\_NetBuf**](structSYN__NetBuf.md) \* buf) <br>_Decrement reference counter and free buffer if count reaches 0._  |
|  size\_t | [**syn\_netbuf\_len**](#function-syn_netbuf_len) (const [**SYN\_NetBuf**](structSYN__NetBuf.md) \* buf) <br>_Get payload length in bytes._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_netbuf\_pool\_init**](#function-syn_netbuf_pool_init) ([**SYN\_NetBufPool**](structSYN__NetBufPool.md) \* pool, [**SYN\_NetBuf**](structSYN__NetBuf.md) \* bufs, uint8\_t \* raw\_mem, size\_t buf\_count, size\_t buf\_size) <br>_Initialize a packet buffer pool._  |
|  uint8\_t \* | [**syn\_netbuf\_pull**](#function-syn_netbuf_pull) ([**SYN\_NetBuf**](structSYN__NetBuf.md) \* buf, size\_t len) <br>_Strip header bytes from start of payload._  |
|  uint8\_t \* | [**syn\_netbuf\_push**](#function-syn_netbuf_push) ([**SYN\_NetBuf**](structSYN__NetBuf.md) \* buf, size\_t len) <br>_Prepend header space before payload data._  |
|  uint8\_t \* | [**syn\_netbuf\_put**](#function-syn_netbuf_put) ([**SYN\_NetBuf**](structSYN__NetBuf.md) \* buf, size\_t len) <br>_Append payload bytes to end of buffer._  |
|  void | [**syn\_netbuf\_ref**](#function-syn_netbuf_ref) ([**SYN\_NetBuf**](structSYN__NetBuf.md) \* buf) <br>_Increment the reference counter of a packet buffer._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_NETBUF\_HEADROOM\_DEFAULT**](syn__netbuf_8h.md#define-syn_netbuf_headroom_default)  `32U`<br> |

## Detailed Description


Provides a fixed-block memory pool manager for network/CAN packet buffers supporting reference counting, zero-copy header push/pull, and slicing. 


    
## Public Functions Documentation




### function syn\_netbuf\_alloc 

_Allocate a packet buffer from the pool._ 
```C++
SYN_NetBuf * syn_netbuf_alloc (
    SYN_NetBufPool * pool,
    size_t headroom
) 
```





**Parameters:**


* `pool` Pointer to pool descriptor. 
* `headroom` Initial headroom bytes reserved for network headers. 



**Returns:**

Allocated [**SYN\_NetBuf**](structSYN__NetBuf.md) pointer, or NULL if pool is exhausted. 





        

<hr>



### function syn\_netbuf\_free 

_Decrement reference counter and free buffer if count reaches 0._ 
```C++
void syn_netbuf_free (
    SYN_NetBufPool * pool,
    SYN_NetBuf * buf
) 
```





**Parameters:**


* `pool` Pointer to pool descriptor. 
* `buf` Pointer to packet buffer. 




        

<hr>



### function syn\_netbuf\_len 

_Get payload length in bytes._ 
```C++
size_t syn_netbuf_len (
    const SYN_NetBuf * buf
) 
```





**Parameters:**


* `buf` Pointer to packet buffer. 



**Returns:**

Payload length in bytes. 





        

<hr>



### function syn\_netbuf\_pool\_init 

_Initialize a packet buffer pool._ 
```C++
SYN_Status syn_netbuf_pool_init (
    SYN_NetBufPool * pool,
    SYN_NetBuf * bufs,
    uint8_t * raw_mem,
    size_t buf_count,
    size_t buf_size
) 
```





**Parameters:**


* `pool` Pointer to pool descriptor. 
* `bufs` Array of buffer descriptors. 
* `raw_mem` Raw memory block storage. 
* `buf_count` Number of buffers in pool. 
* `buf_size` Size of each buffer block in bytes. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_netbuf\_pull 

_Strip header bytes from start of payload._ 
```C++
uint8_t * syn_netbuf_pull (
    SYN_NetBuf * buf,
    size_t len
) 
```





**Parameters:**


* `buf` Pointer to packet buffer. 
* `len` Bytes to strip. 



**Returns:**

Pointer to new payload start, or NULL if payload shorter than len. 





        

<hr>



### function syn\_netbuf\_push 

_Prepend header space before payload data._ 
```C++
uint8_t * syn_netbuf_push (
    SYN_NetBuf * buf,
    size_t len
) 
```





**Parameters:**


* `buf` Pointer to packet buffer. 
* `len` Bytes to prepend. 



**Returns:**

Pointer to new payload start, or NULL if headroom insufficient. 





        

<hr>



### function syn\_netbuf\_put 

_Append payload bytes to end of buffer._ 
```C++
uint8_t * syn_netbuf_put (
    SYN_NetBuf * buf,
    size_t len
) 
```





**Parameters:**


* `buf` Pointer to packet buffer. 
* `len` Bytes to append. 



**Returns:**

Pointer to start of appended bytes, or NULL if tailroom insufficient. 





        

<hr>



### function syn\_netbuf\_ref 

_Increment the reference counter of a packet buffer._ 
```C++
void syn_netbuf_ref (
    SYN_NetBuf * buf
) 
```





**Parameters:**


* `buf` Pointer to packet buffer. 




        

<hr>
## Macro Definition Documentation





### define SYN\_NETBUF\_HEADROOM\_DEFAULT 

```C++
#define SYN_NETBUF_HEADROOM_DEFAULT `32U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_netbuf.h`

