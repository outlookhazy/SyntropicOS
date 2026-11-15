

# Struct SYN\_OCPP\_PendingCall



[**ClassList**](annotated.md) **>** [**SYN\_OCPP\_PendingCall**](structSYN__OCPP__PendingCall.md)



_In-flight OCPP outbound Call tracking entry._ 

* `#include <syn_ocpp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_OCPP\_PendingAction**](syn__ocpp_8h.md#enum-syn_ocpp_pendingaction) | [**action**](#variable-action)  <br> |
|  char | [**unique\_id**](#variable-unique_id)  <br> |












































## Public Attributes Documentation




### variable action 

```C++
SYN_OCPP_PendingAction SYN_OCPP_PendingCall::action;
```



Pending OCPP Call action 


        

<hr>



### variable unique\_id 

```C++
char SYN_OCPP_PendingCall::unique_id[16];
```



UniqueId of outbound Call 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ocpp.h`

