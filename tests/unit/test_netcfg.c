/**
 * @file test_netcfg.c
 * @brief Unit tests for Unified Zero-Heap Network IP Address Manager.
 */

#include "syntropic/net/syn_netcfg.h"
#include "unity/unity.h"

#include <string.h>

static const uint8_t MAC[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint32_t g_link_event_count = 0;
static SYN_NETCFG_LinkState g_last_link_state = SYN_NETCFG_LINK_UP;

static void dummy_link_callback(SYN_NETCFG *netcfg, SYN_NETCFG_LinkState state, void *user_data)
{
    (void)netcfg;
    (void)user_data;
    g_link_event_count++;
    g_last_link_state = state;
}

static void test_netcfg_round4_mac_seeded_xid(void)
{
    const uint8_t mac_a[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
    const uint8_t mac_b[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x56};
    const uint8_t mac_zero[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    SYN_NETCFG cfg_a;
    SYN_NETCFG cfg_b;
    SYN_NETCFG cfg_z;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_netcfg_init(&cfg_a, SYN_NETCFG_MODE_AUTO, mac_a));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_netcfg_init(&cfg_b, SYN_NETCFG_MODE_AUTO, mac_b));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_netcfg_init(&cfg_z, SYN_NETCFG_MODE_AUTO, mac_zero));

    TEST_ASSERT_NOT_EQUAL(0U, cfg_a.dhcp.xid);
    TEST_ASSERT_NOT_EQUAL(0U, cfg_b.dhcp.xid);
    TEST_ASSERT_NOT_EQUAL(cfg_a.dhcp.xid, cfg_b.dhcp.xid);
    TEST_ASSERT_EQUAL_UINT32(0x12345678UL, cfg_z.dhcp.xid);
}

void test_netcfg_init_static(void)
{
    SYN_NETCFG netcfg;
    SYN_ETH eth;
    syn_eth_init(&eth, MAC, 0);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_netcfg_init(&netcfg, SYN_NETCFG_MODE_STATIC, MAC));
    TEST_ASSERT_TRUE(netcfg.is_bound);

    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_netcfg_set_static(&netcfg, &eth, 0xC0A80164, 0xFFFFFF00, 0xC0A80101));
    TEST_ASSERT_EQUAL_UINT32(0xC0A80164, eth.ip_addr);
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFF00, eth.netmask);
    TEST_ASSERT_EQUAL_UINT32(0xC0A80101, eth.gateway);

    test_netcfg_round4_mac_seeded_xid();
}

void test_netcfg_autoip_fallback(void)
{
    SYN_NETCFG netcfg;
    SYN_ETH eth;
    syn_eth_init(&eth, MAC, 0);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_netcfg_init(&netcfg, SYN_NETCFG_MODE_AUTO, MAC));
    TEST_ASSERT_FALSE(netcfg.is_bound);

    /* Trigger AutoIP fallback upon DHCP timeout */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_netcfg_trigger_autoip_fallback(&netcfg, &eth, MAC));
    TEST_ASSERT_TRUE(netcfg.is_bound);

    /* IP address should automatically fall inside 169.254.x.x */
    TEST_ASSERT_EQUAL_UINT32(0xA9FE0000UL, eth.ip_addr & 0xFFFF0000UL);
    TEST_ASSERT_EQUAL_UINT32(0xFFFF0000UL, eth.netmask);
}

