/**
 * @file syn_nn.h
 * @brief Zero-heap TinyML Neural Network Engine with Attention & Quantization Scaling.
 *
 * Implements quantized INT8 (q7_t) Dense, Attention, Softmax, and Activation
 * functions with zero dynamic memory allocation.
 */

#ifndef SYN_NN_H
#define SYN_NN_H

#include "../common/syn_defs.h"
#include "../pt/syn_pt.h"
#include "syn_qmath.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Neural Network Activation Functions.
 */
typedef enum {
    SYN_NN_ACT_NONE = 0,   /**< Linear pass-through (no activation) */
    SYN_NN_ACT_RELU,       /**< Rectified Linear Unit (max(0, x))    */
    SYN_NN_ACT_LEAKY_RELU, /**< Leaky ReLU (x > 0 ? x : x / 16)      */
    SYN_NN_ACT_SIGMOID,    /**< Fixed-point Sigmoid approximation   */
    SYN_NN_ACT_TANH        /**< Fixed-point TanH approximation      */
} SYN_NN_Activation;

/**
 * @brief Affine Quantization Scaling Parameters (Scale & Zero-Point).
 *
 * Implements TFLite-compatible affine output scaling:
 * output = clamp_int8( ((acc * multiplier) >> (16 + shift)) + zero_point )
 */
typedef struct {
    uint16_t multiplier; /**< Fixed-point Q0.16 scale multiplier (1 to 65535) */
    uint8_t shift;       /**< Right bit-shift factor (0 to 16) */
    int8_t zero_point;   /**< Quantization zero-point offset (-128 to 127) */
} syn_nn_quant_t;

/**
 * @brief Layer Type Enum for Declarative Models.
 */
typedef enum { SYN_NN_LAYER_DENSE = 0, SYN_NN_LAYER_ATTENTION } SYN_NN_LayerType;

/**
 * @brief Layer Descriptor Struct.
 */
typedef struct {
    SYN_NN_LayerType type; /**< Layer type enum (Dense or Attention) */
    size_t num_inputs;     /**< Number of input features */
    size_t num_outputs;    /**< Number of output neurons/filters */
    const q7_t *weights;   /**< Pointer to layer INT8 weights array */
    const q16_t *biases;   /**< Pointer to layer Q16 biases array */
    SYN_NN_Activation act; /**< Activation function enum */
    uint8_t out_shift;     /**< Right bit-shift output scaling factor */
} SYN_NN_Layer;

/**
 * @brief Model Descriptor Struct.
 */
typedef struct {
    const SYN_NN_Layer *layers; /**< Pointer to array of network layers */
    size_t num_layers;          /**< Number of layers in model */
} SYN_NN_Model;

/**
 * @brief Evaluate a Dense (Fully Connected) Neural Network layer using INT8 (q7_t) weights.
 *
 * Computes: Output[i] = Activation( ((Sum(Input[j] * Weight[i][j]) + Bias[i]) >> out_shift) )
 *
 * @param inputs       Pointer to input vector (length = num_inputs).
 * @param num_inputs   Number of input features.
 * @param weights      Flat weight matrix [num_outputs * num_inputs].
 * @param biases       Bias vector of length num_outputs in Q16.16 (or NULL).
 * @param outputs      Destination buffer for outputs (length = num_outputs).
 * @param num_outputs  Number of output neurons in layer.
 * @param act          Activation function to apply.
 * @param out_shift    Right bit-shift scaling factor (0 to 15) to prevent overflow.
 * @return SYN_OK on success, SYN_INVALID_PARAM on failure.
 */
SYN_Status syn_nn_dense_q7(const q7_t *inputs, size_t num_inputs, const q7_t *weights,
                           const q16_t *biases, q7_t *outputs, size_t num_outputs,
                           SYN_NN_Activation act, uint8_t out_shift);

/**
 * @brief Evaluate a Dense Neural Network layer cooperatively inside a protothread.
 *
 * @param pt             Pointer to protothread state machine.
 * @param inputs         Pointer to input vector.
 * @param num_inputs     Number of input features.
 * @param weights        Flat weight matrix.
 * @param biases         Bias vector or NULL.
 * @param outputs        Destination output vector.
 * @param num_outputs    Number of output neurons.
 * @param act            Activation function to apply.
 * @param out_shift      Right bit-shift scaling factor.
 * @param current_neuron State variable tracking progress across yields.
 * @param chunk_size     Number of neurons to evaluate per protothread tick.
 * @return PT_MORE while evaluating chunks, PT_ENDED on completion, or PT_EXITED on invalid input.
 */
