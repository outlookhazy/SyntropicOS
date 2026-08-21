

# File syn\_port\_system.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_system.h**](syn__port__system_8h.md)

[Go to the documentation of this file](syn__port__system_8h.md)


```C++

#ifndef SYN_PORT_SYSTEM_H
#define SYN_PORT_SYSTEM_H

#include "../common/syn_compiler.h"
#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

void syn_port_enter_critical(void);

void syn_port_exit_critical(void);

uint32_t syn_port_get_tick_ms(void);

uint32_t syn_port_get_tick_us(void);

void syn_port_delay_ms(uint32_t ms);

void syn_port_nvic_set_priority(uint8_t irq_num, uint8_t preempt_prio, uint8_t sub_prio);

void syn_port_nvic_enable_irq(uint8_t irq_num);

#if defined(SYN_USE_TICKLESS) && SYN_USE_TICKLESS
void syn_port_sleep_until(uint32_t wake_tick_ms);
#endif

SYN_NORETURN void syn_port_system_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_SYSTEM_H */
```


