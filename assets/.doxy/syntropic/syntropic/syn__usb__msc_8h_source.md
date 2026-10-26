

# File syn\_usb\_msc.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_msc.h**](syn__usb__msc_8h.md)

[Go to the documentation of this file](syn__usb__msc_8h.md)


```C++

#ifndef SYN_USB_MSC_H
#define SYN_USB_MSC_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_USB_MSC_BLOCK_SIZE 512U     
#define SYN_USB_MSC_MAX_PACKET_SIZE 64U 
#define SYN_SCSI_TEST_UNIT_READY 0x00U  
#define SYN_SCSI_REQUEST_SENSE 0x03U    
#define SYN_SCSI_INQUIRY 0x12U          
#define SYN_SCSI_READ_CAPACITY_10 0x25U 
#define SYN_SCSI_READ_10 0x28U          
#define SYN_SCSI_WRITE_10 0x2AU         
typedef struct {
    uint32_t dCBWSignature;          
    uint32_t dCBWTag;                
    uint32_t dCBWDataTransferLength; 
    uint8_t bmCBWFlags;              
    uint8_t bCBWLUN;                 
    uint8_t bCBWCBLength;            
    uint8_t CBWCB[16];               
} SYN_USB_MSC_CBW;

typedef struct {
    uint32_t dCSWSignature;   
    uint32_t dCSWTag;         
    uint32_t dCSWDataResidue; 
    uint8_t bCSWStatus;       
} SYN_USB_MSC_CSW;

typedef struct {
    uint32_t block_count; 
    uint16_t block_size;  
    SYN_Status (*read_blocks)(uint32_t lba, uint8_t *buf,
                              uint16_t count); 
    SYN_Status (*write_blocks)(uint32_t lba, const uint8_t *buf,
                               uint16_t count); 
} SYN_USB_MSC_Media;

typedef struct {
    uint8_t ep_in;                               
    uint8_t ep_out;                              
    uint8_t iface_num;                           
    SYN_USB_MSC_Media media;                     
    SYN_USB_MSC_CBW cbw;                         
    SYN_USB_MSC_CSW csw;                         
    uint8_t tx_buf[SYN_USB_MSC_MAX_PACKET_SIZE]; 
    uint16_t tx_len;                             
} SYN_USB_MSC;

SYN_Status syn_usb_msc_init(SYN_USB_MSC *msc);

SYN_Status syn_usb_msc_register(SYN_USB_Device *dev, SYN_USB_MSC *msc);

SYN_Status syn_usb_msc_set_media(SYN_USB_MSC *msc, const SYN_USB_MSC_Media *media);

SYN_Status syn_usb_msc_process_cbw(SYN_USB_MSC *msc, const uint8_t cbw_raw[31]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_MSC_H */
```


