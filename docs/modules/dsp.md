# DSP & Filters

All DSP modules use integer-only math — no floating point.

## Digital Filters

| Module | Header | Config | Description |
|---|---|---|---|
| Filters | `dsp/syn_filter.h` | `SYN_USE_FILTER` | Moving average, exponential moving average (EMA), and median filters. Window size configurable up to `SYN_FILTER_MAX_WINDOW` (default 32). |
| Biquad | `dsp/syn_biquad.h` | `SYN_USE_BIQUAD` | Direct Form II Transposed biquad filter — suitable for low-pass, high-pass, band-pass, and notch configurations |

## Signal Analysis

| Module | Header | Config | Description |
|---|---|---|---|
| Signal Stats | `dsp/syn_signal.h` | `SYN_USE_SIGNAL` | Sliding window statistics: min, max, mean, variance (Q16 fixed-point), and delta. Can be attached to sensors for automatic statistical tracking. |
| FFT | `dsp/syn_fft.h` | `SYN_USE_FFT` | Cooley-Tukey Radix-2 Fast Fourier Transform |

## State Machines

| Module | Header | Config | Description |
|---|---|---|---|
| FSM | `util/syn_fsm.h` | `SYN_USE_FSM` | Table-driven finite state machine with entry/exit actions and event-driven transitions |
