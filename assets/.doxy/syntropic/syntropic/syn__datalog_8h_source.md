

# File syn\_datalog.h

[**File List**](files.md) **>** [**log**](dir_1a216ccd2a39b426a7ff78fb20f852eb.md) **>** [**syn\_datalog.h**](syn__datalog_8h.md)

[Go to the documentation of this file](syn__datalog_8h.md)


```C++

#ifndef SYN_DATALOG_H
#define SYN_DATALOG_H

#include "../common/syn_defs.h"
#include "../util/syn_ringbuf.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t id;    
    uint16_t len;   
} SYN_DataLogHeader;

typedef struct {
    SYN_RingBuf rb;              
    uint32_t dropped_frames;     
} SYN_DataLog;

void syn_datalog_init(SYN_DataLog *log, uint8_t *buf, size_t size);

bool syn_datalog_write(SYN_DataLog *log, uint16_t id, const void *data, uint16_t len);

size_t syn_datalog_read(SYN_DataLog *log, uint16_t *out_id, void *out_data, size_t max_len);

static inline uint32_t syn_datalog_get_dropped(const SYN_DataLog *log) {
    return log->dropped_frames;
}

static inline void syn_datalog_reset(SYN_DataLog *log) {
    syn_ringbuf_reset(&log->rb);
    log->dropped_frames = 0;
}

static inline size_t syn_datalog_available(const SYN_DataLog *log) {
    return syn_ringbuf_count(&log->rb);
}

#ifdef __cplusplus
}
#endif
#endif // SYN_DATALOG_H
```


