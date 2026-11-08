/**
 * @file main.c
 * @brief SyntropicOS Integrated Multi-Task Protothreads (SYN_PT) STM32 HAL Example.
 *
 * Demonstrates 4 concurrent stackless protothreads (SYN_PT) executing cooperatively on
 * bare-metal STM32 hardware with zero stack overhead:
 *  1. Button Task: Debounces GPIO button input, detects Short Press & Long Press gestures.
 *  2. USART CLI Task: Non-blocking UART RX ring buffer processing & interactive CLI shell.
 *  3. Global Resource IPC Task: Central shared system state, message mailbox, event counters.
 *  4. LED Task: Dynamic status LED indicator reflecting global system state & events.
 */

#include "syntropic/syntropic.h"
#include "stm32f4xx_hal.h" /* Replace with target header (stm32f1xx_hal.h / stm32g4xx_hal.h) */

#include <stdio.h>
#include <string.h>

/* Hardware Pin Definitions */
#define LED_GPIO_PORT   GPIOA
#define LED_GPIO_PIN    GPIO_PIN_5

#define BUTTON_GPIO_PORT GPIOC
#define BUTTON_GPIO_PIN  GPIO_PIN_13

/* ── Global Resource IPC Structure ─────────────────────────────────────────── */

typedef enum {
    SYSTEM_MODE_IDLE = 0,
    SYSTEM_MODE_ACTIVE,
    SYSTEM_MODE_ALERT,
    SYSTEM_MODE_CONFIG
} App_SystemMode;

typedef enum {
    EVENT_NONE = 0,
    EVENT_BUTTON_SHORT_PRESS,
    EVENT_BUTTON_LONG_PRESS,
    EVENT_CLI_MODE_CHANGE,
    EVENT_ERROR_FLAGGED
} App_EventType;

typedef struct {
    App_SystemMode current_mode;
    uint32_t button_press_count;
    uint32_t cli_cmd_count;
    uint32_t active_errors;
    App_EventType pending_event;
    uint32_t last_event_tick;
} App_GlobalResources;

static App_GlobalResources g_resources = {
    .current_mode = SYSTEM_MODE_IDLE,
    .button_press_count = 0,
    .cli_cmd_count = 0,
    .active_errors = 0,
    .pending_event = EVENT_NONE,
    .last_event_tick = 0
};

/* ── Protothread Tasks Contexts ────────────────────────────────────────────── */

static SYN_PT pt_button;
static SYN_PT pt_usart;
static SYN_PT pt_ipc;
static SYN_PT pt_led;

/* USART Console & CLI Setup */
extern UART_HandleTypeDef huart2;
static SYN_CLI cli_ctx;

/* ── 1. Button Gesture Protothread ─────────────────────────────────────────── */

static SYN_PT_Status task_button_func(SYN_PT *pt)
{
    static uint32_t press_start_tick = 0;

    PT_BEGIN(pt);

    while (1) {
        /* Wait until user button PC13 is pressed (Active Low) */
        PT_WAIT_UNTIL(pt, HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_GPIO_PIN) == GPIO_PIN_RESET);

        press_start_tick = syn_port_get_tick_ms();

        /* Wait for button release or long press threshold (1000ms) */
        PT_WAIT_UNTIL(pt, (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_GPIO_PIN) == GPIO_PIN_SET) ||
                          ((syn_port_get_tick_ms() - press_start_tick) >= 1000U));

        uint32_t duration = syn_port_get_tick_ms() - press_start_tick;

        if (duration >= 1000U) {
            /* Long Press Detected */
            g_resources.pending_event = EVENT_BUTTON_LONG_PRESS;
            g_resources.button_press_count++;
            g_resources.last_event_tick = syn_port_get_tick_ms();

            /* Wait until button is completely released */
            PT_WAIT_UNTIL(pt, HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_GPIO_PIN) == GPIO_PIN_SET);
        } else if (duration >= 50U) {
            /* Short Press Detected (> 50ms debounce) */
            g_resources.pending_event = EVENT_BUTTON_SHORT_PRESS;
            g_resources.button_press_count++;
            g_resources.last_event_tick = syn_port_get_tick_ms();
        }

        /* Pass turn until next round */
        PT_PASS(pt);
    }

    PT_END(pt);
}

