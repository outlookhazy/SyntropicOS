/**
 * @file test_usb_hid.c
 * @brief Unit tests for Zero-Heap USB 2.0 Human Interface Device (HID) Class Driver.
 */

#include "syntropic/drivers/syn_usb_hid.h"
#include "unity/unity.h"

#include <string.h>

static const uint8_t SAMPLE_REPORT_DESC[24] = {
    0x05, 0x01,       /* Usage Page (Generic Desktop) */
    0x09, 0x00,       /* Usage (Undefined) */
    0xA1, 0x01,       /* Collection (Application) */
    0x09, 0x01,       /* Usage (Pointer) */
    0x15, 0x00,       /* Logical Minimum (0) */
    0x26, 0xFF, 0x00, /* Logical Maximum (255) */
    0x75, 0x08,       /* Report Size (8 bits) */
    0x95, 0x08,       /* Report Count (8 bytes) */
    0x81, 0x02,       /* Input (Data, Variable, Absolute) */
    0xC0              /* End Collection */
};

static const uint8_t TEST_DEVICE_DESC[18] = {0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0xFE,
                                             0xCA, 0xEF, 0xBE, 0x00, 0x01, 0x01, 0x02, 0x00, 0x01};

void test_usb_hid_init_and_register(void)
{
    SYN_USB_Device dev;
    SYN_USB_HID hid;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_init(&dev, TEST_DEVICE_DESC));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_init(&hid));
    TEST_ASSERT_EQUAL_UINT8(0x83, hid.ep_in);

    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_usb_hid_register(&dev, &hid, SAMPLE_REPORT_DESC, sizeof(SAMPLE_REPORT_DESC)));
    TEST_ASSERT_EQUAL_UINT8(0, hid.iface_num);
}

void test_usb_hid_report_send_and_read(void)
{
    SYN_USB_HID hid;
    syn_usb_hid_init(&hid);

    const uint8_t sample_data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    TEST_ASSERT_FALSE(syn_usb_hid_report_available(&hid));
    TEST_ASSERT_TRUE(syn_usb_hid_tx_ready(&hid));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_send_report(&hid, sample_data, sizeof(sample_data)));
    TEST_ASSERT_FALSE(syn_usb_hid_tx_ready(&hid));
    TEST_ASSERT_EQUAL_UINT16(8, hid.tx_len);

    /* Simulate OUT report received */
    memcpy(hid.rx_buf, sample_data, sizeof(sample_data));
    hid.rx_len = sizeof(sample_data);

    TEST_ASSERT_TRUE(syn_usb_hid_report_available(&hid));

    uint8_t read_buf[16] = {0};
    size_t out_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_hid_read_report(&hid, read_buf, sizeof(read_buf), &out_len));
    TEST_ASSERT_EQUAL_INT(8, out_len);
    TEST_ASSERT_EQUAL_MEMORY(sample_data, read_buf, 8);
    TEST_ASSERT_FALSE(syn_usb_hid_report_available(&hid));

    /* Empty report read (lines 150-151) */
    size_t rlen = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_read_report(&hid, read_buf, sizeof(read_buf), &rlen));
    TEST_ASSERT_EQUAL(0, rlen);

    /* Partial report read (lines 159-160) */
    memcpy(hid.rx_buf, sample_data, 8);
    hid.rx_len = 8;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_read_report(&hid, read_buf, 4, &rlen));
    TEST_ASSERT_EQUAL(4, rlen);
    TEST_ASSERT_EQUAL(4, hid.rx_len);

    /* Busy check & oversized report send */
    uint8_t big_buf[100] = {0};
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_usb_hid_send_report(&hid, big_buf, sizeof(big_buf)));
    hid.tx_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_send_report(&hid, big_buf, sizeof(big_buf)));
    TEST_ASSERT_EQUAL(64, hid.tx_len);
}

