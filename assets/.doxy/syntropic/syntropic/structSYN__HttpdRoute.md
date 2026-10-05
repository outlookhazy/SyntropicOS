

# Struct SYN\_HttpdRoute



[**ClassList**](annotated.md) **>** [**SYN\_HttpdRoute**](structSYN__HttpdRoute.md)



_An HTTP routing entry configuration._ 

* `#include <syn_httpd.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  [**SYN\_HttpdHandler**](syn__httpd_8h.md#typedef-syn_httpdhandler) | [**handler**](#variable-handler)  <br> |
|  [**SYN\_HttpMethod**](syn__httpd_8h.md#enum-syn_httpmethod) | [**method**](#variable-method)  <br> |
|  const char \* | [**path**](#variable-path)  <br> |












































## Public Attributes Documentation




### variable ctx 

```C++
void* SYN_HttpdRoute::ctx;
```



User context for handler 
 


        

<hr>



### variable handler 

```C++
SYN_HttpdHandler SYN_HttpdRoute::handler;
```



Handler function 
 


        

<hr>



### variable method 

```C++
SYN_HttpMethod SYN_HttpdRoute::method;
```



HTTP method to match 
 


        

<hr>



### variable path 

```C++
const char* SYN_HttpdRoute::path;
```



Path to match (or prefix + '\*') 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_httpd.h`

