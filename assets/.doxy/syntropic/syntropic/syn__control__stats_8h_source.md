

# File syn\_control\_stats.h

[**File List**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_control\_stats.h**](syn__control__stats_8h.md)

[Go to the documentation of this file](syn__control__stats_8h.md)


```C++

#ifndef SYN_CONTROL_STATS_H
#define SYN_CONTROL_STATS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int64_t  sum_sq_err;      
    int64_t  sum_sq_out;      
    int64_t  sum_abs_delta;   
    int64_t  sum_itae;        
    int32_t  max_error;       
    int32_t  last_output;     
    uint32_t samples;         
} SYN_ControlStats;

typedef struct {
    int32_t  rms_error;       
    int32_t  max_error;       
    int32_t  control_effort;  
    int32_t  jitter;          
    int32_t  itae;            
} SYN_ControlReport;

void syn_control_stats_reset(SYN_ControlStats *stats);

void syn_control_stats_update(SYN_ControlStats *stats, int32_t error, int32_t output);

void syn_control_stats_report(const SYN_ControlStats *stats, SYN_ControlReport *report);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CONTROL_STATS_H */
```


