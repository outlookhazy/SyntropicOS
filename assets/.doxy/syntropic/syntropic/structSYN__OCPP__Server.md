

# Struct SYN\_OCPP\_Server



[**ClassList**](annotated.md) **>** [**SYN\_OCPP\_Server**](structSYN__OCPP__Server.md)



_OCPP Server instance state (CSMS Central System Role)._ 

* `#include <syn_ocpp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_OCPP\_ServerAuthorizeHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_serverauthorizehandler) | [**auth\_cb**](#variable-auth_cb)  <br> |
|  [**SYN\_OCPP\_ServerBootHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_serverboothandler) | [**boot\_cb**](#variable-boot_cb)  <br> |
|  uint32\_t | [**message\_counter**](#variable-message_counter)  <br> |
|  int32\_t | [**next\_transaction\_id**](#variable-next_transaction_id)  <br> |
|  [**SYN\_OCPP\_ServerStartTxHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_serverstarttxhandler) | [**start\_tx\_cb**](#variable-start_tx_cb)  <br> |
|  void \* | [**user\_ctx**](#variable-user_ctx)  <br> |












































## Public Attributes Documentation




### variable auth\_cb 

```C++
SYN_OCPP_ServerAuthorizeHandler SYN_OCPP_Server::auth_cb;
```



Station RFID authorization callback 


        

<hr>



### variable boot\_cb 

```C++
SYN_OCPP_ServerBootHandler SYN_OCPP_Server::boot_cb;
```



Station boot registration callback 


        

<hr>



### variable message\_counter 

```C++
uint32_t SYN_OCPP_Server::message_counter;
```



Monotonic server message counter 


        

<hr>



### variable next\_transaction\_id 

```C++
int32_t SYN_OCPP_Server::next_transaction_id;
```



Auto-incrementing transaction ID 


        

<hr>



### variable start\_tx\_cb 

```C++
SYN_OCPP_ServerStartTxHandler SYN_OCPP_Server::start_tx_cb;
```



Station start transaction callback 


        

<hr>



### variable user\_ctx 

```C++
void* SYN_OCPP_Server::user_ctx;
```



User context pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ocpp.h`