void test_netcfg_link_events(void)
{
    SYN_NETCFG netcfg;
    SYN_ETH eth;
    syn_eth_init(&eth, MAC, 0);
    syn_netcfg_init(&netcfg, SYN_NETCFG_MODE_STATIC, MAC);
    syn_netcfg_set_static(&netcfg, &eth, 0xC0A80164, 0xFFFFFF00, 0xC0A80101);

    g_link_event_count = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_netcfg_set_link_callback(&netcfg, dummy_link_callback, NULL));

    /* Cable Unplug (LINK_DOWN) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_netcfg_set_link_state(&netcfg, &eth, SYN_NETCFG_LINK_DOWN));
    TEST_ASSERT_EQUAL_UINT32(1, g_link_event_count);
    TEST_ASSERT_EQUAL_INT(SYN_NETCFG_LINK_DOWN, g_last_link_state);
    TEST_ASSERT_FALSE(netcfg.is_bound);
    TEST_ASSERT_EQUAL_UINT32(0, eth.ip_addr);

    /* Cable Plug-In (LINK_UP) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_netcfg_set_link_state(&netcfg, &eth, SYN_NETCFG_LINK_UP));
    TEST_ASSERT_EQUAL_UINT32(2, g_link_event_count);
    TEST_ASSERT_EQUAL_INT(SYN_NETCFG_LINK_UP, g_last_link_state);
    TEST_ASSERT_TRUE(netcfg.is_bound);
    TEST_ASSERT_EQUAL_UINT32(0xC0A80164, eth.ip_addr);

    /* Test LINK_UP event in AUTO mode */
    SYN_NETCFG auto_netcfg;
    syn_netcfg_init(&auto_netcfg, SYN_NETCFG_MODE_AUTO, MAC);
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_netcfg_set_link_state(&auto_netcfg, &eth, SYN_NETCFG_LINK_UP));
    TEST_ASSERT_FALSE(auto_netcfg.is_bound);

    /* Test set_static while LINK_DOWN */
    syn_netcfg_set_link_state(&netcfg, &eth, SYN_NETCFG_LINK_DOWN);
    syn_netcfg_set_static(&netcfg, &eth, 0xC0A80164, 0xFFFFFF00, 0xC0A80101);
    TEST_ASSERT_FALSE(netcfg.is_bound);

    /* Test LINK_UP with static_ip == 0 */
    netcfg.static_ip = 0;
    syn_netcfg_set_link_state(&netcfg, &eth, SYN_NETCFG_LINK_UP);
    TEST_ASSERT_FALSE(netcfg.is_bound);

    /* Test AutoIP fallback while LINK_DOWN */
    syn_netcfg_set_link_state(&auto_netcfg, &eth, SYN_NETCFG_LINK_DOWN);
    syn_netcfg_trigger_autoip_fallback(&auto_netcfg, &eth, MAC);
    TEST_ASSERT_FALSE(auto_netcfg.is_bound);

    /* Test invalid link state enum value */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_netcfg_set_link_state(&netcfg, &eth, (SYN_NETCFG_LinkState)99));
}

static SYN_PT_Status netcfg_coroutine_task(SYN_PT *pt, SYN_NETCFG *netcfg)
{
    PT_BEGIN(pt);
    PT_NETCFG_WAIT_BOUND(pt, netcfg);
    PT_END(pt);
}

void test_netcfg_coroutine_pt(void)
{
    SYN_NETCFG netcfg;
    syn_netcfg_init(&netcfg, SYN_NETCFG_MODE_AUTO, MAC);

    SYN_PT pt;
    PT_INIT(&pt);

    /* First step: Netcfg is_bound == false -> coroutine yields (PT_IDLE) */
    TEST_ASSERT_EQUAL_INT(PT_IDLE, netcfg_coroutine_task(&pt, &netcfg));

    netcfg.is_bound = true;

    /* Second step: Netcfg is_bound == true -> coroutine completes (PT_ENDED) */
    TEST_ASSERT_EQUAL_INT(PT_ENDED, netcfg_coroutine_task(&pt, &netcfg));
}

void test_netcfg_null_checks(void)
{
    SYN_NETCFG netcfg;
    SYN_ETH eth;
    uint8_t mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_netcfg_init(NULL, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_netcfg_init(&netcfg, 0, NULL));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_netcfg_set_static(NULL, NULL, 0, 0, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_netcfg_set_static(&netcfg, &eth, 0, 0, 0));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_netcfg_trigger_autoip_fallback(NULL, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_netcfg_trigger_autoip_fallback(&netcfg, &eth, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_netcfg_trigger_autoip_fallback(&netcfg, NULL, mac));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_netcfg_set_link_callback(NULL, NULL, NULL));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_netcfg_set_link_state(NULL, NULL, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_netcfg_set_link_state(&netcfg, NULL, 0));
}
