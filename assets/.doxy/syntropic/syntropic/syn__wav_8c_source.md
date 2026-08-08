

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

SYN_Status syn_wav_parse_header(const uint8_t *buffer, size_t buffer_size, SYN_WAV_Info *info)
{
    if ((buffer == NULL) || (info == NULL) || (buffer_size < 44U)) {
        return SYN_INVALID_PARAM;
    }

    memset(info, 0, sizeof(SYN_WAV_Info));

    /* Check 'RIFF' magic header */
    if (memcmp(&buffer[0], "RIFF", 4) != 0) {
        return SYN_ERROR;
    }

    /* Check 'WAVE' format */
    if (memcmp(&buffer[8], "WAVE", 4) != 0) {
        return SYN_ERROR;
    }

    size_t offset = 12U;
    bool fmt_found = false;
    bool data_found = false;

    while (offset + 8U <= buffer_size) {
        const uint8_t *chunk = &buffer[offset];
        uint32_t chunk_size = peek_u32_le(&chunk[4]);

        if (memcmp(&chunk[0], "fmt ", 4) == 0) {
            if (offset + 8U + 16U > buffer_size) {
                return SYN_ERROR;
            }

            info->audio_format = peek_u16_le(&chunk[8]);
            info->num_channels = peek_u16_le(&chunk[10]);
            info->sample_rate = peek_u32_le(&chunk[12]);
            info->byte_rate = peek_u32_le(&chunk[16]);
            info->block_align = peek_u16_le(&chunk[20]);
            info->bits_per_sample = peek_u16_le(&chunk[22]);

            fmt_found = true;
        } else if (memcmp(&chunk[0], "data", 4) == 0) {
            info->data_offset = offset + 8U;
            info->data_size = chunk_size;
            data_found = true;
            break; /* Found data payload */
        }

        offset += 8U + chunk_size;
        /* Word align chunk padding */
        if ((chunk_size & 1U) != 0U) {
            offset++;
        }
    }

    if (!fmt_found || !data_found) {
        return SYN_ERROR;
    }

    /* Calculate total sample count */
    if (info->audio_format == SYN_WAV_FORMAT_PCM) {
        if ((info->num_channels > 0U) && (info->bits_per_sample > 0U)) {
            info->total_samples =
                info->data_size / (info->num_channels * (info->bits_per_sample / 8U));
        }
    } else if (info->audio_format == SYN_WAV_FORMAT_IMA_ADPCM) {
        /* 4 bits per sample mono = 2 samples per byte */
        info->total_samples = info->data_size * 2U;
    }

    return SYN_OK;
}
```


