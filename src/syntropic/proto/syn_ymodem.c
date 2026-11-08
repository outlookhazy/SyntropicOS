#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_YMODEM) || SYN_USE_YMODEM

/**
 * @file syn_ymodem.c
 * @brief YMODEM / XMODEM-1K serial file transfer receiver implementation.
 */

#include "../util/syn_crc.h"
#include "syn_ymodem.h"

#include <string.h>

/** @brief Single byte receive timeout in milliseconds */
#define SYN_YMODEM_RECV_TIMEOUT_MS 1000U

/**
 * @brief Transmit a single byte via user callback.
 * @param rx  Receiver context.
 * @param b   Byte value to transmit.
 */
static void ymodem_send_byte(SYN_YMODEM_Receiver *rx, uint8_t b)
{
    rx->putchar_fn(b, rx->ctx);
}

/**
 * @brief Transmit a two-byte CAN abort sequence.
 * @param rx  Receiver context.
 */
static void ymodem_abort(SYN_YMODEM_Receiver *rx)
{
    ymodem_send_byte(rx, SYN_YMODEM_CAN);
    ymodem_send_byte(rx, SYN_YMODEM_CAN);
}

/**
 * @brief Receive a single byte via user callback.
 * @param rx          Receiver context.
 * @param timeout_ms  Timeout in milliseconds.
 * @return Received byte value (0-255) or negative on timeout/error.
 */
static int ymodem_recv_byte(SYN_YMODEM_Receiver *rx, uint32_t timeout_ms)
{
    return rx->getchar_fn(timeout_ms, rx->ctx);
}

/**
 * @brief Parse Block 0 payload extracting filename and bounded ASCII file size.
 * @param rx           Receiver context.
 * @param payload      Block 0 payload buffer.
 * @param payload_len  Block payload length in bytes.
 */
static void ymodem_parse_block0(SYN_YMODEM_Receiver *rx, const uint8_t *payload, size_t payload_len)
{
    rx->filename[0] = '\0';
    rx->filesize = 0U;

    const uint8_t *nul_ptr = (const uint8_t *)memchr(payload, '\0', payload_len);
    size_t true_fn_len = (nul_ptr != NULL) ? (size_t)(nul_ptr - payload) : payload_len;
    size_t copy_len = (true_fn_len < (SYN_YMODEM_MAX_FILENAME - 1U))
                          ? true_fn_len
                          : (SYN_YMODEM_MAX_FILENAME - 1U);

    memcpy(rx->filename, payload, copy_len);
    rx->filename[copy_len] = '\0';

    if (nul_ptr == NULL) {
        return;
    }

    size_t pos = true_fn_len + 1U;
    while ((pos < payload_len) && (payload[pos] == (uint8_t)' ')) {
        pos++;
    }

    uint32_t parsed = 0U;
    while ((pos < payload_len) && (payload[pos] >= (uint8_t)'0') &&
           (payload[pos] <= (uint8_t)'9')) {
        uint32_t digit = (uint32_t)(payload[pos] - (uint8_t)'0');
        if (parsed > ((UINT32_MAX - digit) / 10U)) {
            parsed = UINT32_MAX;
        } else {
            parsed = (parsed * 10U) + digit;
        }
        pos++;
    }
    rx->filesize = parsed;
}

/**
 * @brief Read and validate a framed YMODEM packet.
 * @param rx               Receiver context.
 * @param header_byte      Packet header byte (SOH or STX).
 * @param out_payload_len  Output pointer for payload byte count.
 * @return Sequence number (0-255) on success, or negative status error code.
 */
static int ymodem_read_packet(SYN_YMODEM_Receiver *rx, uint8_t header_byte, size_t *out_payload_len)
{
    size_t payload_len = (header_byte == SYN_YMODEM_STX) ? 1024U : 128U;
    size_t total_len = payload_len + 5U;

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

    uint16_t computed_crc = syn_crc16_ccitt_update(0x0000U, &rx->pkt_buf[3], payload_len);
    uint16_t rx_crc =
        (uint16_t)(((uint16_t)rx->pkt_buf[3U + payload_len] << 8) | rx->pkt_buf[4U + payload_len]);
    if (computed_crc != rx_crc) {
        return SYN_YMODEM_ERR_CRC;
    }

    *out_payload_len = payload_len;
    return (int)seq;
}

