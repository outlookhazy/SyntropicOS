

# Struct SYN\_Watchdog



[**ClassList**](annotated.md) **>** [**SYN\_Watchdog**](structSYN__Watchdog.md)



_Software watchdog — monitors task deadlines._ 

* `#include <syn_watchdog.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_WDT\_TimeoutCallback**](syn__watchdog_8h.md#typedef-syn_wdt_timeoutcallback) | [**callback**](#variable-callback)  <br> |
|  uint8\_t | [**capacity**](#variable-capacity)  <br> |
|  uint8\_t | [**count**](#variable-count)  <br> |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  [**SYN\_WDT\_Entry**](structSYN__WDT__Entry.md) \* | [**entries**](#variable-entries)  <br> |
|  [**SYN\_ErrLog**](structSYN__ErrLog.md) \* | [**errlog**](#variable-errlog)  <br> |












































## Public Attributes Documentation




### variable callback 

```C++
SYN_WDT_TimeoutCallback SYN_Watchdog::callback;
```



Function called on timeout deadline miss 


        

<hr>



### variable capacity 

```C++
uint8_t SYN_Watchdog::capacity;
```



Total slots allocated in entries array 


        

<hr>



### variable count 

```C++
uint8_t SYN_Watchdog::count;
```



Current number of registered tasks 


        

<hr>



### variable ctx 

```C++
void* SYN_Watchdog::ctx;
```



Context pointer for the callback 


        

<hr>



### variable entries 

```C++
SYN_WDT_Entry* SYN_Watchdog::entries;
```



Array of task monitoring entries 


        

<hr>



### variable errlog 

```C++
SYN_ErrLog* SYN_Watchdog::errlog;
```



If set, timeouts are logged 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_watchdog.h`

