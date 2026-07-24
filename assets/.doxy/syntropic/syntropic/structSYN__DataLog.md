

# Struct SYN\_DataLog



[**ClassList**](annotated.md) **>** [**SYN\_DataLog**](structSYN__DataLog.md)



_Data logger control block — ring buffer + drop counter._ 

* `#include <syn_datalog.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**dropped\_frames**](#variable-dropped_frames)  <br> |
|  [**SYN\_RingBuf**](structSYN__RingBuf.md) | [**rb**](#variable-rb)  <br> |












































## Public Attributes Documentation




### variable dropped\_frames 

```C++
uint32_t SYN_DataLog::dropped_frames;
```



Counter for frames dropped due to full buffer 


        

<hr>



### variable rb 

```C++
SYN_RingBuf SYN_DataLog::rb;
```



Ring buffer backend 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/log/syn_datalog.h`

