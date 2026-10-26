

# File syn\_lwm2m\_task.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lwm2m\_task.c**](syn__lwm2m__task_8c.md)

[Go to the source code of this file](syn__lwm2m__task_8c_source.md)

_Autonomous OMA LwM2M Client Supervisor Task & Engine Implementation._ 

* `#include "../port/syn_port_system.h"`
* `#include "../util/syn_assert.h"`
* `#include "syn_lwm2m_task.h"`
* `#include <stdio.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lwm2m\_task\_deregister**](#function-syn_lwm2m_task_deregister) ([**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) \* task) <br>_Request a graceful LwM2M Deregistration._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lwm2m\_task\_init**](#function-syn_lwm2m_task_init) ([**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) \* task, const [**SYN\_LwM2M\_TaskConfig**](structSYN__LwM2M__TaskConfig.md) \* cfg) <br>_Initialize the LwM2M Supervisor Task context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lwm2m\_task\_notify\_changed**](#function-syn_lwm2m_task_notify_changed) ([**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) \* task, uint16\_t obj\_id, uint16\_t inst\_id, int32\_t res\_id) <br>_Mark an object instance or resource as modified to evaluate observe notifications._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_lwm2m\_task\_pt**](#function-syn_lwm2m_task_pt) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Protothread task entry function for scheduler integration (_ `syn_sched` _)._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lwm2m\_task\_step**](#function-syn_lwm2m_task_step) ([**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) \* task, uint32\_t now\_ms) <br>_Advance the LwM2M Supervisor state machine by one step / tick._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lwm2m\_task\_trigger\_update**](#function-syn_lwm2m_task_trigger_update) ([**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) \* task) <br>_Trigger an immediate LwM2M Registration Update._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_lwm2m\_task\_check\_object\_hooks**](#function-syn_lwm2m_task_check_object_hooks) ([**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) \* task) <br>_Check registered objects for pending command flags (reboot, reset, fw update)._  |
|  void | [**syn\_lwm2m\_task\_eval\_observations**](#function-syn_lwm2m_task_eval_observations) ([**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) \* task, uint32\_t now\_ms) <br>_Evaluate active observations against resource changes and pmax intervals._  |
|  void | [**syn\_lwm2m\_task\_set\_state**](#function-syn_lwm2m_task_set_state) ([**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md) \* task, [**SYN\_LwM2M\_ClientState**](syn__lwm2m_8h.md#enum-syn_lwm2m_clientstate) new\_state) <br>_Update client state and fire state change callback if transition occurs._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_LWM2M\_COAP\_SERIALIZE\_BUF\_SZ**](syn__lwm2m__task_8c.md#define-syn_lwm2m_coap_serialize_buf_sz)  `512U`<br> |
| define  | [**SYN\_LWM2M\_DEFAULT\_RETRY\_BACKOFF\_MS**](syn__lwm2m__task_8c.md#define-syn_lwm2m_default_retry_backoff_ms)  `5000U`<br> |
| define  | [**SYN\_LWM2M\_RESP\_PAYLOAD\_BUF\_SZ**](syn__lwm2m__task_8c.md#define-syn_lwm2m_resp_payload_buf_sz)  `256U`<br> |

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
## Public Static Functions Documentation




### function syn\_lwm2m\_task\_check\_object\_hooks 

_Check registered objects for pending command flags (reboot, reset, fw update)._ 
```C++
static void syn_lwm2m_task_check_object_hooks (
    SYN_LwM2M_Task * task
) 
```





**Parameters:**


* `task` LwM2M supervisor task instance. 




        

<hr>



### function syn\_lwm2m\_task\_eval\_observations 

_Evaluate active observations against resource changes and pmax intervals._ 
```C++
static void syn_lwm2m_task_eval_observations (
    SYN_LwM2M_Task * task,
    uint32_t now_ms
) 
```





**Parameters:**


* `task` LwM2M supervisor task instance. 
* `now_ms` Current system timestamp in milliseconds. 




        

<hr>



### function syn\_lwm2m\_task\_set\_state 

_Update client state and fire state change callback if transition occurs._ 
```C++
static void syn_lwm2m_task_set_state (
    SYN_LwM2M_Task * task,
    SYN_LwM2M_ClientState new_state
) 
```





**Parameters:**


* `task` LwM2M supervisor task instance. 
* `new_state` Target client registration state. 




        

<hr>
## Macro Definition Documentation





### define SYN\_LWM2M\_COAP\_SERIALIZE\_BUF\_SZ 

```C++
#define SYN_LWM2M_COAP_SERIALIZE_BUF_SZ `512U`
```



Scratch buffer size for serialization 


        

<hr>



### define SYN\_LWM2M\_DEFAULT\_RETRY\_BACKOFF\_MS 

```C++
#define SYN_LWM2M_DEFAULT_RETRY_BACKOFF_MS `5000U`
```



Default network retry backoff (ms) 


        

<hr>



### define SYN\_LWM2M\_RESP\_PAYLOAD\_BUF\_SZ 

```C++
#define SYN_LWM2M_RESP_PAYLOAD_BUF_SZ `256U`
```



Response payload scratch size 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m_task.c`

