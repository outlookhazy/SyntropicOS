/**
 * @file test_nn.c
 * @brief Comprehensive unit tests for syn_nn TinyML Neural Network Engine.
 */

#include "syntropic/dsp/syn_dsp.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/util/syn_nn.h"
#include "unity/unity.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ── 1. Q7 & Q15 Boundary Tests ─────────────────────────────────────────── */

static void test_nn_conv1d_quant_q7_and_null_checks(void)
{
    q7_t inputs[4 * 1] = {10, 20, 30, 40};
    q7_t weights[1 * 2 * 1] = {64, 64};
    q16_t biases[1] = {0};
    q7_t outputs[3 * 1] = {0};

    syn_nn_quant_t quant = {.multiplier = 32768, .shift = 1, .zero_point = 0};

    /* Conv1d quant test */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_conv1d_quant_q7(inputs, 4, 1, weights, biases, outputs, 1,
                                                         2, 1, SYN_NN_ACT_NONE, &quant));
    /* Conv1d quant test with NULL biases */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_conv1d_quant_q7(inputs, 4, 1, weights, NULL, outputs, 1, 2,
                                                         1, SYN_NN_ACT_NONE, &quant));

    /* Null checks and invalid params for conv1d quant */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_nn_conv1d_quant_q7(NULL, 4, 1, weights, biases, outputs, 1, 2, 1,
                                                 SYN_NN_ACT_NONE, &quant));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_nn_conv1d_quant_q7(inputs, 2, 1, weights, biases, outputs, 1, 4, 1,
                                                 SYN_NN_ACT_NONE, &quant)); /* kernel > seq_len */
}

void test_q7_math_boundaries_and_saturation(void)
{
    TEST_ASSERT_EQUAL_INT8(127, Q7_FROM_FLOAT(1.5f));
    TEST_ASSERT_EQUAL_INT8(-128, Q7_FROM_FLOAT(-2.0f));
    TEST_ASSERT_EQUAL_INT8(64, Q7_FROM_FLOAT(0.5f));

    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, Q7_TO_FLOAT((q7_t)64));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -0.5f, Q7_TO_FLOAT((q7_t)-64));
}

void test_q15_math_boundaries_and_saturation(void)
{
    TEST_ASSERT_EQUAL_INT16(32767, Q15_FROM_FLOAT(1.5f));
    TEST_ASSERT_EQUAL_INT16(-32768, Q15_FROM_FLOAT(-2.0f));
    TEST_ASSERT_EQUAL_INT16(16384, Q15_FROM_FLOAT(0.5f));
}

void test_cross_format_conversions(void)
{
    q7_t q7_val = Q7_FROM_FLOAT(0.5f);
    q16_t q16_val = q7_to_q16(q7_val);
    q7_t roundtrip_q7 = q16_to_q7(q16_val);

    TEST_ASSERT_EQUAL_INT8(q7_val, roundtrip_q7);
}

void test_q7_mul_and_mac(void)
{
    q16_t acc = 0;
    q7_t a = Q7_FROM_FLOAT(0.5f);
    q7_t b = Q7_FROM_FLOAT(0.5f);

    acc = q7_mac(acc, a, b);

    /* 0.5 * 0.5 = 0.25 in Q16.16 is 16384 */
    TEST_ASSERT_INT8_WITHIN(2, Q7_FROM_FLOAT(0.25f), q16_to_q7(acc));
}

/* ── 2. Activation Tests ────────────────────────────────────────────────── */

void test_nn_activations(void)
{
    q7_t in[2] = {Q7_FROM_FLOAT(0.5f), Q7_FROM_FLOAT(-0.5f)};
    q7_t weights[2 * 2] = {127, 0, 0, 127};
    q16_t biases[2] = {0, 0};
    q7_t out[2];

    /* RELU */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_nn_dense_q7(in, 2, weights, biases, out, 2, SYN_NN_ACT_RELU, 0));
    TEST_ASSERT_INT8_WITHIN(2, Q7_FROM_FLOAT(0.5f), out[0]);
    TEST_ASSERT_EQUAL_INT8(0, out[1]);

    /* LEAKY_RELU */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_nn_dense_q7(in, 2, weights, biases, out, 2, SYN_NN_ACT_LEAKY_RELU, 0));
    TEST_ASSERT_INT8_WITHIN(2, Q7_FROM_FLOAT(0.5f), out[0]);
    TEST_ASSERT_INT8_WITHIN(2, Q7_FROM_FLOAT(-0.03125f), out[1]);

    /* TANH */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_nn_dense_q7(in, 2, weights, biases, out, 2, SYN_NN_ACT_TANH, 0));
    TEST_ASSERT_INT8_WITHIN(4, Q7_FROM_FLOAT(0.25f), out[0]);
    TEST_ASSERT_INT8_WITHIN(4, Q7_FROM_FLOAT(-0.25f), out[1]);

    /* SIGMOID & NONE */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_nn_dense_q7(in, 2, weights, biases, out, 2, SYN_NN_ACT_SIGMOID, 0));
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_nn_dense_q7(in, 2, weights, biases, out, 2, SYN_NN_ACT_NONE, 0));

    /* Saturated values for Sigmoid and Tanh */
    q7_t in_sat[2] = {127, -128};
    q7_t weights_sat[2 * 2] = {127, 0, 0, 127};
    q16_t biases_sat[2] = {Q16_FROM_INT(5), Q16_FROM_INT(-5)};
    syn_nn_dense_q7(in_sat, 2, weights_sat, biases_sat, out, 2, SYN_NN_ACT_SIGMOID, 1);
    syn_nn_dense_q7(in_sat, 2, weights_sat, biases_sat, out, 2, SYN_NN_ACT_TANH, 1);
}

/* ── 3. Softmax & Attention Tests ───────────────────────────────────────── */

