/**
 * @file syn_dns.h
 * @brief UDP DNS resolver and mDNS responder.
 * @ingroup syn_net
 */

#ifndef SYN_DNS_H
#define SYN_DNS_H

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief DNS resolver context.
 */
typedef struct {
    /* Inputs */
    const SYN_SockAddr *dns_server;      /**< DNS server (e.g. 8.8.8.8) */
    const char         *hostname;        /**< Hostname to resolve       */
    SYN_SockAddr       *addr_out;        /**< Resolved output address   */
    uint32_t            timeout_ms;      /**< Resolution timeout        */

    /* Output status */
    SYN_Status          status;          /**< Final status of query     */

    /* Internal State */
    SYN_Socket          sock;            /**< UDP socket                */
    SYN_SockAddr        server_addr;     /**< Server address            */
    uint32_t            start_ms;        /**< Start timestamp           */
    size_t              query_len;       /**< Sent query length         */
    uint16_t            txid;            /**< Generated transaction ID  */
    uint8_t             buf[256];        /**< Message buffer            */
} SYN_DnsResolver;

/**
 * @brief Cooperative task for resolving a hostname via DNS.
 *
 * Register this as a SYN_TaskFunc with user_data pointing to a
 * SYN_DnsResolver instance. The task runs non-blocking UDP queries,
 * yielding via PT_YIELD until resolution completes or times out.
 *
 * @param pt    Protothread pointer.
 * @param task  Task structure whose user_data points to a SYN_DnsResolver.
 * @return PT_WAITING while running, PT_EXITED when done, or PT_ENDED on early exit.
 */
SYN_PT_Status syn_dns_resolve_task(SYN_PT *pt, SYN_Task *task);

/** @brief mDNS responder instance. */
typedef struct {
    const char *hostname;   /**< Responds to hostname.local              */
    uint8_t     ip[4];      /**< IPv4 address to respond with            */
    SYN_Socket  sock;       /**< UDP socket for mDNS                     */
} SYN_Mdns;

/**
 * @brief Initialize the mDNS responder.
 * @param mdns      mDNS instance.
 * @param hostname  Hostname to respond to (without .local suffix).
 * @param ip        IPv4 address to advertise.
 * @return SYN_OK on success.
 */
SYN_Status syn_mdns_init(SYN_Mdns *mdns, const char *hostname, const uint8_t ip[4]);

/**
 * @brief Cooperative task for responding to local mDNS queries.
 * @param pt    Protothread pointer.
 * @param task  Task structure whose user_data points to a SYN_Mdns.
 * @return PT_WAITING while running, PT_EXITED when done.
 */
SYN_PT_Status syn_mdns_task(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DNS_H */
