/**
 * @file test_dmx512.c
 * @brief Unity unit tests for DMX512 Master & Slave Protocol Stack.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/proto/syn_dmx512.h"

static void test_dmx512_master_init_and_channels(void)
{
    SYN_DMX512_Master master;
    syn_dmx512_master_init(&master, 128);

    TEST_ASSERT_EQUAL(128, master.num_channels);
    TEST_ASSERT_EQUAL(SYN_DMX512_START_CODE_NULL, master.start_code);

    TEST_ASSERT_EQUAL(SYN_OK, syn_dmx512_master_set_channel(&master, 1, 255));
    TEST_ASSERT_EQUAL(SYN_OK, syn_dmx512_master_set_channel(&master, 128, 128));

    TEST_ASSERT_EQUAL(255, syn_dmx512_master_get_channel(&master, 1));
    TEST_ASSERT_EQUAL(128, syn_dmx512_master_get_channel(&master, 128));

    /* Error cases */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_dmx512_master_set_channel(NULL, 1, 100));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_dmx512_master_set_channel(&master, 0, 100));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_dmx512_master_set_channel(&master, 129, 100));

    TEST_ASSERT_EQUAL(0, syn_dmx512_master_get_channel(NULL, 1));
    TEST_ASSERT_EQUAL(0, syn_dmx512_master_get_channel(&master, 0));
    TEST_ASSERT_EQUAL(0, syn_dmx512_master_get_channel(&master, 129));

    /* Default max channels fallback */
    syn_dmx512_master_init(&master, 0);
    TEST_ASSERT_EQUAL(SYN_DMX512_MAX_CHANNELS, master.num_channels);

    syn_dmx512_master_init(&master, 600);
    TEST_ASSERT_EQUAL(SYN_DMX512_MAX_CHANNELS, master.num_channels);
}

static void test_dmx512_master_build_frame(void)
{
    SYN_DMX512_Master master;
    syn_dmx512_master_init(&master, 4);

    syn_dmx512_master_set_channel(&master, 1, 0x11);
    syn_dmx512_master_set_channel(&master, 2, 0x22);
    syn_dmx512_master_set_channel(&master, 3, 0x33);
    syn_dmx512_master_set_channel(&master, 4, 0x44);

    uint8_t buf[8];
    TEST_ASSERT_EQUAL(5, syn_dmx512_master_build_frame(&master, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL(0x00, buf[0]);
    TEST_ASSERT_EQUAL(0x11, buf[1]);
    TEST_ASSERT_EQUAL(0x22, buf[2]);
    TEST_ASSERT_EQUAL(0x33, buf[3]);
    TEST_ASSERT_EQUAL(0x44, buf[4]);

    /* Error cases */
    TEST_ASSERT_EQUAL(0, syn_dmx512_master_build_frame(NULL, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL(0, syn_dmx512_master_build_frame(&master, NULL, sizeof(buf)));
    TEST_ASSERT_EQUAL(0, syn_dmx512_master_build_frame(&master, buf, 3));
}

static void test_dmx512_slave_rx_flow(void)
{
    SYN_DMX512_Slave slave;
    /* 4-channel fixture starting at DMX address 10 */
    syn_dmx512_slave_init(&slave, 10, 4);

    TEST_ASSERT_EQUAL(10, slave.start_address);
    TEST_ASSERT_EQUAL(4, slave.footprint);

    /* Signal line Break */
    syn_dmx512_slave_rx_break(&slave);
    TEST_ASSERT_EQUAL(SYN_DMX512_STATE_WAIT_START, slave.rx_state);

    /* Start Code 0x00 */
    syn_dmx512_slave_rx_byte(&slave, 0x00);
    TEST_ASSERT_EQUAL(SYN_DMX512_STATE_READ_SLOTS, slave.rx_state);

    /* Stream slots 1..9 (outside footprint) */
    for (uint8_t i = 1; i <= 9; i++) {
        syn_dmx512_slave_rx_byte(&slave, i);
    }

    /* Stream slots 10..13 (inside footprint) */
    syn_dmx512_slave_rx_byte(&slave, 0xAA);
    syn_dmx512_slave_rx_byte(&slave, 0xBB);
    syn_dmx512_slave_rx_byte(&slave, 0xCC);
    syn_dmx512_slave_rx_byte(&slave, 0xDD);

    /* Verify channel extractions */
    TEST_ASSERT_EQUAL(0xAA, syn_dmx512_slave_get_channel(&slave, 0));
    TEST_ASSERT_EQUAL(0xBB, syn_dmx512_slave_get_channel(&slave, 1));
    TEST_ASSERT_EQUAL(0xCC, syn_dmx512_slave_get_channel(&slave, 2));
    TEST_ASSERT_EQUAL(0xDD, syn_dmx512_slave_get_channel(&slave, 3));

    /* Stream remaining slots up to 512 to complete frame */
    for (uint16_t i = 14; i <= 512; i++) {
        syn_dmx512_slave_rx_byte(&slave, 0);
    }

    TEST_ASSERT_TRUE(syn_dmx512_slave_is_updated(&slave));
    TEST_ASSERT_FALSE(syn_dmx512_slave_is_updated(&slave));

    /* Test Non-zero Start Code rejection */
    syn_dmx512_slave_rx_break(&slave);
    syn_dmx512_slave_rx_byte(&slave, 0xCF); /* Non-dimmer start code */
    TEST_ASSERT_EQUAL(SYN_DMX512_STATE_WAIT_BREAK, slave.rx_state);

    /* Ignored bytes while in WAIT_BREAK */
    syn_dmx512_slave_rx_byte(&slave, 0xFF);
    TEST_ASSERT_EQUAL(SYN_DMX512_STATE_WAIT_BREAK, slave.rx_state);

    /* Error / edge cases */
    TEST_ASSERT_EQUAL(0, syn_dmx512_slave_get_channel(NULL, 0));
    TEST_ASSERT_EQUAL(0, syn_dmx512_slave_get_channel(&slave, 10));
    TEST_ASSERT_FALSE(syn_dmx512_slave_is_updated(NULL));
    syn_dmx512_slave_rx_break(NULL);
    syn_dmx512_slave_rx_byte(NULL, 0);
}

static void test_dmx512_slave_init_bounds(void)
{
    SYN_DMX512_Slave slave;

    /* Underflow bounds */
    syn_dmx512_slave_init(&slave, 0, 0);
    TEST_ASSERT_EQUAL(1, slave.start_address);
    TEST_ASSERT_EQUAL(1, slave.footprint);

    /* Overflow bounds */
    syn_dmx512_slave_init(&slave, 600, 10);
    TEST_ASSERT_EQUAL(SYN_DMX512_MAX_CHANNELS, slave.start_address);
    TEST_ASSERT_EQUAL(1, slave.footprint);

    /* Footprint overflow */
    syn_dmx512_slave_init(&slave, 510, 20);
    TEST_ASSERT_EQUAL(510, slave.start_address);
    TEST_ASSERT_EQUAL(3, slave.footprint);
}

void run_dmx512_tests(void)
{
    RUN_TEST(test_dmx512_master_init_and_channels);
    RUN_TEST(test_dmx512_master_build_frame);
    RUN_TEST(test_dmx512_slave_rx_flow);
    RUN_TEST(test_dmx512_slave_init_bounds);
}
