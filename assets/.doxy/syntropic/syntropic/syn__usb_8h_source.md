

# File syn\_usb.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb.h**](syn__usb_8h.md)

[Go to the documentation of this file](syn__usb_8h.md)


```C++

#ifndef SYN_USB_H
#define SYN_USB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef SYN_USB_MAX_CLASSES
#define SYN_USB_MAX_CLASSES 4U 
#endif

#ifndef SYN_USB_MAX_CONFIG_DESC
#define SYN_USB_MAX_CONFIG_DESC \
    256U 
#endif

#ifndef SYN_USB_MAX_STRING_DESCS
#define SYN_USB_MAX_STRING_DESCS 8U 
#endif

#define SYN_USB_EP0_BUF_SIZE 64U 
#define SYN_USB_STATE_DEFAULT 0U    
#define SYN_USB_STATE_ADDRESS 1U    
#define SYN_USB_STATE_CONFIGURED 2U 
#define SYN_USB_REQ_GET_STATUS 0x00U        
#define SYN_USB_REQ_CLEAR_FEATURE 0x01U     
#define SYN_USB_REQ_SET_FEATURE 0x03U       
#define SYN_USB_REQ_SET_ADDRESS 0x05U       
#define SYN_USB_REQ_GET_DESCRIPTOR 0x06U    
#define SYN_USB_REQ_SET_DESCRIPTOR 0x07U    
#define SYN_USB_REQ_GET_CONFIGURATION 0x08U 
#define SYN_USB_REQ_SET_CONFIGURATION 0x09U 
#define SYN_USB_DESC_TYPE_DEVICE 0x01U        
#define SYN_USB_DESC_TYPE_CONFIGURATION 0x02U 
#define SYN_USB_DESC_TYPE_STRING 0x03U        
#define SYN_USB_DESC_TYPE_INTERFACE 0x04U     
#define SYN_USB_DESC_TYPE_ENDPOINT 0x05U      
typedef struct {
    uint8_t bmRequestType; 
    uint8_t bRequest;      
    uint16_t wValue;       
    uint16_t wIndex;       
    uint16_t wLength;      
} SYN_USB_SetupPacket;

typedef struct {
    uint8_t iface_start; 
    uint8_t iface_count; 
    void *ctx;           
    SYN_Status (*setup)(void *ctx, const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                        uint16_t *rlen); 
    void (*data_out)(void *ctx, uint8_t ep, const uint8_t *data,
                     uint16_t len);                      
    void (*data_in)(void *ctx, uint8_t ep);              
    SYN_Status (*configured)(void *ctx, uint8_t config); 
} SYN_USB_ClassDriver;

typedef struct {
    uint8_t state;        
    uint8_t dev_address;  
    uint8_t config_value; 
    uint8_t class_count;  
    uint8_t ep0_buf[SYN_USB_EP0_BUF_SIZE]; 
    const uint8_t *device_desc; 
    const uint8_t *config_desc; 
    uint16_t config_desc_len;   
    bool raw_config_override;   
    const uint8_t *string_descs[SYN_USB_MAX_STRING_DESCS]; 
    uint8_t string_desc_count;                             
    uint8_t config_buf[SYN_USB_MAX_CONFIG_DESC]; 
    uint16_t config_buf_used;                    
    SYN_USB_ClassDriver classes[SYN_USB_MAX_CLASSES]; 
} SYN_USB_Device;

SYN_Status syn_usb_init(SYN_USB_Device *dev, const uint8_t *device_desc);

SYN_Status syn_usb_register_class(SYN_USB_Device *dev, const SYN_USB_ClassDriver *cls,
                                  const uint8_t *iface_desc, uint16_t iface_desc_len);

SYN_Status syn_usb_set_string_desc(SYN_USB_Device *dev, uint8_t index, const uint8_t *desc);

SYN_Status syn_usb_set_raw_config_desc(SYN_USB_Device *dev, const uint8_t *desc, uint16_t len);

SYN_Status syn_usb_process_setup(SYN_USB_Device *dev, const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                                 uint16_t *rlen);

bool syn_usb_is_configured(const SYN_USB_Device *dev);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */
#include "syntropic/pt/syn_pt.h"

#define PT_USB_WAIT_CONFIGURED(pt, dev) PT_WAIT_UNTIL(pt, syn_usb_is_configured(dev))

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_H */
```


