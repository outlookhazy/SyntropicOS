/**
 * @file test_usb.c
 * @brief Unit tests for Zero-Heap Modular USB 2.0 Device Core Protocol Engine.
 */

#include "syntropic/drivers/syn_usb.h"
#include "syntropic/drivers/syn_usb_cdc.h"
#include "syntropic/drivers/syn_usb_hid.h"
#include "syntropic/drivers/syn_usb_midi.h"
#include "syntropic/drivers/syn_usb_msc.h"
#include "unity/unity.h"

#include <string.h>

/* Standard 18-byte Device Descriptor Template */
static const uint8_t TEST_DEVICE_DESC[18] = {0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0xFE,
                                             0xCA, 0xEF, 0xBE, 0x00, 0x01, 0x01, 0x02, 0x00, 0x01};

/* Test Class Setup Callback */
static SYN_Status test_class_setup(void *ctx, const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                                   uint16_t resp_cap, uint16_t *rlen)
{
    (void)ctx;
    if (pkt->bRequest == 0x77 && resp_cap >= 1U) {
        resp[0] = 0xAA;
        *rlen = 1;
        return SYN_OK;
    }
    return SYN_ERROR;
}

void test_usb_init_and_state(void)
{
    SYN_USB_Device dev;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_init(&dev, TEST_DEVICE_DESC));
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_STATE_DEFAULT, dev.state);
    TEST_ASSERT_FALSE(syn_usb_is_configured(&dev));
}

void test_usb_set_address_and_config(void)
{
    SYN_USB_Device dev;
    syn_usb_init(&dev, TEST_DEVICE_DESC);

    SYN_USB_SetupPacket pkt = {.bmRequestType = 0x00,
                               .bRequest = SYN_USB_REQ_SET_ADDRESS,
                               .wValue = 0x0A,
                               .wIndex = 0,
                               .wLength = 0};
    uint8_t resp[64];
    uint16_t rlen = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT8(10, dev.dev_address);
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_STATE_ADDRESS, dev.state);

    pkt.bRequest = SYN_USB_REQ_SET_CONFIGURATION;
    pkt.wValue = 1;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_STATE_CONFIGURED, dev.state);
    TEST_ASSERT_TRUE(syn_usb_is_configured(&dev));

    pkt.bRequest = SYN_USB_REQ_GET_CONFIGURATION;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(1, rlen);
    TEST_ASSERT_EQUAL_UINT8(1, resp[0]);
}

void test_usb_get_descriptors(void)
{
    SYN_USB_Device dev;
    syn_usb_init(&dev, TEST_DEVICE_DESC);

    static const uint8_t str0[4] = {0x04, 0x03, 0x09, 0x04};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_set_string_desc(&dev, 0, str0));

    SYN_USB_SetupPacket pkt = {.bmRequestType = 0x80,
                               .bRequest = SYN_USB_REQ_GET_DESCRIPTOR,
                               .wValue = (SYN_USB_DESC_TYPE_DEVICE << 8),
                               .wIndex = 0,
                               .wLength = 18};
    uint8_t resp[SYN_USB_EP0_BUF_SIZE];
    uint16_t rlen = 0;

    /* Get Device Descriptor */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(18, rlen);
    TEST_ASSERT_EQUAL_MEMORY(TEST_DEVICE_DESC, resp, 18);

    /* Get Config Descriptor */
    pkt.wValue = (SYN_USB_DESC_TYPE_CONFIGURATION << 8);
    pkt.wLength = 256;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(9, rlen);

    /* Get String Descriptor */
    pkt.wValue = (SYN_USB_DESC_TYPE_STRING << 8);
    pkt.wLength = 4;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(4, rlen);
    TEST_ASSERT_EQUAL_MEMORY(str0, resp, 4);

    /* Get Status */
    pkt.bRequest = SYN_USB_REQ_GET_STATUS;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(2, rlen);
    TEST_ASSERT_EQUAL_UINT8(0x01, resp[0]);
}

