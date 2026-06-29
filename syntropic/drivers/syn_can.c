#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_CAN) || SYN_USE_CAN

/**
 * @file syn_can.c
 * @brief CAN bus driver implementation.
 */

#include "syn_can.h"
#include "../util/syn_assert.h"

#include <string.h>

SYN_Status syn_can_init(SYN_CAN *can, uint8_t port, uint32_t bitrate)
{
    SYN_ASSERT(can != NULL);

    memset(can, 0, sizeof(*can));
    can->port    = port;
    can->bitrate = bitrate;

    if (!syn_port_can_init(port, bitrate)) {
        return SYN_ERROR;
    }

    return SYN_OK;
}

bool syn_can_send(SYN_CAN *can, const SYN_CAN_Frame *frame)
{
    SYN_ASSERT(can != NULL);
    SYN_ASSERT(frame != NULL);
    SYN_ASSERT(frame->dlc <= 8);

    bool ok = syn_port_can_send(can->port, frame->id, frame->extended,
                                  frame->data, frame->dlc);
    if (ok) {
        can->tx_count++;
    } else {
        can->err_count++;
    }
    return ok;
}

void syn_can_poll(SYN_CAN *can)
{
    SYN_ASSERT(can != NULL);

    SYN_CAN_Frame frame;
    memset(&frame, 0, sizeof(frame));

    while (syn_port_can_receive(can->port, &frame.id, &frame.extended,
                                  frame.data, &frame.dlc)) {
        can->rx_count++;
        if (can->on_rx != NULL) {
            can->on_rx(&frame, can->on_rx_ctx);
        }
    }
}

void syn_can_on_receive(SYN_CAN *can, SYN_CAN_Callback cb, void *ctx)
{
    SYN_ASSERT(can != NULL);
    can->on_rx     = cb;
    can->on_rx_ctx = ctx;
}

void syn_can_set_filter(const SYN_CAN *can, uint32_t id, uint32_t mask)
{
    SYN_ASSERT(can != NULL);
    syn_port_can_set_filter(can->port, id, mask);
}

#endif /* SYN_USE_CAN */
