

# File syn\_nn.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_nn.c**](syn__nn_8c.md)

[Go to the documentation of this file](syn__nn_8c.md)


```C++

#include "syn_nn.h"

#include <math.h>

static q7_t apply_activation_quant(q16_t acc, SYN_NN_Activation act, const syn_nn_quant_t *quant)
{
    if (quant != NULL && quant->multiplier > 0) {
        int64_t scaled = (int64_t)acc * quant->multiplier;
        acc = (q16_t)(scaled >> (16 + quant->shift));
        if (quant->zero_point != 0) {
            acc += (q16_t)quant->zero_point << 8;
        }
    }

    switch (act) {
    case SYN_NN_ACT_RELU:
        if (acc < 0)
            acc = 0;
        break;

    case SYN_NN_ACT_LEAKY_RELU:
        if (acc < 0)
            acc = acc >> 4;
        break;

    case SYN_NN_ACT_SIGMOID:
        if (acc <= Q16_FROM_INT(-4)) {
            acc = 0;
        } else if (acc >= Q16_FROM_INT(4)) {
            acc = Q16_ONE;
        } else {
            acc = Q16_HALF + (acc >> 3);
        }
        break;

    case SYN_NN_ACT_TANH:
        if (acc <= Q16_FROM_INT(-2)) {
            acc = -Q16_ONE;
        } else if (acc >= Q16_FROM_INT(2)) {
            acc = Q16_ONE;
        } else {
            acc = acc >> 1;
        }
        break;

    case SYN_NN_ACT_NONE:
    default:
        break;
    }

    return q16_to_q7(acc);
}

static q7_t apply_activation_q7(q16_t acc, SYN_NN_Activation act, uint8_t out_shift)
{
    if (out_shift > 0 && out_shift <= 16) {
        acc = acc >> out_shift;
    }

    switch (act) {
    case SYN_NN_ACT_RELU:
        if (acc < 0)
            acc = 0;
        break;

    case SYN_NN_ACT_LEAKY_RELU:
        if (acc < 0)
            acc = acc >> 4; /* Leaky slope = 0.0625 */
        break;

    case SYN_NN_ACT_SIGMOID:
        /* Fixed-point piecewise linear Sigmoid approximation: [0.0, 1.0] in Q16 */
        if (acc <= Q16_FROM_INT(-4)) {
            acc = 0;
        } else if (acc >= Q16_FROM_INT(4)) {
            acc = Q16_ONE;
        } else {
            acc = Q16_HALF + (acc >> 3);
        }
        break;

    case SYN_NN_ACT_TANH:
        /* Fixed-point piecewise linear TanH approximation: [-1.0, 1.0] in Q16 */
        if (acc <= Q16_FROM_INT(-2)) {
            acc = -Q16_ONE;
        } else if (acc >= Q16_FROM_INT(2)) {
            acc = Q16_ONE;
        } else {
            acc = acc >> 1;
        }
        break;

    case SYN_NN_ACT_NONE:
    default:
        break;
    }
    return q16_to_q7(acc);
}

SYN_Status syn_nn_dense_q7(const q7_t *inputs, size_t num_inputs, const q7_t *weights,
                           const q16_t *biases, q7_t *outputs, size_t num_outputs,
                           SYN_NN_Activation act, uint8_t out_shift)
{
    if (!inputs || !weights || !outputs || num_inputs == 0 || num_outputs == 0) {
        return SYN_INVALID_PARAM;
    }

    for (size_t i = 0; i < num_outputs; i++) {
        q16_t acc = (biases != NULL) ? biases[i] : 0;
        const q7_t *w_row = &weights[i * num_inputs];

        for (size_t j = 0; j < num_inputs; j++) {
            acc = q7_mac(acc, inputs[j], w_row[j]);
        }

        outputs[i] = apply_activation_q7(acc, act, out_shift);
    }

    return SYN_OK;
}

SYN_PT_Status syn_nn_dense_pt(SYN_PT *pt, const q7_t *inputs, size_t num_inputs,
                              const q7_t *weights, const q16_t *biases, q7_t *outputs,
                              size_t num_outputs, SYN_NN_Activation act, uint8_t out_shift,
                              size_t *current_neuron, size_t chunk_size)
{
    PT_BEGIN(pt);

    if (!inputs || !weights || !outputs || !current_neuron || num_inputs == 0 || num_outputs == 0) {
        PT_EXIT(pt);
    }

    if (chunk_size == 0) {
        chunk_size = num_outputs;
    }

    *current_neuron = 0;

    while (*current_neuron < num_outputs) {
        size_t end = *current_neuron + chunk_size;
        if (end > num_outputs) {
            end = num_outputs;
        }

        for (size_t i = *current_neuron; i < end; i++) {
            q16_t acc = (biases != NULL) ? biases[i] : 0;
            const q7_t *w_row = &weights[i * num_inputs];

            for (size_t j = 0; j < num_inputs; j++) {
                acc = q7_mac(acc, inputs[j], w_row[j]);
            }

            outputs[i] = apply_activation_q7(acc, act, out_shift);
        }

        *current_neuron = end;

        if (*current_neuron < num_outputs) {
            PT_YIELD(pt);
        }
    }

    PT_END(pt);
}

SYN_Status syn_nn_softmax_q7(const q7_t *inputs, q7_t *outputs, size_t num_inputs)
{
    if (!inputs || !outputs || num_inputs == 0) {
        return SYN_INVALID_PARAM;
    }

    /* Stack buffer is fixed at 64 entries — reject larger inputs */
    if (num_inputs > 64) {
        return SYN_INVALID_PARAM;
    }

    /* Find max input for numerical stability */
    q7_t max_val = inputs[0];
    for (size_t i = 1; i < num_inputs; i++) {
        if (inputs[i] > max_val) {
            max_val = inputs[i];
        }
    }

    /* Compute exp(x - max) approximation in Q16.16 */
    double sum = 0.0;
    double exp_vals[64];

    for (size_t i = 0; i < num_inputs; i++) {
        double diff = (double)(inputs[i] - max_val) / 128.0;
        exp_vals[i] = exp(diff);
        sum += exp_vals[i];
    }

    if (sum <= 0.0) {
        sum = 1.0; /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */
    }

    /* Normalize so sum equals 127 in Q7 (+1.0) */
    for (size_t i = 0; i < num_inputs; i++) {
        double prob = exp_vals[i] / sum;
        outputs[i] = Q7_FROM_FLOAT((float)prob);
    }

    return SYN_OK;
}

SYN_Status syn_nn_attention_q7(const q7_t *q, const q7_t *k, const q7_t *v, size_t seq_len,
                               size_t d_k, size_t d_v, q7_t *out, uint8_t attn_shift)
{
    if (!q || !k || !v || !out || seq_len == 0 || d_k == 0 || d_v == 0 || seq_len > 32) {
        return SYN_INVALID_PARAM;
    }

    /* Process attention for each query token */
    for (size_t i = 0; i < seq_len; i++) {
        const q7_t *q_i = &q[i * d_k];
        q7_t attn_scores[32];

        /* Compute dot-product Q_i * K_j^T for all key tokens */
        for (size_t j = 0; j < seq_len; j++) {
            const q7_t *k_j = &k[j * d_k];
            q16_t acc = 0;

            for (size_t m = 0; m < d_k; m++) {
                acc = q7_mac(acc, q_i[m], k_j[m]);
            }

            if (attn_shift > 0) {
                acc = acc >> attn_shift;
            }

            attn_scores[j] = q16_to_q7(acc);
        }

        /* Apply Softmax over attention scores */
        q7_t attn_weights[32];
        syn_nn_softmax_q7(attn_scores, attn_weights, seq_len);

        /* Compute weighted sum over Values V */
        for (size_t m = 0; m < d_v; m++) {
            q16_t acc = 0;
            for (size_t j = 0; j < seq_len; j++) {
                acc = q7_mac(acc, attn_weights[j], v[j * d_v + m]);
            }
            out[i * d_v + m] = q16_to_q7(acc);
        }
    }

    return SYN_OK;
}

SYN_Status syn_nn_conv1d_q7(const q7_t *inputs, size_t seq_len, size_t num_channels,
                            const q7_t *weights, const q16_t *biases, q7_t *outputs,
                            size_t num_filters, size_t kernel_size, size_t stride,
                            SYN_NN_Activation act, uint8_t out_shift)
{
    if (!inputs || !weights || !outputs || seq_len == 0 || num_channels == 0 || num_filters == 0 ||
        kernel_size == 0 || stride == 0 || kernel_size > seq_len) {
        return SYN_INVALID_PARAM;
    }

    size_t out_steps = (seq_len - kernel_size) / stride + 1;

    for (size_t s = 0; s < out_steps; s++) {
        size_t t_start = s * stride;

        for (size_t f = 0; f < num_filters; f++) {
            q16_t acc = (biases != NULL) ? biases[f] : 0;
            const q7_t *w_filter = &weights[f * kernel_size * num_channels];

            for (size_t k = 0; k < kernel_size; k++) {
                const q7_t *in_step = &inputs[(t_start + k) * num_channels];
                const q7_t *w_k = &w_filter[k * num_channels];

                for (size_t c = 0; c < num_channels; c++) {
                    acc = q7_mac(acc, in_step[c], w_k[c]);
                }
            }

            outputs[s * num_filters + f] = apply_activation_q7(acc, act, out_shift);
        }
    }

    return SYN_OK;
}

SYN_PT_Status syn_nn_conv1d_pt(SYN_PT *pt, const q7_t *inputs, size_t seq_len, size_t num_channels,
                               const q7_t *weights, const q16_t *biases, q7_t *outputs,
                               size_t num_filters, size_t kernel_size, size_t stride,
                               SYN_NN_Activation act, uint8_t out_shift, size_t *current_step,
                               size_t chunk_size)
{
    PT_BEGIN(pt);

    if (!inputs || !weights || !outputs || !current_step || seq_len == 0 || num_channels == 0 ||
        num_filters == 0 || kernel_size == 0 || stride == 0 || kernel_size > seq_len) {
        PT_EXIT(pt);
    }

    *current_step = 0;

    while (*current_step < ((seq_len - kernel_size) / stride + 1)) {
        size_t out_steps = (seq_len - kernel_size) / stride + 1;
        size_t step_chunk = (chunk_size == 0) ? out_steps : chunk_size;
        size_t end = *current_step + step_chunk;
        if (end > out_steps) {
            end = out_steps;
        }

        for (size_t s = *current_step; s < end; s++) {
            size_t t_start = s * stride;

            for (size_t f = 0; f < num_filters; f++) {
                q16_t acc = (biases != NULL) ? biases[f] : 0;
                const q7_t *w_filter = &weights[f * kernel_size * num_channels];

                for (size_t k = 0; k < kernel_size; k++) {
                    const q7_t *in_step = &inputs[(t_start + k) * num_channels];
                    const q7_t *w_k = &w_filter[k * num_channels];

                    for (size_t c = 0; c < num_channels; c++) {
                        acc = q7_mac(acc, in_step[c], w_k[c]);
                    }
                }

                outputs[s * num_filters + f] = apply_activation_q7(acc, act, out_shift);
            }
        }

        *current_step = end;

        if (*current_step < out_steps) {
            PT_YIELD(pt);
        }
    }

    PT_END(pt);
}

size_t syn_nn_argmax_q7(const q7_t *outputs, size_t num_outputs)
{
    if (!outputs || num_outputs == 0) {
        return 0;
    }

    size_t max_idx = 0;
    q7_t max_val = outputs[0];

    for (size_t i = 1; i < num_outputs; i++) {
        if (outputs[i] > max_val) {
            max_val = outputs[i];
            max_idx = i;
        }
    }

    return max_idx;
}

SYN_Status syn_nn_conv1d_quant_q7(const q7_t *inputs, size_t seq_len, size_t num_channels,
                                  const q7_t *weights, const q16_t *biases, q7_t *outputs,
                                  size_t num_filters, size_t kernel_size, size_t stride,
                                  SYN_NN_Activation act, const syn_nn_quant_t *quant)
{
    if (!inputs || !weights || !outputs || seq_len == 0 || num_channels == 0 || num_filters == 0 ||
        kernel_size == 0 || stride == 0 || kernel_size > seq_len) {
        return SYN_INVALID_PARAM;
    }

    size_t out_steps = (seq_len - kernel_size) / stride + 1;

    for (size_t s = 0; s < out_steps; s++) {
        size_t t_start = s * stride;

        for (size_t f = 0; f < num_filters; f++) {
            q16_t acc = (biases != NULL) ? biases[f] : 0;
            const q7_t *w_filter = &weights[f * kernel_size * num_channels];

            for (size_t k = 0; k < kernel_size; k++) {
                const q7_t *in_step = &inputs[(t_start + k) * num_channels];
                const q7_t *w_k = &w_filter[k * num_channels];

                for (size_t c = 0; c < num_channels; c++) {
                    acc = q7_mac(acc, in_step[c], w_k[c]);
                }
            }

            outputs[s * num_filters + f] = apply_activation_quant(acc, act, quant);
        }
    }

    return SYN_OK;
}

SYN_Status syn_nn_dense_quant_q7(const q7_t *inputs, size_t num_inputs, const q7_t *weights,
                                 const q16_t *biases, q7_t *outputs, size_t num_outputs,
                                 SYN_NN_Activation act, const syn_nn_quant_t *quant)
{
    if (!inputs || !weights || !outputs || num_inputs == 0 || num_outputs == 0) {
        return SYN_INVALID_PARAM;
    }

    for (size_t i = 0; i < num_outputs; i++) {
        q16_t acc = (biases != NULL) ? biases[i] : 0;
        const q7_t *w_row = &weights[i * num_inputs];

        for (size_t j = 0; j < num_inputs; j++) {
            acc = q7_mac(acc, inputs[j], w_row[j]);
        }

        outputs[i] = apply_activation_quant(acc, act, quant);
    }

    return SYN_OK;
}

SYN_Status syn_nn_maxpool1d_q7(const q7_t *inputs, size_t seq_len, size_t num_channels,
                               q7_t *outputs, size_t pool_size, size_t stride)
{
    if (!inputs || !outputs || seq_len == 0 || num_channels == 0 || pool_size == 0 || stride == 0 ||
        pool_size > seq_len) {
        return SYN_INVALID_PARAM;
    }

    size_t out_steps = (seq_len - pool_size) / stride + 1;

    for (size_t s = 0; s < out_steps; s++) {
        size_t t_start = s * stride;
        for (size_t c = 0; c < num_channels; c++) {
            q7_t max_val = inputs[t_start * num_channels + c];
            for (size_t p = 1; p < pool_size; p++) {
                q7_t val = inputs[(t_start + p) * num_channels + c];
                if (val > max_val) {
                    max_val = val;
                }
            }
            outputs[s * num_channels + c] = max_val;
        }
    }

    return SYN_OK;
}

SYN_Status syn_nn_avgpool1d_q7(const q7_t *inputs, size_t seq_len, size_t num_channels,
                               q7_t *outputs, size_t pool_size, size_t stride)
{
    if (!inputs || !outputs || seq_len == 0 || num_channels == 0 || pool_size == 0 || stride == 0 ||
        pool_size > seq_len) {
        return SYN_INVALID_PARAM;
    }

    size_t out_steps = (seq_len - pool_size) / stride + 1;

    for (size_t s = 0; s < out_steps; s++) {
        size_t t_start = s * stride;
        for (size_t c = 0; c < num_channels; c++) {
            int32_t sum = 0;
            for (size_t p = 0; p < pool_size; p++) {
                sum += inputs[(t_start + p) * num_channels + c];
            }
            int32_t avg = sum / (int32_t)pool_size;
            if (avg > 127)
                avg = 127; /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */
            if (avg < -128)
                avg = -128; /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */
            outputs[s * num_channels + c] = (q7_t)avg;
        }
    }

    return SYN_OK;
}
```


