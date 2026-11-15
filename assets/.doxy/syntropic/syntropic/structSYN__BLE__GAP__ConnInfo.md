

# Struct SYN\_BLE\_GAP\_ConnInfo



[**ClassList**](annotated.md) **>** [**SYN\_BLE\_GAP\_ConnInfo**](structSYN__BLE__GAP__ConnInfo.md)



[More...](#detailed-description)

* `#include <syn_ble_gap.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**conn\_handle**](#variable-conn_handle)  <br> |
|  uint16\_t | [**conn\_interval**](#variable-conn_interval)  <br> |
|  uint16\_t | [**conn\_latency**](#variable-conn_latency)  <br> |
|  uint8\_t | [**peer\_addr**](#variable-peer_addr)  <br> |
|  uint8\_t | [**peer\_addr\_type**](#variable-peer_addr_type)  <br> |
|  uint16\_t | [**supervision\_timeout**](#variable-supervision_timeout)  <br> |












































## Detailed Description


Connection Info 


    
## Public Attributes Documentation




### variable conn\_handle 

```C++
uint16_t SYN_BLE_GAP_ConnInfo::conn_handle;
```



Connection handle 


        

<hr>



### variable conn\_interval 

```C++
uint16_t SYN_BLE_GAP_ConnInfo::conn_interval;
```



Connection interval (units 1.25ms) 


        

<hr>



### variable conn\_latency 

```C++
uint16_t SYN_BLE_GAP_ConnInfo::conn_latency;
```



Slave latency 


        

<hr>



### variable peer\_addr 

```C++
uint8_t SYN_BLE_GAP_ConnInfo::peer_addr[6];
```



6-byte peer MAC address 


        

<hr>



### variable peer\_addr\_type 

```C++
uint8_t SYN_BLE_GAP_ConnInfo::peer_addr_type;
```



Peer address type 


        

<hr>



### variable supervision\_timeout 

```C++
uint16_t SYN_BLE_GAP_ConnInfo::supervision_timeout;
```



Supervision timeout (units 10ms) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_gap.h`

