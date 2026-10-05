

# Struct SYN\_Httpd



[**ClassList**](annotated.md) **>** [**SYN\_Httpd**](structSYN__Httpd.md)



_HTTP server context structure._ 

* `#include <syn_httpd.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**client**](#variable-client)  <br> |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**listener**](#variable-listener)  <br> |
|  uint16\_t | [**port**](#variable-port)  <br> |
|  uint32\_t | [**recv\_deadline**](#variable-recv_deadline)  <br> |
|  size\_t | [**route\_count**](#variable-route_count)  <br> |
|  const [**SYN\_HttpdRoute**](structSYN__HttpdRoute.md) \* | [**routes**](#variable-routes)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**running**](#variable-running)  <br> |
|  size\_t | [**rx\_total**](#variable-rx_total)  <br> |
|  [**SYN\_HttpdState**](syn__httpd_8h.md#enum-syn_httpdstate) | [**state**](#variable-state)  <br> |
|  uint8\_t \* | [**work\_buf**](#variable-work_buf)  <br> |
|  size\_t | [**work\_buf\_size**](#variable-work_buf_size)  <br> |












































## Public Attributes Documentation




### variable client 

```C++
SYN_Socket SYN_Httpd::client;
```



Active client socket 
 


        

<hr>



### variable listener 

```C++
SYN_Socket SYN_Httpd::listener;
```



Bound listener socket handle 


        

<hr>



### variable port 

```C++
uint16_t SYN_Httpd::port;
```



Listening TCP port number 


        

<hr>



### variable recv\_deadline 

```C++
uint32_t SYN_Httpd::recv_deadline;
```



Tick deadline for header recv 


        

<hr>



### variable route\_count 

```C++
size_t SYN_Httpd::route_count;
```



Number of routes in array 


        

<hr>



### variable routes 

```C++
const SYN_HttpdRoute* SYN_Httpd::routes;
```



Array of registered route entries 


        

<hr>



### variable running 

```C++
bool SYN_Httpd::running;
```



Server state active flag 


        

<hr>



### variable rx\_total 

```C++
size_t SYN_Httpd::rx_total;
```



Bytes accumulated in work\_buf 


        

<hr>



### variable state 

```C++
SYN_HttpdState SYN_Httpd::state;
```



Current processing phase 
 


        

<hr>



### variable work\_buf 

```C++
uint8_t* SYN_Httpd::work_buf;
```



Buffer for request processing 


        

<hr>



### variable work\_buf\_size 

```C++
size_t SYN_Httpd::work_buf_size;
```



Size of work buffer in bytes 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_httpd.h`

