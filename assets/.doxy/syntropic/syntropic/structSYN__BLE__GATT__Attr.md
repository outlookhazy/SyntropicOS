

# Struct SYN\_BLE\_GATT\_Attr



[**ClassList**](annotated.md) **>** [**SYN\_BLE\_GATT\_Attr**](structSYN__BLE__GATT__Attr.md)



[More...](#detailed-description)

* `#include <syn_ble_gatt.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**handle**](#variable-handle)  <br> |
|  uint8\_t | [**permissions**](#variable-permissions)  <br> |
|  [**SYN\_BLE\_GATT\_ReadCb**](syn__ble__gatt_8h.md#typedef-syn_ble_gatt_readcb) | [**read\_cb**](#variable-read_cb)  <br> |
|  const uint8\_t \* | [**static\_val**](#variable-static_val)  <br> |
|  uint16\_t | [**static\_val\_len**](#variable-static_val_len)  <br> |
|  uint16\_t | [**uuid16**](#variable-uuid16)  <br> |
|  [**SYN\_BLE\_GATT\_WriteCb**](syn__ble__gatt_8h.md#typedef-syn_ble_gatt_writecb) | [**write\_cb**](#variable-write_cb)  <br> |












































## Detailed Description


Attribute Table Entry 


    
## Public Attributes Documentation




### variable handle 

```C++
uint16_t SYN_BLE_GATT_Attr::handle;
```



Attribute 16-bit handle 


        

<hr>



### variable permissions 

```C++
uint8_t SYN_BLE_GATT_Attr::permissions;
```



Property/permission bitmask 


        

<hr>



### variable read\_cb 

```C++
SYN_BLE_GATT_ReadCb SYN_BLE_GATT_Attr::read_cb;
```



Dynamic read callback 


        

<hr>



### variable static\_val 

```C++
const uint8_t* SYN_BLE_GATT_Attr::static_val;
```



Static value buffer pointer 


        

<hr>



### variable static\_val\_len 

```C++
uint16_t SYN_BLE_GATT_Attr::static_val_len;
```



Static value length 


        

<hr>



### variable uuid16 

```C++
uint16_t SYN_BLE_GATT_Attr::uuid16;
```



Attribute 16-bit UUID 


        

<hr>



### variable write\_cb 

```C++
SYN_BLE_GATT_WriteCb SYN_BLE_GATT_Attr::write_cb;
```



Dynamic write callback 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_gatt.h`

