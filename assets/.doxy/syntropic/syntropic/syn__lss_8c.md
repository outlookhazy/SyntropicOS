

# File syn\_lss.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lss.c**](syn__lss_8c.md)

[Go to the source code of this file](syn__lss_8c_source.md)

_CANopen CiA 305 Layer Setting Services (LSS) Implementation._ 

* `#include "syn_lss.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_lss\_master\_build\_config\_bit\_timing**](#function-syn_lss_master_build_config_bit_timing) ([**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, uint8\_t baud\_table\_idx) <br>_Build an LSS Master request frame to configure Bit Timing / Baud rate._  |
|  void | [**syn\_lss\_master\_build\_config\_node\_id**](#function-syn_lss_master_build_config_node_id) ([**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, uint8\_t new\_node\_id) <br>_Build an LSS Master request frame to configure Node-ID._  |
|  void | [**syn\_lss\_master\_build\_store\_config**](#function-syn_lss_master_build_store_config) ([**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Build an LSS Master request frame to store configuration to NVM._  |
|  void | [**syn\_lss\_master\_build\_switch\_mode**](#function-syn_lss_master_build_switch_mode) ([**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, uint8\_t mode) <br>_Build an LSS Master request frame to switch global mode._  |
|  void | [**syn\_lss\_slave\_init**](#function-syn_lss_slave_init) ([**SYN\_LSSSlave**](structSYN__LSSSlave.md) \* slave, const [**SYN\_LSSAddress**](structSYN__LSSAddress.md) \* addr, uint8\_t initial\_node\_id) <br>_Initialize LSS Slave state instance._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lss\_slave\_process**](#function-syn_lss_slave_process) ([**SYN\_LSSSlave**](structSYN__LSSSlave.md) \* slave, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* resp) <br>_Process an incoming CAN frame for LSS Slave processing._  |




























## Public Functions Documentation




### function syn\_lss\_master\_build\_config\_bit\_timing 

_Build an LSS Master request frame to configure Bit Timing / Baud rate._ 
```C++
void syn_lss_master_build_config_bit_timing (
    SYN_CAN_Frame * frame,
    uint8_t baud_table_idx
) 
```





**Parameters:**


* `frame` Output frame buffer. 
* `baud_table_idx` Baud rate index (SYN\_LSS\_BAUD\_\*). 




        

<hr>



### function syn\_lss\_master\_build\_config\_node\_id 

_Build an LSS Master request frame to configure Node-ID._ 
```C++
void syn_lss_master_build_config_node_id (
    SYN_CAN_Frame * frame,
    uint8_t new_node_id
) 
```





**Parameters:**


* `frame` Output frame buffer. 
* `new_node_id` Target Node-ID (1..127). 




        

<hr>



### function syn\_lss\_master\_build\_store\_config 

_Build an LSS Master request frame to store configuration to NVM._ 
```C++
void syn_lss_master_build_store_config (
    SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `frame` Output frame buffer. 




        

<hr>



### function syn\_lss\_master\_build\_switch\_mode 

_Build an LSS Master request frame to switch global mode._ 
```C++
void syn_lss_master_build_switch_mode (
    SYN_CAN_Frame * frame,
    uint8_t mode
) 
```





**Parameters:**


* `frame` Output frame buffer. 
* `mode` Target LSS mode (SYN\_LSS\_MODE\_OPERATION or SYN\_LSS\_MODE\_CONFIGURATION). 




        

<hr>



### function syn\_lss\_slave\_init 

_Initialize LSS Slave state instance._ 
```C++
void syn_lss_slave_init (
    SYN_LSSSlave * slave,
    const SYN_LSSAddress * addr,
    uint8_t initial_node_id
) 
```





**Parameters:**


* `slave` Pointer to slave handle. 
* `addr` Pointer to LSS unique 128-bit identity address. 
* `initial_node_id` Starting node-ID (or 0xFF for unconfigured). 




        

<hr>



### function syn\_lss\_slave\_process 

_Process an incoming CAN frame for LSS Slave processing._ 
```C++
bool syn_lss_slave_process (
    SYN_LSSSlave * slave,
    const SYN_CAN_Frame * frame,
    SYN_CAN_Frame * resp
) 
```





**Parameters:**


* `slave` Pointer to LSS Slave handle. 
* `frame` Incoming CAN frame (COB-ID 0x7E5). 
* `resp` Output CAN frame buffer (populated with 0x7E4 response if required). 



**Returns:**

True if a response frame should be transmitted over CAN. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lss.c`

