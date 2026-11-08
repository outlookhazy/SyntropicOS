/**
 * @file syn_dnssd.h
 * @brief DNS-Based Service Discovery (DNS-SD - RFC 6763 / RFC 6762).
 * @ingroup syn_net
 *
 * Provides zero-allocation, cleanroom multicast DNS-SD service announcement
 * and query responding for embedded devices (PTR, SRV, TXT, and A records).
 */

#ifndef SYN_DNSSD_H
#define SYN_DNSSD_H

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Maximum registered services per DNS-SD instance. */
#define SYN_DNSSD_MAX_SERVICES 4U
/** @brief Maximum TXT key-value attributes per service. */
#define SYN_DNSSD_MAX_TXT_RECORDS 8U
/** @brief Standard mDNS / DNS-SD Multicast Port. */
#define SYN_DNSSD_PORT 5353U

/**
 * @brief DNS-SD Service Definition.
 */
typedef struct {
    const char *instance_name; /**< Service Instance Name (e.g. "Syntropic Node") */
    const char *service_type;  /**< Service Type (e.g. "_http._tcp")               */
    const char *hostname;      /**< Target Hostname without .local (e.g. "node")  */
    uint16_t port;             /**< TCP/UDP Port number (e.g. 80)                 */
    uint8_t ip[4];             /**< IPv4 address to announce                      */
    const char *txt_records[SYN_DNSSD_MAX_TXT_RECORDS]; /**< Key-value TXT strings */
    size_t txt_count; /**< Number of TXT strings                         */
} SYN_DnsSd_Service;

/**
 * @brief DNS-SD Daemon Context.
 */
typedef struct {
    SYN_DnsSd_Service services[SYN_DNSSD_MAX_SERVICES]; /**< Service registry     */
    size_t service_count;                               /**< Registered count     */
    SYN_Socket sock;                                    /**< Multicast UDP socket */
} SYN_DnsSd;

/**
 * @brief Discovered Service Information received from mDNS response.
 */
typedef struct {
    char instance_name[64]; /**< Discovered Instance Name (e.g. "Syntropic Node") */
    char service_type[64];  /**< Discovered Service Type (e.g. "_http._tcp")      */
    char hostname[64];      /**< Discovered Target Hostname (e.g. "node")        */
    uint16_t port;          /**< Port number                                     */
    uint8_t ip[4];          /**< IPv4 address                                    */
    char txt[128];          /**< First TXT record or summary                     */
} SYN_DnsSd_Discovered;

/**
 * @brief Initialize DNS-SD daemon context.
 *
 * @param sd DNS-SD instance.
 * @return SYN_OK on success, SYN_ERROR on socket/multicast error.
 */
SYN_Status syn_dnssd_init(SYN_DnsSd *sd);

/**
 * @brief Register a service for DNS-SD broadcast/discovery.
 *
 * @param sd  DNS-SD instance.
 * @param svc Service configuration to register.
 * @return SYN_OK on success, SYN_ERROR if table is full or invalid parameters.
 */
SYN_Status syn_dnssd_register(SYN_DnsSd *sd, const SYN_DnsSd_Service *svc);

/**
 * @brief Process an incoming mDNS / DNS-SD query packet and format response.
 *
 * @param sd           DNS-SD context containing registered services.
 * @param query_buf    Incoming raw DNS packet.
 * @param query_len    Query packet byte length.
 * @param resp_buf     [out] Buffer to receive formatted response packet.
 * @param max_resp_len Capacity of resp_buf.
 * @param resp_len     [out] Number of bytes written to resp_buf.
 * @return SYN_OK if query matched and response generated, SYN_ERROR/SYN_NOT_FOUND otherwise.
 */
SYN_Status syn_dnssd_process_query(const SYN_DnsSd *sd, const uint8_t *query_buf, size_t query_len,
                                   uint8_t *resp_buf, size_t max_resp_len, size_t *resp_len);

/**
 * @brief Format and send gratuitous multicast DNS-SD announcement (RFC 6762 §8.3).
 *
 * @param sd            DNS-SD context.
 * @param service_index Index of service to announce.
 * @param resp_buf      [out] Buffer to receive formatted response packet.
 * @param max_resp_len  Capacity of resp_buf.
 * @param resp_len      [out] Number of bytes written to resp_buf.
 * @return SYN_OK on success, SYN_ERROR on invalid parameters or send failure.
 */
SYN_Status syn_dnssd_announce(const SYN_DnsSd *sd, size_t service_index, uint8_t *resp_buf,
                              size_t max_resp_len, size_t *resp_len);

