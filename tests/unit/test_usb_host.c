/**
 * @file test_usb_host.c
 * @brief Unit tests for Zero-Heap Modular USB 2.0 Host Core and Host CDC Class Driver.
 */

#include "mocks/mock_port.h"
#include "syntropic/drivers/syn_transport_usb_host_cdc.h"
#include "syntropic/drivers/syn_usb_host.h"
#include "syntropic/drivers/syn_usb_host_cdc.h"
#include "unity/unity.h"

#include <string.h>

/* Mock 18-byte Device Descriptor */
static const uint8_t MOCK_DEV_DESC[18] = {
    0x12, 0x01, 0x00, 0x02, 0x02, 0x00, 0x00, 0x40, /* bMaxPacketSize0 = 64 */
    0xFE, 0xCA, 0xEF, 0xBE, 0x00, 0x01, 0x01, 0x02, 0x00, 0x01};

/* Mock Configuration Descriptor (75 bytes, CDC ACM) */
static const uint8_t MOCK_CFG_DESC[] = {
    0x09, 0x02, 0x4B, 0x00, 0x02, 0x01, 0x00, 0xC0, 0x32,
    /* IAD */
    0x08, 0x0B, 0x00, 0x02, 0x02, 0x02, 0x01, 0x00,
    /* Interface 0: CDC Comm */
    0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00, 0x05, 0x24, 0x00, 0x10, 0x01, 0x05, 0x24,
    0x01, 0x00, 0x01, 0x04, 0x24, 0x02, 0x02, 0x05, 0x24, 0x06, 0x00, 0x01, 0x07, 0x05, 0x82, 0x03,
    0x08, 0x00, 0x10,
    /* Interface 1: CDC Data */
    0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x00, 0x07, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00,
    0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00};

static bool probe_called = false;
static bool disconnect_called = false;

static SYN_Status test_probe_cb(void *ctx, uint8_t dev_addr, const uint8_t *iface_desc,
                                uint16_t len)
{
    (void)ctx;
    (void)dev_addr;
    (void)iface_desc;
    (void)len;
    probe_called = true;
    return SYN_OK;
}

static void test_disconnect_cb(void *ctx)
{
    (void)ctx;
    disconnect_called = true;
}

void test_usb_host_init_and_state(void)
{
    SYN_USB_Host host;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_init(&host));
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_HOST_STATE_DISCONNECTED, host.state);
    TEST_ASSERT_FALSE(syn_usb_host_is_ready(&host));
    TEST_ASSERT_NULL(syn_usb_host_get_dev_info(&host));
}