SYN_PT_Status syn_nn_dense_pt(SYN_PT *pt, const q7_t *inputs, size_t num_inputs,
                              const q7_t *weights, const q16_t *biases, q7_t *outputs,
                              size_t num_outputs, SYN_NN_Activation act, uint8_t out_shift,
                              size_t *current_neuron, size_t chunk_size);

/**
 * @brief Compute normalized Softmax probability distribution over input logits in Q7.
 *
 * Output values sum to 127 (+1.0 in Q7).
 *
 * @param inputs      Pointer to logit input vector.
 * @param outputs     Pointer to destination probability vector.
 * @param num_inputs  Number of features / classes.
 * @return SYN_OK on success, SYN_INVALID_PARAM on failure.
 */
SYN_Status syn_nn_softmax_q7(const q7_t *inputs, q7_t *outputs, size_t num_inputs);

/**
 * @brief INT8 Quantized Scaled Dot-Product Self-Attention Layer.
 *
 * Computes Attention(Q, K, V) = Softmax( (Q * K^T) >> attn_shift ) * V
 *
 * @param q           Query matrix [seq_len * d_k].
 * @param k           Key matrix [seq_len * d_k].
 * @param v           Value matrix [seq_len * d_v].
 * @param seq_len     Sequence length / token count.
 * @param d_k         Query/Key dimension per token.
 * @param d_v         Value dimension per token.
 * @param out         Output matrix [seq_len * d_v].
 * @param attn_shift  Right bit-shift for dot-product scaling.
 * @return SYN_OK on success, SYN_INVALID_PARAM on failure.
 */
SYN_Status syn_nn_attention_q7(const q7_t *q, const q7_t *k, const q7_t *v, size_t seq_len,
                               size_t d_k, size_t d_v, q7_t *out, uint8_t attn_shift);

/**
 * @brief Evaluate a Scaled Dot-Product Self-Attention layer cooperatively inside a protothread.
 *
 * Slices attention evaluation across query tokens in chunks of @p chunk_size.
 * Pair with PT_SPAWN_IF_NEEDED() inside a SYN_Task to run at full speed when
 * uncontended and yield mid-sequence when another task requires CPU time.
 *
 * @param pt            Pointer to protothread state machine.
 * @param q             Query matrix [seq_len * d_k].
 * @param k             Key matrix [seq_len * d_k].
 * @param v             Value matrix [seq_len * d_v].
 * @param seq_len       Sequence length / token count (1 to 32).
 * @param d_k           Query/Key dimension per token.
 * @param d_v           Value dimension per token.
 * @param out           Output matrix [seq_len * d_v].
 * @param attn_shift    Right bit-shift for dot-product scaling.
 * @param current_token State variable tracking query token progress across yields.
 * @param chunk_size    Number of query tokens to evaluate per chunk.
 * @return PT_MORE while evaluating chunks, PT_ENDED on completion, or PT_EXITED on invalid input.
 */
SYN_PT_Status syn_nn_attention_pt(SYN_PT *pt, const q7_t *q, const q7_t *k, const q7_t *v,
                                  size_t seq_len, size_t d_k, size_t d_v, q7_t *out,
                                  uint8_t attn_shift, size_t *current_token, size_t chunk_size);

/**
 * @brief Evaluate a 1D Temporal Convolutional Neural Network layer in INT8 (q7_t).
 *
 * Scans a 1D kernel filter matrix [num_filters * kernel_size * num_channels] across a time series
 * [seq_len * num_channels].
 *
 * @param inputs       Pointer to input matrix [seq_len * num_channels].
 * @param seq_len      Input sequence length (time steps).
 * @param num_channels Number of input channels/features per time step.
 * @param weights      Flat kernel weights matrix [num_filters * kernel_size * num_channels].
 * @param biases       Bias vector of length num_filters in Q16 (or NULL).
 * @param outputs      Destination output matrix [out_steps * num_filters].
 * @param num_filters  Number of output filters/channels.
 * @param kernel_size  Size of 1D sliding window kernel.
 * @param stride       Stride step across time steps.
 * @param act          Activation function to apply to output filters.
 * @param out_shift    Right bit-shift scaling factor (0 to 15) to prevent overflow.
 * @return SYN_OK on success, SYN_INVALID_PARAM on failure.
 */
SYN_Status syn_nn_conv1d_q7(const q7_t *inputs, size_t seq_len, size_t num_channels,
                            const q7_t *weights, const q16_t *biases, q7_t *outputs,
                            size_t num_filters, size_t kernel_size, size_t stride,
                            SYN_NN_Activation act, uint8_t out_shift);

