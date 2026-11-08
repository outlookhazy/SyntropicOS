/**
 * @file    main.c
 * @brief   SyntropicOS Example — STM32 Blue Pill (STM32F103C8)
 *
 * Demonstrates key SyntropicOS features on the STM32F103 "Blue Pill" board
 * using the STM32 HAL (stm32cube) framework:
 *
 *   - Cooperative scheduler with protothread-based tasks
 *   - Serial CLI with custom commands over USART1 (PA9/PA10, 115200 baud)
 *   - Structured logging (SYN_LOG)
 *   - LED driver with blink, flash, and Morse pattern support (PC13)
 *   - Finite State Machine (FSM) module
 *
 * Hardware:
 *   - Board: STM32F103C8T6 "Blue Pill" (72 MHz, 64 KB Flash, 20 KB RAM)
 *   - LED:   PC13 (active-low, directly on the board)
 *   - UART:  USART1 — PA9 (TX), PA10 (RX) at 115200 8N1
 *
 * To build:
 *   pio run -d examples/stm32_serial
 */

#include "stm32f1xx_hal.h"
#include "syntropic/syntropic.h"
#include "syntropic/port/syn_port_uart.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/sched/syn_timer.h"
#include "syntropic/util/syn_fsm.h"
#include "syntropic/output/syn_led.h"
#include <string.h>
#include <stdlib.h>

#define TAG "main"

/*
 * SyntropicOS pin encoding: (port_index << 4) | pin_number
 * Port A = 0, Port B = 1, Port C = 2
 * PC13 = (2 << 4) | 13 = 45
 */
#define LED_PIN 45

/* STM32 HAL UART handle — shared with the SyntropicOS port layer */
UART_HandleTypeDef huart1;
extern UART_HandleTypeDef* syn_port_uart_handles[6];

/* ── Application objects ────────────────────────────────────────────────── */

static SYN_CLI   cli;
static SYN_Sched sched;
static SYN_Task  tasks[2];
static SYN_LED   status_led;

/* ── Finite State Machine ───────────────────────────────────────────────── */
/*
 * A simple three-state FSM to demonstrate SyntropicOS state management.
 *
 *   IDLE --[start]--> RUNNING --[stop]--> IDLE
 *                     RUNNING --[fault]--> FAULT --[stop]--> IDLE
 *
 * States are numbered: IDLE=0, RUNNING=1, FAULT=2
 * Events are numbered: start=0, stop=1, fault=2
 */

static SYN_FSM my_fsm;
static bool    fsm_initialized = false;

static const char *state_names[] = { "IDLE", "RUNNING", "FAULT" };

static void on_enter_running(void *ctx) { (void)ctx; SYN_LOG_I(TAG, "FSM -> RUNNING"); }
static void on_enter_fault(void *ctx)   { (void)ctx; SYN_LOG_W(TAG, "FSM -> FAULT"); }
static void on_enter_idle(void *ctx)    { (void)ctx; SYN_LOG_I(TAG, "FSM -> IDLE"); }

/* Transition table: { from_state, event, to_state, guard_fn, action_fn } */
static const SYN_FSM_Transition transitions[] = {
    { 0, 0, 1, NULL, on_enter_running },   /* IDLE    --start--> RUNNING */
    { 1, 1, 0, NULL, on_enter_idle },      /* RUNNING --stop-->  IDLE    */
    { 1, 2, 2, NULL, on_enter_fault },     /* RUNNING --fault--> FAULT   */
    { 2, 1, 0, NULL, on_enter_idle },      /* FAULT   --stop-->  IDLE    */
    SYN_FSM_END
};

/* ── CLI Commands ───────────────────────────────────────────────────────── */

/** CLI command: control the FSM — `fsm <start|stop|fault>` */
static int cmd_fsm(int argc, char *argv[])
{
    if (argc < 2) {
        syn_cli_printf(&cli, "Usage: fsm <start|stop|fault>\r\n");
        return 1;
    }

    /* Lazy-initialize the FSM on first use */
    if (!fsm_initialized) {
        syn_fsm_init(&my_fsm, transitions, 0, "demo_fsm");
        syn_fsm_set_state_names(&my_fsm, state_names);
        fsm_initialized = true;
    }

    /* Map command argument to event number */
    SYN_FSM_Event event = -1;
    if      (strcmp(argv[1], "start") == 0) event = 0;
    else if (strcmp(argv[1], "stop")  == 0) event = 1;
    else if (strcmp(argv[1], "fault") == 0) event = 2;
    else {
        syn_cli_printf(&cli, "Unknown event: %s\r\n", argv[1]);
        return 1;
    }

    bool ok = syn_fsm_dispatch(&my_fsm, event);
    syn_cli_printf(&cli, "FSM: event=%s result=%s state=%s\r\n",
                   argv[1], ok ? "OK" : "IGNORED",
                   state_names[syn_fsm_state(&my_fsm)]);
    return 0;
}

/** CLI command: control the LED — `led <on|off|blink|flash|sos>` */
static int cmd_led(int argc, char *argv[])
{
    if (argc < 2) {
        syn_cli_printf(&cli, "Usage: led <on|off|blink|flash|sos>\r\n");
        return 1;
    }

    if      (strcmp(argv[1], "on")    == 0) { syn_led_on(&status_led);                                 }
    else if (strcmp(argv[1], "off")   == 0) { syn_led_off(&status_led);                                }
    else if (strcmp(argv[1], "blink") == 0) { syn_led_blink(&status_led, 200, 200);                    }
    else if (strcmp(argv[1], "flash") == 0) { syn_led_flash(&status_led, 100, 100, 5);                 }
    else if (strcmp(argv[1], "sos")   == 0) { syn_led_pattern(&status_led, "... --- ... |", 150);      }
    else {
        syn_cli_printf(&cli, "Unknown mode: %s\r\n", argv[1]);
        return 1;
    }

    syn_cli_printf(&cli, "LED: %s\r\n", argv[1]);
    return 0;
}