void test_usb_class_registration_and_routing(void)
{
    SYN_USB_Device dev;
    syn_usb_init(&dev, TEST_DEVICE_DESC);

    SYN_USB_ClassDriver cls = {.iface_start = 0,
                               .iface_count = 1,
                               .ctx = NULL,
                               .setup = test_class_setup,
                               .data_out = NULL,
                               .data_in = NULL,
                               .configured = NULL};

    uint8_t dummy_iface_desc[9] = {0x09, 0x04, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00};
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_usb_register_class(&dev, &cls, dummy_iface_desc, sizeof(dummy_iface_desc)));
    TEST_ASSERT_EQUAL_UINT16(18, dev.config_desc_len);

    /* Process class setup request targeted to interface 0 */
    SYN_USB_SetupPacket pkt = {.bmRequestType = 0x21, /* Class request to Interface */
                               .bRequest = 0x77,
                               .wValue = 0,
                               .wIndex = 0,
                               .wLength = 1};
    uint8_t resp[16];
    uint16_t rlen = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(1, rlen);
    TEST_ASSERT_EQUAL_UINT8(0xAA, resp[0]);
}

void test_usb_raw_config_override(void)
{
    SYN_USB_Device dev;
    syn_usb_init(&dev, TEST_DEVICE_DESC);

    static const uint8_t raw_config[9] = {0x09, 0x02, 0x09, 0x00, 0x01, 0x01, 0x00, 0xC0, 0x32};
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_set_raw_config_desc(&dev, raw_config, sizeof(raw_config)));

    SYN_USB_SetupPacket pkt = {.bmRequestType = 0x80,
                               .bRequest = SYN_USB_REQ_GET_DESCRIPTOR,
                               .wValue = (SYN_USB_DESC_TYPE_CONFIGURATION << 8),
                               .wIndex = 0,
                               .wLength = 9};
    uint8_t resp[16];
    uint16_t rlen = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(9, rlen);
    TEST_ASSERT_EQUAL_MEMORY(raw_config, resp, 9);
}

void test_usb_null_checks(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_init(NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_register_class(NULL, NULL, NULL, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_set_string_desc(NULL, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_set_raw_config_desc(NULL, NULL, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_process_setup(NULL, NULL, NULL, 0U, NULL));
    TEST_ASSERT_FALSE(syn_usb_is_configured(NULL));

    SYN_USB_Device dev;
    syn_usb_init(&dev, TEST_DEVICE_DESC);
    SYN_USB_ClassDriver dummy_cls = {0};
    dev.class_count = SYN_USB_MAX_CLASSES;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_usb_register_class(&dev, &dummy_cls, NULL, 0));

    dev.class_count = 0;
    dev.config_buf_used = SYN_USB_MAX_CONFIG_DESC;
    uint8_t dummy_iface[5] = {0};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_usb_register_class(&dev, &dummy_cls, dummy_iface, 5));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_set_string_desc(&dev, 99, dummy_iface));

    /* SET_ADDRESS 0 -> DEFAULT state */
    SYN_USB_SetupPacket pkt = {
        .bmRequestType = 0, .bRequest = SYN_USB_REQ_SET_ADDRESS, .wValue = 0};
    uint8_t resp[16];
    uint16_t rlen = 0;
    syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen);
    TEST_ASSERT_EQUAL_INT(SYN_USB_STATE_DEFAULT, dev.state);

    /* SET_CONFIGURATION 0 -> ADDRESS state */
    pkt.bRequest = SYN_USB_REQ_SET_CONFIGURATION;
    pkt.wValue = 0;
    syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen);
    TEST_ASSERT_EQUAL_INT(SYN_USB_STATE_ADDRESS, dev.state);

    /* String desc request wLength truncation & invalid index */
    uint8_t sdesc[4] = {4, 3, 'A', 0};
    syn_usb_set_string_desc(&dev, 0, sdesc);
    pkt.bRequest = SYN_USB_REQ_GET_DESCRIPTOR;
    pkt.wValue = (SYN_USB_DESC_TYPE_STRING << 8) | 0;
    pkt.wLength = 2;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(2, rlen);

    pkt.wValue = (SYN_USB_DESC_TYPE_STRING << 8) | 10;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));

    /* Unknown bRequest */
    pkt.bRequest = 0xFF;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
}

