

# Struct SYN\_SNTP



[**ClassList**](annotated.md) **>** [**SYN\_SNTP**](structSYN__SNTP.md)



_SNTP client context — caller-owned._ 

* `#include <syn_sntp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_Backoff**](structSYN__Backoff.md) | [**backoff**](#variable-backoff)  <br> |
|  [**SYN\_FilterEMA**](structSYN__FilterEMA.md) | [**drift\_filter**](#variable-drift_filter)  <br> |
|  int32\_t | [**drift\_ppm**](#variable-drift_ppm)  <br> |
|  uint32\_t | [**epoch\_frac**](#variable-epoch_frac)  <br> |
|  uint32\_t | [**epoch\_s**](#variable-epoch_s)  <br> |
|  uint32\_t | [**prev\_sync\_epoch**](#variable-prev_sync_epoch)  <br> |
|  uint32\_t | [**prev\_sync\_tick\_ms**](#variable-prev_sync_tick_ms)  <br> |
|  uint32\_t | [**recv\_deadline**](#variable-recv_deadline)  <br> |
|  [**SYN\_SockAddr**](structSYN__SockAddr.md) | [**server**](#variable-server)  <br> |
|  uint32\_t | [**sync\_interval\_s**](#variable-sync_interval_s)  <br> |
|  uint32\_t | [**sync\_tick\_ms**](#variable-sync_tick_ms)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**synced**](#variable-synced)  <br> |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**udp\_sock**](#variable-udp_sock)  <br> |












































## Public Attributes Documentation




### variable backoff 

```C++
SYN_Backoff SYN_SNTP::backoff;
```



Retry backoff state 
 


        

<hr>



### variable drift\_filter 

```C++
SYN_FilterEMA SYN_SNTP::drift_filter;
```



EMA filter for clock drift PPM smoothing 


        

<hr>



### variable drift\_ppm 

```C++
int32_t SYN_SNTP::drift_ppm;
```



Calculated & filtered clock drift in PPM (+ = fast, - = slow) 


        

<hr>



### variable epoch\_frac 

```C++
uint32_t SYN_SNTP::epoch_frac;
```



Fractional seconds (NTP format) 
 


        

<hr>



### variable epoch\_s 

```C++
uint32_t SYN_SNTP::epoch_s;
```



Last synced UTC epoch (seconds) 
 


        

<hr>



### variable prev\_sync\_epoch 

```C++
uint32_t SYN_SNTP::prev_sync_epoch;
```



Epoch at previous sync for PPM calculation 


        

<hr>



### variable prev\_sync\_tick\_ms 

```C++
uint32_t SYN_SNTP::prev_sync_tick_ms;
```



Local tick at previous sync for PPM calculation 


        

<hr>



### variable recv\_deadline 

```C++
uint32_t SYN_SNTP::recv_deadline;
```



Tick deadline for non-blocking recv 


        

<hr>



### variable server 

```C++
SYN_SockAddr SYN_SNTP::server;
```



NTP server address (IP + port) 
 


        

<hr>



### variable sync\_interval\_s 

```C++
uint32_t SYN_SNTP::sync_interval_s;
```



Re-sync interval in seconds 
 


        

<hr>



### variable sync\_tick\_ms 

```C++
uint32_t SYN_SNTP::sync_tick_ms;
```



Local tick at time of last sync 
 


        

<hr>



### variable synced 

```C++
bool SYN_SNTP::synced;
```



true after first successful sync 
 


        

<hr>



### variable udp\_sock 

```C++
SYN_Socket SYN_SNTP::udp_sock;
```



UDP socket handle 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_sntp.h`

