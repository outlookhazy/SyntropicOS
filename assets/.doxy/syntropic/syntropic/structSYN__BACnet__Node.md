

# Struct SYN\_BACnet\_Node



[**ClassList**](annotated.md) **>** [**SYN\_BACnet\_Node**](structSYN__BACnet__Node.md)



_BACnet MS/TP Node Context._ 

* `#include <syn_bacnet.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**device\_id**](#variable-device_id)  <br> |
|  uint8\_t | [**mac\_address**](#variable-mac_address)  <br> |
|  uint8\_t | [**max\_master**](#variable-max_master)  <br> |
|  size\_t | [**object\_count**](#variable-object_count)  <br> |
|  [**SYN\_BACnet\_Object**](structSYN__BACnet__Object.md) | [**objects**](#variable-objects)  <br> |












































## Public Attributes Documentation




### variable device\_id 

```C++
uint32_t SYN_BACnet_Node::device_id;
```



BACnet Device Object Instance 


        

<hr>



### variable mac\_address 

```C++
uint8_t SYN_BACnet_Node::mac_address;
```



Local MS/TP MAC (0..127) 


        

<hr>



### variable max\_master 

```C++
uint8_t SYN_BACnet_Node::max_master;
```



Max Master MAC (default 127) 


        

<hr>



### variable object\_count 

```C++
size_t SYN_BACnet_Node::object_count;
```



Active Object count 


        

<hr>



### variable objects 

```C++
SYN_BACnet_Object SYN_BACnet_Node::objects[SYN_BACNET_MAX_OBJECTS];
```



Static Object Table 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_bacnet.h`