void test_usb_hid_class_requests(void)
{
    SYN_USB_Device dev;
    SYN_USB_HID hid;

    syn_usb_init(&dev, TEST_DEVICE_DESC);
    syn_usb_hid_init(&hid);
    syn_usb_hid_register(&dev, &hid, SAMPLE_REPORT_DESC, sizeof(SAMPLE_REPORT_DESC));

    SYN_USB_SetupPacket pkt = {.bmRequestType = 0x21,
                               .bRequest = SYN_USB_HID_REQ_SET_IDLE,
                               .wValue = (10U << 8), /* 40ms idle rate */
                               .wIndex = 0,
                               .wLength = 0};
    uint8_t resp[64];
    uint16_t rlen = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT8(10, hid.idle_rate);

    pkt.bRequest = SYN_USB_HID_REQ_GET_IDLE;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(1, rlen);
    TEST_ASSERT_EQUAL_UINT8(10, resp[0]);

    /* Get Report Descriptor */
    pkt.bmRequestType = 0x81;
    pkt.bRequest = SYN_USB_REQ_GET_DESCRIPTOR;
    pkt.wValue = (0x22U << 8);
    pkt.wLength = sizeof(SAMPLE_REPORT_DESC);

    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(sizeof(SAMPLE_REPORT_DESC), rlen);
    TEST_ASSERT_EQUAL_MEMORY(SAMPLE_REPORT_DESC, resp, rlen);

    /* Get Report request when tx_len == 0 */
    pkt.bmRequestType = 0xA1;
    pkt.bRequest = SYN_USB_HID_REQ_GET_REPORT;
    pkt.wLength = 8;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));

    /* Get Report request when tx_len > 0 */
    uint8_t dummy_report[8] = {0x11, 0x22};
    syn_usb_hid_send_report(&hid, dummy_report, sizeof(dummy_report));
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(8, rlen);
    TEST_ASSERT_EQUAL_MEMORY(dummy_report, resp, 8);

    /* Set Report request */
    pkt.bmRequestType = 0x21;
    pkt.bRequest = SYN_USB_HID_REQ_SET_REPORT;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));

    /* Get & Set Protocol requests */
    pkt.bRequest = SYN_USB_HID_REQ_SET_PROTOCOL;
    pkt.wValue = 0; /* Boot protocol */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT8(0, hid.active_protocol);

    pkt.bRequest = SYN_USB_HID_REQ_GET_PROTOCOL;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(1, rlen);
    TEST_ASSERT_EQUAL_UINT8(0, resp[0]);

    /* GET_DESCRIPTOR with invalid desc_type != 0x22 -> SYN_ERROR */
    pkt.bmRequestType = 0x81;
    pkt.bRequest = SYN_USB_REQ_GET_DESCRIPTOR;
    pkt.wValue = (0x99U << 8);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));

    /* Default unknown setup request */
    pkt.bRequest = 0xFF;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
}

#include "syntropic/drivers/syn_usb_hid_keyboard.h"
#include "syntropic/drivers/syn_usb_hid_mouse.h"

void test_usb_hid_keyboard_helpers(void)
{
    SYN_USB_HID hid;
    syn_usb_hid_init(&hid);

    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_usb_hid_keyboard_press(&hid, SYN_USB_HID_MOD_LSHIFT, SYN_USB_HID_KEY_A));
    TEST_ASSERT_EQUAL_UINT16(8, hid.tx_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_HID_MOD_LSHIFT, hid.tx_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00, hid.tx_buf[1]);
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_HID_KEY_A, hid.tx_buf[2]);

    hid.tx_len = 0; /* Clear TX buffer */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_keyboard_release(&hid));
    TEST_ASSERT_EQUAL_UINT16(8, hid.tx_len);
    TEST_ASSERT_EQUAL_UINT8(0x00, hid.tx_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00, hid.tx_buf[2]);

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_hid_keyboard_send(NULL, 0, NULL));
}

void test_usb_hid_mouse_helpers(void)
{
    SYN_USB_HID hid;
    syn_usb_hid_init(&hid);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_mouse_move(&hid, 10, -5));
    TEST_ASSERT_EQUAL_UINT16(4, hid.tx_len);
    TEST_ASSERT_EQUAL_UINT8(0, hid.tx_buf[0]);
    TEST_ASSERT_EQUAL_INT8(10, (int8_t)hid.tx_buf[1]);
    TEST_ASSERT_EQUAL_INT8(-5, (int8_t)hid.tx_buf[2]);

    hid.tx_len = 0; /* Clear TX buffer */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_mouse_click(&hid, SYN_USB_HID_MOUSE_BTN_LEFT));
    TEST_ASSERT_EQUAL_UINT16(4, hid.tx_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_HID_MOUSE_BTN_LEFT, hid.tx_buf[0]);

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_hid_mouse_send(NULL, 0, 0, 0, 0));
}

void test_usb_hid_null_checks(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_hid_init(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_hid_register(NULL, NULL, NULL, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_hid_send_report(NULL, NULL, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_hid_read_report(NULL, NULL, 0, NULL));
    TEST_ASSERT_FALSE(syn_usb_hid_report_available(NULL));

    SYN_USB_Device dev;
    SYN_USB_HID hid;
    syn_usb_init(&dev, TEST_DEVICE_DESC);
    syn_usb_hid_init(&hid);
    syn_usb_hid_register(&dev, &hid, SAMPLE_REPORT_DESC, sizeof(SAMPLE_REPORT_DESC));

    /* Call registered setup callback with NULL params (line 33) */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, dev.classes[0].setup(NULL, NULL, NULL, 0U, NULL));
    TEST_ASSERT_FALSE(syn_usb_hid_tx_ready(NULL));
}

void test_r5_usb_hid_regressions(void)
{
    SYN_USB_Device dev;
    SYN_USB_HID hid;
    syn_usb_init(&dev, TEST_DEVICE_DESC);
    syn_usb_hid_init(&hid);
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_usb_hid_register(&dev, &hid, SAMPLE_REPORT_DESC, sizeof(SAMPLE_REPORT_DESC)));

    const uint8_t rep1[4] = {0xAAU, 0xBBU, 0xCCU, 0xDDU};
    const uint8_t rep2[4] = {0x11U, 0x22U, 0x33U, 0x44U};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_send_report(&hid, rep1, sizeof(rep1)));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_usb_hid_send_report(&hid, rep2, sizeof(rep2)));
    TEST_ASSERT_EQUAL_MEMORY(rep1, hid.tx_buf, sizeof(rep1));

    SYN_USB_ClassDriver *drv = &dev.classes[0];
    drv->data_in(NULL, hid.ep_in);
    drv->data_in(&hid, 0x89U);
    TEST_ASSERT_FALSE(syn_usb_hid_tx_ready(&hid));
    drv->data_in(&hid, hid.ep_in);
    TEST_ASSERT_TRUE(syn_usb_hid_tx_ready(&hid));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_send_report(&hid, rep2, sizeof(rep2)));
}

