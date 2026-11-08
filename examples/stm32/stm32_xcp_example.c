/**
 * @file stm32_xcp_example.c
 * @brief SyntropicOS STM32 XCP (ASAM MCD-1 XCP v1.x) Example Application.
 *
 * Demonstrates integrating ASAM XCP slave protocol stack on STM32 with bxCAN.
 * Enables live tuning of PID parameters, motor control variables, and periodic
 * DAQ list measurement streaming.
 */

#include "syntropic/syntropic.h"
#include "syntropic/control/syn_pid.h"
#include "syntropic/drivers/syn_can.h"
#include "syntropic/log/syn_log.h"
#include "syntropic/proto/syn_xcp.h"
#include "syntropic/sched/syn_sched.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define STM32_XCP_STATION_ID 0x0200U

/* Live tunable PID controller parameters */
static SYN_PID g_pid;
static float g_kp = 2.5f;
static float g_ki = 0.1f;
static float g_kd = 0.05f;
static float g_setpoint = 100.0f;
static float g_process_var = 0.0f;

/* XCP Slave & CAN Bus Instances */
static SYN_XCP_Slave g_xcp;
static SYN_CAN g_can;

/* Scheduler & Task Handles */
static SYN_Task g_task;
static SYN_Sched g_sched;

/* Simulated CAN hardware receive hook */
static void on_can_frame_received(uint32_t can_id, const uint8_t data[8], uint8_t len)
{
    (void)len;
    /* Master Command Transfer Object (CTO) CAN ID = 0x200 */
    if (can_id == 0x200U) {
        uint8_t dto[8] = {0};
        if (syn_xcp_process_cto(&g_xcp, data, dto)) {
            /* Transmit Command Return Message (DTO/RES) on CAN ID = 0x201 */
            SYN_CAN_Frame tx_frame = {0};
            tx_frame.id = 0x201U;
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

        /* Service periodic XCP DAQ Event Channel 1 */
        uint8_t daq_dto[8] = {0};
        uint8_t list_idx = 0;
        uint8_t odt_idx = 0;
        if (syn_xcp_service_daq(&g_xcp, 0x01U, daq_dto, &list_idx, &odt_idx)) {
            /* Send DAQ telemetry packet on CAN ID = 0x202 + list_idx */
            SYN_CAN_Frame tx_frame = {0};
            tx_frame.id = 0x202U + list_idx;
            tx_frame.dlc = 8;
            memcpy(tx_frame.data, daq_dto, 8);
            syn_can_send(&g_can, &tx_frame);
        }

        PT_TASK_DELAY_MS(pt, task, 10);
    }

    PT_END(pt);
}

void stm32_xcp_example_init(void)
{
    (void)on_can_frame_received;

    syn_can_init(&g_can, 0U, 500000U);

    /* Initialize PID controller */
    SYN_PID_Config pid_cfg = {
        .kp = g_kp, .ki = g_ki, .kd = g_kd, .out_min = -100.0f, .out_max = 100.0f};
    syn_pid_init(&g_pid, &pid_cfg);

    /* Initialize XCP Slave */
    syn_xcp_init(&g_xcp, STM32_XCP_STATION_ID);

    /* Initialize task and scheduler */
    syn_task_create(&g_task, "xcp_pid_task", app_10ms_task, SYN_PRIO_HIGH, NULL);
    syn_sched_init(&g_sched, &g_task, 1);
}
