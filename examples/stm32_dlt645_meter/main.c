/**
 * @file main.c
 * @brief STM32 DL/T 645 Smart Electricity Meter Communication Example.
 *
 * Demonstrates non-blocking DL/T 645-2007 smart meter reading over RS-485
 * (2400 baud, 8E1) using single-byte UART interrupts and SyntropicOS protothreads.
 */

#include "syntropic/proto/syn_dlt645.h"
#include "syntropic/syntropic.h"
#include "stm32f4xx_hal.h"

#include <stdio.h>
#include <string.h>

#include "port/stm32_hal/port_stm32_hal.h"

#define RS485_DE_PIN SYN_PORT_STM32_PIN(GPIOB, GPIO_PIN_2) /* PB2 RS-485 DE Pin */

extern UART_HandleTypeDef huart1; /* Configured for 2400 Baud, 8 Data, Even Parity, 1 Stop */

/* ── Global State ───────────────────────────────────────────────────────── */

static SYN_UART meter_uart;
static uint8_t rx_byte;
static SYN_DLT645_Decoder decoder;
static uint32_t last_energy_kwh_x100 = 0; /* Active energy (0.01 kWh resolution) */

/* ── Meter Response Callback ────────────────────────────────────────────── */

/**
 * @brief Called automatically when a valid DL/T 645 frame is decoded.
 */
static void on_dlt645_frame_received(const SYN_DLT645_Frame *frame, void *ctx)
{
    (void)ctx;

    /* Verify Slave Read Data Response (0x91) */
    if (frame->control == SYN_DLT645_CMD_READ_DATA_RESP) {
        /* Verify Data ID: 0x00010000 = Combination Active Energy */
        if (frame->data_id == 0x00010000 && frame->payload_len >= 4) {
            /* Decode 4-byte BCD payload (0.01 kWh resolution) */
            uint8_t bcd3 = frame->payload[3];
            uint8_t bcd2 = frame->payload[2];
            uint8_t bcd1 = frame->payload[1];
            uint8_t bcd0 = frame->payload[0];

            uint32_t val = (uint32_t)((bcd3 >> 4) * 1000000 + (bcd3 & 0x0F) * 100000 +
                                      (bcd2 >> 4) * 10000 + (bcd2 & 0x0F) * 1000 +
                                      (bcd1 >> 4) * 100 + (bcd1 & 0x0F) * 10 +
                                      (bcd0 >> 4) * 1 + (bcd0 & 0x0F));

            last_energy_kwh_x100 = val;
            (void)last_energy_kwh_x100;

            printf("[DL/T 645] Meter %02X%02X%02X%02X%02X%02X Active Energy: %lu.%02lu kWh\n",
                   frame->address[5], frame->address[4], frame->address[3],
                   frame->address[2], frame->address[1], frame->address[0],
                   (unsigned long)(val / 100), (unsigned long)(val % 100));
        }
    } else if (frame->control == SYN_DLT645_CMD_ERROR_RESP) {
        printf("[DL/T 645] Error response from meter! ERR_CODE = 0x%02X\n",
               frame->payload_len > 0 ? frame->payload[0] : 0);
    }
}

/* ── UART RX Interrupt Callback ─────────────────────────────────────────── */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        /* Feed incoming byte into streaming decoder */
        syn_dlt645_decoder_feed(&decoder, rx_byte);

        /* Re-arm single-byte UART interrupt */
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}

void meter_uart_tx_isr(void)
{
    syn_uart_tx_isr_flush(&meter_uart);
}

/* ── Meter Request Polling Task ─────────────────────────────────────────── */

static SYN_PT_Status meter_poll_task(SYN_PT *pt, SYN_Task *task)
{
    static uint8_t tx_buf[64];
    static size_t tx_len;
    static SYN_DLT645_Frame req;

    PT_BEGIN(pt);

    for (;;) {
        /* Poll meter every 3000ms */
        PT_TASK_DELAY_MS(pt, task, 3000);

        /* Construct DL/T 645-2007 Read Active Energy Request */
        memset(&req, 0, sizeof(req));
        req.version = SYN_DLT645_VER_2007;
        req.control = SYN_DLT645_CMD_READ_DATA;
        req.data_id = 0x00010000; /* Total Active Energy Data ID */
        memset(req.address, 0x99, 6); /* Broadcast address */
        req.payload_len = 0;

        tx_len = syn_dlt645_encode(&req, tx_buf, sizeof(tx_buf));
        if (tx_len > 0) {
            /* Enable RS-485 Driver (TX mode) */
            syn_gpio_write(RS485_DE_PIN, SYN_GPIO_HIGH);

            /* Queue frame asynchronously and wait until shift register drains */
            PT_WAIT_UNTIL(pt, syn_uart_write_async(&meter_uart, tx_buf, tx_len) == SYN_OK);
            PT_WAIT_UNTIL(pt, syn_uart_tx_complete(&meter_uart));

            /* Disable RS-485 Driver (RX mode) */
            syn_gpio_write(RS485_DE_PIN, SYN_GPIO_LOW);
        }
    }

    PT_END(pt);
}

/* ── Main Application Entry Point ───────────────────────────────────────── */

int main(void)
{
    HAL_Init();
    /* MCU clock & USART1 (2400 baud, 8E1) initialization here */
    syn_port_stm32_register_uart(0, &huart1);
    syn_uart_init(&meter_uart, 0, 2400);

    /* Initialize DL/T 645 streaming decoder */
    syn_dlt645_decoder_init(&decoder, SYN_DLT645_VER_2007, on_dlt645_frame_received, NULL);

    /* Start initial UART RX interrupt */
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);

    /* Create scheduler tasks */
    static SYN_Task tasks[1];
    static SYN_Sched sched;

    syn_task_create(&tasks[0], "meter_poll", meter_poll_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&sched, tasks, 1);

    /* Run cooperative OS kernel */
    syn_sched_run_forever(&sched);
}
