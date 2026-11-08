/**
 * @file main.c
 * @brief STM32 HAL SIM800 GPRS Cellular Modem + Native SyntropicOS MQTT Example.
 *
 * Demonstrates non-blocking UART stream parsing via syn_at_parser to establish
 * a GPRS IP context on SIM800 and run SyntropicOS's native syn_mqtt client engine.
 */

#include "port/stm32_hal/port_stm32_hal.h"
#include "syntropic/log/syn_log.h"
#include "syntropic/net/syn_mqtt.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/syntropic.h"
#include "stm32f4xx_hal.h" /* Or stm32f1xx_hal.h / stm32g4xx_hal.h */

#include <stdio.h>
#include <string.h>

/* ── Hardware Instances ─────────────────────────────────────────────────── */
extern UART_HandleTypeDef huart2; /* Modem USART handle */

/* ── Buffers & Contexts ─────────────────────────────────────────────────── */
static SYN_UART modem_uart;
static uint8_t modem_rx_backing[256];
static SYN_Stream modem_rx_stream;

static char at_line_buf[128];
static SYN_AtParser at_parser;

static uint8_t mqtt_rx_buf[256];
static uint8_t mqtt_tx_buf[256];
static SYN_MqttClient mqtt_client;

/* SIM800 Connection State */
typedef enum {
    MODEM_STATE_POWER_ON,
    MODEM_STATE_CHECK_AT,
    MODEM_STATE_CHECK_SIM,
    MODEM_STATE_CHECK_CSQ,
    MODEM_STATE_ATTACH_GPRS,
    MODEM_STATE_SET_APN,
    MODEM_STATE_BRINGUP_WIRELESS,
    MODEM_STATE_GET_IP,
    MODEM_STATE_CONNECT_TCP,
    MODEM_STATE_READY,
    MODEM_STATE_FAILED,
} ModemState;

static ModemState g_modem_state = MODEM_STATE_POWER_ON;
static SYN_Socket g_sim800_sock = SYN_SOCKET_INVALID;

static SYN_Task tasks[2];
static SYN_Sched sched;

/* ── STM32 USART Interrupt Handler ──────────────────────────────────────── */

/**
 * @brief Ingest incoming byte from STM32 USART ISR into SyntropicOS Stream.
 */
void sim800_uart_rx_isr(uint8_t byte)
{
    syn_stream_put(&modem_rx_stream, byte);
}

/**
 * @brief Drain queued outgoing modem bytes from STM32 USART TXE ISR.
 */
void sim800_uart_tx_isr(void)
{
    syn_uart_tx_isr_flush(&modem_uart);
}

/* ── SIM800 Non-blocking Protothread Task ───────────────────────────────── */

typedef struct {
    ModemState state;
    const char *cmd;
    uint32_t timeout_ms;
    SYN_AtRespType expected;
    const char *line_prefix;
} ModemCmdStep;

static const ModemCmdStep k_modem_steps[] = {
    /* 1. Send initial AT command */
    { MODEM_STATE_CHECK_AT,         "AT\r\n",                2000U,  SYN_AT_RESP_OK,   NULL },
    /* 2. Verify SIM card insertion (AT+CPIN?) -> +CPIN: READY followed by OK */
    { MODEM_STATE_CHECK_SIM,        "AT+CPIN?\r\n",          3000U,  SYN_AT_RESP_LINE, "+CPIN: READY" },
    { MODEM_STATE_CHECK_SIM,        NULL,                    1000U,  SYN_AT_RESP_OK,   NULL },
    /* 3. Attach GPRS service (AT+CGATT=1) */
    { MODEM_STATE_ATTACH_GPRS,      "AT+CGATT=1\r\n",        5000U,  SYN_AT_RESP_OK,   NULL },
    /* 4. Configure APN (AT+CSTT="CMNET") */
    { MODEM_STATE_SET_APN,          "AT+CSTT=\"CMNET\"\r\n", 3000U,  SYN_AT_RESP_OK,   NULL },
    /* 5. Bring up wireless connection (AT+CIICR) */
    { MODEM_STATE_BRINGUP_WIRELESS, "AT+CIICR\r\n",          10000U, SYN_AT_RESP_OK,   NULL },
};

