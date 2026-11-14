

# File syn\_lwm2m\_task.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lwm2m\_task.h**](syn__lwm2m__task_8h.md)

[Go to the source code of this file](syn__lwm2m__task_8h_source.md)

_Autonomous OMA LwM2M Client Supervisor Task & Engine._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/proto/syn_lwm2m.h"`
































































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_LWM2M\_TASK\_ENABLED**](syn__lwm2m__task_8h.md#define-syn_lwm2m_task_enabled)  `0`<br>_Non-zero when LwM2M autonomous task and scheduler dependencies are enabled._  |

## Detailed Description


Implements an event-driven cooperative client task for OMA LwM2M v1.1/v1.2:
* Autonomous Server Registration, 80% Lifetime Renewal, and Deregistration.
* Transport management over UDP or DTLS 1.3 (via [**SYN\_CoapsClient**](syn__coap_8h.md#typedef-syn_coapsclient)).
* Downlink CoAP Request Dispatch (Read, Write, Execute, Discover).
* RFC 7641 Observe & Notify scheduler matching periodic pmax & value triggers.
* Standard Device Object (Obj 3) reboot/reset and Firmware Update (Obj 5) dispatch. 




    
## Macro Definition Documentation





### define SYN\_LWM2M\_TASK\_ENABLED 

_Non-zero when LwM2M autonomous task and scheduler dependencies are enabled._ 
```C++
#define SYN_LWM2M_TASK_ENABLED `0`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m_task.h`

