

# Struct SYN\_HttpdResponse



[**ClassList**](annotated.md) **>** [**SYN\_HttpdResponse**](structSYN__HttpdResponse.md)



_HTTP response formatting state._ 

* `#include <syn_httpd.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**buf**](#variable-buf)  <br> |
|  size\_t | [**buf\_size**](#variable-buf_size)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**headers\_sent**](#variable-headers_sent)  <br> |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**sock**](#variable-sock)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**upgraded**](#variable-upgraded)  <br> |












































## Public Attributes Documentation




### variable buf 

```C++
uint8_t* SYN_HttpdResponse::buf;
```



Shared work buffer 
 


        

<hr>



### variable buf\_size 

```C++
size_t SYN_HttpdResponse::buf_size;
```



Buffer capacity 
 


        

<hr>



### variable headers\_sent 

```C++
bool SYN_HttpdResponse::headers_sent;
```



Have headers been finalized? 
 


        

<hr>



### variable sock 

```C++
SYN_Socket SYN_HttpdResponse::sock;
```



Client socket 
 


        

<hr>



### variable upgraded 

```C++
bool SYN_HttpdResponse::upgraded;
```



Has connection been upgraded? 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_httpd.h`

