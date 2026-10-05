

# Struct SYN\_BLE\_L2CAP\_Connection



[**ClassList**](annotated.md) **>** [**SYN\_BLE\_L2CAP\_Connection**](structSYN__BLE__L2CAP__Connection.md)



[More...](#detailed-description)

* `#include <syn_ble_l2cap.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  uint16\_t | [**cid**](#variable-cid)  <br> |
|  uint16\_t | [**conn\_handle**](#variable-conn_handle)  <br> |
|  uint16\_t | [**expected\_len**](#variable-expected_len)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**reassembling**](#variable-reassembling)  <br> |
|  uint8\_t | [**rx\_buf**](#variable-rx_buf)  <br> |
|  uint16\_t | [**rx\_len**](#variable-rx_len)  <br> |












































## Detailed Description


Connection State Entry 


    
## Public Attributes Documentation




### variable active 

```C++
bool SYN_BLE_L2CAP_Connection::active;
```



True if slot is active 


        

<hr>



### variable cid 

```C++
uint16_t SYN_BLE_L2CAP_Connection::cid;
```



Target channel ID 


        

<hr>



### variable conn\_handle 

```C++
uint16_t SYN_BLE_L2CAP_Connection::conn_handle;
```



Connection handle 


        

<hr>



### variable expected\_len 

```C++
uint16_t SYN_BLE_L2CAP_Connection::expected_len;
```



Total expected PDU size 


        

<hr>



### variable reassembling 

```C++
bool SYN_BLE_L2CAP_Connection::reassembling;
```



True if currently reassembling 


        

<hr>



### variable rx\_buf 

```C++
uint8_t SYN_BLE_L2CAP_Connection::rx_buf[SYN_BLE_L2CAP_BUF_SIZE];
```



Reassembly buffer 


        

<hr>



### variable rx\_len 

```C++
uint16_t SYN_BLE_L2CAP_Connection::rx_len;
```



Current accumulated bytes 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_l2cap.h`

