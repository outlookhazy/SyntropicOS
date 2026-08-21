

# Struct SYN\_OPCUA\_Node



[**ClassList**](annotated.md) **>** [**SYN\_OPCUA\_Node**](structSYN__OPCUA__Node.md)



_In-memory OPC UA Node representation._ 

* `#include <syn_opcua.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**access\_level**](#variable-access_level)  <br> |
|  const char \* | [**browse\_name**](#variable-browse_name)  <br> |
|  [**SYN\_OPCUA\_DataType**](syn__opcua_8h.md#enum-syn_opcua_datatype) | [**data\_type**](#variable-data_type)  <br> |
|  const char \* | [**display\_name**](#variable-display_name)  <br> |
|  [**SYN\_OPCUA\_NodeClass**](syn__opcua_8h.md#enum-syn_opcua_nodeclass) | [**node\_class**](#variable-node_class)  <br> |
|  [**SYN\_OPCUA\_NodeId**](structSYN__OPCUA__NodeId.md) | [**node\_id**](#variable-node_id)  <br> |
|  [**SYN\_OPCUA\_ReadCb**](syn__opcua_8h.md#typedef-syn_opcua_readcb) | [**on\_read**](#variable-on_read)  <br> |
|  [**SYN\_OPCUA\_WriteCb**](syn__opcua_8h.md#typedef-syn_opcua_writecb) | [**on\_write**](#variable-on_write)  <br> |
|  [**SYN\_OPCUA\_NodeId**](structSYN__OPCUA__NodeId.md) | [**parent\_id**](#variable-parent_id)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |
|  [**SYN\_OPCUA\_DataValue**](structSYN__OPCUA__DataValue.md) | [**value**](#variable-value)  <br> |












































## Public Attributes Documentation




### variable access\_level 

```C++
uint8_t SYN_OPCUA_Node::access_level;
```



0x01 = Read, 0x02 = Write, 0x03 = Read/Write 


        

<hr>



### variable browse\_name 

```C++
const char* SYN_OPCUA_Node::browse_name;
```



Qualified browse name 


        

<hr>



### variable data\_type 

```C++
SYN_OPCUA_DataType SYN_OPCUA_Node::data_type;
```



Data type (for Variables) 


        

<hr>



### variable display\_name 

```C++
const char* SYN_OPCUA_Node::display_name;
```



Localized display name 


        

<hr>



### variable node\_class 

```C++
SYN_OPCUA_NodeClass SYN_OPCUA_Node::node_class;
```



NodeClass (Object, Variable, Method) 


        

<hr>



### variable node\_id 

```C++
SYN_OPCUA_NodeId SYN_OPCUA_Node::node_id;
```



Unique NodeId 


        

<hr>



### variable on\_read 

```C++
SYN_OPCUA_ReadCb SYN_OPCUA_Node::on_read;
```



Optional dynamic read hook 


        

<hr>



### variable on\_write 

```C++
SYN_OPCUA_WriteCb SYN_OPCUA_Node::on_write;
```



Optional dynamic write hook 


        

<hr>



### variable parent\_id 

```C++
SYN_OPCUA_NodeId SYN_OPCUA_Node::parent_id;
```



Parent NodeId 


        

<hr>



### variable user\_data 

```C++
void* SYN_OPCUA_Node::user_data;
```



Context passed to callbacks 


        

<hr>



### variable value 

```C++
SYN_OPCUA_DataValue SYN_OPCUA_Node::value;
```



Current cached value 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_opcua.h`

