

# Struct SYN\_OPCUA\_NodeId



[**ClassList**](annotated.md) **>** [**SYN\_OPCUA\_NodeId**](structSYN__OPCUA__NodeId.md)



_OPC UA NodeId._ 

* `#include <syn_opcua.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  union [**SYN\_OPCUA\_NodeId**](structSYN__OPCUA__NodeId.md) | [**id**](#variable-id)  <br> |
|  [**SYN\_OPCUA\_NodeIdType**](syn__opcua_8h.md#enum-syn_opcua_nodeidtype) | [**id\_type**](#variable-id_type)  <br> |
|  uint16\_t | [**ns\_index**](#variable-ns_index)  <br> |
|  uint32\_t | [**num**](#variable-num)  <br> |
|  const char \* | [**str**](#variable-str)  <br> |












































## Public Attributes Documentation




### variable id 

```C++
union SYN_OPCUA_NodeId SYN_OPCUA_NodeId::id;
```



Identifier payload union 


        

<hr>



### variable id\_type 

```C++
SYN_OPCUA_NodeIdType SYN_OPCUA_NodeId::id_type;
```



Numeric or String ID type 


        

<hr>



### variable ns\_index 

```C++
uint16_t SYN_OPCUA_NodeId::ns_index;
```



Namespace index 


        

<hr>



### variable num 

```C++
uint32_t SYN_OPCUA_NodeId::num;
```



32-bit numeric identifier 


        

<hr>



### variable str 

```C++
const char* SYN_OPCUA_NodeId::str;
```



String identifier 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_opcua.h`

