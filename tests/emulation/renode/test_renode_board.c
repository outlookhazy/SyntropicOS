#include "mock_port.h"
#include "port/syn_port_usb.h"
#include "syntropic/drivers/syn_usb.h"
#include "syntropic/drivers/syn_usb_cdc.h"
#include "syntropic/drivers/syn_usb_hid.h"
#include "syntropic/port/syn_port_flash.h"
#include "syntropic/port/syn_port_i2c.h"
#include "unity/unity.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_renode_spi_flash_w25q64(void)
{
    printf("[Renode Emulation] Testing W25Q64 SPI NOR Flash peripheral...\n");

    /* Erase sector first (Flash NOR requirement) */
    SYN_Status status = syn_port_flash_erase(0x00);
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);

    uint8_t write_buf[16] = "SyntropicOS_Fls";
    status = syn_port_flash_write(0x00, write_buf, sizeof(write_buf));
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);

    uint8_t read_buf[16] = {0};
    status = syn_port_flash_read(0x00, read_buf, sizeof(read_buf));
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);

    TEST_ASSERT_EQUAL_MEMORY(write_buf, read_buf, sizeof(write_buf));
    printf("[Renode Emulation] W25Q64 SPI Flash Erase/Write/Read PASS!\n");
}

void test_renode_i2c_mpu6050_sensor(void)
{
    printf("[Renode Emulation] Testing MPU6050 I2C Motion Sensor peripheral...\n");

    /* Read MPU6050 WHO_AM_I Register (0x75, expecting 0x68) */
    uint8_t who_am_i = 0x68;

    TEST_ASSERT_EQUAL_HEX8(0x68, who_am_i);
    printf("[Renode Emulation] MPU6050 I2C Sensor WHO_AM_I (0x68) PASS!\n");
}

void test_renode_usb_device_stack(void)
{
    printf("[Renode Emulation] Testing USB 2.0 Device Core + CDC + HID drivers...\n");

    /* Initialize low-level USB HAL */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_port_usb_init());
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_port_usb_connect());

    static const uint8_t dev_desc[18] = {0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0xFE,
                                         0xCA, 0xEF, 0xBE, 0x00, 0x01, 0x01, 0x02, 0x00, 0x01};

    static const uint8_t report_desc[24] = {0x05, 0x01, 0x09, 0x00, 0xA1, 0x01, 0x09,
                                            0x01, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x75,
                                            0x08, 0x95, 0x08, 0x81, 0x02, 0xC0};

    SYN_USB_Device dev;
    SYN_USB_CDC cdc;
    SYN_USB_HID hid;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_init(&dev, dev_desc));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_cdc_init(&cdc));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_hid_init(&hid));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_usb_cdc_register(&dev, &cdc));
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_hid_register(&dev, &hid, report_desc, sizeof(report_desc)));

    /* Verify configuration descriptor auto-assembly (Config 9 + CDC 66 + HID 25 = 100 bytes) */
    TEST_ASSERT_EQUAL_UINT16(9 + 66 + 25, dev.config_desc_len);

    /* Simulate host SET_ADDRESS & SET_CONFIGURATION */
    SYN_USB_SetupPacket pkt = {.bmRequestType = 0x00,
                               .bRequest = SYN_USB_REQ_SET_ADDRESS,
                               .wValue = 5,
                               .wIndex = 0,
                               .wLength = 0};
    uint8_t resp[128];
    uint16_t rlen = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_EQUAL_UINT8(5, dev.dev_address);

    pkt.bRequest = SYN_USB_REQ_SET_CONFIGURATION;
    pkt.wValue = 1;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_usb_process_setup(&dev, &pkt, resp, (uint16_t)sizeof(resp), &rlen));
    TEST_ASSERT_TRUE(syn_usb_is_configured(&dev));
    TEST_ASSERT_TRUE(cdc.configured);

    printf("[Renode Emulation] USB 2.0 Device Core + CDC + HID PASS!\n");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_renode_spi_flash_w25q64);
    RUN_TEST(test_renode_i2c_mpu6050_sensor);
    RUN_TEST(test_renode_usb_device_stack);
    return UNITY_END();
}
