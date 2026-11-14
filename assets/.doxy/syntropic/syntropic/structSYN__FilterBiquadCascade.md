

# Struct SYN\_FilterBiquadCascade



[**ClassList**](annotated.md) **>** [**SYN\_FilterBiquadCascade**](structSYN__FilterBiquadCascade.md)



_Multi-stage cascaded biquad filter structure for high-order filtering._ 

* `#include <syn_biquad.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**num\_stages**](#variable-num_stages)  <br> |
|  [**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) | [**stages**](#variable-stages)  <br> |












































## Public Attributes Documentation




### variable num\_stages 

```C++
uint8_t SYN_FilterBiquadCascade::num_stages;
```



Number of active stages 


        

<hr>



### variable stages 

```C++
SYN_FilterBiquad SYN_FilterBiquadCascade::stages[SYN_BIQUAD_CASCADE_MAX_STAGES];
```



Array of biquad filter stages 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_biquad.h`

