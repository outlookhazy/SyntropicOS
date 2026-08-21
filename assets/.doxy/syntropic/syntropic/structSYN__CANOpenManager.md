

# Struct SYN\_CANOpenManager



[**ClassList**](annotated.md) **>** [**SYN\_CANOpenManager**](structSYN__CANOpenManager.md)



_CANopen Manager Master State Handle._ 

* `#include <syn_canopen_mgr.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_CANOpenNodeMonitor**](structSYN__CANOpenNodeMonitor.md) | [**nodes**](#variable-nodes)  <br> |
|  [**SYN\_CANOpenSDOClient**](structSYN__CANOpenSDOClient.md) | [**sdo\_client**](#variable-sdo_client)  <br> |












































## Public Attributes Documentation




### variable nodes 

```C++
SYN_CANOpenNodeMonitor SYN_CANOpenManager::nodes[128];
```



Node tracking table (1..127) 


        

<hr>



### variable sdo\_client 

```C++
SYN_CANOpenSDOClient SYN_CANOpenManager::sdo_client;
```



SDO client instance 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_canopen_mgr.h`