void test_nn_softmax(void)
{
    q7_t logits[3] = {Q7_FROM_FLOAT(0.2f), Q7_FROM_FLOAT(0.8f), Q7_FROM_FLOAT(-0.5f)};
    q7_t probs[3];

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_softmax_q7(logits, probs, 3));

    /* ArgMax of Softmax probabilities must match highest logit (index 1) */
    TEST_ASSERT_EQUAL_UINT32(1, syn_nn_argmax_q7(probs, 3));

    /* Probabilities must be positive and ordered: probs[1] > probs[0] > probs[2] */
    TEST_ASSERT_GREATER_THAN_INT8(probs[0], probs[1]);
    TEST_ASSERT_GREATER_THAN_INT8(probs[2], probs[0]);
}

void test_nn_attention(void)
{
    /* 2 tokens, d_k=2, d_v=2 */
    q7_t q[2 * 2] = {127, 0, 0, 127};
    q7_t k[2 * 2] = {127, 0, 0, 127};
    q7_t v[2 * 2] = {64, 0, 0, 64};
    q7_t out[2 * 2];

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_attention_q7(q, k, v, 2, 2, 2, out, 0));
    TEST_ASSERT_GREATER_THAN_INT8(0, out[0]);
    TEST_ASSERT_GREATER_THAN_INT8(0, out[3]);
}

/* ── 4. End-to-End DCT-II + Self-Attention Transformer Classifier Benchmark */

void test_nn_dct_transformer_pipeline(void)
{
    /* 1. Simulate 16 raw sensor samples (Vibration signal with high frequency component) */
    q7_t raw_signal[16];
    for (int i = 0; i < 16; i++) {
        float val = sinf(2.0f * (float)M_PI * (float)i / 4.0f) * 0.8f;
        raw_signal[i] = Q7_FROM_FLOAT(val);
    }

    /* 2. DCT-II Spectral Feature Extraction (Extract 4 frequency coefficients) */
    q7_t dct_coeffs[4];
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dsp_dct2_q7(raw_signal, 16, dct_coeffs, 4));

    /* 3. Scaled Dot-Product Self-Attention Layer (seq_len=2, d_k=2, d_v=2) */
    q7_t attn_out[2 * 2];
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_nn_attention_q7(dct_coeffs, dct_coeffs, dct_coeffs, 2, 2, 2, attn_out, 0));

    /* 4. Dense Output Layer -> Softmax Classification */
    q7_t dense_w[3 * 4] = {64, 32, -32, 0, -16, 64, 32, 0, 0, 0, 64, 64};
    q16_t dense_b[3] = {0, 0, 0};
    q7_t logits[3];
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_nn_dense_q7(attn_out, 4, dense_w, dense_b, logits, 3, SYN_NN_ACT_NONE, 0));

    q7_t probs[3];
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_softmax_q7(logits, probs, 3));

    size_t predicted_class = syn_nn_argmax_q7(probs, 3);
    TEST_ASSERT_LESS_THAN_UINT32(3, predicted_class);
}

/* ── 5. Protothread Coroutine Test ──────────────────────────────────────── */

void test_nn_protothread_coroutine(void)
{
    SYN_PT pt;
    PT_INIT(&pt);

    q7_t inputs[4] = {Q7_FROM_FLOAT(0.8f), Q7_FROM_FLOAT(0.9f), Q7_FROM_FLOAT(0.1f),
                      Q7_FROM_FLOAT(0.0f)};
    q7_t weights[8 * 4];
    q16_t biases[8] = {0};
    q7_t outputs[8];
    for (int i = 0; i < 32; i++) {
        weights[i] = Q7_FROM_FLOAT(0.1f);
    }

    size_t current_neuron = 0;
    size_t yields = 0;

    SYN_PT_Status status;
    do {
        status = syn_nn_dense_pt(&pt, inputs, 4, weights, biases, outputs, 8, SYN_NN_ACT_RELU, 0,
                                 &current_neuron, 2);
        if (status == PT_MORE) {
            yields++;
        }
    } while (status == PT_MORE);

    TEST_ASSERT_EQUAL_INT(PT_ENDED, status);
    TEST_ASSERT_EQUAL_UINT32(3, yields);
}

/* ── 6. 1D Convolution & Coroutine Tests ───────────────────────────────── */

void test_nn_conv1d_and_coroutine(void)
{
    /* 16 time steps, 2 input channels */
    q7_t inputs[16 * 2];
    for (int i = 0; i < 32; i++) {
        inputs[i] = Q7_FROM_FLOAT((float)(i % 5) * 0.2f);
    }

    /* 4 filters, kernel_size=3, num_channels=2 -> 4 * 3 * 2 = 24 weights */
    q7_t weights[4 * 3 * 2];
    for (int i = 0; i < 24; i++) {
        weights[i] = Q7_FROM_FLOAT(0.2f);
    }
    q16_t biases[4] = {0, 0, 0, 0};

    /* out_steps = (16 - 3) / 1 + 1 = 14 steps -> 14 * 4 filters = 56 outputs */
    q7_t outputs[14 * 4];

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_conv1d_q7(inputs, 16, 2, weights, biases, outputs, 4, 3, 1,
                                                   SYN_NN_ACT_RELU, 0));

    /* Test protothread yieldable 1D convolution */
    SYN_PT pt;
    PT_INIT(&pt);

    q7_t pt_outputs[14 * 4];
    size_t current_step = 0;
    size_t yields = 0;

    SYN_PT_Status status;
    do {
        status = syn_nn_conv1d_pt(&pt, inputs, 16, 2, weights, biases, pt_outputs, 4, 3, 1,
                                  SYN_NN_ACT_RELU, 0, &current_step, 4);
        if (status == PT_MORE) {
            yields++;
        }
    } while (status == PT_MORE);

    TEST_ASSERT_EQUAL_INT(PT_ENDED, status);
    TEST_ASSERT_EQUAL_UINT32(3, yields); /* 14 steps in chunks of 4 -> yields at step 4, 8, 12 */
    TEST_ASSERT_EQUAL_INT8_ARRAY(outputs, pt_outputs, 56);
}

