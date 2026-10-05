

# Struct SYN\_BACnet\_Object



[**ClassList**](annotated.md) **>** [**SYN\_BACnet\_Object**](structSYN__BACnet__Object.md)



_BACnet Object Instance Definition._ 

* `#include <syn_bacnet.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**instance\_id**](#variable-instance_id)  <br> |
|  const char \* | [**name**](#variable-name)  <br> |
|  uint8\_t | [**object\_type**](#variable-object_type)  <br> |
|  float | [**present\_value**](#variable-present_value)  <br> |












































## Public Attributes Documentation




### variable instance\_id 

```C++
uint32_t SYN_BACnet_Object::instance_id;
```



Object Instance Number (0..4194303) 


        

<hr>



### variable name 

```C++
const char* SYN_BACnet_Object::name;
```



Object Name string 


        

<hr>



### variable object\_type 

```C++
uint8_t SYN_BACnet_Object::object_type;
```



SYN\_BACnet\_ObjectType (AI, AO, BI, BO, Device) 


        

<hr>



### variable present\_value 

```C++
float SYN_BACnet_Object::present_value;
```



Current Numeric/Boolean Present Value 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_bacnet.h`

