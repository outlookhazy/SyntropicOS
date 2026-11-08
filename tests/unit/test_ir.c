/**
 * @file test_ir.c
 * @brief Unity unit tests for Infrared (IR) Remote Control Protocol Engine (syn_ir).
 */

#include "mocks/mock_port.h"
#include "syntropic/proto/syn_ir.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

#if defined(SYN_USE_IR) && SYN_USE_IR

static void test_ir_nec_decode_encode(void)
{
    SYN_IR_Decoder decoder;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_decoder_init(&decoder));

    /* 1. Standard NEC Frame: Address = 0x12, Command = 0x34 */
    SYN_IR_Frame tx_frame = {.protocol = SYN_IR_PROTO_NEC, .address = 0x12, .command = 0x34};

    SYN_IR_Pulse pulses[100];
    size_t count = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&tx_frame, pulses, 100, &count));
    TEST_ASSERT_GREATER_THAN(0, count);

    SYN_IR_Frame rx_frame;
    bool decoded = false;

    for (size_t i = 0; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            decoded = true;
        }
    }

    TEST_ASSERT_TRUE(decoded);
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_NEC, rx_frame.protocol);
    TEST_ASSERT_EQUAL(0x12, rx_frame.address);
    TEST_ASSERT_EQUAL(0x34, rx_frame.command);
    TEST_ASSERT_EQUAL(38, rx_frame.carrier_khz);

    /* 2. Test NEC Repeat Code */
    decoded = syn_ir_decode_pulse(&decoder, 9000, true, &rx_frame);
    TEST_ASSERT_FALSE(decoded);
    decoded = syn_ir_decode_pulse(&decoder, 2250, false, &rx_frame);
    TEST_ASSERT_TRUE(decoded);
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_NEC, rx_frame.protocol);
    TEST_ASSERT_EQUAL(0x12, rx_frame.address);
    TEST_ASSERT_EQUAL(0x34, rx_frame.command);
    TEST_ASSERT_TRUE((rx_frame.flags & SYN_IR_FLAG_REPEAT) != 0);

    /* 3. NEC Extended Frame: Address = 0x1234, Command = 0x56 */
    syn_ir_decoder_init(&decoder);
    SYN_IR_Frame ext_tx = {
        .protocol = SYN_IR_PROTO_NEC_EXTENDED, .address = 0x1234, .command = 0x56};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&ext_tx, pulses, 100, &count));

    for (size_t i = 0; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            decoded = true;
        }
    }
    TEST_ASSERT_TRUE(decoded);
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_NEC_EXTENDED, rx_frame.protocol);
    TEST_ASSERT_EQUAL(0x1234, rx_frame.address);
    TEST_ASSERT_EQUAL(0x56, rx_frame.command);

    /* 4. Apple IR Frame */
    syn_ir_decoder_init(&decoder);
    SYN_IR_Frame apple_tx = {.protocol = SYN_IR_PROTO_APPLE, .address = 0xEE87, .command = 0x0B};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&apple_tx, pulses, 100, &count));

    for (size_t i = 0; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            decoded = true;
        }
    }
    TEST_ASSERT_TRUE(decoded);
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_APPLE, rx_frame.protocol);
    TEST_ASSERT_EQUAL(0xEE87, rx_frame.address);
    TEST_ASSERT_EQUAL(0x0B, rx_frame.command);
}

