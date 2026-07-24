

# File syn\_datalog.h



[**FileList**](files.md) **>** [**log**](dir_1a216ccd2a39b426a7ff78fb20f852eb.md) **>** [**syn\_datalog.h**](syn__datalog_8h.md)

[Go to the source code of this file](syn__datalog_8h_source.md)

_High-speed binary telemetry logger._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../util/syn_ringbuf.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DataLog**](structSYN__DataLog.md) <br>_Data logger control block — ring buffer + drop counter._  |
| struct | [**SYN\_DataLogHeader**](structSYN__DataLogHeader.md) <br>_Telemetry frame header._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_datalog\_init**](#function-syn_datalog_init) ([**SYN\_DataLog**](structSYN__DataLog.md) \* log, uint8\_t \* buf, size\_t size) <br>_Initialize a datalogger._  |
|  size\_t | [**syn\_datalog\_read**](#function-syn_datalog_read) ([**SYN\_DataLog**](structSYN__DataLog.md) \* log, uint16\_t \* out\_id, void \* out\_data, size\_t max\_len) <br>_Read the next telemetry frame from the logger._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_datalog\_write**](#function-syn_datalog_write) ([**SYN\_DataLog**](structSYN__DataLog.md) \* log, uint16\_t id, const void \* data, uint16\_t len) <br>_Write a telemetry frame to the logger._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_datalog\_available**](#function-syn_datalog_available) (const [**SYN\_DataLog**](structSYN__DataLog.md) \* log) <br>_Get number of bytes available for reading._  |
|  uint32\_t | [**syn\_datalog\_get\_dropped**](#function-syn_datalog_get_dropped) (const [**SYN\_DataLog**](structSYN__DataLog.md) \* log) <br>_Get number of dropped frames._  |
|  void | [**syn\_datalog\_reset**](#function-syn_datalog_reset) ([**SYN\_DataLog**](structSYN__DataLog.md) \* log) <br>_Reset the datalog, discarding all frames._  |


























## Detailed Description


Captures arbitrary structs or data frames into a static ringbuffer. Useful for high-speed PID tuning, state recording, or telemetry streams.


Uses a zero-allocation, lock-free ringbuffer backend. Data is prefixed with a 16-bit ID and 16-bit length to allow demultiplexing on the host. 


    
## Public Functions Documentation




### function syn\_datalog\_init 

_Initialize a datalogger._ 
```C++
void syn_datalog_init (
    SYN_DataLog * log,
    uint8_t * buf,
    size_t size
) 
```





**Parameters:**


* `log` Pointer to logger instance. 
* `buf` Caller-provided backing array. 
* `size` Size of backing array in bytes. 




        

<hr>



### function syn\_datalog\_read 

_Read the next telemetry frame from the logger._ 
```C++
size_t syn_datalog_read (
    SYN_DataLog * log,
    uint16_t * out_id,
    void * out_data,
    size_t max_len
) 
```





**Parameters:**


* `log` Pointer to logger instance. 
* `out_id` [out] Stream ID of read frame. 
* `out_data` [out] Buffer to store payload. 
* `max_len` Maximum size of out\_data. 



**Returns:**

Number of payload bytes read, or 0 if empty or buffer too small. 





        

<hr>



### function syn\_datalog\_write 

_Write a telemetry frame to the logger._ 
```C++
bool syn_datalog_write (
    SYN_DataLog * log,
    uint16_t id,
    const void * data,
    uint16_t len
) 
```





**Parameters:**


* `log` Pointer to logger instance. 
* `id` Stream ID. 
* `data` Payload to write. 
* `len` Payload size in bytes. 



**Returns:**

true if written, false if buffer is full (frame dropped). 





        

<hr>
## Public Static Functions Documentation




### function syn\_datalog\_available 

_Get number of bytes available for reading._ 
```C++
static inline size_t syn_datalog_available (
    const SYN_DataLog * log
) 
```





**Parameters:**


* `log` Logger instance. 



**Returns:**

Bytes available. 





        

<hr>



### function syn\_datalog\_get\_dropped 

_Get number of dropped frames._ 
```C++
static inline uint32_t syn_datalog_get_dropped (
    const SYN_DataLog * log
) 
```





**Parameters:**


* `log` Logger instance. 



**Returns:**

Drop count. 





        

<hr>



### function syn\_datalog\_reset 

_Reset the datalog, discarding all frames._ 
```C++
static inline void syn_datalog_reset (
    SYN_DataLog * log
) 
```





**Parameters:**


* `log` Logger instance. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/log/syn_datalog.h`

