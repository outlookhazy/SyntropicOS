

# Struct SYN\_BLE\_GAP\_s



[**ClassList**](annotated.md) **>** [**SYN\_BLE\_GAP\_s**](structSYN__BLE__GAP__s.md)



[More...](#detailed-description)

* `#include <syn_ble_gap.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**advertising**](#variable-advertising)  <br> |
|  [**SYN\_BLE\_GAP\_EventCb**](syn__ble__gap_8h.md#typedef-syn_ble_gap_eventcb) | [**evt\_cb**](#variable-evt_cb)  <br> |
|  SYN\_BLE\_HCI \* | [**hci**](#variable-hci)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**scanning**](#variable-scanning)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Detailed Description


GAP Engine Context 


    
## Public Attributes Documentation




### variable advertising 

```C++
bool SYN_BLE_GAP::advertising;
```



True if currently advertising 


        

<hr>



### variable evt\_cb 

```C++
SYN_BLE_GAP_EventCb SYN_BLE_GAP::evt_cb;
```



GAP event callback 


        

<hr>



### variable hci 

```C++
SYN_BLE_HCI* SYN_BLE_GAP::hci;
```



Associated HCI driver handle 


        

<hr>



### variable scanning 

```C++
bool SYN_BLE_GAP::scanning;
```



True if currently scanning 


        

<hr>



### variable user\_data 

```C++
void* SYN_BLE_GAP::user_data;
```



User context pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_gap.h`

