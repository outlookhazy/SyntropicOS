

# File syn\_ntp\_server.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_ntp\_server.h**](syn__ntp__server_8h.md)

[Go to the documentation of this file](syn__ntp__server_8h.md)


```C++

#ifndef SYN_NTP_SERVER_H
#define SYN_NTP_SERVER_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/port/syn_port_socket.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_NTP_SERVER_PORT 123U          
#define SYN_NTP_PACKET_LEN 48U            
#define SYN_NTP_EPOCH_OFFSET 2208988800UL 
typedef struct {
    SYN_Socket sock;                      
    uint8_t stratum;                      
    uint32_t (*get_epoch_sec_cb)(void);   
    uint8_t req_buf[SYN_NTP_PACKET_LEN];  
    uint8_t resp_buf[SYN_NTP_PACKET_LEN]; 
} SYN_NTPServer;

SYN_Status syn_ntp_server_init(SYN_NTPServer *server, uint8_t stratum,
                               uint32_t (*get_epoch_sec_cb)(void));

SYN_Status syn_ntp_server_process_packet(SYN_NTPServer *server, const uint8_t request_pkt[48],
                                         uint8_t response_pkt[48]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_NTP_SERVER_H */
```


