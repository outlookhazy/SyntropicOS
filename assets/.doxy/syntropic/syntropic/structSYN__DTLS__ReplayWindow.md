

# Struct SYN\_DTLS\_ReplayWindow



[**ClassList**](annotated.md) **>** [**SYN\_DTLS\_ReplayWindow**](structSYN__DTLS__ReplayWindow.md)



_64-packet Sliding Window Anti-Replay Filter (RFC 9147 Section 4.5.2)._ 

* `#include <syn_dtls.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint64\_t | [**bitmap**](#variable-bitmap)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**initialized**](#variable-initialized)  <br> |
|  uint64\_t | [**max\_seq**](#variable-max_seq)  <br> |












































## Public Attributes Documentation




### variable bitmap 

```C++
uint64_t SYN_DTLS_ReplayWindow::bitmap;
```



64-bit window bitmask 


        

<hr>



### variable initialized 

```C++
bool SYN_DTLS_ReplayWindow::initialized;
```



True once the first valid packet is processed 


        

<hr>



### variable max\_seq 

```C++
uint64_t SYN_DTLS_ReplayWindow::max_seq;
```



Highest verified sequence number received 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dtls.h`

