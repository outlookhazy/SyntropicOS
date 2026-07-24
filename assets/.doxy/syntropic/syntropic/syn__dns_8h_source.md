

# File syn\_dns.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dns.h**](syn__dns_8h.md)

[Go to the documentation of this file](syn__dns_8h.md)


```C++

#ifndef SYN_DNS_H
#define SYN_DNS_H

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* Inputs */
    const SYN_SockAddr *dns_server;      
    const char         *hostname;        
    SYN_SockAddr       *addr_out;        
    uint32_t            timeout_ms;      
    /* Output status */
    SYN_Status          status;          
    /* Internal State */
    SYN_Socket          sock;            
    SYN_SockAddr        server_addr;     
    uint32_t            start_ms;        
    size_t              query_len;       
    uint16_t            txid;            
    uint8_t             buf[256];        
} SYN_DnsResolver;

SYN_PT_Status syn_dns_resolve_task(SYN_PT *pt, SYN_Task *task);

typedef struct {
    const char *hostname;   
    uint8_t     ip[4];      
    SYN_Socket  sock;       
} SYN_Mdns;

SYN_Status syn_mdns_init(SYN_Mdns *mdns, const char *hostname, const uint8_t ip[4]);

SYN_PT_Status syn_mdns_task(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DNS_H */
```


