

# Struct SYN\_CANOpenNodeMonitor



[**ClassList**](annotated.md) **>** [**SYN\_CANOpenNodeMonitor**](structSYN__CANOpenNodeMonitor.md)



_Remote Node Heartbeat Monitor Entry._ 

* `#include <syn_canopen_mgr.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**nmt\_state**](#variable-nmt_state)  <br> |
|  uint8\_t | [**node\_id**](#variable-node_id)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**online**](#variable-online)  <br> |
|  uint16\_t | [**timer\_ms**](#variable-timer_ms)  <br> |












































## Public Attributes Documentation




### variable nmt\_state 

```C++
uint8_t SYN_CANOpenNodeMonitor::nmt_state;
```



Last known NMT state 


        

<hr>



### variable node\_id 

```C++
uint8_t SYN_CANOpenNodeMonitor::node_id;
```



Remote node-ID 


        

<hr>



### variable online 

```C++
bool SYN_CANOpenNodeMonitor::online;
```



True if node is active and within timeout 


        

<hr>



### variable timer\_ms 

```C++
uint16_t SYN_CANOpenNodeMonitor::timer_ms;
```



Elapsed time since last heartbeat 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_canopen_mgr.h`

