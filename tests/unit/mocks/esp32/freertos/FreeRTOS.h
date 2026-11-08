#ifndef MOCK_FREERTOS_H
#define MOCK_FREERTOS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef void *TaskHandle_t;
typedef uint32_t TickType_t;
typedef uint32_t portMUX_TYPE;

#define portMUX_INITIALIZER_UNLOCKED 0U
#define portENTER_CRITICAL(mux) ((void)(mux))
#define portEXIT_CRITICAL(mux) ((void)(mux))
#define portENTER_CRITICAL_SAFE(mux) ((void)(mux))
#define portEXIT_CRITICAL_SAFE(mux) ((void)(mux))

#define pdMS_TO_TICKS(ms) (ms)
#define portTICK_PERIOD_MS 1
#define pdTRUE 1
#define pdFALSE 0
#define BIT0 (1UL << 0)
#define tskIDLE_PRIORITY 0

static inline void vTaskDelay(TickType_t ticks)
{
    (void)ticks;
}
static inline void vTaskPrioritySet(TaskHandle_t t, uint32_t p)
{
    (void)t;
    (void)p;
}
static inline void taskYIELD(void)
{
}

#endif /* MOCK_FREERTOS_H */
