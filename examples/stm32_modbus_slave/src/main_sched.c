/**
 * @file main_sched.c
 * @brief STM32 Modbus RTU Slave Example (`syn_sched` Cooperative Task Scheduler Variant).
 *
 * Demonstrates Modbus RTU slave execution under the SyntropicOS task scheduler:
 * - High-priority Modbus RTU RX frame polling task
 * - Periodic 100ms ADC sensor sampling & register update task
 * - Clean separation of application tasks using stackless protothreads (syn_pt)
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "syntropic/proto/syn_modbus.h"
#include "syntropic/port/syn_port_uart.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"

#define MODBUS_SLAVE_ADDR      1
#define HOLDING_REG_COUNT     16
#define INPUT_REG_COUNT       16
#define COIL_COUNT            16
#define DISCRETE_INPUT_COUNT  16

static uint16_t s_holding_regs[HOLDING_REG_COUNT];
static uint16_t s_input_regs[INPUT_REG_COUNT];
static uint8_t  s_coils[(COIL_COUNT + 7) / 8];
static uint8_t  s_discrete_inputs[(DISCRETE_INPUT_COUNT + 7) / 8];

static SYN_Modbus s_modbus_slave;
static uint8_t    s_modbus_rx_buf[256];

#define TASK_MODBUS_POLL   0
#define TASK_SENSOR_UPDATE 1
#define TASK_COUNT         2

static SYN_Task  s_tasks[TASK_COUNT];
static SYN_Sched s_sched;

static uint16_t stm32_read_adc_temperature(void)
{
    return 245U;
}

static uint16_t stm32_read_adc_bus_voltage(void)
{
    return 2400U;
}

static void stm32_set_relay_state(bool state)
{
    (void)state;
}

static void stm32_set_pwm_duty(uint16_t duty_0_to_1000)
{
    (void)duty_0_to_1000;
}

/* Task 0: High-Priority Modbus RTU RX Polling Task */
static SYN_PT_Status task_modbus_poll_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    while (1) {
        syn_modbus_poll(&s_modbus_slave);
        PT_PASS(pt);
    }

    PT_END(pt);
}

/* Task 1: Periodic 100ms Sensor Sampling & Output Update Task */
static SYN_PT_Status task_sensor_update_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    while (1) {
        uint32_t now_ms = syn_port_get_tick_ms();
        s_input_regs[0] = stm32_read_adc_temperature();
        s_input_regs[1] = stm32_read_adc_bus_voltage();
        s_input_regs[2] = (uint16_t)(now_ms / 1000u);

        bool relay_on = (s_coils[0] & 0x01) != 0;
        stm32_set_relay_state(relay_on);

        uint16_t pwm_setpoint = s_holding_regs[0];
        if (pwm_setpoint > 1000U) {
            pwm_setpoint = 1000U;
        }
        stm32_set_pwm_duty(pwm_setpoint);

        PT_TASK_DELAY_MS(pt, task, 100);
    }

    PT_END(pt);
}

/* Scheduler-based entry point */
int main_sched(void)
{
    SYN_Modbus_Config cfg = {
        .slave_addr      = MODBUS_SLAVE_ADDR,
        .uart            = 0,
        .holding_regs    = s_holding_regs,
        .holding_count   = HOLDING_REG_COUNT,
        .input_regs      = s_input_regs,
        .input_count     = INPUT_REG_COUNT,
        .coils           = s_coils,
        .coils_count     = COIL_COUNT,
        .discrete_inputs = s_discrete_inputs,
        .discrete_count  = DISCRETE_INPUT_COUNT,
    };

    syn_modbus_init(&s_modbus_slave, &cfg, s_modbus_rx_buf, sizeof(s_modbus_rx_buf));

    s_holding_regs[0] = 500U;
    s_holding_regs[1] = 1152;

    syn_task_create(&s_tasks[TASK_MODBUS_POLL], "ModbusPoll", task_modbus_poll_func, SYN_PRIO_HIGH,
                    NULL);
    syn_task_create(&s_tasks[TASK_SENSOR_UPDATE], "SensorUpdate", task_sensor_update_func,
                    SYN_PRIO_NORMAL, NULL);

    syn_sched_init(&s_sched, s_tasks, TASK_COUNT);

    while (1) {
        syn_sched_run(&s_sched);
    }

    return 0;
}