void test_nn_affine_quantization(void)
{
    q7_t inputs[4] = {Q7_FROM_FLOAT(0.5f), Q7_FROM_FLOAT(0.5f), Q7_FROM_FLOAT(0.5f),
                      Q7_FROM_FLOAT(0.5f)};
    q7_t weights[2 * 4] = {64, 64, 64, 64, -64, -64, -64, -64};
    q16_t biases[2] = {0, 0};
    q7_t outputs[2];

    syn_nn_quant_t quant = {.multiplier = 32768, /* Scale factor = 0.5 */
                            .shift = 1,          /* Shift factor */
                            .zero_point = 0};

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_dense_quant_q7(inputs, 4, weights, biases, outputs, 2,
                                                        SYN_NN_ACT_RELU, &quant));
    TEST_ASSERT_GREATER_THAN_INT8(0, outputs[0]);
    TEST_ASSERT_EQUAL_INT8(0, outputs[1]);
}

void test_nn_pooling_layers(void)
{
    /* 4 time steps, 2 channels */
    q7_t inputs[4 * 2] = {10, 20, 30, 40, 50, 60, 70, 80};
    q7_t max_out[2 * 2];
    q7_t avg_out[2 * 2];

    /* pool_size=2, stride=2 -> 2 output steps */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_maxpool1d_q7(inputs, 4, 2, max_out, 2, 2));
    TEST_ASSERT_EQUAL_INT8(30, max_out[0]);
    TEST_ASSERT_EQUAL_INT8(40, max_out[1]);
    TEST_ASSERT_EQUAL_INT8(70, max_out[2]);
    TEST_ASSERT_EQUAL_INT8(80, max_out[3]);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_avgpool1d_q7(inputs, 4, 2, avg_out, 2, 2));
    TEST_ASSERT_EQUAL_INT8(20, avg_out[0]);
    TEST_ASSERT_EQUAL_INT8(30, avg_out[1]);
    TEST_ASSERT_EQUAL_INT8(60, avg_out[2]);
    TEST_ASSERT_EQUAL_INT8(70, avg_out[3]);
}

void test_nn_edge_cases_and_null_checks(void)
{
    q7_t buf[16] = {0};

    /* Invalid parameters for Pooling */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_nn_maxpool1d_q7(NULL, 4, 2, buf, 2, 2));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_nn_maxpool1d_q7(buf, 4, 2, NULL, 2, 2));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_nn_maxpool1d_q7(buf, 0, 2, buf, 2, 2));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_nn_maxpool1d_q7(buf, 4, 0, buf, 2, 2));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_nn_maxpool1d_q7(buf, 4, 2, buf, 0, 2));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_nn_maxpool1d_q7(buf, 4, 2, buf, 2, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_nn_maxpool1d_q7(buf, 4, 2, buf, 8, 2)); /* pool_size > seq_len */

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_nn_avgpool1d_q7(NULL, 4, 2, buf, 2, 2));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_nn_avgpool1d_q7(buf, 4, 2, NULL, 2, 2));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_nn_avgpool1d_q7(buf, 0, 2, buf, 2, 2));

    /* Invalid parameters for Quantized Conv1D & Dense */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_nn_conv1d_quant_q7(NULL, 4, 2, buf, NULL, buf, 2,
                                                                    2, 1, SYN_NN_ACT_NONE, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_nn_dense_quant_q7(NULL, 4, buf, NULL, buf, 2, SYN_NN_ACT_NONE, NULL));

    /* Test Quantized Activations with Sigmoid, TanH, LeakyReLU, and Zero-Point */
    syn_nn_quant_t quant_zp = {.multiplier = 32768, .shift = 1, .zero_point = 10};
    q7_t in[4] = {64, 64, 64, 64};
    q7_t w[4] = {32, 32, 32, 32};
    q7_t out[1];

    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_nn_dense_quant_q7(in, 4, w, NULL, out, 1, SYN_NN_ACT_LEAKY_RELU, &quant_zp));
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_nn_dense_quant_q7(in, 4, w, NULL, out, 1, SYN_NN_ACT_SIGMOID, &quant_zp));
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_nn_dense_quant_q7(in, 4, w, NULL, out, 1, SYN_NN_ACT_TANH, &quant_zp));

    /* ArgMax NULL checks */
    TEST_ASSERT_EQUAL_UINT32(0, syn_nn_argmax_q7(NULL, 10));
    TEST_ASSERT_EQUAL_UINT32(0, syn_nn_argmax_q7(buf, 0));
}

void test_nn_conv1d_coroutine(void)
{
    q7_t inputs[4 * 1] = {10, 20, 30, 40};
    q7_t weights[1 * 2 * 1] = {64, 64}; /* 0.5, 0.5 */
    q16_t biases[1] = {0};
    q7_t outputs[3 * 1] = {0};

    SYN_PT pt;
    PT_INIT(&pt);
    size_t current_step = 0;

    /* Execute with chunk_size = 1 so it yields every step */
    SYN_PT_Status status;
    status = syn_nn_conv1d_pt(&pt, inputs, 4, 1, weights, biases, outputs, 1, 2, 1, SYN_NN_ACT_NONE,
                              0, &current_step, 1);
    TEST_ASSERT_EQUAL_INT(PT_MORE, status);

    status = syn_nn_conv1d_pt(&pt, inputs, 4, 1, weights, biases, outputs, 1, 2, 1, SYN_NN_ACT_NONE,
                              0, &current_step, 1);
    TEST_ASSERT_EQUAL_INT(PT_MORE, status);

    status = syn_nn_conv1d_pt(&pt, inputs, 4, 1, weights, biases, outputs, 1, 2, 1, SYN_NN_ACT_NONE,
                              0, &current_step, 1);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, status);

    TEST_ASSERT_INT8_WITHIN(2, 15, outputs[0]);
    TEST_ASSERT_INT8_WITHIN(2, 25, outputs[1]);
    TEST_ASSERT_INT8_WITHIN(2, 35, outputs[2]);
}

