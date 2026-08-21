

# File syn\_blackbox.c



[**FileList**](files.md) **>** [**log**](dir_1a216ccd2a39b426a7ff78fb20f852eb.md) **>** [**syn\_blackbox.c**](syn__blackbox_8c.md)

[Go to the source code of this file](syn__blackbox_8c_source.md)

_Zero-Heap Flight Telemetry Blackbox Binary Recorder Implementation._ 

* `#include "syntropic/log/syn_blackbox.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_blackbox\_encode\_delta**](#function-syn_blackbox_encode_delta) ([**SYN\_Blackbox**](structSYN__Blackbox.md) \* bb, const [**SYN\_Blackbox\_Record**](structSYN__Blackbox__Record.md) \* record, uint8\_t \* buf\_out, size\_t \* out\_len) <br>_Encode a compact Predictive Delta Frame ('P') relative to last frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_blackbox\_encode\_intra**](#function-syn_blackbox_encode_intra) ([**SYN\_Blackbox**](structSYN__Blackbox.md) \* bb, const [**SYN\_Blackbox\_Record**](structSYN__Blackbox__Record.md) \* record, uint8\_t \* buf\_out, size\_t \* out\_len) <br>_Encode a full Intra Frame ('I') into raw binary log stream._  |
|  size\_t | [**syn\_blackbox\_encode\_varint**](#function-syn_blackbox_encode_varint) (int32\_t val, uint8\_t \* buf\_out) <br>_Encode a 32-bit signed integer using ZigZag + LEB128 variable-length format._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_blackbox\_init**](#function-syn_blackbox_init) ([**SYN\_Blackbox**](structSYN__Blackbox.md) \* bb) <br>_Initialize Blackbox recorder._  |




























## Public Functions Documentation




### function syn\_blackbox\_encode\_delta 

_Encode a compact Predictive Delta Frame ('P') relative to last frame._ 
```C++
SYN_Status syn_blackbox_encode_delta (
    SYN_Blackbox * bb,
    const SYN_Blackbox_Record * record,
    uint8_t * buf_out,
    size_t * out_len
) 
```





**Parameters:**


* `bb` Pointer to Blackbox instance. 
* `record` Pointer to flight state record. 
* `buf_out` Output buffer (must hold at least 32 bytes). 
* `out_len` Pointer to receive encoded byte length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_blackbox\_encode\_intra 

_Encode a full Intra Frame ('I') into raw binary log stream._ 
```C++
SYN_Status syn_blackbox_encode_intra (
    SYN_Blackbox * bb,
    const SYN_Blackbox_Record * record,
    uint8_t * buf_out,
    size_t * out_len
) 
```





**Parameters:**


* `bb` Pointer to Blackbox instance. 
* `record` Pointer to flight state record. 
* `buf_out` Output buffer (must hold at least 32 bytes). 
* `out_len` Pointer to receive encoded byte length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_blackbox\_encode\_varint 

_Encode a 32-bit signed integer using ZigZag + LEB128 variable-length format._ 
```C++
size_t syn_blackbox_encode_varint (
    int32_t val,
    uint8_t * buf_out
) 
```





**Parameters:**


* `val` Signed integer. 
* `buf_out` Output buffer (must hold at least 5 bytes). 



**Returns:**

Number of bytes written to buffer (1..5). 





        

<hr>



### function syn\_blackbox\_init 

_Initialize Blackbox recorder._ 
```C++
SYN_Status syn_blackbox_init (
    SYN_Blackbox * bb
) 
```





**Parameters:**


* `bb` Pointer to Blackbox instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/log/syn_blackbox.c`

