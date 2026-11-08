/**
 * @file test_can.c
 * @brief Unity tests for syn_can.
 */

#include "mocks/mock_port.h"
#include "syntropic/drivers/syn_can.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

#include <string.h>

static int can_rx_n = 0;
static void can_rxcb(const SYN_CAN_Frame *f, void *c)
{
    (void)f;
    (void)c;
    can_rx_n++;
}

static void test_can(void)
{
    SYN_CAN can;
    TEST_ASSERT_EQUAL(SYN_OK, syn_can_init(&can, 0, 500000));

    mock_can_tx_ok = true;
    SYN_CAN_Frame tx;
    memset(&tx, 0, sizeof(tx));
    tx.id = 0x100U;
    tx.dlc = 2U;
    tx.data[0] = 0x42U;
    TEST_ASSERT_TRUE(syn_can_send(&can, &tx));
    TEST_ASSERT_EQUAL_INT(1, can.tx_count);

    mock_can_tx_ok = false;
    TEST_ASSERT_FALSE(syn_can_send(&can, &tx));
    TEST_ASSERT_EQUAL_INT(1, can.err_count);

    can_rx_n = 0;
    syn_can_on_receive(&can, can_rxcb, NULL);
    mock_can_rx.id = 0x200U;
    mock_can_rx.dlc = 1U;
    mock_can_rx.data[0] = 0xAAU;
    mock_can_rx_avail = true;
    syn_can_poll(&can);
    TEST_ASSERT_EQUAL_INT(1, can_rx_n);
    TEST_ASSERT_EQUAL_INT(1, can.rx_count);

    syn_can_poll(&can);
    TEST_ASSERT_EQUAL_INT(1, can_rx_n);

    /* Poll with NULL on_rx callback */
    syn_can_on_receive(&can, NULL, NULL);
    mock_can_rx.id = 0x300U;
    mock_can_rx.dlc = 1U;
    mock_can_rx_avail = true;
    syn_can_poll(&can);
    TEST_ASSERT_EQUAL_INT(2, can.rx_count);

    syn_can_set_filter(&can, 0x100U, 0x7FFU);
}

/** syn_can_init failure — exercises line 26 (syn_port_can_init returns false) */
static void test_can_init_fail(void)
{
    SYN_CAN can;
    mock_can_init_fail = true;
    SYN_Status st = syn_can_init(&can, 0, 500000);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
}

static void test_r5_a03_can_fd_64b_send(void)
{
    SYN_CAN can;
    mock_can_init_fail = false;
    TEST_ASSERT_EQUAL(SYN_OK, syn_can_init(&can, 0, 1000000));

    mock_can_tx_ok = true;
    SYN_CAN_Frame fd_tx;
    memset(&fd_tx, 0, sizeof(fd_tx));
    fd_tx.id = 0x18DA10F1U;
    fd_tx.extended = true;
    fd_tx.dlc = SYN_CAN_MAX_DATA_LEN;
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    fd_tx.is_fd = true;
    fd_tx.brs = true;
    TEST_ASSERT_EQUAL_UINT8(64U, SYN_CAN_MAX_DATA_LEN);
#endif
    for (uint8_t i = 0U; i < SYN_CAN_MAX_DATA_LEN; i++) {
        fd_tx.data[i] = i;
    }

    TEST_ASSERT_TRUE(syn_can_send(&can, &fd_tx));
    TEST_ASSERT_EQUAL_UINT32(1U, can.tx_count);

    mock_can_rx = fd_tx;
    mock_can_rx_avail = true;
    syn_can_poll(&can);
    TEST_ASSERT_EQUAL_UINT32(1U, can.rx_count);
}

void run_can_tests(void)
{
    RUN_TEST(test_can);
    RUN_TEST(test_can_init_fail);
    RUN_TEST(test_r5_a03_can_fd_64b_send);
}
