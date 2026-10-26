

# Struct SYN\_LwM2M\_TLV



[**ClassList**](annotated.md) **>** [**SYN\_LwM2M\_TLV**](structSYN__LwM2M__TLV.md)



_Decoded / to-be-encoded LwM2M TLV item._ 

* `#include <syn_lwm2m.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**id**](#variable-id)  <br> |
|  size\_t | [**len**](#variable-len)  <br> |
|  [**SYN\_LwM2M\_TLV\_Type**](syn__lwm2m_8h.md#enum-syn_lwm2m_tlv_type) | [**type**](#variable-type)  <br> |
|  const uint8\_t \* | [**val**](#variable-val)  <br> |












































## Public Attributes Documentation




### variable id 

```C++
uint16_t SYN_LwM2M_TLV::id;
```



Identifier (Object/Instance/Resource ID) 


        

<hr>



### variable len 

```C++
size_t SYN_LwM2M_TLV::len;
```



Length of value in bytes 


        

<hr>



### variable type 

```C++
SYN_LwM2M_TLV_Type SYN_LwM2M_TLV::type;
```



Item type 


        

<hr>



### variable val 

```C++
const uint8_t* SYN_LwM2M_TLV::val;
```



Pointer to value bytes 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m.h`

