

# File syn\_spi\_queue.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_spi\_queue.h**](syn__spi__queue_8h.md)

[Go to the source code of this file](syn__spi__queue_8h_source.md)

_Non-blocking SPI transaction queue driver with CS management & parameter switching._ 

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_gpio.h"`
* `#include "../port/syn_port_spi.h"`
* `#include "syn_spi.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SPI\_Queue**](structSYN__SPI__Queue.md) <br>_SPI Transaction Queue instance handle._  |
| struct | [**SYN\_SPI\_Transaction**](structSYN__SPI__Transaction.md) <br>_SPI Transaction Descriptor._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_SPI\_Queue\_Callback**](#typedef-syn_spi_queue_callback)  <br>_SPI queue completion callback signature._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spi\_queue\_cancel\_all**](#function-syn_spi_queue_cancel_all) ([**SYN\_SPI\_Queue**](structSYN__SPI__Queue.md) \* q) <br>_Cancel all pending transactions in queue._  |
|  size\_t | [**syn\_spi\_queue\_count**](#function-syn_spi_queue_count) (const [**SYN\_SPI\_Queue**](structSYN__SPI__Queue.md) \* q) <br>_Query current pending transaction count._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spi\_queue\_enqueue**](#function-syn_spi_queue_enqueue) ([**SYN\_SPI\_Queue**](structSYN__SPI__Queue.md) \* q, const [**SYN\_SPI\_Transaction**](structSYN__SPI__Transaction.md) \* tx) <br>_Enqueue an SPI transaction for non-blocking execution._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spi\_queue\_init**](#function-syn_spi_queue_init) ([**SYN\_SPI\_Queue**](structSYN__SPI__Queue.md) \* q, uint8\_t bus) <br>_Initialize an SPI transaction queue for a bus._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spi\_queue\_process**](#function-syn_spi_queue_process) ([**SYN\_SPI\_Queue**](structSYN__SPI__Queue.md) \* q) <br>_Process queue state machine (triggers pending transaction if idle)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SPI\_QUEUE\_MAX\_DEPTH**](syn__spi__queue_8h.md#define-syn_spi_queue_max_depth)  `16`<br> |

## Public Types Documentation




### typedef SYN\_SPI\_Queue\_Callback 

_SPI queue completion callback signature._ 
```C++
typedef void(* SYN_SPI_Queue_Callback) (uint8_t bus, SYN_Status result, void *user_data);
```





**Parameters:**


* `bus` SPI bus index. 
* `result` SYN\_OK on success, error status code on failure. 
* `user_data` Context pointer passed with transaction descriptor. 




        

<hr>
## Public Functions Documentation




### function syn\_spi\_queue\_cancel\_all 

_Cancel all pending transactions in queue._ 
```C++
SYN_Status syn_spi_queue_cancel_all (
    SYN_SPI_Queue * q
) 
```





**Parameters:**


* `q` Pointer to initialized queue handle. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_spi\_queue\_count 

_Query current pending transaction count._ 
```C++
size_t syn_spi_queue_count (
    const SYN_SPI_Queue * q
) 
```





**Parameters:**


* `q` Pointer to queue handle. 



**Returns:**

Number of queued items (0 if NULL or empty). 





        

<hr>



### function syn\_spi\_queue\_enqueue 

_Enqueue an SPI transaction for non-blocking execution._ 
```C++
SYN_Status syn_spi_queue_enqueue (
    SYN_SPI_Queue * q,
    const SYN_SPI_Transaction * tx
) 
```





**Parameters:**


* `q` Pointer to initialized queue handle. 
* `tx` Transaction descriptor. 



**Returns:**

SYN\_OK if enqueued, SYN\_BUSY if queue is full, SYN\_INVALID\_PARAM on invalid inputs. 





        

<hr>



### function syn\_spi\_queue\_init 

_Initialize an SPI transaction queue for a bus._ 
```C++
SYN_Status syn_spi_queue_init (
    SYN_SPI_Queue * q,
    uint8_t bus
) 
```





**Parameters:**


* `q` Pointer to queue handle. 
* `bus` Target SPI bus index. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if q is NULL. 





        

<hr>



### function syn\_spi\_queue\_process 

_Process queue state machine (triggers pending transaction if idle)._ 
```C++
SYN_Status syn_spi_queue_process (
    SYN_SPI_Queue * q
) 
```





**Parameters:**


* `q` Pointer to initialized queue handle. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Macro Definition Documentation





### define SYN\_SPI\_QUEUE\_MAX\_DEPTH 

```C++
#define SYN_SPI_QUEUE_MAX_DEPTH `16`
```



Maximum pending transactions per SPI queue instance 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_spi_queue.h`

