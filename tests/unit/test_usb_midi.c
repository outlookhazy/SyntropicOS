/**
 * @file test_usb_midi.c
 * @brief Unity tests for Zero-Heap USB MIDI Class Device Driver.
 */

#include "mocks/mock_port.h"
#include "syntropic/drivers/syn_usb_midi.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static SYN_USB_MIDI g_midi;
static SYN_USB_Device g_usb_dev;

void test_usb_midi_init_and_null_checks(void)
{
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_init(NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_usb_midi_init(&g_midi));
    TEST_ASSERT_EQUAL(0x81U, g_midi.ep_in);
    TEST_ASSERT_EQUAL(0x01U, g_midi.ep_out);

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_register(NULL, &g_midi));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_register(&g_usb_dev, NULL));

    static const uint8_t dev_desc[18] = {18,   1,    0x00, 0x02, 0, 0, 0, 64, 0xFE,
                                         0xCA, 0x01, 0x00, 0,    1, 1, 2, 0,  1};
    syn_usb_init(&g_usb_dev, dev_desc);
    TEST_ASSERT_EQUAL(SYN_OK, syn_usb_midi_register(&g_usb_dev, &g_midi));

    /* Test class driver callbacks */
    TEST_ASSERT_EQUAL(1, g_usb_dev.class_count);
    SYN_USB_ClassDriver *drv = &g_usb_dev.classes[0];
    SYN_USB_SetupPacket pkt = {0};
    uint8_t resp[16];
    uint16_t rlen = 0;
    TEST_ASSERT_EQUAL(SYN_OK, drv->setup(g_midi.ep_in == 0 ? NULL : &g_midi, &pkt, resp, &rlen));
    TEST_ASSERT_EQUAL(SYN_OK, drv->configured(&g_midi, 1));

    uint8_t dummy_data[4] = {0x09, 0x90, 0x3C, 0x64};
    drv->data_out(NULL, 0x01, dummy_data, 4);
    drv->data_out(&g_midi, 0x01, NULL, 4);
    drv->data_out(&g_midi, 0x01, dummy_data, 0);
    drv->data_out(&g_midi, 0x02, dummy_data, 4);
    drv->data_out(&g_midi, 0x01, dummy_data, 4);
    TEST_ASSERT_EQUAL(4, g_midi.rx_len);

    uint8_t oversized_data[128];
    memset(oversized_data, 0x55, sizeof(oversized_data));
    drv->data_out(&g_midi, 0x01, oversized_data, 128);
    TEST_ASSERT_EQUAL(64, g_midi.rx_len);

    drv->data_in(NULL, 0x81);
    drv->data_in(&g_midi, 0x82);
    g_midi.tx_len = 4;
    drv->data_in(&g_midi, 0x81);
    TEST_ASSERT_EQUAL(0, g_midi.tx_len);

    SYN_USB_MIDI_Packet midi_pkt;
    uint8_t raw[4] = {0x09, 0x90, 0x3C, 0x64};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_parse_packet(NULL, &midi_pkt));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_parse_packet(raw, NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_usb_midi_parse_packet(raw, &midi_pkt));
    TEST_ASSERT_EQUAL(0x09, midi_pkt.header);
    TEST_ASSERT_EQUAL(0x90, midi_pkt.midi0);
    TEST_ASSERT_EQUAL(0x3C, midi_pkt.midi1);
    TEST_ASSERT_EQUAL(0x64, midi_pkt.midi2);
}

