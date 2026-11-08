/**
 * @file test_iolink.c
 * @brief Complete unit tests for IO-Link (IEC 61131-9) Master & Device Protocol Engine (100%
 * coverage).
 */

#include "mocks/mock_port.h"
#include "unity/unity.h"

#include <string.h>
#include <syntropic/proto/syn_iolink.h>
#include <syntropic/sched/syn_sched.h>

/* ── Loopback Transport between Master and Device ────────────────────────── */

typedef struct {
    SYN_IOLink_Device *dev;
    uint8_t master_tx[128];
    size_t master_tx_len;
    uint8_t master_rx[128];
    size_t master_rx_len;
    bool fail_send;
    bool fail_recv;
    bool corrupt_rx;
    bool short_rx;
    uint32_t recv_count;
    bool fail_second_recv;
} IOLinkLoopbackTransport;

static bool loopback_send(const uint8_t *data, size_t len, void *user_ctx)
{
    IOLinkLoopbackTransport *lb = (IOLinkLoopbackTransport *)user_ctx;
    if (lb->fail_send) {
        return false;
    }
    if (len > sizeof(lb->master_tx)) {
        return false;
    }
    (void)memcpy(lb->master_tx, data, len);
    lb->master_tx_len = len;

    /* Process immediately on device */
    if (lb->dev != NULL) {
        lb->master_rx_len = 0U;
        (void)syn_iolink_device_process_frame(lb->dev, lb->master_tx, lb->master_tx_len,
                                              lb->master_rx, sizeof(lb->master_rx),
                                              &lb->master_rx_len);
        if (lb->corrupt_rx && lb->master_rx_len > 0U) {
            lb->master_rx[lb->master_rx_len - 1U] ^= 0xFFU; /* Corrupt checksum */
        }
    }
    return true;
}

static bool loopback_recv(uint8_t *buf, size_t max_len, size_t *out_len, void *user_ctx)
{
    IOLinkLoopbackTransport *lb = (IOLinkLoopbackTransport *)user_ctx;
    lb->recv_count++;
    if (lb->fail_recv || lb->master_rx_len == 0U ||
        (lb->fail_second_recv && lb->recv_count == 2U)) {
        return false;
    }
    size_t copy_len = (lb->master_rx_len < max_len) ? lb->master_rx_len : max_len;
    if (lb->short_rx && copy_len > 1U) {
        copy_len = 1U;
    }
    (void)memcpy(buf, lb->master_rx, copy_len);
    *out_len = copy_len;
    lb->master_rx_len = 0U;
    return true;
}

/* ── ISDU Callbacks ──────────────────────────────────────────────────────── */

static uint16_t g_device_serial = 12345U;

static SYN_Status on_dev_isdu_read(uint16_t index, uint8_t subindex, uint8_t *out_data,
                                   size_t max_len, size_t *out_len, void *user_data)
{
    (void)user_data;
    if (index == 0x0015U && subindex == 0U && max_len >= 2U) { /* Serial Number Index */
        out_data[0] = (uint8_t)((g_device_serial >> 8U) & 0xFFU);
        out_data[1] = (uint8_t)(g_device_serial & 0xFFU);
        *out_len = 2U;
        return SYN_OK;
    }
    return SYN_ERROR;
}

static SYN_Status on_dev_isdu_write(uint16_t index, uint8_t subindex, const uint8_t *data,
                                    size_t len, void *user_data)
{
    (void)user_data;
    if (index == 0x0015U && subindex == 0U && len >= 2U) {
        g_device_serial = (uint16_t)(((uint16_t)data[0] << 8U) | data[1]);
        return SYN_OK;
    }
    return SYN_ERROR;
}

/* ── Tests ───────────────────────────────────────────────────────────────── */

void test_iolink_checksum_calculation(void)
{
    uint8_t test_data[] = {0x80U, 0x01U, 0x11U};
    uint8_t cks = syn_iolink_calc_checksum(test_data, sizeof(test_data));
    TEST_ASSERT_NOT_EQUAL(0, cks);

    /* Null data handling */
    TEST_ASSERT_EQUAL_HEX8(0, syn_iolink_calc_checksum(NULL, 0));
    TEST_ASSERT_EQUAL_HEX8(0, syn_iolink_calc_checksum(test_data, 0));
}

