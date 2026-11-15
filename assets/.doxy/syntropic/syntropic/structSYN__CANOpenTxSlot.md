

# Struct SYN\_CANOpenTxSlot



[**ClassList**](annotated.md) **>** [**SYN\_CANOpenTxSlot**](structSYN__CANOpenTxSlot.md)



_Queued outbound CANopen frame slot._ 

* `#include <syn_canopen.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**cob\_id**](#variable-cob_id)  <br> |
|  uint8\_t | [**data**](#variable-data)  <br> |
|  uint8\_t | [**len**](#variable-len)  <br> |












































## Public Attributes Documentation




### variable cob\_id 

```C++
uint32_t SYN_CANOpenTxSlot::cob_id;
```



11-bit CAN COB-ID 


        

<hr>



### variable data 

```C++
uint8_t SYN_CANOpenTxSlot::data[8];
```



Frame payload bytes 


        

<hr>



### variable len 

```C++
uint8_t SYN_CANOpenTxSlot::len;
```



Frame payload length (0..8) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_canopen.h`