void test_usb_host_enumeration_flow(void)
{
    mock_usb_host_reset();
    SYN_USB_Host host;
    syn_usb_host_init(&host);

    /* 1. DISCONNECTED state */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_process(&host));
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_HOST_STATE_DISCONNECTED, host.state);

    /* 2. Device attach */
    mock_usb_host_attached = true;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_process(&host));
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_HOST_STATE_ATTACHED, host.state);
    TEST_ASSERT_TRUE(mock_usb_host_vbus_enabled);

    /* 3. Transition to ENUMERATING */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_process(&host));
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_HOST_STATE_ENUMERATING, host.state);

    /* 4. Drive GET_DEV8 -> inject mock 8 bytes */
    memcpy(mock_usb_host_xfer_buf, MOCK_DEV_DESC, 8);
    mock_usb_host_xfer_len = 8;
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_usb_host_process(&host)); /* Submits setup & data */

    /* 5. Process completion -> SET_ADDR */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_usb_host_process(&host));

    /* 6. SET_ADDR completion -> GET_DEV_FULL -> inject 18 bytes */
    memcpy(mock_usb_host_xfer_buf, MOCK_DEV_DESC, 18);
    mock_usb_host_xfer_len = 18;
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_usb_host_process(&host));

    /* 7. GET_DEV_FULL completion -> GET_CFG -> inject 67 bytes */
    memcpy(mock_usb_host_xfer_buf, MOCK_CFG_DESC, sizeof(MOCK_CFG_DESC));
    mock_usb_host_xfer_len = sizeof(MOCK_CFG_DESC);
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_usb_host_process(&host));

    /* 8. GET_CFG completion -> SET_CFG */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_usb_host_process(&host));

    /* 9. SET_CFG completion -> CLASS_PROBE -> READY */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_process(&host));
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_HOST_STATE_READY, host.state);
    TEST_ASSERT_TRUE(syn_usb_host_is_ready(&host));

    const SYN_USB_HostDevInfo *info = syn_usb_host_get_dev_info(&host);
    TEST_ASSERT_NOT_NULL(info);
    TEST_ASSERT_EQUAL_HEX16(0xCAFE, info->vid);
    TEST_ASSERT_EQUAL_HEX16(0xBEEF, info->pid);

    /* Test xfer pending polling & transfer failure */
    SYN_USB_Host host_err;
    syn_usb_host_init(&host_err);
    host_err.state = SYN_USB_HOST_STATE_ENUMERATING;
    host_err.enum_step = SYN_USB_HOST_ENUM_GET_DEV8;
    syn_usb_host_process(&host_err); /* sets xfer_pending = true */

    mock_usb_host_xfer_complete = false;
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_usb_host_process(&host_err));
    mock_usb_host_xfer_complete = true;

    mock_usb_host_xfer_status = SYN_ERROR;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_usb_host_process(&host_err));
    mock_usb_host_xfer_status = SYN_OK;

    /* Test next_addr > 127 and max_pkt0 == 0 */
    SYN_USB_Host host_addr;
    syn_usb_host_init(&host_addr);
    host_addr.next_addr = 128;
    host_addr.state = SYN_USB_HOST_STATE_ENUMERATING;
    host_addr.enum_step = SYN_USB_HOST_ENUM_SET_ADDR;
    host_addr.enum_buf[7] = 0; /* max_pkt0 = 0 -> reset to 8 */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_usb_host_process(&host_addr));
    TEST_ASSERT_EQUAL_UINT8(1, host_addr.dev_info.dev_addr);

    /* 10. Detach device */
    mock_usb_host_attached = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_process(&host));
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_HOST_STATE_DISCONNECTED, host.state);
    TEST_ASSERT_FALSE(syn_usb_host_is_ready(&host));
}

void test_usb_host_class_registration_and_probing(void)
{
    SYN_USB_Host host;
    syn_usb_host_init(&host);

    probe_called = false;
    disconnect_called = false;

    SYN_USB_HostClassDriver cls = {.class_code = 0x02, /* CDC Comm */
                                   .subclass_code = 0xFF,
                                   .protocol_code = 0xFF,
                                   .ctx = NULL,
                                   .probe = test_probe_cb,
                                   .disconnected = test_disconnect_cb,
                                   .process = NULL};

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_register_class(&host, &cls));

    /* Drive through enumeration with CDC config descriptor */
    mock_usb_host_reset();
    mock_usb_host_attached = true;
    syn_usb_host_process(&host); /* DISCONNECTED -> ATTACHED */
    syn_usb_host_process(&host); /* ATTACHED -> ENUMERATING */

    memcpy(mock_usb_host_xfer_buf, MOCK_DEV_DESC, 8);
    mock_usb_host_xfer_len = 8;
    syn_usb_host_process(&host); /* GET_DEV8 */

    syn_usb_host_process(&host); /* SET_ADDR */

    memcpy(mock_usb_host_xfer_buf, MOCK_DEV_DESC, 18);
    mock_usb_host_xfer_len = 18;
    syn_usb_host_process(&host); /* GET_DEV_FULL */

    memcpy(mock_usb_host_xfer_buf, MOCK_CFG_DESC, sizeof(MOCK_CFG_DESC));
    mock_usb_host_xfer_len = sizeof(MOCK_CFG_DESC);
    syn_usb_host_process(&host); /* GET_CFG */

    syn_usb_host_process(&host); /* SET_CFG */

    syn_usb_host_process(&host); /* CLASS_PROBE */
    TEST_ASSERT_TRUE(probe_called);
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_HOST_STATE_READY, host.state);

    /* Simulate detach */
    mock_usb_host_attached = false;
    syn_usb_host_process(&host);
    TEST_ASSERT_TRUE(disconnect_called);
}