void test_iolink_master_and_device_startup_handshake(void)
{
    SYN_IOLink_Device dev;
    SYN_IOLink_DeviceConfig dev_cfg = {
        .params =
            {
                .min_cycle_time = 0x18U, /* 2.4 ms */
                .revision_id = SYN_IOLINK_REV_1_1,
                .vendor_id = 0x0123U,
                .device_id = 0x00045678U,
                .pd_in_len = 2U,
                .pd_out_len = 2U,
            },
        .on_read = on_dev_isdu_read,
        .on_write = on_dev_isdu_write,
    };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_device_init(&dev, &dev_cfg));

    /* Null validation */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_iolink_device_init(NULL, &dev_cfg));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_iolink_device_init(&dev, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_iolink_master_init(NULL, NULL));

    IOLinkLoopbackTransport lb;
    (void)memset(&lb, 0, sizeof(lb));
    lb.dev = &dev;

    SYN_Transport transport = {.send = loopback_send, .recv = loopback_recv, .ctx = &lb};

    uint8_t rx[128], tx[128];
    SYN_IOLink_MasterConfig master_cfg = {
        .target_baud = SYN_IOLINK_BAUD_COM3,
        .transport = &transport,
        .cycle_time_ms = 5U,
        .rx_buf = rx,
        .rx_buf_size = sizeof(rx),
        .tx_buf = tx,
        .tx_buf_size = sizeof(tx),
    };

    SYN_IOLink_Master master;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_init(&master, &master_cfg));
    TEST_ASSERT_EQUAL_INT(SYN_IOLINK_PORT_INACTIVE, master.state);

    /* Buffer boundary check */
    master_cfg.rx_buf_size = 16;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_iolink_master_init(&master, &master_cfg));
    master_cfg.rx_buf_size = sizeof(rx);

    /* Start handshake */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_start(&master));
    TEST_ASSERT_EQUAL_INT(SYN_IOLINK_PORT_OPERATE, master.state);
    TEST_ASSERT_EQUAL_HEX16(0x0123, master.dev_params.vendor_id);
    TEST_ASSERT_EQUAL_HEX32(0x00045678, master.dev_params.device_id);
    TEST_ASSERT_EQUAL_HEX8(SYN_IOLINK_REV_1_1, master.dev_params.revision_id);
}

void test_iolink_cyclic_process_data_exchange(void)
{
    SYN_IOLink_Device dev;
    SYN_IOLink_DeviceConfig dev_cfg = {
        .params =
            {
                .min_cycle_time = 0x18U,
                .revision_id = SYN_IOLINK_REV_1_1,
                .vendor_id = 0x0567U,
                .device_id = 0x00112233U,
                .pd_in_len = 2U,
                .pd_out_len = 2U,
            },
    };
    syn_iolink_device_init(&dev, &dev_cfg);

    /* Device getters / setters null checks */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_iolink_device_set_pd_in(NULL, NULL, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_iolink_device_get_pd_out(NULL, NULL, 0, NULL));

    IOLinkLoopbackTransport lb;
    (void)memset(&lb, 0, sizeof(lb));
    lb.dev = &dev;

    SYN_Transport transport = {.send = loopback_send, .recv = loopback_recv, .ctx = &lb};
    uint8_t rx[128], tx[128];
    SYN_IOLink_MasterConfig master_cfg = {
        .transport = &transport,
        .rx_buf = rx,
        .rx_buf_size = sizeof(rx),
        .tx_buf = tx,
        .tx_buf_size = sizeof(tx),
    };

    SYN_IOLink_Master master;
    syn_iolink_master_init(&master, &master_cfg);
    syn_iolink_master_start(&master);

    /* Set device sensor reading (PD_IN) */
    uint8_t sensor_reading[] = {0x04U, 0xD2U}; /* 1234 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_device_set_pd_in(&dev, sensor_reading, 2U));

    /* Master exchanges PD: sends actuator command 0x55, 0xAA */
    uint8_t actuator_cmd[] = {0x55U, 0xAAU};
    uint8_t in_buf[16];
    size_t in_len = 0U;

    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_iolink_master_exchange_pd(&master, actuator_cmd, 2U, in_buf, &in_len));
    TEST_ASSERT_EQUAL_UINT(2, in_len);
    TEST_ASSERT_EQUAL_HEX8(0x04, in_buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0xD2, in_buf[1]);

    /* Verify device received actuator output */
    uint8_t dev_pd_out[16];
    size_t dev_pd_out_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_device_get_pd_out(&dev, dev_pd_out, sizeof(dev_pd_out),
                                                               &dev_pd_out_len));
    TEST_ASSERT_EQUAL_HEX8(0x55, dev_pd_out[0]);
    TEST_ASSERT_EQUAL_HEX8(0xAA, dev_pd_out[1]);
}

