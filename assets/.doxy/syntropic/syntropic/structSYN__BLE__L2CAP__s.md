

# Struct SYN\_BLE\_L2CAP\_s



[**ClassList**](annotated.md) **>** [**SYN\_BLE\_L2CAP\_s**](structSYN__BLE__L2CAP__s.md)



[More...](#detailed-description)

* `#include <syn_ble_l2cap.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_BLE\_L2CAP\_Connection**](structSYN__BLE__L2CAP__Connection.md) | [**conns**](#variable-conns)  <br> |
|  [**SYN\_BLE\_L2CAP\_RxCb**](syn__ble__l2cap_8h.md#typedef-syn_ble_l2cap_rxcb) | [**rx\_cb**](#variable-rx_cb)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Detailed Description


L2CAP Engine Context 


    
## Public Attributes Documentation




### variable conns 

```C++
SYN_BLE_L2CAP_Connection SYN_BLE_L2CAP::conns[SYN_BLE_MAX_CONNECTIONS];
```



Connection table 


        

<hr>



### variable rx\_cb 

```C++
SYN_BLE_L2CAP_RxCb SYN_BLE_L2CAP::rx_cb;
```



PDU RX callback 


        

<hr>



### variable user\_data 

```C++
void* SYN_BLE_L2CAP::user_data;
```



User context pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_l2cap.h`

