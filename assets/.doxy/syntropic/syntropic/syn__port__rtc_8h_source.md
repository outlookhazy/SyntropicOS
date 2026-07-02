

# File syn\_port\_rtc.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_rtc.h**](syn__port__rtc_8h.md)

[Go to the documentation of this file](syn__port__rtc_8h.md)


```C++

#ifndef SYN_PORT_RTC_H
#define SYN_PORT_RTC_H

#include <stdint.h>
#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t year;    
    uint8_t  month;   
    uint8_t  day;     
    uint8_t  hour;    
    uint8_t  minute;  
    uint8_t  second;  
} SYN_RTC_DateTime;

SYN_Status syn_port_rtc_init(void);

SYN_Status syn_port_rtc_get(SYN_RTC_DateTime *dt);

SYN_Status syn_port_rtc_set(const SYN_RTC_DateTime *dt);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_RTC_H */
```


