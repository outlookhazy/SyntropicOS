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
- **Double-Buffering**: Ping-pong PCM buffer (`SYN_Audio`, `syn_audio_init`, `syn_audio_start`, `syn_audio_feed`) preventing audio underrun.
- **ISR Hooks**: Decoupled `syn_audio_isr_half()` and `syn_audio_isr_complete()` handlers.

### Multi-Channel PCM Audio Mixer (`syn_audio_mixer`)
- **Polyphonic Mixing (`audio/syn_audio_mixer.h`, `SYN_USE_AUDIO`)**: Blends up to `SYN_AUDIO_MIXER_MAX_CHANNELS` (`4U` default) 16-bit PCM streams into a single output buffer (`syn_audio_mixer_render`) with per-channel (`syn_audio_mixer_play`, `syn_audio_mixer_set_channel_volume`) and master (`syn_audio_mixer_set_master_volume`) Q15 gain scaling (`SYN_AUDIO_GAIN_UNITY = 32768U`), looping, and saturating clamp.

### RIFF / WAVE Container Parser (`syn_wav`)
- **Zero-Copy Header Parsing (`audio/syn_wav.h`, `SYN_USE_AUDIO`)**: `syn_wav_parse_header()` inspects in-memory RIFF/WAVE containers (`SYN_WAV_Info`) to extract sample rate, channel count, bit depth, payload offset, and total sample count for linear PCM (`SYN_WAV_FORMAT_PCM`) and IMA-ADPCM (`SYN_WAV_FORMAT_IMA_ADPCM`) assets.

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
