

# Struct SYN\_DoIP\_Server



[**ClassList**](annotated.md) **>** [**SYN\_DoIP\_Server**](structSYN__DoIP__Server.md)



_DoIP Server State Structure._ 

* `#include <syn_doip.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**active\_tester\_address**](#variable-active_tester_address)  <br> |
|  uint8\_t | [**eid**](#variable-eid)  <br> |
|  uint8\_t | [**gid**](#variable-gid)  <br> |
|  uint16\_t | [**logical\_address**](#variable-logical_address)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**routing\_activated**](#variable-routing_activated)  <br> |
|  uint8\_t | [**vin**](#variable-vin)  <br> |












































## Public Attributes Documentation




### variable active\_tester\_address 

```C++
uint16_t SYN_DoIP_Server::active_tester_address;
```



Logical address of activated diagnostic tester 


        

<hr>



### variable eid 

```C++
uint8_t SYN_DoIP_Server::eid[6];
```



Entity Identifier (MAC Address) 


        

<hr>



### variable gid 

```C++
uint8_t SYN_DoIP_Server::gid[6];
```



Group Identifier 


        

<hr>



### variable logical\_address 

```C++
uint16_t SYN_DoIP_Server::logical_address;
```



Server / ECU logical address (e.g. 0x1001) 


        

<hr>



### variable routing\_activated 

```C++
bool SYN_DoIP_Server::routing_activated;
```



Routing activation flag 


        

<hr>



### variable vin 

```C++
uint8_t SYN_DoIP_Server::vin[17];
```



Vehicle Identification Number 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_doip.h`