void test_iolink_acyclic_isdu_read_write(void)
{
    SYN_IOLink_Device dev;
    SYN_IOLink_DeviceConfig dev_cfg = {
        .params = {.revision_id = SYN_IOLINK_REV_1_1, .pd_in_len = 2, .pd_out_len = 2},
        .on_read = on_dev_isdu_read,
        .on_write = on_dev_isdu_write,
    };
    syn_iolink_device_init(&dev, &dev_cfg);

    IOLinkLoopbackTransport lb;
    (void)memset(&lb, 0, sizeof(lb));
    lb.dev = &dev;

    SYN_Transport transport = {.send = loopback_send, .recv = loopback_recv, .ctx = &lb};
    uint8_t rx[128], tx[128];
    SYN_IOLink_MasterConfig master_cfg = {
        .transport = &transport,
        .rx_buf = rx,
        .rx_buf_size = sizeof(rx),
        .tx_buf = tx,
        .tx_buf_size = sizeof(tx),
    };

    SYN_IOLink_Master master;
    syn_iolink_master_init(&master, &master_cfg);
    syn_iolink_master_start(&master);

    /* Read ISDU */
    uint8_t isdu_data[16];
    size_t isdu_len = 0U;
    g_device_serial = 12345U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_read_isdu(&master, 0x0015U, 0U, isdu_data,
                                                              sizeof(isdu_data), &isdu_len));
    TEST_ASSERT_EQUAL_UINT(2, isdu_len);
    uint16_t read_serial = (uint16_t)(((uint16_t)isdu_data[0] << 8U) | isdu_data[1]);
    TEST_ASSERT_EQUAL_UINT16(12345, read_serial);

    /* Write ISDU */
    uint8_t new_serial_bytes[] = {0x27, 0x10}; /* 10000 */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_iolink_master_write_isdu(&master, 0x0015U, 0U, new_serial_bytes, 2U));
    TEST_ASSERT_EQUAL_UINT16(10000, g_device_serial);
}

void test_iolink_master_errors_and_edge_cases(void)
{
    SYN_IOLink_Device dev;
    SYN_IOLink_DeviceConfig dev_cfg = {
        .params = {.revision_id = SYN_IOLINK_REV_1_1, .pd_in_len = 2, .pd_out_len = 2},
        .on_read = on_dev_isdu_read,
        .on_write = on_dev_isdu_write,
    };
    syn_iolink_device_init(&dev, &dev_cfg);

    IOLinkLoopbackTransport lb;
    (void)memset(&lb, 0, sizeof(lb));
    lb.dev = &dev;

    SYN_Transport transport = {.send = loopback_send, .recv = loopback_recv, .ctx = &lb};
    uint8_t rx[128], tx[128];
    SYN_IOLink_MasterConfig master_cfg = {
        .transport = &transport,
        .rx_buf = rx,
        .rx_buf_size = sizeof(rx),
        .tx_buf = tx,
        .tx_buf_size = sizeof(tx),
    };

    SYN_IOLink_Master master;
    syn_iolink_master_init(&master, &master_cfg);

    /* Null validation */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_iolink_master_start(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_iolink_master_exchange_pd(NULL, NULL, 0, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_iolink_master_read_isdu(NULL, 0, 0, NULL, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_iolink_master_write_isdu(NULL, 0, 0, NULL, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_iolink_master_step(NULL, 0));

    /* Transport failure during start */
    lb.fail_send = true;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_iolink_master_start(&master));
    TEST_ASSERT_EQUAL_INT(SYN_IOLINK_PORT_FAULT, master.state);
    lb.fail_send = false;

    /* Checksum corruption during start */
    lb.corrupt_rx = true;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_iolink_master_start(&master));
    lb.corrupt_rx = false;

    /* Checksum corruption and send failure during PD exchange */
    syn_iolink_master_start(&master);
    lb.corrupt_rx = true;
    uint8_t dummy_in[8] = {0};
    size_t in_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_iolink_master_exchange_pd(&master, rx, 1, dummy_in, &in_len));
    lb.corrupt_rx = false;

    lb.fail_send = true;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_iolink_master_exchange_pd(&master, rx, 1, dummy_in, &in_len));
    lb.fail_send = false;

    /* ISDU Read/Write errors */
    lb.fail_send = true;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_iolink_master_read_isdu(&master, 0x0015U, 0U, dummy_in,
                                                                 sizeof(dummy_in), &in_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_iolink_master_write_isdu(&master, 0x0015U, 0U, dummy_in, 1U));
    lb.fail_send = false;

    lb.fail_recv = true;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_iolink_master_read_isdu(&master, 0x0015U, 0U, dummy_in,
                                                                 sizeof(dummy_in), &in_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_iolink_master_write_isdu(&master, 0x0015U, 0U, dummy_in, 1U));
    lb.fail_recv = false;

    master.state = SYN_IOLINK_PORT_OPERATE;
    lb.corrupt_rx = true;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_iolink_master_read_isdu(&master, 0x0015U, 0U, dummy_in,
                                                                 sizeof(dummy_in), &in_len));
    master.state = SYN_IOLINK_PORT_OPERATE;
    uint8_t write_ser[2] = {0x11, 0x22};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_iolink_master_write_isdu(&master, 0x0015U, 0U, write_ser, 2U));
    lb.corrupt_rx = false;

    /* Short rx (rx_len < 2) tests */
    lb.short_rx = true;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_iolink_master_start(&master));
    master.state = SYN_IOLINK_PORT_OPERATE;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_iolink_master_read_isdu(&master, 0x0015U, 0U, dummy_in,
                                                                 sizeof(dummy_in), &in_len));
    master.state = SYN_IOLINK_PORT_OPERATE;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_iolink_master_write_isdu(&master, 0x0015U, 0U, dummy_in, 1U));
    lb.short_rx = false;

    /* Failure on Revision ID during startup */
    lb.fail_second_recv = true;
    lb.recv_count = 0;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_iolink_master_start(&master));
    lb.fail_second_recv = false;

    /* Device frame error validations */
    uint8_t dev_tx[64];
    size_t dev_tx_len = 0;
    TEST_ASSERT_EQUAL_INT(
        SYN_INVALID_PARAM,
        syn_iolink_device_process_frame(NULL, rx, 10, dev_tx, sizeof(dev_tx), &dev_tx_len));
    TEST_ASSERT_EQUAL_INT(
        SYN_INVALID_PARAM,
        syn_iolink_device_process_frame(&dev, NULL, 10, dev_tx, sizeof(dev_tx), &dev_tx_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_iolink_device_process_frame(
                                                 &dev, rx, 10, NULL, sizeof(dev_tx), &dev_tx_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_iolink_device_process_frame(&dev, rx, 10, dev_tx,
                                                                             sizeof(dev_tx), NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_iolink_device_process_frame(
                                                 &dev, rx, 1, dev_tx, sizeof(dev_tx), &dev_tx_len));
    uint8_t bad_frame[4] = {0x80, 0x01, 0x00, 0x00}; /* Corrupt CRC */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_iolink_device_process_frame(&dev, bad_frame, 4, dev_tx,
                                                                     sizeof(dev_tx), &dev_tx_len));

    /* Direct Parameter Page default address (0x3F) */
    uint8_t unk_param_frame[2] = {0xBF, 0x00}; /* Read address 0x3F */
    unk_param_frame[1] = syn_iolink_calc_checksum(unk_param_frame, 1);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_device_process_frame(&dev, unk_param_frame, 2, dev_tx,
                                                                  sizeof(dev_tx), &dev_tx_len));

    /* PD exchange with max_tx too small */
    uint8_t pd_frame[3] = {0x00, 0x11, 0x00};
    pd_frame[2] = syn_iolink_calc_checksum(pd_frame, 2);
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_iolink_device_process_frame(&dev, pd_frame, 3, dev_tx, 1, &dev_tx_len));
}

