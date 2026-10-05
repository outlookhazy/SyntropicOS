

# Struct SYN\_LwM2M\_ResourceDesc



[**ClassList**](annotated.md) **>** [**SYN\_LwM2M\_ResourceDesc**](structSYN__LwM2M__ResourceDesc.md)



_Resource descriptor defining ID, permissions, and type._ 

* `#include <syn_lwm2m.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**id**](#variable-id)  <br> |
|  uint8\_t | [**operations**](#variable-operations)  <br> |
|  [**SYN\_LwM2M\_ValType**](syn__lwm2m_8h.md#enum-syn_lwm2m_valtype) | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable id 

```C++
uint16_t SYN_LwM2M_ResourceDesc::id;
```



Resource ID 


        

<hr>



### variable operations 

```C++
uint8_t SYN_LwM2M_ResourceDesc::operations;
```



Bitmask of allowed operations (SYN\_LWM2M\_OP\_\*) 


        

<hr>



### variable type 

```C++
SYN_LwM2M_ValType SYN_LwM2M_ResourceDesc::type;
```



Data type 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m.h`

