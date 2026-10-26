

# File syn\_usb\_host.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_host.h**](syn__usb__host_8h.md)

[Go to the documentation of this file](syn__usb__host_8h.md)


```C++

#ifndef SYN_USB_HOST_H
#define SYN_USB_HOST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef SYN_USB_HOST_MAX_CLASSES
#define SYN_USB_HOST_MAX_CLASSES 4U 
#endif

#ifndef SYN_USB_HOST_MAX_PIPES
#define SYN_USB_HOST_MAX_PIPES 4U 
#endif

#ifndef SYN_USB_HOST_ENUM_BUF_SIZE
#define SYN_USB_HOST_ENUM_BUF_SIZE 256U 
#endif

#define SYN_USB_HOST_STATE_DISCONNECTED 0U 
#define SYN_USB_HOST_STATE_ATTACHED 1U     
#define SYN_USB_HOST_STATE_RESET 2U        
#define SYN_USB_HOST_STATE_ENUMERATING 3U  
#define SYN_USB_HOST_STATE_READY 4U        
#define SYN_USB_HOST_STATE_ERROR 5U        
#define SYN_USB_HOST_ENUM_GET_DEV8 0U     
#define SYN_USB_HOST_ENUM_SET_ADDR 1U     
#define SYN_USB_HOST_ENUM_GET_DEV_FULL 2U 
#define SYN_USB_HOST_ENUM_GET_CFG 3U      
#define SYN_USB_HOST_ENUM_SET_CFG 4U      
#define SYN_USB_HOST_ENUM_CLASS_PROBE 5U  
#define SYN_USB_HOST_ENUM_DONE 6U         
typedef struct {
    uint8_t dev_addr;     
    uint16_t vid;         
    uint16_t pid;         
    uint8_t dev_class;    
    uint8_t dev_subclass; 
    uint8_t dev_protocol; 
    uint8_t max_pkt_ep0;  
    uint8_t num_configs;  
} SYN_USB_HostDevInfo;

typedef struct {
    uint8_t class_code;    
    uint8_t subclass_code; 
    uint8_t protocol_code; 
    void *ctx;             
    bool matched;          
    SYN_Status (*probe)(void *ctx, uint8_t dev_addr, const uint8_t *iface_desc, uint16_t len);

    void (*disconnected)(void *ctx);

    void (*process)(void *ctx);
} SYN_USB_HostClassDriver;

typedef struct {
    uint8_t state;       
    uint8_t enum_step;   
    uint8_t next_addr;   
    uint8_t class_count; 
    bool xfer_pending; 
    SYN_USB_HostDevInfo dev_info; 
    uint8_t enum_buf[SYN_USB_HOST_ENUM_BUF_SIZE]; 
    uint16_t enum_buf_len;                        
    SYN_USB_HostClassDriver classes[SYN_USB_HOST_MAX_CLASSES]; 
} SYN_USB_Host;

SYN_Status syn_usb_host_init(SYN_USB_Host *host);

SYN_Status syn_usb_host_register_class(SYN_USB_Host *host, const SYN_USB_HostClassDriver *cls);

SYN_Status syn_usb_host_process(SYN_USB_Host *host);

bool syn_usb_host_is_ready(const SYN_USB_Host *host);

const SYN_USB_HostDevInfo *syn_usb_host_get_dev_info(const SYN_USB_Host *host);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */
#include "syntropic/pt/syn_pt.h"

#define PT_USB_HOST_WAIT_READY(pt, host) PT_WAIT_UNTIL(pt, syn_usb_host_is_ready(host))

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_HOST_H */
```


