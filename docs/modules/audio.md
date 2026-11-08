# SyntropicOS Embedded Audio Subsystem (`src/syntropic/audio/`)

The SyntropicOS Audio Subsystem provides lightweight, zero-heap, integer-only audio codecs and PCM streaming engine for microcontrollers.

## Architecture

```text
+-----------------------+     +-----------------------+
|  IMA-ADPCM Codec      |     |  Bluetooth SBC Codec  |
|  (syn_adpcm.h/.c)     |     |  (syn_sbc.h/.c)       |
+-----------+-----------+     +-----------+-----------+
            |                             |
            +------------+  +-------------+
                         |  |
                         v  v
            +---------------------------+
            |  PCM Streaming Engine     |
            |  (syn_audio.h/.c)         |
            +-------------+-------------+
                          |
                          v
            +---------------------------+
            |  MCU I2S / SAI / DAC DMA  |
            +---------------------------+
```

## Features

### IMA-ADPCM Codec (`syn_adpcm`)
- **Compression**: 4:1 lossy compression (16-bit linear PCM <-> 4-bit ADPCM nibbles).
- **Footprint**: ~200 bytes code, ~8 bytes state (`SYN_ADPCM_State`).
- **Compatibility**: Standard WAV IMA-ADPCM format (ffmpeg, Audacity compatible).

### Bluetooth SBC Decoder (`syn_sbc`)
- **Specification**: Bluetooth SIG A2DP Sub-Band Codec spec compliant.
- **Subbands**: 4 and 8 subbands, 4/8/12/16 blocks.
- **Modes**: Mono, Dual Channel, Stereo, Joint Stereo.

### PCM Streaming Playback Engine (`syn_audio`)
- **Double-Buffering**: Ping-pong PCM buffer preventing audio underrun.
- **ISR Hooks**: Decoupled `syn_audio_isr_half()` and `syn_audio_isr_complete()` handlers.

### Voice Activity Detector (`syn_vad`)
- **Algorithms**: Multi-feature acoustic onset/voice detector:
  - Short-Time Energy (STE)
  - Zero-Crossing Rate (ZCR)
  - High-Frequency Differential Energy (HFDE)
- **Noise Tracking**: Adaptive background noise floor tracking via Exponential Moving Average (EMA).
- **Hysteresis**: Configurable attack frame threshold (`attack_frames`) and hangover release timing (`hangover_frames`).
- **Presets**: `SYN_VAD_SENSITIVITY_SENSITIVE`, `SYN_VAD_SENSITIVITY_NORMAL`, `SYN_VAD_SENSITIVITY_AGGRESSIVE`.

```c
#include <syntropic/audio/syn_vad.h>

static SYN_VAD vad;

void vad_setup(void) {
    SYN_VAD_Config cfg = {
        .sample_rate_hz = 16000U,
        .frame_length = 160U, /* 10 ms frame */
        .sensitivity = SYN_VAD_SENSITIVITY_NORMAL,
        .attack_frames = 2U,
        .hangover_frames = 10U,
    };
    syn_vad_init(&vad, &cfg);
}

void process_audio_frame(const int16_t *pcm_samples, size_t count) {
    SYN_VAD_State state = syn_vad_process_frame(&vad, pcm_samples, count, NULL);
    if (state == SYN_VAD_STATE_SPEECH) {
        /* Wake wake-word detector or stream to ASR / cloud */
    }
}
```
