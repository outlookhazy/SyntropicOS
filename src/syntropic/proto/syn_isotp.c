#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_ISOTP) || SYN_USE_ISOTP

/**
 * @file syn_isotp.c
 * @brief ISO 15765-2 (ISO-TP) Multi-Frame CAN Transport implementation.
 */

#include "../util/syn_assert.h"
#include "../util/syn_pack.h"
#include "syn_isotp.h"

#include <string.h>

#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
/* ── CAN FD length padding helper (ISO 11898-1 valid lengths) ───────────── */
static uint8_t syn_can_fd_pad_len(uint8_t len)
{
    if (len <= 8)
        return len;
    if (len <= 12)
        return 12;
    if (len <= 16)
        return 16;
    if (len <= 20)
        return 20;
    if (len <= 24)
        return 24;
    if (len <= 32)
        return 32;
    if (len <= 48)
        return 48;
    return 64;
}
#endif

/**
 * @brief Decode ISO 15765-2 STmin byte value into microsecond delay.
 * @param st_min Raw STmin byte value.
 * @return Delay in microseconds.
 */
static uint32_t syn_isotp_decode_stmin_us(uint8_t st_min)
{
    if (st_min <= 0x7F) {
        return (uint32_t)st_min * 1000U; /* 0..127 ms -> us */
    }
    if (st_min >= 0xF1 && st_min <= 0xF9) {
        return (uint32_t)(st_min - 0xF0) * 100U; /* 100..900 us */
    }
    return 127000U; /* Reserved values default to 127ms per ISO 15765-2:2016 Clause 9.6.6 */
}

/**
 * @brief Convert milliseconds to microseconds with 32-bit unsigned saturation.
 * @param ms Duration in milliseconds.
 * @return Duration in microseconds, saturated at UINT32_MAX.
 */
static uint32_t isotp_ms_to_us(uint32_t ms)
{
    if (ms > (UINT32_MAX / 1000U)) {
        return UINT32_MAX;
    }
    return ms * 1000U;
}

void syn_isotp_set_timeouts(SYN_ISOTP_Link *link, uint32_t n_bs_ms, uint32_t n_cr_ms)
{
    if (link == NULL)
        return;
    link->n_bs_timeout_us =
        (n_bs_ms > 0U) ? isotp_ms_to_us(n_bs_ms) : (SYN_ISOTP_DEFAULT_N_BS_MS * 1000U);
    link->n_cr_timeout_us =
        (n_cr_ms > 0U) ? isotp_ms_to_us(n_cr_ms) : (SYN_ISOTP_DEFAULT_N_CR_MS * 1000U);
}

void syn_isotp_init(SYN_ISOTP_Link *link, uint32_t rx_id, uint32_t tx_id, uint8_t *rx_buf,
                    size_t rx_buf_size, uint8_t *tx_buf, size_t tx_buf_size)
{
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    syn_isotp_init_fd(link, rx_id, tx_id, rx_buf, rx_buf_size, tx_buf, tx_buf_size, false);
#else
    SYN_ASSERT(link != NULL);

    memset(link, 0, sizeof(*link));
    link->rx_id = rx_id;
    link->tx_id = tx_id;
    link->rx_buf = rx_buf;
    link->rx_buf_size = rx_buf_size;
    link->tx_buf = tx_buf;
    link->tx_buf_size = tx_buf_size;
    link->tx_state = SYN_ISOTP_TX_IDLE;
    link->rx_state = SYN_ISOTP_RX_IDLE;
    link->rx_fc_bs = 0;
    link->rx_fc_stmin = 0;
    link->padding_byte = SYN_ISOTP_DEFAULT_PADDING;
    link->n_bs_timeout_us = SYN_ISOTP_DEFAULT_N_BS_MS * 1000U;
    link->n_cr_timeout_us = SYN_ISOTP_DEFAULT_N_CR_MS * 1000U;
#endif
}

