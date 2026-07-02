

# File syn\_rtc.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_rtc.h**](syn__rtc_8h.md)

[Go to the documentation of this file](syn__rtc_8h.md)


```C++

#ifndef SYN_RTC_H
#define SYN_RTC_H

#include "../port/syn_port_rtc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* syn_port_rtc.h re-exports SYN_RTC_DateTime; no need to redefine. */

SYN_Status syn_rtc_init(void);

SYN_Status syn_rtc_get(SYN_RTC_DateTime *dt);

SYN_Status syn_rtc_set(const SYN_RTC_DateTime *dt);

bool syn_rtc_is_valid(const SYN_RTC_DateTime *dt);

uint32_t syn_rtc_to_epoch(const SYN_RTC_DateTime *dt);

void syn_rtc_from_epoch(uint32_t epoch, SYN_RTC_DateTime *dt);

#ifdef __cplusplus
}
#endif

#endif /* SYN_RTC_H */
```


