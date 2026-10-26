

# File syn\_nn.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_nn.h**](syn__nn_8h.md)

[Go to the documentation of this file](syn__nn_8h.md)


```C++

#ifndef SYN_NN_H
#define SYN_NN_H

#include "syn_qmath.h"
#include "syntropic/common/syn_defs.h"
#include "syntropic/pt/syn_pt.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_NN_ACT_NONE = 0,   
    SYN_NN_ACT_RELU,       
    SYN_NN_ACT_LEAKY_RELU, 
    SYN_NN_ACT_SIGMOID,    
    SYN_NN_ACT_TANH        
} SYN_NN_Activation;

typedef struct {
    uint16_t multiplier; 
    uint8_t shift;       
    int8_t zero_point;   
} syn_nn_quant_t;

typedef enum { SYN_NN_LAYER_DENSE = 0, SYN_NN_LAYER_ATTENTION } SYN_NN_LayerType;

typedef struct {
    SYN_NN_LayerType type; 
    size_t num_inputs;     
    size_t num_outputs;    
    const q7_t *weights;   
    const q16_t *biases;   
    SYN_NN_Activation act; 
    uint8_t out_shift;     
} SYN_NN_Layer;

typedef struct {
    const SYN_NN_Layer *layers; 
    size_t num_layers;          
} SYN_NN_Model;

SYN_Status syn_nn_dense_q7(const q7_t *inputs, size_t num_inputs, const q7_t *weights,
                           const q16_t *biases, q7_t *outputs, size_t num_outputs,
                           SYN_NN_Activation act, uint8_t out_shift);

SYN_PT_Status syn_nn_dense_pt(SYN_PT *pt, const q7_t *inputs, size_t num_inputs,
                              const q7_t *weights, const q16_t *biases, q7_t *outputs,
                              size_t num_outputs, SYN_NN_Activation act, uint8_t out_shift,
                              size_t *current_neuron, size_t chunk_size);

SYN_Status syn_nn_softmax_q7(const q7_t *inputs, q7_t *outputs, size_t num_inputs);

SYN_Status syn_nn_attention_q7(const q7_t *q, const q7_t *k, const q7_t *v, size_t seq_len,
                               size_t d_k, size_t d_v, q7_t *out, uint8_t attn_shift);

SYN_Status syn_nn_conv1d_q7(const q7_t *inputs, size_t seq_len, size_t num_channels,
                            const q7_t *weights, const q16_t *biases, q7_t *outputs,
                            size_t num_filters, size_t kernel_size, size_t stride,
                            SYN_NN_Activation act, uint8_t out_shift);

SYN_PT_Status syn_nn_conv1d_pt(SYN_PT *pt, const q7_t *inputs, size_t seq_len, size_t num_channels,
                               const q7_t *weights, const q16_t *biases, q7_t *outputs,
                               size_t num_filters, size_t kernel_size, size_t stride,
                               SYN_NN_Activation act, uint8_t out_shift, size_t *current_step,
                               size_t chunk_size);

SYN_Status syn_nn_conv1d_quant_q7(const q7_t *inputs, size_t seq_len, size_t num_channels,
                                  const q7_t *weights, const q16_t *biases, q7_t *outputs,
                                  size_t num_filters, size_t kernel_size, size_t stride,
                                  SYN_NN_Activation act, const syn_nn_quant_t *quant);

SYN_Status syn_nn_dense_quant_q7(const q7_t *inputs, size_t num_inputs, const q7_t *weights,
                                 const q16_t *biases, q7_t *outputs, size_t num_outputs,
                                 SYN_NN_Activation act, const syn_nn_quant_t *quant);

SYN_Status syn_nn_maxpool1d_q7(const q7_t *inputs, size_t seq_len, size_t num_channels,
                               q7_t *outputs, size_t pool_size, size_t stride);

SYN_Status syn_nn_avgpool1d_q7(const q7_t *inputs, size_t seq_len, size_t num_channels,
                               q7_t *outputs, size_t pool_size, size_t stride);

size_t syn_nn_argmax_q7(const q7_t *outputs, size_t num_outputs);

#ifdef __cplusplus
}
#endif

#endif /* SYN_NN_H */
```