void test_iolink_master_step_and_protothread(void)
{
    IOLinkLoopbackTransport lb;
    (void)memset(&lb, 0, sizeof(lb));
    SYN_Transport transport = {.send = loopback_send, .recv = loopback_recv, .ctx = &lb};

    uint8_t rx[64], tx[64];
    SYN_IOLink_MasterConfig master_cfg = {
        .transport = &transport,
        .cycle_time_ms = 10U,
        .rx_buf = rx,
        .rx_buf_size = sizeof(rx),
        .tx_buf = tx,
        .tx_buf_size = sizeof(tx),
    };

    SYN_IOLink_Master master;
    syn_iolink_master_init(&master, &master_cfg);
    master.state = SYN_IOLINK_PORT_OPERATE;

    /* Step initial (hits last_cycle_ms == 0 with small now_ms) and non-periodic */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_step(&master, 5U));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_step(&master, 6U));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_step(&master, 120U));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &master};

    SYN_PT_Status st = syn_iolink_master_pt(&pt, &task);
    TEST_ASSERT_EQUAL_INT(PT_IDLE, st);

    /* Null checks */
    TEST_ASSERT_EQUAL_INT(PT_EXITED, syn_iolink_master_pt(NULL, &task));
    TEST_ASSERT_EQUAL_INT(PT_EXITED, syn_iolink_master_pt(&pt, NULL));
    SYN_Task null_task = {.user_data = NULL};
    TEST_ASSERT_EQUAL_INT(PT_EXITED, syn_iolink_master_pt(&pt, &null_task));

    /* Step error termination */
    master.cfg.transport = NULL;
    st = syn_iolink_master_pt(&pt, &task);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, st);
}

typedef struct {
    IOLinkLoopbackTransport base;
    bool has_data_flag;
    bool ready_after_first_query;
    uint32_t has_data_calls;
} AsyncIOLinkTransport;

static uint32_t s_async_iolink_has_data_calls = 0U;

static bool async_iolink_has_data(const void *user_ctx)
{
    const AsyncIOLinkTransport *a = (const AsyncIOLinkTransport *)user_ctx;
    if (a == NULL) {
        return false;
    }
    s_async_iolink_has_data_calls++;
    if (a->ready_after_first_query) {
        return (s_async_iolink_has_data_calls >= 2U);
    }
    return a->has_data_flag;
}

