#ifndef MOCK_ESP_LOG_H
#define MOCK_ESP_LOG_H

#include "esp_rom_sys.h"

#include <stdint.h>

#define ESP_LOGI(tag, fmt, ...) (void)0
#define ESP_LOGE(tag, fmt, ...) (void)0
#define ESP_LOGW(tag, fmt, ...) (void)0

static inline int64_t esp_timer_get_time(void)
{
    return 0;
}

#endif /* MOCK_ESP_LOG_H */
