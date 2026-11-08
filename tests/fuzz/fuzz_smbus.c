/**
 * @file fuzz_smbus.c
 * @brief libFuzzer target for syn_smbus frame decoder.
 */

#include "syntropic/proto/syn_smbus.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size == 0U) {
        return 0;
    }

    SYN_SMBUS_Packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.slave_addr = 0x16U;
    pkt.command = 0x08U;
    syn_smbus_decode_packet(&pkt, data, size, SYN_SMBUS_PROTO_WRITE_WORD, true);
    syn_smbus_decode_packet(&pkt, data, size, SYN_SMBUS_PROTO_READ_BLOCK, false);
    syn_smbus_decode_packet(&pkt, data, size, SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL, true);

    return 0;
}