void test_iolink_async_split_tx_rx_timeout_fault_and_pt_yielding(void)
{
    mock_port_reset();

    SYN_IOLink_Device dev;
    SYN_IOLink_DeviceConfig dev_cfg = {
        .params =
            {
                .min_cycle_time = 0x18U,
                .revision_id = SYN_IOLINK_REV_1_1,
                .vendor_id = 0x0123U,
                .device_id = 0x00045678U,
                .pd_in_len = 2U,
                .pd_out_len = 2U,
            },
        .on_read = on_dev_isdu_read,
        .on_write = on_dev_isdu_write,
    };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_device_init(&dev, &dev_cfg));
    uint8_t pd_in_vals[2] = {0xABU, 0xCDU};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_device_set_pd_in(&dev, pd_in_vals, 2U));

    AsyncIOLinkTransport async_lb;
    (void)memset(&async_lb, 0, sizeof(async_lb));
    async_lb.base.dev = &dev;

    SYN_Transport tr = {
        .send = loopback_send,
        .recv = loopback_recv,
        .has_data = async_iolink_has_data,
        .ctx = &async_lb,
    };

    uint8_t rx[64], tx[64];
    SYN_IOLink_MasterConfig mcfg = {
        .transport = &tr,
        .cycle_time_ms = 10U,
        .rx_buf = rx,
        .rx_buf_size = sizeof(rx),
        .tx_buf = tx,
        .tx_buf_size = sizeof(tx),
    };

    SYN_IOLink_Master master;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_init(&master, &mcfg));
    async_lb.has_data_flag = true;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_start(&master));
    TEST_ASSERT_EQUAL_INT(SYN_IOLINK_PORT_OPERATE, master.state);

    /* 1. Step while in PORT_FAULT returns SYN_OK without TX (syn_iolink.c:331-333) */
    master.state = SYN_IOLINK_PORT_FAULT;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_step(&master, 10U));

    /* 2. TX send failure in syn_iolink_master_step transitions to PORT_FAULT (syn_iolink.c:354-358)
     */
    master.state = SYN_IOLINK_PORT_OPERATE;
    master.last_cycle_ms = 0U;
    async_lb.base.fail_send = true;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_step(&master, 10U));
    TEST_ASSERT_EQUAL_INT(SYN_IOLINK_PORT_FAULT, master.state);
    async_lb.base.fail_send = false;

    /* 3. Async split TX/RX cycle: has_data == false on TX sets awaiting_rx + rx_deadline_ms
     *    (syn_iolink.c:335-348, 360-366) */
    master.state = SYN_IOLINK_PORT_OPERATE;
    master.last_cycle_ms = 0U;
    async_lb.has_data_flag = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_step(&master, 100U));
    TEST_ASSERT_TRUE(master.awaiting_rx);
    TEST_ASSERT_EQUAL_UINT32(110U, master.rx_deadline_ms);

    /* Before deadline while has_data == false: stays awaiting_rx in OPERATE */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_step(&master, 105U));
    TEST_ASSERT_TRUE(master.awaiting_rx);
    TEST_ASSERT_EQUAL_INT(SYN_IOLINK_PORT_OPERATE, master.state);

    /* Response arrives (has_data == true): completes PD exchange and clears awaiting_rx */
    async_lb.has_data_flag = true;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_step(&master, 106U));
    TEST_ASSERT_FALSE(master.awaiting_rx);
    TEST_ASSERT_EQUAL_INT(SYN_IOLINK_PORT_OPERATE, master.state);
    TEST_ASSERT_EQUAL_HEX8(0xABU, master.current_pd_in[0]);
    TEST_ASSERT_EQUAL_HEX8(0xCDU, master.current_pd_in[1]);

    /* 4. Async RX recv error while awaiting_rx transitions to PORT_FAULT (syn_iolink.c:345-347) */
    async_lb.has_data_flag = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_step(&master, 120U));
    TEST_ASSERT_TRUE(master.awaiting_rx);
    async_lb.has_data_flag = true;
    async_lb.base.fail_recv = true;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_step(&master, 122U));
    TEST_ASSERT_FALSE(master.awaiting_rx);
    TEST_ASSERT_EQUAL_INT(SYN_IOLINK_PORT_FAULT, master.state);
    async_lb.base.fail_recv = false;

    /* 5. Async RX timeout when has_data stays false past rx_deadline_ms (syn_iolink.c:338-341) */
    master.state = SYN_IOLINK_PORT_OPERATE;
    master.cfg.cycle_time_ms = 0U; /* Also exercises SYN_IOLINK_DEFAULT_CYCLE_MS fallback (362) */
    master.last_cycle_ms = 0U;
    async_lb.has_data_flag = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_step(&master, 200U));
    TEST_ASSERT_TRUE(master.awaiting_rx);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_step(&master, master.rx_deadline_ms));
    TEST_ASSERT_FALSE(master.awaiting_rx);
    TEST_ASSERT_EQUAL_INT(SYN_IOLINK_PORT_FAULT, master.state);

    /* 6. syn_iolink_master_pt async PT_CONTINUE_IF_NEEDED (syn_iolink.c:391-395):
     *    call #1 to has_data (in step) returns false -> awaiting_rx = true;
     *    call #2 to has_data (in pt) returns true -> yields PT_MORE, next step finishes RX */
    master.state = SYN_IOLINK_PORT_OPERATE;
    master.cfg.cycle_time_ms = 10U;
    master.last_cycle_ms = 0U;
    master.awaiting_rx = false;
    mock_tick_ms = 500U;
    async_lb.ready_after_first_query = true;
    s_async_iolink_has_data_calls = 0U;

    SYN_Sched sched = {0};
    SYN_Task task = {0};
    task.user_data = &master;
    task.sched = &sched;
    sched.running = &task;
    sched.share_contended = true;

    SYN_PT pt;
    PT_INIT(&pt);

    TEST_ASSERT_EQUAL_INT(PT_MORE, syn_iolink_master_pt(&pt, &task));
    TEST_ASSERT_TRUE(master.awaiting_rx);

    TEST_ASSERT_EQUAL_INT(PT_IDLE, syn_iolink_master_pt(&pt, &task));
    TEST_ASSERT_FALSE(master.awaiting_rx);
    TEST_ASSERT_EQUAL_INT(SYN_IOLINK_PORT_OPERATE, master.state);

    /* 7. Device ISDU read/write callback error fallback (syn_iolink.c:550) */
    async_lb.ready_after_first_query = false;
    async_lb.has_data_flag = true;
    size_t out_len = 0U;
    uint8_t isdu_buf[8] = {0};
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_iolink_master_read_isdu(&master, 0x9999U, 0U, isdu_buf, 8U, &out_len));
    master.state = SYN_IOLINK_PORT_OPERATE;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_iolink_master_write_isdu(&master, 0x9999U, 0U, isdu_buf, 2U));
}

