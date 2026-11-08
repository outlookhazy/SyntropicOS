/**
 * @file syn_netcfg.h
 * @brief Unified Zero-Heap Network IP Address Manager.
 *
 * Unifies Static IP, DHCP Client, RFC 3927 AutoIP fallback, and Physical Link Event handling.
 */

#ifndef SYN_NETCFG_H
#define SYN_NETCFG_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_autoip.h"
#include "syntropic/net/syn_dhcp.h"
#include "syntropic/net/syn_eth.h"
#include "syntropic/pt/syn_pt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** IP Manager Modes. */
typedef enum {
    SYN_NETCFG_MODE_STATIC = 0,
    SYN_NETCFG_MODE_DHCP,
    SYN_NETCFG_MODE_AUTO /* DHCP with RFC 3927 AutoIP fallback */
} SYN_NETCFG_Mode;

/** Physical Link States. */
typedef enum { SYN_NETCFG_LINK_DOWN = 0, SYN_NETCFG_LINK_UP } SYN_NETCFG_LinkState;

struct SYN_NETCFG_s;

/** Application Link Event Callback Signature. */
typedef void (*SYN_NETCFG_LinkCb)(struct SYN_NETCFG_s *netcfg, SYN_NETCFG_LinkState state,
                                  void *user_data);

/** Network IP Manager Context. */
typedef struct SYN_NETCFG_s {
    SYN_NETCFG_Mode mode;            /**< IP configuration mode (Static, DHCP, AutoIP, Hybrid) */
    SYN_NETCFG_LinkState link_state; /**< Physical link state (Down/Up) */
    SYN_DHCP dhcp;                   /**< Embedded DHCP client context */
    SYN_AUTOIP autoip;               /**< Embedded AutoIP (Link-Local) context */
    bool is_bound;                   /**< True if IP configuration is active and bound */
    uint32_t static_ip;              /**< Configured static IPv4 address */
    uint32_t static_netmask;         /**< Configured static subnet mask */
    uint32_t static_gateway;         /**< Configured static default gateway IP */
    uint32_t assigned_ip;            /**< Currently bound active IPv4 address */
    uint32_t assigned_netmask;       /**< Currently bound active subnet mask */
    uint32_t assigned_gateway;       /**< Currently bound active gateway IP */
    SYN_NETCFG_LinkCb link_cb;       /**< Optional application link state change callback */
    void *user_data;                 /**< User context pointer passed to callback */
} SYN_NETCFG;

/**
 * @brief Initialize Unified Network IP Manager.
 *
 * @param netcfg Pointer to Netcfg context.
 * @param mode   Configuration mode (STATIC, DHCP, or AUTO).
 * @param mac    6-byte client MAC address.
 * @return SYN_OK on success.
 */
SYN_Status syn_netcfg_init(SYN_NETCFG *netcfg, SYN_NETCFG_Mode mode, const uint8_t mac[6]);

/**
 * @brief Register Application Link Event Callback.
 *
 * @param netcfg    Pointer to Netcfg context.
 * @param cb        Callback function pointer.
 * @param user_data User data context passed to callback.
 * @return SYN_OK on success.
 */
SYN_Status syn_netcfg_set_link_callback(SYN_NETCFG *netcfg, SYN_NETCFG_LinkCb cb, void *user_data);

/**
 * @brief Notify Network Manager of Physical Link State Change (Link Up / Link Down).
 *
 * On LINK_DOWN: Invalidates IP binding, resets state machines back to INIT, and triggers callback.
 * On LINK_UP: Automatically re-triggers IP acquisition state machine and triggers callback.
 *
 * @param netcfg Pointer to Netcfg context.
 * @param eth    Pointer to Ethernet context.
 * @param state  Physical Link State (SYN_NETCFG_LINK_DOWN or SYN_NETCFG_LINK_UP).
 * @return SYN_OK on success.
 */
SYN_Status syn_netcfg_set_link_state(SYN_NETCFG *netcfg, SYN_ETH *eth, SYN_NETCFG_LinkState state);

/**
 * @brief Set Static IP configuration parameters.
 *
 * @param netcfg  Pointer to Netcfg context.
 * @param eth     Pointer to Ethernet context.
 * @param ip      32-bit IPv4 address.
 * @param netmask 32-bit netmask.
 * @param gateway 32-bit router gateway IP.
 * @return SYN_OK on success.
 */
SYN_Status syn_netcfg_set_static(SYN_NETCFG *netcfg, SYN_ETH *eth, uint32_t ip, uint32_t netmask,
                                 uint32_t gateway);

/**
 * @brief Trigger AutoIP fallback when DHCP times out.
 *
 * @param netcfg Pointer to Netcfg context.
 * @param eth    Pointer to Ethernet context.
 * @param mac    6-byte client MAC address.
 * @return SYN_OK on success.
 */
SYN_Status syn_netcfg_trigger_autoip_fallback(SYN_NETCFG *netcfg, SYN_ETH *eth,
                                              const uint8_t mac[6]);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */

/**
 * @brief Block a protothread coroutine until IP configuration binding completes.
 *
 * @param pt     Protothread context.
 * @param netcfg Pointer to Netcfg context.
 */
#define PT_NETCFG_WAIT_BOUND(pt, netcfg) PT_WAIT_UNTIL(pt, (netcfg)->is_bound == true)

/**
 * @brief Block task execution (SYN_TASK_BLOCKED) until IP configuration binding completes.
 * @note Network event handler must call syn_task_resume(task) when netcfg->is_bound becomes true.
 */
#define PT_NETCFG_BLOCK_BOUND(pt, task, netcfg) \
    PT_BLOCK_CONDITION(pt, task, (netcfg)->is_bound == true)

/**
 * @brief Block a protothread coroutine until physical link is UP.
 *
 * @param pt     Protothread context.
 * @param netcfg Pointer to Netcfg context.
 */
#define PT_NETCFG_WAIT_LINK_UP(pt, netcfg) \
    PT_WAIT_UNTIL(pt, (netcfg)->link_state == SYN_NETCFG_LINK_UP)

/**
 * @brief Block task execution (SYN_TASK_BLOCKED) until physical link is UP.
 * @note Link state handler must call syn_task_resume(task) on SYN_NETCFG_LINK_UP transition.
 */
#define PT_NETCFG_BLOCK_LINK_UP(pt, task, netcfg) \
    PT_BLOCK_CONDITION(pt, task, (netcfg)->link_state == SYN_NETCFG_LINK_UP)

#ifdef __cplusplus
}
#endif

#endif /* SYN_NETCFG_H */
