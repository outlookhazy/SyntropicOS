/**
 * SyntropicOS — Telemetry CBOR & COBS Pipeline
 *
 * Demonstrates zero-heap binary serialization and packet framing:
 *   - CBOR encoding & decoding (syn_cbor_write, syn_cbor_read)
 *   - Consistent Overhead Byte Stuffing packet framing (syn_cobs)
 *   - Non-blocking circular ring buffer (syn_ringbuf)
 *   - CRC-16 integrity verification (syn_crc)
 *   - Cooperative multi-task scheduler (syn_sched)
 *
 * Producer task packages telemetry data into CBOR, frames with COBS,
 * and pushes to ring buffer. Consumer task decodes and validates packet.
 *
 * Open Serial Monitor at 115200 baud.
 *
 * Documentation & Related Features:
 *   - Protocols Guide:      https://outlookhazy.github.io/SyntropicOS/modules/communication/
 *   - Protocol API Ref:     https://outlookhazy.github.io/SyntropicOS/syntropic/group__syn__protocol/
 *   - Network Transports:   https://outlookhazy.github.io/SyntropicOS/syntropic/group__syn__net/ (syn_mqtt / syn_coap)
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/util/syn_cbor_write.h>
#include <syntropic/util/syn_cbor_read.h>
#include <syntropic/proto/syn_cobs.h>
#include <syntropic/util/syn_ringbuf.h>
#include <syntropic/util/syn_crc.h>
#include <syntropic/output/syn_led.h>
#include <string.h>

#define TAG "telemetry"

static SYN_CLI       cli;
static SYN_Sched     sched;
static SYN_Task      tasks[3];
static SYN_LED       led;

/* Communication Ring Buffer (128 bytes) */
static uint8_t       ringbuf_mem[128];
static SYN_RingBuf   ringbuf;

/* Simulated Sensor Data */
static int16_t  temp_x10 = 245;   /* 24.5°C */
static uint8_t  humidity = 55;    /* 55% */
static uint16_t battery_mv = 3300;/* 3.3V */
static uint32_t sample_count = 0;

/* ── Platform Hooks ───────────────────────────────────────────────────── */

extern "C" void syn_assert_failed(const char *f, int l) { (void)f; (void)l; for(;;); }

/* ── Tasks ────────────────────────────────────────────────────────────── */

static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&led);
        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

/* Producer Task — encodes telemetry to CBOR + COBS every 1000ms */
static SYN_PT_Status producer_task(SYN_PT *pt, SYN_Task *task)
{
    static uint8_t cbor_buf[48];
    static uint8_t cobs_buf[64];
    PT_BEGIN(pt);
    for (;;) {
        sample_count++;
        temp_x10 = 245 + (sample_count % 30); /* Simulate small variation */

        /* 1. Encode telemetry as a 4-item CBOR map */
        SYN_CborWriter w;
        syn_cbor_writer_init(&w, cbor_buf, sizeof(cbor_buf));
        syn_cbor_write_map_begin(&w, 4);

        syn_cbor_write_uint(&w, 1); /* Key 1: Temp */
        syn_cbor_write_int(&w, temp_x10);

        syn_cbor_write_uint(&w, 2); /* Key 2: Humidity */
        syn_cbor_write_uint(&w, humidity);

        syn_cbor_write_uint(&w, 3); /* Key 3: Battery mV */
        syn_cbor_write_uint(&w, battery_mv);

        syn_cbor_write_uint(&w, 4); /* Key 4: Sample count */
        syn_cbor_write_uint(&w, sample_count);

        if (syn_cbor_writer_ok(&w)) {
            /* 2. Compute CRC16 over CBOR payload */
            size_t cbor_len = syn_cbor_writer_len(&w);
            uint16_t crc = syn_crc16_ccitt(cbor_buf, cbor_len);
            cbor_buf[cbor_len++] = (uint8_t)(crc >> 8);
            cbor_buf[cbor_len++] = (uint8_t)(crc & 0xFF);

            /* 3. COBS encode payload */
            size_t cobs_len = syn_cobs_encode(cbor_buf, cbor_len, cobs_buf);
            cobs_buf[cobs_len++] = 0x00; /* Delimiter byte */

            /* 4. Push encoded packet to ring buffer */
            syn_ringbuf_write(&ringbuf, cobs_buf, cobs_len);
        }

        PT_TASK_DELAY_MS(pt, task, 1000);
    }
    PT_END(pt);
}

