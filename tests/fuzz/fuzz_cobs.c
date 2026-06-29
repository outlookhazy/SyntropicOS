#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "syntropic/proto/syn_cobs.h"

// Dummy callback for streaming decoder
static void cobs_callback(const uint8_t *data, size_t len, void *ctx) {
    (void)data;
    (void)len;
    (void)ctx;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0 || size > 4096) return 0;

    // 1. One-shot decode
    uint8_t out_buf[4096];
    syn_cobs_decode(data, size, out_buf);

    // 2. Streaming decode
    SYN_COBS_Decoder dec;
    uint8_t stream_buf[4096];
    syn_cobs_decoder_init(&dec, stream_buf, sizeof(stream_buf), cobs_callback, NULL);

    for (size_t i = 0; i < size; i++) {
        syn_cobs_decoder_feed(&dec, data[i]);
    }

    return 0;
}