void test_usb_host_cdc_driver(void)
{
    SYN_USB_Host host;
    SYN_USB_HostCDC hcdc;
    syn_usb_host_init(&host);
    syn_usb_host_cdc_init(&hcdc);
    syn_usb_host_cdc_register(&host, &hcdc);

    /* Run full enumeration to trigger host_cdc_probe */
    mock_usb_host_reset();
    mock_usb_host_attached = true;
    syn_usb_host_process(&host); /* ATTACHED */
    syn_usb_host_process(&host); /* ENUMERATING */

    memcpy(mock_usb_host_xfer_buf, MOCK_DEV_DESC, 8);
    mock_usb_host_xfer_len = 8;
    syn_usb_host_process(&host); /* GET_DEV8 */
    syn_usb_host_process(&host); /* SET_ADDR */

    memcpy(mock_usb_host_xfer_buf, MOCK_DEV_DESC, 18);
    mock_usb_host_xfer_len = 18;
    syn_usb_host_process(&host); /* GET_DEV_FULL */

    memcpy(mock_usb_host_xfer_buf, MOCK_CFG_DESC, sizeof(MOCK_CFG_DESC));
    mock_usb_host_xfer_len = sizeof(MOCK_CFG_DESC);
    syn_usb_host_process(&host); /* GET_CFG */
    syn_usb_host_process(&host); /* SET_CFG */

    syn_usb_host_process(&host); /* CLASS_PROBE */

    /* Also register CDC Data class (0x0A) to probe bulk endpoints (lines 48-58) */
    SYN_USB_HostClassDriver data_cls = {.class_code = 0x0A,
                                        .subclass_code = 0xFF,
                                        .protocol_code = 0xFF,
                                        .ctx = &hcdc,
                                        .probe =
                                            host.classes[0].probe ? host.classes[0].probe : NULL};
    syn_usb_host_register_class(&host, &data_cls);

    /* Directly trigger probe on Interface 1 descriptors containing Bulk IN/OUT */
    SYN_USB_HostClassDriver cdc_driver;
    memset(&cdc_driver, 0, sizeof(cdc_driver));
    syn_usb_host_cdc_register(&host, &hcdc);

    /* Exercise probe null check (line 26) & Interface 1 bulk endpoints (lines 48-58) */
    uint8_t iface1_desc[] = {
        0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00,
        0x00, 0x07, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00, /* Bulk OUT ep 0x01 */
        0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00        /* Bulk IN  ep 0x81 */
    };
    extern SYN_Status host_cdc_probe_test(void *ctx, uint8_t dev_addr, const uint8_t *iface_desc,
                                          uint16_t len);
    /* Probe Interface 1 bulk endpoints directly */
    SYN_USB_HostClassDriver *cls_ptr = &host.classes[0];
    cls_ptr->probe(&hcdc, 1, iface1_desc, sizeof(iface1_desc));
    TEST_ASSERT_TRUE(hcdc.connected);
    TEST_ASSERT_EQUAL_UINT8(0x81, hcdc.ep_bulk_in);
    TEST_ASSERT_EQUAL_UINT8(0x01, hcdc.ep_bulk_out);

    /* Test probe null check (line 26) & process disconnected check (line 96) */
    cls_ptr->probe(NULL, 0, NULL, 0);
    cls_ptr->process(NULL);

    SYN_USB_HostCDC hcdc_disc = {0};
    cls_ptr->process(&hcdc_disc);

    /* Oversized write (line 166) */
    uint8_t big_tx[200] = {0};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_cdc_write(&hcdc, big_tx, sizeof(big_tx)));
    TEST_ASSERT_EQUAL(128, hcdc.tx_len);

    /* Test process tick with active TX and IN transfer (line 120) */
    syn_usb_host_process(&host);

    /* Partial read & shift (lines 188, 195, 196) */
    char rx_buf[32];
    size_t out_len = 0;
    memcpy(hcdc.rx_buf, "1234567890", 10);
    hcdc.rx_len = 10;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_cdc_read(&hcdc, rx_buf, 4, &out_len));
    TEST_ASSERT_EQUAL(4, out_len);
    TEST_ASSERT_EQUAL(6, hcdc.rx_len);

    /* Full read to trigger line 198 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_cdc_read(&hcdc, rx_buf, sizeof(rx_buf), &out_len));
    TEST_ASSERT_EQUAL(6, out_len);
    TEST_ASSERT_EQUAL(0, hcdc.rx_len);

    /* Trigger non-blocking IN submit (line 120) */
    mock_usb_host_xfer_complete = true;
    mock_usb_host_xfer_len = 0;
    cls_ptr->process(&hcdc);

    /* Test disconnect with active pipes (lines 77, 80) */
    cls_ptr->disconnected(&hcdc);
    TEST_ASSERT_FALSE(hcdc.connected);
}