void syn_isotp_set_fc_params(SYN_ISOTP_Link *link, uint8_t bs, uint8_t stmin)
{
    if (link == NULL)
        return;
    link->rx_fc_bs = bs;
    link->rx_fc_stmin = stmin;
}

void syn_isotp_set_padding(SYN_ISOTP_Link *link, uint8_t pad_byte)
{
    if (link != NULL) {
        link->padding_byte = pad_byte;
    }
}

#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
void syn_isotp_init_fd(SYN_ISOTP_Link *link, uint32_t rx_id, uint32_t tx_id, uint8_t *rx_buf,
                       size_t rx_buf_size, uint8_t *tx_buf, size_t tx_buf_size, bool is_fd)
{
    SYN_ASSERT(link != NULL);

    memset(link, 0, sizeof(*link));
    link->rx_id = rx_id;
    link->tx_id = tx_id;
    link->is_fd = is_fd;
    link->rx_buf = rx_buf;
    link->rx_buf_size = rx_buf_size;
    link->tx_buf = tx_buf;
    link->tx_buf_size = tx_buf_size;
    link->tx_state = SYN_ISOTP_TX_IDLE;
    link->rx_state = SYN_ISOTP_RX_IDLE;
    link->rx_fc_bs = 0;
    link->rx_fc_stmin = 0;
    link->padding_byte = SYN_ISOTP_DEFAULT_PADDING;
    link->n_bs_timeout_us = SYN_ISOTP_DEFAULT_N_BS_MS * 1000U;
    link->n_cr_timeout_us = SYN_ISOTP_DEFAULT_N_CR_MS * 1000U;
}
#endif

SYN_Status syn_isotp_send(SYN_ISOTP_Link *link, const uint8_t *payload, size_t payload_len)
{
    if (link == NULL || payload == NULL || payload_len < 1) {
        return SYN_INVALID_PARAM;
    }
    if (link->tx_state != SYN_ISOTP_TX_IDLE) {
        return SYN_BUSY;
    }
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    if ((!link->is_fd) && (payload_len > 4095U)) {
        return SYN_INVALID_PARAM;
    }
#else
    if (payload_len > 4095U) {
        return SYN_INVALID_PARAM;
    }
#endif
    if (payload_len > link->tx_buf_size) {
        return SYN_INVALID_PARAM;
    }

    memcpy(link->tx_buf, payload, payload_len);
    link->tx_len = payload_len;
    link->tx_offset = 0;
    link->tx_seq = 1;
    link->tx_bs_count = 0;

#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    size_t max_sf_len = link->is_fd ? 62 : 7;
#else
    size_t max_sf_len = 7;
#endif

    if (payload_len <= max_sf_len) {
        link->tx_state = SYN_ISOTP_TX_SEND_SF;
    } else {
        link->tx_state = SYN_ISOTP_TX_SEND_FF;
    }

    return SYN_OK;
}

/**
 * @brief Encode an outgoing ISO-TP Single Frame (SF) into frame.
 * @param link ISO-TP link context.
 * @param frame Output CAN frame.
 */
static void isotp_tx_build_sf(SYN_ISOTP_Link *link, SYN_CAN_Frame *frame)
{
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    if (link->is_fd) {
        size_t sf_len = link->tx_len;
        if (sf_len <= 7U) {
            frame->data[0] = SYN_ISOTP_PCI_SF | (uint8_t)sf_len;
            memcpy(&frame->data[1], link->tx_buf, sf_len);
            frame->dlc = syn_can_fd_pad_len((uint8_t)(sf_len + 1U));
        } else {
            frame->data[0] = SYN_ISOTP_PCI_SF;
            frame->data[1] = (uint8_t)sf_len;
            memcpy(&frame->data[2], link->tx_buf, sf_len);
            frame->dlc = syn_can_fd_pad_len((uint8_t)(sf_len + 2U));
        }
    } else {
        frame->data[0] = SYN_ISOTP_PCI_SF | (uint8_t)link->tx_len;
        memcpy(&frame->data[1], link->tx_buf, link->tx_len);
        frame->dlc = 8U;
    }
#else
    frame->data[0] = SYN_ISOTP_PCI_SF | (uint8_t)link->tx_len;
    memcpy(&frame->data[1], link->tx_buf, link->tx_len);
    frame->dlc = 8U;
#endif
    link->tx_state = SYN_ISOTP_TX_IDLE;
}