static void test_nn_conv1d_quant_q7_param_validation_failures(void)
{
    q7_t inputs[10] = {0};
    q7_t weights[10] = {0};
    q7_t outputs[10] = {0};
    syn_nn_quant_t q = {.multiplier = 1, .shift = 0, .zero_point = 0};

    /* kernel_size > seq_len */
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_nn_conv1d_quant_q7(inputs, 2, 1, weights, NULL, outputs, 1, 5, 1, SYN_NN_ACT_NONE, &q));

    /* stride == 0 */
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_nn_conv1d_quant_q7(inputs, 5, 1, weights, NULL, outputs, 1, 2, 0, SYN_NN_ACT_NONE, &q));

    /* num_filters == 0 */
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_nn_conv1d_quant_q7(inputs, 5, 1, weights, NULL, outputs, 0, 2, 1, SYN_NN_ACT_NONE, &q));
}

static void test_nn_activation_functions_saturation(void)
{
    q7_t inputs[2] = {Q7_FROM_FLOAT(-0.8f), Q7_FROM_FLOAT(0.8f)};
    q7_t weights[2 * 2] = {127, 0, 0, 127};
    q16_t biases[2] = {0, 0};
    q7_t outputs[2];

    /* Test Leaky ReLU with negative input */
    syn_nn_dense_q7(inputs, 2, weights, biases, outputs, 2, SYN_NN_ACT_LEAKY_RELU, 0);

    /* Test Sigmoid with large inputs */
    biases[0] = Q16_FROM_INT(-10);
    biases[1] = Q16_FROM_INT(10);
    syn_nn_dense_q7(inputs, 2, weights, biases, outputs, 2, SYN_NN_ACT_SIGMOID, 0);

    /* Test Tanh with large inputs */
    syn_nn_dense_q7(inputs, 2, weights, biases, outputs, 2, SYN_NN_ACT_TANH, 0);
}

static void test_nn_conv1d_quant_q7_valid_conv_with_biases(void)
{
    q7_t inputs[4 * 1] = {Q7_FROM_FLOAT(0.5f), Q7_FROM_FLOAT(0.2f), Q7_FROM_FLOAT(-0.3f),
                          Q7_FROM_FLOAT(0.8f)};
    q7_t weights[2 * 2 * 1] = {Q7_FROM_FLOAT(0.5f), Q7_FROM_FLOAT(0.5f), Q7_FROM_FLOAT(-0.5f),
                               Q7_FROM_FLOAT(0.5f)};
    q16_t biases[2] = {Q16_FROM_INT(1), Q16_FROM_INT(-1)};
    q7_t outputs[3 * 2];
    syn_nn_quant_t q = {.multiplier = 1, .shift = 0, .zero_point = 0};

    TEST_ASSERT_EQUAL(SYN_OK, syn_nn_conv1d_quant_q7(inputs, 4, 1, weights, biases, outputs, 2, 2,
                                                     1, SYN_NN_ACT_RELU, &q));
}

static void test_nn_conv1d_quant_q7_invalid_params(void)
{
    q7_t in[4] = {10, 20, 30, 40};
    q7_t w[4] = {1, 2, 3, 4};
    q7_t out[4];

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_nn_conv1d_quant_q7(NULL, 4, 1, w, NULL, out, 1, 2, 1,
                                                                SYN_NN_ACT_NONE, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_nn_conv1d_quant_q7(in, 0, 1, w, NULL, out, 1, 2, 1,
                                                                SYN_NN_ACT_NONE, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_nn_conv1d_quant_q7(in, 4, 1, w, NULL, out, 1, 5, 1,
                                                                SYN_NN_ACT_NONE, NULL));
}

static void test_nn_softmax_q7_null_and_basic(void)
{
    q7_t in[3] = {10, 20, 30};
    q7_t out[3];
    TEST_ASSERT_EQUAL(SYN_OK, syn_nn_softmax_q7(in, out, 3));
    TEST_ASSERT_TRUE(out[2] >= out[1]);
    TEST_ASSERT_TRUE(out[1] >= out[0]);
}

static void test_nn_quant_activations_and_avgpool_clamping(void)
{
    q7_t in[2] = {100, -100};
    q7_t w[2 * 2] = {127, 0, 0, 127};
    q16_t b_sat_pos[2] = {Q16_FROM_INT(10), Q16_FROM_INT(10)};
    q16_t b_sat_neg[2] = {Q16_FROM_INT(-10), Q16_FROM_INT(-10)};
    q7_t out[2];
    syn_nn_quant_t quant = {.multiplier = 32768, .shift = 0, .zero_point = 0};

    /* LEAKY_RELU, TANH, SIGMOID with positive/negative saturation in quant mode */
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_nn_dense_quant_q7(in, 2, w, b_sat_pos, out, 2, SYN_NN_ACT_SIGMOID, &quant));
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_nn_dense_quant_q7(in, 2, w, b_sat_neg, out, 2, SYN_NN_ACT_SIGMOID, &quant));

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_nn_dense_quant_q7(in, 2, w, b_sat_pos, out, 2, SYN_NN_ACT_TANH, &quant));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_nn_dense_quant_q7(in, 2, w, b_sat_neg, out, 2, SYN_NN_ACT_TANH, &quant));

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_nn_dense_quant_q7(in, 2, w, b_sat_neg, out, 2, SYN_NN_ACT_LEAKY_RELU, &quant));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_nn_dense_quant_q7(in, 2, w, b_sat_pos, out, 2, SYN_NN_ACT_NONE, &quant));

    /* AvgPool1d clamping */
    q7_t pool_in[2] = {127, 127};
    q7_t pool_out[1];
    TEST_ASSERT_EQUAL(SYN_OK, syn_nn_avgpool1d_q7(pool_in, 2, 1, pool_out, 2, 1));

    /* Dense null checks and chunk_size=0 protothread test (lines 124, 149, 153) */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_nn_dense_q7(NULL, 2, w, b_sat_pos, out, 2, SYN_NN_ACT_NONE, 0));
    SYN_PT pt;
    PT_INIT(&pt);
    size_t cur = 0;
    syn_nn_dense_pt(&pt, NULL, 2, w, b_sat_pos, out, 2, SYN_NN_ACT_NONE, 0, &cur, 1);
    PT_INIT(&pt);
    syn_nn_dense_pt(&pt, in, 2, w, b_sat_pos, out, 2, SYN_NN_ACT_NONE, 0, &cur, 0);

    /* Dense PT chunking past end (line 161) */
    q7_t out3[3];
    q7_t w3[2 * 3] = {10, 20, 30, 40, 50, 60};
    q16_t b3[3] = {0, 0, 0};
    PT_INIT(&pt);
    cur = 0;
    syn_nn_dense_pt(&pt, in, 2, w3, b3, out3, 3, SYN_NN_ACT_NONE, 0, &cur, 2);
    syn_nn_dense_pt(&pt, in, 2, w3, b3, out3, 3, SYN_NN_ACT_NONE, 0, &cur, 2);

    /* Softmax num_inputs > 64 and num_inputs == 0 (lines 188 & 193) */
    q7_t big65[65];
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_nn_softmax_q7(big65, big65, 65));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_nn_softmax_q7(big65, big65, 0));

    /* Attention seq_len > 32 and attn_shift > 0 (lines 231 & 249) */
    q7_t q_att[2] = {10, 20}, k_att[2] = {10, 20}, v_att[2] = {10, 20}, out_att[2];
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_nn_attention_q7(q_att, k_att, v_att, 33, 1, 1, out_att, 0));
    TEST_ASSERT_EQUAL(SYN_OK, syn_nn_attention_q7(q_att, k_att, v_att, 2, 1, 1, out_att, 1));

    /* Conv1d kernel_size > seq_len (lines 279 & 317) */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_nn_conv1d_q7(in, 2, 1, w, NULL, out, 1, 3, 1, SYN_NN_ACT_NONE, 0));
    PT_INIT(&pt);
    cur = 0;
    syn_nn_conv1d_pt(&pt, in, 2, 1, w, NULL, out, 1, 3, 1, SYN_NN_ACT_NONE, 0, &cur, 1);

    /* NULL pt checks (N-14) */
    TEST_ASSERT_EQUAL(PT_EXITED,
                      syn_nn_dense_pt(NULL, in, 2, w, NULL, out, 1, SYN_NN_ACT_NONE, 0, &cur, 1));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_nn_conv1d_pt(NULL, in, 2, 1, w, NULL, out, 1, 1, 1,
                                                  SYN_NN_ACT_NONE, 0, &cur, 1));
}

