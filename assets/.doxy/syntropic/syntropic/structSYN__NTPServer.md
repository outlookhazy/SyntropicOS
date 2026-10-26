

# Struct SYN\_NTPServer



[**ClassList**](annotated.md) **>** [**SYN\_NTPServer**](structSYN__NTPServer.md)



[More...](#detailed-description)

* `#include <syn_ntp_server.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t(\* | [**get\_epoch\_sec\_cb**](#variable-get_epoch_sec_cb)  <br> |
|  uint8\_t | [**req\_buf**](#variable-req_buf)  <br> |
|  uint8\_t | [**resp\_buf**](#variable-resp_buf)  <br> |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**sock**](#variable-sock)  <br> |
|  uint8\_t | [**stratum**](#variable-stratum)  <br> |












































## Detailed Description


NTP Server Context 


    
## Public Attributes Documentation




### variable get\_epoch\_sec\_cb 

```C++
uint32_t(* SYN_NTPServer::get_epoch_sec_cb) (void);
```



Callback for UTC epoch seconds 


        

<hr>



### variable req\_buf 

```C++
uint8_t SYN_NTPServer::req_buf[SYN_NTP_PACKET_LEN];
```



Input request buffer 


        

<hr>



### variable resp\_buf 

```C++
uint8_t SYN_NTPServer::resp_buf[SYN_NTP_PACKET_LEN];
```



Output response buffer 


        

<hr>



### variable sock 

```C++
SYN_Socket SYN_NTPServer::sock;
```



Bound UDP socket handle 


        

<hr>



### variable stratum 

```C++
uint8_t SYN_NTPServer::stratum;
```



Server stratum (1..15) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_ntp_server.h`

