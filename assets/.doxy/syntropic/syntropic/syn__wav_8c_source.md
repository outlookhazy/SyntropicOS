

# File syn\_wav.c

[**File List**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_wav.c**](syn__wav_8c.md)

[Go to the documentation of this file](syn__wav_8c.md)


```C++


#include "syn_wav.h"

#include <string.h>

static inline uint16_t peek_u16_le(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static inline uint32_t peek_u32_le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static bool wav_parse_fmt_chunk(const uint8_t *chunk, uint32_t chunk_size, size_t rem_payload,
                                SYN_WAV_Info *info)
{
    if ((chunk_size < 16U) || (rem_payload < 16U)) {
        return false;
    }

    info->audio_format = peek_u16_le(&chunk[8]);
    info->num_channels = peek_u16_le(&chunk[10]);
    info->sample_rate = peek_u32_le(&chunk[12]);
    info->byte_rate = peek_u32_le(&chunk[16]);
    info->block_align = peek_u16_le(&chunk[20]);
    info->bits_per_sample = peek_u16_le(&chunk[22]);
    return true;
}

static bool wav_advance_chunk(size_t *offset, size_t buffer_size, uint32_t chunk_size,
                              size_t rem_payload)
{
    if ((uint64_t)chunk_size > (uint64_t)rem_payload) {
        return false;
    }

    *offset += 8U + (size_t)chunk_size;
    /* Word align chunk padding without wrapping or stepping past buffer_size */
    if ((chunk_size & 1U) != 0U) {
        if (*offset >= buffer_size) {
            return false;
        }
        (*offset)++;
    }
    return true;
}

static void wav_compute_total_samples(SYN_WAV_Info *info)
{
    if (info->audio_format == SYN_WAV_FORMAT_PCM) {
        uint32_t bytes_per_sample = info->bits_per_sample / 8U;
        if ((info->num_channels > 0U) && (bytes_per_sample > 0U)) {
            uint64_t frame_bytes = (uint64_t)info->num_channels * (uint64_t)bytes_per_sample;
            info->total_samples = (uint32_t)((uint64_t)info->data_size / frame_bytes);
        }
    } else if (info->audio_format == SYN_WAV_FORMAT_IMA_ADPCM) {
        /* 4 bits per sample = 2 samples per byte divided across channels; saturate at UINT32_MAX */
        if (info->num_channels > 0U) {
            uint64_t samples = ((uint64_t)info->data_size * 2ULL) / (uint64_t)info->num_channels;
            info->total_samples = (samples > (uint64_t)UINT32_MAX) ? UINT32_MAX : (uint32_t)samples;
        }
    }
}

SYN_Status syn_wav_parse_header(const uint8_t *buffer, size_t buffer_size, SYN_WAV_Info *info)
{
    if ((buffer == NULL) || (info == NULL) || (buffer_size < 44U)) {
        return SYN_INVALID_PARAM;
    }

    memset(info, 0, sizeof(SYN_WAV_Info));

    /* Check 'RIFF' magic header and 'WAVE' format */
    if ((memcmp(&buffer[0], "RIFF", 4) != 0) || (memcmp(&buffer[8], "WAVE", 4) != 0)) {
        return SYN_ERROR;
    }

    size_t offset = 12U;
    bool fmt_found = false;
    bool data_found = false;

    while ((buffer_size - offset) >= 8U) {
        const uint8_t *chunk = &buffer[offset];
        uint32_t chunk_size = peek_u32_le(&chunk[4]);
        size_t rem_payload = buffer_size - offset - 8U;

        if (memcmp(&chunk[0], "fmt ", 4) == 0) {
            if (!wav_parse_fmt_chunk(chunk, chunk_size, rem_payload, info)) {
                return SYN_ERROR;
            }
            fmt_found = true;
        } else if (memcmp(&chunk[0], "data", 4) == 0) {
            info->data_offset = offset + 8U;
            info->data_size = chunk_size;
            data_found = true;
            break; /* Found data payload */
        }

        if (!wav_advance_chunk(&offset, buffer_size, chunk_size, rem_payload)) {
            break;
        }
    }

    if (!fmt_found || !data_found) {
        return SYN_ERROR;
    }

    wav_compute_total_samples(info);
    return SYN_OK;
}
```


