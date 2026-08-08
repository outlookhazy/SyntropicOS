

# Struct SYN\_DeviceNet\_Node



[**ClassList**](annotated.md) **>** [**SYN\_DeviceNet\_Node**](structSYN__DeviceNet__Node.md)



_DeviceNet Node Context Structure._ 

* `#include <syn_devicenet.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_CIP\_Assembly**](structSYN__CIP__Assembly.md) | [**assembly**](#variable-assembly)  <br> |
|  [**SYN\_CIP\_DeviceNet**](structSYN__CIP__DeviceNet.md) | [**devicenet\_obj**](#variable-devicenet_obj)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dup\_mac\_failed**](#variable-dup_mac_failed)  <br> |
|  uint32\_t | [**dup\_mac\_timer\_ms**](#variable-dup_mac_timer_ms)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**explicit\_connected**](#variable-explicit_connected)  <br> |
|  [**SYN\_CIP\_Identity**](structSYN__CIP__Identity.md) | [**identity**](#variable-identity)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**polled\_connected**](#variable-polled_connected)  <br> |
|  [**SYN\_DeviceNet\_State**](syn__devicenet_8h.md#enum-syn_devicenet_state) | [**state**](#variable-state)  <br> |












































## Public Attributes Documentation




### variable assembly 

```C++
SYN_CIP_Assembly SYN_DeviceNet_Node::assembly;
```



CIP Assembly object context 


        

<hr>



### variable devicenet\_obj 

```C++
SYN_CIP_DeviceNet SYN_DeviceNet_Node::devicenet_obj;
```



CIP DeviceNet object attributes 


        

<hr>



### variable dup\_mac\_failed 

```C++
bool SYN_DeviceNet_Node::dup_mac_failed;
```



Duplicate MAC check failure flag 


        

<hr>



### variable dup\_mac\_timer\_ms 

```C++
uint32_t SYN_DeviceNet_Node::dup_mac_timer_ms;
```



Duplicate MAC check timer in ms 


        

<hr>



### variable explicit\_connected 

```C++
bool SYN_DeviceNet_Node::explicit_connected;
```



Explicit connection active flag 


        

<hr>



### variable identity 

```C++
SYN_CIP_Identity SYN_DeviceNet_Node::identity;
```



CIP Identity object attributes 


        

<hr>



### variable polled\_connected 

```C++
bool SYN_DeviceNet_Node::polled_connected;
```



Polled I/O connection active flag 


        

<hr>



### variable state 

```C++
SYN_DeviceNet_State SYN_DeviceNet_Node::state;
```



Current node state 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_devicenet.h`

