

# Struct SYN\_CIP\_DeviceNet



[**ClassList**](annotated.md) **>** [**SYN\_CIP\_DeviceNet**](structSYN__CIP__DeviceNet.md)



_CIP DeviceNet Object (Class 0x03) Attributes._ 

* `#include <syn_devicenet.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**allocation\_choice**](#variable-allocation_choice)  <br> |
|  [**SYN\_DeviceNet\_Baud**](syn__devicenet_8h.md#enum-syn_devicenet_baud) | [**baud\_rate**](#variable-baud_rate)  <br> |
|  uint8\_t | [**bus\_off\_action**](#variable-bus_off_action)  <br> |
|  uint8\_t | [**mac\_id**](#variable-mac_id)  <br> |
|  uint8\_t | [**master\_mac\_id**](#variable-master_mac_id)  <br> |












































## Public Attributes Documentation




### variable allocation\_choice 

```C++
uint8_t SYN_CIP_DeviceNet::allocation_choice;
```



Allocation choice byte 


        

<hr>



### variable baud\_rate 

```C++
SYN_DeviceNet_Baud SYN_CIP_DeviceNet::baud_rate;
```



Configured CAN baud rate 


        

<hr>



### variable bus\_off\_action 

```C++
uint8_t SYN_CIP_DeviceNet::bus_off_action;
```



Bus-off reset action 


        

<hr>



### variable mac\_id 

```C++
uint8_t SYN_CIP_DeviceNet::mac_id;
```



Node MAC ID address 


        

<hr>



### variable master\_mac\_id 

```C++
uint8_t SYN_CIP_DeviceNet::master_mac_id;
```



Master node MAC ID 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_devicenet.h`

