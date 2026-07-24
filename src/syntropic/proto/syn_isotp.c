#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_ISOTP) || SYN_USE_ISOTP

/**
 * @file syn_isotp.c
 * @brief ISO 15765-2 (ISO-TP) Multi-Frame CAN Transport implementation.
 */

#include "syn_isotp.h"
#include "../util/syn_assert.h"

#include <string.h>

void syn_isotp_init(SYN_ISOTP_Link *link, uint32_t rx_id, uint32_t tx_id,
                    uint8_t *rx_buf, size_t rx_buf_size,
                    uint8_t *tx_buf, size_t tx_buf_size)
{
    SYN_ASSERT(link != NULL);

    memset(link, 0, sizeof(*link));
    link->rx_id        = rx_id;
    link->tx_id        = tx_id;
    link->rx_buf       = rx_buf;
    link->rx_buf_size  = rx_buf_size;
    link->tx_buf       = tx_buf;
    link->tx_buf_size  = tx_buf_size;
    link->tx_state     = SYN_ISOTP_TX_IDLE;
    link->rx_state     = SYN_ISOTP_RX_IDLE;
}

SYN_Status syn_isotp_send(SYN_ISOTP_Link *link, const uint8_t *payload, size_t payload_len)
{
    if (link == NULL || payload == NULL || payload_len < 1 || payload_len > SYN_ISOTP_MAX_PAYLOAD) {
        return SYN_INVALID_PARAM;
    }
    if (link->tx_state != SYN_ISOTP_TX_IDLE) {
        return SYN_BUSY;
    }
    if (payload_len > link->tx_buf_size) {
        return SYN_INVALID_PARAM;
    }

    memcpy(link->tx_buf, payload, payload_len);
    link->tx_len     = payload_len;
    link->tx_offset  = 0;
    link->tx_seq     = 1;
    link->tx_bs_count = 0;

    if (payload_len <= 7) {
        link->tx_state = SYN_ISOTP_TX_SEND_SF;
    } else {
        link->tx_state = SYN_ISOTP_TX_SEND_FF;
    }

    return SYN_OK;
}

bool syn_isotp_get_tx_frame(SYN_ISOTP_Link *link, SYN_CAN_Frame *frame)
{
    if (link == NULL || frame == NULL) return false;

    memset(frame, 0, sizeof(*frame));
    frame->id  = link->tx_id;
    frame->dlc = 8;

    /* Flow Control response takes priority */
    if (link->rx_fc_pending) {
        link->rx_fc_pending = false;
        frame->data[0] = SYN_ISOTP_PCI_FC | link->rx_fc_status;
        frame->data[1] = 0; /* BS = 0 (unlimited) */
        frame->data[2] = 0; /* STmin = 0 ms */
        return true;
    }

    switch (link->tx_state) {
    case SYN_ISOTP_TX_SEND_SF:
        frame->data[0] = SYN_ISOTP_PCI_SF | (uint8_t)(link->tx_len & 0x0F);
        memcpy(&frame->data[1], link->tx_buf, link->tx_len);
        link->tx_state = SYN_ISOTP_TX_IDLE;
        return true;

    case SYN_ISOTP_TX_SEND_FF:
        frame->data[0] = SYN_ISOTP_PCI_FF | (uint8_t)((link->tx_len >> 8) & 0x0F);
        frame->data[1] = (uint8_t)(link->tx_len & 0xFF);
        memcpy(&frame->data[2], link->tx_buf, 6);
        link->tx_offset = 6;
        link->tx_state  = SYN_ISOTP_TX_WAIT_FC;
        return true;

    case SYN_ISOTP_TX_SEND_CF:
        if (link->tx_st_timer > 0) {
            return false; /* Enforce STmin separation time */
        }

        frame->data[0] = SYN_ISOTP_PCI_CF | (link->tx_seq & 0x0F);
        size_t rem = link->tx_len - link->tx_offset;
        size_t chunk = (rem > 7) ? 7 : rem;
        memcpy(&frame->data[1], &link->tx_buf[link->tx_offset], chunk);

        link->tx_offset += chunk;
        link->tx_seq = (uint8_t)((link->tx_seq + 1) & 0x0F);
        link->tx_bs_count++;
        link->tx_st_timer = link->tx_st_min;

        if (link->tx_offset >= link->tx_len) {
            link->tx_state = SYN_ISOTP_TX_IDLE;
        } else if (link->tx_bs > 0 && link->tx_bs_count >= link->tx_bs) {
            link->tx_bs_count = 0;
            link->tx_state    = SYN_ISOTP_TX_WAIT_FC;
        }
        return true;

    default:
        return false;
    }
}

