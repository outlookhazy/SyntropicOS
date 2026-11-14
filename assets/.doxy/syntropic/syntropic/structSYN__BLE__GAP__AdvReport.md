

# Struct SYN\_BLE\_GAP\_AdvReport



[**ClassList**](annotated.md) **>** [**SYN\_BLE\_GAP\_AdvReport**](structSYN__BLE__GAP__AdvReport.md)



[More...](#detailed-description)

* `#include <syn_ble_gap.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**addr**](#variable-addr)  <br> |
|  uint8\_t | [**addr\_type**](#variable-addr_type)  <br> |
|  const uint8\_t \* | [**data**](#variable-data)  <br> |
|  uint8\_t | [**data\_len**](#variable-data_len)  <br> |
|  uint8\_t | [**evt\_type**](#variable-evt_type)  <br> |
|  int8\_t | [**rssi**](#variable-rssi)  <br> |












































## Detailed Description


Advertising Report Info 


    
## Public Attributes Documentation




### variable addr 

```C++
uint8_t SYN_BLE_GAP_AdvReport::addr[6];
```



6-byte advertiser MAC address 


        

<hr>



### variable addr\_type 

```C++
uint8_t SYN_BLE_GAP_AdvReport::addr_type;
```



Address type (public/random) 


        

<hr>



### variable data 

```C++
const uint8_t* SYN_BLE_GAP_AdvReport::data;
```



Pointer to advertising payload bytes 


        

<hr>



### variable data\_len 

```C++
uint8_t SYN_BLE_GAP_AdvReport::data_len;
```



Advertising payload length 


        

<hr>



### variable evt\_type 

```C++
uint8_t SYN_BLE_GAP_AdvReport::evt_type;
```



Event type (e.g. SYN\_BLE\_ADV\_IND) 


        

<hr>



### variable rssi 

```C++
int8_t SYN_BLE_GAP_AdvReport::rssi;
```



RSSI signal strength (dBm) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_gap.h`

