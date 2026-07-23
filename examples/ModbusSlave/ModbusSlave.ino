/**
 * @file ModbusSlave.ino
 * @brief SyntropicOS Non-Blocking Modbus RTU Slave Example
 *
 * Demonstrates:
 *   - Modbus RTU Slave (RFC standard framing & CRC-16 validation)
 *   - FC03 (Read Holding Registers), FC04 (Read Input Registers), FC06 (Write Single), FC16 (Write Multiple)
 *   - Non-blocking inter-character 3.5-char silence detection
 *   - Concurrent execution with scheduler tasks (status LED)
 *
 * How to Test:
 *   1. Connect Modbus Master (e.g., Modbus Poll / QModMaster) over Serial at 115200 8N1
 *   2. Set Slave ID = 1
 *   3. Read Holding Regs (FC03, Addr 0..1 -> 0x1234, 0x5678) or Input Regs (FC04, Addr 0..1)
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/proto/syn_modbus.h>

#if defined(ARDUINO_ARCH_AVR)
#include <avr/interrupt.h>
ISR(BADISR_vect) {
    /* Safe fallback */
}
#endif

/* ── Modbus Register Maps ─────────────────────────────────────────────────── */

#define HOLDING_REG_COUNT 16
#define INPUT_REG_COUNT   16

static uint16_t holding_regs[HOLDING_REG_COUNT];
static uint16_t input_regs[INPUT_REG_COUNT];
static uint8_t  modbus_buf[256];

static SYN_Modbus        modbus_slave;
static SYN_Modbus_Config modbus_cfg;

static SYN_Sched sched;
static SYN_Task  tasks[2];
static SYN_LED   status_led;

#define STATUS_LED_PIN 7  /* Pin 7 status LED */

/* ── Protothread Tasks ─────────────────────────────────────────────────── */

/** Task 1: Blink Status LED */
static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&status_led);
        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

/** Task 2: Modbus Polling & Input Register Update Task */
static SYN_PT_Status modbus_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        /* Update live input registers (simulated sensor value) */
        input_regs[0] = (uint16_t)(millis() & 0xFFFF);
        input_regs[1] = 0x4242;

        /* Poll Modbus RTU slave for incoming frames */
        syn_modbus_poll(&modbus_slave);

        PT_DEFER(pt, task);
    }
    PT_END(pt);
}

/* ── Setup & Main Loop ─────────────────────────────────────────────────── */

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 2000);

    /* Initialize register default values */
    holding_regs[0] = 0x1234;
    holding_regs[1] = 0x5678;
    holding_regs[2] = 0x0000;
    input_regs[0]   = 0x0100;
    input_regs[1]   = 0x0200;

    /* Initialize Modbus RTU Slave Configuration */
    memset(&modbus_cfg, 0, sizeof(modbus_cfg));
    modbus_cfg.slave_addr    = 1;          /* Slave ID = 1 */
    modbus_cfg.uart          = 0;          /* UART 0 (Serial) */
    modbus_cfg.holding_regs  = holding_regs;
    modbus_cfg.holding_count = HOLDING_REG_COUNT;
    modbus_cfg.input_regs    = input_regs;
    modbus_cfg.input_count   = INPUT_REG_COUNT;

    syn_modbus_init(&modbus_slave, &modbus_cfg, modbus_buf, sizeof(modbus_buf));

    /* Initialize LED */
    syn_led_init(&status_led, STATUS_LED_PIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&status_led, 100, 100);

    /* Register Tasks */
    syn_task_create(&tasks[0], "modbus", modbus_task, 1, NULL);
    syn_task_create(&tasks[1], "blink",  blink_task,  2, NULL);

    syn_sched_init(&sched, tasks, 2);
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
