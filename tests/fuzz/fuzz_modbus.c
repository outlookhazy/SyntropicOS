#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "syntropic/proto/syn_modbus.h"
#include "tests/mocks/mock_port.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0 || size > 256) return 0;

    // Reset mock state
    mock_port_reset();

    SYN_Modbus mb;
    uint8_t mb_buf[260];
    uint16_t holding_regs[32] = {0};
    uint16_t input_regs[16] = {0};

    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .uart = 0,
        .holding_regs = holding_regs,
        .holding_count = 32,
        .input_regs = input_regs,
        .input_count = 16,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    // Manually inject the fuzzer data into the modbus RX buffer
    memcpy(mb.buf, data, size);
    mb.rx_len = size;

    // Run the processor
    syn_modbus_process(&mb);

    return 0;
}
