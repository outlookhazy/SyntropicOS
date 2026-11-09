# DSP & Digital Signal Processing Modules

SyntropicOS provides fixed-point (integer-only) Digital Signal Processing (DSP) components designed for microcontrollers without Hardware Floating-Point Units (FPUs).

---

## Technical Specifications

| Feature | Specification |
|---|---|
| **Math Representation** | Q16.16 Fixed-Point (`q16_t`). 16 bits integer, 16 bits fractional. |
| **Accumulator Resolution** | 64-bit integer (`int64_t`) to prevent overflow during sum/square accumulation. |
| **Memory Allocation** | **100% Static / Zero Heap**. Buffers are caller-owned. |

---

## 1. Digital Filters (`dsp/syn_filter.h` & `dsp/syn_biquad.h`)

SyntropicOS includes Moving Average, Exponential Moving Average (EMA), Median spike rejection, Direct-Form FIR, and Butterworth Biquad IIR filters.

### Signal Processing Flow

```mermaid
flowchart LR
    RawADC["Raw ADC Sample"] --> MedianFilter["Median Filter (Spike Removal)"]
    MedianFilter --> EMAFilter["EMA / Biquad Filter (Noise Reduction)"]
    EMAFilter --> SignalStats["Signal Statistics (Min, Max, Mean, RMS)"]
```

### Complete Code Example (Biquad Butterworth Lowpass Filter)

```c
#include <syntropic/dsp/syn_biquad.h>
#include <syntropic/dsp/syn_filter.h>

static SYN_FilterBiquad lpf;
static SYN_FilterEMA    ema;

void dsp_init(void) {
    // 1. Initialize EMA Filter (alpha = 64/256 = 0.25)
    syn_filter_ema_init(&ema, 64);

    // 2. Initialize 2nd-order Butterworth Lowpass Filter: 100 Hz cutoff, 1000 Hz sample rate
    syn_filter_biquad_lowpass(&lpf, Q16_FROM_INT(100), Q16_FROM_INT(1000));
}

int16_t process_adc_sample(int16_t raw_sample) {
    // Convert sample to Q16.16 fixed-point format
    q16_t in_q16 = Q16_FROM_INT(raw_sample);

    // Filter through lowpass Biquad
    q16_t filtered_q16 = syn_filter_biquad_update(&lpf, in_q16);

    // Return integer result
    return (int16_t)Q16_TO_INT(filtered_q16);
}
```

---

## 2. Signal Statistics (`dsp/syn_signal.h`)

Provides real-time sliding window statistics (min, max, mean, variance, standard deviation, RMS) over a caller-owned circular buffer.

```c
#include <syntropic/dsp/syn_signal.h>

static int32_t stats_buffer[64];
static SYN_Signal sig;

void stats_init(void) {
    syn_signal_init(&sig, stats_buffer, 64);
}

void on_adc_sample(int32_t val) {
    syn_signal_push(&sig, val);

    int32_t min_val = syn_signal_min(&sig);
    int32_t max_val = syn_signal_max(&sig);
    int32_t mean_val = syn_signal_mean(&sig);
    int32_t rms_val  = syn_signal_rms_q16(&sig);
}
```

---

## 3. Fast Fourier Transform & Peak Detection (`dsp/syn_fft.h`)

Provides fixed-point Radix-2 FFT spectral analysis, Hanning/Hamming windowing, peak frequency identification, and Total Harmonic Distortion (THD) calculation.

```c
#include <syntropic/dsp/syn_fft.h>

#define FFT_SIZE 64

static q16_t real_buf[FFT_SIZE];
static q16_t imag_buf[FFT_SIZE];
static q16_t win_buf[FFT_SIZE];
static q16_t mag_buf[FFT_SIZE / 2 + 1];

void analyze_spectrum(void) {
    // Generate and apply Hanning window
    syn_fft_window_hanning(win_buf, FFT_SIZE);
    for (uint16_t i = 0; i < FFT_SIZE; i++) {
        real_buf[i] = q16_mul(real_buf[i], win_buf[i]);
    }

    // Perform in-place Radix-2 FFT
    syn_dsp_fft(real_buf, imag_buf, FFT_SIZE);

    // Compute magnitude spectrum (FFT_SIZE / 2 + 1 bins)
    syn_fft_magnitude_spectrum(real_buf, imag_buf, mag_buf, FFT_SIZE);

    // Find dominant frequency peak (at 1000 Hz sample rate)
    SYN_FFTPeak peak;
    uint8_t found = 0;
    syn_fft_find_peaks(mag_buf, FFT_SIZE / 2 + 1, Q16_FROM_INT(1000), &peak, 1, &found);
}
```

---

## 4. TinyML & Fixed-Point Neural Networks (`util/syn_nn.h`)

