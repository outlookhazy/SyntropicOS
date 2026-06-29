/**
 * @file test_firmware_multinode.c
 * @brief Test firmware for multi-node Renode simulation.
 *
 * Implements a simple networking node that uses the SyntropicOS router,
 * heartbeat, and COBS transport over USART3. Log output is sent via
 * USART2 (printf redirection).
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "syntropic/syntropic.h"
#include "syntropic/net/syn_router.h"
#include "syntropic/net/syn_heartbeat.h"
#include "syntropic/proto/syn_cobs.h"

/* Port functions we need */
extern SYN_Status syn_port_uart_init(SYN_UARTInstance inst, uint32_t baud);
extern SYN_Status syn_port_uart_transmit(SYN_UARTInstance inst, const uint8_t *data, size_t len, uint32_t timeout_ms);
extern SYN_Status syn_port_uart_receive_byte(SYN_UARTInstance inst, uint8_t *byte, uint32_t timeout_ms);
extern uint32_t    syn_port_get_tick_ms(void);
extern void        syn_port_delay_ms(uint32_t ms);

/* NODE_ID must be defined at compile time (1 or 2) */
#ifndef NODE_ID
#error "NODE_ID must be defined (e.g., -DNODE_ID=1)"
#endif

#define MY_NODE_ID    ((uint8_t)NODE_ID)
#define PEER_NODE_ID  ((uint8_t)(NODE_ID == 1 ? 2 : 1))

/* ── Transport implementation ───────────────────────────────────────────── */

typedef struct {
    SYN_UARTInstance uart;
    SYN_COBS_Decoder cobs_dec;
    uint8_t cobs_rx_buf[256];
    
    uint8_t pkt_buf[256];
    size_t pkt_len;
    bool pkt_ready;
} UART_COBS_TransportCtx;

static void on_cobs_packet(const uint8_t *data, size_t len, void *ctx)
{
    UART_COBS_TransportCtx *t_ctx = (UART_COBS_TransportCtx *)ctx;
    if (len <= sizeof(t_ctx->pkt_buf)) {
        memcpy(t_ctx->pkt_buf, data, len);
        t_ctx->pkt_len = len;
        t_ctx->pkt_ready = true;
    }
}

static bool uart_cobs_send(const uint8_t *data, size_t len, void *ctx)
{
    UART_COBS_TransportCtx *t_ctx = (UART_COBS_TransportCtx *)ctx;
    uint8_t encoded[512];
    
    /* COBS encode */
    size_t enc_len = syn_cobs_encode(data, len, encoded);
    if (enc_len == 0) return false;
    
    /* Add delimiter byte (0x00) */
    encoded[enc_len++] = 0x00;
    
    /* Transmit over UART */
    SYN_Status status = syn_port_uart_transmit(t_ctx->uart, encoded, enc_len, 100);
    return (status == SYN_OK);
}

static bool uart_cobs_recv(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    UART_COBS_TransportCtx *t_ctx = (UART_COBS_TransportCtx *)ctx;
    
    /* Read all available bytes from UART and feed to decoder */
    uint8_t byte;
    while (syn_port_uart_receive_byte(t_ctx->uart, &byte, 0) == SYN_OK) {
        syn_cobs_decoder_feed(&t_ctx->cobs_dec, byte);
        
        /* If a packet became ready, return it */
        if (t_ctx->pkt_ready) {
            if (t_ctx->pkt_len > max_len) {
                t_ctx->pkt_ready = false;
                return false; /* Buffer too small */
            }
            memcpy(data, t_ctx->pkt_buf, t_ctx->pkt_len);
            *out_len = t_ctx->pkt_len;
            t_ctx->pkt_ready = false;
            return true;
        }
    }
    
    return false;
}

/* ── Callbacks / Handlers ────────────────────────────────────────────────── */

static volatile bool peer_found_flag = false;