/* ── 2. USART CLI Shell Protothread ────────────────────────────────────────── */

static int cmd_status(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    uint32_t now = syn_port_get_tick_ms();
    const char *mode_str[] = {"IDLE", "ACTIVE", "ALERT", "CONFIG"};

    syn_cli_printf(&cli_ctx, "\r\n=== SyntropicOS Multi-Task Telemetry ===\r\n");
    syn_cli_printf(&cli_ctx, "Uptime: %lu ms\r\n", now);
    syn_cli_printf(&cli_ctx, "System Mode: %s\r\n", mode_str[g_resources.current_mode]);
    syn_cli_printf(&cli_ctx, "Button Presses: %lu\r\n", g_resources.button_press_count);
    syn_cli_printf(&cli_ctx, "CLI Commands: %lu\r\n", g_resources.cli_cmd_count);
    syn_cli_printf(&cli_ctx, "Active Errors: %lu\r\n", g_resources.active_errors);
    syn_cli_printf(&cli_ctx, "========================================\r\n\r\n");
    return 0;
}

static int cmd_mode(int argc, char *argv[])
{
    if (argc < 2) {
        syn_cli_printf(&cli_ctx, "Usage: mode <idle|active|alert|config>\r\n");
        return 1;
    }

    if (strcmp(argv[1], "idle") == 0) {
        g_resources.current_mode = SYSTEM_MODE_IDLE;
    } else if (strcmp(argv[1], "active") == 0) {
        g_resources.current_mode = SYSTEM_MODE_ACTIVE;
    } else if (strcmp(argv[1], "alert") == 0) {
        g_resources.current_mode = SYSTEM_MODE_ALERT;
    } else if (strcmp(argv[1], "config") == 0) {
        g_resources.current_mode = SYSTEM_MODE_CONFIG;
    } else {
        syn_cli_printf(&cli_ctx, "Unknown mode: %s\r\n", argv[1]);
        return 1;
    }

    g_resources.cli_cmd_count++;
    g_resources.pending_event = EVENT_CLI_MODE_CHANGE;
    syn_cli_printf(&cli_ctx, "System mode updated to %s\r\n", argv[1]);
    return 0;
}

static const SYN_CLI_Command cli_commands[] = {
    {"status", "status - Print multi-task system state telemetry", cmd_status},
    {"mode",   "mode <idle|active|alert|config> - Set global system mode", cmd_mode},
};

static SYN_PT_Status task_usart_func(SYN_PT *pt)
{
    PT_BEGIN(pt);

    syn_cli_init(&cli_ctx, cli_commands, sizeof(cli_commands) / sizeof(cli_commands[0]), "> ");

    while (1) {
        /* Process UART input buffer characters */
        uint8_t c;
        if (syn_port_serial_read(&c, 1) == 1) {
            syn_cli_process_char(&cli_ctx, (char)c);
            PT_CONTINUE(pt);
        } else {
            PT_PASS(pt);
        }
    }

    PT_END(pt);
}

/* ── 3. Global Resource IPC Coordinator Protothread ───────────────────────── */

static SYN_PT_Status task_ipc_func(SYN_PT *pt)
{
    PT_BEGIN(pt);

    while (1) {
        /* Check if any event has been posted to global resources */
        if (g_resources.pending_event != EVENT_NONE) {
            if (g_resources.pending_event == EVENT_BUTTON_SHORT_PRESS) {
                /* Cycle system mode on short button press */
                g_resources.current_mode = (App_SystemMode)((g_resources.current_mode + 1) % 4);
                syn_cli_printf(&cli_ctx, "[IPC Event] Button Short Press -> Mode: %d\r\n", g_resources.current_mode);
            } else if (g_resources.pending_event == EVENT_BUTTON_LONG_PRESS) {
                /* Toggle Alert mode on long button press */
                g_resources.current_mode = SYSTEM_MODE_ALERT;
                syn_cli_printf(&cli_ctx, "[IPC Event] Button Long Press -> ALERT Mode\r\n");
            } else if (g_resources.pending_event == EVENT_CLI_MODE_CHANGE) {
                syn_cli_printf(&cli_ctx, "[IPC Event] CLI Mode Change Processed\r\n");
            }

            /* Clear processed event flag */
            g_resources.pending_event = EVENT_NONE;
        }

        PT_PASS(pt);
    }

    PT_END(pt);
}

