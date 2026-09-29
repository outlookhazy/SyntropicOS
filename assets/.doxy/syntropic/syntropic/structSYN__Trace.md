

# Struct SYN\_Trace



[**ClassList**](annotated.md) **>** [**SYN\_Trace**](structSYN__Trace.md)



_Circular trace event buffer._ 

* `#include <syn_trace.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**capacity**](#variable-capacity)  <br> |
|  uint32\_t | [**count**](#variable-count)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**enabled**](#variable-enabled)  <br> |
|  [**SYN\_TraceEntry**](structSYN__TraceEntry.md) \* | [**entries**](#variable-entries)  <br> |
|  uint16\_t | [**head**](#variable-head)  <br> |












































## Public Attributes Documentation




### variable capacity 

```C++
uint16_t SYN_Trace::capacity;
```



Buffer size 
 


        

<hr>



### variable count 

```C++
uint32_t SYN_Trace::count;
```



Total events recorded (may wrap) 
 


        

<hr>



### variable enabled 

```C++
bool SYN_Trace::enabled;
```



Recording state active flag 


        

<hr>



### variable entries 

```C++
SYN_TraceEntry* SYN_Trace::entries;
```



Circular buffer (caller-owned) 
 


        

<hr>



### variable head 

```C++
uint16_t SYN_Trace::head;
```



Next write position 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/debug/syn_trace.h`

