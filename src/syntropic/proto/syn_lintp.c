/**
 * @file syn_lintp.c
 * @brief ISO 17987-2 (LIN Transport Layer & Network Layer Services / UDSonLIN).
 *
 * Implements non-blocking, zero-allocation ISO 17987-2 LIN TP segmentation and
 * reassembly state machine.
 */

#include "syn_lintp.h"

#if !defined(SYN_USE_LINTP) || SYN_USE_LINTP

#include <string.h>

void syn_lintp_init(SYN_LINTP_Link *link, uint8_t nad, uint8_t *rx_buf, size_t rx_size,
                    uint8_t *tx_buf, size_t tx_size)
{
    if (link == NULL) {
        return;
    }

    memset(link, 0, sizeof(*link));
    link->nad = nad;
    link->padding_byte = 0xFFU;
    link->rx_buf = rx_buf;
    link->rx_buf_size = rx_size;
    link->tx_buf = tx_buf;
    link->tx_size = tx_size;
    link->timer_n_as_ms = 1000U;
    link->timer_n_cr_ms = 1000U;
    link->rx_state = SYN_LINTP_STATE_IDLE;
    link->tx_state = SYN_LINTP_STATE_IDLE;
}

void syn_lintp_set_timeouts(SYN_LINTP_Link *link, uint32_t n_as_ms, uint32_t n_cr_ms)
{
    if (link != NULL) {
        link->timer_n_as_ms = (n_as_ms > 0U) ? n_as_ms : 1000U;
        link->timer_n_cr_ms = (n_cr_ms > 0U) ? n_cr_ms : 1000U;
    }
}

void syn_lintp_set_padding(SYN_LINTP_Link *link, uint8_t pad_byte)
{
    if (link != NULL) {
        link->padding_byte = pad_byte;
    }
}

SYN_Status syn_lintp_send(SYN_LINTP_Link *link, uint8_t nad, const uint8_t *payload, size_t len)
{
    if (link == NULL || payload == NULL || len == 0U || len > 4095U) {
        return SYN_INVALID_PARAM;
    }

    if (len > 6U && (link->tx_buf == NULL || len > link->tx_size)) {
        return SYN_INVALID_PARAM;
    }

    if (link->tx_state != SYN_LINTP_STATE_IDLE || link->tx_frame_pending) {
        return SYN_BUSY;
    }

    link->tx_nad = nad;
    link->tx_total_len = len;
    link->tx_offset = 0U;
    link->tx_sn = 1U;
    link->tx_timer_ms = 0U;

    memset(link->tx_frame, link->padding_byte, 8U);
    link->tx_frame[0] = nad;

    if (len <= 6U) {
        if (link->tx_buf != NULL && len <= link->tx_size) {
            memcpy(link->tx_buf, payload, len);
        }
        link->tx_frame[1] = (uint8_t)(SYN_LINTP_PCI_SF | (len & 0x0FU));
        memcpy(&link->tx_frame[2], payload, len);
        link->tx_offset = len;
        link->tx_state = SYN_LINTP_STATE_IDLE;
    } else {
        memcpy(link->tx_buf, payload, len);
        link->tx_frame[1] = (uint8_t)(0x10U | ((len >> 8U) & 0x0FU));
        link->tx_frame[2] = (uint8_t)(len & 0xFFU);
        memcpy(&link->tx_frame[3], link->tx_buf, 5U);
        link->tx_offset = 5U;
        link->tx_state = SYN_LINTP_STATE_TRANSMITTING_CF;
    }

    link->tx_frame_pending = true;
    return SYN_OK;
}

bool syn_lintp_get_tx_frame(SYN_LINTP_Link *link, uint8_t out_frame[8])
{
    if (link == NULL || out_frame == NULL || !link->tx_frame_pending) {
        return false;
    }

    memcpy(out_frame, link->tx_frame, 8U);
    link->tx_frame_pending = false;
    link->tx_timer_ms = 0U;

    if (link->tx_state == SYN_LINTP_STATE_TRANSMITTING_CF) {
        if (link->tx_offset < link->tx_total_len) {
            memset(link->tx_frame, link->padding_byte, 8U);
            link->tx_frame[0] = link->tx_nad;
            link->tx_frame[1] = (uint8_t)(0x20U | (link->tx_sn & 0x0FU));

            size_t bytes_to_copy = link->tx_total_len - link->tx_offset;
            if (bytes_to_copy > 6U) {
                bytes_to_copy = 6U;
            }

            memcpy(&link->tx_frame[2], &link->tx_buf[link->tx_offset], bytes_to_copy);
            link->tx_offset += bytes_to_copy;
            link->tx_sn = (uint8_t)((link->tx_sn + 1U) & 0x0FU);
            link->tx_frame_pending = true;
        } else {
            link->tx_state = SYN_LINTP_STATE_IDLE;
        }
    }

    return true;
}

/**
 * @brief Process an inbound LIN-TP Consecutive Frame (CF) and advance multi-frame RX state.
 * @param link LIN-TP link instance.
 * @param frame_nad Node Address for Diagnostics from frame byte 0.
 * @param frame 8-byte LIN diagnostic frame payload.
 */
