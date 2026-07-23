# DSP & Filters

All DSP modules use integer-only math — no floating point.

## Digital Filters

| Module | Header | Config | Description |
|---|---|---|---|
| Filters | `dsp/syn_filter.h` | `SYN_USE_FILTER` | Moving average, exponential moving average (EMA), and median filters. Window size configurable up to `SYN_FILTER_MAX_WINDOW` (default 32). |
| Biquad | `dsp/syn_biquad.h` | `SYN_USE_BIQUAD` | Direct Form I biquad filter with built-in Butterworth design functions for lowpass, highpass, bandpass, and notch configurations. Uses Q16.16 coefficients with int64 accumulator. |

### Biquad Filter Design

Four design functions compute Butterworth coefficients from cutoff frequency and sample rate:

```c
SYN_FilterBiquad lpf, hpf, bpf, notch;

// Lowpass: passes below fc, attenuates above
syn_filter_biquad_lowpass(&lpf, Q16_FROM_INT(100), Q16_FROM_INT(1000));

// Highpass: passes above fc, attenuates below
syn_filter_biquad_highpass(&hpf, Q16_FROM_INT(100), Q16_FROM_INT(1000));

// Bandpass: passes band around fc, Q controls bandwidth
syn_filter_biquad_bandpass(&bpf, Q16_FROM_INT(100), Q16_FROM_INT(1000), Q16_FROM_INT(2));

// Notch: rejects band around fc, passes all else
syn_filter_biquad_notch(&notch, Q16_FROM_INT(60), Q16_FROM_INT(1000), Q16_FROM_INT(5));

// Process samples:
q16_t out = syn_filter_biquad_update(&lpf, sample);
```

## Signal Analysis

| Module | Header | Config | Description |
|---|---|---|---|
| Signal Stats | `dsp/syn_signal.h` | `SYN_USE_SIGNAL` | Sliding window statistics: min, max, mean, peak-to-peak, variance (Q16), standard deviation (Q16), RMS (Q16), delta, and latest value. |
| FFT | `dsp/syn_fft.h` | `SYN_USE_FFT` | Cooley-Tukey Radix-2 Fast Fourier Transform with cooperative protothread support for yielding mid-computation. |

### Signal Statistics

The signal module provides all common statistical functions over a caller-provided circular buffer:

```c
int32_t samples[64];
SYN_Signal sig;
syn_signal_init(&sig, samples, 64);

// Feed samples from sensor/ADC:
syn_signal_push(&sig, adc_read());

// Query:
int32_t min  = syn_signal_min(&sig);
int32_t rms  = syn_signal_rms_q16(&sig);       // RMS in Q16.16
int32_t sdev = syn_signal_std_dev_q16(&sig);    // Std deviation in Q16.16
```

## State Estimation

| Module | Header | Config | Description |
|---|---|---|---|
| Kalman Filter | `dsp/syn_kalman.h` | `SYN_USE_KALMAN` | General-purpose discrete-time linear Kalman filter using fixed-point matrix algebra. Supports arbitrary state and measurement dimensions (compile-time). Zero heap allocation. |

### Kalman Filter

Implements the standard predict/update Kalman loop using `syn_matrix` for all linear algebra. Caller allocates all matrices on the stack or statically — no heap.

```c
#define N_STATE 2   // [position, velocity]
#define N_MEAS  1   // [position only]

SYN_Kalman kf;
SYN_Kalman_Config cfg;

// All matrices are caller-owned:
SYN_MAT_DECL(x,  N_STATE, 1);        // State vector
SYN_MAT_DECL(P,  N_STATE, N_STATE);  // Error covariance
SYN_MAT_DECL(F,  N_STATE, N_STATE);  // State transition (constant-velocity model)
SYN_MAT_DECL(Q,  N_STATE, N_STATE);  // Process noise
SYN_MAT_DECL(H,  N_MEAS,  N_STATE);  // Measurement model
SYN_MAT_DECL(R,  N_MEAS,  N_MEAS);   // Measurement noise
SYN_MAT_DECL(z,  N_MEAS,  1);        // Measurement vector

// Scratch buffers (macro allocates all 8 needed):
SYN_KALMAN_SCRATCH_DECL(scratch, N_STATE, N_MEAS);

cfg.x = &x;  cfg.P = &P;  cfg.F = &F;
cfg.Q = &Q;  cfg.H = &H;  cfg.R = &R;
cfg.n_state = N_STATE;
cfg.n_meas  = N_MEAS;

// Set up constant-velocity model: F = [[1, dt], [0, 1]]
syn_matrix_identity(&F);
SYN_MAT_AT(&F, 0, 1) = Q16_FROM_FRAC(1, 100);  // dt = 10ms

// We only measure position: H = [[1, 0]]
syn_matrix_zero(&H);
SYN_MAT_AT(&H, 0, 0) = Q16_ONE;

syn_kalman_init(&kf, &cfg);
SYN_KALMAN_SCRATCH_ASSIGN(&kf, scratch);

// In control loop:
z.data[0] = sensor_position;
syn_kalman_predict(&kf);
syn_kalman_update(&kf, &z);

q16_t filtered_position = x.data[0];
q16_t estimated_velocity = x.data[1];
```

**Configuration limits**: Default maximum dimensions are 6 states × 4 measurements. Override via `SYN_KALMAN_MAX_STATE` and `SYN_KALMAN_MAX_MEAS` in `syn_config.h`.

## State Machines

| Module | Header | Config | Description |
|---|---|---|---|
| FSM | `util/syn_fsm.h` | `SYN_USE_FSM` | Table-driven finite state machine with entry/exit actions and event-driven transitions |