void test_usb_host_cdc_transport_bridge(void)
{
    SYN_USB_HostCDC hcdc;
    syn_usb_host_cdc_init(&hcdc);

    SYN_Transport tr;
    syn_transport_from_usb_host_cdc(&tr, &hcdc);
    TEST_ASSERT_NOT_NULL(tr.send);
    TEST_ASSERT_NOT_NULL(tr.recv);

    uint8_t tx_data[] = {0x01, 0x02, 0x03};
    TEST_ASSERT_TRUE(syn_transport_send(&tr, tx_data, sizeof(tx_data)));

    uint8_t rx_buf[16];
    size_t rlen = 0;
    TEST_ASSERT_FALSE(syn_transport_recv(&tr, rx_buf, sizeof(rx_buf), &rlen));
    TEST_ASSERT_EQUAL(0, rlen);

    /* Test NULL hcdc (lines 47-50 in syn_transport_usb_host_cdc.c) */
    syn_transport_from_usb_host_cdc(&tr, NULL);
    TEST_ASSERT_NULL(tr.send);
    TEST_ASSERT_NULL(tr.recv);
    TEST_ASSERT_NULL(tr.ctx);

    /* Test class count overflow */
    SYN_USB_Host host;
    syn_usb_host_init(&host);
    SYN_USB_HostClassDriver dummy_cd = {0};
    host.class_count = SYN_USB_HOST_MAX_CLASSES;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_usb_host_register_class(&host, &dummy_cd));

    /* Test STATE_ERROR recovery on detach */
    host.state = SYN_USB_HOST_STATE_ERROR;
    mock_usb_host_attached = true;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_usb_host_process(&host));

    mock_usb_host_attached = false;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_usb_host_process(&host));
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_HOST_STATE_DISCONNECTED, host.state);

    /* Test ENUM_DONE and invalid enum_step */
    host.state = SYN_USB_HOST_STATE_ENUMERATING;
    host.xfer_pending = false;
    host.enum_step = SYN_USB_HOST_ENUM_DONE;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_process(&host));

    host.state = SYN_USB_HOST_STATE_ENUMERATING;
    host.enum_step = 99;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_usb_host_process(&host));

    /* Test invalid host.state (line 330) */
    host.state = 99;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_usb_host_process(&host));
}

