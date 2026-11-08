/**
 * @file fuzz_usb.c
 * @brief LibFuzzer harness for Zero-Heap Modular USB 2.0 Device Core, CDC, and HID class drivers.
 */

#include "syntropic/drivers/syn_usb.h"
#include "syntropic/drivers/syn_usb_cdc.h"
#include "syntropic/drivers/syn_usb_hid.h"
#include "syntropic/drivers/syn_usb_host.h"
#include "syntropic/drivers/syn_usb_host_cdc.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const uint8_t SAMPLE_DEVICE_DESC[18] = {0x12, 0x01, 0x00, 0x02, 0x00, 0x00,
                                               0x00, 0x40, 0xFE, 0xCA, 0xEF, 0xBE,
                                               0x00, 0x01, 0x01, 0x02, 0x00, 0x01};

static const uint8_t SAMPLE_REPORT_DESC[24] = {0x05, 0x01, 0x09, 0x00, 0xA1, 0x01, 0x09,
                                               0x01, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x75,
                                               0x08, 0x95, 0x08, 0x81, 0x02, 0xC0};

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 8) {
        return 0;
    }

    SYN_USB_Device dev;
    SYN_USB_CDC cdc;
    SYN_USB_HID hid;

    if (syn_usb_init(&dev, SAMPLE_DEVICE_DESC) != SYN_OK) {
        return 0;
    }
    if (syn_usb_cdc_init(&cdc) != SYN_OK) {
        return 0;
    }
    if (syn_usb_hid_init(&hid) != SYN_OK) {
        return 0;
    }

    syn_usb_cdc_register(&dev, &cdc);
    syn_usb_hid_register(&dev, &hid, SAMPLE_REPORT_DESC, sizeof(SAMPLE_REPORT_DESC));

    /* Interpret first 8 bytes as a USB Setup Packet */
    SYN_USB_SetupPacket pkt;
    pkt.bmRequestType = data[0];
    pkt.bRequest = data[1];
    pkt.wValue = (uint16_t)data[2] | ((uint16_t)data[3] << 8);
    pkt.wIndex = (uint16_t)data[4] | ((uint16_t)data[5] << 8);
    pkt.wLength = (uint16_t)data[6] | ((uint16_t)data[7] << 8);

    uint8_t resp_buf[256];
    uint16_t resp_len = 0;

    /* Fuzz core setup packet processing */
    syn_usb_process_setup(&dev, &pkt, resp_buf, (uint16_t)sizeof(resp_buf), &resp_len);

    /* Fuzz CDC setup processing */
    size_t cdc_rlen = 0;
    syn_usb_cdc_handle_setup(&cdc, &pkt, resp_buf, &cdc_rlen);

    /* Fuzz CDC read & write with remaining payload */
    if (size > 8) {
        const uint8_t *payload = &data[8];
        size_t payload_len = size - 8;

        syn_usb_cdc_write(&cdc, payload, payload_len);

        uint8_t read_out[128];
        size_t out_len = 0;
        syn_usb_cdc_read(&cdc, read_out, sizeof(read_out), &out_len);

        /* Fuzz HID report send & read */
        syn_usb_hid_send_report(&hid, payload, payload_len);
        syn_usb_hid_read_report(&hid, read_out, sizeof(read_out), &out_len);
    }

    /* Fuzz USB Host enumeration descriptor parsing */
    SYN_USB_Host host;
    SYN_USB_HostCDC hcdc;
    if (syn_usb_host_init(&host) == SYN_OK && syn_usb_host_cdc_init(&hcdc) == SYN_OK) {
        syn_usb_host_cdc_register(&host, &hcdc);

        if (size <= sizeof(host.enum_buf)) {
            memcpy(host.enum_buf, data, size);
            host.enum_buf_len = (uint16_t)size;
            host.state = SYN_USB_HOST_STATE_ENUMERATING;
            host.enum_step = SYN_USB_HOST_ENUM_CLASS_PROBE;

            /* Drive class probing with fuzzed descriptor bytes */
            syn_usb_host_process(&host);
        }
    }

    return 0;
}
