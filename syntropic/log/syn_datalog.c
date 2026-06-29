#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_DATALOG) || SYN_USE_DATALOG

#include "syn_datalog.h"
#include "../util/syn_assert.h"

void syn_datalog_init(SYN_DataLog *log, uint8_t *buf, size_t size) {
    SYN_ASSERT(log != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(size > sizeof(SYN_DataLogHeader));
    
    syn_ringbuf_init(&log->rb, buf, size);
    log->dropped_frames = 0;
}

bool syn_datalog_write(SYN_DataLog *log, uint16_t id, const void *data, uint16_t len) {
    SYN_ASSERT(log != NULL);
    SYN_ASSERT(data != NULL || len == 0);

    size_t required = sizeof(SYN_DataLogHeader) + len;
    
    // Check if we have enough space
    if (syn_ringbuf_free(&log->rb) < required) {
        log->dropped_frames++;
        return false; // Not enough space, drop the frame atomically
    }

    // Write header
    SYN_DataLogHeader header = { .id = id, .len = len };
    syn_ringbuf_write(&log->rb, (const uint8_t*)&header, sizeof(header));
    
    // Write payload
    if (len > 0) {
        syn_ringbuf_write(&log->rb, (const uint8_t*)data, len);
    }
    
    return true;
}

size_t syn_datalog_read(SYN_DataLog *log, uint16_t *out_id, void *out_data, size_t max_len) {
    SYN_ASSERT(log != NULL);
    SYN_ASSERT(out_id != NULL);
    SYN_ASSERT(out_data != NULL);

    if (syn_ringbuf_count(&log->rb) < sizeof(SYN_DataLogHeader)) {
        return 0; /* empty or incomplete header */
    }

    /*
     * Peek the header without consuming it. This lets us inspect the
     * payload length before committing to a read, so if max_len is too
     * small the frame is left intact in the buffer and the caller can
     * retry with a larger buffer.
     */
    SYN_DataLogHeader header;
    if (syn_ringbuf_peek_n(&log->rb, (uint8_t *)&header, sizeof(header))
            < sizeof(header)) {
        return 0; /* should not happen if count check above passed */
    }

    *out_id = header.id;

    if (header.len > max_len) {
        /*
         * Buffer too small — leave the frame in place so the caller can
         * retry. Do NOT increment dropped_frames; this is not a data loss
         * event, just a sizing mismatch.
         */
        return 0;
    }

    /* Now consume the header and payload together */
    syn_ringbuf_read(&log->rb, (uint8_t *)&header, sizeof(header));

    if (header.len > 0) {
        syn_ringbuf_read(&log->rb, (uint8_t *)out_data, header.len);
    }

    return header.len;
}



#endif /* SYN_USE_DATALOG */