/**
 * @brief Encode an outgoing ISO-TP First Frame (FF) into frame and arm N_Bs wait state.
 * @param link ISO-TP link context.
 * @param frame Output CAN frame.
 */
static void isotp_tx_build_ff(SYN_ISOTP_Link *link, SYN_CAN_Frame *frame)
{
    size_t payload_in_ff;
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    if (link->is_fd) {
        if (link->tx_len <= 4095U) {
            frame->data[0] = SYN_ISOTP_PCI_FF | (uint8_t)((link->tx_len >> 8U) & 0x0FU);
            frame->data[1] = (uint8_t)(link->tx_len & 0xFFU);
            payload_in_ff = (link->tx_len > 62U) ? 62U : link->tx_len;
            memcpy(&frame->data[2], link->tx_buf, payload_in_ff);
            frame->dlc = syn_can_fd_pad_len((uint8_t)(payload_in_ff + 2U));
        } else {
            frame->data[0] = SYN_ISOTP_PCI_FF;
            frame->data[1] = 0U;
            syn_poke_u32((uint32_t)link->tx_len, frame->data, 2U);
            payload_in_ff = (link->tx_len > 58U) ? 58U : link->tx_len;
            memcpy(&frame->data[6], link->tx_buf, payload_in_ff);
            frame->dlc = syn_can_fd_pad_len((uint8_t)(payload_in_ff + 6U));
        }
    } else {
        frame->data[0] = SYN_ISOTP_PCI_FF | (uint8_t)((link->tx_len >> 8U) & 0x0FU);
        frame->data[1] = (uint8_t)(link->tx_len & 0xFFU);
        payload_in_ff = 6U;
        memcpy(&frame->data[2], link->tx_buf, payload_in_ff);
        frame->dlc = 8U;
    }
#else
    frame->data[0] = SYN_ISOTP_PCI_FF | (uint8_t)((link->tx_len >> 8U) & 0x0FU);
    frame->data[1] = (uint8_t)(link->tx_len & 0xFFU);
    payload_in_ff = 6U;
    memcpy(&frame->data[2], link->tx_buf, payload_in_ff);
    frame->dlc = 8U;
#endif
    link->tx_offset = payload_in_ff;
    link->tx_state = SYN_ISOTP_TX_WAIT_FC;
    link->tx_timeout_timer_us = link->n_bs_timeout_us;
}

/**
 * @brief Encode an outgoing ISO-TP Consecutive Frame (CF) if STmin elapsed.
 * @param link ISO-TP link context.
 * @param frame Output CAN frame.
 * @return true if a CF frame was emitted, false otherwise.
 */