static void on_peer_found(uint8_t node_id, void *ctx)
{
    (void)ctx;
    printf("Peer %d found!\n", node_id);
    peer_found_flag = true;
}

static void on_peer_lost(uint8_t node_id, void *ctx)
{
    (void)ctx;
    printf("Peer %d lost!\n", node_id);
}

/* App data packet handler (type 0x20) */
static void on_app_data(const SYN_Packet *pkt, void *ctx)
{
    (void)ctx;
    /* Print the string payload received */
    printf("Node %d received: %s\n", MY_NODE_ID, pkt->payload);
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void)
{
    /* Init USART2 for debug output/printf at 115200 */
    syn_port_uart_init(1 /* USART2 */, 115200);
    printf("Starting Node %d...\n", MY_NODE_ID);

    /* Init USART3 for network interface at 115200 */
    syn_port_uart_init(2 /* USART3 */, 115200);

    /* Init transport context */
    static UART_COBS_TransportCtx trans_ctx;
    trans_ctx.uart = 2; /* USART3 */
    trans_ctx.pkt_ready = false;
    syn_cobs_decoder_init(&trans_ctx.cobs_dec, trans_ctx.cobs_rx_buf, sizeof(trans_ctx.cobs_rx_buf), on_cobs_packet, &trans_ctx);

    SYN_Transport transport = {
        .send = uart_cobs_send,
        .recv = uart_cobs_recv,
        .ctx = &trans_ctx
    };

    /* Init router */
    static SYN_RouterHandler handlers[4];
    static SYN_Router router;
    syn_router_init(&router, MY_NODE_ID, &transport, handlers, 4);

    /* Enable reliable ACK tracking */
    static SYN_PendingAck pending_acks[4];
    syn_router_enable_ack(&router, pending_acks, 4, 200, 3);

    /* Register APP data handler */
    syn_router_register(&router, 0x20, on_app_data, NULL);

    /* Init heartbeat */
    static SYN_HB_Peer peer_list[2];
    static SYN_Heartbeat heartbeat;
    syn_heartbeat_init(&heartbeat, &router, peer_list, 2, 500, 1500);
    syn_heartbeat_add_peer(&heartbeat, PEER_NODE_ID);
    peer_list[0].alive = false; /* Force transition on first received heartbeat */
    syn_heartbeat_on_peer_found(&heartbeat, on_peer_found, NULL);
    syn_heartbeat_on_peer_lost(&heartbeat, on_peer_lost, NULL);

    printf("Node %d configured. Monitoring Peer %d...\n", MY_NODE_ID, PEER_NODE_ID);

    uint32_t last_send = syn_port_get_tick_ms();
    bool message_sent = false;

    while (1) {
        /* Poll incoming packets and update heartbeats */
        syn_router_poll(&router);
        syn_heartbeat_update(&heartbeat);

        uint32_t now = syn_port_get_tick_ms();

        /* If Node 1, and peer Node 2 was found, send a test packet once */
        if (MY_NODE_ID == 1 && peer_found_flag && !message_sent) {
            /* Wait 100ms before sending message to let link stabilize */
            if (now - last_send > 100) {
                const char *msg = "Hello Node 2!";
                printf("Node 1 sending app packet to Node 2...\n");
                bool sent = syn_router_send(&router, PEER_NODE_ID, 0x20, (const uint8_t *)msg, strlen(msg) + 1, true);
                if (sent) {
                    printf("Node 1 message queued/sent successfully.\n");
                } else {
                    printf("Node 1 message send failed!\n");
                }
                message_sent = true;
            }
        }

        /* Print stats every 2 seconds */
        static uint32_t last_stats = 0;
        if (now - last_stats >= 2000) {
            printf("Node %d: TX=%d, RX=%d, Drop=%d\n", 
                   MY_NODE_ID, (int)router.tx_count, (int)router.rx_count, (int)router.drop_count);
            last_stats = now;
        }

        /* Tiny delay to prevent emulator starvation */
        syn_port_delay_ms(10);
    }
}
