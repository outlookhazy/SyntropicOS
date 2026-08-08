

# Struct SYN\_PendingAck



[**ClassList**](annotated.md) **>** [**SYN\_PendingAck**](structSYN__PendingAck.md)



_Pending ACK entry for reliable delivery tracking._ 

* `#include <syn_router.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  uint8\_t | [**dst**](#variable-dst)  <br> |
|  uint8\_t | [**len**](#variable-len)  <br> |
|  uint8\_t | [**payload**](#variable-payload)  <br> |
|  uint8\_t | [**retries**](#variable-retries)  <br> |
|  uint32\_t | [**sent\_tick**](#variable-sent_tick)  <br> |
|  uint8\_t | [**seq**](#variable-seq)  <br> |
|  uint8\_t | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_PendingAck::active;
```



Slot in use 
 


        

<hr>



### variable dst 

```C++
uint8_t SYN_PendingAck::dst;
```



Destination node ID 
 


        

<hr>



### variable len 

```C++
uint8_t SYN_PendingAck::len;
```



Payload length 
 


        

<hr>



### variable payload 

```C++
uint8_t SYN_PendingAck::payload[SYN_ROUTER_MAX_PAYLOAD];
```



Payload copy 
 


        

<hr>



### variable retries 

```C++
uint8_t SYN_PendingAck::retries;
```



Retransmission count 
 


        

<hr>



### variable sent\_tick 

```C++
uint32_t SYN_PendingAck::sent_tick;
```



Tick when last sent 
 


        

<hr>



### variable seq 

```C++
uint8_t SYN_PendingAck::seq;
```



Sequence number 
 


        

<hr>



### variable type 

```C++
uint8_t SYN_PendingAck::type;
```



Message type 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_router.h`

