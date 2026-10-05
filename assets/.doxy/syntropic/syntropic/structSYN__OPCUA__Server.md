

# Struct SYN\_OPCUA\_Server



[**ClassList**](annotated.md) **>** [**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md)



_Micro OPC UA Server Instance Context._ 

* `#include <syn_opcua.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_OPCUA\_Config**](structSYN__OPCUA__Config.md) | [**cfg**](#variable-cfg)  <br> |
|  size\_t | [**node\_count**](#variable-node_count)  <br> |
|  [**SYN\_OPCUA\_Node**](structSYN__OPCUA__Node.md) | [**nodes**](#variable-nodes)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**pt**](#variable-pt)  <br> |
|  uint32\_t | [**request\_id**](#variable-request_id)  <br> |
|  uint32\_t | [**secure\_channel\_id**](#variable-secure_channel_id)  <br> |
|  uint32\_t | [**security\_token\_id**](#variable-security_token_id)  <br> |
|  uint32\_t | [**sequence\_number**](#variable-sequence_number)  <br> |
|  [**SYN\_OPCUA\_ServerState**](syn__opcua_8h.md#enum-syn_opcua_serverstate) | [**state**](#variable-state)  <br> |












































## Public Attributes Documentation




### variable cfg 

```C++
SYN_OPCUA_Config SYN_OPCUA_Server::cfg;
```



Server configuration 


        

<hr>



### variable node\_count 

```C++
size_t SYN_OPCUA_Server::node_count;
```



Number of registered nodes 


        

<hr>



### variable nodes 

```C++
SYN_OPCUA_Node SYN_OPCUA_Server::nodes[SYN_OPCUA_MAX_NODES];
```



Static address space table 


        

<hr>



### variable pt 

```C++
SYN_PT SYN_OPCUA_Server::pt;
```



Protothread state 


        

<hr>



### variable request\_id 

```C++
uint32_t SYN_OPCUA_Server::request_id;
```



Last processed request ID 


        

<hr>



### variable secure\_channel\_id 

```C++
uint32_t SYN_OPCUA_Server::secure_channel_id;
```



Assigned secure channel ID 


        

<hr>



### variable security\_token\_id 

```C++
uint32_t SYN_OPCUA_Server::security_token_id;
```



Assigned security token ID 


        

<hr>



### variable sequence\_number 

```C++
uint32_t SYN_OPCUA_Server::sequence_number;
```



Outgoing sequence number 


        

<hr>



### variable state 

```C++
SYN_OPCUA_ServerState SYN_OPCUA_Server::state;
```



Active session state 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_opcua.h`

