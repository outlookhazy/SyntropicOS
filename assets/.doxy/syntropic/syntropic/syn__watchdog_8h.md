

# File syn\_watchdog.h



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_watchdog.h**](syn__watchdog_8h.md)

[Go to the source code of this file](syn__watchdog_8h_source.md)

_Task-level watchdog monitor._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../system/syn_errlog.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_WDT\_Entry**](structSYN__WDT__Entry.md) <br>_Single watchdog entry for task monitoring._  |
| struct | [**SYN\_Watchdog**](structSYN__Watchdog.md) <br>_Software watchdog — monitors task deadlines._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_WDT\_TimeoutCallback**](#typedef-syn_wdt_timeoutcallback)  <br>_Called when a task misses its check-in deadline._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_watchdog\_checkin**](#function-syn_watchdog_checkin) ([**SYN\_Watchdog**](structSYN__Watchdog.md) \* wdt, int8\_t id) <br>_Check in a task (reset its timeout)._  |
|  void | [**syn\_watchdog\_init**](#function-syn_watchdog_init) ([**SYN\_Watchdog**](structSYN__Watchdog.md) \* wdt, [**SYN\_WDT\_Entry**](structSYN__WDT__Entry.md) \* entries, uint8\_t capacity, [**SYN\_WDT\_TimeoutCallback**](syn__watchdog_8h.md#typedef-syn_wdt_timeoutcallback) callback, void \* ctx) <br>_Initialize the watchdog monitor._  |
|  int8\_t | [**syn\_watchdog\_register**](#function-syn_watchdog_register) ([**SYN\_Watchdog**](structSYN__Watchdog.md) \* wdt, const char \* name, uint32\_t timeout\_ms) <br>_Register a task for monitoring._  |
|  void | [**syn\_watchdog\_unregister**](#function-syn_watchdog_unregister) ([**SYN\_Watchdog**](structSYN__Watchdog.md) \* wdt, int8\_t id) <br>_Unregister a task._  |
|  void | [**syn\_watchdog\_update**](#function-syn_watchdog_update) ([**SYN\_Watchdog**](structSYN__Watchdog.md) \* wdt) <br>_Check all tasks for timeouts._  |




























## Detailed Description


Each registered task must "check in" periodically. If any task misses its deadline, the watchdog fires a callback (typically system reset).


This is distinct from a hardware watchdog — it monitors individual software tasks, not just the main loop.




**
**


```C++
static SYN_Watchdog wdt;
static SYN_WDT_Entry entries[3];

syn_watchdog_init(&wdt, entries, 3, on_timeout, NULL);

int id0 = syn_watchdog_register(&wdt, "sensor", 5000);  // 5s timeout
int id1 = syn_watchdog_register(&wdt, "comms",  2000);

// In each task:
syn_watchdog_checkin(&wdt, id0);

// In main loop:
syn_watchdog_update(&wdt);
```
 





    
## Public Types Documentation




### typedef SYN\_WDT\_TimeoutCallback 

_Called when a task misses its check-in deadline._ 
```C++
typedef void(* SYN_WDT_TimeoutCallback) (struct SYN_Watchdog *wdt, const SYN_WDT_Entry *entry, void *ctx);
```





**Parameters:**


* `wdt` Watchdog instance. 
* `entry` The entry that timed out. 
* `ctx` User context. 




        

<hr>
## Public Functions Documentation




### function syn\_watchdog\_checkin 

_Check in a task (reset its timeout)._ 
```C++
void syn_watchdog_checkin (
    SYN_Watchdog * wdt,
    int8_t id
) 
```





**Parameters:**


* `wdt` Watchdog. 
* `id` Task ID returned by [**syn\_watchdog\_register()**](syn__watchdog_8h.md#function-syn_watchdog_register). 




        

<hr>



### function syn\_watchdog\_init 

_Initialize the watchdog monitor._ 
```C++
void syn_watchdog_init (
    SYN_Watchdog * wdt,
    SYN_WDT_Entry * entries,
    uint8_t capacity,
    SYN_WDT_TimeoutCallback callback,
    void * ctx
) 
```





**Parameters:**


* `wdt` Watchdog instance. 
* `entries` Array of entries (caller-owned). 
* `capacity` Maximum number of monitored tasks. 
* `callback` Timeout handler. 
* `ctx` Context for callback. 




        

<hr>



### function syn\_watchdog\_register 

_Register a task for monitoring._ 
```C++
int8_t syn_watchdog_register (
    SYN_Watchdog * wdt,
    const char * name,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `wdt` Watchdog. 
* `name` Task name. 
* `timeout_ms` Maximum allowed time between check-ins. 



**Returns:**

Task ID (index) for use with [**syn\_watchdog\_checkin()**](syn__watchdog_8h.md#function-syn_watchdog_checkin), or -1 if full. 





        

<hr>



### function syn\_watchdog\_unregister 

_Unregister a task._ 
```C++
void syn_watchdog_unregister (
    SYN_Watchdog * wdt,
    int8_t id
) 
```





**Parameters:**


* `wdt` Watchdog. 
* `id` Task ID returned by [**syn\_watchdog\_register()**](syn__watchdog_8h.md#function-syn_watchdog_register). 




        

<hr>



### function syn\_watchdog\_update 

_Check all tasks for timeouts._ 
```C++
void syn_watchdog_update (
    SYN_Watchdog * wdt
) 
```



Call from your main loop or a periodic timer. If any task has exceeded its timeout, the callback is invoked.




**Parameters:**


* `wdt` Watchdog instance. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_watchdog.h`

