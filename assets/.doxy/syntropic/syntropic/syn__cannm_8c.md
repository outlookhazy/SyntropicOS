

# File syn\_cannm.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cannm.c**](syn__cannm_8c.md)

[Go to the source code of this file](syn__cannm_8c_source.md)

_AUTOSAR CAN Network Management (CanNm) Protocol Implementation._ 

* `#include "syn_cannm.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_cannm\_init**](#function-syn_cannm_init) ([**SYN\_CanNM\_Session**](structSYN__CanNM__Session.md) \* session, const [**SYN\_CanNM\_Config**](structSYN__CanNM__Config.md) \* cfg) <br>_Initialize CAN NM Session._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cannm\_process\_rx\_frame**](#function-syn_cannm_process_rx_frame) ([**SYN\_CanNM\_Session**](structSYN__CanNM__Session.md) \* session, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Process an incoming CAN frame for CAN NM filtering and state machine._  |
|  void | [**syn\_cannm\_release\_network**](#function-syn_cannm_release_network) ([**SYN\_CanNM\_Session**](structSYN__CanNM__Session.md) \* session) <br>_Release network request (allow transition to sleep)._  |
|  void | [**syn\_cannm\_request\_network**](#function-syn_cannm_request_network) ([**SYN\_CanNM\_Session**](structSYN__CanNM__Session.md) \* session) <br>_Request network active operation (keep awake / active wakeup)._  |
|  void | [**syn\_cannm\_request\_repeat\_msg**](#function-syn_cannm_request_repeat_msg) ([**SYN\_CanNM\_Session**](structSYN__CanNM__Session.md) \* session) <br>_Request Repeat Message state transition._  |
|  void | [**syn\_cannm\_set\_user\_data**](#function-syn_cannm_set_user_data) ([**SYN\_CanNM\_Session**](structSYN__CanNM__Session.md) \* session, const uint8\_t \* data, size\_t len) <br>_Set local NM user payload bytes (6 bytes max)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cannm\_step**](#function-syn_cannm_step) ([**SYN\_CanNM\_Session**](structSYN__CanNM__Session.md) \* session, uint32\_t delta\_ms, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* tx\_frame) <br>_Step CAN NM state machine and timer tick._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**pack\_nm\_tx\_pdu**](#function-pack_nm_tx_pdu) ([**SYN\_CanNM\_Session**](structSYN__CanNM__Session.md) \* session, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* tx\_frame) <br>_Pack CAN NM PDU payload into tx\_frame._  |


























## Public Functions Documentation




### function syn\_cannm\_init 

_Initialize CAN NM Session._ 
```C++
void syn_cannm_init (
    SYN_CanNM_Session * session,
    const SYN_CanNM_Config * cfg
) 
```





**Parameters:**


* `session` Session handle pointer 
* `cfg` Configuration structure (or NULL for defaults) 




        

<hr>



### function syn\_cannm\_process\_rx\_frame 

_Process an incoming CAN frame for CAN NM filtering and state machine._ 
```C++
bool syn_cannm_process_rx_frame (
    SYN_CanNM_Session * session,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `session` Session handle pointer 
* `frame` Incoming CAN frame pointer 



**Returns:**

true if frame was a valid matching CAN NM PDU 





        

<hr>



### function syn\_cannm\_release\_network 

_Release network request (allow transition to sleep)._ 
```C++
void syn_cannm_release_network (
    SYN_CanNM_Session * session
) 
```





**Parameters:**


* `session` Session handle pointer 




        

<hr>



### function syn\_cannm\_request\_network 

_Request network active operation (keep awake / active wakeup)._ 
```C++
void syn_cannm_request_network (
    SYN_CanNM_Session * session
) 
```





**Parameters:**


* `session` Session handle pointer 




        

<hr>



### function syn\_cannm\_request\_repeat\_msg 

_Request Repeat Message state transition._ 
```C++
void syn_cannm_request_repeat_msg (
    SYN_CanNM_Session * session
) 
```





**Parameters:**


* `session` Session handle pointer 




        

<hr>



### function syn\_cannm\_set\_user\_data 

_Set local NM user payload bytes (6 bytes max)._ 
```C++
void syn_cannm_set_user_data (
    SYN_CanNM_Session * session,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `session` Session handle pointer 
* `data` Data pointer 
* `len` Data length (0 to 6 bytes) 




        

<hr>



### function syn\_cannm\_step 

_Step CAN NM state machine and timer tick._ 
```C++
bool syn_cannm_step (
    SYN_CanNM_Session * session,
    uint32_t delta_ms,
    SYN_CAN_Frame * tx_frame
) 
```





**Parameters:**


* `session` Session handle pointer 
* `delta_ms` Elapsed time in milliseconds 
* `tx_frame` Pointer to CAN frame structure to receive pending CAN NM message 



**Returns:**

true if a CAN frame is ready in tx\_frame to be transmitted 





        

<hr>
## Public Static Functions Documentation




### function pack\_nm\_tx\_pdu 

_Pack CAN NM PDU payload into tx\_frame._ 
```C++
static void pack_nm_tx_pdu (
    SYN_CanNM_Session * session,
    SYN_CAN_Frame * tx_frame
) 
```





**Parameters:**


* `session` Pointer to CAN NM session instance. 
* `tx_frame` Pointer to target CAN frame structure. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cannm.c`