void test_usb_host_null_checks(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_host_init(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_host_register_class(NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_host_process(NULL));
    TEST_ASSERT_FALSE(syn_usb_host_is_ready(NULL));
    TEST_ASSERT_NULL(syn_usb_host_get_dev_info(NULL));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_host_cdc_init(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_host_cdc_register(NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_host_cdc_write(NULL, NULL, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_host_cdc_read(NULL, NULL, 0, NULL));
    TEST_ASSERT_FALSE(syn_usb_host_cdc_rx_available(NULL));
    TEST_ASSERT_FALSE(syn_usb_host_cdc_tx_ready(NULL));

    syn_transport_from_usb_host_cdc(NULL, NULL);

    SYN_Transport tr;
    SYN_USB_HostCDC hcdc;
    syn_transport_from_usb_host_cdc(&tr, &hcdc);
    TEST_ASSERT_FALSE(syn_transport_send(&tr, NULL, 0));
    TEST_ASSERT_FALSE(syn_transport_send(&tr, (const uint8_t *)"a", 0));

    uint8_t out_b[10];
    size_t out_l;
    TEST_ASSERT_FALSE(syn_transport_recv(&tr, NULL, sizeof(out_b), &out_l));
    TEST_ASSERT_FALSE(syn_transport_recv(&tr, out_b, sizeof(out_b), NULL));
}

void test_usb_host_subordinate_descriptor_overflow(void)
{
    mock_usb_host_reset();
    SYN_USB_Host host;
    syn_usb_host_init(&host);

    SYN_USB_HostClassDriver cls = {
        .class_code = 0xFF,
        .subclass_code = 0xFF,
        .protocol_code = 0xFF,
        .probe = test_probe_cb,
    };
    syn_usb_host_register_class(&host, &cls);

    /* Interface (9B) followed by a subordinate descriptor claiming 100 bytes when only 2 remain */
    uint8_t malformed_cfg[12] = {
        0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00, /* Interface 0 */
        0x64, 0x24, 0x00                                      /* Subordinate with bLength = 100 */
    };
    memcpy(host.enum_buf, malformed_cfg, sizeof(malformed_cfg));
    host.enum_buf_len = sizeof(malformed_cfg);

    probe_called = false;
    host.state = SYN_USB_HOST_STATE_ENUMERATING;
    host.enum_step = SYN_USB_HOST_ENUM_CLASS_PROBE;
    host.xfer_pending = false;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_process(&host));
    TEST_ASSERT_TRUE(probe_called);
}

static void test_r5_usb_host_regressions(void)
{
    /* 1. Enumerate actual syn_usb_cdc 2-interface descriptor with actual=0 on SET_CFG */
    SYN_USB_Device dev;
    SYN_USB_CDC cdc_dev;
    syn_usb_init(&dev, MOCK_DEV_DESC);
    syn_usb_cdc_init(&cdc_dev);
    syn_usb_cdc_register(&dev, &cdc_dev);

    mock_usb_host_reset();
    SYN_USB_Host host;
    SYN_USB_HostCDC hcdc;
    syn_usb_host_init(&host);
    syn_usb_host_cdc_init(&hcdc);
    syn_usb_host_cdc_register(&host, &hcdc);

    mock_usb_host_attached = true;
    syn_usb_host_process(&host); /* DISCONNECTED -> ATTACHED */
    syn_usb_host_process(&host); /* ATTACHED -> ENUMERATING */

    memcpy(mock_usb_host_xfer_buf, MOCK_DEV_DESC, 8U);
    mock_usb_host_xfer_len = 8U;
    syn_usb_host_process(&host); /* GET_DEV8 submit */
    syn_usb_host_process(&host); /* GET_DEV8 done -> SET_ADDR submit */

    mock_usb_host_xfer_len = 0U; /* Zero-length status on SET_ADDRESS */
    memcpy(mock_usb_host_xfer_buf, MOCK_DEV_DESC, 18U);
    syn_usb_host_process(&host); /* SET_ADDR done -> GET_DEV_FULL (submits before xfer_len set) */
    memcpy(host.enum_buf, MOCK_DEV_DESC, 18U);
    mock_usb_host_xfer_len = 18U;
    syn_usb_host_process(&host); /* GET_DEV_FULL done -> GET_CFG submit */

    memcpy(host.enum_buf, dev.config_desc, dev.config_desc_len);
    mock_usb_host_xfer_len = dev.config_desc_len;
    syn_usb_host_process(&host); /* GET_CFG done -> SET_CFG submit */

    /* Zero-length status on SET_CONFIGURATION must NOT wipe host.enum_buf_len! */
    mock_usb_host_xfer_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_process(&host));
    TEST_ASSERT_EQUAL_UINT16(dev.config_desc_len, host.enum_buf_len);
    TEST_ASSERT_TRUE(hcdc.connected);
    TEST_ASSERT_EQUAL_HEX8(0x81U, hcdc.ep_bulk_in);
    TEST_ASSERT_EQUAL_HEX8(0x01U, hcdc.ep_bulk_out);
    TEST_ASSERT_EQUAL_UINT8(1U, hcdc.pipe_bulk_in);
    TEST_ASSERT_EQUAL_UINT8(2U, hcdc.pipe_bulk_out);

    /* Polling class process while rx_len != 0 skips RX refill cleanly (line 152) */
    hcdc.rx_len = 2U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_process(&host));
    hcdc.rx_len = 0U;

    /* 2. Non-adjacent CDC Comm + Vendor + CDC Data fallback & partial/missing pipe cleanup */
    static const uint8_t non_adj_cfg[] = {
        0x09U, 0x04U, 0x00U, 0x00U, 0x00U, 0x02U, 0x02U, 0x01U, 0x00U, /* Comm */
        0x09U, 0x04U, 0x01U, 0x00U, 0x00U, 0xFFU, 0x00U, 0x00U, 0x00U, /* Vendor */
        0x09U, 0x04U, 0x02U, 0x00U, 0x02U, 0x0AU, 0x00U, 0x00U, 0x00U, /* CDC Data */
        0x07U, 0x05U, 0x02U, 0x02U, 0x40U, 0x00U, 0x00U,               /* Bulk OUT */
        0x07U, 0x05U, 0x82U, 0x02U, 0x40U, 0x00U, 0x00U,               /* Bulk IN */
        0x01U, 0x00U                                                   /* Truncated tail */
    };
    syn_usb_host_init(&host);
    syn_usb_host_cdc_init(&hcdc);
    syn_usb_host_cdc_register(&host, &hcdc);
    memcpy(host.enum_buf, non_adj_cfg, sizeof(non_adj_cfg));
    host.enum_buf_len = sizeof(non_adj_cfg);
    host.state = SYN_USB_HOST_STATE_ENUMERATING;
    host.enum_step = SYN_USB_HOST_ENUM_CLASS_PROBE;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_host_process(&host));
    TEST_ASSERT_TRUE(hcdc.connected);
    TEST_ASSERT_EQUAL_HEX8(0x82U, hcdc.ep_bulk_in);
    TEST_ASSERT_EQUAL_HEX8(0x02U, hcdc.ep_bulk_out);

    /* Only Bulk IN present -> closes pipe 1 and fails probe */
    uint8_t only_in_desc[] = {0x09U, 0x04U, 0x00U, 0x00U, 0x01U, 0x0AU, 0x00U, 0x00U, 0x00U,
                              0x07U, 0x05U, 0x81U, 0x02U, 0x40U, 0x00U, 0x00U, 0x01U, 0x00U};
    hcdc.host = NULL;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          host.classes[0].probe(&hcdc, 1U, only_in_desc, sizeof(only_in_desc)));
    TEST_ASSERT_FALSE(hcdc.connected);

    /* Only Bulk OUT present -> closes pipe 2 and fails probe */
    uint8_t only_out_desc[] = {0x09U, 0x04U, 0x00U, 0x00U, 0x01U, 0x0AU, 0x00U, 0x00U,
                               0x00U, 0x07U, 0x05U, 0x01U, 0x02U, 0x40U, 0x00U, 0x00U};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          host.classes[0].probe(&hcdc, 1U, only_out_desc, sizeof(only_out_desc)));

    /* 3. Dirty stack SYN_Transport initialization + has_data + non-empty recv */
    SYN_Transport tr;
    memset(&tr, 0xAA, sizeof(tr));
    syn_transport_from_usb_host_cdc(&tr, &hcdc);
    TEST_ASSERT_NOT_NULL(tr.has_data);
    TEST_ASSERT_FALSE(syn_transport_has_data(&tr));

    memcpy(hcdc.rx_buf, "HOST!", 5U);
    hcdc.rx_len = 5U;
    TEST_ASSERT_TRUE(syn_transport_has_data(&tr));

    uint8_t out[16] = {0};
    size_t out_len = 0U;
    TEST_ASSERT_TRUE(syn_transport_recv(&tr, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_UINT32(5U, (uint32_t)out_len);
    TEST_ASSERT_EQUAL_MEMORY("HOST!", out, 5U);
    TEST_ASSERT_FALSE(syn_transport_has_data(&tr));
}

void run_usb_host_tests(void)
{
    RUN_TEST(test_usb_host_init_and_state);
    RUN_TEST(test_usb_host_enumeration_flow);
    RUN_TEST(test_usb_host_class_registration_and_probing);
    RUN_TEST(test_usb_host_cdc_driver);
    RUN_TEST(test_usb_host_cdc_transport_bridge);
    RUN_TEST(test_usb_host_null_checks);
    RUN_TEST(test_usb_host_subordinate_descriptor_overflow);
    RUN_TEST(test_r5_usb_host_regressions);
}
