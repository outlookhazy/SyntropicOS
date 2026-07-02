

# Struct SYN\_Transport



[**ClassList**](annotated.md) **>** [**SYN\_Transport**](structSYN__Transport.md)



_Abstract transport interface (send/receive function pointers)._ 

* `#include <syn_transport.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool)(\* | [**recv**](#variable-recv)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool)(\* | [**send**](#variable-send)  <br> |












































## Public Attributes Documentation




### variable ctx 

```C++
void* SYN_Transport::ctx;
```



Transport-specific context 


        

<hr>



### variable recv 

```C++
bool(* SYN_Transport::recv) (uint8_t *data, size_t max_len, size_t *out_len, void *ctx);
```



Receive a packet (non-blocking). 

**Parameters:**


* `data` Buffer for received data. 
* `max_len` Buffer capacity. 
* `out_len` [out] Actual received length. 
* `ctx` Transport context. 



**Returns:**

true if a complete packet was received. 





        

<hr>



### variable send 

```C++
bool(* SYN_Transport::send) (const uint8_t *data, size_t len, void *ctx);
```



Send a packet. 

**Parameters:**


* `data` Packet data. 
* `len` Packet length. 
* `ctx` Transport context. 



**Returns:**

true if sent successfully. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_transport.h`

