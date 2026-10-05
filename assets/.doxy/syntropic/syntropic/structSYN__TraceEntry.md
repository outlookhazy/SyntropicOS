

# Struct SYN\_TraceEntry



[**ClassList**](annotated.md) **>** [**SYN\_TraceEntry**](structSYN__TraceEntry.md)



_Single trace event entry._ 

* `#include <syn_trace.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**event\_id**](#variable-event_id)  <br> |
|  uint32\_t | [**timestamp**](#variable-timestamp)  <br> |
|  uint16\_t | [**value**](#variable-value)  <br> |












































## Public Attributes Documentation




### variable event\_id 

```C++
uint16_t SYN_TraceEntry::event_id;
```



Application-defined event ID 
 


        

<hr>



### variable timestamp 

```C++
uint32_t SYN_TraceEntry::timestamp;
```



Tick when event occurred 
 


        

<hr>



### variable value 

```C++
uint16_t SYN_TraceEntry::value;
```



Optional 16-bit payload 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/debug/syn_trace.h`

