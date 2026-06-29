*** Keywords ***
Create Machine
    Execute Command         mach create "syntropic-test"
    Execute Command         machine LoadPlatformDescription @${CURDIR}/syn_test_spiflash.repl
    Execute Command         sysbus LoadELF @${CURDIR}/../../test_firmware.elf
    Execute Command         showAnalyzer sysbus.usart2

*** Test Cases ***
Unity Tests Should All Pass On STM32F4
    [Documentation]         Boots Unity test firmware on simulated STM32F4 Discovery
    ...                     and verifies all tests pass by parsing UART output.
    Create Machine
    Create Terminal Tester  sysbus.usart2   timeout=5

    Start Emulation

    # Wait for each test to report
    Wait For Line On Uart   test_gpio_output_readback
    Wait For Line On Uart   test_gpio_multiple_pins
    Wait For Line On Uart   test_systick_advancing
    Wait For Line On Uart   test_ringbuf_onmcu
    Wait For Line On Uart   test_cobs_onmcu
    Wait For Line On Uart   test_crc_onmcu
    Wait For Line On Uart   test_flash_erase_write_read
    Wait For Line On Uart   test_adc_reading
    Wait For Line On Uart   test_spi_flash_jedec_id
    Wait For Line On Uart   test_fsm_onmcu
    Wait For Line On Uart   test_basic_protothread
    Wait For Line On Uart   test_wait_until
    Wait For Line On Uart   test_delay_ms
    Wait For Line On Uart   test_scheduler
    Wait For Line On Uart   test_suspend_resume
    Wait For Line On Uart   test_software_timer
    Wait For Line On Uart   test_timeout_onmcu

    # Wait for summary line
    Wait For Line On Uart   0 Failures
