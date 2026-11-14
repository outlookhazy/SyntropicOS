

# Struct SYN\_Metric



[**ClassList**](annotated.md) **>** [**SYN\_Metric**](structSYN__Metric.md)



_Metric descriptor and storage._ [More...](#detailed-description)

* `#include <syn_metrics.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const char \* | [**help**](#variable-help)  <br> |
|  const char \* | [**name**](#variable-name)  <br> |
|  struct [**SYN\_Metric**](structSYN__Metric.md) \* | [**next**](#variable-next)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**registered**](#variable-registered)  <br> |
|  [**SYN\_MetricType**](syn__metrics_8h.md#enum-syn_metrictype) | [**type**](#variable-type)  <br> |
|  [**SYN\_MetricVal**](unionSYN__MetricVal.md) | [**val**](#variable-val)  <br> |












































## Detailed Description


Metrics are typically declared statically and registered in the global registry. They are designed to be zero-heap and bare-metal friendly. 


    
## Public Attributes Documentation




### variable help 

```C++
const char* SYN_Metric::help;
```



Human-readable description 


        

<hr>



### variable name 

```C++
const char* SYN_Metric::name;
```



Unique name (e.g., "wg\_tx\_bytes") 


        

<hr>



### variable next 

```C++
struct SYN_Metric* SYN_Metric::next;
```



Next in global registry list 


        

<hr>



### variable registered 

```C++
bool SYN_Metric::registered;
```



True if in global registry 


        

<hr>



### variable type 

```C++
SYN_MetricType SYN_Metric::type;
```



Counter or Gauge 


        

<hr>



### variable val 

```C++
SYN_MetricVal SYN_Metric::val;
```



Current value 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_metrics.h`