static void test_ir_sony_sircs_decode(void)
{
    SYN_IR_Decoder decoder;

    /* 1. Sony 12-bit SIRCS: Cmd = 0x1A (26), Addr = 0x01 */
    SYN_IR_Frame s12_tx = {.protocol = SYN_IR_PROTO_SONY_12, .address = 0x01, .command = 0x1A};
    SYN_IR_Pulse pulses[100];
    size_t count = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&s12_tx, pulses, 100, &count));

    syn_ir_decoder_init(&decoder);
    SYN_IR_Frame rx_frame;
    bool decoded = false;

    for (size_t i = 0; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            decoded = true;
        }
    }
    TEST_ASSERT_TRUE(decoded);
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_SONY_12, rx_frame.protocol);
    TEST_ASSERT_EQUAL(0x01, rx_frame.address);
    TEST_ASSERT_EQUAL(0x1A, rx_frame.command);
    TEST_ASSERT_EQUAL(40, rx_frame.carrier_khz);

    /* 2. Sony 15-bit SIRCS */
    SYN_IR_Frame s15_tx = {.protocol = SYN_IR_PROTO_SONY_15, .address = 0x8A, .command = 0x2B};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&s15_tx, pulses, 100, &count));

    syn_ir_decoder_init(&decoder);
    for (size_t i = 0; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            decoded = true;
        }
    }
    TEST_ASSERT_TRUE(decoded);
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_SONY_15, rx_frame.protocol);
    TEST_ASSERT_EQUAL(0x8A, rx_frame.address);
    TEST_ASSERT_EQUAL(0x2B, rx_frame.command);

    /* 3. Sony 20-bit SIRCS */
    SYN_IR_Frame s20_tx = {.protocol = SYN_IR_PROTO_SONY_20, .address = 0x05, .command = 0x3F};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&s20_tx, pulses, 100, &count));

    syn_ir_decoder_init(&decoder);
    for (size_t i = 0; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            decoded = true;
        }
    }
    TEST_ASSERT_TRUE(decoded);
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_SONY_20, rx_frame.protocol);
    TEST_ASSERT_EQUAL(0x05, rx_frame.address & 0x1FU);
    TEST_ASSERT_EQUAL(0x3F, rx_frame.command);
}

static void test_ir_rc5_rc6_samsung_kaseikyo_denon(void)
{
    SYN_IR_Decoder decoder;
    SYN_IR_Pulse pulses[150];
    size_t count = 0;
    SYN_IR_Frame rx_frame;
    bool decoded = false;

    /* 1. Samsung 32-bit: Address = 0x0707, Command = 0x02 */
    SYN_IR_Frame sam_tx = {.protocol = SYN_IR_PROTO_SAMSUNG, .address = 0x0707, .command = 0x02};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&sam_tx, pulses, 150, &count));
    syn_ir_decoder_init(&decoder);
    for (size_t i = 0; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            decoded = true;
        }
    }
    TEST_ASSERT_TRUE(decoded);
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_SAMSUNG, rx_frame.protocol);
    TEST_ASSERT_EQUAL(0x0707, rx_frame.address);
    TEST_ASSERT_EQUAL(0x02, rx_frame.command);

    /* 2. Kaseikyo 48-bit */
    SYN_IR_Frame kas_tx = {
        .protocol = SYN_IR_PROTO_KASEIKYO, .address = 0x4004, .command = 0x01002000};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&kas_tx, pulses, 150, &count));
    syn_ir_decoder_init(&decoder);
    for (size_t i = 0; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            decoded = true;
        }
    }
    TEST_ASSERT_TRUE(decoded);
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_KASEIKYO, rx_frame.protocol);
    TEST_ASSERT_EQUAL(0x4004, rx_frame.address);
    TEST_ASSERT_EQUAL(0x01002000, rx_frame.command);

    /* 3. Denon 15-bit */
    SYN_IR_Frame den_tx = {.protocol = SYN_IR_PROTO_DENON, .address = 0x0A, .command = 0x1C};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&den_tx, pulses, 150, &count));
    syn_ir_decoder_init(&decoder);
    for (size_t i = 0; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            decoded = true;
        }
    }
    TEST_ASSERT_TRUE(decoded);
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_DENON, rx_frame.protocol);
    TEST_ASSERT_EQUAL(0x0A, rx_frame.address);
    TEST_ASSERT_EQUAL(0x1C, rx_frame.command);

    /* 4. RC5 Frame with Toggle Bit */
    SYN_IR_Frame rc5_tx = {.protocol = SYN_IR_PROTO_RC5,
                           .address = 0x05,
                           .command = 0x0C,
                           .flags = SYN_IR_FLAG_TOGGLE};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&rc5_tx, pulses, 150, &count));
    syn_ir_decoder_init(&decoder);
    decoded = false;
    for (size_t i = 0; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            decoded = true;
        }
    }
    if (!decoded) {
        decoded = syn_ir_decode_timeout(&decoder, &rx_frame);
    }
    TEST_ASSERT_TRUE(decoded);
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_RC5, rx_frame.protocol);
    TEST_ASSERT_EQUAL(0x05, rx_frame.address);
    TEST_ASSERT_EQUAL(0x0C, rx_frame.command);

    /* 5. RC6 Frame */
    SYN_IR_Frame rc6_tx = {.protocol = SYN_IR_PROTO_RC6, .address = 0x04, .command = 0x11};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&rc6_tx, pulses, 150, &count));
    syn_ir_decoder_init(&decoder);
    decoded = false;
    for (size_t i = 0; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            decoded = true;
        }
    }
    if (!decoded) {
        decoded = syn_ir_decode_timeout(&decoder, &rx_frame);
    }
    TEST_ASSERT_TRUE(decoded);
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_RC6, rx_frame.protocol);
    TEST_ASSERT_EQUAL(0x04, rx_frame.address);
    TEST_ASSERT_EQUAL(0x11, rx_frame.command);
}