void test_tier_a_usb_hid_regressions(void)
{
    /* 1. Boot Mouse protocol=2 patches bInterfaceSubClass=1 and bInterfaceProtocol=2 */
    SYN_USB_Device dev;
    SYN_USB_HID hid;
    syn_usb_init(&dev, TEST_DEVICE_DESC);
    syn_usb_hid_init(&hid);
    hid.protocol = 2U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_register(&dev, &hid, SYN_USB_HID_MOUSE_REPORT_DESC,
                                                       sizeof(SYN_USB_HID_MOUSE_REPORT_DESC)));
    TEST_ASSERT_EQUAL_UINT8(0x01U, dev.config_buf[9U + 6U]);
    TEST_ASSERT_EQUAL_UINT8(0x02U, dev.config_buf[9U + 7U]);

    /* 2. Mouse click stages press report and auto-queues zero release report on data_in */
    SYN_USB_ClassDriver *drv = &dev.classes[0];
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_mouse_click(&hid, SYN_USB_HID_MOUSE_BTN_LEFT));
    TEST_ASSERT_EQUAL_UINT16(4U, hid.tx_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_HID_MOUSE_BTN_LEFT, hid.tx_buf[0]);
    TEST_ASSERT_TRUE(hid.release_pending);

    /* Busy click while press is still staged returns SYN_BUSY without corrupting state */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_usb_hid_mouse_click(&hid, SYN_USB_HID_MOUSE_BTN_RIGHT));

    /* First IN completion stages the 4-byte button-release report */
    drv->data_in(&hid, hid.ep_in);
    TEST_ASSERT_FALSE(hid.release_pending);
    TEST_ASSERT_EQUAL_UINT16(4U, hid.tx_len);
    TEST_ASSERT_EQUAL_UINT8(0U, hid.tx_buf[0]);
    TEST_ASSERT_FALSE(syn_usb_hid_tx_ready(&hid));

    /* Second IN completion finishes the click cycle cleanly */
    drv->data_in(&hid, hid.ep_in);
    TEST_ASSERT_TRUE(syn_usb_hid_tx_ready(&hid));

    /* Zero-button click does not arm a duplicate release report */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_mouse_click(&hid, 0U));
    TEST_ASSERT_FALSE(hid.release_pending);
    drv->data_in(&hid, hid.ep_in);
    TEST_ASSERT_TRUE(syn_usb_hid_tx_ready(&hid));

    /* Unknown class-specific request routed to HID interface returns SYN_OK */
    uint8_t resp[8];
    uint16_t rlen = 0U;
    SYN_USB_SetupPacket pkt = {0x21U, 0xFFU, 0U, 0U, 0U};
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));

    /* USB-1: HID Report Descriptor (0x22) exceeding resp_cap returns SYN_ERROR */
    pkt.bmRequestType = 0x81U;
    pkt.bRequest = SYN_USB_REQ_GET_DESCRIPTOR;
    pkt.wValue = (uint16_t)(0x22U << 8);
    pkt.wLength = 255U;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(0U, rlen);

    /* AUD-L4: SET_CONFIGURATION clears staged tx_len and release_pending */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_mouse_click(&hid, SYN_USB_HID_MOUSE_BTN_LEFT));
    TEST_ASSERT_TRUE(hid.release_pending);
    TEST_ASSERT_FALSE(syn_usb_hid_tx_ready(&hid));
    TEST_ASSERT_EQUAL_INT(SYN_OK, drv->configured(NULL, 1U));
    pkt.bmRequestType = 0x00U;
    pkt.bRequest = SYN_USB_REQ_SET_CONFIGURATION;
    pkt.wValue = 1U;
    pkt.wIndex = 0U;
    pkt.wLength = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_FALSE(hid.release_pending);
    TEST_ASSERT_TRUE(syn_usb_hid_tx_ready(&hid));
}
