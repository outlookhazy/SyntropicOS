

# Struct SYN\_Blackbox



[**ClassList**](annotated.md) **>** [**SYN\_Blackbox**](structSYN__Blackbox.md)



[More...](#detailed-description)

* `#include <syn_blackbox.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**bytes\_written**](#variable-bytes_written)  <br> |
|  uint32\_t | [**frame\_count**](#variable-frame_count)  <br> |
|  [**SYN\_Blackbox\_Record**](structSYN__Blackbox__Record.md) | [**last\_record**](#variable-last_record)  <br> |












































## Detailed Description


Blackbox Recorder Instance. 


    
## Public Attributes Documentation




### variable bytes\_written 

```C++
uint32_t SYN_Blackbox::bytes_written;
```



Total bytes written to stream 


        

<hr>



### variable frame\_count 

```C++
uint32_t SYN_Blackbox::frame_count;
```



Total frames encoded 


        

<hr>



### variable last\_record 

```C++
SYN_Blackbox_Record SYN_Blackbox::last_record;
```



Last logged record (used for delta prediction) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/log/syn_blackbox.h`