static void test_ir_edge_cases(void)
{
    SYN_IR_Decoder decoder;
    SYN_IR_Frame rx_frame;

    /* Invalid parameters */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ir_decoder_init(NULL));
    TEST_ASSERT_FALSE(syn_ir_decode_pulse(NULL, 100, true, &rx_frame));
    TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, 100, true, NULL));
    TEST_ASSERT_FALSE(syn_ir_decode_timeout(NULL, &rx_frame));
    TEST_ASSERT_FALSE(syn_ir_decode_timeout(&decoder, NULL));

    size_t count = 0;
    SYN_IR_Pulse pulses[10];
    SYN_IR_Frame frame = {.protocol = SYN_IR_PROTO_NEC, .address = 1, .command = 1};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ir_encode_frame(NULL, pulses, 10, &count));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ir_encode_frame(&frame, NULL, 10, &count));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ir_encode_frame(&frame, pulses, 10, NULL));

    SYN_IR_Frame bad_proto = {.protocol = (SYN_IR_Protocol)999, .address = 1, .command = 1};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ir_encode_frame(&bad_proto, pulses, 10, &count));

    /* Buffer capacity too small */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ir_encode_frame(&frame, pulses, 5, &count));

    /* Timeout decoding idle state */
    syn_ir_decoder_init(&decoder);
    TEST_ASSERT_FALSE(syn_ir_decode_timeout(&decoder, &rx_frame));

    /* Protocol names */
    TEST_ASSERT_EQUAL_STRING("NEC", syn_ir_protocol_name(SYN_IR_PROTO_NEC));
    TEST_ASSERT_EQUAL_STRING("Philips-RC5", syn_ir_protocol_name(SYN_IR_PROTO_RC5));
    TEST_ASSERT_EQUAL_STRING("Unknown", syn_ir_protocol_name(SYN_IR_PROTO_UNKNOWN));
    TEST_ASSERT_EQUAL_STRING("Unknown", syn_ir_protocol_name((SYN_IR_Protocol)999));

    /* Invalid timing corrupt pulse decoding */
    syn_ir_decoder_init(&decoder);
    /* Leader mark 9000us, but corrupt space 1000us */
    syn_ir_decode_pulse(&decoder, 9000, true, &rx_frame);
    TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, 1000, false, &rx_frame));

    /* Corrupt NEC checksum bit inversion */
    syn_ir_decoder_init(&decoder);
    /* Leader */
    syn_ir_decode_pulse(&decoder, 9000, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 4500, false, &rx_frame);
    /* Transmit 32 zeros (corrupt checksum: 0x00 ^ 0x00 != 0xFF) */
    for (int i = 0; i < 32; i++) {
        syn_ir_decode_pulse(&decoder, 560, true, &rx_frame);
        syn_ir_decode_pulse(&decoder, 560, false, &rx_frame);
    }
    /* Should fail validation */
    TEST_ASSERT_EQUAL(SYN_IR_STATE_IDLE, decoder.state);

    /* Corrupt PDM mark duration */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 9000, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 4500, false, &rx_frame);
    /* Bit 1: invalid mark duration 1500us */
    syn_ir_decode_pulse(&decoder, 1500, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 560, false, &rx_frame);
    TEST_ASSERT_EQUAL(SYN_IR_STATE_IDLE, decoder.state);

    /* Corrupt PDM space duration */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 9000, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 4500, false, &rx_frame);
    /* Bit 1: valid mark 560us, but invalid space 3000us */
    syn_ir_decode_pulse(&decoder, 560, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 3000, false, &rx_frame);
    TEST_ASSERT_EQUAL(SYN_IR_STATE_IDLE, decoder.state);

    /* Corrupt Sony PWM mark duration */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 2400, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 600, false, &rx_frame);
    /* Corrupt mark duration 2000us */
    syn_ir_decode_pulse(&decoder, 2000, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 600, false, &rx_frame);
    TEST_ASSERT_EQUAL(SYN_IR_STATE_IDLE, decoder.state);

    /* RC6 Frame with Toggle flag */
    SYN_IR_Frame rc6_toggle = {.protocol = SYN_IR_PROTO_RC6,
                               .address = 0x07,
                               .command = 0x02,
                               .flags = SYN_IR_FLAG_TOGGLE};
    SYN_IR_Pulse rc6_pulses[100];
    size_t rc6_cnt = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&rc6_toggle, rc6_pulses, 100, &rc6_cnt));
    syn_ir_decoder_init(&decoder);
    for (size_t i = 0; i < rc6_cnt; i++) {
        syn_ir_decode_pulse(&decoder, rc6_pulses[i].duration_us, rc6_pulses[i].is_mark, &rx_frame);
    }
    TEST_ASSERT_TRUE((rx_frame.flags & SYN_IR_FLAG_TOGGLE) != 0);

    /* Corrupt NEC command checksum (cmd ^ cmd_inv != 0xFF) */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 9000, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 4500, false, &rx_frame);
    /* 16 valid address bits (addr=0, addr_inv=0xFF) */
    for (int i = 0; i < 8; i++) {
        syn_ir_decode_pulse(&decoder, 560, true, &rx_frame);
        syn_ir_decode_pulse(&decoder, 560, false, &rx_frame);
    }
    for (int i = 0; i < 8; i++) {
        syn_ir_decode_pulse(&decoder, 560, true, &rx_frame);
        syn_ir_decode_pulse(&decoder, 1690, false, &rx_frame);
    }
    /* 16 corrupt command bits (cmd=0, cmd_inv=0) */
    for (int i = 0; i < 16; i++) {
        syn_ir_decode_pulse(&decoder, 560, true, &rx_frame);
        syn_ir_decode_pulse(&decoder, 560, false, &rx_frame);
    }
    TEST_ASSERT_EQUAL(SYN_IR_STATE_IDLE, decoder.state);

    /* Corrupt Samsung frame checksum */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 4500, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 4500, false, &rx_frame);
    for (int i = 0; i < 32; i++) {
        syn_ir_decode_pulse(&decoder, 590, true, &rx_frame);
        syn_ir_decode_pulse(&decoder, 590, false, &rx_frame);
    }
    TEST_ASSERT_EQUAL(SYN_IR_STATE_IDLE, decoder.state);

    /* Timeout decode while in DATA state */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 9000, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 4500, false, &rx_frame);
    /* Receive 32 valid NEC pulses */
    for (int i = 0; i < 32; i++) {
        syn_ir_decode_pulse(&decoder, 560, true, &rx_frame);
        uint16_t sp = (i == 0 || i == 8 || i == 16 || i == 24) ? 1690 : 560;
        syn_ir_decode_pulse(&decoder, sp, false, &rx_frame);
    }
    /* Timeout decode while in DATA state with Sony 12-bit frame */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 2400, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 600, false, &rx_frame);
    for (int i = 0; i < 12; i++) {
        syn_ir_decode_pulse(&decoder, 600, true, &rx_frame);
        syn_ir_decode_pulse(&decoder, 600, false, &rx_frame);
    }
    TEST_ASSERT_TRUE(syn_ir_decode_timeout(&decoder, &rx_frame));

    /* Denon start bit with one_space_us */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 310, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 1780, false, &rx_frame);
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_DENON, decoder.active_proto);

    /* Manchester full-bit transition (1778us) */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 889, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 889, false, &rx_frame);
    syn_ir_decode_pulse(&decoder, 1778, true, &rx_frame);

    /* Forced invalid active_proto in decode_pulse to hit default case (line 362) */
    syn_ir_decoder_init(&decoder);
    decoder.state = SYN_IR_STATE_DATA;
    decoder.active_proto = (SYN_IR_Protocol)99;
    TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, 500, true, &rx_frame));

    /* Data pulse error reset in active protocol (line 402) */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 9000, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 4500, false, &rx_frame); /* NEC header matched */
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_NEC, decoder.active_proto);
    /* Send invalid pulse length in DATA state -> line 402 reset_decoder_state */
    TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, 50000, true, &rx_frame));

    /* Encode frame with invalid/unknown protocol (SYN_IR_PROTO_UNKNOWN) */
    SYN_IR_Pulse pulse_buf[100];
    size_t pulse_count = 0;
    SYN_IR_Frame invalid_frame = {.protocol = SYN_IR_PROTO_UNKNOWN};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ir_encode_frame(&invalid_frame, pulse_buf, 100, &pulse_count));
}