/**
 * @brief Negotiate CRC mode and process YMODEM Block 0 header.
 * @param rx            Receiver context.
 * @param session_done  [out] Set to true when empty Block 0 terminates session.
 * @return SYN_YMODEM_OK on success, or negative error status.
 */
static SYN_YMODEM_Status ymodem_handshake_block0(SYN_YMODEM_Receiver *rx, bool *session_done)
{
    uint8_t retries = 0U;
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
    if (first_byte == (int)SYN_YMODEM_CAN) {
        return SYN_YMODEM_ERR_CANCEL;
    }
    if ((first_byte != (int)SYN_YMODEM_SOH) && (first_byte != (int)SYN_YMODEM_STX)) {
        ymodem_abort(rx);
        return SYN_YMODEM_ERR_SEQUENCE;
    }

    size_t payload_len = 0U;
    int seq_or_err = ymodem_read_packet(rx, (uint8_t)first_byte, &payload_len);
    if (seq_or_err < 0) {
        ymodem_abort(rx);
        return (SYN_YMODEM_Status)seq_or_err;
    }
    if (seq_or_err != 0) {
        ymodem_abort(rx);
        return SYN_YMODEM_ERR_SEQUENCE;
    }

    if (rx->pkt_buf[3] == (uint8_t)'\0') {
        ymodem_send_byte(rx, SYN_YMODEM_ACK);
        if (rx->event_fn != NULL) {
            (void)rx->event_fn(SYN_YMODEM_EVENT_SESSION_END, NULL, 0U, rx->ctx);
        }
        *session_done = true;
        return SYN_YMODEM_OK;
    }

    ymodem_parse_block0(rx, &rx->pkt_buf[3], payload_len);
    if (rx->event_fn != NULL) {
        int cb_res = rx->event_fn(SYN_YMODEM_EVENT_FILE_START, (const uint8_t *)rx->filename,
                                  strlen(rx->filename), rx->ctx);
        if (cb_res != 0) {
            ymodem_abort(rx);
            return SYN_YMODEM_ERR_CANCEL;
        }
    }

    ymodem_send_byte(rx, SYN_YMODEM_ACK);
    ymodem_send_byte(rx, SYN_YMODEM_CRC);
    *session_done = false;
    return SYN_YMODEM_OK;
}

/**
 * @brief Complete EOT handshake and emit FILE_END callback.
 * @param rx Receiver context.
 */
static void ymodem_finish_eot(SYN_YMODEM_Receiver *rx)
{
    ymodem_send_byte(rx, SYN_YMODEM_NAK);
    (void)ymodem_recv_byte(rx, SYN_YMODEM_RECV_TIMEOUT_MS);
    ymodem_send_byte(rx, SYN_YMODEM_ACK);

    if (rx->event_fn != NULL) {
        (void)rx->event_fn(SYN_YMODEM_EVENT_FILE_END, NULL, 0U, rx->ctx);
    }
}

/**
 * @brief Deliver validated data block payload to application and advance sequence.
 * @param rx               Receiver context.
 * @param data_payload_len Raw block payload size (128 or 1024).
 * @return SYN_YMODEM_OK on success, or SYN_YMODEM_ERR_CANCEL if callback aborts.
 */
static SYN_YMODEM_Status ymodem_process_data_block(SYN_YMODEM_Receiver *rx, size_t data_payload_len)
{
    size_t chunk_len = data_payload_len;
    if (rx->filesize > 0U) {
        uint32_t remaining =
            (rx->bytes_received < rx->filesize) ? (rx->filesize - rx->bytes_received) : 0U;
        if (remaining < chunk_len) {
            chunk_len = (size_t)remaining;
        }
    }

    if ((rx->event_fn != NULL) && (chunk_len > 0U)) {
        int cb_res = rx->event_fn(SYN_YMODEM_EVENT_DATA, &rx->pkt_buf[3], chunk_len, rx->ctx);
        if (cb_res != 0) {
            ymodem_abort(rx);
            return SYN_YMODEM_ERR_CANCEL;
        }
    }

    rx->bytes_received += (uint32_t)chunk_len;
    rx->expected_seq++;
    ymodem_send_byte(rx, SYN_YMODEM_ACK);
    return SYN_YMODEM_OK;
}

