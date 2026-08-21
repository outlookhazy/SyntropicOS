

# File syn\_transport\_udp.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_transport\_udp.h**](syn__transport__udp_8h.md)

[Go to the source code of this file](syn__transport__udp_8h_source.md)

_Bridge binding syn\_port\_udp\_\* API to native syn\_udp stack._ 

* `#include "syntropic/net/syn_udp.h"`
* `#include "syntropic/port/syn_port_socket.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_UDP**](structSYN__UDP.md) \* | [**syn\_transport\_udp\_get\_instance**](#function-syn_transport_udp_get_instance) (void) <br>_Get the active native_ [_**SYN\_UDP**_](structSYN__UDP.md) _stack instance._ |
|  void | [**syn\_transport\_udp\_set\_instance**](#function-syn_transport_udp_set_instance) ([**SYN\_UDP**](structSYN__UDP.md) \* udp) <br>_Bind native syn\_udp instance to the syn\_port\_udp\_\* socket interface._  |




























## Public Functions Documentation




### function syn\_transport\_udp\_get\_instance 

_Get the active native_ [_**SYN\_UDP**_](structSYN__UDP.md) _stack instance._
```C++
SYN_UDP * syn_transport_udp_get_instance (
    void
) 
```





**Returns:**

Pointer to [**SYN\_UDP**](structSYN__UDP.md) instance, or NULL.


Get the active native [**SYN\_UDP**](structSYN__UDP.md) stack instance.




**Returns:**

Pointer to global [**SYN\_UDP**](structSYN__UDP.md) instance, or NULL if unavailable. 





        

<hr>



### function syn\_transport\_udp\_set\_instance 

_Bind native syn\_udp instance to the syn\_port\_udp\_\* socket interface._ 
```C++
void syn_transport_udp_set_instance (
    SYN_UDP * udp
) 
```





**Parameters:**


* `udp` Pointer to native [**SYN\_UDP**](structSYN__UDP.md) stack instance. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_transport_udp.h`

