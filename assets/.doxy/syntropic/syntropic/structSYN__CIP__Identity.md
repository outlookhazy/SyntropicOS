

# Struct SYN\_CIP\_Identity



[**ClassList**](annotated.md) **>** [**SYN\_CIP\_Identity**](structSYN__CIP__Identity.md)



_CIP Identity Object (Class 0x01) Attributes._ 

* `#include <syn_devicenet.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**device\_type**](#variable-device_type)  <br> |
|  uint16\_t | [**product\_code**](#variable-product_code)  <br> |
|  char | [**product\_name**](#variable-product_name)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**quick\_connect\_enabled**](#variable-quick_connect_enabled)  <br> |
|  uint8\_t | [**rev\_major**](#variable-rev_major)  <br> |
|  uint8\_t | [**rev\_minor**](#variable-rev_minor)  <br> |
|  uint32\_t | [**serial\_number**](#variable-serial_number)  <br> |
|  uint16\_t | [**status**](#variable-status)  <br> |
|  uint16\_t | [**vendor\_id**](#variable-vendor_id)  <br> |












































## Public Attributes Documentation




### variable device\_type 

```C++
uint16_t SYN_CIP_Identity::device_type;
```



Device type profile 


        

<hr>



### variable product\_code 

```C++
uint16_t SYN_CIP_Identity::product_code;
```



Product code 


        

<hr>



### variable product\_name 

```C++
char SYN_CIP_Identity::product_name[32];
```



ASCII product name string 


        

<hr>



### variable quick\_connect\_enabled 

```C++
bool SYN_CIP_Identity::quick_connect_enabled;
```



QuickConnect feature enabled flag 


        

<hr>



### variable rev\_major 

```C++
uint8_t SYN_CIP_Identity::rev_major;
```



Major revision number 


        

<hr>



### variable rev\_minor 

```C++
uint8_t SYN_CIP_Identity::rev_minor;
```



Minor revision number 


        

<hr>



### variable serial\_number 

```C++
uint32_t SYN_CIP_Identity::serial_number;
```



32-bit device serial number 


        

<hr>



### variable status 

```C++
uint16_t SYN_CIP_Identity::status;
```



Device status word 


        

<hr>



### variable vendor\_id 

```C++
uint16_t SYN_CIP_Identity::vendor_id;
```



Vendor ID code 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_devicenet.h`