/** R3-K08: SIZE_MAX chunk_size wrap protection and mid-yield chunk_size == 0 completion. */
static void test_nn_pt_chunk_size_overflow_and_zero_resume(void)
{
    q7_t d_in[2] = {32, 64};
    q7_t d_w[4 * 2] = {16, 16, 32, 32, 48, 48, 64, 64};
    q16_t d_b[4] = {0, 0, 0, 0};
    q7_t d_ref[4] = {0};
    q7_t d_out[4] = {0};
    SYN_PT pt;
    size_t cur = 0U;

    TEST_ASSERT_EQUAL(SYN_OK, syn_nn_dense_q7(d_in, 2U, d_w, d_b, d_ref, 4U, SYN_NN_ACT_NONE, 0U));

    /* 1a. syn_nn_dense_pt: yield after 1 neuron, then resume with chunk_size = SIZE_MAX */
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_MORE, syn_nn_dense_pt(&pt, d_in, 2U, d_w, d_b, d_out, 4U, SYN_NN_ACT_NONE,
                                               0U, &cur, 1U));
    TEST_ASSERT_EQUAL_UINT32(1U, (uint32_t)cur);
    TEST_ASSERT_EQUAL(PT_ENDED, syn_nn_dense_pt(&pt, d_in, 2U, d_w, d_b, d_out, 4U, SYN_NN_ACT_NONE,
                                                0U, &cur, SIZE_MAX));
    TEST_ASSERT_EQUAL_UINT32(4U, (uint32_t)cur);
    TEST_ASSERT_EQUAL_INT8_ARRAY(d_ref, d_out, 4);

    /* 1b. syn_nn_dense_pt: yield after 1 neuron, then resume with chunk_size = 0U */
    memset(d_out, 0, sizeof(d_out));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_MORE, syn_nn_dense_pt(&pt, d_in, 2U, d_w, d_b, d_out, 4U, SYN_NN_ACT_NONE,
                                               0U, &cur, 1U));
    TEST_ASSERT_EQUAL(PT_ENDED, syn_nn_dense_pt(&pt, d_in, 2U, d_w, d_b, d_out, 4U, SYN_NN_ACT_NONE,
                                                0U, &cur, 0U));
    TEST_ASSERT_EQUAL_UINT32(4U, (uint32_t)cur);
    TEST_ASSERT_EQUAL_INT8_ARRAY(d_ref, d_out, 4);

    /* 2a. syn_nn_conv1d_pt: yield after 1 step, then resume with chunk_size = SIZE_MAX */
    q7_t c_in[4] = {10, 20, 30, 40};
    q7_t c_w[2] = {64, 64};
    q16_t c_b[1] = {0};
    q7_t c_ref[3] = {0};
    q7_t c_out[3] = {0};

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_nn_conv1d_q7(c_in, 4U, 1U, c_w, c_b, c_ref, 1U, 2U, 1U, SYN_NN_ACT_NONE, 0U));

    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_MORE, syn_nn_conv1d_pt(&pt, c_in, 4U, 1U, c_w, c_b, c_out, 1U, 2U, 1U,
                                                SYN_NN_ACT_NONE, 0U, &cur, 1U));
    TEST_ASSERT_EQUAL_UINT32(1U, (uint32_t)cur);
    TEST_ASSERT_EQUAL(PT_ENDED, syn_nn_conv1d_pt(&pt, c_in, 4U, 1U, c_w, c_b, c_out, 1U, 2U, 1U,
                                                 SYN_NN_ACT_NONE, 0U, &cur, SIZE_MAX));
    TEST_ASSERT_EQUAL_UINT32(3U, (uint32_t)cur);
    TEST_ASSERT_EQUAL_INT8_ARRAY(c_ref, c_out, 3);

    /* 2b. syn_nn_conv1d_pt: yield after 1 step, then resume with chunk_size = 0U */
    memset(c_out, 0, sizeof(c_out));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_MORE, syn_nn_conv1d_pt(&pt, c_in, 4U, 1U, c_w, c_b, c_out, 1U, 2U, 1U,
                                                SYN_NN_ACT_NONE, 0U, &cur, 1U));
    TEST_ASSERT_EQUAL(PT_ENDED, syn_nn_conv1d_pt(&pt, c_in, 4U, 1U, c_w, c_b, c_out, 1U, 2U, 1U,
                                                 SYN_NN_ACT_NONE, 0U, &cur, 0U));
    TEST_ASSERT_EQUAL_UINT32(3U, (uint32_t)cur);
    TEST_ASSERT_EQUAL_INT8_ARRAY(c_ref, c_out, 3);
}

