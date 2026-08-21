

# Struct SYN\_HttpdRequest



[**ClassList**](annotated.md) **>** [**SYN\_HttpdRequest**](structSYN__HttpdRequest.md)



_Parsed HTTP request container presented to route handlers._ 

* `#include <syn_httpd.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**body\_buffered\_len**](#variable-body_buffered_len)  <br> |
|  size\_t | [**body\_buffered\_offset**](#variable-body_buffered_offset)  <br> |
|  size\_t | [**body\_consumed**](#variable-body_consumed)  <br> |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**client\_sock**](#variable-client_sock)  <br> |
|  uint32\_t | [**content\_length**](#variable-content_length)  <br> |
|  const char \* | [**content\_type**](#variable-content_type)  <br> |
|  const char \* | [**headers**](#variable-headers)  <br> |
|  [**SYN\_HttpMethod**](syn__httpd_8h.md#enum-syn_httpmethod) | [**method**](#variable-method)  <br> |
|  const char \* | [**path**](#variable-path)  <br> |
|  const char \* | [**query**](#variable-query)  <br> |












































## Public Attributes Documentation




### variable body\_buffered\_len 

```C++
size_t SYN_HttpdRequest::body_buffered_len;
```



Length of buffered body bytes 
 


        

<hr>



### variable body\_buffered\_offset 

```C++
size_t SYN_HttpdRequest::body_buffered_offset;
```



Offset in work\_buf to buffered body 


        

<hr>



### variable body\_consumed 

```C++
size_t SYN_HttpdRequest::body_consumed;
```



Bytes of body already consumed 
 


        

<hr>



### variable client\_sock 

```C++
SYN_Socket SYN_HttpdRequest::client_sock;
```



Client socket (for body reads) 
 


        

<hr>



### variable content\_length 

```C++
uint32_t SYN_HttpdRequest::content_length;
```



Content-Length, 0 if absent 
 


        

<hr>



### variable content\_type 

```C++
const char* SYN_HttpdRequest::content_type;
```



Content-Type, or NULL 
 


        

<hr>



### variable headers 

```C++
const char* SYN_HttpdRequest::headers;
```



Pointer to start of headers 
 


        

<hr>



### variable method 

```C++
SYN_HttpMethod SYN_HttpdRequest::method;
```



GET, POST, etc. 
 


        

<hr>



### variable path 

```C++
const char* SYN_HttpdRequest::path;
```



Request path (in work\_buf) 
 


        

<hr>



### variable query 

```C++
const char* SYN_HttpdRequest::query;
```



Query string after '?', or NULL 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_httpd.h`

