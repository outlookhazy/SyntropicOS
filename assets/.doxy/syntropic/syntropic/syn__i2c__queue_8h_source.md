

# File syn\_i2c\_queue.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_i2c\_queue.h**](syn__i2c__queue_8h.md)

[Go to the documentation of this file](syn__i2c__queue_8h.md)


```C++

#ifndef SYN_I2C_QUEUE_H
#define SYN_I2C_QUEUE_H

#include "../common/syn_defs.h"
#include "../port/syn_port_i2c.h"
#include "../port/syn_port_i2c_async.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_I2C_QUEUE_MAX_DEPTH
#define SYN_I2C_QUEUE_MAX_DEPTH 16
#endif

typedef void (*SYN_I2C_Queue_Callback)(uint8_t bus, SYN_Status result, void *user_data);

typedef struct {
    uint8_t bus;                     
    uint16_t addr;                   
    uint32_t clock_speed_hz;         
    const uint8_t *tx_data;          
    size_t tx_len;                   
    uint8_t *rx_data;                
    size_t rx_len;                   
    SYN_I2C_Queue_Callback callback; 
    void *user_data;                 
} SYN_I2C_Transaction;

typedef struct {
    SYN_I2C_Transaction ring[SYN_I2C_QUEUE_MAX_DEPTH]; 
    uint16_t head;                                     
    uint16_t tail;                                     
    uint16_t count;                                    
    uint8_t bus;                                       
    bool active;                                       
    bool initialized;                                  
} SYN_I2C_Queue;

SYN_Status syn_i2c_queue_init(SYN_I2C_Queue *q, uint8_t bus);

SYN_Status syn_i2c_queue_enqueue(SYN_I2C_Queue *q, const SYN_I2C_Transaction *tx);

SYN_Status syn_i2c_queue_process(SYN_I2C_Queue *q);

SYN_Status syn_i2c_queue_cancel_all(SYN_I2C_Queue *q);

size_t syn_i2c_queue_count(const SYN_I2C_Queue *q);

#ifdef __cplusplus
}
#endif

#endif /* SYN_I2C_QUEUE_H */
```