static bool modem_poll_step(SYN_Timeout *timer, SYN_AtRespType *out_resp)
{
    SYN_AtRespType r = syn_at_parser_feed_stream(&at_parser, &modem_rx_stream);
    *out_resp = r;
    if (r != SYN_AT_RESP_NONE && r != SYN_AT_RESP_URC) {
        return true;
    }
    return syn_timeout_expired(timer);
}

static bool modem_step_ok(const ModemCmdStep *step, SYN_AtRespType resp)
{
    if (resp != step->expected) {
        return false;
    }
    if (step->line_prefix != NULL) {
        const char *line = syn_at_parser_get_line(&at_parser);
        return strncmp(line, step->line_prefix, strlen(step->line_prefix)) == 0;
    }
    return true;
}

static SYN_PT_Status sim800_modem_task(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    (void)g_modem_state;
    (void)g_sim800_sock;
    static SYN_Timeout timer;
    static SYN_AtRespType resp;
    static size_t step_idx;
    static const ModemCmdStep *step;

    PT_BEGIN(pt);

    for (step_idx = 0; step_idx < SYN_ARRAY_SIZE(k_modem_steps); step_idx++) {
        step = &k_modem_steps[step_idx];
        g_modem_state = step->state;
        if (step->cmd != NULL) {
            PT_WAIT_UNTIL(pt, syn_uart_write_async(&modem_uart, (const uint8_t *)step->cmd,
                                                   strlen(step->cmd)) == SYN_OK);
            PT_WAIT_UNTIL(pt, syn_uart_tx_complete(&modem_uart));
        }
        syn_timeout_start(&timer, step->timeout_ms);
        PT_WAIT_UNTIL(pt, modem_poll_step(&timer, &resp));
        if (!modem_step_ok(step, resp)) {
            g_modem_state = MODEM_STATE_FAILED;
            PT_EXIT(pt);
        }
    }

    /* 6. Network connected! Set modem state to ready and resume MQTT task */
    g_modem_state = MODEM_STATE_READY;
    SYN_LOG_I("SIM800", "SIM800 GPRS Wireless Connection Established");
    syn_task_resume(&tasks[1]);

    PT_END(pt);
}

/* ── MQTT Callbacks ─────────────────────────────────────────────────────── */

static void on_mqtt_message(const char *topic, const uint8_t *payload, size_t len, void *ctx)
{
    (void)ctx;
    SYN_LOG_I("SIM800", "MQTT Rx [%s]: %.*s", topic, (int)len, (const char *)payload);
}

/* ── Application Main ───────────────────────────────────────────────────── */

int main(void)
{
    /* Initialize STM32 HAL Hardware Peripherals */
    HAL_Init();

    /* Initialize SyntropicOS UART, Stream & Parser */
    syn_port_stm32_register_uart(0, &huart2);
    syn_uart_init(&modem_uart, 0, 115200);
    syn_stream_init(&modem_rx_stream, modem_rx_backing, sizeof(modem_rx_backing));
    syn_at_parser_init(&at_parser, at_line_buf, sizeof(at_line_buf));

    /* Initialize SyntropicOS Native MQTT Client */
    syn_mqtt_init(&mqtt_client, "broker.hivemq.com", 1883, "stm32_sim800_client",
                  NULL, NULL, 60, mqtt_rx_buf, sizeof(mqtt_rx_buf),
                  mqtt_tx_buf, sizeof(mqtt_tx_buf));
    mqtt_client.on_message = on_mqtt_message;

    /* Create SyntropicOS Tasks & Scheduler */
    syn_task_create(&tasks[0], "sim800", sim800_modem_task, SYN_PRIO_NORMAL, NULL);
    syn_task_create(&tasks[1], "mqtt", syn_mqtt_task, SYN_PRIO_HIGH, &mqtt_client);
    syn_task_suspend(&tasks[1]);

    syn_sched_init(&sched, tasks, 2);

    /* Run Cooperative Scheduler Loop */
    for (;;) {
        syn_sched_run(&sched);
    }
}