/* Consumer Task — decodes COBS + CBOR packets from ring buffer */
static SYN_PT_Status consumer_task(SYN_PT *pt, SYN_Task *task)
{
    static uint8_t frame_buf[64];
    static size_t frame_len = 0;
    static uint8_t decoded_cbor[48];

    PT_BEGIN(pt);
    for (;;) {
        /* Read available bytes into frame buffer until 0x00 delimiter */
        uint8_t byte;
        while (syn_ringbuf_get(&ringbuf, &byte)) {
            if (byte == 0x00) {
                if (frame_len > 2) {
                    /* COBS Decode */
                    size_t raw_len = syn_cobs_decode(frame_buf, frame_len, decoded_cbor);
                    if (raw_len > 2) {
                        size_t cbor_len = raw_len - 2;
                        uint16_t rx_crc = ((uint16_t)decoded_cbor[cbor_len] << 8) | decoded_cbor[cbor_len + 1];
                        uint16_t calc_crc = syn_crc16_ccitt(decoded_cbor, cbor_len);

                        if (rx_crc == calc_crc) {
                            /* Parse CBOR map */
                            SYN_CborReader r;
                            syn_cbor_reader_init(&r, decoded_cbor, cbor_len);
                            size_t pairs = syn_cbor_read_map_begin(&r);

                            int16_t rx_temp = 0;
                            uint8_t rx_hum = 0;
                            uint16_t rx_bat = 0;
                            uint32_t rx_cnt = 0;

                            for (size_t i = 0; i < pairs; i++) {
                                uint64_t key = syn_cbor_read_uint(&r);
                                if      (key == 1) rx_temp = (int16_t)syn_cbor_read_int(&r);
                                else if (key == 2) rx_hum  = (uint8_t)syn_cbor_read_uint(&r);
                                else if (key == 3) rx_bat  = (uint16_t)syn_cbor_read_uint(&r);
                                else if (key == 4) rx_cnt  = (uint32_t)syn_cbor_read_uint(&r);
                                else syn_cbor_skip(&r);
                            }

                            if (syn_cbor_reader_ok(&r)) {
                                syn_cli_printf(&cli, "[PASS] CBOR Telemetry packet decoded: #%ld temp=%d.%dC hum=%u%% bat=%umV\r\n",
                                               rx_cnt, rx_temp / 10, abs(rx_temp % 10), rx_hum, rx_bat);
                                syn_led_flash(&led, 50, 50, 1);
                            }
                        } else {
                            syn_cli_printf(&cli, "[FAIL] CRC Mismatch\r\n");
                        }
                    }
                }
                frame_len = 0;
            } else {
                if (frame_len < sizeof(frame_buf)) {
                    frame_buf[frame_len++] = byte;
                } else {
                    frame_len = 0; /* Overflow, reset frame */
                }
            }
        }

        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

/* ── Entry Point ──────────────────────────────────────────────────────── */

void setup()
{
    syn_port_serial_init(115200);
    syn_led_init(&led, LED_BUILTIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&led, 500, 500);

    syn_ringbuf_init(&ringbuf, ringbuf_mem, sizeof(ringbuf_mem));

    syn_log_init(SYN_LOG_INFO);
    SYN_LOG_I(TAG, "Telemetry CBOR Pipeline Ready");

    syn_cli_init(&cli, NULL, 0, "> ");
    syn_cli_printf(&cli, "\r\n--- SyntropicOS Telemetry CBOR Demo ---\r\n");

    syn_task_create(&tasks[0], "blink",    blink_task,    2, NULL);
    syn_task_create(&tasks[1], "producer", producer_task, 1, NULL);
    syn_task_create(&tasks[2], "consumer", consumer_task, 0, NULL);
    syn_sched_init(&sched, tasks, 3);
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
