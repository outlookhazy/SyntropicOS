

# Struct SYN\_RouterHandler



[**ClassList**](annotated.md) **>** [**SYN\_RouterHandler**](structSYN__RouterHandler.md)



_Handler registration entry — message type + callback._ 

* `#include <syn_router.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  [**SYN\_RouterHandlerFn**](syn__router_8h.md#typedef-syn_routerhandlerfn) | [**handler**](#variable-handler)  <br> |
|  uint8\_t | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable ctx 

```C++
void* SYN_RouterHandler::ctx;
```



Handler context 
 


        

<hr>



### variable handler 

```C++
SYN_RouterHandlerFn SYN_RouterHandler::handler;
```



Handler function 
 


        

<hr>



### variable type 

```C++
uint8_t SYN_RouterHandler::type;
```



Message type to match 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_router.h`

