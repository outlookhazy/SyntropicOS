

# File syn\_ymodem.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ymodem.c**](syn__ymodem_8c.md)

[Go to the documentation of this file](syn__ymodem_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_YMODEM) || SYN_USE_YMODEM

#include "../util/syn_assert.h"
#include "../util/syn_crc.h"
#include "syn_ymodem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYN_YMODEM_RECV_TIMEOUT_MS 1000U

static void ymodem_send_byte(SYN_YMODEM_Receiver *rx, uint8_t b)
{
    if (rx != NULL && rx->putchar_fn != NULL) {
        rx->putchar_fn(b, rx->ctx);
    }
}

static int ymodem_recv_byte(SYN_YMODEM_Receiver *rx, uint32_t timeout_ms)
{
    return rx->getchar_fn(timeout_ms, rx->ctx);
}

static void ymodem_parse_block0(SYN_YMODEM_Receiver *rx, const uint8_t *payload, size_t payload_len)
{
    rx->filename[0] = '\0';
    rx->filesize = 0;

    /* Extract NULL-terminated filename */
    size_t fn_len = 0;
    while (fn_len < payload_len && payload[fn_len] != '\0' &&
           fn_len < (SYN_YMODEM_MAX_FILENAME - 1U)) {
        rx->filename[fn_len] = (char)payload[fn_len];
        fn_len++;
    }
    rx->filename[fn_len] = '\0';

    /* Extract file size (ASCII space-delimited decimal after filename) */
    size_t size_pos = fn_len + 1U;
    if (size_pos < payload_len && payload[size_pos] != '\0') {
        rx->filesize = (uint32_t)strtoul((const char *)&payload[size_pos], NULL, 10);
    }
}

static int ymodem_read_packet(SYN_YMODEM_Receiver *rx, uint8_t header_byte, size_t *out_payload_len)
{
    size_t payload_len = (header_byte == SYN_YMODEM_STX) ? 1024U : 128U;
    size_t total_len = payload_len + 5U; /* header + seq + ~seq + payload + 2B CRC */

    rx->pkt_buf[0] = header_byte;

    for (size_t i = 1U; i < total_len; i++) {
        int b = ymodem_recv_byte(rx, SYN_YMODEM_RECV_TIMEOUT_MS);
        if (b < 0) {
            return SYN_YMODEM_ERR_TIMEOUT;
        }
        rx->pkt_buf[i] = (uint8_t)b;
    }

    uint8_t seq = rx->pkt_buf[1];
    uint8_t seq_inv = rx->pkt_buf[2];

    if ((uint8_t)(seq + seq_inv) != 0xFFU) {
        return SYN_YMODEM_ERR_SEQUENCE;
    }

    /* Compute CRC-16/XMODEM (poly 0x1021, init 0x0000) over payload */
    uint16_t computed_crc = syn_crc16_ccitt_update(0x0000U, &rx->pkt_buf[3], payload_len);
    uint16_t rx_crc =
        (uint16_t)(((uint16_t)rx->pkt_buf[3U + payload_len] << 8) | rx->pkt_buf[4U + payload_len]);

    if (computed_crc != rx_crc) {
        return SYN_YMODEM_ERR_CRC;
    }

    *out_payload_len = payload_len;
    return (int)seq;
}

void syn_ymodem_receiver_init(SYN_YMODEM_Receiver *rx, SYN_YMODEM_PutChar putchar_fn,
                              SYN_YMODEM_GetChar getchar_fn, SYN_YMODEM_EventCallback event_fn,
                              void *ctx)
{
    if (rx == NULL) {
        return;
    }
    SYN_ASSERT(rx != NULL);

    memset(rx, 0, sizeof(SYN_YMODEM_Receiver));
    rx->putchar_fn = putchar_fn;
    rx->getchar_fn = getchar_fn;
    rx->event_fn = event_fn;
    rx->ctx = ctx;
}

