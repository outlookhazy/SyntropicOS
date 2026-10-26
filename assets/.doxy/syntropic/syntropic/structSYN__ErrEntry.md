

# Struct SYN\_ErrEntry



[**ClassList**](annotated.md) **>** [**SYN\_ErrEntry**](structSYN__ErrEntry.md)



_Error log entry — code + severity + context + timestamps._ 

* `#include <syn_errlog.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**\_pad**](#variable-_pad)  <br> |
|  uint32\_t | [**boot\_count**](#variable-boot_count)  <br> |
|  uint16\_t | [**code**](#variable-code)  <br> |
|  uint32\_t | [**context**](#variable-context)  <br> |
|  uint8\_t | [**severity**](#variable-severity)  <br> |
|  uint32\_t | [**timestamp**](#variable-timestamp)  <br> |












































## Public Attributes Documentation




### variable \_pad 

```C++
uint8_t SYN_ErrEntry::_pad;
```



Padding for alignment 
 


        

<hr>



### variable boot\_count 

```C++
uint32_t SYN_ErrEntry::boot_count;
```



Boot number when error occurred 
 


        

<hr>



### variable code 

```C++
uint16_t SYN_ErrEntry::code;
```



Application-defined error code 
 


        

<hr>



### variable context 

```C++
uint32_t SYN_ErrEntry::context;
```



Application-defined context value 
 


        

<hr>



### variable severity 

```C++
uint8_t SYN_ErrEntry::severity;
```



SYN\_ErrSeverity 
 


        

<hr>



### variable timestamp 

```C++
uint32_t SYN_ErrEntry::timestamp;
```



Tick at time of error 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_errlog.h`

