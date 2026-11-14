

# Struct syn\_nn\_quant\_t



[**ClassList**](annotated.md) **>** [**syn\_nn\_quant\_t**](structsyn__nn__quant__t.md)



_Affine Quantization Scaling Parameters (Scale & Zero-Point)._ [More...](#detailed-description)

* `#include <syn_nn.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**multiplier**](#variable-multiplier)  <br> |
|  uint8\_t | [**shift**](#variable-shift)  <br> |
|  int8\_t | [**zero\_point**](#variable-zero_point)  <br> |












































## Detailed Description


Implements TFLite-compatible affine output scaling: output = clamp\_int8( ((acc \* multiplier) &gt;&gt; (16 + shift)) + zero\_point ) 


    
## Public Attributes Documentation




### variable multiplier 

```C++
uint16_t syn_nn_quant_t::multiplier;
```



Fixed-point Q0.16 scale multiplier (1 to 65535) 


        

<hr>



### variable shift 

```C++
uint8_t syn_nn_quant_t::shift;
```



Right bit-shift factor (0 to 16) 


        

<hr>



### variable zero\_point 

```C++
int8_t syn_nn_quant_t::zero_point;
```



Quantization zero-point offset (-128 to 127) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_nn.h`