typedef struct {
    uint8_t raw_rx[64];
    size_t raw_rx_len;
    bool has_data;
} RawFrameTransport;

static bool raw_frame_send(const uint8_t *data, size_t len, void *user_ctx)
{
    (void)data;
    (void)len;
    (void)user_ctx;
    return true;
}

static bool raw_frame_recv(uint8_t *buf, size_t max_len, size_t *out_len, void *user_ctx)
{
    RawFrameTransport *rt = (RawFrameTransport *)user_ctx;
    if (rt->raw_rx_len == 0U || rt->raw_rx_len > max_len) {
        return false;
    }
    (void)memcpy(buf, rt->raw_rx, rt->raw_rx_len);
    *out_len = rt->raw_rx_len;
    return true;
}

static bool raw_frame_has_data(const void *user_ctx)
{
    const RawFrameTransport *rt = (const RawFrameTransport *)user_ctx;
    return rt->has_data;
}

void test_iolink_pd_bounds_stale_awaiting_rx_and_deadline_clamp(void)
{
    RawFrameTransport rt;
    (void)memset(&rt, 0, sizeof(rt));

    SYN_Transport tr = {
        .send = raw_frame_send,
        .recv = raw_frame_recv,
        .has_data = raw_frame_has_data,
        .ctx = &rt,
    };

    uint8_t rx[64], tx[64];
    SYN_IOLink_MasterConfig mcfg = {
        .transport = &tr,
        .cycle_time_ms = 5U,
        .rx_buf = rx,
        .rx_buf_size = sizeof(rx),
        .tx_buf = tx,
        .tx_buf_size = sizeof(tx),
    };

    SYN_IOLink_Master master;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_init(&master, &mcfg));
    master.state = SYN_IOLINK_PORT_OPERATE;

    /* 1. Exact max PD length (32 payload + 1 checksum = 33 bytes) succeeds */
    for (size_t i = 0U; i < SYN_IOLINK_MAX_PD_LEN; i++) {
        rt.raw_rx[i] = (uint8_t)(i + 1U);
    }
    rt.raw_rx[SYN_IOLINK_MAX_PD_LEN] = syn_iolink_calc_checksum(rt.raw_rx, SYN_IOLINK_MAX_PD_LEN);
    rt.raw_rx_len = SYN_IOLINK_MAX_PD_LEN + 1U;

    uint8_t pd_in_guarded[SYN_IOLINK_MAX_PD_LEN + 4U];
    (void)memset(pd_in_guarded, 0xEE, sizeof(pd_in_guarded));
    size_t in_len = 0U;
    SYN_Status st = syn_iolink_master_exchange_pd(&master, NULL, 0U, pd_in_guarded, &in_len);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    TEST_ASSERT_EQUAL_UINT(SYN_IOLINK_MAX_PD_LEN, in_len);
    TEST_ASSERT_EQUAL_HEX8(0xEEU, pd_in_guarded[SYN_IOLINK_MAX_PD_LEN]);

    /* 2. Oversized PD response (33 payload + 1 checksum = 34 bytes) rejected without overflow */
    (void)memset(pd_in_guarded, 0xA5, sizeof(pd_in_guarded));
    for (size_t i = 0U; i < SYN_IOLINK_MAX_PD_LEN + 1U; i++) {
        rt.raw_rx[i] = 0x77U;
    }
    rt.raw_rx[SYN_IOLINK_MAX_PD_LEN + 1U] =
        syn_iolink_calc_checksum(rt.raw_rx, SYN_IOLINK_MAX_PD_LEN + 1U);
    rt.raw_rx_len = SYN_IOLINK_MAX_PD_LEN + 2U;
    in_len = 99U;

    st = syn_iolink_master_exchange_pd(&master, NULL, 0U, pd_in_guarded, &in_len);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, st);
    TEST_ASSERT_EQUAL_INT(SYN_IOLINK_PORT_FAULT, master.state);
    TEST_ASSERT_EQUAL_HEX8(0xA5U, pd_in_guarded[0]);
    TEST_ASSERT_EQUAL_HEX8(0xA5U, pd_in_guarded[SYN_IOLINK_MAX_PD_LEN]);

    /* 3. syn_iolink_master_start clears stale awaiting_rx and rx_deadline_ms */
    master.awaiting_rx = true;
    master.rx_deadline_ms = 0x12345678U;
    rt.raw_rx[0] = 0x11U;
    rt.raw_rx[1] = syn_iolink_calc_checksum(rt.raw_rx, 1U);
    rt.raw_rx_len = 2U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_start(&master));
    TEST_ASSERT_FALSE(master.awaiting_rx);
    TEST_ASSERT_EQUAL_UINT32(0U, master.rx_deadline_ms);

    /* 4. Unclamped cycle_time_ms > 0x7FFFFFFFU is clamped when arming rx_deadline_ms */
    master.cfg.cycle_time_ms = 0xFFFFFFFFU;
    master.last_cycle_ms = 0U;
    rt.has_data = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_step(&master, 1000U));
    TEST_ASSERT_TRUE(master.awaiting_rx);
    TEST_ASSERT_EQUAL_UINT32(1000U + 0x7FFFFFFFU, master.rx_deadline_ms);

    /* Advancing 1 ms must NOT prematurely trigger signed wraparound fault */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_step(&master, 1001U));
    TEST_ASSERT_TRUE(master.awaiting_rx);
    TEST_ASSERT_EQUAL_INT(SYN_IOLINK_PORT_OPERATE, master.state);
}