SyntropicOS features a zero-heap, fixed-point TinyML inference engine tailored for edge sensor processing (e.g., Human Activity Recognition, vibration fault detection, anomaly classification).

### Key Neural Network Features

- **Quantized 1D Convolution (`syn_nn_conv1d_quant_q7`)**: Multi-channel temporal feature extraction with TFLite-style affine quantization scaling.
- **Quantized Dense Layers (`syn_nn_dense_quant_q7`)**: Fully connected layers supporting arbitrary activation functions.
- **1D Max & Average Pooling (`syn_nn_maxpool1d_q7` / `syn_nn_avgpool1d_q7`)**: INT8 temporal downsampling.
- **Self-Attention Transformer Engine (`syn_nn_attention_q7`)**: Multi-head / single-head QKV dot-product self-attention mechanism.
- **Stackless Protothread Coroutines (`syn_nn_conv1d_pt` / `syn_nn_dense_pt`)**: Time-sliced inference that yields execution back to the RTOS event loop every `chunk_size` steps without stack memory overhead.

### Complete Code Example (Affine-Quantized 1D-CNN Inference)

```c
#include <syntropic/util/syn_nn.h>

// Scaled affine quantization parameters
static const syn_nn_quant_t layer1_quant = {
    .multiplier = 32768, // Fixed-point multiplier (Q15)
    .shift = 1,          // Bit-shift
    .zero_point = 0      // INT8 zero-point offset
};

void run_sensor_inference(const q7_t *sensor_data, q7_t *class_probabilities) {
    q7_t conv_out[16 * 32];
    q7_t pool_out[8 * 32];

    // 1. 1D Convolution: 16 time steps, 3 accelerometer channels -> 32 filters, 3x1 kernel
    syn_nn_conv1d_quant_q7(sensor_data, 16, 3, conv1_weights, conv1_biases,
                           conv_out, 32, 3, 1, SYN_NN_ACT_RELU, &layer1_quant);

    // 2. 1D Max Pooling: Pool size 2, Stride 2 (16 steps -> 8 steps)
    syn_nn_maxpool1d_q7(conv_out, 16, 32, pool_out, 2, 2);

    // 3. Dense Classifier Output
    syn_nn_dense_quant_q7(pool_out, 8 * 32, dense_weights, dense_biases,
                          class_probabilities, 6, SYN_NN_ACT_NONE, &layer1_quant);

    // 4. ArgMax Class Index Selection
    size_t predicted_activity = syn_nn_argmax_q7(class_probabilities, 6);
}
```

---

## 5. Waveform Synthesis, Tone Detection, Kalman Estimation & Audio Features

- **Runtime Biquad Filter Designer (`dsp/syn_filter_design.h`, requires `SYN_USE_BIQUAD` & `SYN_USE_QMATH`)**: Computes normalized Q16.16 second-order IIR coefficients (`SYN_BiquadCoeffs`) at runtime for lowpass, highpass, bandpass, and notch responses (`syn_filter_design_lowpass/highpass/bandpass/notch`, `syn_filter_design_apply_to_biquad`).
- **Direct Digital Synthesis (`dsp/syn_dds.h`, `SYN_USE_DDS`)**: 32-bit phase-accumulator waveform generator (`SYN_DDS`: `syn_dds_init`, `syn_dds_step`, `syn_dds_fill_q16`, `syn_dds_fill_u16`) producing sine, triangle, sawtooth, variable-duty square, and white-noise signals for DAC or test injection.
- **Goertzel Single-Bin Tone Detector (`dsp/syn_goertzel.h`, `SYN_USE_GOERTZEL`)**: `SYN_Goertzel` (`syn_goertzel_init`, `syn_goertzel_process_sample`, `syn_goertzel_process_block`, `syn_goertzel_get_magnitude_sq`) computes Q15 single-frequency `|X(k)|^2` power for DTMF and pilot-tone detection without full FFT cost.
- **Fixed-Point Linear Kalman Filter (`dsp/syn_kalman.h`, `SYN_USE_KALMAN`)**: Discrete-time Q16.16 state estimator (`SYN_Kalman`: `syn_kalman_init`, `syn_kalman_predict`, `syn_kalman_update`) operating on caller-allocated `SYN_Matrix` state/covariance workspaces (`SYN_KALMAN_SCRATCH_DECL`).
- **MFCC & DCT-II Spectral Features (`dsp/syn_mfcc.h`, `SYN_USE_MFCC`; `dsp/syn_dsp.h`)**: `syn_mfcc_process_frame()` extracts 13 quantized `q7_t` Mel-Frequency Cepstral Coefficients across 26 Mel filterbanks from 16-bit PCM frames; `syn_dsp_dct2_q7()` computes standalone `q7_t` Type-II Discrete Cosine Transforms.

