

# Struct SYN\_NN\_Layer



[**ClassList**](annotated.md) **>** [**SYN\_NN\_Layer**](structSYN__NN__Layer.md)



_Layer Descriptor Struct._ 

* `#include <syn_nn.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_NN\_Activation**](syn__nn_8h.md#enum-syn_nn_activation) | [**act**](#variable-act)  <br> |
|  const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* | [**biases**](#variable-biases)  <br> |
|  size\_t | [**num\_inputs**](#variable-num_inputs)  <br> |
|  size\_t | [**num\_outputs**](#variable-num_outputs)  <br> |
|  uint8\_t | [**out\_shift**](#variable-out_shift)  <br> |
|  [**SYN\_NN\_LayerType**](syn__nn_8h.md#enum-syn_nn_layertype) | [**type**](#variable-type)  <br> |
|  const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* | [**weights**](#variable-weights)  <br> |












































## Public Attributes Documentation




### variable act 

```C++
SYN_NN_Activation SYN_NN_Layer::act;
```



Activation function enum 


        

<hr>



### variable biases 

```C++
const q16_t* SYN_NN_Layer::biases;
```



Pointer to layer Q16 biases array 


        

<hr>



### variable num\_inputs 

```C++
size_t SYN_NN_Layer::num_inputs;
```



Number of input features 


        

<hr>



### variable num\_outputs 

```C++
size_t SYN_NN_Layer::num_outputs;
```



Number of output neurons/filters 


        

<hr>



### variable out\_shift 

```C++
uint8_t SYN_NN_Layer::out_shift;
```



Right bit-shift output scaling factor 


        

<hr>



### variable type 

```C++
SYN_NN_LayerType SYN_NN_Layer::type;
```



Layer type enum (Dense or Attention) 


        

<hr>



### variable weights 

```C++
const q7_t* SYN_NN_Layer::weights;
```



Pointer to layer INT8 weights array 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_nn.h`

