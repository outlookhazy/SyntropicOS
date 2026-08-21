

# File syn\_lwm2m\_task.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lwm2m\_task.h**](syn__lwm2m__task_8h.md)

[Go to the source code of this file](syn__lwm2m__task_8h_source.md)

_Autonomous OMA LwM2M Client Supervisor Task & Engine._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/net/syn_coap.h"`
* `#include "syntropic/proto/syn_lwm2m.h"`
* `#include "syntropic/pt/syn_pt.h"`
* `#include "syntropic/sched/syn_sched.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) <br>_LwM2M Supervisor Task Context._  |
| struct | [**SYN\_LwM2M\_TaskConfig**](structSYN__LwM2M__TaskConfig.md) <br>_Configuration parameters for LwM2M Client Supervisor Task._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_LwM2M\_FactoryResetCb**](#typedef-syn_lwm2m_factoryresetcb)  <br>_Callback for Device Object 3 Factory Reset command (Res 5)._  |
| typedef void(\* | [**SYN\_LwM2M\_FwUpdateCb**](#typedef-syn_lwm2m_fwupdatecb)  <br>_Callback for Firmware Update Object 5 Update command (Res 2)._  |
| typedef void(\* | [**SYN\_LwM2M\_RebootCb**](#typedef-syn_lwm2m_rebootcb)  <br>_Callback for Device Object 3 Reboot command (Res 4)._  |
| typedef void(\* | [**SYN\_LwM2M\_StateChangeCb**](#typedef-syn_lwm2m_statechangecb)  <br>_Callback on client registration state transition._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lwm2m\_task\_deregister**](#function-syn_lwm2m_task_deregister) ([**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) \* task) <br>_Request a graceful LwM2M Deregistration._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lwm2m\_task\_init**](#function-syn_lwm2m_task_init) ([**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) \* task, const [**SYN\_LwM2M\_TaskConfig**](structSYN__LwM2M__TaskConfig.md) \* cfg) <br>_Initialize the LwM2M Supervisor Task context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lwm2m\_task\_notify\_changed**](#function-syn_lwm2m_task_notify_changed) ([**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) \* task, uint16\_t obj\_id, uint16\_t inst\_id, int32\_t res\_id) <br>_Mark an object instance or resource as modified to evaluate observe notifications._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_lwm2m\_task\_pt**](#function-syn_lwm2m_task_pt) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Protothread task entry function for scheduler integration (_ `syn_sched` _)._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lwm2m\_task\_step**](#function-syn_lwm2m_task_step) ([**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) \* task, uint32\_t now\_ms) <br>_Advance the LwM2M Supervisor state machine by one step / tick._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lwm2m\_task\_trigger\_update**](#function-syn_lwm2m_task_trigger_update) ([**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) \* task) <br>_Trigger an immediate LwM2M Registration Update._  |




























## Detailed Description


Implements an event-driven cooperative client task for OMA LwM2M v1.1/v1.2:
* Autonomous Server Registration, 80% Lifetime Renewal, and Deregistration.
* Transport management over UDP or DTLS 1.3 (via [**SYN\_CoapsClient**](structSYN__CoapsClient.md)).
* Downlink CoAP Request Dispatch (Read, Write, Execute, Discover).
* RFC 7641 Observe & Notify scheduler matching periodic pmax & value triggers.
* Standard Device Object (Obj 3) reboot/reset and Firmware Update (Obj 5) dispatch. 




    
## Public Types Documentation




### typedef SYN\_LwM2M\_FactoryResetCb 

_Callback for Device Object 3 Factory Reset command (Res 5)._ 
```C++
typedef void(* SYN_LwM2M_FactoryResetCb) (void *user_data);
```





**Parameters:**


* `user_data` User-defined context pointer. 




        

<hr>



### typedef SYN\_LwM2M\_FwUpdateCb 

_Callback for Firmware Update Object 5 Update command (Res 2)._ 
```C++
typedef void(* SYN_LwM2M_FwUpdateCb) (const char *package_uri, void *user_data);
```





**Parameters:**


* `package_uri` Download URI / package identifier. 
* `user_data` User-defined context pointer. 




        

<hr>



### typedef SYN\_LwM2M\_RebootCb 

_Callback for Device Object 3 Reboot command (Res 4)._ 
```C++
typedef void(* SYN_LwM2M_RebootCb) (void *user_data);
```





**Parameters:**


* `user_data` User-defined context pointer. 




        

<hr>



### typedef SYN\_LwM2M\_StateChangeCb 

_Callback on client registration state transition._ 
```C++
typedef void(* SYN_LwM2M_StateChangeCb) (SYN_LwM2M_ClientState old_state, SYN_LwM2M_ClientState new_state, void *user_data);
```





**Parameters:**


* `old_state` Previous registration state. 
* `new_state` New registration state. 
* `user_data` User-defined context pointer. 




        

<hr>
## Public Functions Documentation




### function syn\_lwm2m\_task\_deregister 

_Request a graceful LwM2M Deregistration._ 
```C++
SYN_Status syn_lwm2m_task_deregister (
    SYN_LwM2M_Task * task
) 
```





**Parameters:**


* `task` Task instance. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if task is NULL. 





        

<hr>



### function syn\_lwm2m\_task\_init 

_Initialize the LwM2M Supervisor Task context._ 
```C++
SYN_Status syn_lwm2m_task_init (
    SYN_LwM2M_Task * task,
    const SYN_LwM2M_TaskConfig * cfg
) 
```





**Parameters:**


* `task` Task instance to initialize. 
* `cfg` Task configuration parameters. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL or invalid arguments. 





        

<hr>



### function syn\_lwm2m\_task\_notify\_changed 

_Mark an object instance or resource as modified to evaluate observe notifications._ 
```C++
SYN_Status syn_lwm2m_task_notify_changed (
    SYN_LwM2M_Task * task,
    uint16_t obj_id,
    uint16_t inst_id,
    int32_t res_id
) 
```





**Parameters:**


* `task` Task instance. 
* `obj_id` Object ID that changed. 
* `inst_id` Instance ID that changed. 
* `res_id` Resource ID that changed (-1 for entire instance). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if task is NULL. 





        

<hr>



### function syn\_lwm2m\_task\_pt 

_Protothread task entry function for scheduler integration (_ `syn_sched` _)._
```C++
SYN_PT_Status syn_lwm2m_task_pt (
    SYN_PT * pt,
    SYN_Task * task
) 
```





**Parameters:**


* `pt` Protothread continuation structure. 
* `task` Generic scheduler task descriptor containing [**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) in user\_data. 



**Returns:**

SYN\_PT\_Status continuation status. 





        

<hr>



### function syn\_lwm2m\_task\_step 

_Advance the LwM2M Supervisor state machine by one step / tick._ 
```C++
SYN_Status syn_lwm2m_task_step (
    SYN_LwM2M_Task * task,
    uint32_t now_ms
) 
```



Handles DTLS handshake progress, registration renewal timers, receiving incoming CoAP requests, dispatching responses, evaluating observation rules, and firing notifications.




**Parameters:**


* `task` Task instance. 
* `now_ms` Current system timestamp in milliseconds. 



**Returns:**

SYN\_OK on normal operation, error code on fatal condition. 





        

<hr>



### function syn\_lwm2m\_task\_trigger\_update 

_Trigger an immediate LwM2M Registration Update._ 
```C++
SYN_Status syn_lwm2m_task_trigger_update (
    SYN_LwM2M_Task * task
) 
```





**Parameters:**


* `task` Task instance. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if task is NULL. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m_task.h`

