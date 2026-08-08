

# File syn\_nn.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_nn.c**](syn__nn_8c.md)

[Go to the source code of this file](syn__nn_8c_source.md)

_Zero-heap TinyML Neural Network Engine implementation._ 

* `#include "syn_nn.h"`
* `#include <math.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_nn\_argmax\_q7**](#function-syn_nn_argmax_q7) (const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* outputs, size\_t num\_outputs) <br>_Find the class index with the highest output value (ArgMax)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_nn\_attention\_q7**](#function-syn_nn_attention_q7) (const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* q, const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* k, const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* v, size\_t seq\_len, size\_t d\_k, size\_t d\_v, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* out, uint8\_t attn\_shift) <br>_INT8 Quantized Scaled Dot-Product Self-Attention Layer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_nn\_avgpool1d\_q7**](#function-syn_nn_avgpool1d_q7) (const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* inputs, size\_t seq\_len, size\_t num\_channels, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* outputs, size\_t pool\_size, size\_t stride) <br>_1D Average Pooling Layer for INT8 Feature Maps._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_nn\_conv1d\_pt**](#function-syn_nn_conv1d_pt) ([**SYN\_PT**](structSYN__PT.md) \* pt, const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* inputs, size\_t seq\_len, size\_t num\_channels, const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* weights, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* biases, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* outputs, size\_t num\_filters, size\_t kernel\_size, size\_t stride, [**SYN\_NN\_Activation**](syn__nn_8h.md#enum-syn_nn_activation) act, uint8\_t out\_shift, size\_t \* current\_step, size\_t chunk\_size) <br>_Evaluate a 1D Temporal Convolution layer cooperatively inside a protothread._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_nn\_conv1d\_q7**](#function-syn_nn_conv1d_q7) (const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* inputs, size\_t seq\_len, size\_t num\_channels, const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* weights, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* biases, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* outputs, size\_t num\_filters, size\_t kernel\_size, size\_t stride, [**SYN\_NN\_Activation**](syn__nn_8h.md#enum-syn_nn_activation) act, uint8\_t out\_shift) <br>_Evaluate a 1D Temporal Convolutional Neural Network layer in INT8 (q7\_t)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_nn\_conv1d\_quant\_q7**](#function-syn_nn_conv1d_quant_q7) (const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* inputs, size\_t seq\_len, size\_t num\_channels, const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* weights, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* biases, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* outputs, size\_t num\_filters, size\_t kernel\_size, size\_t stride, [**SYN\_NN\_Activation**](syn__nn_8h.md#enum-syn_nn_activation) act, const [**syn\_nn\_quant\_t**](structsyn__nn__quant__t.md) \* quant) <br>_Evaluate a 1D Temporal Convolution layer with affine quantization scaling._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_nn\_dense\_pt**](#function-syn_nn_dense_pt) ([**SYN\_PT**](structSYN__PT.md) \* pt, const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* inputs, size\_t num\_inputs, const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* weights, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* biases, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* outputs, size\_t num\_outputs, [**SYN\_NN\_Activation**](syn__nn_8h.md#enum-syn_nn_activation) act, uint8\_t out\_shift, size\_t \* current\_neuron, size\_t chunk\_size) <br>_Evaluate a Dense Neural Network layer cooperatively inside a protothread._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_nn\_dense\_q7**](#function-syn_nn_dense_q7) (const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* inputs, size\_t num\_inputs, const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* weights, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* biases, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* outputs, size\_t num\_outputs, [**SYN\_NN\_Activation**](syn__nn_8h.md#enum-syn_nn_activation) act, uint8\_t out\_shift) <br>_Evaluate a Dense (Fully Connected) Neural Network layer using INT8 (q7\_t) weights._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_nn\_dense\_quant\_q7**](#function-syn_nn_dense_quant_q7) (const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* inputs, size\_t num\_inputs, const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* weights, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* biases, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* outputs, size\_t num\_outputs, [**SYN\_NN\_Activation**](syn__nn_8h.md#enum-syn_nn_activation) act, const [**syn\_nn\_quant\_t**](structsyn__nn__quant__t.md) \* quant) <br>_Evaluate a Dense layer with affine quantization scaling._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_nn\_maxpool1d\_q7**](#function-syn_nn_maxpool1d_q7) (const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* inputs, size\_t seq\_len, size\_t num\_channels, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* outputs, size\_t pool\_size, size\_t stride) <br>_1D Max Pooling Layer for INT8 Feature Maps._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_nn\_softmax\_q7**](#function-syn_nn_softmax_q7) (const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* inputs, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* outputs, size\_t num\_inputs) <br>_Compute normalized Softmax probability distribution over input logits in Q7._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) | [**apply\_activation\_q7**](#function-apply_activation_q7) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) acc, [**SYN\_NN\_Activation**](syn__nn_8h.md#enum-syn_nn_activation) act, uint8\_t out\_shift) <br>_Apply activation and shift scaling to Q16.16 accumulator for Q7 layer._  |
|  [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) | [**apply\_activation\_quant**](#function-apply_activation_quant) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) acc, [**SYN\_NN\_Activation**](syn__nn_8h.md#enum-syn_nn_activation) act, const [**syn\_nn\_quant\_t**](structsyn__nn__quant__t.md) \* quant) <br>_Apply activation and quantization to Q16.16 accumulator._  |


























## Public Functions Documentation




### function syn\_nn\_argmax\_q7 

_Find the class index with the highest output value (ArgMax)._ 
```C++
size_t syn_nn_argmax_q7 (
    const q7_t * outputs,
    size_t num_outputs
) 
```





**Parameters:**


* `outputs` Pointer to output vector. 
* `num_outputs` Number of output elements. 



**Returns:**

Index of maximum value element (0 if num\_outputs == 0 or NULL). 





        

<hr>



### function syn\_nn\_attention\_q7 

_INT8 Quantized Scaled Dot-Product Self-Attention Layer._ 
```C++
SYN_Status syn_nn_attention_q7 (
    const q7_t * q,
    const q7_t * k,
    const q7_t * v,
    size_t seq_len,
    size_t d_k,
    size_t d_v,
    q7_t * out,
    uint8_t attn_shift
) 
```



Computes Attention(Q, K, V) = Softmax( (Q \* K^T) &gt;&gt; attn\_shift ) \* V




**Parameters:**


* `q` Query matrix [seq\_len \* d\_k]. 
* `k` Key matrix [seq\_len \* d\_k]. 
* `v` Value matrix [seq\_len \* d\_v]. 
* `seq_len` Sequence length / token count. 
* `d_k` Query/Key dimension per token. 
* `d_v` Value dimension per token. 
* `out` Output matrix [seq\_len \* d\_v]. 
* `attn_shift` Right bit-shift for dot-product scaling. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on failure. 





        

<hr>



### function syn\_nn\_avgpool1d\_q7 

_1D Average Pooling Layer for INT8 Feature Maps._ 
```C++
SYN_Status syn_nn_avgpool1d_q7 (
    const q7_t * inputs,
    size_t seq_len,
    size_t num_channels,
    q7_t * outputs,
    size_t pool_size,
    size_t stride
) 
```





**Parameters:**


* `inputs` Pointer to input feature matrix. 
* `seq_len` Input sequence length. 
* `num_channels` Number of feature channels. 
* `outputs` Destination output matrix. 
* `pool_size` Pooling window size. 
* `stride` Stride step size. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on failure. 





        

<hr>



### function syn\_nn\_conv1d\_pt 

_Evaluate a 1D Temporal Convolution layer cooperatively inside a protothread._ 
```C++
SYN_PT_Status syn_nn_conv1d_pt (
    SYN_PT * pt,
    const q7_t * inputs,
    size_t seq_len,
    size_t num_channels,
    const q7_t * weights,
    const q16_t * biases,
    q7_t * outputs,
    size_t num_filters,
    size_t kernel_size,
    size_t stride,
    SYN_NN_Activation act,
    uint8_t out_shift,
    size_t * current_step,
    size_t chunk_size
) 
```





**Parameters:**


* `pt` Pointer to protothread state machine. 
* `inputs` Pointer to input matrix. 
* `seq_len` Sequence length. 
* `num_channels` Number of input channels. 
* `weights` Kernel weights matrix. 
* `biases` Bias vector or NULL. 
* `outputs` Destination output matrix. 
* `num_filters` Number of output filters. 
* `kernel_size` Kernel window size. 
* `stride` Stride step. 
* `act` Activation function. 
* `out_shift` Right bit-shift scaling factor. 
* `current_step` State variable tracking step progress across yields. 
* `chunk_size` Number of output steps to evaluate per tick. 



**Returns:**

SYN\_PT\_YIELDING while evaluating, SYN\_PT\_ENDED on completion. 





        

<hr>



### function syn\_nn\_conv1d\_q7 

_Evaluate a 1D Temporal Convolutional Neural Network layer in INT8 (q7\_t)._ 
```C++
SYN_Status syn_nn_conv1d_q7 (
    const q7_t * inputs,
    size_t seq_len,
    size_t num_channels,
    const q7_t * weights,
    const q16_t * biases,
    q7_t * outputs,
    size_t num_filters,
    size_t kernel_size,
    size_t stride,
    SYN_NN_Activation act,
    uint8_t out_shift
) 
```



Scans a 1D kernel filter matrix [num\_filters \* kernel\_size \* num\_channels] across a time series [seq\_len \* num\_channels].




**Parameters:**


* `inputs` Pointer to input matrix [seq\_len \* num\_channels]. 
* `seq_len` Input sequence length (time steps). 
* `num_channels` Number of input channels/features per time step. 
* `weights` Flat kernel weights matrix [num\_filters \* kernel\_size \* num\_channels]. 
* `biases` Bias vector of length num\_filters in Q16 (or NULL). 
* `outputs` Destination output matrix [out\_steps \* num\_filters]. 
* `num_filters` Number of output filters/channels. 
* `kernel_size` Size of 1D sliding window kernel. 
* `stride` Stride step across time steps. 
* `act` Activation function to apply to output filters. 
* `out_shift` Right bit-shift scaling factor (0 to 15) to prevent overflow. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on failure. 





        

<hr>



### function syn\_nn\_conv1d\_quant\_q7 

_Evaluate a 1D Temporal Convolution layer with affine quantization scaling._ 
```C++
SYN_Status syn_nn_conv1d_quant_q7 (
    const q7_t * inputs,
    size_t seq_len,
    size_t num_channels,
    const q7_t * weights,
    const q16_t * biases,
    q7_t * outputs,
    size_t num_filters,
    size_t kernel_size,
    size_t stride,
    SYN_NN_Activation act,
    const syn_nn_quant_t * quant
) 
```





**Parameters:**


* `inputs` Pointer to input matrix. 
* `seq_len` Sequence length. 
* `num_channels` Number of input channels. 
* `weights` Kernel weights matrix. 
* `biases` Bias vector or NULL. 
* `outputs` Destination output matrix. 
* `num_filters` Number of output filters. 
* `kernel_size` Kernel window size. 
* `stride` Stride step. 
* `act` Activation function. 
* `quant` Pointer to affine quantization scaling parameters. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on failure. 





        

<hr>



### function syn\_nn\_dense\_pt 

_Evaluate a Dense Neural Network layer cooperatively inside a protothread._ 
```C++
SYN_PT_Status syn_nn_dense_pt (
    SYN_PT * pt,
    const q7_t * inputs,
    size_t num_inputs,
    const q7_t * weights,
    const q16_t * biases,
    q7_t * outputs,
    size_t num_outputs,
    SYN_NN_Activation act,
    uint8_t out_shift,
    size_t * current_neuron,
    size_t chunk_size
) 
```





**Parameters:**


* `pt` Pointer to protothread state machine. 
* `inputs` Pointer to input vector. 
* `num_inputs` Number of input features. 
* `weights` Flat weight matrix. 
* `biases` Bias vector or NULL. 
* `outputs` Destination output vector. 
* `num_outputs` Number of output neurons. 
* `act` Activation function to apply. 
* `out_shift` Right bit-shift scaling factor. 
* `current_neuron` State variable tracking progress across yields. 
* `chunk_size` Number of neurons to evaluate per protothread tick. 



**Returns:**

SYN\_PT\_YIELDING while evaluating, SYN\_PT\_ENDED on completion. 





        

<hr>



### function syn\_nn\_dense\_q7 

_Evaluate a Dense (Fully Connected) Neural Network layer using INT8 (q7\_t) weights._ 
```C++
SYN_Status syn_nn_dense_q7 (
    const q7_t * inputs,
    size_t num_inputs,
    const q7_t * weights,
    const q16_t * biases,
    q7_t * outputs,
    size_t num_outputs,
    SYN_NN_Activation act,
    uint8_t out_shift
) 
```



Computes: Output[i] = Activation( ((Sum(Input[j] \* Weight[i][j]) + Bias[i]) &gt;&gt; out\_shift) )




**Parameters:**


* `inputs` Pointer to input vector (length = num\_inputs). 
* `num_inputs` Number of input features. 
* `weights` Flat weight matrix [num\_outputs \* num\_inputs]. 
* `biases` Bias vector of length num\_outputs in Q16.16 (or NULL). 
* `outputs` Destination buffer for outputs (length = num\_outputs). 
* `num_outputs` Number of output neurons in layer. 
* `act` Activation function to apply. 
* `out_shift` Right bit-shift scaling factor (0 to 15) to prevent overflow. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on failure. 





        

<hr>



### function syn\_nn\_dense\_quant\_q7 

_Evaluate a Dense layer with affine quantization scaling._ 
```C++
SYN_Status syn_nn_dense_quant_q7 (
    const q7_t * inputs,
    size_t num_inputs,
    const q7_t * weights,
    const q16_t * biases,
    q7_t * outputs,
    size_t num_outputs,
    SYN_NN_Activation act,
    const syn_nn_quant_t * quant
) 
```





**Parameters:**


* `inputs` Pointer to input vector. 
* `num_inputs` Number of input features. 
* `weights` Flat weight matrix. 
* `biases` Bias vector or NULL. 
* `outputs` Destination output vector. 
* `num_outputs` Number of output neurons. 
* `act` Activation function. 
* `quant` Pointer to affine quantization scaling parameters. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on failure. 





        

<hr>



### function syn\_nn\_maxpool1d\_q7 

_1D Max Pooling Layer for INT8 Feature Maps._ 
```C++
SYN_Status syn_nn_maxpool1d_q7 (
    const q7_t * inputs,
    size_t seq_len,
    size_t num_channels,
    q7_t * outputs,
    size_t pool_size,
    size_t stride
) 
```





**Parameters:**


* `inputs` Pointer to input feature matrix. 
* `seq_len` Input sequence length. 
* `num_channels` Number of feature channels. 
* `outputs` Destination output matrix. 
* `pool_size` Pooling window size. 
* `stride` Stride step size. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on failure. 





        

<hr>



### function syn\_nn\_softmax\_q7 

_Compute normalized Softmax probability distribution over input logits in Q7._ 
```C++
SYN_Status syn_nn_softmax_q7 (
    const q7_t * inputs,
    q7_t * outputs,
    size_t num_inputs
) 
```



Output values sum to 127 (+1.0 in Q7).




**Parameters:**


* `inputs` Pointer to logit input vector. 
* `outputs` Pointer to destination probability vector. 
* `num_inputs` Number of features / classes. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on failure. 





        

<hr>
## Public Static Functions Documentation




### function apply\_activation\_q7 

_Apply activation and shift scaling to Q16.16 accumulator for Q7 layer._ 
```C++
static q7_t apply_activation_q7 (
    q16_t acc,
    SYN_NN_Activation act,
    uint8_t out_shift
) 
```





**Parameters:**


* `acc` Q16.16 accumulator value. 
* `act` Activation function enum. 
* `out_shift` Output right shift bit count. 



**Returns:**

Q7 output byte. 





        

<hr>



### function apply\_activation\_quant 

_Apply activation and quantization to Q16.16 accumulator._ 
```C++
static q7_t apply_activation_quant (
    q16_t acc,
    SYN_NN_Activation act,
    const syn_nn_quant_t * quant
) 
```





**Parameters:**


* `acc` Q16.16 accumulator value. 
* `act` Activation function enum. 
* `quant` Pointer to quantization parameters structure. 



**Returns:**

Quantized Q7 output byte. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_nn.c`

