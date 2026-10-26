

# Struct SYN\_J1939\_DTCLog



[**ClassList**](annotated.md) **>** [**SYN\_J1939\_DTCLog**](structSYN__J1939__DTCLog.md)



_J1939 active and previously active DTC log container._ 

* `#include <syn_j1939.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**active\_count**](#variable-active_count)  <br> |
|  [**SYN\_J1939\_DTC**](structSYN__J1939__DTC.md) | [**active\_dtcs**](#variable-active_dtcs)  <br> |
|  size\_t | [**prev\_count**](#variable-prev_count)  <br> |
|  [**SYN\_J1939\_DTC**](structSYN__J1939__DTC.md) | [**prev\_dtcs**](#variable-prev_dtcs)  <br> |












































## Public Attributes Documentation




### variable active\_count 

```C++
size_t SYN_J1939_DTCLog::active_count;
```



Count of active DTCs. 


        

<hr>



### variable active\_dtcs 

```C++
SYN_J1939_DTC SYN_J1939_DTCLog::active_dtcs[SYN_J1939_MAX_LOGGED_DTCS];
```



Currently active DTC array (DM1). 


        

<hr>



### variable prev\_count 

```C++
size_t SYN_J1939_DTCLog::prev_count;
```



Count of previously active DTCs. 


        

<hr>



### variable prev\_dtcs 

```C++
SYN_J1939_DTC SYN_J1939_DTCLog::prev_dtcs[SYN_J1939_MAX_LOGGED_DTCS];
```



Previously active DTC array (DM2). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_j1939.h`