void syn_isotp_process_rx_frame(SYN_ISOTP_Link *link, const SYN_CAN_Frame *frame)
{
    if (link == NULL || frame == NULL) return;
    if (frame->id != link->rx_id) return;

    uint8_t pci_type = frame->data[0] & 0xF0U;

    switch (pci_type) {
    case SYN_ISOTP_PCI_SF: {
        size_t sf_len = frame->data[0] & 0x0FU;
        if (sf_len >= 1 && sf_len <= 7 && sf_len <= link->rx_buf_size && link->rx_buf != NULL) {
            memcpy(link->rx_buf, &frame->data[1], sf_len);
            link->rx_len   = sf_len;
            link->rx_state = SYN_ISOTP_RX_COMPLETE;
        }
        break;
    }

    case SYN_ISOTP_PCI_FF: {
        size_t ff_len = ((size_t)(frame->data[0] & 0x0FU) << 8) | frame->data[1];
        if (ff_len > 7 && link->rx_buf != NULL) {
            if (ff_len > link->rx_buf_size) {
                link->rx_fc_pending = true;
                link->rx_fc_status  = SYN_ISOTP_FC_OVERFLOW;
                link->rx_state      = SYN_ISOTP_RX_IDLE;
            } else {
                memcpy(link->rx_buf, &frame->data[2], 6);
                link->rx_len        = 6;
                link->rx_expected   = ff_len;
                link->rx_seq        = 1;
                link->rx_fc_pending = true;
                link->rx_fc_status  = SYN_ISOTP_FC_CTS;
                link->rx_state      = SYN_ISOTP_RX_WAIT_CF;
            }
        }
        break;
    }

    case SYN_ISOTP_PCI_CF: {
        if (link->rx_state == SYN_ISOTP_RX_WAIT_CF) {
            uint8_t seq = frame->data[0] & 0x0FU;
            if (seq == link->rx_seq) {
                size_t rem = link->rx_expected - link->rx_len;
                size_t chunk = (rem > 7) ? 7 : rem;
                memcpy(&link->rx_buf[link->rx_len], &frame->data[1], chunk);

                link->rx_len += chunk;
                link->rx_seq = (uint8_t)((link->rx_seq + 1) & 0x0F);

                if (link->rx_len >= link->rx_expected) {
                    link->rx_state = SYN_ISOTP_RX_COMPLETE;
                }
            } else {
                /* Sequence mismatch error */
                link->rx_state = SYN_ISOTP_RX_IDLE;
            }
        }
        break;
    }

    case SYN_ISOTP_PCI_FC: {
        if (link->tx_state == SYN_ISOTP_TX_WAIT_FC) {
            uint8_t fc_status = frame->data[0] & 0x0FU;
            if (fc_status == SYN_ISOTP_FC_CTS) {
                link->tx_bs       = frame->data[1];
                link->tx_st_min   = frame->data[2];
                link->tx_st_timer = 0;
                link->tx_state    = SYN_ISOTP_TX_SEND_CF;
            } else if (fc_status == SYN_ISOTP_FC_OVERFLOW) {
                link->tx_state    = SYN_ISOTP_TX_IDLE;
            }
        }
        break;
    }

    default:
        break;
    }
}

ssize_t syn_isotp_receive(SYN_ISOTP_Link *link, uint8_t *out_buf, size_t max_len)
{
    if (link == NULL || out_buf == NULL) {
        return -1;
    }

    if (link->rx_state == SYN_ISOTP_RX_COMPLETE) {
        size_t copy_len = (link->rx_len > max_len) ? max_len : link->rx_len;
        memcpy(out_buf, link->rx_buf, copy_len);
        link->rx_state = SYN_ISOTP_RX_IDLE;
        return (ssize_t)copy_len;
    }

    return -1;
}

void syn_isotp_step(SYN_ISOTP_Link *link, uint32_t dt_ms)
{
    if (link == NULL) return;

    if (link->tx_st_timer > 0) {
        if (dt_ms >= link->tx_st_timer) {
            link->tx_st_timer = 0;
        } else {
            link->tx_st_timer -= dt_ms;
        }
    }
}

#endif /* SYN_USE_ISOTP */
