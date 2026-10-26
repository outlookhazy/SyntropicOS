

# Struct SYN\_BLE\_GATT\_s



[**ClassList**](annotated.md) **>** [**SYN\_BLE\_GATT\_s**](structSYN__BLE__GATT__s.md)



[More...](#detailed-description)

* `#include <syn_ble_gatt.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**cccd\_vals**](#variable-cccd_vals)  <br> |
|  const [**SYN\_BLE\_GATT\_Attr**](structSYN__BLE__GATT__Attr.md) \* | [**table**](#variable-table)  <br> |
|  uint16\_t | [**table\_len**](#variable-table_len)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Detailed Description


GATT Server Instance Context 


    
## Public Attributes Documentation




### variable cccd\_vals 

```C++
uint16_t SYN_BLE_GATT::cccd_vals[SYN_BLE_MAX_CONNECTIONS];
```



CCCD state per connection 


        

<hr>



### variable table 

```C++
const SYN_BLE_GATT_Attr* SYN_BLE_GATT::table;
```



Attribute table array pointer 


        

<hr>



### variable table\_len 

```C++
uint16_t SYN_BLE_GATT::table_len;
```



Attribute table entry count 


        

<hr>



### variable user\_data 

```C++
void* SYN_BLE_GATT::user_data;
```



User context pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_gatt.h`

