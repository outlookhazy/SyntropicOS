

# Struct SYN\_Boot



[**ClassList**](annotated.md) **>** [**SYN\_Boot**](structSYN__Boot.md)



_Boot manager — crash-loop detection and safe-mode state._ 

* `#include <syn_boot.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_BootData**](structSYN__BootData.md) | [**data**](#variable-data)  <br> |
|  [**SYN\_ErrLog**](structSYN__ErrLog.md) \* | [**errlog**](#variable-errlog)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**initialized**](#variable-initialized)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**safe\_mode**](#variable-safe_mode)  <br> |
|  uint16\_t | [**safe\_threshold**](#variable-safe_threshold)  <br> |
|  [**SYN\_ParamStore**](structSYN__ParamStore.md) \* | [**store**](#variable-store)  <br> |












































## Public Attributes Documentation




### variable data 

```C++
SYN_BootData SYN_Boot::data;
```



Current boot data 
 


        

<hr>



### variable errlog 

```C++
SYN_ErrLog* SYN_Boot::errlog;
```



If set, boot events are logged 
 


        

<hr>



### variable initialized 

```C++
bool SYN_Boot::initialized;
```



Init complete flag 
 


        

<hr>



### variable safe\_mode 

```C++
bool SYN_Boot::safe_mode;
```



Are we in safe mode? 
 


        

<hr>



### variable safe\_threshold 

```C++
uint16_t SYN_Boot::safe_threshold;
```



fail\_count to trigger safe mode 


        

<hr>



### variable store 

```C++
SYN_ParamStore* SYN_Boot::store;
```



Param store for persistence 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_boot.h`

