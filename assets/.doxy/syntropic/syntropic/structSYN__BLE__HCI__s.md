

# Struct SYN\_BLE\_HCI\_s



[**ClassList**](annotated.md) **>** [**SYN\_BLE\_HCI\_s**](structSYN__BLE__HCI__s.md)



[More...](#detailed-description)

* `#include <syn_ble_hci.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_BLE\_HCI\_Config**](structSYN__BLE__HCI__Config.md) | [**cfg**](#variable-cfg)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**cmd\_pending**](#variable-cmd_pending)  <br> |
|  uint16\_t | [**last\_cmd\_opcode**](#variable-last_cmd_opcode)  <br> |
|  uint8\_t | [**last\_cmd\_status**](#variable-last_cmd_status)  <br> |
|  uint8\_t | [**rx\_buf**](#variable-rx_buf)  <br> |
|  uint16\_t | [**rx\_idx**](#variable-rx_idx)  <br> |
|  uint8\_t | [**rx\_pkt\_type**](#variable-rx_pkt_type)  <br> |
|  uint8\_t | [**rx\_state**](#variable-rx_state)  <br> |
|  uint16\_t | [**rx\_target\_len**](#variable-rx_target_len)  <br> |












































## Detailed Description


HCI Driver Context State 


    
## Public Attributes Documentation




### variable cfg 

```C++
SYN_BLE_HCI_Config SYN_BLE_HCI::cfg;
```



Driver configuration 


        

<hr>



### variable cmd\_pending 

```C++
bool SYN_BLE_HCI::cmd_pending;
```



True if awaiting command response 


        

<hr>



### variable last\_cmd\_opcode 

```C++
uint16_t SYN_BLE_HCI::last_cmd_opcode;
```



Last completed command opcode 


        

<hr>



### variable last\_cmd\_status 

```C++
uint8_t SYN_BLE_HCI::last_cmd_status;
```



Last completed command status 


        

<hr>



### variable rx\_buf 

```C++
uint8_t SYN_BLE_HCI::rx_buf[SYN_BLE_HCI_RX_BUF_SIZE];
```



RX framing buffer 


        

<hr>



### variable rx\_idx 

```C++
uint16_t SYN_BLE_HCI::rx_idx;
```



Accumulated byte count 


        

<hr>



### variable rx\_pkt\_type 

```C++
uint8_t SYN_BLE_HCI::rx_pkt_type;
```



Current packet type (EVT/ACL) 


        

<hr>



### variable rx\_state 

```C++
uint8_t SYN_BLE_HCI::rx_state;
```



Current state machine phase 


        

<hr>



### variable rx\_target\_len 

```C++
uint16_t SYN_BLE_HCI::rx_target_len;
```



Expected frame target size 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_hci.h`