/** Command table registered with the CLI */
static const SYN_CLI_Command commands[] = {
    { "led", "Control the status LED",        cmd_led },
    { "fsm", "Drive the demo state machine",  cmd_fsm },
};

/* ── Callbacks ──────────────────────────────────────────────────────────── */

/* CLI and log output goes directly through syn_port_serial_write.
 * No callback glue needed. */

/** Called when a SYN_ASSERT fails — spin forever (or trigger a watchdog) */
void syn_assert_failed(const char *file, int line)
{
    (void)file; (void)line;
    for (;;);
}

/* ── Tasks ──────────────────────────────────────────────────────────────── */

/**
 * Task 1: Heartbeat — updates the LED pattern and logs a periodic tick.
 *
 * This task runs every 50 ms to keep LED animations smooth.  Once per
 * second it also prints a timestamp so you can verify the scheduler is
 * running correctly.
 */
static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    static uint32_t last_print = 0;

    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&status_led);

        uint32_t now = syn_port_get_tick_ms();
        if (now - last_print >= 1000) {
            last_print = now;
            SYN_LOG_I(TAG, "tick: %lu", now);
        }

        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

static bool uart0_read_byte(uint8_t *ch)
{
    size_t received = 0;
    return (syn_port_uart_receive(0, ch, 1, &received, 0) == SYN_OK) && (received > 0);
}

/**
 * Task 2: Serial CLI — polls USART1 for incoming characters and feeds
 * them to the CLI line editor one byte at a time.
 *
 * Each call to syn_port_uart_receive attempts to read a single byte
 * non-blockingly (0 ms).  While bytes remain, the task yields only if
 * another task needs CPU time; once drained, it passes the turn.
 */
static SYN_PT_Status cli_task(SYN_PT *pt, SYN_Task *task)
{
    uint8_t ch;

    PT_BEGIN(pt);
    for (;;) {
        while (uart0_read_byte(&ch)) {
            syn_cli_process_char(&cli, (char)ch);
            PT_CONTINUE_IF_NEEDED(pt, task);
        }
        PT_PASS(pt);
    }
    PT_END(pt);
}

/* ── Application Setup ──────────────────────────────────────────────────── */

void setup(void)
{
    /* 1. Initialize the UART port layer (index 0 = USART1) */
    syn_port_uart_init(0, 115200);

    /* 2. Initialize the on-board LED (PC13, active-low on the Blue Pill) */
    syn_led_init(&status_led, LED_PIN, SYN_LED_ACTIVE_LOW);
    syn_led_blink(&status_led, 500, 500);  /* Start with a slow heartbeat */

    /* 3. Initialize structured logging over serial */
    syn_log_init(SYN_LOG_INFO);
    SYN_LOG_I(TAG, "boot");
    extern uint32_t SystemCoreClock;
    SYN_LOG_I(TAG, "clock: %lu", SystemCoreClock);

    /* 4. Initialize the interactive CLI */
    syn_cli_init(&cli, commands,
                 sizeof(commands) / sizeof(commands[0]),
                 "> ");
    syn_cli_set_scheduler(&sched);
    syn_cli_printf(&cli, "\r\n--- SyntropicOS (STM32) ---\r\n");
    syn_cli_print_prompt(&cli);

    /* 5. Create cooperative tasks and start the scheduler */
    syn_task_create(&tasks[0], "blink", blink_task, SYN_PRIO_LOW,  NULL);
    syn_task_create(&tasks[1], "cli",   cli_task,   SYN_PRIO_HIGH, NULL);
    syn_sched_init(&sched, tasks, 2);
}

/* ── STM32 HAL Boilerplate ──────────────────────────────────────────────── */
/*
 * Everything below is standard STM32CubeMX-generated initialization code.
 * It configures the system clock to 72 MHz (HSE + PLL) and sets up USART1
 * and the GPIO clocks.  This section is board-specific; the SyntropicOS
 * application logic above is portable across all supported platforms.
 */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

int main(void)
{
    HAL_Init();
    __enable_irq();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    /* Register the HAL UART handle with the SyntropicOS port layer */
    syn_port_uart_handles[0] = &huart1;

    setup();

    /* Run the cooperative scheduler forever */
    while (1) {
        syn_sched_run(&sched);
    }
}

/* HSE 8 MHz → PLL ×9 → SYSCLK 72 MHz */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    osc.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState        = RCC_HSE_ON;
    osc.HSEPredivValue  = RCC_HSE_PREDIV_DIV1;
    osc.HSIState        = RCC_HSI_ON;
    osc.PLL.PLLState    = RCC_PLL_ON;
    osc.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLMUL      = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&osc);

    RCC_ClkInitTypeDef clk = {0};
    clk.ClockType       = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                        | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource    = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider   = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider  = RCC_HCLK_DIV2;
    clk.APB2CLKDivider  = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2);
}

static void MX_USART1_UART_Init(void)
{
    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

/* UART MSP (MCU Support Package) — called internally by HAL_UART_Init */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio = {0};
    if (huart->Instance == USART1) {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* PA9 = USART1_TX (alternate-function push-pull) */
        gpio.Pin   = GPIO_PIN_9;
        gpio.Mode  = GPIO_MODE_AF_PP;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &gpio);

        /* PA10 = USART1_RX (input with pull-up) */
        gpio.Pin  = GPIO_PIN_10;
        gpio.Mode = GPIO_MODE_INPUT;
        gpio.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOA, &gpio);
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        __HAL_RCC_USART1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
    }
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}