static bool isotp_tx_build_cf(SYN_ISOTP_Link *link, SYN_CAN_Frame *frame)
{
    if (link->tx_st_timer_us > 0U) {
        return false;
    }

    size_t rem = link->tx_len - link->tx_offset;
    if (rem == 0U) {
        link->tx_state = SYN_ISOTP_TX_IDLE;
        return false;
    }

    frame->data[0] = SYN_ISOTP_PCI_CF | (link->tx_seq & 0x0FU);
    size_t payload_len;
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    if (link->is_fd) {
        payload_len = (rem > 63U) ? 63U : rem;
        memcpy(&frame->data[1], &link->tx_buf[link->tx_offset], payload_len);
        frame->dlc = syn_can_fd_pad_len((uint8_t)(payload_len + 1U));
    } else {
        payload_len = (rem > 7U) ? 7U : rem;
        memcpy(&frame->data[1], &link->tx_buf[link->tx_offset], payload_len);
        frame->dlc = 8U;
    }
#else
    payload_len = (rem > 7U) ? 7U : rem;
    memcpy(&frame->data[1], &link->tx_buf[link->tx_offset], payload_len);
    frame->dlc = 8U;
#endif

    link->tx_offset += payload_len;
    link->tx_seq = (uint8_t)((link->tx_seq + 1U) & 0x0FU);
    link->tx_st_timer_us = syn_isotp_decode_stmin_us(link->tx_st_min);

    if (link->tx_bs > 0U) {
        link->tx_bs_count++;
        if ((link->tx_bs_count >= link->tx_bs) && (link->tx_offset < link->tx_len)) {
            link->tx_bs_count = 0U;
            link->tx_state = SYN_ISOTP_TX_WAIT_FC;
            link->tx_timeout_timer_us = link->n_bs_timeout_us;
        }
    }

    if (link->tx_offset >= link->tx_len) {
        link->tx_state = SYN_ISOTP_TX_IDLE;
    }
    return true;
}

bool syn_isotp_get_tx_frame(SYN_ISOTP_Link *link, SYN_CAN_Frame *frame)
{
    if ((link == NULL) || (frame == NULL)) {
        return false;
    }

    memset(frame, 0, sizeof(*frame));
    memset(frame->data, link->padding_byte, sizeof(frame->data));
    frame->id = link->tx_id;
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    frame->is_fd = link->is_fd;
#endif

    if (link->rx_fc_pending) {
        link->rx_fc_pending = false;
        frame->data[0] = SYN_ISOTP_PCI_FC | link->rx_fc_status;
        frame->data[1] = link->rx_fc_bs;
        frame->data[2] = link->rx_fc_stmin;
        frame->dlc = 8U;
        return true;
    }

    switch (link->tx_state) {
    case SYN_ISOTP_TX_SEND_SF:
        isotp_tx_build_sf(link, frame);
        return true;
    case SYN_ISOTP_TX_SEND_FF:
        isotp_tx_build_ff(link, frame);
        return true;
    case SYN_ISOTP_TX_SEND_CF:
        return isotp_tx_build_cf(link, frame);
    default:
        return false;
    }
}

/**
 * @brief Process an incoming ISO-TP Single Frame (SF).
 * @param link ISO-TP link context.
 * @param frame Received CAN frame (dlc >= 1).
 */
static void isotp_handle_sf(SYN_ISOTP_Link *link, const SYN_CAN_Frame *frame)
{
    size_t sf_len = (size_t)(frame->data[0] & 0x0FU);
    size_t data_offset = 1U;

#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    if (frame->dlc > 8U) {
        if (sf_len != 0U) {
            return;
        }
        sf_len = (size_t)frame->data[1];
        if (sf_len <= 7U) {
            return;
        }
        data_offset = 2U;
    }
#endif

    size_t avail_bytes = (size_t)frame->dlc - data_offset;
    if ((sf_len == 0U) || (sf_len > avail_bytes)) {
        return;
    }

    if ((sf_len <= link->rx_buf_size) && (link->rx_buf != NULL)) {
        memcpy(link->rx_buf, &frame->data[data_offset], sf_len);
        link->rx_len = sf_len;
        link->rx_state = SYN_ISOTP_RX_COMPLETE;
        link->rx_fc_pending = false;
        link->rx_timeout_timer_us = 0U;
    }
}

/**
 * @brief Process an incoming ISO-TP First Frame (FF).
 * @param link ISO-TP link context.
 * @param frame Received CAN frame (dlc >= 1).
 */
