

# File syn\_biometric.h

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_biometric.h**](syn__biometric_8h.md)

[Go to the documentation of this file](syn__biometric_8h.md)


```C++

#ifndef SYN_BIOMETRIC_H
#define SYN_BIOMETRIC_H

#include "../common/syn_defs.h"
#include "../drivers/syn_soft_i2c.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_BIOMETRIC_MAX30102 = 0, 
    SYN_BIOMETRIC_MAX30100 = 1  
} SYN_BiometricType;

typedef struct {
    SYN_BiometricType type;  
    SYN_SoftI2C i2c;         
    uint8_t i2c_addr;        
    uint16_t heart_rate_bpm; 
    float spo2_pct;          
    bool finger_detected;    
} SYN_Biometric;

SYN_Status syn_biometric_init(SYN_Biometric *bio, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda,
                              uint8_t i2c_addr, SYN_BiometricType type);

void syn_biometric_feed_samples(SYN_Biometric *bio, uint32_t red_raw, uint32_t ir_raw);

uint16_t syn_biometric_get_bpm(const SYN_Biometric *bio);

float syn_biometric_get_spo2(const SYN_Biometric *bio);

bool syn_biometric_is_finger_detected(const SYN_Biometric *bio);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BIOMETRIC_H */
```


