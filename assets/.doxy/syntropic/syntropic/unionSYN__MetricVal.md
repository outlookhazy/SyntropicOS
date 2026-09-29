

# Union SYN\_MetricVal



[**ClassList**](annotated.md) **>** [**SYN\_MetricVal**](unionSYN__MetricVal.md)



_Internal union for metric values._ 

* `#include <syn_metrics.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint64\_t | [**counter**](#variable-counter)  <br> |
|  int32\_t | [**gauge**](#variable-gauge)  <br> |












































## Public Attributes Documentation




### variable counter 

```C++
uint64_t SYN_MetricVal::counter;
```



Value for SYN\_METRIC\_TYPE\_COUNTER 


        

<hr>



### variable gauge 

```C++
int32_t SYN_MetricVal::gauge;
```



Value for SYN\_METRIC\_TYPE\_GAUGE 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_metrics.h`

