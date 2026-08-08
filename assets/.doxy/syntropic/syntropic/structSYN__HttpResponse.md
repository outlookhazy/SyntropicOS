

# Struct SYN\_HttpResponse



[**ClassList**](annotated.md) **>** [**SYN\_HttpResponse**](structSYN__HttpResponse.md)



_HTTP response metadata._ 

* `#include <syn_http.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**chunked**](#variable-chunked)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**connection\_close**](#variable-connection_close)  <br> |
|  uint32\_t | [**content\_length**](#variable-content_length)  <br> |
|  char | [**location**](#variable-location)  <br> |
|  int | [**status\_code**](#variable-status_code)  <br> |












































## Public Attributes Documentation




### variable chunked 

```C++
bool SYN_HttpResponse::chunked;
```



Transfer-Encoding: chunked? 
 


        

<hr>



### variable connection\_close 

```C++
bool SYN_HttpResponse::connection_close;
```



Connection: close? 
 


        

<hr>



### variable content\_length 

```C++
uint32_t SYN_HttpResponse::content_length;
```



Content-Length value, 0 if absent 
 


        

<hr>



### variable location 

```C++
char SYN_HttpResponse::location[128];
```



Location header for redirects 
 


        

<hr>



### variable status\_code 

```C++
int SYN_HttpResponse::status_code;
```



HTTP status (200, 404, etc.) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_http.h`