/**
 * @brief Receive all data packets for a single file transfer with bounded retries.
 * @param rx Receiver context.
 * @return SYN_YMODEM_OK on EOT completion, or negative error status.
 */
static SYN_YMODEM_Status ymodem_receive_file_data(SYN_YMODEM_Receiver *rx)
{
    rx->expected_seq = 1U;
    rx->bytes_received = 0U;
    uint8_t retries = 0U;

    for (;;) {
        int header_byte = ymodem_recv_byte(rx, SYN_YMODEM_RECV_TIMEOUT_MS);
        if (header_byte < 0) {
            ymodem_abort(rx);
            return SYN_YMODEM_ERR_TIMEOUT;
        }

        if (header_byte == (int)SYN_YMODEM_CAN) {
            if (ymodem_recv_byte(rx, 100U) == (int)SYN_YMODEM_CAN) {
                return SYN_YMODEM_ERR_CANCEL;
            }
        }

        if (header_byte == (int)SYN_YMODEM_EOT) {
            ymodem_finish_eot(rx);
            return SYN_YMODEM_OK;
        }

        int block_seq = -1;
        size_t data_payload_len = 0U;
        if ((header_byte == (int)SYN_YMODEM_SOH) || (header_byte == (int)SYN_YMODEM_STX)) {
            block_seq = ymodem_read_packet(rx, (uint8_t)header_byte, &data_payload_len);
            if (block_seq == SYN_YMODEM_ERR_TIMEOUT) {
                ymodem_abort(rx);
                return SYN_YMODEM_ERR_TIMEOUT;
            }
        }

        if (block_seq < 0) {
            retries++;
            if (retries >= SYN_YMODEM_MAX_RETRIES) {
                ymodem_abort(rx);
                return SYN_YMODEM_ERR_CRC;
            }
            ymodem_send_byte(rx, SYN_YMODEM_NAK);
            continue;
        }

        retries = 0U;
        if ((uint8_t)block_seq == (uint8_t)(rx->expected_seq - 1U)) {
            ymodem_send_byte(rx, SYN_YMODEM_ACK);
            continue;
        }
        if ((uint8_t)block_seq != rx->expected_seq) {
            ymodem_abort(rx);
            return SYN_YMODEM_ERR_SEQUENCE;
        }

        SYN_YMODEM_Status st = ymodem_process_data_block(rx, data_payload_len);
        if (st != SYN_YMODEM_OK) {
            return st;
        }
    }
}

void syn_ymodem_receiver_init(SYN_YMODEM_Receiver *rx, SYN_YMODEM_PutChar putchar_fn,
                              SYN_YMODEM_GetChar getchar_fn, SYN_YMODEM_EventCallback event_fn,
                              void *ctx)
{
    if (rx == NULL) {
        return;
    }

    memset(rx, 0, sizeof(SYN_YMODEM_Receiver));
    rx->putchar_fn = putchar_fn;
    rx->getchar_fn = getchar_fn;
    rx->event_fn = event_fn;
    rx->ctx = ctx;
}

SYN_YMODEM_Status syn_ymodem_receive(SYN_YMODEM_Receiver *rx)
{
    if ((rx == NULL) || (rx->putchar_fn == NULL) || (rx->getchar_fn == NULL)) {
        return SYN_YMODEM_ERR_PARAM;
    }

    for (;;) {
        bool session_done = false;
        SYN_YMODEM_Status st = ymodem_handshake_block0(rx, &session_done);
        if (st != SYN_YMODEM_OK) {
            return st;
        }
        if (session_done) {
            return SYN_YMODEM_OK;
        }

        st = ymodem_receive_file_data(rx);
        if (st != SYN_YMODEM_OK) {
            return st;
        }
    }
}

#endif /* SYN_USE_YMODEM */