/**
 * @brief Evaluate a 1D Temporal Convolution layer cooperatively inside a protothread.
 *
 * @param pt           Pointer to protothread state machine.
 * @param inputs       Pointer to input matrix.
 * @param seq_len      Sequence length.
 * @param num_channels Number of input channels.
 * @param weights      Kernel weights matrix.
 * @param biases       Bias vector or NULL.
 * @param outputs      Destination output matrix.
 * @param num_filters  Number of output filters.
 * @param kernel_size  Kernel window size.
 * @param stride       Stride step.
 * @param act          Activation function.
 * @param out_shift    Right bit-shift scaling factor.
 * @param current_step State variable tracking step progress across yields.
 * @param chunk_size   Number of output steps to evaluate per tick.
 * @return PT_MORE while evaluating chunks, PT_ENDED on completion, or PT_EXITED on invalid input.
 */
SYN_PT_Status syn_nn_conv1d_pt(SYN_PT *pt, const q7_t *inputs, size_t seq_len, size_t num_channels,
                               const q7_t *weights, const q16_t *biases, q7_t *outputs,
                               size_t num_filters, size_t kernel_size, size_t stride,
                               SYN_NN_Activation act, uint8_t out_shift, size_t *current_step,
                               size_t chunk_size);

/**
 * @brief Evaluate a 1D Temporal Convolution layer with affine quantization scaling.
 *
 * @param inputs       Pointer to input matrix.
 * @param seq_len      Sequence length.
 * @param num_channels Number of input channels.
 * @param weights      Kernel weights matrix.
 * @param biases       Bias vector or NULL.
 * @param outputs      Destination output matrix.
 * @param num_filters  Number of output filters.
 * @param kernel_size  Kernel window size.
 * @param stride       Stride step.
 * @param act          Activation function.
 * @param quant        Pointer to affine quantization scaling parameters.
 * @return SYN_OK on success, SYN_INVALID_PARAM on failure.
 */
SYN_Status syn_nn_conv1d_quant_q7(const q7_t *inputs, size_t seq_len, size_t num_channels,
                                  const q7_t *weights, const q16_t *biases, q7_t *outputs,
                                  size_t num_filters, size_t kernel_size, size_t stride,
                                  SYN_NN_Activation act, const syn_nn_quant_t *quant);

/**
 * @brief Evaluate a Dense layer with affine quantization scaling.
 *
 * @param inputs      Pointer to input vector.
 * @param num_inputs  Number of input features.
 * @param weights     Flat weight matrix.
 * @param biases      Bias vector or NULL.
 * @param outputs     Destination output vector.
 * @param num_outputs Number of output neurons.
 * @param act         Activation function.
 * @param quant       Pointer to affine quantization scaling parameters.
 * @return SYN_OK on success, SYN_INVALID_PARAM on failure.
 */
SYN_Status syn_nn_dense_quant_q7(const q7_t *inputs, size_t num_inputs, const q7_t *weights,
                                 const q16_t *biases, q7_t *outputs, size_t num_outputs,
                                 SYN_NN_Activation act, const syn_nn_quant_t *quant);

/**
 * @brief 1D Max Pooling Layer for INT8 Feature Maps.
 *
 * @param inputs       Pointer to input feature matrix.
 * @param seq_len      Input sequence length.
 * @param num_channels Number of feature channels.
 * @param outputs      Destination output matrix.
 * @param pool_size    Pooling window size.
 * @param stride       Stride step size.
 * @return SYN_OK on success, SYN_INVALID_PARAM on failure.
 */
SYN_Status syn_nn_maxpool1d_q7(const q7_t *inputs, size_t seq_len, size_t num_channels,
                               q7_t *outputs, size_t pool_size, size_t stride);

/**
 * @brief 1D Average Pooling Layer for INT8 Feature Maps.
 *
 * @param inputs       Pointer to input feature matrix.
 * @param seq_len      Input sequence length.
 * @param num_channels Number of feature channels.
 * @param outputs      Destination output matrix.
 * @param pool_size    Pooling window size.
 * @param stride       Stride step size.
 * @return SYN_OK on success, SYN_INVALID_PARAM on failure.
 */
SYN_Status syn_nn_avgpool1d_q7(const q7_t *inputs, size_t seq_len, size_t num_channels,
                               q7_t *outputs, size_t pool_size, size_t stride);

/**
 * @brief Find the class index with the highest output value (ArgMax).
 *
 * @param outputs     Pointer to output vector.
 * @param num_outputs Number of output elements.
 * @return Index of maximum value element (0 if num_outputs == 0 or NULL).
 */
size_t syn_nn_argmax_q7(const q7_t *outputs, size_t num_outputs);

#ifdef __cplusplus
}
#endif

#endif /* SYN_NN_H */
