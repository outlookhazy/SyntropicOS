

# File syn\_dnssd.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dnssd.h**](syn__dnssd_8h.md)

[Go to the documentation of this file](syn__dnssd_8h.md)


```C++

#ifndef SYN_DNSSD_H
#define SYN_DNSSD_H

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_DNSSD_MAX_SERVICES 4U
#define SYN_DNSSD_MAX_TXT_RECORDS 8U
#define SYN_DNSSD_PORT 5353U

typedef struct {
    const char *instance_name; 
    const char *service_type;  
    const char *hostname;      
    uint16_t port;             
    uint8_t ip[4];             
    const char *txt_records[SYN_DNSSD_MAX_TXT_RECORDS]; 
    size_t txt_count; 
} SYN_DnsSd_Service;

typedef struct {
    SYN_DnsSd_Service services[SYN_DNSSD_MAX_SERVICES]; 
    size_t service_count;                               
    SYN_Socket sock;                                    
} SYN_DnsSd;

typedef struct {
    char instance_name[64]; 
    char service_type[64];  
    char hostname[64];      
    uint16_t port;          
    uint8_t ip[4];          
    char txt[128];          
} SYN_DnsSd_Discovered;

SYN_Status syn_dnssd_init(SYN_DnsSd *sd);

SYN_Status syn_dnssd_register(SYN_DnsSd *sd, const SYN_DnsSd_Service *svc);

SYN_Status syn_dnssd_process_query(const SYN_DnsSd *sd, const uint8_t *query_buf, size_t query_len,
                                   uint8_t *resp_buf, size_t max_resp_len, size_t *resp_len);

SYN_Status syn_dnssd_announce(const SYN_DnsSd *sd, size_t service_index, uint8_t *resp_buf,
                              size_t max_resp_len, size_t *resp_len);

SYN_Status syn_dnssd_build_query(const char *service_type, uint8_t *query_buf, size_t max_len,
                                 size_t *query_len);

SYN_Status syn_dnssd_parse_response(const uint8_t *resp_buf, size_t resp_len,
                                    SYN_DnsSd_Discovered *out_disc);

SYN_Status syn_dnssd_discover(const SYN_DnsSd *sd, const char *service_type);

typedef void (*SYN_DnsSd_DiscoverCallback)(const SYN_DnsSd_Discovered *service, void *user_data);

typedef struct {
    const SYN_DnsSd *sd;                 
    const char *service_type;            
    SYN_DnsSd_DiscoverCallback callback; 
    void *user_data;                     
    uint32_t timeout_ms;                 
    uint32_t start_time;                 
    uint32_t discovered_count;           
    bool active;                         
} SYN_DnsSd_Browser;

SYN_Status syn_dnssd_browser_init(SYN_DnsSd_Browser *browser, const SYN_DnsSd *sd,
                                  const char *service_type, SYN_DnsSd_DiscoverCallback cb,
                                  void *user_data, uint32_t timeout_ms);

SYN_PT_Status syn_dnssd_browse_task(SYN_PT *pt, SYN_Task *task);

SYN_PT_Status syn_dnssd_task(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DNSSD_H */
```


