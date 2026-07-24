

# File syn\_modbus\_tcp.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_modbus\_tcp.h**](syn__modbus__tcp_8h.md)

[Go to the documentation of this file](syn__modbus__tcp_8h.md)


```C++

#ifndef SYN_MODBUS_TCP_H
#define SYN_MODBUS_TCP_H

#include "../common/syn_defs.h"
#include "syn_modbus.h"
#include "syn_modbus_master.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_MBAP_HEADER_LEN   7     
#define SYN_MB_TCP_MAX_ADU    260   
typedef struct {
    uint16_t transaction_id;  
    uint16_t protocol_id;     
    uint16_t length;          
    uint8_t  unit_id;         
} SYN_MBAP_Header;

void syn_mbap_encode_header(const SYN_MBAP_Header *hdr, uint8_t *buf);

bool syn_mbap_decode_header(const uint8_t *buf, SYN_MBAP_Header *hdr);

bool syn_modbus_tcp_process_slave(SYN_Modbus *mb,
                                  const uint8_t *req_adu, uint16_t req_len,
                                  uint8_t *resp_adu, uint16_t resp_max,
                                  uint16_t *resp_len);

SYN_Status syn_modbus_tcp_build_client_adu(uint16_t transaction_id, uint8_t unit_id,
                                           const uint8_t *pdu, uint16_t pdu_len,
                                           uint8_t *req_adu, uint16_t req_max,
                                           uint16_t *req_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MODBUS_TCP_H */
```


