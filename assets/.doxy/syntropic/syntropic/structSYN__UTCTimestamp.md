

# Struct SYN\_UTCTimestamp



[**ClassList**](annotated.md) **>** [**SYN\_UTCTimestamp**](structSYN__UTCTimestamp.md)



_Universal UTC timestamp with error bound and quality tier._ 

* `#include <syn_timesync.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**nsec**](#variable-nsec)  <br> |
|  uint64\_t | [**sec**](#variable-sec)  <br> |
|  [**SYN\_TimeSource**](syn__timesync_8h.md#enum-syn_timesource) | [**source**](#variable-source)  <br> |
|  uint32\_t | [**uncertainty\_ns**](#variable-uncertainty_ns)  <br> |












































## Public Attributes Documentation




### variable nsec 

```C++
uint32_t SYN_UTCTimestamp::nsec;
```



Sub-second nanoseconds (0..999999999) 
 


        

<hr>



### variable sec 

```C++
uint64_t SYN_UTCTimestamp::sec;
```



UTC epoch seconds (1970 base) 
 


        

<hr>



### variable source 

```C++
SYN_TimeSource SYN_UTCTimestamp::source;
```



Quality tier at resolution instant 
 


        

<hr>



### variable uncertainty\_ns 

```C++
uint32_t SYN_UTCTimestamp::uncertainty_ns;
```



Quantified error bound (± nanoseconds) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_timesync.h`