void test_usb_midi_send_events(void)
{
    syn_usb_midi_init(&g_midi);

    /* Note On: Channel 0 (C4 = 60, Velocity = 100) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_usb_midi_send_note_on(&g_midi, 0, 60, 100));
    TEST_ASSERT_EQUAL(4, g_midi.tx_len);
    TEST_ASSERT_EQUAL(0x09, g_midi.tx_buf[0]);
    TEST_ASSERT_EQUAL(0x90, g_midi.tx_buf[1]);
    TEST_ASSERT_EQUAL(60, g_midi.tx_buf[2]);
    TEST_ASSERT_EQUAL(100, g_midi.tx_buf[3]);

    /* Note Off: Channel 0 (C4 = 60, Velocity = 0) */
    g_midi.tx_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_usb_midi_send_note_off(&g_midi, 0, 60, 0));
    TEST_ASSERT_EQUAL(4, g_midi.tx_len);
    TEST_ASSERT_EQUAL(0x08, g_midi.tx_buf[0]);
    TEST_ASSERT_EQUAL(0x80, g_midi.tx_buf[1]);

    /* Control Change: Channel 1 (CC 7 = Volume, Value 127) */
    g_midi.tx_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_usb_midi_send_cc(&g_midi, 1, 7, 127));
    TEST_ASSERT_EQUAL(4, g_midi.tx_len);
    TEST_ASSERT_EQUAL(0x0B, g_midi.tx_buf[0]);
    TEST_ASSERT_EQUAL(0xB1, g_midi.tx_buf[1]);
    TEST_ASSERT_EQUAL(7, g_midi.tx_buf[2]);
    TEST_ASSERT_EQUAL(127, g_midi.tx_buf[3]);

    /* Pitch Bend: Channel 0 (Center = 0 -> 8192 => 0x2000 -> LSB=0, MSB=64) */
    g_midi.tx_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_usb_midi_send_pitch_bend(&g_midi, 0, 0));
    TEST_ASSERT_EQUAL(4, g_midi.tx_len);
    TEST_ASSERT_EQUAL(0x0E, g_midi.tx_buf[0]);
    TEST_ASSERT_EQUAL(0xE0, g_midi.tx_buf[1]);
    TEST_ASSERT_EQUAL(0x00, g_midi.tx_buf[2]);
    TEST_ASSERT_EQUAL(0x40, g_midi.tx_buf[3]);

    /* Invalid parameters checks */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_send_note_on(NULL, 0, 60, 100));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_send_note_on(&g_midi, 16, 60, 100));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_send_note_on(&g_midi, 0, 128, 100));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_send_note_on(&g_midi, 0, 60, 128));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_send_note_off(NULL, 0, 60, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_send_note_off(&g_midi, 16, 60, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_send_cc(NULL, 0, 7, 127));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_send_cc(&g_midi, 0, 128, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_send_pitch_bend(NULL, 0, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_send_pitch_bend(&g_midi, 0, 9000));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_midi_send_pitch_bend(&g_midi, 0, -9000));
}

void test_usb_midi_buffer_overflow(void)
{
    syn_usb_midi_init(&g_midi);
    g_midi.tx_len = SYN_USB_MIDI_MAX_PACKET_SIZE;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_usb_midi_send_note_on(&g_midi, 0, 60, 100));
}

static void test_r5_usb_midi_regressions(void)
{
    static const uint8_t dev_desc[18] = {18,   1,    0x00, 0x02, 0, 0, 0, 64, 0xFE,
                                         0xCA, 0x01, 0x00, 0,    1, 1, 2, 0,  1};
    syn_usb_init(&g_usb_dev, dev_desc);
    syn_usb_midi_init(&g_midi);
    TEST_ASSERT_EQUAL(SYN_OK, syn_usb_midi_register(&g_usb_dev, &g_midi));
    TEST_ASSERT_EQUAL_UINT16(9U + SYN_USB_MIDI_DESC_SIZE, g_usb_dev.config_desc_len);
    TEST_ASSERT_EQUAL_UINT8(2U, g_usb_dev.config_buf[4]);

    SYN_USB_ClassDriver *drv = &g_usb_dev.classes[0];
    SYN_USB_SetupPacket pkt = {0};
    uint8_t resp[8];
    TEST_ASSERT_EQUAL(SYN_OK, drv->setup(&g_midi, &pkt, resp, NULL));
    TEST_ASSERT_EQUAL(SYN_OK, drv->configured(NULL, 0U));

    for (uint8_t i = 0U; i < 16U; i++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_usb_midi_send_note_on(&g_midi, 0U, (uint8_t)(40U + i), 90U));
    }
    TEST_ASSERT_EQUAL_UINT16(64U, g_midi.tx_len);
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_usb_midi_send_note_on(&g_midi, 0U, 72U, 90U));
    TEST_ASSERT_EQUAL_UINT8(40U, g_midi.tx_buf[2]);

    drv->data_in(&g_midi, g_midi.ep_in);
    TEST_ASSERT_EQUAL_UINT16(0U, g_midi.tx_len);
}

void run_usb_midi_tests(void)
{
    RUN_TEST(test_usb_midi_init_and_null_checks);
    RUN_TEST(test_usb_midi_send_events);
    RUN_TEST(test_usb_midi_buffer_overflow);
    RUN_TEST(test_r5_usb_midi_regressions);
}