void test_r5_usb_regressions(void)
{
    static const uint8_t hid_rep_desc[8] = {0x05, 0x01, 0x09, 0x00, 0xA1, 0x01, 0xC0, 0x00};

    SYN_USB_Device dev;
    SYN_USB_MSC msc;
    SYN_USB_CDC cdc;
    SYN_USB_HID hid;
    SYN_USB_MIDI midi;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_init(&dev, TEST_DEVICE_DESC));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_msc_init(&msc));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_cdc_init(&cdc));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_init(&hid));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_midi_init(&midi));

    /* Register all 4 classes on one composite USB device */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_msc_register(&dev, &msc));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_cdc_register(&dev, &cdc));
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_hid_register(&dev, &hid, hid_rep_desc, sizeof(hid_rep_desc)));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_midi_register(&dev, &midi));

    TEST_ASSERT_EQUAL_UINT8(4U, dev.class_count);
    TEST_ASSERT_EQUAL_UINT8(6U, dev.iface_count);
    TEST_ASSERT_EQUAL_UINT8(6U, dev.config_buf[4]);

    /* Verify non-colliding interface numbers */
    TEST_ASSERT_EQUAL_UINT8(0U, msc.iface_num);
    TEST_ASSERT_EQUAL_UINT8(1U, cdc.iface_num);
    TEST_ASSERT_EQUAL_UINT8(3U, hid.iface_num);
    TEST_ASSERT_EQUAL_UINT8(4U, midi.iface_num);

    /* Verify non-colliding IN endpoints across MSC, CDC (cmd+in), HID, MIDI */
    TEST_ASSERT_EQUAL_HEX8(0x82U, msc.ep_in);
    TEST_ASSERT_EQUAL_HEX8(0x81U, cdc.ep_cmd);
    TEST_ASSERT_EQUAL_HEX8(0x83U, cdc.ep_in);
    TEST_ASSERT_EQUAL_HEX8(0x84U, hid.ep_in);
    TEST_ASSERT_EQUAL_HEX8(0x85U, midi.ep_in);

    /* Verify non-colliding OUT endpoints across MSC, CDC, MIDI */
    TEST_ASSERT_EQUAL_HEX8(0x02U, msc.ep_out);
    TEST_ASSERT_EQUAL_HEX8(0x01U, cdc.ep_out);
    TEST_ASSERT_EQUAL_HEX8(0x03U, midi.ep_out);

    /* Verify patched CDC IAD & Union functional descriptors in config_buf */
    uint16_t cdc_off = 9U + SYN_USB_MSC_DESC_SIZE;
    TEST_ASSERT_EQUAL_UINT8(1U, dev.config_buf[cdc_off + 2U]);  /* IAD bFirstInterface */
    TEST_ASSERT_EQUAL_UINT8(1U, dev.config_buf[cdc_off + 10U]); /* Comm bInterfaceNumber */
    TEST_ASSERT_EQUAL_UINT8(2U, dev.config_buf[cdc_off + 26U]); /* CM bDataInterface */
    TEST_ASSERT_EQUAL_UINT8(1U, dev.config_buf[cdc_off + 34U]); /* Union bMasterInterface0 */
    TEST_ASSERT_EQUAL_UINT8(2U, dev.config_buf[cdc_off + 35U]); /* Union bSlaveInterface0 */
    TEST_ASSERT_EQUAL_UINT8(2U, dev.config_buf[cdc_off + 45U]); /* Data bInterfaceNumber */

    /* Verify patched MIDI Audio Control baInterfaceNr -> 5 */
    uint16_t midi_off = (uint16_t)(cdc_off + 66U + 25U);
    TEST_ASSERT_EQUAL_UINT8(4U, dev.config_buf[midi_off + 2U]);
    TEST_ASSERT_EQUAL_UINT8(5U, dev.config_buf[midi_off + 17U]);
    TEST_ASSERT_EQUAL_UINT8(5U, dev.config_buf[midi_off + 20U]);

    /* Verify setup packet routing to each composite interface + unmatched fallback */
    uint8_t resp[SYN_USB_EP0_BUF_SIZE];
    uint16_t rlen = 0U;
    SYN_USB_SetupPacket pkt = {
        .bmRequestType = 0xA1U, .bRequest = 0xFEU, .wValue = 0U, .wIndex = 0U, .wLength = 1U};
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(1U, rlen);

    pkt.bRequest = SYN_USB_CDC_GET_LINE_CODING;
    pkt.wIndex = 1U;
    pkt.wLength = 7U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(7U, rlen);

    pkt.bmRequestType = 0x81U;
    pkt.bRequest = SYN_USB_REQ_GET_DESCRIPTOR;
    pkt.wValue = (uint16_t)(0x22U << 8);
    pkt.wIndex = 3U;
    pkt.wLength = sizeof(hid_rep_desc);
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(sizeof(hid_rep_desc), rlen);

    /* Unmatched interface falls through to standard handler */
    pkt.bmRequestType = 0x21U;
    pkt.bRequest = 0x99U;
    pkt.wIndex = 99U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));

    /* Endpoint allocator edge cases & exhaustion rollback */
    uint8_t ep_out = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_alloc_ep(NULL, 0x81U, &ep_out));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_usb_alloc_ep(&dev, 0x81U, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_alloc_ep(&dev, 0x80U, &ep_out));
    TEST_ASSERT_EQUAL_HEX8(0x86U, ep_out);

    SYN_USB_Device dev2;
    syn_usb_init(&dev2, TEST_DEVICE_DESC);
    dev2.ep_in_mask = 0xFFFEU;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_usb_alloc_ep(&dev2, 0x81U, &ep_out));
    SYN_USB_HID hid2;
    syn_usb_hid_init(&hid2);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_usb_hid_register(&dev2, &hid2, hid_rep_desc, sizeof(hid_rep_desc)));
    TEST_ASSERT_EQUAL_UINT8(0U, dev2.class_count);

    /* Truncated trailing byte in custom descriptor breaks cleanly */
    dev2.ep_in_mask = 0U;
    SYN_USB_ClassDriver raw_cls = {.iface_start = 2U, .iface_count = 1U};
    uint8_t short_desc[10] = {0x09U, 0x04U, 0x00U, 0x00U, 0x00U, 0xFFU, 0x00U, 0x00U, 0x00U, 0x01U};
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_register_class(&dev2, &raw_cls, short_desc, sizeof(short_desc)));
    TEST_ASSERT_EQUAL_UINT8(2U, dev2.classes[0].iface_start);
}

