

# Struct SYN\_Packet



[**ClassList**](annotated.md) **>** [**SYN\_Packet**](structSYN__Packet.md)



_Router packet — wire format header + payload buffer._ 

* `#include <syn_router.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**dst**](#variable-dst)  <br> |
|  uint8\_t | [**flags**](#variable-flags)  <br> |
|  uint8\_t | [**len**](#variable-len)  <br> |
|  uint8\_t | [**payload**](#variable-payload)  <br> |
|  uint8\_t | [**seq**](#variable-seq)  <br> |
|  uint8\_t | [**src**](#variable-src)  <br> |
|  uint8\_t | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable dst 

```C++
uint8_t SYN_Packet::dst;
```



Destination (0xFF = broadcast) 
 


        

<hr>



### variable flags 

```C++
uint8_t SYN_Packet::flags;
```



Packet flags 
 


        

<hr>



### variable len 

```C++
uint8_t SYN_Packet::len;
```



Payload length 
 


        

<hr>



### variable payload 

```C++
uint8_t SYN_Packet::payload[SYN_ROUTER_MAX_PAYLOAD];
```



Payload data 
 


        

<hr>



### variable seq 

```C++
uint8_t SYN_Packet::seq;
```



Sequence number 
 


        

<hr>



### variable src 

```C++
uint8_t SYN_Packet::src;
```



Source node ID 
 


        

<hr>



### variable type 

```C++
uint8_t SYN_Packet::type;
```



Message type (app-defined) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_router.h`

