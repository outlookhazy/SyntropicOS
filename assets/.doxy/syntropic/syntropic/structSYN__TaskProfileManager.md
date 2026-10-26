

# Struct SYN\_TaskProfileManager



[**ClassList**](annotated.md) **>** [**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md)



_Task Profiler Registry / Manager Instance._ 

* `#include <syn_task_profile.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**count**](#variable-count)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**enabled**](#variable-enabled)  <br> |
|  [**SYN\_TaskProfile**](structSYN__TaskProfile.md) | [**profiles**](#variable-profiles)  <br> |
|  uint32\_t | [**window\_start\_us**](#variable-window_start_us)  <br> |












































## Public Attributes Documentation




### variable count 

```C++
size_t SYN_TaskProfileManager::count;
```



Profile count 
 


        

<hr>



### variable enabled 

```C++
bool SYN_TaskProfileManager::enabled;
```



Profiler enable flag 


        

<hr>



### variable profiles 

```C++
SYN_TaskProfile SYN_TaskProfileManager::profiles[SYN_TASK_PROFILE_MAX_TASKS];
```



Task profile array 
 


        

<hr>



### variable window\_start\_us 

```C++
uint32_t SYN_TaskProfileManager::window_start_us;
```



Profile window start 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/debug/syn_task_profile.h`

