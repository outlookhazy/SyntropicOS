

# Struct SYN\_ErrLog



[**ClassList**](annotated.md) **>** [**SYN\_ErrLog**](structSYN__ErrLog.md)



_Error log instance — circular buffer of error entries._ 

* `#include <syn_errlog.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**boot\_count**](#variable-boot_count)  <br> |
|  size\_t | [**capacity**](#variable-capacity)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**enabled**](#variable-enabled)  <br> |
|  [**SYN\_ErrEntry**](structSYN__ErrEntry.md) \* | [**entries**](#variable-entries)  <br> |
|  size\_t | [**head**](#variable-head)  <br> |
|  size\_t | [**total\_count**](#variable-total_count)  <br> |












































## Public Attributes Documentation




### variable boot\_count 

```C++
uint32_t SYN_ErrLog::boot_count;
```



Current boot number 
 


        

<hr>



### variable capacity 

```C++
size_t SYN_ErrLog::capacity;
```



Max entries 
 


        

<hr>



### variable enabled 

```C++
bool SYN_ErrLog::enabled;
```



Recording enabled 
 


        

<hr>



### variable entries 

```C++
SYN_ErrEntry* SYN_ErrLog::entries;
```



Circular buffer of entries 
 


        

<hr>



### variable head 

```C++
size_t SYN_ErrLog::head;
```



Next write position 
 


        

<hr>



### variable total\_count 

```C++
size_t SYN_ErrLog::total_count;
```



Total errors ever recorded 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_errlog.h`

