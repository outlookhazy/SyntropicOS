

# File syn\_stream.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_stream.h**](syn__stream_8h.md)

[Go to the source code of this file](syn__stream_8h_source.md)

_Cooperative byte stream — protothread-aware ringbuf wrapper._ [More...](#detailed-description)

* `#include "../util/syn_ringbuf.h"`
* `#include "../pt/syn_pt.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Stream**](structSYN__Stream.md) <br>_Cooperative byte stream — ringbuf with readability awareness._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_stream\_clear\_delimiter**](#function-syn_stream_clear_delimiter) ([**SYN\_Stream**](structSYN__Stream.md) \* s) <br>_Disable delimiter mode, reverting to default or threshold mode._  |
|  size\_t | [**syn\_stream\_count**](#function-syn_stream_count) (const [**SYN\_Stream**](structSYN__Stream.md) \* s) <br>_Return the number of bytes currently in the stream._  |
|  void | [**syn\_stream\_flush**](#function-syn_stream_flush) ([**SYN\_Stream**](structSYN__Stream.md) \* s) <br>_Discard all data in the stream._  |
|  size\_t | [**syn\_stream\_free**](#function-syn_stream_free) (const [**SYN\_Stream**](structSYN__Stream.md) \* s) <br>_Return the number of free bytes available for writing._  |
|  void | [**syn\_stream\_init**](#function-syn_stream_init) ([**SYN\_Stream**](structSYN__Stream.md) \* s, uint8\_t \* buf, size\_t size) <br>_Initialize a stream with a caller-owned backing buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_stream\_put**](#function-syn_stream_put) ([**SYN\_Stream**](structSYN__Stream.md) \* s, uint8\_t byte) <br>_Put a single byte into the stream._  |
|  size\_t | [**syn\_stream\_read**](#function-syn_stream_read) ([**SYN\_Stream**](structSYN__Stream.md) \* s, uint8\_t \* buf, size\_t max\_len) <br>_Read up to_ `max_len` _bytes from the stream._ |
|  size\_t | [**syn\_stream\_read\_line**](#function-syn_stream_read_line) ([**SYN\_Stream**](structSYN__Stream.md) \* s, uint8\_t \* buf, size\_t max\_len) <br>_Read up to and including the next delimiter byte._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_stream\_readable**](#function-syn_stream_readable) (const [**SYN\_Stream**](structSYN__Stream.md) \* s) <br>_Check if the stream is readable (mode-aware)._  |
|  void | [**syn\_stream\_set\_delimiter**](#function-syn_stream_set_delimiter) ([**SYN\_Stream**](structSYN__Stream.md) \* s, uint8\_t delim) <br>_Enable delimiter mode._  |
|  void | [**syn\_stream\_set\_threshold**](#function-syn_stream_set_threshold) ([**SYN\_Stream**](structSYN__Stream.md) \* s, size\_t n) <br>_Set the byte threshold for readability._  |
|  size\_t | [**syn\_stream\_write**](#function-syn_stream_write) ([**SYN\_Stream**](structSYN__Stream.md) \* s, const uint8\_t \* data, size\_t len) <br>_Write bytes into the stream._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_STREAM\_WAIT**](syn__stream_8h.md#define-pt_stream_wait) (pt, stream) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, [**syn\_stream\_readable**](syn__stream_8c.md#function-syn_stream_readable)(stream))`<br>_Wait until the stream is readable (mode-aware)._  |

## Detailed Description


Composes [**SYN\_RingBuf**](structSYN__RingBuf.md) with readability awareness for cooperative tasks. Three readability modes:
* **Default**: readable when any bytes are available.
* **Threshold**: readable when N+ bytes are available (frame protocols).
* **Delimiter**: readable when a delimiter byte is in the buffer (line protocols).




Producer side is ISR-safe (single-producer / single-consumer). Consumer side integrates with protothreads via PT\_STREAM\_WAIT.




**
**


```C++
static uint8_t backing[128];
static SYN_Stream uart_rx;
syn_stream_init(&uart_rx, backing, sizeof(backing));
syn_stream_set_delimiter(&uart_rx, '\n');

// ISR fills the stream:
syn_stream_put(&uart_rx, byte);

// Protothread consumes complete lines:
SYN_PT_Status my_task(SYN_PT *pt, SYN_Task *task) {
    static uint8_t line[80];
    static size_t  n;
    PT_BEGIN(pt);
    for (;;) {
        PT_STREAM_WAIT(pt, &uart_rx);
        n = syn_stream_read_line(&uart_rx, line, sizeof(line));
        process_line(line, n);
    }
    PT_END(pt);
}
```





**
**


```C++
syn_stream_set_threshold(&adc_stream, 64);  // wait for 64 bytes
PT_STREAM_WAIT(pt, &adc_stream);
n = syn_stream_read(&adc_stream, frame, 64);
```
 





    
## Public Functions Documentation




### function syn\_stream\_clear\_delimiter 

_Disable delimiter mode, reverting to default or threshold mode._ 
```C++
void syn_stream_clear_delimiter (
    SYN_Stream * s
) 
```





**Parameters:**


* `s` Stream instance. 




        

<hr>



### function syn\_stream\_count 

_Return the number of bytes currently in the stream._ 
```C++
size_t syn_stream_count (
    const SYN_Stream * s
) 
```





**Parameters:**


* `s` Stream instance. 



**Returns:**

Byte count. 





        

<hr>



### function syn\_stream\_flush 

_Discard all data in the stream._ 
```C++
void syn_stream_flush (
    SYN_Stream * s
) 
```





**Parameters:**


* `s` Stream instance. 




        

<hr>



### function syn\_stream\_free 

_Return the number of free bytes available for writing._ 
```C++
size_t syn_stream_free (
    const SYN_Stream * s
) 
```





**Parameters:**


* `s` Stream instance. 



**Returns:**

Free byte count. 





        

<hr>



### function syn\_stream\_init 

_Initialize a stream with a caller-owned backing buffer._ 
```C++
void syn_stream_init (
    SYN_Stream * s,
    uint8_t * buf,
    size_t size
) 
```





**Parameters:**


* `s` Stream instance. 
* `buf` Backing array (caller-owned, must outlive the stream). 
* `size` Size of backing array. Usable capacity is size - 1. 




        

<hr>



### function syn\_stream\_put 

_Put a single byte into the stream._ 
```C++
bool syn_stream_put (
    SYN_Stream * s,
    uint8_t byte
) 
```



ISR-safe convenience wrapper around [**syn\_stream\_write()**](syn__stream_8h.md#function-syn_stream_write).




**Parameters:**


* `s` Stream instance. 
* `byte` Byte to store. 



**Returns:**

true if the byte was stored, false if the buffer is full. 





        

<hr>



### function syn\_stream\_read 

_Read up to_ `max_len` _bytes from the stream._
```C++
size_t syn_stream_read (
    SYN_Stream * s,
    uint8_t * buf,
    size_t max_len
) 
```



Non-blocking — returns 0 if the buffer is empty. Does not respect threshold or delimiter; reads whatever is available.




**Parameters:**


* `s` Stream instance. 
* `buf` Destination buffer. 
* `max_len` Maximum bytes to read. 



**Returns:**

Number of bytes actually read. 





        

<hr>



### function syn\_stream\_read\_line 

_Read up to and including the next delimiter byte._ 
```C++
size_t syn_stream_read_line (
    SYN_Stream * s,
    uint8_t * buf,
    size_t max_len
) 
```



Reads bytes from the stream into `buf`, stopping after the first delimiter byte (which is included in the output). If no delimiter is found in the buffer, returns 0 and reads nothing.


Pair with PT\_STREAM\_WAIT in delimiter mode to wait for a complete line: 
```C++
PT_STREAM_WAIT(pt, &stream);
n = syn_stream_read_line(&stream, buf, sizeof(buf));
```





**Parameters:**


* `s` Stream instance. 
* `buf` Destination buffer. 
* `max_len` Maximum bytes to read (including delimiter). 



**Returns:**

Number of bytes read (0 if no delimiter found or empty). 





        

<hr>



### function syn\_stream\_readable 

_Check if the stream is readable (mode-aware)._ 
```C++
bool syn_stream_readable (
    const SYN_Stream * s
) 
```



Behavior depends on configured mode:
* **Delimiter mode**: true if the delimiter byte is in the buffer.
* **Threshold mode**: true if count &gt;= threshold.
* **Default**: true if any bytes are available.




Delimiter mode takes precedence over threshold mode.




**Parameters:**


* `s` Stream instance. 



**Returns:**

true if the stream has data meeting the readability criteria. 





        

<hr>



### function syn\_stream\_set\_delimiter 

_Enable delimiter mode._ 
```C++
void syn_stream_set_delimiter (
    SYN_Stream * s,
    uint8_t delim
) 
```



When enabled, [**syn\_stream\_readable()**](syn__stream_8h.md#function-syn_stream_readable) returns true only when the delimiter byte is present in the buffer. Pair with [**syn\_stream\_read\_line()**](syn__stream_8h.md#function-syn_stream_read_line) to consume one delimited chunk at a time.


Delimiter mode takes precedence over threshold mode.




**Parameters:**


* `s` Stream instance. 
* `delim` Delimiter byte (e.g. '\n'). 




        

<hr>



### function syn\_stream\_set\_threshold 

_Set the byte threshold for readability._ 
```C++
void syn_stream_set_threshold (
    SYN_Stream * s,
    size_t n
) 
```



When set to N &gt; 0, [**syn\_stream\_readable()**](syn__stream_8h.md#function-syn_stream_readable) returns true only when at least N bytes are in the buffer. Set to 0 to revert to default (readable when any bytes are available).




**Parameters:**


* `s` Stream instance. 
* `n` Byte threshold (0 = any). 




        

<hr>



### function syn\_stream\_write 

_Write bytes into the stream._ 
```C++
size_t syn_stream_write (
    SYN_Stream * s,
    const uint8_t * data,
    size_t len
) 
```



ISR-safe (single-producer). Writes up to `len` bytes. Returns the number actually written (may be less if the buffer fills up).




**Parameters:**


* `s` Stream instance. 
* `data` Source data. 
* `len` Number of bytes to write. 



**Returns:**

Number of bytes actually written. 





        

<hr>
## Macro Definition Documentation





### define PT\_STREAM\_WAIT 

_Wait until the stream is readable (mode-aware)._ 
```C++
#define PT_STREAM_WAIT (
    pt,
    stream
) `PT_WAIT_UNTIL (pt, syn_stream_readable (stream))`
```



Yields the protothread until [**syn\_stream\_readable()**](syn__stream_8h.md#function-syn_stream_readable) returns true.




**Parameters:**


* `pt` Protothread context. 
* `stream` Pointer to [**SYN\_Stream**](structSYN__Stream.md). 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_stream.h`