static void isotp_handle_ff(SYN_ISOTP_Link *link, const SYN_CAN_Frame *frame)
{
    if (frame->dlc < 8U) {
        return;
    }

    size_t ff_len;
    size_t data_offset;
    uint16_t len_12 = syn_peek_u16(frame->data, 0U) & 0x0FFFU;
    if (len_12 != 0U) {
        ff_len = (size_t)len_12;
        data_offset = 2U;
    } else {
        ff_len = (size_t)syn_peek_u32(frame->data, 2U);
        data_offset = 6U;
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
        if ((link->is_fd || frame->is_fd) && (ff_len <= 4095U)) {
            return;
        }
#endif
    }

    size_t max_sf = (frame->dlc > 8U) ? ((size_t)frame->dlc - 2U) : 7U;
    if ((ff_len <= max_sf) || (link->rx_buf == NULL)) {
        return;
    }
    if (ff_len > link->rx_buf_size) {
        link->rx_fc_pending = true;
        link->rx_fc_status = SYN_ISOTP_FC_OVERFLOW;
        link->rx_state = SYN_ISOTP_RX_IDLE;
        return;
    }

    size_t payload_in_ff = (size_t)frame->dlc - data_offset;

    memcpy(link->rx_buf, &frame->data[data_offset], payload_in_ff);
    link->rx_len = payload_in_ff;
    link->rx_expected = ff_len;
    link->rx_seq = 1U;
    link->rx_bs_count = 0U;
    link->rx_fc_pending = true;
    link->rx_fc_status = SYN_ISOTP_FC_CTS;
    link->rx_state = SYN_ISOTP_RX_WAIT_CF;
    link->rx_timeout_timer_us = link->n_cr_timeout_us;
}

/**
 * @brief Process an incoming ISO-TP Consecutive Frame (CF).
 * @param link ISO-TP link context.
 * @param frame Received CAN frame (dlc >= 1).
 */
static void isotp_handle_cf(SYN_ISOTP_Link *link, const SYN_CAN_Frame *frame)
{
    if (link->rx_state != SYN_ISOTP_RX_WAIT_CF) {
        return;
    }

    uint8_t seq = frame->data[0] & 0x0FU;
    if (seq != link->rx_seq) {
        link->rx_state = SYN_ISOTP_RX_IDLE;
        link->rx_timeout_timer_us = 0U;
        return;
    }

    size_t rem = link->rx_expected - link->rx_len;
    size_t cf_payload = (size_t)frame->dlc - 1U;
    if (cf_payload > rem) {
        cf_payload = rem;
    }

    memcpy(&link->rx_buf[link->rx_len], &frame->data[1], cf_payload);
    link->rx_len += cf_payload;
    link->rx_seq = (uint8_t)((link->rx_seq + 1U) & 0x0FU);
    link->rx_bs_count++;

    if (link->rx_len >= link->rx_expected) {
        link->rx_state = SYN_ISOTP_RX_COMPLETE;
        link->rx_timeout_timer_us = 0U;
    } else {
        link->rx_timeout_timer_us = link->n_cr_timeout_us;
        if ((link->rx_fc_bs > 0U) && (link->rx_bs_count >= link->rx_fc_bs)) {
            link->rx_bs_count = 0U;
            link->rx_fc_pending = true;
            link->rx_fc_status = SYN_ISOTP_FC_CTS;
        }
    }
}

/**
 * @brief Process an incoming ISO-TP Flow Control (FC) frame.
 * @param link ISO-TP link context.
 * @param frame Received CAN frame (dlc >= 1).
 */
static void isotp_handle_fc(SYN_ISOTP_Link *link, const SYN_CAN_Frame *frame)
{
    if ((link->tx_state != SYN_ISOTP_TX_WAIT_FC) || (frame->dlc < 3U)) {
        return;
    }

    uint8_t fc_status = frame->data[0] & 0x0FU;
    if (fc_status == SYN_ISOTP_FC_CTS) {
        link->tx_bs = frame->data[1];
        link->tx_st_min = frame->data[2];
        link->tx_st_timer_us = 0U;
        link->tx_timeout_timer_us = 0U;
        link->tx_state = SYN_ISOTP_TX_SEND_CF;
    } else if (fc_status == SYN_ISOTP_FC_WAIT) {
        link->tx_timeout_timer_us = link->n_bs_timeout_us;
    } else {
        link->tx_state = SYN_ISOTP_TX_IDLE;
        link->tx_timeout_timer_us = 0U;
    }
}

