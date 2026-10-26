

# File syn\_ocpp.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ocpp.c**](syn__ocpp_8c.md)

[Go to the source code of this file](syn__ocpp_8c_source.md)

_Open Charge Point Protocol over JSON (OCPP-J 1.6 / 2.0.1) Dual-Role Engine Implementation._ 

* `#include "syn_ocpp.h"`
* `#include <stdio.h>`
* `#include <stdlib.h>`
* `#include <string.h>`







































## Public Static Functions

| Type | Name |
| ---: | :--- |
|  const char \* | [**status\_to\_str**](#function-status_to_str) ([**SYN\_OCPP\_ChargePointStatus**](syn__ocpp_8h.md#enum-syn_ocpp_chargepointstatus) status) <br>_Convert connector status enumeration to OCPP specification string._  |


























## Public Static Functions Documentation




### function status\_to\_str 

_Convert connector status enumeration to OCPP specification string._ 
```C++
static const char * status_to_str (
    SYN_OCPP_ChargePointStatus status
) 
```





**Parameters:**


* `status` Connector status enum. 



**Returns:**

Const string representation of status. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ocpp.c`

