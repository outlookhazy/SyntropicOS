/**
 * @file test_spi_queue.c
 * @brief Unity unit tests for asynchronous SPI transaction queue driver (syn_spi_queue).
 */

#include "mocks/mock_port.h"
#include "syntropic/drivers/syn_spi_queue.h"
#include "unity/unity.h"

#include <string.h>

static bool callback_executed = false;
static SYN_Status last_callback_status = SYN_ERROR;
static void *last_user_data = NULL;

static uint8_t last_callback_bus = 0xFFU;

static void test_spi_cb(uint8_t bus, SYN_Status result, void *user_data)
{
    last_callback_bus = bus;
    callback_executed = true;
    last_callback_status = result;
    last_user_data = user_data;
}

static void test_spi_queue_null_and_init(void)
{
    SYN_SPI_Queue q;
    SYN_SPI_Transaction tx = {0};

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_queue_init(NULL, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_queue_enqueue(NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_queue_process(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_queue_cancel_all(NULL));
    TEST_ASSERT_EQUAL_UINT(0, syn_spi_queue_count(NULL));

    /* Uninitialized handle checks */
    memset(&q, 0, sizeof(q));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_queue_enqueue(&q, &tx));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_queue_process(&q));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_queue_cancel_all(&q));
    TEST_ASSERT_EQUAL_UINT(0, syn_spi_queue_count(&q));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_init(&q, 1));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_queue_enqueue(&q, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_process(&q));
    TEST_ASSERT_EQUAL_UINT(0, syn_spi_queue_count(&q));
}

static void test_spi_queue_enqueue_and_process(void)
{
    SYN_SPI_Queue q;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_init(&q, 0));

    uint8_t tx_buf[] = {0x03, 0x00, 0x00};
    uint8_t rx_buf[3] = {0};
    int dummy_ctx = 99;

    SYN_SPI_Transaction tx = {
        .bus = 0,
        .cs_pin = 5,
        .mode = SYN_SPI_MODE_0,
        .baudrate_hz = 1000000,
        .keep_cs_active = false,
        .tx_data = tx_buf,
        .rx_data = rx_buf,
        .len = sizeof(tx_buf),
        .callback = test_spi_cb,
        .user_data = &dummy_ctx,
    };

    callback_executed = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_enqueue(&q, &tx));
    TEST_ASSERT_TRUE(callback_executed);
    TEST_ASSERT_EQUAL_INT(SYN_OK, last_callback_status);
    TEST_ASSERT_EQUAL_PTR(&dummy_ctx, last_user_data);
    TEST_ASSERT_EQUAL_UINT(0, syn_spi_queue_count(&q));
}

static void test_spi_queue_keep_cs_and_overflow(void)
{
    SYN_SPI_Queue q;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_init(&q, 0));

    SYN_SPI_Transaction tx = {
        .bus = 0,
        .cs_pin = 4,
        .keep_cs_active = true,
        .callback = NULL,
    };

    q.active = true;

    for (int i = 0; i < SYN_SPI_QUEUE_MAX_DEPTH; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_enqueue(&q, &tx));
    }
    TEST_ASSERT_EQUAL_UINT(SYN_SPI_QUEUE_MAX_DEPTH, syn_spi_queue_count(&q));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_spi_queue_enqueue(&q, &tx));

    /* Test recursive queue drain */
    q.active = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_process(&q));
    TEST_ASSERT_EQUAL_UINT(0, syn_spi_queue_count(&q));

    /* Test cancel all on fresh queue */
    for (int i = 0; i < 3; i++) {
        q.active = true;
        (void)syn_spi_queue_enqueue(&q, &tx);
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_cancel_all(&q));
    TEST_ASSERT_EQUAL_UINT(0, syn_spi_queue_count(&q));
}

static void test_r5_spi_queue_per_txn_bus_mode_baud(void)
{
    mock_port_reset();
    SYN_SPI_Queue q;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_init(&q, 0));

    /* Reject out-of-range SPI mode (> 3) */
    SYN_SPI_Transaction bad_mode = {.mode = (SYN_SPI_Mode)4U};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_queue_enqueue(&q, &bad_mode));

    /* Per-transaction bus = 2, mode = MODE_3, baudrate_hz = 8000000 */
    uint8_t payload[2] = {0x9FU, 0x00U};
    SYN_SPI_Transaction tx_bus2 = {
        .bus = 2U,
        .cs_pin = 7U,
        .mode = SYN_SPI_MODE_3,
        .baudrate_hz = 8000000U,
        .keep_cs_active = false,
        .tx_data = payload,
        .rx_data = NULL,
        .len = sizeof(payload),
        .callback = test_spi_cb,
        .user_data = NULL,
    };
    callback_executed = false;
    last_callback_bus = 0xFFU;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_enqueue(&q, &tx_bus2));
    TEST_ASSERT_TRUE(callback_executed);
    TEST_ASSERT_EQUAL_UINT8(2U, last_callback_bus);
    TEST_ASSERT_EQUAL_INT(SYN_OK, last_callback_status);

    /* Chained transaction with identical bus/baud/mode must NOT re-call syn_port_spi_init */
    mock_spi_init_ok = false;
    tx_bus2.keep_cs_active = true;
    callback_executed = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_enqueue(&q, &tx_bus2));
    TEST_ASSERT_TRUE(callback_executed);
    TEST_ASSERT_EQUAL_INT(SYN_OK, last_callback_status);
    TEST_ASSERT_EQUAL_UINT8(SYN_GPIO_LOW, mock_gpio_states[7]);

    tx_bus2.keep_cs_active = false;
    callback_executed = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_enqueue(&q, &tx_bus2));
    TEST_ASSERT_TRUE(callback_executed);
    TEST_ASSERT_EQUAL_INT(SYN_OK, last_callback_status);
    TEST_ASSERT_EQUAL_UINT8(SYN_GPIO_HIGH, mock_gpio_states[7]);

    /* Changing mode or baudrate triggers reconfiguration */
    tx_bus2.mode = SYN_SPI_MODE_0;
    callback_executed = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_enqueue(&q, &tx_bus2));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, last_callback_status);
    tx_bus2.mode = SYN_SPI_MODE_3;
    tx_bus2.baudrate_hz = 4000000U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_enqueue(&q, &tx_bus2));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, last_callback_status);
    mock_spi_init_ok = true;

    /* Bus init failure reports error to callback without toggling CS low */
    mock_gpio_states[9] = SYN_GPIO_HIGH;
    mock_spi_init_ok = false;
    SYN_SPI_Transaction tx_fail = {
        .bus = 1U,
        .cs_pin = 9U,
        .mode = SYN_SPI_MODE_1,
        .baudrate_hz = 0U,
        .callback = test_spi_cb,
    };
    callback_executed = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_queue_enqueue(&q, &tx_fail));
    TEST_ASSERT_TRUE(callback_executed);
    TEST_ASSERT_EQUAL_UINT8(1U, last_callback_bus);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, last_callback_status);
    TEST_ASSERT_EQUAL_UINT8(SYN_GPIO_HIGH, mock_gpio_states[9]);
    mock_spi_init_ok = true;
}

void run_spi_queue_tests(void)
{
    RUN_TEST(test_spi_queue_null_and_init);
    RUN_TEST(test_spi_queue_enqueue_and_process);
    RUN_TEST(test_spi_queue_keep_cs_and_overflow);
    RUN_TEST(test_r5_spi_queue_per_txn_bus_mode_baud);
}
