

# Struct SYN\_CoapsClient



[**ClassList**](annotated.md) **>** [**SYN\_CoapsClient**](structSYN__CoapsClient.md)



_CoAP over DTLS 1.3 Client Session (RFC 7252 Section 12.1.2 & RFC 9147)._ 

* `#include <syn_coap.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_DTLS\_Context**](structSYN__DTLS__Context.md) | [**dtls**](#variable-dtls)  <br> |
|  [**SYN\_Transport**](structSYN__Transport.md) | [**dtls\_transport**](#variable-dtls_transport)  <br> |












































## Public Attributes Documentation




### variable dtls 

```C++
SYN_DTLS_Context SYN_CoapsClient::dtls;
```



Underlying DTLS 1.3 protocol engine 


        

<hr>



### variable dtls\_transport 

```C++
SYN_Transport SYN_CoapsClient::dtls_transport;
```



DTLS transport adapter 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_coap.h`