/** R4-K05: Zero-point scaling order, negative zero_point UB fix, and shift clamping. */
static void test_r4_nn_quant_zero_point_and_shift_clamp(void)
{
    q7_t in_zero[1] = {0};
    q7_t in_neg[1] = {-64};
    q7_t in_pos[1] = {64};
    q7_t w_one[1] = {127};
    q7_t out[1] = {0};

    /* 1. Exact 1.0x zero_point at acc == 0 for -128, -10, and +10 */
    syn_nn_quant_t q_zp_min = {.multiplier = 32768U, .shift = 0U, .zero_point = -128};
    syn_nn_quant_t q_zp_neg = {.multiplier = 32768U, .shift = 0U, .zero_point = -10};
    syn_nn_quant_t q_zp_pos = {.multiplier = 32768U, .shift = 0U, .zero_point = 10};

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_dense_quant_q7(in_zero, 1U, w_one, NULL, out, 1U,
                                                        SYN_NN_ACT_NONE, &q_zp_min));
    TEST_ASSERT_EQUAL_INT8(-128, out[0]);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_dense_quant_q7(in_zero, 1U, w_one, NULL, out, 1U,
                                                        SYN_NN_ACT_NONE, &q_zp_neg));
    TEST_ASSERT_EQUAL_INT8(-10, out[0]);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_dense_quant_q7(in_zero, 1U, w_one, NULL, out, 1U,
                                                        SYN_NN_ACT_NONE, &q_zp_pos));
    TEST_ASSERT_EQUAL_INT8(10, out[0]);

    /* 2. ReLU applies BEFORE zero_point: negative pre-activation clamps to 0 -> zero_point */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_dense_quant_q7(in_neg, 1U, w_one, NULL, out, 1U,
                                                        SYN_NN_ACT_RELU, &q_zp_neg));
    TEST_ASSERT_EQUAL_INT8(-10, out[0]);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_dense_quant_q7(in_neg, 1U, w_one, NULL, out, 1U,
                                                        SYN_NN_ACT_RELU, &q_zp_pos));
    TEST_ASSERT_EQUAL_INT8(10, out[0]);

    /* 3. Saturation on both bounds (+127 and -128) and NULL quant fallback */
    syn_nn_quant_t q_sat_hi = {.multiplier = 65535U, .shift = 0U, .zero_point = 120};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_dense_quant_q7(in_pos, 1U, w_one, NULL, out, 1U,
                                                        SYN_NN_ACT_NONE, &q_sat_hi));
    TEST_ASSERT_EQUAL_INT8(127, out[0]);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_dense_quant_q7(in_neg, 1U, w_one, NULL, out, 1U,
                                                        SYN_NN_ACT_NONE, &q_zp_min));
    TEST_ASSERT_EQUAL_INT8(-128, out[0]);

    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_nn_dense_quant_q7(in_pos, 1U, w_one, NULL, out, 1U, SYN_NN_ACT_NONE, NULL));

    /* 4. Large shift clamping: quant->shift = 48 and attn_shift = 32 */
    syn_nn_quant_t q_big_shift = {.multiplier = 32768U, .shift = 48U, .zero_point = -5};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_dense_quant_q7(in_pos, 1U, w_one, NULL, out, 1U,
                                                        SYN_NN_ACT_NONE, &q_big_shift));
    TEST_ASSERT_EQUAL_INT8(-5, out[0]);

    q7_t q_att[2] = {64, -64};
    q7_t k_att[2] = {64, -64};
    q7_t v_att[2] = {32, 16};
    q7_t out_att[2] = {0};
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_nn_attention_q7(q_att, k_att, v_att, 2U, 1U, 1U, out_att, 32U));
}

