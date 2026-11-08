/**
 * @file    main.c
 * @brief   SyntropicOS Example — Bare-Metal STM32 Button (No Protothreads / No Scheduler)
 *
 * Demonstrates bare-metal button handling with syn_button on STM32 (F1/F4/L4/etc.)
 * without requiring protothreads, tasks, or the cooperative scheduler.
 *
 * Hardware:
 *   - Board: STM32F103 Blue Pill / STM32 Nucleo
 *   - Button 1: PA0 (User key / active-high)
 *   - Button 2: PA1 (Active-low pullup)
 *   - LED:      PC13 (Status LED)
 */

#include "stm32f1xx_hal.h"
#include "syntropic/syntropic.h"
#include "syntropic/input/syn_button.h"
#include "syntropic/output/syn_led.h"

#include "port/stm32_hal/port_stm32_hal.h"

/* Pin encodings: SYN_PORT_STM32_PIN(gpio_port, gpio_pin_mask) */
#define PIN_BTN1 SYN_PORT_STM32_PIN(GPIOA, GPIO_PIN_0)  /* PA0 */
#define PIN_BTN2 SYN_PORT_STM32_PIN(GPIOA, GPIO_PIN_1)  /* PA1 */
#define PIN_LED  SYN_PORT_STM32_PIN(GPIOC, GPIO_PIN_13) /* PC13 */

static SYN_Button btn1;
static SYN_Button btn2;
static SYN_ButtonCombo combo;
static SYN_Button *combo_btns[2];
static SYN_LED status_led;

/* Button Callbacks */
static void on_single_click(SYN_Button *btn, void *ctx)
{
    (void)btn; (void)ctx;
    /* Toggle LED on single click */
    syn_led_toggle(&status_led);
}

static void on_double_click(SYN_Button *btn, void *ctx)
{
    (void)btn; (void)ctx;
    /* Blink fast on double click */
    syn_led_blink(&status_led, 100, 100);
}

static void on_long_press(SYN_Button *btn, void *ctx)
{
    (void)btn; (void)ctx;
    /* Steady ON for long press */
    syn_led_on(&status_led);
}

static void on_combo_press(SYN_Button *btn, void *ctx)
{
    (void)btn; (void)ctx;
    /* Flash LED SOS pattern when PA0 + PA1 are pressed together */
    syn_led_pattern(&status_led, "... --- ... |", 150);
}

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    /* 1. Initialize LED */
    syn_led_init(&status_led, PIN_LED, SYN_LED_ACTIVE_LOW);

    /* 2. Initialize Buttons (No Protothreads / No Scheduler needed!) */
    syn_button_init(&btn1, PIN_BTN1, SYN_BUTTON_ACTIVE_HIGH, 50);
    syn_button_init(&btn2, PIN_BTN2, SYN_BUTTON_ACTIVE_LOW, 50);

    /* Configure 250ms multi-click timing window */
    syn_button_set_click_window(&btn1, 250);

    /* Register callback handlers */
    syn_button_on_single_click(&btn1, on_single_click, NULL);
    syn_button_on_double_click(&btn1, on_double_click, NULL);
    syn_button_on_long_press(&btn1, on_long_press, 1000, NULL);

    /* Configure combination button (PA0 + PA1 pressed simultaneously) */
    combo_btns[0] = &btn1;
    combo_btns[1] = &btn2;
    syn_button_combo_init(&combo, combo_btns, 2, on_combo_press, NULL);

    /* 3. Pure Bare-Metal Main Loop */
    while (1)
    {
        /* Poll buttons every 10ms */
        syn_button_update(&btn1);
        syn_button_update(&btn2);
        syn_button_combo_update(&combo);

        /* Update LED animation */
        syn_led_update(&status_led);

        HAL_Delay(10);
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    osc.HSIState = RCC_HSI_ON;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&osc);

    RCC_ClkInitTypeDef clk = {0};
    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2);
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_13;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpio);
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}