typedef struct {
    SYN_IOLink_Device *dev;
    uint32_t recv_idx;
    uint32_t fail_on_idx;
    uint8_t rx_buf[64];
    size_t rx_len;
} StartupFailTransport;

static bool startup_fail_send(const uint8_t *data, size_t len, void *ctx)
{
    StartupFailTransport *st = (StartupFailTransport *)ctx;
    st->rx_len = 0U;
    (void)syn_iolink_device_process_frame(st->dev, data, len, st->rx_buf, sizeof(st->rx_buf),
                                          &st->rx_len);
    return true;
}

static bool startup_fail_recv(uint8_t *buf, size_t max_len, size_t *out_len, void *ctx)
{
    StartupFailTransport *st = (StartupFailTransport *)ctx;
    st->recv_idx++;
    if (st->recv_idx == st->fail_on_idx || st->rx_len == 0U || st->rx_len > max_len) {
        return false;
    }
    (void)memcpy(buf, st->rx_buf, st->rx_len);
    *out_len = st->rx_len;
    return true;
}

void test_r5_a18_iolink_max_tx_guard_and_startup_read_errors(void)
{
    SYN_IOLink_Device dev;
    SYN_IOLink_DeviceConfig dev_cfg = {
        .params = {.min_cycle_time = 0x18U,
                   .revision_id = SYN_IOLINK_REV_1_1,
                   .vendor_id = 0x0123U,
                   .device_id = 0x00045678U,
                   .pd_in_len = 2U,
                   .pd_out_len = 2U},
        .on_read = on_dev_isdu_read,
        .on_write = on_dev_isdu_write,
    };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_device_init(&dev, &dev_cfg));

    /* 1. max_tx < 2 rejected with SYN_ERROR on Direct Param Read & ISDU Write without OOB write */
    uint8_t guard_tx[4] = {0xEEU, 0xEEU, 0xEEU, 0xEEU};
    size_t out_len = 99U;
    uint8_t dp_req[2] = {0x81U, 0U};
    dp_req[1] = syn_iolink_calc_checksum(dp_req, 1U);
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_iolink_device_process_frame(&dev, dp_req, 2U, guard_tx, 0U, &out_len));
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_iolink_device_process_frame(&dev, dp_req, 2U, guard_tx, 1U, &out_len));
    TEST_ASSERT_EQUAL_HEX8(0xEEU, guard_tx[0]);
    TEST_ASSERT_EQUAL_HEX8(0xEEU, guard_tx[1]);

    /* PD exchange with max_tx == 2 when pd_in_len == 2 (requires 3B) hits pd_in_len + 1 > max_tx */
    uint8_t pd_req[3] = {0x00U, 0x11U, 0U};
    pd_req[2] = syn_iolink_calc_checksum(pd_req, 2U);
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_iolink_device_process_frame(&dev, pd_req, 3U, guard_tx, 2U, &out_len));

    /* Runt ISDU (< 5 bytes) rejected cleanly */
    uint8_t runt_isdu[3] = {0xC0U, 0x00U, 0U};
    runt_isdu[2] = syn_iolink_calc_checksum(runt_isdu, 2U);
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_iolink_device_process_frame(&dev, runt_isdu, 3U, guard_tx, 4U, &out_len));

    /* 2. Later direct-parameter read failure (Vendor/Device/PD len #5) aborts startup -> FAULT */
    StartupFailTransport sft = {.dev = &dev, .recv_idx = 0U, .fail_on_idx = 5U};
    SYN_Transport tr = {.send = startup_fail_send, .recv = startup_fail_recv, .ctx = &sft};
    uint8_t rx[64], tx[64];
    SYN_IOLink_MasterConfig mcfg = {
        .transport = &tr,
        .rx_buf = rx,
        .rx_buf_size = sizeof(rx),
        .tx_buf = tx,
        .tx_buf_size = sizeof(tx),
    };
    SYN_IOLink_Master master;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_master_init(&master, &mcfg));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_iolink_master_start(&master));
    TEST_ASSERT_EQUAL_INT(SYN_IOLINK_PORT_FAULT, master.state);
}

