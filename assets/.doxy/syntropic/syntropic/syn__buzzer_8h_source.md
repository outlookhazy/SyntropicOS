

# File syn\_buzzer.h

[**File List**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_buzzer.h**](syn__buzzer_8h.md)

[Go to the documentation of this file](syn__buzzer_8h.md)


```C++

#ifndef SYN_BUZZER_H
#define SYN_BUZZER_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    SYN_GPIO_Pin pin; 
    bool active_high; 
    bool is_playing;  
    uint32_t freq_hz;     
    uint32_t duration_ms; 
    uint32_t elapsed_ms;  
    const uint16_t *pattern_freqs; 
    const uint16_t *pattern_durs;  
    size_t pattern_count;          
    size_t pattern_index;          
} SYN_Buzzer;

SYN_Status syn_buzzer_init(SYN_Buzzer *buz, SYN_GPIO_Pin pin, bool active_high);

SYN_Status syn_buzzer_beep(SYN_Buzzer *buz, uint32_t freq_hz, uint32_t duration_ms);

SYN_Status syn_buzzer_play_pattern(SYN_Buzzer *buz, const uint16_t *freqs, const uint16_t *durs,
                                   size_t count);

void syn_buzzer_stop(SYN_Buzzer *buz);

void syn_buzzer_step(SYN_Buzzer *buz, uint32_t dt_ms);

bool syn_buzzer_is_playing(const SYN_Buzzer *buz);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BUZZER_H */
```


