

# File syn\_lintp.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lintp.c**](syn__lintp_8c.md)

[Go to the documentation of this file](syn__lintp_8c.md)


```C++

#include "syn_lintp.h"

#if !defined(SYN_USE_LINTP) || SYN_USE_LINTP

#include <string.h>

void syn_lintp_init(SYN_LINTP_Link *link, uint8_t nad, uint8_t *rx_buf, size_t rx_size,
                    uint8_t *tx_buf, size_t tx_size)
{
    (void)tx_buf;
    (void)tx_size;

    if (link == NULL) {
        return;
    }

    memset(link, 0, sizeof(*link));
    link->nad = nad;
    link->padding_byte = 0xFF;
    link->rx_buf = rx_buf;
    link->rx_buf_size = rx_size;
    link->timer_n_as_ms = 1000;
    link->timer_n_cr_ms = 1000;
    link->rx_state = SYN_LINTP_STATE_IDLE;
    link->tx_state = SYN_LINTP_STATE_IDLE;
}

void syn_lintp_set_timeouts(SYN_LINTP_Link *link, uint32_t n_as_ms, uint32_t n_cr_ms)
{
    if (link != NULL) {
        link->timer_n_as_ms = (n_as_ms > 0) ? n_as_ms : 1000;
        link->timer_n_cr_ms = (n_cr_ms > 0) ? n_cr_ms : 1000;
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
    if (link == NULL || payload == NULL || len == 0 || len > 4095) {
        return SYN_INVALID_PARAM;
    }

    if (link->tx_state != SYN_LINTP_STATE_IDLE) {
        return SYN_BUSY;
    }

    link->tx_nad = nad;
    link->tx_buf = payload;
    link->tx_total_len = len;
    link->tx_offset = 0;
    link->tx_sn = 1;
    link->tx_timer_ms = 0;

    /* Build first frame */
    memset(link->tx_frame, link->padding_byte, 8);
    link->tx_frame[0] = nad;

    if (len <= 6) {
        /* Single Frame (SF): Byte 0 = NAD, Byte 1 = 0x00 | len, Bytes 2..7 = payload */
        link->tx_frame[1] = (uint8_t)(SYN_LINTP_PCI_SF | (len & 0x0F));
        memcpy(&link->tx_frame[2], payload, len);
        link->tx_offset = len;
        link->tx_state = SYN_LINTP_STATE_IDLE; /* SF is queued in one shot */
    } else {
        /* First Frame (FF): Byte 0 = NAD, Byte 1 = 0x10 | (len >> 8), Byte 2 = len & 0xFF */
        link->tx_frame[1] = (uint8_t)(0x10 | ((len >> 8) & 0x0F));
        link->tx_frame[2] = (uint8_t)(len & 0xFF);
        memcpy(&link->tx_frame[3], payload, 5);
        link->tx_offset = 5;
        link->tx_state = SYN_LINTP_STATE_TRANSMITTING_CF;
    }

    link->tx_frame_pending = true;
    return SYN_OK;
}

bool syn_lintp_get_tx_frame(SYN_LINTP_Link *link, uint8_t out_frame[8])
{
    if (link == NULL || out_frame == NULL) {
        return false;
    }

    if (link->tx_frame_pending) {
        memcpy(out_frame, link->tx_frame, 8);
        link->tx_frame_pending = false;
        link->tx_timer_ms = 0;

        /* If more CF frames remain, format the next CF */
        if (link->tx_state == SYN_LINTP_STATE_TRANSMITTING_CF) {
            if (link->tx_offset < link->tx_total_len) {
                memset(link->tx_frame, link->padding_byte, 8);
                link->tx_frame[0] = link->tx_nad;
                link->tx_frame[1] = (uint8_t)(0x20 | (link->tx_sn & 0x0F));

                size_t bytes_to_copy = link->tx_total_len - link->tx_offset;
                if (bytes_to_copy > 6) {
                    bytes_to_copy = 6;
                }

                memcpy(&link->tx_frame[2], &link->tx_buf[link->tx_offset], bytes_to_copy);
                link->tx_offset += bytes_to_copy;
                link->tx_sn = (uint8_t)((link->tx_sn + 1) & 0x0F);
                link->tx_frame_pending = true;
            } else {
                link->tx_state = SYN_LINTP_STATE_IDLE;
            }
        }
        return true;
    }

    return false;
}

void syn_lintp_process_rx_frame(SYN_LINTP_Link *link, const uint8_t frame[8])
{
    if (link == NULL || frame == NULL) {
        return;
    }

    uint8_t frame_nad = frame[0];

    /* NAD Filtering: Ignore frames not destined for this node or functional/broadcast */
    if (frame_nad != link->nad && frame_nad != SYN_LINTP_NAD_FUNCTIONAL &&
        frame_nad != SYN_LINTP_NAD_BROADCAST) {
        return;
    }

    uint8_t pci_type = (frame[1] >> 4) & 0x0F;

    switch (pci_type) {
    case SYN_LINTP_PCI_SF: { /* Single Frame */
        size_t sf_len = frame[1] & 0x0F;
        if (sf_len == 0 || sf_len > 6 || link->rx_buf == NULL || sf_len > link->rx_buf_size) {
            return;
        }

        memcpy(link->rx_buf, &frame[2], sf_len);
        link->rx_total_len = sf_len;
        link->rx_offset = sf_len;
        link->rx_nad = frame_nad;
        link->rx_completed = true;
        link->rx_state = SYN_LINTP_STATE_IDLE;
        break;
    }

    case SYN_LINTP_PCI_FF: { /* First Frame */
        size_t ff_len = (size_t)(((frame[1] & 0x0F) << 8) | frame[2]);
        if (ff_len <= 6 || link->rx_buf == NULL || ff_len > link->rx_buf_size) {
            link->rx_state = SYN_LINTP_STATE_IDLE;
            return;
        }

        memcpy(link->rx_buf, &frame[3], 5);
        link->rx_total_len = ff_len;
        link->rx_offset = 5;
        link->rx_sn = 1;
        link->rx_nad = frame_nad;
        link->rx_completed = false;
        link->rx_timer_ms = 0;
        link->rx_state = SYN_LINTP_STATE_RECEIVING_CF;
        break;
    }

    case SYN_LINTP_PCI_CF: { /* Consecutive Frame */
        if (link->rx_state != SYN_LINTP_STATE_RECEIVING_CF) {
            return;
        }

        uint8_t sn = frame[1] & 0x0F;
        if (sn != link->rx_sn) { /* Sequence Number Mismatch Error */
            link->rx_state = SYN_LINTP_STATE_IDLE;
            link->rx_offset = 0;
            return;
        }

        size_t remaining = link->rx_total_len - link->rx_offset;
        size_t copy_bytes = (remaining > 6) ? 6 : remaining;

        if ((link->rx_offset + copy_bytes) <= link->rx_buf_size) {
            memcpy(&link->rx_buf[link->rx_offset], &frame[2], copy_bytes);
            link->rx_offset += copy_bytes;
        }

        link->rx_sn = (uint8_t)((link->rx_sn + 1) & 0x0F);
        link->rx_timer_ms = 0;

        if (link->rx_offset >= link->rx_total_len) {
            link->rx_completed = true;
            link->rx_state = SYN_LINTP_STATE_IDLE;
        }
        break;
    }

    default:
        break;
    }
}

ssize_t syn_lintp_receive(SYN_LINTP_Link *link, uint8_t *out_buf, size_t max_len)
{
    if (link == NULL || out_buf == NULL || max_len == 0) {
        return SYN_INVALID_PARAM;
    }

    if (!link->rx_completed) {
        return 0;
    }

    size_t copy_len = (link->rx_total_len < max_len) ? link->rx_total_len : max_len;
    memcpy(out_buf, link->rx_buf, copy_len);

    link->rx_completed = false;
    link->rx_offset = 0;
    link->rx_total_len = 0;

    return (ssize_t)copy_len;
}

void syn_lintp_step(SYN_LINTP_Link *link, uint32_t dt_ms)
{
    if (link == NULL) {
        return;
    }

    /* RX Inter-frame timeout (N_Cr) */
    if (link->rx_state == SYN_LINTP_STATE_RECEIVING_CF) {
        link->rx_timer_ms += dt_ms;
        if (link->rx_timer_ms >= link->timer_n_cr_ms) {
            link->rx_state = SYN_LINTP_STATE_IDLE;
            link->rx_offset = 0;
            link->rx_completed = false;
        }
    }

    /* TX Confirmation timeout (N_As) */
    if (link->tx_state != SYN_LINTP_STATE_IDLE || link->tx_frame_pending) {
        link->tx_timer_ms += dt_ms;
        if (link->tx_timer_ms >= link->timer_n_as_ms) {
            link->tx_state = SYN_LINTP_STATE_IDLE;
            link->tx_frame_pending = false;
            link->tx_offset = 0;
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
```