SYN_YMODEM_Status syn_ymodem_receive(SYN_YMODEM_Receiver *rx)
{
    if (rx == NULL || rx->putchar_fn == NULL || rx->getchar_fn == NULL) {
        return SYN_YMODEM_ERR_PARAM;
    }

    bool session_active = true;

    while (session_active) {
        /* Initiate transfer by requesting CRC-16 mode */
        uint8_t retries = 0;
        int first_byte = -1;

        while (retries < SYN_YMODEM_MAX_RETRIES) {
            ymodem_send_byte(rx, SYN_YMODEM_CRC);
            first_byte = ymodem_recv_byte(rx, SYN_YMODEM_RECV_TIMEOUT_MS);
            if (first_byte >= 0) {
                break;
            }
            retries++;
        }

        if (first_byte < 0) {
            return SYN_YMODEM_ERR_TIMEOUT;
        }

        /* Check for initial Cancel request */
        if (first_byte == SYN_YMODEM_CAN) {
            return SYN_YMODEM_ERR_CANCEL;
        }

        /* Expect Block 0 (SOH or STX) */
        if (first_byte != SYN_YMODEM_SOH && first_byte != SYN_YMODEM_STX) {
            return SYN_YMODEM_ERR_SEQUENCE;
        }

        size_t payload_len = 0;
        int seq_or_err = ymodem_read_packet(rx, (uint8_t)first_byte, &payload_len);
        if (seq_or_err < 0) {
            ymodem_send_byte(rx, SYN_YMODEM_CAN);
            return (SYN_YMODEM_Status)seq_or_err;
        }

        if (seq_or_err != 0) {
            ymodem_send_byte(rx, SYN_YMODEM_CAN);
            return SYN_YMODEM_ERR_SEQUENCE;
        }

        /* Parse Block 0 header */
        if (rx->pkt_buf[3] == '\0') {
            /* Empty Block 0 -> End of session */
            ymodem_send_byte(rx, SYN_YMODEM_ACK);
            if (rx->event_fn != NULL) {
                rx->event_fn(SYN_YMODEM_EVENT_SESSION_END, NULL, 0, rx->ctx);
            }
            break;
        }

        ymodem_parse_block0(rx, &rx->pkt_buf[3], payload_len);

        /* Valid Block 0 -> Fire FILE_START and send ACK + 'C' */
        if (rx->event_fn != NULL) {
            int cb_res = rx->event_fn(SYN_YMODEM_EVENT_FILE_START, (const uint8_t *)rx->filename,
                                      strlen(rx->filename), rx->ctx);
            if (cb_res != 0) {
                ymodem_send_byte(rx, SYN_YMODEM_CAN);
                return SYN_YMODEM_ERR_CANCEL;
            }
        }

        ymodem_send_byte(rx, SYN_YMODEM_ACK);
        ymodem_send_byte(rx, SYN_YMODEM_CRC);

        /* Receive data blocks */
        rx->expected_seq = 1U;
        rx->bytes_received = 0U;
        bool file_active = true;

        while (file_active) {
            int header_byte = ymodem_recv_byte(rx, SYN_YMODEM_RECV_TIMEOUT_MS);
            if (header_byte < 0) {
                return SYN_YMODEM_ERR_TIMEOUT;
            }

            if (header_byte == SYN_YMODEM_CAN) {
                int second_can = ymodem_recv_byte(rx, 100U);
                if (second_can == SYN_YMODEM_CAN) {
                    return SYN_YMODEM_ERR_CANCEL;
                }
            }

            if (header_byte == SYN_YMODEM_EOT) {
                /* End of transmission sequence: send NAK, wait for 2nd EOT, then ACK */
                ymodem_send_byte(rx, SYN_YMODEM_NAK);
                int second_eot = ymodem_recv_byte(rx, SYN_YMODEM_RECV_TIMEOUT_MS);
                if (second_eot == SYN_YMODEM_EOT) {
                    ymodem_send_byte(rx, SYN_YMODEM_ACK);
                } else {
                    ymodem_send_byte(rx, SYN_YMODEM_ACK);
                }

                if (rx->event_fn != NULL) {
                    rx->event_fn(SYN_YMODEM_EVENT_FILE_END, NULL, 0, rx->ctx);
                }
                break;
            }

            if (header_byte != SYN_YMODEM_SOH && header_byte != SYN_YMODEM_STX) {
                ymodem_send_byte(rx, SYN_YMODEM_NAK);
                continue;
            }

            size_t data_payload_len = 0;
            int block_seq = ymodem_read_packet(rx, (uint8_t)header_byte, &data_payload_len);

            if (block_seq < 0) {
                ymodem_send_byte(rx, SYN_YMODEM_NAK);
                continue;
            }

            if ((uint8_t)block_seq == (uint8_t)(rx->expected_seq - 1U)) {
                /* Duplicate packet — ACK and discard */
                ymodem_send_byte(rx, SYN_YMODEM_ACK);
                continue;
            }

            if ((uint8_t)block_seq != rx->expected_seq) {
                ymodem_send_byte(rx, SYN_YMODEM_CAN);
                return SYN_YMODEM_ERR_SEQUENCE;
            }

            /* Calculate chunk size based on remaining filesize */
            size_t chunk_len = data_payload_len;
            if (rx->filesize > 0U) {
                uint32_t remaining = rx->filesize - rx->bytes_received;
                if (remaining < chunk_len) {
                    chunk_len = (size_t)remaining;
                }
            }

            if (rx->event_fn != NULL && chunk_len > 0U) {
                int cb_res =
                    rx->event_fn(SYN_YMODEM_EVENT_DATA, &rx->pkt_buf[3], chunk_len, rx->ctx);
                if (cb_res != 0) {
                    ymodem_send_byte(rx, SYN_YMODEM_CAN);
                    return SYN_YMODEM_ERR_CANCEL;
                }
            }

            rx->bytes_received += (uint32_t)chunk_len;
            rx->expected_seq++;
            ymodem_send_byte(rx, SYN_YMODEM_ACK);
        }
    }

    return SYN_YMODEM_OK;
}

#endif /* SYN_USE_YMODEM */
```


