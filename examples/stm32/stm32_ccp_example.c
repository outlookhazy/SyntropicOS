/**
 * @file stm32_ccp_example.c
 * @brief SyntropicOS STM32 CAN Calibration Protocol (CCP v2.1) Example Application.
 *
 * Demonstrates integrating CCP v2.1 slave protocol stack on STM32 with bxCAN.
 * Enables live tuning of PID parameters, motor control variables, and periodic
 * DAQ list measurement streaming.
 */

#include "syntropic/syntropic.h"
#include "syntropic/control/syn_pid.h"
#include "syntropic/drivers/syn_can.h"
#include "syntropic/log/syn_log.h"
#include "syntropic/proto/syn_ccp.h"
#include "syntropic/sched/syn_sched.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define STM32_CCP_STATION_ADDR 0x0100U

/* Live tunable PID controller parameters */
static SYN_PID g_pid;
static float g_kp = 2.5f;
static float g_ki = 0.1f;
static float g_kd = 0.05f;
static float g_setpoint = 100.0f;
static float g_process_var = 0.0f;

/* CCP Slave & CAN Bus Instances */
static SYN_CCP_Slave g_ccp;
static SYN_CAN g_can;

/* Scheduler & Task Handles */
static SYN_Task g_task;
static SYN_Sched g_sched;

/* Simulated CAN hardware receive hook */
static void on_can_frame_received(uint32_t can_id, const uint8_t data[8], uint8_t len)
{
    (void)len;
    /* Master Command Receive Object (CRO) CAN ID = 0x100 */
    if (can_id == 0x100U) {
        uint8_t dto[8] = {0};
        if (syn_ccp_process_cro(&g_ccp, data, dto)) {
            /* Transmit Command Return Message (DTO/CRM) on CAN ID = 0x101 */
            SYN_CAN_Frame tx_frame = {0};
            tx_frame.id = 0x101U;
            tx_frame.dlc = 8;
            memcpy(tx_frame.data, dto, 8);
            syn_can_send(&g_can, &tx_frame);
        }
    }
}

/* Periodic 10ms timer task servicing live DAQ streaming & PID control */
static SYN_PT_Status app_10ms_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    for (;;) {
        /* Update simulated system & compute PID */
        float control_output = syn_pid_update(&g_pid, g_setpoint, g_process_var, 0.010f);
        g_process_var += (control_output * 0.01f);

        /* Service periodic CCP DAQ Event Channel 1 */
        uint8_t daq_dto[8] = {0};
        uint8_t list_idx = 0;
        uint8_t odt_idx = 0;
        if (syn_ccp_service_daq(&g_ccp, 0x01U, daq_dto, &list_idx, &odt_idx)) {
            /* Send DAQ packet on CAN ID = 0x102 + list_idx */
            SYN_CAN_Frame tx_frame = {0};
            tx_frame.id = 0x102U + list_idx;
            tx_frame.dlc = 8;
            memcpy(tx_frame.data, daq_dto, 8);
            syn_can_send(&g_can, &tx_frame);
        }

        PT_TASK_DELAY_MS(pt, task, 10);
    }

    PT_END(pt);
}

void stm32_ccp_example_init(void)
{
    (void)on_can_frame_received;

    syn_can_init(&g_can, 0U, 500000U);

    /* Initialize live tunable PID controller */
    SYN_PID_Config pid_cfg = {
        .kp = g_kp, .ki = g_ki, .kd = g_kd, .out_min = -100.0f, .out_max = 100.0f};
    syn_pid_init(&g_pid, &pid_cfg);

    /* Initialize CCP v2.1 slave stack */
    syn_ccp_init(&g_ccp, STM32_CCP_STATION_ADDR);

    /* Initialize task and scheduler */
    syn_task_create(&g_task, "ccp_task", app_10ms_task, SYN_PRIO_HIGH, NULL);
    syn_sched_init(&g_sched, &g_task, 1);
}
