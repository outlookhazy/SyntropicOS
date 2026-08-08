

# File syn\_ymodem.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ymodem.h**](syn__ymodem_8h.md)

[Go to the documentation of this file](syn__ymodem_8h.md)


```C++

#ifndef SYN_YMODEM_H
#define SYN_YMODEM_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_YMODEM_SOH 0x01U 
#define SYN_YMODEM_STX 0x02U 
#define SYN_YMODEM_EOT 0x04U 
#define SYN_YMODEM_ACK 0x06U 
#define SYN_YMODEM_NAK 0x15U 
#define SYN_YMODEM_CAN 0x18U 
#define SYN_YMODEM_CRC 0x43U 
#ifndef SYN_YMODEM_MAX_FILENAME
#define SYN_YMODEM_MAX_FILENAME 64U 
#endif

#ifndef SYN_YMODEM_MAX_BLOCK_SIZE
#define SYN_YMODEM_MAX_BLOCK_SIZE 1024U 
#endif

#ifndef SYN_YMODEM_MAX_RETRIES
#define SYN_YMODEM_MAX_RETRIES 10U 
#endif
typedef enum {
    SYN_YMODEM_OK = 0,            
    SYN_YMODEM_ERR_TIMEOUT = -1,  
    SYN_YMODEM_ERR_CRC = -2,      
    SYN_YMODEM_ERR_CANCEL = -3,   
    SYN_YMODEM_ERR_SEQUENCE = -4, 
    SYN_YMODEM_ERR_OVERFLOW = -5, 
    SYN_YMODEM_ERR_PARAM = -6     
} SYN_YMODEM_Status;

typedef enum {
    SYN_YMODEM_EVENT_FILE_START = 0, 
    SYN_YMODEM_EVENT_DATA = 1,       
    SYN_YMODEM_EVENT_FILE_END = 2,   
    SYN_YMODEM_EVENT_SESSION_END = 3 
} SYN_YMODEM_Event;

typedef void (*SYN_YMODEM_PutChar)(uint8_t byte, void *ctx);

typedef int (*SYN_YMODEM_GetChar)(uint32_t timeout_ms, void *ctx);

typedef int (*SYN_YMODEM_EventCallback)(SYN_YMODEM_Event event, const uint8_t *data, size_t len,
                                        void *ctx);

typedef struct {
    SYN_YMODEM_PutChar putchar_fn;                   
    SYN_YMODEM_GetChar getchar_fn;                   
    SYN_YMODEM_EventCallback event_fn;               
    void *ctx;                                       
    uint8_t pkt_buf[SYN_YMODEM_MAX_BLOCK_SIZE + 5U]; 
    uint8_t expected_seq;                            
    char filename[SYN_YMODEM_MAX_FILENAME];          
    uint32_t filesize;                               
    uint32_t bytes_received;                         
} SYN_YMODEM_Receiver;

void syn_ymodem_receiver_init(SYN_YMODEM_Receiver *rx, SYN_YMODEM_PutChar putchar_fn,
                              SYN_YMODEM_GetChar getchar_fn, SYN_YMODEM_EventCallback event_fn,
                              void *ctx);

SYN_YMODEM_Status syn_ymodem_receive(SYN_YMODEM_Receiver *rx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_YMODEM_H */
```


