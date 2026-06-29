/**
 * @file test_can.c
 * @brief Unity tests for syn_can.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/drivers/syn_can.h"

static int can_rx_n = 0;
static void can_rxcb(const SYN_CAN_Frame *f, void *c) { (void)f; (void)c; can_rx_n++; }

static void test_can(void)
{
    SYN_CAN can;
    TEST_ASSERT_EQUAL(SYN_OK, syn_can_init(&can, 0, 500000));

    mock_can_tx_ok = true;
    SYN_CAN_Frame tx = { .id = 0x100, .dlc = 2, .data = {0} };
    tx.data[0] = 0x42;
    TEST_ASSERT_TRUE(syn_can_send(&can, &tx));
    TEST_ASSERT_EQUAL_INT(1, can.tx_count);

    mock_can_tx_ok = false;
    TEST_ASSERT_FALSE(syn_can_send(&can, &tx));
    TEST_ASSERT_EQUAL_INT(1, can.err_count);

    can_rx_n = 0;
    syn_can_on_receive(&can, can_rxcb, NULL);
    mock_can_rx.id = 0x200;
    mock_can_rx.dlc = 1;
    mock_can_rx.data[0] = 0xAA;
    mock_can_rx_avail = true;
    syn_can_poll(&can);
    TEST_ASSERT_EQUAL_INT(1, can_rx_n);
    TEST_ASSERT_EQUAL_INT(1, can.rx_count);

    syn_can_poll(&can);
    TEST_ASSERT_EQUAL_INT(1, can_rx_n);

    syn_can_set_filter(&can, 0x100, 0x7FF);
}

void run_can_tests(void)
{
    RUN_TEST(test_can);
}
