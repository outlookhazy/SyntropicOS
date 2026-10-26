

# Struct SYN\_SockAddr



[**ClassList**](annotated.md) **>** [**SYN\_SockAddr**](structSYN__SockAddr.md)



[More...](#detailed-description)

* `#include <syn_port_socket.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**ip**](#variable-ip)  <br> |
|  uint16\_t | [**port**](#variable-port)  <br> |












































## Detailed Description


IPv4 address + port. 


    
## Public Attributes Documentation




### variable ip 

```C++
uint8_t SYN_SockAddr::ip[4];
```



IPv4 address bytes (network order) 
 


        

<hr>



### variable port 

```C++
uint16_t SYN_SockAddr::port;
```



Port number (host order) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_socket.h`

