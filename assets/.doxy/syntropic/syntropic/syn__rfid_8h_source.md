

# File syn\_rfid.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_rfid.h**](syn__rfid_8h.md)

[Go to the documentation of this file](syn__rfid_8h.md)


```C++

#ifndef SYN_RFID_H
#define SYN_RFID_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_RFID_MFRC522 = 0, 
    SYN_RFID_PN532 = 1,   
    SYN_RFID_RDM6300 = 2  
} SYN_RFIDType;

typedef struct {
    SYN_RFIDType type;    
    SYN_GPIO_Pin ss_pin;  
    SYN_GPIO_Pin rst_pin; 
    uint8_t uid[10];      
    uint8_t uid_len;      
    bool card_present;    
} SYN_RFID;

SYN_Status syn_rfid_init(SYN_RFID *rfid, SYN_GPIO_Pin ss_pin, SYN_GPIO_Pin rst_pin,
                         SYN_RFIDType type);

void syn_rfid_feed_card(SYN_RFID *rfid, const uint8_t *uid, uint8_t len);

void syn_rfid_clear_card(SYN_RFID *rfid);

bool syn_rfid_is_card_present(const SYN_RFID *rfid);

const uint8_t *syn_rfid_get_uid(const SYN_RFID *rfid, uint8_t *len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_RFID_H */
```