/* ── 4. LED Indicator Protothread ──────────────────────────────────────────── */

static SYN_PT_Status task_led_func(SYN_PT *pt)
{
    static uint32_t last_toggle_tick = 0;

    PT_BEGIN(pt);

    while (1) {
        uint32_t interval_ms = 1000U; /* Default IDLE blink (1Hz) */

        if (g_resources.current_mode == SYSTEM_MODE_ACTIVE) {
            interval_ms = 250U;  /* Fast blink */
        } else if (g_resources.current_mode == SYSTEM_MODE_ALERT) {
            interval_ms = 100U;  /* Rapid flash */
        } else if (g_resources.current_mode == SYSTEM_MODE_CONFIG) {
            HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET); /* Solid ON */
            PT_PASS(pt);
            continue;
        } else {
            interval_ms = 1000U; /* Slow blink */
        }

        if ((syn_port_get_tick_ms() - last_toggle_tick) >= interval_ms) {
            last_toggle_tick = syn_port_get_tick_ms();
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        }

        PT_PASS(pt);
    }

    PT_END(pt);
}

/* ── Main Entry Point ──────────────────────────────────────────────────────── */

int main(void)
{
    /* Initialize STM32 HAL, Clocks, GPIO, and USART peripherals */
    HAL_Init();

    /* Initialize Protothread Context Blocks (costs 2 bytes RAM each) */
    PT_INIT(&pt_button);
    PT_INIT(&pt_usart);
    PT_INIT(&pt_ipc);
    PT_INIT(&pt_led);

    /* ── Option A: Bare-Metal Cooperative Super-Loop (Used Here) ────────────────
     * In this standalone mode, the super-loop polls each protothread round-robin.
     * Each task returns PT_MORE or PT_IDLE immediately when yielding, allowing
     * CPU cycles to pass to the next task in nanoseconds (~3-5 cycles context switch).
     */
    while (1) {
        task_button_func(&pt_button);
        task_usart_func(&pt_usart);
        task_ipc_func(&pt_ipc);
        task_led_func(&pt_led);
    }

    /* ── Option B: SyntropicOS Kernel Scheduler (syn_sched) Alternative ───────
     * If using the full SyntropicOS kernel scheduler (syn_sched.h), manual loop
     * polling is replaced by priority-driven scheduling and event blocking:
     *
     *   static SYN_Task tasks[4];
     *   static SYN_Sched sched;
     *
     *   syn_task_create(&tasks[0], "button", task_button_adapter, SYN_PRIO_HIGH,     NULL);
     *   syn_task_create(&tasks[1], "usart",  task_usart_adapter,  SYN_PRIO_HIGH,     NULL);
     *   syn_task_create(&tasks[2], "ipc",    task_ipc_adapter,    SYN_PRIO_CRITICAL, NULL);
     *   syn_task_create(&tasks[3], "led",    task_led_adapter,    SYN_PRIO_LOW,      NULL);
     *
     *   syn_sched_init(&sched, tasks, 4);
     *   syn_sched_run_forever(&sched);
     *
     * Key Scheduler Operational Differences:
     *  1. Priority & Starvation Bounds: Higher priority work runs first, bounded by max_wait_ms.
     *  2. Event-Driven Sleeping: Tasks call PT_BLOCK_EVENT() or PT_TASK_DELAY_MS(),
     *     marking their state SYN_TASK_BLOCKED or setting a wakeup deadline so the kernel skips polling them.
     *  3. Tickless Low-Power Sleep: When all tasks are delayed/blocked, syn_sched
     *     automatically invokes MCU WFI (Wait For Interrupt) to save power.
     */

    return 0;
}
