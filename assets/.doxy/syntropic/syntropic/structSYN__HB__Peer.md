

# Struct SYN\_HB\_Peer



[**ClassList**](annotated.md) **>** [**SYN\_HB\_Peer**](structSYN__HB__Peer.md)



_Tracked peer entry._ 

* `#include <syn_heartbeat.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**alive**](#variable-alive)  <br> |
|  uint32\_t | [**last\_seen**](#variable-last_seen)  <br> |
|  uint8\_t | [**node\_id**](#variable-node_id)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**used**](#variable-used)  <br> |












































## Public Attributes Documentation




### variable alive 

```C++
bool SYN_HB_Peer::alive;
```



Is peer currently alive? 
 


        

<hr>



### variable last\_seen 

```C++
uint32_t SYN_HB_Peer::last_seen;
```



Tick of last heartbeat received 
 


        

<hr>



### variable node\_id 

```C++
uint8_t SYN_HB_Peer::node_id;
```



Peer's node ID 
 


        

<hr>



### variable used 

```C++
bool SYN_HB_Peer::used;
```



Is this slot in use? 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_heartbeat.h`