/**
 * @brief Validate link/frame pointers, CAN ID, RTR flag, and Classical/FD DLC bounds on RX.
 * @param link  ISO-TP link instance.
 * @param frame Incoming CAN or CAN FD frame.
 * @return true if frame is a valid data frame addressed to @p link, false otherwise.
 */
static bool isotp_rx_frame_valid(const SYN_ISOTP_Link *link, const SYN_CAN_Frame *frame)
{
    if ((link == NULL) || (frame == NULL) || frame->rtr || (frame->id != link->rx_id)) {
        return false;
    }
    if ((frame->dlc == 0U) || (frame->dlc > (uint8_t)sizeof(frame->data))) {
        return false;
    }
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    if (!frame->is_fd && (frame->dlc > 8U)) {
        return false;
    }
#endif
    return true;
}

void syn_isotp_process_rx_frame(SYN_ISOTP_Link *link, const SYN_CAN_Frame *frame)
{
    if (!isotp_rx_frame_valid(link, frame)) {
        return;
    }

    uint8_t pci_type = frame->data[0] & 0xF0U;
    switch (pci_type) {
    case SYN_ISOTP_PCI_SF:
        isotp_handle_sf(link, frame);
        break;
    case SYN_ISOTP_PCI_FF:
        isotp_handle_ff(link, frame);
        break;
    case SYN_ISOTP_PCI_CF:
        isotp_handle_cf(link, frame);
        break;
    case SYN_ISOTP_PCI_FC:
        isotp_handle_fc(link, frame);
        break;
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
        if (copy_len > 32767U)
            copy_len = 32767U; /* LCOV_EXCL_LINE: Defensive 16-bit signed ssize_t cast clamp for
                                  32-bit CAN FD transfers >32KB */
        return (ssize_t)copy_len;
    }

    return -1;
}

void syn_isotp_step(SYN_ISOTP_Link *link, uint32_t dt_ms)
{
    syn_isotp_step_us(link, isotp_ms_to_us(dt_ms));
}

void syn_isotp_step_us(SYN_ISOTP_Link *link, uint32_t dt_us)
{
    if (link == NULL)
        return;

    if (link->tx_st_timer_us > 0) {
        if (dt_us >= link->tx_st_timer_us) {
            link->tx_st_timer_us = 0;
        } else {
            link->tx_st_timer_us -= dt_us;
        }
    }

    /* N_Bs timeout (waiting for Flow Control frame on sender side) */
    if (link->tx_state == SYN_ISOTP_TX_WAIT_FC && link->tx_timeout_timer_us > 0) {
        if (dt_us >= link->tx_timeout_timer_us) {
            link->tx_timeout_timer_us = 0;
            link->tx_state = SYN_ISOTP_TX_IDLE; /* Abort Tx on N_Bs timeout */
        } else {
            link->tx_timeout_timer_us -= dt_us;
        }
    }

    /* N_Cr timeout (waiting for Consecutive Frame on receiver side) */
    if (link->rx_state == SYN_ISOTP_RX_WAIT_CF && link->rx_timeout_timer_us > 0) {
        if (dt_us >= link->rx_timeout_timer_us) {
            link->rx_timeout_timer_us = 0;
            link->rx_state = SYN_ISOTP_RX_IDLE; /* Abort Rx on N_Cr timeout */
        } else {
            link->rx_timeout_timer_us -= dt_us;
        }
    }
}

bool syn_isotp_is_tx_idle(const SYN_ISOTP_Link *link)
{
    if (link == NULL) {
        return true;
    }
    return link->tx_state == SYN_ISOTP_TX_IDLE;
}

#endif /* SYN_USE_ISOTP */
