

# File syn\_event\_flags.h

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_event\_flags.h**](syn__event__flags_8h.md)

[Go to the documentation of this file](syn__event__flags_8h.md)


```C++

#ifndef SYN_EVENT_FLAGS_H
#define SYN_EVENT_FLAGS_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_EVENT_FLAGS_WAIT_ANY      = 0x01U, 
    SYN_EVENT_FLAGS_WAIT_ALL      = 0x02U, 
    SYN_EVENT_FLAGS_AUTO_CLEAR    = 0x04U  
} SYN_EventFlagsMode;

typedef struct {
    volatile uint32_t flags; 
} SYN_EventFlags;

SYN_Status syn_event_flags_init(SYN_EventFlags *ef);

SYN_Status syn_event_flags_set(SYN_EventFlags *ef, uint32_t flags_mask);

SYN_Status syn_event_flags_clear(SYN_EventFlags *ef, uint32_t flags_mask);

uint32_t syn_event_flags_get(const SYN_EventFlags *ef);

SYN_Status syn_event_flags_wait(SYN_EventFlags *ef, uint32_t wait_mask, uint32_t mode, uint32_t *out_flags);

#ifdef __cplusplus
}
#endif

#endif /* SYN_EVENT_FLAGS_H */
```