static void lintp_rx_cf(SYN_LINTP_Link *link, uint8_t frame_nad, const uint8_t frame[8])
{
    if (link->rx_state != SYN_LINTP_STATE_RECEIVING_CF || frame_nad != link->rx_nad) {
        return;
    }

    uint8_t sn = (uint8_t)(frame[1] & 0x0FU);
    if (sn != link->rx_sn) {
        link->rx_state = SYN_LINTP_STATE_IDLE;
        link->rx_offset = 0U;
        return;
    }

    size_t remaining = link->rx_total_len - link->rx_offset;
    size_t copy_bytes = (remaining > 6U) ? 6U : remaining;

    memcpy(&link->rx_buf[link->rx_offset], &frame[2], copy_bytes);
    link->rx_offset += copy_bytes;
    link->rx_sn = (uint8_t)((link->rx_sn + 1U) & 0x0FU);
    link->rx_timer_ms = 0U;

    if (link->rx_offset >= link->rx_total_len) {
        link->rx_completed = true;
        link->rx_state = SYN_LINTP_STATE_IDLE;
    }
}

/**
 * @brief Process an inbound LIN-TP Single Frame (SF) and mark reception complete.
 * @param link LIN-TP link instance.
 * @param frame_nad Node Address for Diagnostics from frame byte 0.
 * @param frame 8-byte LIN diagnostic frame payload.
 */
static void lintp_rx_sf(SYN_LINTP_Link *link, uint8_t frame_nad, const uint8_t frame[8])
{
    size_t sf_len = (size_t)(frame[1] & 0x0FU);
    if (sf_len == 0U || sf_len > 6U || link->rx_buf == NULL || sf_len > link->rx_buf_size) {
        return;
    }
    memcpy(link->rx_buf, &frame[2], sf_len);
    link->rx_total_len = sf_len;
    link->rx_offset = sf_len;
    link->rx_nad = frame_nad;
    link->rx_completed = true;
    link->rx_state = SYN_LINTP_STATE_IDLE;
}

/**
 * @brief Process an inbound LIN-TP First Frame (FF) and start segmented reception.
 * @param link LIN-TP link instance.
 * @param frame_nad Node Address for Diagnostics from frame byte 0.
 * @param frame 8-byte LIN diagnostic frame payload.
 */
static void lintp_rx_ff(SYN_LINTP_Link *link, uint8_t frame_nad, const uint8_t frame[8])
{
    size_t ff_len = (size_t)((((size_t)frame[1] & 0x0FU) << 8U) | (size_t)frame[2]);
    if (ff_len <= 6U || link->rx_buf == NULL || ff_len > link->rx_buf_size) {
        link->rx_state = SYN_LINTP_STATE_IDLE;
        return;
    }
    memcpy(link->rx_buf, &frame[3], 5U);
    link->rx_total_len = ff_len;
    link->rx_offset = 5U;
    link->rx_sn = 1U;
    link->rx_nad = frame_nad;
    link->rx_completed = false;
    link->rx_timer_ms = 0U;
    link->rx_state = SYN_LINTP_STATE_RECEIVING_CF;
}

void syn_lintp_process_rx_frame(SYN_LINTP_Link *link, const uint8_t frame[8])
{
    if (link == NULL || frame == NULL) {
        return;
    }

    uint8_t frame_nad = frame[0];
    if (frame_nad != link->nad && frame_nad != SYN_LINTP_NAD_FUNCTIONAL &&
        frame_nad != SYN_LINTP_NAD_BROADCAST) {
        return;
    }

    uint8_t pci_type = (uint8_t)((frame[1] >> 4U) & 0x0FU);

    switch (pci_type) {
    case SYN_LINTP_PCI_SF:
        lintp_rx_sf(link, frame_nad, frame);
        break;

    case SYN_LINTP_PCI_FF:
        lintp_rx_ff(link, frame_nad, frame);
        break;

    case SYN_LINTP_PCI_CF:
        lintp_rx_cf(link, frame_nad, frame);
        break;

    default:
        break;
    }
}

ssize_t syn_lintp_receive(SYN_LINTP_Link *link, uint8_t *out_buf, size_t max_len)
{
    if (link == NULL || out_buf == NULL || max_len == 0U) {
        return SYN_INVALID_PARAM;
    }

    if (!link->rx_completed) {
        return 0;
    }

    size_t copy_len = (link->rx_total_len < max_len) ? link->rx_total_len : max_len;
    memcpy(out_buf, link->rx_buf, copy_len);

    link->rx_completed = false;
    link->rx_offset = 0U;
    link->rx_total_len = 0U;

    return (ssize_t)copy_len;
}

void syn_lintp_step(SYN_LINTP_Link *link, uint32_t dt_ms)
{
    if (link == NULL) {
        return;
    }

    if (link->rx_state == SYN_LINTP_STATE_RECEIVING_CF) {
        link->rx_timer_ms += dt_ms;
        if (link->rx_timer_ms >= link->timer_n_cr_ms) {
            link->rx_state = SYN_LINTP_STATE_IDLE;
            link->rx_offset = 0U;
            link->rx_completed = false;
        }
    }

    if (link->tx_state != SYN_LINTP_STATE_IDLE || link->tx_frame_pending) {
        link->tx_timer_ms += dt_ms;
        if (link->tx_timer_ms >= link->timer_n_as_ms) {
            link->tx_state = SYN_LINTP_STATE_IDLE;
            link->tx_frame_pending = false;
            link->tx_offset = 0U;
        }
    }
}

bool syn_lintp_is_tx_idle(const SYN_LINTP_Link *link)
{
    if (link == NULL) {
        return true;
    }
    return (link->tx_state == SYN_LINTP_STATE_IDLE && !link->tx_frame_pending);
}

#endif /* SYN_USE_LINTP */