void test_pro13_iolink_device_pd_len_bounds_and_clamp(void)
{
    SYN_IOLink_Device dev;
    SYN_IOLink_DeviceConfig cfg = {
        .params =
            {
                .min_cycle_time = 0x18U,
                .revision_id = SYN_IOLINK_REV_1_1,
                .vendor_id = 0x0123U,
                .device_id = 0x00045678U,
                .pd_in_len = (uint8_t)(SYN_IOLINK_MAX_PD_LEN + 1U),
                .pd_out_len = 2U,
            },
    };
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_iolink_device_init(&dev, &cfg));

    cfg.params.pd_in_len = 2U;
    cfg.params.pd_out_len = (uint8_t)(SYN_IOLINK_MAX_PD_LEN + 1U);
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_iolink_device_init(&dev, &cfg));

    cfg.params.pd_out_len = 4U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_iolink_device_init(&dev, &cfg));

    /* Mutated pd_out_len > SYN_IOLINK_MAX_PD_LEN is clamped in get_pd_out without OOB read */
    (void)memset(dev.pd_out, 0x5AU, sizeof(dev.pd_out));
    dev.cfg.params.pd_out_len = 64U;
    uint8_t out_buf[64];
    (void)memset(out_buf, 0xEEU, sizeof(out_buf));
    size_t out_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_iolink_device_get_pd_out(&dev, out_buf, sizeof(out_buf), &out_len));
    TEST_ASSERT_EQUAL_size_t(SYN_IOLINK_MAX_PD_LEN, out_len);
    TEST_ASSERT_EQUAL_HEX8(0x5AU, out_buf[SYN_IOLINK_MAX_PD_LEN - 1U]);
    TEST_ASSERT_EQUAL_HEX8(0xEEU, out_buf[SYN_IOLINK_MAX_PD_LEN]);
}

void run_iolink_tests(void)
{
    RUN_TEST(test_iolink_checksum_calculation);
    RUN_TEST(test_iolink_master_and_device_startup_handshake);
    RUN_TEST(test_iolink_cyclic_process_data_exchange);
    RUN_TEST(test_iolink_acyclic_isdu_read_write);
    RUN_TEST(test_iolink_master_errors_and_edge_cases);
    RUN_TEST(test_iolink_master_step_and_protothread);
    RUN_TEST(test_iolink_async_split_tx_rx_timeout_fault_and_pt_yielding);
    RUN_TEST(test_iolink_pd_bounds_stale_awaiting_rx_and_deadline_clamp);
    RUN_TEST(test_r5_a18_iolink_max_tx_guard_and_startup_read_errors);
    RUN_TEST(test_pro13_iolink_device_pd_len_bounds_and_clamp);
}
