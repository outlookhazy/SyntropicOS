

# File syn\_sntp.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_sntp.h**](syn__sntp_8h.md)

[Go to the documentation of this file](syn__sntp_8h.md)


```C++

#ifndef SYN_SNTP_H
#define SYN_SNTP_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SNTP) || SYN_USE_SNTP

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"
#include "../util/syn_backoff.h"
#include "../dsp/syn_filter.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants ──────────────────────────────────────────────────────────── */

#define SYN_SNTP_PACKET_SIZE    48

#define SYN_SNTP_EPOCH_OFFSET   2208988800UL

#ifndef SYN_SNTP_TIMEOUT_MS
#define SYN_SNTP_TIMEOUT_MS     3000
#endif

#ifndef SYN_SNTP_MAX_RETRIES
#define SYN_SNTP_MAX_RETRIES    3
#endif

/* ── Context ────────────────────────────────────────────────────────────── */

typedef struct {
    SYN_SockAddr server;            
    SYN_Socket   udp_sock;          
    uint32_t     epoch_s;           
    uint32_t     epoch_frac;        
    uint32_t     sync_tick_ms;      
    uint32_t     sync_interval_s;   
    bool         synced;            
    uint32_t     recv_deadline;     
    SYN_Backoff  backoff;           
    int32_t      drift_ppm;         
    SYN_FilterEMA drift_filter;     
    uint32_t     prev_sync_epoch;   
    uint32_t     prev_sync_tick_ms; 
} SYN_SNTP;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_sntp_init(SYN_SNTP *sntp, const SYN_SockAddr *server,
                   uint32_t sync_interval_s);

SYN_Status syn_sntp_query(SYN_SNTP *sntp);

SYN_PT_Status syn_sntp_task(SYN_PT *pt, SYN_Task *task);

uint32_t syn_sntp_get_epoch_s(const SYN_SNTP *sntp);

uint32_t syn_sntp_get_epoch_ns(const SYN_SNTP *sntp);

int32_t syn_sntp_get_drift_ppm(const SYN_SNTP *sntp);

static inline bool syn_sntp_is_synced(const SYN_SNTP *sntp)
{
    return sntp->synced;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_SNTP */

#endif /* SYN_SNTP_H */
```


