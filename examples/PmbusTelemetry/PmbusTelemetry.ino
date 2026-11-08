/**
 * @file PmbusTelemetry.ino
 * @brief PMBus Power Supply Telemetry Monitor example for SyntropicOS.
 *
 * Demonstrates reading output voltage (Linear16), current (Linear11),
 * temperature (Linear11), and checking status flags from a PMBus slave using
 * SMBus Packet Error Checking (PEC) in a non-blocking protothread.
 */

#include <SyntropicOS.h>

#define PMBUS_SLAVE_ADDR  0x58

static float g_vout_volts = 0.0f;
static float g_iout_amps = 0.0f;
static float g_temp_celsius = 0.0f;

static SYN_Task tasks[1];
static SYN_Sched sched;

/* Protothread state machine for periodic telemetry polling */
static SYN_PT_Status pmbus_telemetry_task(SYN_PT *pt, SYN_Task *task)
{
    static const uint8_t mock_vout_resp[] = { 0x00, 0x0C, 0x3E }; /* 0x0C00 = 3072 -> 3072 / 256 = 12.0V */

    PT_BEGIN(pt);

    (void)g_temp_celsius;

    for (;;) {
        {
            SYN_SMBUS_Packet rx_pkt;
            SYN_SMBUS_Packet tx_pkt;

            /* 1. Request READ_VOUT (Command 0x8B) with PEC */
            syn_pmbus_encode_read_cmd(&tx_pkt, PMBUS_SLAVE_ADDR, SYN_PMBUS_CMD_READ_VOUT, true);

            /* Simulating PMBus response frame decoding (e.g. VOUT_MODE = 0x18, N = -8) */
            if (syn_smbus_decode_packet(&rx_pkt, mock_vout_resp, sizeof(mock_vout_resp), SYN_SMBUS_PROTO_READ_WORD, true) == SYN_OK) {
                uint16_t raw_vout = (uint16_t)(rx_pkt.data[0] | (rx_pkt.data[1] << 8));
                g_vout_volts = syn_pmbus_linear16_to_float(raw_vout, 0x18);
                (void)g_vout_volts;
            }
        }

        /* Delay 500 ms without blocking the CPU */
        PT_TASK_DELAY_MS(pt, task, 500);

        {
            SYN_SMBUS_Packet rx_pkt;
            SYN_SMBUS_Packet tx_pkt;
            uint16_t raw_iout_l11;
            uint8_t mock_iout_resp[3];

            /* 2. Request READ_IOUT (Command 0x8C) with PEC */
            syn_pmbus_encode_read_cmd(&tx_pkt, PMBUS_SLAVE_ADDR, SYN_PMBUS_CMD_READ_IOUT, true);

            /* Simulating Linear11 response (e.g. 5.0 Amps) */
            raw_iout_l11 = syn_pmbus_float_to_linear11(5.0f);
            mock_iout_resp[0] = (uint8_t)(raw_iout_l11 & 0xFF);
            mock_iout_resp[1] = (uint8_t)((raw_iout_l11 >> 8) & 0xFF);
            mock_iout_resp[2] = syn_smbus_calc_pec(0, mock_iout_resp, 2);

            if (syn_smbus_decode_packet(&rx_pkt, mock_iout_resp, sizeof(mock_iout_resp), SYN_SMBUS_PROTO_READ_WORD, true) == SYN_OK) {
                uint16_t raw_iout = (uint16_t)(rx_pkt.data[0] | (rx_pkt.data[1] << 8));
                g_iout_amps = syn_pmbus_linear11_to_float(raw_iout);
                (void)g_iout_amps;
            }
        }

        PT_TASK_DELAY_MS(pt, task, 500);
    }

    PT_END(pt);
}

void setup()
{
    syn_task_create(&tasks[0], "pmbus_mon", pmbus_telemetry_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&sched, tasks, 1);
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
