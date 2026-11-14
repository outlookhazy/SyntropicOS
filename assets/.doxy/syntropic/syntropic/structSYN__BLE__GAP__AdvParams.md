

# Struct SYN\_BLE\_GAP\_AdvParams



[**ClassList**](annotated.md) **>** [**SYN\_BLE\_GAP\_AdvParams**](structSYN__BLE__GAP__AdvParams.md)



[More...](#detailed-description)

* `#include <syn_ble_gap.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**channel\_map**](#variable-channel_map)  <br> |
|  uint8\_t | [**filter\_policy**](#variable-filter_policy)  <br> |
|  uint16\_t | [**interval\_max**](#variable-interval_max)  <br> |
|  uint16\_t | [**interval\_min**](#variable-interval_min)  <br> |
|  uint8\_t | [**own\_addr\_type**](#variable-own_addr_type)  <br> |
|  uint8\_t | [**peer\_addr**](#variable-peer_addr)  <br> |
|  uint8\_t | [**peer\_addr\_type**](#variable-peer_addr_type)  <br> |
|  uint8\_t | [**type**](#variable-type)  <br> |












































## Detailed Description


Advertising Parameters 


    
## Public Attributes Documentation




### variable channel\_map 

```C++
uint8_t SYN_BLE_GAP_AdvParams::channel_map;
```



Channel map bitmask (default 0x07 for ch 37,38,39) 


        

<hr>



### variable filter\_policy 

```C++
uint8_t SYN_BLE_GAP_AdvParams::filter_policy;
```



Advertising filter policy 


        

<hr>



### variable interval\_max 

```C++
uint16_t SYN_BLE_GAP_AdvParams::interval_max;
```



Advertising interval max (units 0.625ms) 


        

<hr>



### variable interval\_min 

```C++
uint16_t SYN_BLE_GAP_AdvParams::interval_min;
```



Advertising interval min (units 0.625ms) 


        

<hr>



### variable own\_addr\_type 

```C++
uint8_t SYN_BLE_GAP_AdvParams::own_addr_type;
```



Own address type 


        

<hr>



### variable peer\_addr 

```C++
uint8_t SYN_BLE_GAP_AdvParams::peer_addr[6];
```



Peer MAC address 


        

<hr>



### variable peer\_addr\_type 

```C++
uint8_t SYN_BLE_GAP_AdvParams::peer_addr_type;
```



Peer address type 


        

<hr>



### variable type 

```C++
uint8_t SYN_BLE_GAP_AdvParams::type;
```



Advertising type (e.g. SYN\_BLE\_ADV\_IND) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_gap.h`