static void test_r5_ir_sony_single_frame_and_manchester_roundtrip(void)
{
    SYN_IR_Decoder decoder;
    SYN_IR_Frame rx_frame;
    SYN_IR_Pulse pulses[120];
    size_t count = 0U;

    /* 1. Raw hardware Sony-12 capture: 12 marks + 11 600us spaces + timeout (1 frame!) */
    syn_ir_decoder_init(&decoder);
    TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, 2400U, true, &rx_frame));
    TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, 600U, false, &rx_frame));
    /* Command = 0x15 (0010101), Addr = 0x01 (00001) */
    uint16_t s12_raw = (uint16_t)(0x15U | (0x01U << 7U));
    for (uint8_t b = 0U; b < 12U; b++) {
        uint16_t mark = (((s12_raw >> b) & 1U) != 0U) ? 1200U : 600U;
        TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, mark, true, &rx_frame));
        if (b < 11U) {
            TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, 600U, false, &rx_frame));
        }
    }
    TEST_ASSERT_TRUE(syn_ir_decode_timeout(&decoder, &rx_frame));
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_SONY_12, rx_frame.protocol);
    TEST_ASSERT_EQUAL_UINT32(0x01U, rx_frame.address);
    TEST_ASSERT_EQUAL_UINT32(0x15U, rx_frame.command);

    /* 2. Sony-20 fires ONCE (not prematurely at bits 12 and 15) */
    SYN_IR_Frame s20_tx = {.protocol = SYN_IR_PROTO_SONY_20, .address = 0x125U, .command = 0x3AU};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&s20_tx, pulses, 120U, &count));
    syn_ir_decoder_init(&decoder);
    int fire_count = 0;
    for (size_t i = 0U; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            fire_count++;
        }
    }
    TEST_ASSERT_EQUAL_INT(1, fire_count);
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_SONY_20, rx_frame.protocol);
    TEST_ASSERT_EQUAL_UINT32(0x125U, rx_frame.address);
    TEST_ASSERT_EQUAL_UINT32(0x3AU, rx_frame.command);

    /* 3. RC5 round-trip with toggle=0, toggle=1, ending on mark (cmd=1), long final space, timeout
     */
    for (int t = 0; t <= 1; t++) {
        SYN_IR_Frame rc5_tx = {.protocol = SYN_IR_PROTO_RC5,
                               .address = 0x1AU,
                               .command = (t == 0) ? 0x2DU : 0x0CU,
                               .flags = (t != 0) ? SYN_IR_FLAG_TOGGLE : SYN_IR_FLAG_NONE};
        TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&rc5_tx, pulses, 120U, &count));
        syn_ir_decoder_init(&decoder);
        bool ok = false;
        for (size_t i = 0U; i < count; i++) {
            if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark,
                                    &rx_frame)) {
                ok = true;
            }
        }
        TEST_ASSERT_TRUE(ok);
        TEST_ASSERT_EQUAL(SYN_IR_PROTO_RC5, rx_frame.protocol);
        TEST_ASSERT_EQUAL_UINT32(rc5_tx.address, rx_frame.address);
        TEST_ASSERT_EQUAL_UINT32(rc5_tx.command, rx_frame.command);
        TEST_ASSERT_EQUAL_UINT8(rc5_tx.flags, rx_frame.flags & SYN_IR_FLAG_TOGGLE);
    }

    /* RC5 ending on space (cmd=0x0C): test merged >3T idle space AND syn_ir_decode_timeout */
    SYN_IR_Frame rc5_even = {.protocol = SYN_IR_PROTO_RC5, .address = 0x03U, .command = 0x0CU};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&rc5_even, pulses, 120U, &count));
    syn_ir_decoder_init(&decoder);
    for (size_t i = 0U; i + 1U < count; i++) {
        TEST_ASSERT_FALSE(
            syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame));
    }
    TEST_ASSERT_TRUE(syn_ir_decode_pulse(&decoder, 5000U, false, &rx_frame));
    TEST_ASSERT_EQUAL_UINT32(0x0CU, rx_frame.command);

    syn_ir_decoder_init(&decoder);
    for (size_t i = 0U; i + 1U < count; i++) {
        TEST_ASSERT_FALSE(
            syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame));
    }
    TEST_ASSERT_TRUE(syn_ir_decode_timeout(&decoder, &rx_frame));
    TEST_ASSERT_EQUAL_UINT32(0x0CU, rx_frame.command);

    /* 4. RC6 round-trip with toggle=0 and toggle=1, ending on mark (cmd=0x10) & space (cmd=0x11) */
    for (int t = 0; t <= 1; t++) {
        SYN_IR_Frame rc6_tx = {.protocol = SYN_IR_PROTO_RC6,
                               .address = 0xA5U,
                               .command = (t == 0) ? 0x10U : 0x5BU,
                               .flags = (t != 0) ? SYN_IR_FLAG_TOGGLE : SYN_IR_FLAG_NONE};
        TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&rc6_tx, pulses, 120U, &count));
        syn_ir_decoder_init(&decoder);
        bool ok = false;
        for (size_t i = 0U; i < count; i++) {
            if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark,
                                    &rx_frame)) {
                ok = true;
            }
        }
        TEST_ASSERT_TRUE(ok);
        TEST_ASSERT_EQUAL(SYN_IR_PROTO_RC6, rx_frame.protocol);
        TEST_ASSERT_EQUAL_UINT32(rc6_tx.address, rx_frame.address);
        TEST_ASSERT_EQUAL_UINT32(rc6_tx.command, rx_frame.command);
        TEST_ASSERT_EQUAL_UINT8(rc6_tx.flags, rx_frame.flags & SYN_IR_FLAG_TOGGLE);
    }

    /* 5. Partial-frame timeouts & Manchester/Sony framing error guards */
    /* Partial NEC timeout (bit_idx != 32) */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 9000U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 4500U, false, &rx_frame);
    syn_ir_decode_pulse(&decoder, 560U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 560U, false, &rx_frame);
    TEST_ASSERT_FALSE(syn_ir_decode_timeout(&decoder, &rx_frame));

    /* Partial Sony space error (bit_idx == 1) & partial Sony timeout */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 2400U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 600U, false, &rx_frame);
    syn_ir_decode_pulse(&decoder, 600U, true, &rx_frame);
    TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, 2000U, false, &rx_frame));

    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 2400U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 600U, false, &rx_frame);
    syn_ir_decode_pulse(&decoder, 600U, true, &rx_frame);
    TEST_ASSERT_FALSE(syn_ir_decode_timeout(&decoder, &rx_frame));

    /* Partial RC5 timeout (bit_idx != 14) & Manchester framing violation + bad duration */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 889U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 889U, false, &rx_frame);
    TEST_ASSERT_FALSE(syn_ir_decode_timeout(&decoder, &rx_frame));

    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 889U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 889U, false, &rx_frame); /* S1=1 decoded, S2 phase=1 (space) */
    /* 1778us space violates bi-phase (space followed by space) and tests loop state exit */
    TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, 1778U, false, &rx_frame));

    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 889U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 889U, false, &rx_frame);
    TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, 9000U, true, &rx_frame));

    /* Partial RC6 timeout & RC6 2T Toggle half-bit mismatch (1T instead of 2T at bit_idx == 4) */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 2666U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 889U, false, &rx_frame);
    syn_ir_decode_pulse(&decoder, 444U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 888U, false, &rx_frame); /* S=1 + M2 first half */
    TEST_ASSERT_FALSE(syn_ir_decode_timeout(&decoder, &rx_frame));

    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 2666U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 889U, false, &rx_frame);
    /* Start=1 (444M,444S), M2=0 (444S,444M), M1=0 (444S,444M), M0=0 (444S,444M) -> bit_idx == 4 */
    syn_ir_decode_pulse(&decoder, 444U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 888U, false, &rx_frame);
    syn_ir_decode_pulse(&decoder, 444U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 444U, false, &rx_frame);
    syn_ir_decode_pulse(&decoder, 444U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 444U, false, &rx_frame);
    syn_ir_decode_pulse(&decoder, 444U, true, &rx_frame);
    /* Now bit_idx == 4 (RC6 2T toggle): feed 1T mark then 1T space -> prev_mark != is_mark */
    syn_ir_decode_pulse(&decoder, 444U, true, &rx_frame);
    TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, 444U, false, &rx_frame));
    TEST_ASSERT_EQUAL(SYN_IR_STATE_IDLE, decoder.state);

    /* Partial Kaseikyo & Denon timeouts */
    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 3456U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 1728U, false, &rx_frame);
    syn_ir_decode_pulse(&decoder, 432U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 432U, false, &rx_frame);
    TEST_ASSERT_FALSE(syn_ir_decode_timeout(&decoder, &rx_frame));

    syn_ir_decoder_init(&decoder);
    syn_ir_decode_pulse(&decoder, 310U, true, &rx_frame);
    syn_ir_decode_pulse(&decoder, 780U, false, &rx_frame);
    TEST_ASSERT_FALSE(syn_ir_decode_timeout(&decoder, &rx_frame));
}

