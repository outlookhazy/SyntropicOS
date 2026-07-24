

# File syn\_metrics.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_metrics.h**](syn__metrics_8h.md)

[Go to the source code of this file](syn__metrics_8h_source.md)

_Lightweight telemetry and metrics (Counter/Gauge)._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Metric**](structSYN__Metric.md) <br>_Metric descriptor and storage._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_MetricType**](#enum-syn_metrictype)  <br>_Type of value stored in a metric._  |
| union  | [**SYN\_MetricVal**](#union-syn_metricval)  <br>_Internal union for metric values._  |






















## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_metrics\_count**](#function-syn_metrics_count) (const char \* name, int32\_t delta) <br> |
|  void | [**syn\_metrics\_foreach**](#function-syn_metrics_foreach) (void(\*)(const struct [**SYN\_Metric**](structSYN__Metric.md) \*m, void \*ctx) cb, void \* ctx) <br>_Iterate over all registered metrics (no-op when disabled)._  |
|  void | [**syn\_metrics\_init**](#function-syn_metrics_init) (void) <br> |
|  void | [**syn\_metrics\_record**](#function-syn_metrics_record) (const char \* name, float value) <br> |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_METRIC\_ADD**](syn__metrics_8h.md#define-syn_metric_add) (m, v) `((void)0)`<br>_Add value to a metric (disabled)._  |
| define  | [**SYN\_METRIC\_DEC**](syn__metrics_8h.md#define-syn_metric_dec) (m) `((void)0)`<br>_Decrement a metric (disabled)._  |
| define  | [**SYN\_METRIC\_DECLARE**](syn__metrics_8h.md#define-syn_metric_declare) (var, name, help, type) <br>_Declare a metric statically (disabled)._  |
| define  | [**SYN\_METRIC\_INC**](syn__metrics_8h.md#define-syn_metric_inc) (m) `((void)0)`<br>_Increment a metric (disabled)._  |
| define  | [**SYN\_METRIC\_REGISTER**](syn__metrics_8h.md#define-syn_metric_register) (m) `((void)0)`<br>_Register a metric (disabled)._  |
| define  | [**SYN\_METRIC\_SET**](syn__metrics_8h.md#define-syn_metric_set) (m, v) `((void)0)`<br>_Set a metric value (disabled)._  |
| define  | [**SYN\_METRIC\_SUB**](syn__metrics_8h.md#define-syn_metric_sub) (m, v) `((void)0)`<br>_Subtract value from a metric (disabled)._  |

## Detailed Description


This module follows an opt-in pattern. If SYN\_USE\_METRICS is not defined or is 0, all macros expand to nothing, resulting in zero overhead. 


    
## Public Types Documentation




### enum SYN\_MetricType 

_Type of value stored in a metric._ 
```C++
enum SYN_MetricType {
    SYN_METRIC_TYPE_COUNTER,
    SYN_METRIC_TYPE_GAUGE
};
```




<hr>



### union SYN\_MetricVal 

_Internal union for metric values._ 
```C++

```




<hr>
## Public Static Functions Documentation




### function syn\_metrics\_count 

```C++
static inline void syn_metrics_count (
    const char * name,
    int32_t delta
) 
```




<hr>



### function syn\_metrics\_foreach 

_Iterate over all registered metrics (no-op when disabled)._ 
```C++
static inline void syn_metrics_foreach (
    void(*)(const struct SYN_Metric *m, void *ctx) cb,
    void * ctx
) 
```





**Parameters:**


* `cb` Callback. 
* `ctx` Context. 




        

<hr>



### function syn\_metrics\_init 

```C++
static inline void syn_metrics_init (
    void
) 
```




<hr>



### function syn\_metrics\_record 

```C++
static inline void syn_metrics_record (
    const char * name,
    float value
) 
```




<hr>
## Macro Definition Documentation





### define SYN\_METRIC\_ADD 

_Add value to a metric (disabled)._ 
```C++
#define SYN_METRIC_ADD (
    m,
    v
) `((void)0)`
```




<hr>



### define SYN\_METRIC\_DEC 

_Decrement a metric (disabled)._ 
```C++
#define SYN_METRIC_DEC (
    m
) `((void)0)`
```




<hr>



### define SYN\_METRIC\_DECLARE 

_Declare a metric statically (disabled)._ 
```C++
#define SYN_METRIC_DECLARE (
    var,
    name,
    help,
    type
) 
```




<hr>



### define SYN\_METRIC\_INC 

_Increment a metric (disabled)._ 
```C++
#define SYN_METRIC_INC (
    m
) `((void)0)`
```




<hr>



### define SYN\_METRIC\_REGISTER 

_Register a metric (disabled)._ 
```C++
#define SYN_METRIC_REGISTER (
    m
) `((void)0)`
```




<hr>



### define SYN\_METRIC\_SET 

_Set a metric value (disabled)._ 
```C++
#define SYN_METRIC_SET (
    m,
    v
) `((void)0)`
```




<hr>



### define SYN\_METRIC\_SUB 

_Subtract value from a metric (disabled)._ 
```C++
#define SYN_METRIC_SUB (
    m,
    v
) `((void)0)`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_metrics.h`

