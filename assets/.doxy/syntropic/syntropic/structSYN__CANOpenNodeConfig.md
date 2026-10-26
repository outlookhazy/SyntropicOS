

# Struct SYN\_CANOpenNodeConfig



[**ClassList**](annotated.md) **>** [**SYN\_CANOpenNodeConfig**](structSYN__CANOpenNodeConfig.md)



_CANopen Node Configuration._ 

* `#include <syn_canopen.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**heartbeat\_ms**](#variable-heartbeat_ms)  <br> |
|  uint8\_t | [**node\_id**](#variable-node_id)  <br> |
|  [**SYN\_CANOpenPDOMap**](structSYN__CANOpenPDOMap.md) | [**rpdo**](#variable-rpdo)  <br> |
|  [**SYN\_CANOpenPDOMap**](structSYN__CANOpenPDOMap.md) | [**tpdo**](#variable-tpdo)  <br> |












































## Public Attributes Documentation




### variable heartbeat\_ms 

```C++
uint16_t SYN_CANOpenNodeConfig::heartbeat_ms;
```



Heartbeat period in ms (0 = disabled) 


        

<hr>



### variable node\_id 

```C++
uint8_t SYN_CANOpenNodeConfig::node_id;
```



Node-ID (1..127) 


        

<hr>



### variable rpdo 

```C++
SYN_CANOpenPDOMap SYN_CANOpenNodeConfig::rpdo[4];
```



Receive PDO Mappings 1..4 


        

<hr>



### variable tpdo 

```C++
SYN_CANOpenPDOMap SYN_CANOpenNodeConfig::tpdo[4];
```



Transmit PDO Mappings 1..4 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_canopen.h`