static void test_tier_a_ir_rc6_mode_repeat_and_orphan_space(void)
{
    SYN_IR_Decoder decoder;
    SYN_IR_Frame rx_frame;
    SYN_IR_Pulse pulses[150];
    size_t count = 0U;

    /* 1. Orphan space pulse at idle (last_mark_us == 0) and stale mark reuse prevention */
    syn_ir_decoder_init(&decoder);
    TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, 4500U, false, &rx_frame));

    syn_ir_decode_pulse(&decoder, 9000U, true, &rx_frame);
    TEST_ASSERT_FALSE(
        syn_ir_decode_pulse(&decoder, 100U, false, &rx_frame)); /* Consumes 9000us mark */
    TEST_ASSERT_EQUAL_UINT16(0U, decoder.last_mark_us);
    TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, 4500U, false, &rx_frame)); /* Ignored! */
    TEST_ASSERT_EQUAL(SYN_IR_STATE_IDLE, decoder.state);

    /* 2. NEC repeat pulse is ONLY accepted after NEC-family frames (NEC, NEC_EXT, APPLE) */
    SYN_IR_Frame sony_tx = {.protocol = SYN_IR_PROTO_SONY_12, .address = 0x01U, .command = 0x15U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&sony_tx, pulses, 150U, &count));
    syn_ir_decoder_init(&decoder);
    bool got = false;
    for (size_t i = 0U; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            got = true;
        }
    }
    TEST_ASSERT_TRUE(got);
    /* NEC repeat (9000us mark + 2250us space) after Sony must NOT repeat Sony! */
    syn_ir_decode_pulse(&decoder, 9000U, true, &rx_frame);
    TEST_ASSERT_FALSE(syn_ir_decode_pulse(&decoder, 2250U, false, &rx_frame));

    /* NEC_EXTENDED and APPLE DO accept NEC repeat pulses */
    SYN_IR_Frame nec_ext_tx = {
        .protocol = SYN_IR_PROTO_NEC_EXTENDED, .address = 0x1234U, .command = 0x56U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&nec_ext_tx, pulses, 150U, &count));
    syn_ir_decoder_init(&decoder);
    got = false;
    for (size_t i = 0U; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            got = true;
        }
    }
    TEST_ASSERT_TRUE(got);
    syn_ir_decode_pulse(&decoder, 9000U, true, &rx_frame);
    TEST_ASSERT_TRUE(syn_ir_decode_pulse(&decoder, 2250U, false, &rx_frame));
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_NEC_EXTENDED, rx_frame.protocol);
    TEST_ASSERT_TRUE((rx_frame.flags & SYN_IR_FLAG_REPEAT) != 0U);

    SYN_IR_Frame apple_tx = {.protocol = SYN_IR_PROTO_APPLE, .address = 0xEE87U, .command = 0x0BU};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ir_encode_frame(&apple_tx, pulses, 150U, &count));
    syn_ir_decoder_init(&decoder);
    got = false;
    for (size_t i = 0U; i < count; i++) {
        if (syn_ir_decode_pulse(&decoder, pulses[i].duration_us, pulses[i].is_mark, &rx_frame)) {
            got = true;
        }
    }
    TEST_ASSERT_TRUE(got);
    syn_ir_decode_pulse(&decoder, 9000U, true, &rx_frame);
    TEST_ASSERT_TRUE(syn_ir_decode_pulse(&decoder, 2250U, false, &rx_frame));
    TEST_ASSERT_EQUAL(SYN_IR_PROTO_APPLE, rx_frame.protocol);

    /* 3. RC6 Mode != 0 (e.g. Mode 6) or Start bit == 0 must be rejected in unpack_rc6 */
    syn_ir_decoder_init(&decoder);
    decoder.state = SYN_IR_STATE_DATA;
    decoder.active_proto = SYN_IR_PROTO_RC6;
    decoder.bit_idx = 21U;
    decoder.bits = (1ULL << 20U) | (6ULL << 17U) | 0x0411ULL; /* Mode 6 != 0 */
    TEST_ASSERT_FALSE(syn_ir_decode_timeout(&decoder, &rx_frame));

    syn_ir_decoder_init(&decoder);
    decoder.state = SYN_IR_STATE_DATA;
    decoder.active_proto = SYN_IR_PROTO_RC6;
    decoder.bit_idx = 21U;
    decoder.bits = 0x0411ULL; /* Start bit 20 == 0 */
    TEST_ASSERT_FALSE(syn_ir_decode_timeout(&decoder, &rx_frame));
}

void run_ir_tests(void)
{
    RUN_TEST(test_ir_nec_decode_encode);
    RUN_TEST(test_ir_sony_sircs_decode);
    RUN_TEST(test_ir_rc5_rc6_samsung_kaseikyo_denon);
    RUN_TEST(test_ir_edge_cases);
    RUN_TEST(test_r5_ir_sony_single_frame_and_manchester_roundtrip);
    RUN_TEST(test_tier_a_ir_rc6_mode_repeat_and_orphan_space);
}

#endif /* SYN_USE_IR */