static void test_r5_nn_fixed_softmax_and_large_out_shift(void)
{
    /* 1. Single-class softmax hits scaled == 128 -> clamps to 127 */
    q7_t single_in[1] = {42};
    q7_t single_out[1] = {0};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_softmax_q7(single_in, single_out, 1U));
    TEST_ASSERT_EQUAL_INT8(127, single_out[0]);

    /* 2. Extreme logit spread [-128, 127] using pure fixed-point q16_exp */
    q7_t ext_in[2] = {-128, 127};
    q7_t ext_out[2] = {0, 0};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_softmax_q7(ext_in, ext_out, 2U));
    TEST_ASSERT_TRUE(ext_out[1] > ext_out[0]);
    TEST_ASSERT_TRUE(ext_out[0] > 0);

    /* 3. out_shift in [17, 31] and > 31 clamping in dense_q7 / conv1d_q7 */
    q7_t in_zero[1] = {0};
    q7_t w_zero[1] = {0};
    q16_t big_bias[1] = {(q16_t)(1UL << 26U)}; /* 1024.0 in Q16 */
    q7_t out[1] = {0};

    /* 2^26 >> 17 = 512 (Q16) -> 512 >> 9 = 1 (Q7); old code ignored shift > 16 and clamped 127 */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_nn_dense_q7(in_zero, 1U, w_zero, big_bias, out, 1U, SYN_NN_ACT_NONE, 17U));
    TEST_ASSERT_EQUAL_INT8(1, out[0]);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_nn_conv1d_q7(in_zero, 1U, 1U, w_zero, big_bias, out, 1U, 1U,
                                                   1U, SYN_NN_ACT_NONE, 17U));
    TEST_ASSERT_EQUAL_INT8(1, out[0]);

    /* out_shift = 32 clamps to 31 without shift UB */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_nn_dense_q7(in_zero, 1U, w_zero, big_bias, out, 1U, SYN_NN_ACT_NONE, 32U));
    TEST_ASSERT_EQUAL_INT8(0, out[0]);
}

typedef struct {
    SYN_PT child_pt;
    size_t progress;
    uint32_t dispatches;
    bool wait_gate;
    q7_t in_seq[4 * 2];
    q7_t conv_w[2 * 2 * 2];
    q16_t conv_b[2];
    q7_t conv_out[3 * 2];
    q7_t attn_out[3 * 2];
    q7_t dense_w[4 * 6];
    q16_t dense_b[4];
    q7_t dense_out[4];
} NnPipelineTaskCtx;

static SYN_PT_Status nn_idle_gate_child(SYN_PT *pt, const bool *gate)
{
    PT_BEGIN(pt);
    PT_WAIT_UNTIL(pt, *gate);
    PT_END(pt);
}

static SYN_PT_Status nn_pipeline_task(SYN_PT *pt, SYN_Task *task)
{
    NnPipelineTaskCtx *ctx = (NnPipelineTaskCtx *)task->user_data;
    ctx->dispatches++;

    PT_BEGIN(pt);

    PT_SPAWN_IF_NEEDED(pt, task, &ctx->child_pt,
                       nn_idle_gate_child(&ctx->child_pt, &ctx->wait_gate));

    PT_SPAWN_IF_NEEDED(pt, task, &ctx->child_pt,
                       syn_nn_conv1d_pt(&ctx->child_pt, ctx->in_seq, 4U, 2U, ctx->conv_w,
                                        ctx->conv_b, ctx->conv_out, 2U, 2U, 1U, SYN_NN_ACT_RELU, 0U,
                                        &ctx->progress, 1U));

    PT_SPAWN_IF_NEEDED(pt, task, &ctx->child_pt,
                       syn_nn_attention_pt(&ctx->child_pt, ctx->conv_out, ctx->conv_out,
                                           ctx->conv_out, 3U, 2U, 2U, ctx->attn_out, 0U,
                                           &ctx->progress, 1U));

    PT_SPAWN_IF_NEEDED(pt, task, &ctx->child_pt,
                       syn_nn_dense_pt(&ctx->child_pt, ctx->attn_out, 6U, ctx->dense_w,
                                       ctx->dense_b, ctx->dense_out, 4U, SYN_NN_ACT_NONE, 0U,
                                       &ctx->progress, 1U));

    PT_END(pt);
}

