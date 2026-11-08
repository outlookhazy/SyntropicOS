#ifndef MOCK_ESP_ROM_SYS_H
#define MOCK_ESP_ROM_SYS_H

#include <stdint.h>

static inline void esp_rom_delay_us(uint32_t us)
{
    (void)us;
}

#endif /* MOCK_ESP_ROM_SYS_H */
