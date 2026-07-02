

# File syn\_port\_can.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_can.h**](syn__port__can_8h.md)

[Go to the source code of this file](syn__port__can_8h_source.md)

_CAN bus port interface._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include <stddef.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_port\_can\_init**](#function-syn_port_can_init) (uint8\_t port, uint32\_t bitrate) <br>_Initialize CAN peripheral._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_port\_can\_receive**](#function-syn_port_can_receive) (uint8\_t port, uint32\_t \* id, [**bool**](syn__defs_8h.md#enum-bool) \* extended, uint8\_t \* data, uint8\_t \* dlc) <br>_Receive a CAN frame (non-blocking)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_port\_can\_send**](#function-syn_port_can_send) (uint8\_t port, uint32\_t id, [**bool**](syn__defs_8h.md#enum-bool) extended, const uint8\_t \* data, uint8\_t dlc) <br>_Send a CAN frame._  |
|  void | [**syn\_port\_can\_set\_filter**](#function-syn_port_can_set_filter) (uint8\_t port, uint32\_t id, uint32\_t mask) <br>_Set hardware acceptance filter._  |




























## Detailed Description


Implement these functions for your CAN peripheral. 


    
## Public Functions Documentation




### function syn\_port\_can\_init 

_Initialize CAN peripheral._ 
```C++
bool syn_port_can_init (
    uint8_t port,
    uint32_t bitrate
) 
```





**Parameters:**


* `port` CAN port number (0, 1, ...). 
* `bitrate` Bitrate in bits/sec (e.g. 500000 for 500kbps). 



**Returns:**

true on success. 





        

<hr>



### function syn\_port\_can\_receive 

_Receive a CAN frame (non-blocking)._ 
```C++
bool syn_port_can_receive (
    uint8_t port,
    uint32_t * id,
    bool * extended,
    uint8_t * data,
    uint8_t * dlc
) 
```





**Parameters:**


* `port` CAN port. 
* `id` [out] CAN identifier. 
* `extended` [out] true if 29-bit ID. 
* `data` [out] Frame data buffer (at least 8 bytes). 
* `dlc` [out] Data length code. 



**Returns:**

true if a frame was received. 





        

<hr>



### function syn\_port\_can\_send 

_Send a CAN frame._ 
```C++
bool syn_port_can_send (
    uint8_t port,
    uint32_t id,
    bool extended,
    const uint8_t * data,
    uint8_t dlc
) 
```





**Parameters:**


* `port` CAN port. 
* `id` CAN identifier (11-bit or 29-bit). 
* `extended` true for 29-bit ID. 
* `data` Frame data (up to 8 bytes). 
* `dlc` Data length code (0-8). 



**Returns:**

true if frame was queued for transmission. 





        

<hr>



### function syn\_port\_can\_set\_filter 

_Set hardware acceptance filter._ 
```C++
void syn_port_can_set_filter (
    uint8_t port,
    uint32_t id,
    uint32_t mask
) 
```





**Parameters:**


* `port` CAN port. 
* `id` Filter ID. 
* `mask` Filter mask (1 = must match, 0 = don't care). 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_can.h`

