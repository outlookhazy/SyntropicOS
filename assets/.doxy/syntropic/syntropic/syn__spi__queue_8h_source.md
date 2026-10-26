

# File syn\_spi\_queue.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_spi\_queue.h**](syn__spi__queue_8h.md)

[Go to the documentation of this file](syn__spi__queue_8h.md)


```C++

#ifndef SYN_SPI_QUEUE_H
#define SYN_SPI_QUEUE_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"
#include "../port/syn_port_spi.h"
#include "syn_spi.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_SPI_QUEUE_MAX_DEPTH
#define SYN_SPI_QUEUE_MAX_DEPTH 16
#endif

typedef void (*SYN_SPI_Queue_Callback)(uint8_t bus, SYN_Status result, void *user_data);

typedef struct {
    uint8_t bus;                     
    SYN_GPIO_Pin cs_pin;             
    SYN_SPI_Mode mode;               
    uint32_t baudrate_hz;            
    bool keep_cs_active;             
    const uint8_t *tx_data;          
    uint8_t *rx_data;                
    size_t len;                      
    SYN_SPI_Queue_Callback callback; 
    void *user_data;                 
} SYN_SPI_Transaction;

typedef struct {
    SYN_SPI_Transaction ring[SYN_SPI_QUEUE_MAX_DEPTH]; 
    uint16_t head;                                     
    uint16_t tail;                                     
    uint16_t count;                                    
    uint8_t bus;                                       
    bool active;                                       
    bool initialized;                                  
} SYN_SPI_Queue;

SYN_Status syn_spi_queue_init(SYN_SPI_Queue *q, uint8_t bus);

SYN_Status syn_spi_queue_enqueue(SYN_SPI_Queue *q, const SYN_SPI_Transaction *tx);

SYN_Status syn_spi_queue_process(SYN_SPI_Queue *q);

SYN_Status syn_spi_queue_cancel_all(SYN_SPI_Queue *q);

size_t syn_spi_queue_count(const SYN_SPI_Queue *q);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SPI_QUEUE_H */
```