/**
 * @brief Build an mDNS service discovery query (RFC 6762 / RFC 6763).
 *
 * Formats a standard PTR query for `_service._proto.local`.
 *
 * @param service_type Service type to search for (e.g. "_http._tcp", "_coap._udp").
 * @param query_buf    [out] Output buffer for DNS query packet.
 * @param max_len      Capacity of query_buf.
 * @param query_len    [out] Written length of query packet.
 * @return SYN_OK on success, SYN_ERROR on buffer overflow or invalid arguments.
 */
SYN_Status syn_dnssd_build_query(const char *service_type, uint8_t *query_buf, size_t max_len,
                                 size_t *query_len);

/**
 * @brief Parse an incoming mDNS response packet into a discovered service structure.
 *
 * @param resp_buf Buffer containing raw DNS response.
 * @param resp_len Length of raw DNS response.
 * @param out_disc [out] Structure to receive parsed service attributes.
 * @return SYN_OK if successfully parsed, SYN_ERROR otherwise.
 */
SYN_Status syn_dnssd_parse_response(const uint8_t *resp_buf, size_t resp_len,
                                    SYN_DnsSd_Discovered *out_disc);

/**
 * @brief Send an mDNS discovery query for a specific service type on multicast UDP.
 *
 * @param sd           DNS-SD context containing open multicast socket.
 * @param service_type Service type to query for (e.g. "_http._tcp").
 * @return SYN_OK on successful multicast transmit, SYN_ERROR otherwise.
 */
SYN_Status syn_dnssd_discover(const SYN_DnsSd *sd, const char *service_type);

/**
 * @brief Callback invoked when a remote service is discovered via DNS-SD.
 *
 * @param service   Pointer to the discovered service attributes.
 * @param user_data User context pointer passed to syn_dnssd_browser_init().
 */
typedef void (*SYN_DnsSd_DiscoverCallback)(const SYN_DnsSd_Discovered *service, void *user_data);

/**
 * @brief DNS-SD Service Discovery Browser Context.
 */
typedef struct {
    const SYN_DnsSd *sd;                 /**< Shared DNS-SD context with open socket */
    const char *service_type;            /**< Target service type (e.g. "_http._tcp") */
    SYN_DnsSd_DiscoverCallback callback; /**< Discovery result callback               */
    void *user_data;                     /**< User callback context                   */
    uint32_t timeout_ms;                 /**< Maximum active browsing duration in ms  */
    uint32_t start_time;                 /**< Timestamp when discovery started        */
    uint32_t discovered_count;           /**< Number of services discovered           */
    bool active;                         /**< True while browsing session is active   */
} SYN_DnsSd_Browser;

/**
 * @brief Initialize and start a DNS-SD service browsing session.
 *
 * Transmits the initial multicast PTR query and arms the browser context.
 *
 * @param browser      Browser context instance.
 * @param sd           Initialized DNS-SD daemon instance (provides socket).
 * @param service_type Service type to search for (e.g. "_http._tcp", "_coap._udp").
 * @param cb           Callback invoked whenever a valid response is received.
 * @param user_data    Optional user context pointer.
 * @param timeout_ms   Maximum time in ms to listen for discovery responses.
 * @return SYN_OK on success, SYN_ERROR on invalid parameters.
 */
SYN_Status syn_dnssd_browser_init(SYN_DnsSd_Browser *browser, const SYN_DnsSd *sd,
                                  const char *service_type, SYN_DnsSd_DiscoverCallback cb,
                                  void *user_data, uint32_t timeout_ms);

/**
 * @brief Cooperative protothread coroutine for background DNS-SD browsing.
 *
 * Yields until responses arrive or timeout expires. Parses incoming packets and
 * invokes the discovery callback.
 *
 * @param pt   Protothread pointer.
 * @param task Task descriptor with user_data pointing to SYN_DnsSd_Browser.
 * @return PT_IDLE while running, PT_ENDED when discovery timeout elapses.
 */
SYN_PT_Status syn_dnssd_browse_task(SYN_PT *pt, SYN_Task *task);

/**
 * @brief Cooperative protothread task for responding to DNS-SD discovery queries.
 *
 * @param pt   Protothread pointer.
 * @param task Task structure with user_data pointing to SYN_DnsSd instance.
 * @return PT_IDLE while running, PT_ENDED when done.
 */
SYN_PT_Status syn_dnssd_task(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DNSSD_H */
