

# File syn\_i2c\_queue.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_i2c\_queue.h**](syn__i2c__queue_8h.md)

[Go to the source code of this file](syn__i2c__queue_8h_source.md)

_Non-blocking I2C transaction queue driver for multi-client bus access._ 

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_i2c.h"`
* `#include "../port/syn_port_i2c_async.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_I2C\_Queue**](structSYN__I2C__Queue.md) <br>_I2C Transaction Queue instance handle._  |
| struct | [**SYN\_I2C\_Transaction**](structSYN__I2C__Transaction.md) <br>_I2C Transaction Descriptor._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_I2C\_Queue\_Callback**](#typedef-syn_i2c_queue_callback)  <br>_I2C queue completion callback signature._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_i2c\_queue\_cancel\_all**](#function-syn_i2c_queue_cancel_all) ([**SYN\_I2C\_Queue**](structSYN__I2C__Queue.md) \* q) <br>_Cancel all pending transactions in queue._  |
|  size\_t | [**syn\_i2c\_queue\_count**](#function-syn_i2c_queue_count) (const [**SYN\_I2C\_Queue**](structSYN__I2C__Queue.md) \* q) <br>_Query current pending transaction count._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_i2c\_queue\_enqueue**](#function-syn_i2c_queue_enqueue) ([**SYN\_I2C\_Queue**](structSYN__I2C__Queue.md) \* q, const [**SYN\_I2C\_Transaction**](structSYN__I2C__Transaction.md) \* tx) <br>_Enqueue a transaction for non-blocking execution._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_i2c\_queue\_init**](#function-syn_i2c_queue_init) ([**SYN\_I2C\_Queue**](structSYN__I2C__Queue.md) \* q, uint8\_t bus) <br>_Initialize an I2C transaction queue for a bus._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_i2c\_queue\_process**](#function-syn_i2c_queue_process) ([**SYN\_I2C\_Queue**](structSYN__I2C__Queue.md) \* q) <br>_Process queue state machine (triggers pending transaction if idle)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_I2C\_QUEUE\_MAX\_DEPTH**](syn__i2c__queue_8h.md#define-syn_i2c_queue_max_depth)  `16`<br> |

## Public Types Documentation




### typedef SYN\_I2C\_Queue\_Callback 

_I2C queue completion callback signature._ 
```C++
typedef void(* SYN_I2C_Queue_Callback) (uint8_t bus, SYN_Status result, void *user_data);
```





**Parameters:**


* `bus` I2C bus index. 
* `result` SYN\_OK on success, error status code on failure. 
* `user_data` Context pointer passed with transaction descriptor. 




        

<hr>
## Public Functions Documentation




### function syn\_i2c\_queue\_cancel\_all 

_Cancel all pending transactions in queue._ 
```C++
SYN_Status syn_i2c_queue_cancel_all (
    SYN_I2C_Queue * q
) 
```





**Parameters:**


* `q` Pointer to initialized queue handle. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_i2c\_queue\_count 

_Query current pending transaction count._ 
```C++
size_t syn_i2c_queue_count (
    const SYN_I2C_Queue * q
) 
```





**Parameters:**


* `q` Pointer to queue handle. 



**Returns:**

Number of queued items (0 if NULL or empty). 





        

<hr>



### function syn\_i2c\_queue\_enqueue 

_Enqueue a transaction for non-blocking execution._ 
```C++
SYN_Status syn_i2c_queue_enqueue (
    SYN_I2C_Queue * q,
    const SYN_I2C_Transaction * tx
) 
```





**Parameters:**


* `q` Pointer to initialized queue handle. 
* `tx` Transaction descriptor. 



**Returns:**

SYN\_OK if enqueued, SYN\_FULL if queue is full, SYN\_INVALID\_PARAM on invalid inputs. 





        

<hr>



### function syn\_i2c\_queue\_init 

_Initialize an I2C transaction queue for a bus._ 
```C++
SYN_Status syn_i2c_queue_init (
    SYN_I2C_Queue * q,
    uint8_t bus
) 
```





**Parameters:**


* `q` Pointer to queue handle. 
* `bus` Target I2C bus index. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if q is NULL. 





        

<hr>



### function syn\_i2c\_queue\_process 

_Process queue state machine (triggers pending transaction if idle)._ 
```C++
SYN_Status syn_i2c_queue_process (
    SYN_I2C_Queue * q
) 
```





**Parameters:**


* `q` Pointer to initialized queue handle. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Macro Definition Documentation





### define SYN\_I2C\_QUEUE\_MAX\_DEPTH 

```C++
#define SYN_I2C_QUEUE_MAX_DEPTH `16`
```



Maximum pending transactions per I2C queue instance 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_i2c_queue.h`

