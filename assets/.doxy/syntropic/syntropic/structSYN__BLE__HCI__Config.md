

# Struct SYN\_BLE\_HCI\_Config



[**ClassList**](annotated.md) **>** [**SYN\_BLE\_HCI\_Config**](structSYN__BLE__HCI__Config.md)



[More...](#detailed-description)

* `#include <syn_ble_hci.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_BLE\_HCI\_AclCb**](syn__ble__hci_8h.md#typedef-syn_ble_hci_aclcb) | [**acl\_cb**](#variable-acl_cb)  <br> |
|  [**SYN\_BLE\_HCI\_EventCb**](syn__ble__hci_8h.md#typedef-syn_ble_hci_eventcb) | [**evt\_cb**](#variable-evt_cb)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Detailed Description


HCI Driver Configuration 


    
## Public Attributes Documentation




### variable acl\_cb 

```C++
SYN_BLE_HCI_AclCb SYN_BLE_HCI_Config::acl_cb;
```



ACL Data callback 


        

<hr>



### variable evt\_cb 

```C++
SYN_BLE_HCI_EventCb SYN_BLE_HCI_Config::evt_cb;
```



Event callback 


        

<hr>



### variable user\_data 

```C++
void* SYN_BLE_HCI_Config::user_data;
```



User context pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_hci.h`