static SYN_PT_Status nn_contending_peer_task(SYN_PT *pt, SYN_Task *task)
{
    uint32_t *runs = (uint32_t *)task->user_data;
    (*runs)++;
    PT_BEGIN(pt);
    for (;;) {
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static void test_nn_attention_pt_and_sched_cooperative_pipeline(void)
{
    q7_t q[4 * 2] = {64, 16, -32, 48, 32, -16, 16, 64};
    q7_t k[4 * 2] = {48, 32, -16, 64, 64, -32, 32, 32};
    q7_t v[4 * 2] = {40, -20, 20, 60, -40, 30, 50, 10};
    q7_t ref_out[4 * 2] = {0};
    q7_t pt_out[4 * 2] = {0};
    SYN_PT pt;
    size_t cur = 0U;

    /* 1. NULL and invalid parameter checks for syn_nn_attention_pt */
    TEST_ASSERT_EQUAL(PT_EXITED,
                      syn_nn_attention_pt(NULL, q, k, v, 4U, 2U, 2U, pt_out, 0U, &cur, 1U));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_EXITED,
                      syn_nn_attention_pt(&pt, NULL, k, v, 4U, 2U, 2U, pt_out, 0U, &cur, 1U));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_EXITED,
                      syn_nn_attention_pt(&pt, q, k, v, 4U, 2U, 2U, pt_out, 0U, NULL, 1U));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_EXITED,
                      syn_nn_attention_pt(&pt, q, k, v, 33U, 2U, 2U, pt_out, 0U, &cur, 1U));

    /* 2. Standalone chunked execution matches syn_nn_attention_q7 bit-for-bit */
    TEST_ASSERT_EQUAL(SYN_OK, syn_nn_attention_q7(q, k, v, 4U, 2U, 2U, ref_out, 1U));
    PT_INIT(&pt);
    size_t yields = 0U;
    SYN_PT_Status st;
    do {
        st = syn_nn_attention_pt(&pt, q, k, v, 4U, 2U, 2U, pt_out, 1U, &cur, 1U);
        if (st == PT_MORE) {
            yields++;
        }
    } while (st == PT_MORE);
    TEST_ASSERT_EQUAL(PT_ENDED, st);
    TEST_ASSERT_EQUAL_UINT32(3U, (uint32_t)yields);
    TEST_ASSERT_EQUAL_INT8_ARRAY(ref_out, pt_out, 8);

    /* chunk_size == 0, chunk_size > rem, and shift > 31 clamping */
    memset(pt_out, 0, sizeof(pt_out));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_MORE, syn_nn_attention_pt(&pt, q, k, v, 4U, 2U, 2U, pt_out, 1U, &cur, 1U));
    TEST_ASSERT_EQUAL(PT_ENDED,
                      syn_nn_attention_pt(&pt, q, k, v, 4U, 2U, 2U, pt_out, 1U, &cur, SIZE_MAX));
    TEST_ASSERT_EQUAL_INT8_ARRAY(ref_out, pt_out, 8);
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_ENDED,
                      syn_nn_attention_pt(&pt, q, k, v, 4U, 2U, 2U, pt_out, 32U, &cur, 0U));
    TEST_ASSERT_EQUAL_UINT32(4U, (uint32_t)cur);

    /* 3. Scheduled Conv1D -> Attention -> Dense pipeline via PT_SPAWN_IF_NEEDED */
    NnPipelineTaskCtx ctx;
    memset(&ctx, 0, sizeof(ctx));
    for (size_t i = 0U; i < 8U; i++) {
        ctx.in_seq[i] = (q7_t)(16 + (int)i * 8);
        ctx.conv_w[i] = (q7_t)(32 - (int)i * 4);
    }
    for (size_t i = 0U; i < 24U; i++) {
        ctx.dense_w[i] = (q7_t)(24 - (int)(i % 7U) * 4);
    }

    /* Compute synchronous reference */
    q7_t ref_conv[6] = {0}, ref_attn[6] = {0}, ref_dense[4] = {0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_nn_conv1d_q7(ctx.in_seq, 4U, 2U, ctx.conv_w, ctx.conv_b, ref_conv,
                                               2U, 2U, 1U, SYN_NN_ACT_RELU, 0U));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_nn_attention_q7(ref_conv, ref_conv, ref_conv, 3U, 2U, 2U, ref_attn, 0U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_nn_dense_q7(ref_attn, 6U, ctx.dense_w, ctx.dense_b, ref_dense, 4U,
                                              SYN_NN_ACT_NONE, 0U));

    /* 3a. Uncontended: first yields PT_IDLE while wait_gate == false, then completes ALL 3 layers
     *     (3 conv steps + 3 attention tokens + 4 dense neurons) in 1 single scheduler dispatch! */
    SYN_Task single_task[1];
    SYN_Sched sched;
    syn_task_create(&single_task[0], "nn_pipe", nn_pipeline_task, SYN_PRIO_NORMAL, &ctx);
    syn_sched_init(&sched, single_task, 1U);

    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(1U, ctx.dispatches);
    TEST_ASSERT_EQUAL(SYN_TASK_READY, single_task[0].state);

    ctx.wait_gate = true;
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(2U, ctx.dispatches);
    TEST_ASSERT_EQUAL(SYN_TASK_DEAD, single_task[0].state);
    TEST_ASSERT_EQUAL_INT8_ARRAY(ref_dense, ctx.dense_out, 4);

    /* 3b. Contended: equal-priority PT_MORE peer forces PT_SPAWN_IF_NEEDED to yield after every
     *     single chunk across Conv1D (3), Attention (3), and Dense (4) = 8 mid-layer yields! */
    memset(ctx.conv_out, 0, sizeof(ctx.conv_out));
    memset(ctx.attn_out, 0, sizeof(ctx.attn_out));
    memset(ctx.dense_out, 0, sizeof(ctx.dense_out));
    ctx.dispatches = 0U;
    ctx.wait_gate = true;
    uint32_t peer_runs = 0U;

    SYN_Task pair_tasks[2];
    syn_task_create(&pair_tasks[0], "peer", nn_contending_peer_task, SYN_PRIO_NORMAL, &peer_runs);
    syn_task_create(&pair_tasks[1], "nn_pipe", nn_pipeline_task, SYN_PRIO_NORMAL, &ctx);
    syn_sched_init(&sched, pair_tasks, 2U);

    while (pair_tasks[1].state != SYN_TASK_DEAD) {
        syn_sched_run(&sched);
    }
    /* 2 leftoverconv chunks + 2 leftover attn chunks + 3 leftover dense chunks + 1 initial = 8 */
    TEST_ASSERT_EQUAL_UINT32(8U, ctx.dispatches);
    TEST_ASSERT_TRUE(peer_runs >= 8U);
    TEST_ASSERT_EQUAL_INT8_ARRAY(ref_dense, ctx.dense_out, 4);
}

void run_nn_tests(void)
{
    RUN_TEST(test_nn_activations);
    RUN_TEST(test_nn_softmax);
    RUN_TEST(test_nn_attention);
    RUN_TEST(test_nn_dct_transformer_pipeline);
    RUN_TEST(test_nn_protothread_coroutine);
    RUN_TEST(test_nn_conv1d_and_coroutine);
    RUN_TEST(test_nn_affine_quantization);
    RUN_TEST(test_nn_pooling_layers);
    RUN_TEST(test_nn_edge_cases_and_null_checks);
    RUN_TEST(test_nn_conv1d_coroutine);
    RUN_TEST(test_nn_conv1d_quant_q7_and_null_checks);
    RUN_TEST(test_nn_conv1d_quant_q7_param_validation_failures);
    RUN_TEST(test_nn_activation_functions_saturation);
    RUN_TEST(test_nn_conv1d_quant_q7_valid_conv_with_biases);
    RUN_TEST(test_nn_conv1d_quant_q7_invalid_params);
    RUN_TEST(test_nn_softmax_q7_null_and_basic);
    RUN_TEST(test_nn_quant_activations_and_avgpool_clamping);
    RUN_TEST(test_nn_pt_chunk_size_overflow_and_zero_resume);
    RUN_TEST(test_r4_nn_quant_zero_point_and_shift_clamp);
    RUN_TEST(test_r5_nn_fixed_softmax_and_large_out_shift);
    RUN_TEST(test_nn_attention_pt_and_sched_cooperative_pipeline);
}
