

# Struct SYN\_CANOpenSDOClient



[**ClassList**](annotated.md) **>** [**SYN\_CANOpenSDOClient**](structSYN__CANOpenSDOClient.md)



_SDO Client Transaction Handle._ 

* `#include <syn_canopen_mgr.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**uint32\_t**](structSYN__CANOpenSDOClient.md#variable-index) | [**abort\_code**](#variable-abort_code)  <br> |
|  [**uint8\_t**](structSYN__CANOpenSDOClient.md#variable-index) | [**data**](#variable-data)  <br> |
|  [**size\_t**](structSYN__CANOpenSDOClient.md#variable-index) | [**data\_len**](#variable-data_len)  <br> |
|  uint16\_t | [**index**](#variable-index)  <br> |
|  [**SYN\_SDOClientState**](syn__canopen__mgr_8h.md#enum-syn_sdoclientstate) | [**state**](#variable-state)  <br> |
|  [**uint8\_t**](structSYN__CANOpenSDOClient.md#variable-index) | [**subindex**](#variable-subindex)  <br> |
|  [**uint8\_t**](structSYN__CANOpenSDOClient.md#variable-index) | [**target\_node**](#variable-target_node)  <br> |
|  [**uint16\_t**](structSYN__CANOpenSDOClient.md#variable-index) | [**timeout\_ms**](#variable-timeout_ms)  <br> |












































## Public Attributes Documentation




### variable abort\_code 

```C++
uint32_t SYN_CANOpenSDOClient::abort_code;
```



SDO abort code if error occurred 


        

<hr>



### variable data 

```C++
uint8_t SYN_CANOpenSDOClient::data[8];
```



Read/Write data buffer 


        

<hr>



### variable data\_len 

```C++
size_t SYN_CANOpenSDOClient::data_len;
```



Length of payload data 


        

<hr>



### variable index 

```C++
uint16_t SYN_CANOpenSDOClient::index;
```



Target OD Index 


        

<hr>



### variable state 

```C++
SYN_SDOClientState SYN_CANOpenSDOClient::state;
```



Current SDO state 


        

<hr>



### variable subindex 

```C++
uint8_t SYN_CANOpenSDOClient::subindex;
```



Target OD Subindex 


        

<hr>



### variable target\_node 

```C++
uint8_t SYN_CANOpenSDOClient::target_node;
```



Target slave Node-ID (1..127) 


        

<hr>



### variable timeout\_ms 

```C++
uint16_t SYN_CANOpenSDOClient::timeout_ms;
```



Transaction timeout timer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_canopen_mgr.h`