void test_tier_a_usb_regressions(void)
{
    SYN_USB_Device dev;
    SYN_USB_CDC cdc;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_init(&dev, TEST_DEVICE_DESC));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_cdc_init(&cdc));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_cdc_register(&dev, &cdc));

    uint8_t resp[16] = {0xFFU, 0xFFU};
    uint16_t rlen = 0U;

    /* Standard GET_STATUS to Interface (0x81) stays in Chapter 9 core and returns 0x0000 */
    SYN_USB_SetupPacket pkt = {.bmRequestType = 0x81U,
                               .bRequest = SYN_USB_REQ_GET_STATUS,
                               .wValue = 0U,
                               .wIndex = 0U,
                               .wLength = 2U};
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(2U, rlen);
    TEST_ASSERT_EQUAL_UINT8(0x00U, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00U, resp[1]);

    /* Standard GET_STATUS to Endpoint (0x82) returns 0x0000; Device (0x80) returns 0x0001 */
    pkt.bmRequestType = 0x82U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT8(0x00U, resp[0]);

    pkt.bmRequestType = 0x80U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT8(0x01U, resp[0]);

    /* Vendor request to interface (0x41) is not misrouted to CDC class setup */
    pkt.bmRequestType = 0x41U;
    pkt.bRequest = SYN_USB_CDC_GET_LINE_CODING;
    pkt.wLength = 7U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT16(0U, rlen);

    /* USB-1: 75-byte CDC config descriptor rejected on 64B buffer, fits dev.ep0_buf (256B) */
    uint8_t small_ep0[64];
    pkt.bmRequestType = 0x80U;
    pkt.bRequest = SYN_USB_REQ_GET_DESCRIPTOR;
    pkt.wValue = (uint16_t)(SYN_USB_DESC_TYPE_CONFIGURATION << 8);
    pkt.wIndex = 0U;
    pkt.wLength = 255U;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_usb_process_setup(&dev, &pkt, small_ep0,
                                                           (uint16_t)sizeof(small_ep0), &rlen));
    TEST_ASSERT_EQUAL_UINT16(0U, rlen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_process_setup(&dev, &pkt, dev.ep0_buf,
                                                        (uint16_t)sizeof(dev.ep0_buf), &rlen));
    TEST_ASSERT_EQUAL_UINT16(75U, rlen);
}
