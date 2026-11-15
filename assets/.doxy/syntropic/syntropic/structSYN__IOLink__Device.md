

# Struct SYN\_IOLink\_Device



[**ClassList**](annotated.md) **>** [**SYN\_IOLink\_Device**](structSYN__IOLink__Device.md)



_IO-Link Device instance context._ 

* `#include <syn_iolink.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_IOLink\_DeviceConfig**](structSYN__IOLink__DeviceConfig.md) | [**cfg**](#variable-cfg)  <br> |
|  uint8\_t | [**pd\_in**](#variable-pd_in)  <br> |
|  uint8\_t | [**pd\_out**](#variable-pd_out)  <br> |
|  [**SYN\_IOLink\_PortState**](syn__iolink_8h.md#enum-syn_iolink_portstate) | [**state**](#variable-state)  <br> |












































## Public Attributes Documentation




### variable cfg 

```C++
SYN_IOLink_DeviceConfig SYN_IOLink_Device::cfg;
```



Configuration 


        

<hr>



### variable pd\_in 

```C++
uint8_t SYN_IOLink_Device::pd_in[SYN_IOLINK_MAX_PD_LEN];
```



Sensor input process data buffer 


        

<hr>



### variable pd\_out 

```C++
uint8_t SYN_IOLink_Device::pd_out[SYN_IOLINK_MAX_PD_LEN];
```



Actuator output process data buffer 


        

<hr>



### variable state 

```C++
SYN_IOLink_PortState SYN_IOLink_Device::state;
```



Port state 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_iolink.h`

