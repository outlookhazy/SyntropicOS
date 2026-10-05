

# File syn\_ntp\_server.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_ntp\_server.c**](syn__ntp__server_8c.md)

[Go to the documentation of this file](syn__ntp__server_8c.md)


```C++

#include "syn_ntp_server.h"

#include <string.h>

static void write_u32_be(uint8_t *buf, uint32_t val)
{
    buf[0] = (uint8_t)((val >> 24) & 0xFFU);
    buf[1] = (uint8_t)((val >> 16) & 0xFFU);
    buf[2] = (uint8_t)((val >> 8) & 0xFFU);
    buf[3] = (uint8_t)(val & 0xFFU);
}

SYN_Status syn_ntp_server_init(SYN_NTPServer *server, uint8_t stratum,
                               uint32_t (*get_epoch_sec_cb)(void))
{
    if (server == NULL) {
        return SYN_INVALID_PARAM;
    }
    if ((stratum == 0U) || (stratum > 15U)) {
        return SYN_INVALID_PARAM;
    }

    memset(server, 0, sizeof(*server));
    server->stratum = stratum;
    server->get_epoch_sec_cb = get_epoch_sec_cb;

    return SYN_OK;
}

SYN_Status syn_ntp_server_process_packet(SYN_NTPServer *server, const uint8_t request_pkt[48],
                                         uint8_t response_pkt[48])
{
    if ((server == NULL) || (request_pkt == NULL) || (response_pkt == NULL)) {
        return SYN_INVALID_PARAM;
    }

    /* Extract Leap Indicator (LI), Version Number (VN), Mode from client packet */
    uint8_t mode = request_pkt[0] & 0x07U;
    if ((mode != 3U) && (mode != 1U)) {
        /* Not a client request (mode 3) or symmetric active (mode 1) */
        return SYN_ERROR;
    }

    memset(response_pkt, 0, 48);

    /* LI = 0 (no warning), VN = 4 (NTPv4), Mode = 4 (Server response) => 0x24 */
    response_pkt[0] = 0x24U;
    response_pkt[1] = server->stratum;
    response_pkt[2] = 6U;           /* Poll interval (64 sec) */
    response_pkt[3] = (uint8_t)-20; /* Precision ~1 microsecond (2^-20 s) */

    /* Root Delay (0) & Root Dispersion (0) */
    write_u32_be(&response_pkt[4], 0);
    write_u32_be(&response_pkt[8], 0);

    /* Reference ID: "LOCL" for local clock */
    response_pkt[12] = 'L';
    response_pkt[13] = 'O';
    response_pkt[14] = 'C';
    response_pkt[15] = 'L';

    uint32_t now_utc = 0;
    if (server->get_epoch_sec_cb != NULL) {
        now_utc = server->get_epoch_sec_cb();
    }
    uint32_t ntp_sec = now_utc + SYN_NTP_EPOCH_OFFSET;

    /* Reference Timestamp (bytes 16..23) */
    write_u32_be(&response_pkt[16], ntp_sec);
    write_u32_be(&response_pkt[20], 0);

    /* Originate Timestamp (bytes 24..31) <- Copy client transmit timestamp from bytes 40..47 */
    memcpy(&response_pkt[24], &request_pkt[40], 8);

    /* Receive Timestamp (bytes 32..39) */
    write_u32_be(&response_pkt[32], ntp_sec);
    write_u32_be(&response_pkt[36], 0);

    /* Transmit Timestamp (bytes 40..47) */
    write_u32_be(&response_pkt[40], ntp_sec);
    write_u32_be(&response_pkt[44], 0);

    return SYN_OK;
}
```


