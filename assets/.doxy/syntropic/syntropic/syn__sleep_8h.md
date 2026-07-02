

# File syn\_sleep.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_sleep.h**](syn__sleep_8h.md)

[Go to the source code of this file](syn__sleep_8h_source.md)

_Sleep coordinator — low-power mode management._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Sleep**](structSYN__Sleep.md) <br>_Sleep coordinator — tracks wake locks and sleep statistics._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_SleepMode**](#enum-syn_sleepmode)  <br>_Sleep depth selector._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_port\_sleep**](#function-syn_port_sleep) ([**SYN\_SleepMode**](syn__sleep_8h.md#enum-syn_sleepmode) mode) <br>_Enter low-power mode. Implement for your platform._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_sleep\_any\_locked**](#function-syn_sleep_any_locked) (const [**SYN\_Sleep**](structSYN__Sleep.md) \* s) <br>_Check if any lock is held._  |
|  void | [**syn\_sleep\_enable**](#function-syn_sleep_enable) ([**SYN\_Sleep**](structSYN__Sleep.md) \* s, [**bool**](syn__defs_8h.md#enum-bool) en) <br>_Enable/disable sleep globally._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_sleep\_enter**](#function-syn_sleep_enter) ([**SYN\_Sleep**](structSYN__Sleep.md) \* s) <br>_Try to enter sleep mode._  |
|  void | [**syn\_sleep\_init**](#function-syn_sleep_init) ([**SYN\_Sleep**](structSYN__Sleep.md) \* s, [**SYN\_SleepMode**](syn__sleep_8h.md#enum-syn_sleepmode) max\_depth) <br>_Initialize the sleep coordinator._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_sleep\_is\_locked**](#function-syn_sleep_is_locked) (const [**SYN\_Sleep**](structSYN__Sleep.md) \* s, uint32\_t lock\_id) <br>_Check if a specific lock is held._  |
|  void | [**syn\_sleep\_lock**](#function-syn_sleep_lock) ([**SYN\_Sleep**](structSYN__Sleep.md) \* s, uint32\_t lock\_id) <br>_Acquire a wake lock (prevents sleep)._  |
|  uint32\_t | [**syn\_sleep\_locks**](#function-syn_sleep_locks) (const [**SYN\_Sleep**](structSYN__Sleep.md) \* s) <br>_Get active lock mask (for debug)._  |
|  void | [**syn\_sleep\_unlock**](#function-syn_sleep_unlock) ([**SYN\_Sleep**](structSYN__Sleep.md) \* s, uint32\_t lock\_id) <br>_Release a wake lock._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SLEEP\_LOCK\_ADC**](syn__sleep_8h.md#define-syn_sleep_lock_adc)  `(1u &lt;&lt; 4)`<br> |
| define  | [**SYN\_SLEEP\_LOCK\_APP0**](syn__sleep_8h.md#define-syn_sleep_lock_app0)  `(1u &lt;&lt; 8)`<br> |
| define  | [**SYN\_SLEEP\_LOCK\_APP1**](syn__sleep_8h.md#define-syn_sleep_lock_app1)  `(1u &lt;&lt; 9)`<br> |
| define  | [**SYN\_SLEEP\_LOCK\_APP2**](syn__sleep_8h.md#define-syn_sleep_lock_app2)  `(1u &lt;&lt; 10)`<br> |
| define  | [**SYN\_SLEEP\_LOCK\_APP3**](syn__sleep_8h.md#define-syn_sleep_lock_app3)  `(1u &lt;&lt; 11)`<br> |
| define  | [**SYN\_SLEEP\_LOCK\_DMA**](syn__sleep_8h.md#define-syn_sleep_lock_dma)  `(1u &lt;&lt; 3)`<br> |
| define  | [**SYN\_SLEEP\_LOCK\_I2C**](syn__sleep_8h.md#define-syn_sleep_lock_i2c)  `(1u &lt;&lt; 2)`<br> |
| define  | [**SYN\_SLEEP\_LOCK\_SPI**](syn__sleep_8h.md#define-syn_sleep_lock_spi)  `(1u &lt;&lt; 1)`<br> |
| define  | [**SYN\_SLEEP\_LOCK\_TIMER**](syn__sleep_8h.md#define-syn_sleep_lock_timer)  `(1u &lt;&lt; 5)`<br> |
| define  | [**SYN\_SLEEP\_LOCK\_UART**](syn__sleep_8h.md#define-syn_sleep_lock_uart)  `(1u &lt;&lt; 0)`<br> |

## Detailed Description


Coordinates sleep entry with the scheduler. When all tasks are idle (or only timer-blocked), the system can enter a low-power mode. Clients can veto sleep by holding a "stay awake" lock.




**
**


* Each subsystem that needs the CPU awake holds a lock.
* When all locks are released, the scheduler calls [**syn\_sleep\_enter()**](syn__sleep_8h.md#function-syn_sleep_enter) which invokes the port sleep function.
* Interrupts wake the MCU, and the scheduler resumes.






**
**


```C++
SYN_Sleep sleep;
syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);

// UART ISR: hold lock while transmitting
syn_sleep_lock(&sleep, SYN_SLEEP_LOCK_UART);
// ... transmit complete ...
syn_sleep_unlock(&sleep, SYN_SLEEP_LOCK_UART);

// In scheduler idle hook:
syn_sleep_enter(&sleep);  // sleeps only if no locks held
```
 





    
## Public Types Documentation




### enum SYN\_SleepMode 

_Sleep depth selector._ 
```C++
enum SYN_SleepMode {
    SYN_SLEEP_NONE = 0,
    SYN_SLEEP_LIGHT = 1,
    SYN_SLEEP_DEEP = 2
};
```




<hr>
## Public Functions Documentation




### function syn\_port\_sleep 

_Enter low-power mode. Implement for your platform._ 
```C++
void syn_port_sleep (
    SYN_SleepMode mode
) 
```



For Cortex-M: SYN\_SLEEP\_LIGHT → WFI, SYN\_SLEEP\_DEEP → STOP mode. Must return when an interrupt wakes the MCU.




**Parameters:**


* `mode` Requested sleep depth. 




        

<hr>
## Public Static Functions Documentation




### function syn\_sleep\_any\_locked 

_Check if any lock is held._ 
```C++
static inline bool syn_sleep_any_locked (
    const SYN_Sleep * s
) 
```





**Parameters:**


* `s` Sleep instance. 



**Returns:**

true if any lock is active. 





        

<hr>



### function syn\_sleep\_enable 

_Enable/disable sleep globally._ 
```C++
static inline void syn_sleep_enable (
    SYN_Sleep * s,
    bool en
) 
```





**Parameters:**


* `s` Sleep instance. 
* `en` true to enable, false to disable. 




        

<hr>



### function syn\_sleep\_enter 

_Try to enter sleep mode._ 
```C++
static inline bool syn_sleep_enter (
    SYN_Sleep * s
) 
```



If no locks are held and sleep is enabled, calls [**syn\_port\_sleep()**](syn__sleep_8h.md#function-syn_port_sleep).




**Parameters:**


* `s` Sleep instance. 



**Returns:**

true if we actually slept, false if vetoed. 





        

<hr>



### function syn\_sleep\_init 

_Initialize the sleep coordinator._ 
```C++
static inline void syn_sleep_init (
    SYN_Sleep * s,
    SYN_SleepMode max_depth
) 
```





**Parameters:**


* `s` Sleep instance. 
* `max_depth` Deepest sleep mode allowed. 




        

<hr>



### function syn\_sleep\_is\_locked 

_Check if a specific lock is held._ 
```C++
static inline bool syn_sleep_is_locked (
    const SYN_Sleep * s,
    uint32_t lock_id
) 
```





**Parameters:**


* `s` Sleep instance. 
* `lock_id` Lock bitmask to check. 



**Returns:**

true if the lock is active. 





        

<hr>



### function syn\_sleep\_lock 

_Acquire a wake lock (prevents sleep)._ 
```C++
static inline void syn_sleep_lock (
    SYN_Sleep * s,
    uint32_t lock_id
) 
```





**Parameters:**


* `s` Sleep instance. 
* `lock_id` Lock bitmask (e.g. SYN\_SLEEP\_LOCK\_UART). 




        

<hr>



### function syn\_sleep\_locks 

_Get active lock mask (for debug)._ 
```C++
static inline uint32_t syn_sleep_locks (
    const SYN_Sleep * s
) 
```





**Parameters:**


* `s` Sleep instance. 



**Returns:**

Active lock bitmask. 





        

<hr>



### function syn\_sleep\_unlock 

_Release a wake lock._ 
```C++
static inline void syn_sleep_unlock (
    SYN_Sleep * s,
    uint32_t lock_id
) 
```





**Parameters:**


* `s` Sleep instance. 
* `lock_id` Lock bitmask to release. 




        

<hr>
## Macro Definition Documentation





### define SYN\_SLEEP\_LOCK\_ADC 

```C++
#define SYN_SLEEP_LOCK_ADC `(1u << 4)`
```



ADC conversion in progress 


        

<hr>



### define SYN\_SLEEP\_LOCK\_APP0 

```C++
#define SYN_SLEEP_LOCK_APP0 `(1u << 8)`
```



Application-defined lock 0 


        

<hr>



### define SYN\_SLEEP\_LOCK\_APP1 

```C++
#define SYN_SLEEP_LOCK_APP1 `(1u << 9)`
```



Application-defined lock 1 


        

<hr>



### define SYN\_SLEEP\_LOCK\_APP2 

```C++
#define SYN_SLEEP_LOCK_APP2 `(1u << 10)`
```



Application-defined lock 2 


        

<hr>



### define SYN\_SLEEP\_LOCK\_APP3 

```C++
#define SYN_SLEEP_LOCK_APP3 `(1u << 11)`
```



Application-defined lock 3 


        

<hr>



### define SYN\_SLEEP\_LOCK\_DMA 

```C++
#define SYN_SLEEP_LOCK_DMA `(1u << 3)`
```



DMA transfer in progress 
 


        

<hr>



### define SYN\_SLEEP\_LOCK\_I2C 

```C++
#define SYN_SLEEP_LOCK_I2C `(1u << 2)`
```



I2C peripheral active 
 


        

<hr>



### define SYN\_SLEEP\_LOCK\_SPI 

```C++
#define SYN_SLEEP_LOCK_SPI `(1u << 1)`
```



SPI peripheral active 
 


        

<hr>



### define SYN\_SLEEP\_LOCK\_TIMER 

```C++
#define SYN_SLEEP_LOCK_TIMER `(1u << 5)`
```



Hardware timer active 
 


        

<hr>



### define SYN\_SLEEP\_LOCK\_UART 

```C++
#define SYN_SLEEP_LOCK_UART `(1u << 0)`
```



UART peripheral active 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_sleep.h`

