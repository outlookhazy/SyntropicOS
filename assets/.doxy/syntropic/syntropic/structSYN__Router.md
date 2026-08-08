

# Struct SYN\_Router



[**ClassList**](annotated.md) **>** [**SYN\_Router**](structSYN__Router.md)



_Router instance — node ID, transport, handler table, ACK tracking._ 

* `#include <syn_router.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**ack\_timeout\_ms**](#variable-ack_timeout_ms)  <br> |
|  uint32\_t | [**drop\_count**](#variable-drop_count)  <br> |
|  uint8\_t | [**handler\_cap**](#variable-handler_cap)  <br> |
|  uint8\_t | [**handler\_count**](#variable-handler_count)  <br> |
|  [**SYN\_RouterHandler**](structSYN__RouterHandler.md) \* | [**handlers**](#variable-handlers)  <br> |
|  uint8\_t | [**max\_retries**](#variable-max_retries)  <br> |
|  uint8\_t | [**node\_id**](#variable-node_id)  <br> |
|  [**SYN\_PendingAck**](structSYN__PendingAck.md) \* | [**pending**](#variable-pending)  <br> |
|  uint8\_t | [**pending\_cap**](#variable-pending_cap)  <br> |
|  uint32\_t | [**rx\_count**](#variable-rx_count)  <br> |
|  [**SYN\_Transport**](structSYN__Transport.md) \* | [**transport**](#variable-transport)  <br> |
|  uint32\_t | [**tx\_count**](#variable-tx_count)  <br> |
|  uint8\_t | [**tx\_seq**](#variable-tx_seq)  <br> |












































## Public Attributes Documentation




### variable ack\_timeout\_ms 

```C++
uint16_t SYN_Router::ack_timeout_ms;
```



ACK timeout in ms 
 


        

<hr>



### variable drop\_count 

```C++
uint32_t SYN_Router::drop_count;
```



Unhandled / bad packets 
 


        

<hr>



### variable handler\_cap 

```C++
uint8_t SYN_Router::handler_cap;
```



Array capacity 
 


        

<hr>



### variable handler\_count 

```C++
uint8_t SYN_Router::handler_count;
```



Registered handler count 
 


        

<hr>



### variable handlers 

```C++
SYN_RouterHandler* SYN_Router::handlers;
```



Caller-owned handler array 
 


        

<hr>



### variable max\_retries 

```C++
uint8_t SYN_Router::max_retries;
```



Max retransmissions 
 


        

<hr>



### variable node\_id 

```C++
uint8_t SYN_Router::node_id;
```



Our node ID 
 


        

<hr>



### variable pending 

```C++
SYN_PendingAck* SYN_Router::pending;
```



Caller-owned pending ACK array 


        

<hr>



### variable pending\_cap 

```C++
uint8_t SYN_Router::pending_cap;
```



Array capacity 
 


        

<hr>



### variable rx\_count 

```C++
uint32_t SYN_Router::rx_count;
```



Total packets received 
 


        

<hr>



### variable transport 

```C++
SYN_Transport* SYN_Router::transport;
```



Underlying transport 
 


        

<hr>



### variable tx\_count 

```C++
uint32_t SYN_Router::tx_count;
```



Total packets sent 
 


        

<hr>



### variable tx\_seq 

```C++
uint8_t SYN_Router::tx_seq;
```



Auto-incrementing TX seq 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_router.h`

