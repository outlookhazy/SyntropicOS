/**
 * @file stm32_devicenet_example.c
 * @brief SyntropicOS STM32 ODVA DeviceNet (CIP over CAN) Slave Node Example.
 *
 * Demonstrates integrating a zero-heap DeviceNet slave node on STM32 using CAN.
 * Features Duplicate MAC ID check, CIP Object Model (Identity 0x01, DeviceNet 0x03,
 * Assembly 0x04), Polled I/O assembly data exchange, and QuickConnect™ hot-swapping.
 */

#include "syntropic/syntropic.h"
#include "syntropic/drivers/syn_can.h"
#include "syntropic/log/syn_log.h"
#include "syntropic/proto/syn_devicenet.h"
#include "syntropic/sched/syn_sched.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define STM32_DEVICENET_MAC_ID 0x06U /* DeviceNet Node Address: 6 */

/* DeviceNet Node & CAN Bus Instances */
static SYN_DeviceNet_Node g_dnet;
static SYN_CAN g_can;

/* Scheduler & Task Handles */
static SYN_Task g_task;
static SYN_Sched g_sched;

/* Assembly Object (Class 0x04) Buffers */
static uint8_t g_sensor_inputs[4] = {0x01, 0x02, 0x03, 0x04};
static uint8_t g_actuator_outputs[4] = {0x00};

/* Simulated CAN hardware receive hook */
static void on_can_frame_received(uint32_t can_id, const uint8_t data[8], uint8_t len)
{
    uint32_t tx_can_id = 0;
    uint8_t tx_data[8] = {0};
    uint8_t tx_len = 0;

    /* Process incoming CAN frame through DeviceNet stack */
    if (syn_devicenet_on_can_rx(&g_dnet, can_id, data, len, &tx_can_id, tx_data, &tx_len)) {
        /* Transmit DeviceNet response frame over STM32 bxCAN / FDCAN driver */
        SYN_CAN_Frame tx_frame = {0};
        tx_frame.id = tx_can_id;
        tx_frame.dlc = tx_len;
        memcpy(tx_frame.data, tx_data, tx_len);
        syn_can_send(&g_can, &tx_frame);
    }
}

/* Periodic 10ms timer task servicing DeviceNet state machine */
static SYN_PT_Status app_10ms_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    for (;;) {
        /* Advance DeviceNet timers (Duplicate MAC check & QuickConnect) */
        syn_devicenet_poll(&g_dnet, 10);

        /* Update simulated sensor input assembly data */
        g_sensor_inputs[0]++;

        PT_TASK_DELAY_MS(pt, task, 10);
    }

    PT_END(pt);
}

void stm32_devicenet_example_init(void)
{
    (void)on_can_frame_received;

    syn_can_init(&g_can, 0U, 500000U);

    /* Initialize DeviceNet Node at MAC ID 6, 500kbps Baud Rate */
    syn_devicenet_init(&g_dnet, STM32_DEVICENET_MAC_ID, SYN_DEVICENET_BAUD_500K);

    /* Configure I/O Assembly Object buffers */
    syn_devicenet_set_assembly(&g_dnet, g_sensor_inputs, sizeof(g_sensor_inputs),
                               g_actuator_outputs, sizeof(g_actuator_outputs));

    /* Enable QuickConnect™ for fast hot-swap tool changing */
    syn_devicenet_set_quickconnect(&g_dnet, true);

    /* Initialize task and scheduler */
    syn_task_create(&g_task, "dnet_task", app_10ms_task, SYN_PRIO_HIGH, NULL);
    syn_sched_init(&g_sched, &g_task, 1);
}
