/**
 * @file test_cannm.c
 * @brief Unit tests for AUTOSAR CAN Network Management (CanNm) Protocol.
 */

#include "syntropic/proto/syn_cannm.h"
#include "unity/unity.h"

#include <string.h>

void test_cannm_init(void)
{
    SYN_CanNM_Session session;
    syn_cannm_init(&session, NULL);

    TEST_ASSERT_EQUAL_UINT8(0x01U, session.config.node_id);
    TEST_ASSERT_EQUAL_UINT32(0x400U, session.config.can_id_base);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_BUS_SLEEP, session.state);

    SYN_CanNM_Config custom_cfg = {.node_id = 0x05U,
                                   .can_id_base = 0x500U,
                                   .can_id_mask = 0x7F0U,
                                   .msg_cycle_ms = 50U,
                                   .nm_timeout_ms = 500U,
                                   .wait_bus_sleep_ms = 1000U,
                                   .repeat_msg_time_ms = 800U};
    syn_cannm_init(&session, &custom_cfg);
    TEST_ASSERT_EQUAL_UINT8(0x05U, session.config.node_id);
    TEST_ASSERT_EQUAL_UINT32(0x500U, session.config.can_id_base);
}

void test_cannm_null_params(void)
{
    SYN_CAN_Frame tx_frame;
    TEST_ASSERT_FALSE(syn_cannm_step(NULL, 10, &tx_frame));
    TEST_ASSERT_FALSE(syn_cannm_process_rx_frame(NULL, &tx_frame));

    SYN_CanNM_Session session;
    syn_cannm_init(&session, NULL);
    TEST_ASSERT_FALSE(syn_cannm_process_rx_frame(&session, NULL));

    syn_cannm_request_network(NULL);
    syn_cannm_release_network(NULL);
    syn_cannm_request_repeat_msg(NULL);
    syn_cannm_set_user_data(NULL, NULL, 0);
}

void test_cannm_wakeup_and_tx(void)
{
    SYN_CanNM_Session session;
    syn_cannm_init(&session, NULL);

    uint8_t payload[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    syn_cannm_set_user_data(&session, payload, sizeof(payload));

    syn_cannm_request_network(&session);
    TEST_ASSERT_TRUE(session.node_comm_req);

    SYN_CAN_Frame tx_frame;
    bool has_tx = syn_cannm_step(&session, 1, &tx_frame);
    TEST_ASSERT_TRUE(has_tx);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_REPEAT_MSG, session.state);
    TEST_ASSERT_EQUAL_UINT32(0x401U, tx_frame.id);
    TEST_ASSERT_EQUAL_UINT8(8U, tx_frame.dlc);
    TEST_ASSERT_EQUAL_UINT8(0x01U, tx_frame.data[0]); /* Node ID */
    TEST_ASSERT_EQUAL_UINT8(SYN_CANNM_CBV_ACTIVE_WAKEUP_REQ,
                            tx_frame.data[1] & SYN_CANNM_CBV_ACTIVE_WAKEUP_REQ);
    TEST_ASSERT_EQUAL_MEMORY(payload, &tx_frame.data[2], 6);
}

void test_cannm_full_state_cycle(void)
{
    SYN_CanNM_Session session;
    syn_cannm_init(&session, NULL);

    /* 1. Wakeup -> REPEAT_MSG */
    syn_cannm_request_network(&session);
    SYN_CAN_Frame tx_frame;
    syn_cannm_step(&session, 1, &tx_frame);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_REPEAT_MSG, session.state);

    /* 2. REPEAT_MSG duration expires -> NORMAL_OP */
    syn_cannm_step(&session, 1600, &tx_frame);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_NORMAL_OP, session.state);

    /* 3. Application releases network -> READY_SLEEP */
    syn_cannm_release_network(&session);
    syn_cannm_step(&session, 10, &tx_frame);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_READY_SLEEP, session.state);

    /* 4. Timeout expires -> PRE_BUS_SLEEP */
    syn_cannm_step(&session, 1000, &tx_frame);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_PRE_BUS_SLEEP, session.state);

    /* 5. Wait bus sleep expires -> BUS_SLEEP */
    syn_cannm_step(&session, 1500, &tx_frame);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_BUS_SLEEP, session.state);
}

void test_cannm_rx_filtering_and_passive_wakeup(void)
{
    SYN_CanNM_Session session;
    syn_cannm_init(&session, NULL);

    SYN_CAN_Frame short_frame = {.id = 0x401U, .dlc = 1};
    TEST_ASSERT_FALSE(syn_cannm_process_rx_frame(&session, &short_frame));

    SYN_CAN_Frame invalid_id_frame = {.id = 0x100U, .dlc = 8};
    TEST_ASSERT_FALSE(syn_cannm_process_rx_frame(&session, &invalid_id_frame));

    SYN_CAN_Frame remote_nm_frame = {
        .id = 0x402U,
        .dlc = 8U,
        .data = {0x02U, SYN_CANNM_CBV_REPEAT_MSG_REQ, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66}};

    TEST_ASSERT_TRUE(syn_cannm_process_rx_frame(&session, &remote_nm_frame));
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_REPEAT_MSG, session.state);
    TEST_ASSERT_EQUAL_UINT8(0x02U, session.rx_source_node_id);
    TEST_ASSERT_EQUAL_UINT8(SYN_CANNM_CBV_REPEAT_MSG_REQ, session.rx_cbv);

    uint8_t expected_rx[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    TEST_ASSERT_EQUAL_MEMORY(expected_rx, session.rx_user_data, 6);

    /* Rx frame with dlc = 4 */
    SYN_CAN_Frame dlc4_frame = {.id = 0x403U, .dlc = 4U, .data = {0x03U, 0x00U, 0x12, 0x34}};
    TEST_ASSERT_TRUE(syn_cannm_process_rx_frame(&session, &dlc4_frame));

    /* Rx frame in NORMAL_OP with REPEAT_MSG_REQ bit set */
    session.state = SYN_CANNM_STATE_NORMAL_OP;
    TEST_ASSERT_TRUE(syn_cannm_process_rx_frame(&session, &remote_nm_frame));
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_REPEAT_MSG, session.state);

    /* Rx frame in PRE_BUS_SLEEP */
    session.state = SYN_CANNM_STATE_PRE_BUS_SLEEP;
    TEST_ASSERT_TRUE(syn_cannm_process_rx_frame(&session, &remote_nm_frame));
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_REPEAT_MSG, session.state);

    /* Request repeat message transition explicitly */
    session.state = SYN_CANNM_STATE_NORMAL_OP;
    session.node_comm_req = true;
    syn_cannm_request_repeat_msg(&session);
    SYN_CAN_Frame tx_frame;
    syn_cannm_step(&session, 1, &tx_frame);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_REPEAT_MSG, session.state);
}

void test_cannm_edge_branches(void)
{
    SYN_CanNM_Session session;
    syn_cannm_init(NULL, NULL);
    syn_cannm_init(&session, NULL);

    /* BUS_SLEEP step with node_comm_req = false */
    session.node_comm_req = false;
    session.state = SYN_CANNM_STATE_BUS_SLEEP;
    TEST_ASSERT_FALSE(syn_cannm_step(&session, 10, NULL));

    /* Test repeat_msg_req Tx packing */
    session.state = SYN_CANNM_STATE_REPEAT_MSG;
    session.repeat_msg_req = true;
    session.repeat_msg_timer = 1600U;
    session.msg_cycle_timer = 0U;
    SYN_CAN_Frame tx_frame;
    TEST_ASSERT_TRUE(syn_cannm_step(&session, 1, &tx_frame));
    TEST_ASSERT_EQUAL_UINT8(SYN_CANNM_CBV_REPEAT_MSG_REQ,
                            tx_frame.data[1] & SYN_CANNM_CBV_REPEAT_MSG_REQ);
    session.repeat_msg_req = false;

    /* Test NORMAL_OP cycle Tx */
    session.state = SYN_CANNM_STATE_NORMAL_OP;
    session.node_comm_req = true;
    session.msg_cycle_timer = 0U;
    TEST_ASSERT_TRUE(syn_cannm_step(&session, 1, &tx_frame));

    /* Test null tx_frame in step */
    session.state = SYN_CANNM_STATE_REPEAT_MSG;
    session.msg_cycle_timer = 0U;
    TEST_ASSERT_FALSE(syn_cannm_step(&session, 1, NULL));

    /* Test partial timer decrements */
    session.node_comm_req = false;
    session.state = SYN_CANNM_STATE_PRE_BUS_SLEEP;
    session.wait_bus_sleep_timer = 1000U;
    syn_cannm_step(&session, 10, NULL);
    TEST_ASSERT_EQUAL_UINT32(990U, session.wait_bus_sleep_timer);

    session.state = SYN_CANNM_STATE_REPEAT_MSG;
    session.repeat_msg_timer = 1000U;
    session.msg_cycle_timer = 100U;
    syn_cannm_step(&session, 10, NULL);
    TEST_ASSERT_EQUAL_UINT32(990U, session.repeat_msg_timer);
    TEST_ASSERT_EQUAL_UINT32(90U, session.msg_cycle_timer);

    session.state = SYN_CANNM_STATE_NORMAL_OP;
    session.node_comm_req = true;
    session.msg_cycle_timer = 100U;
    syn_cannm_step(&session, 10, NULL);
    TEST_ASSERT_EQUAL_UINT32(90U, session.msg_cycle_timer);

    session.node_comm_req = false;
    session.state = SYN_CANNM_STATE_READY_SLEEP;
    session.timeout_timer = 1000U;
    syn_cannm_step(&session, 10, NULL);
    TEST_ASSERT_EQUAL_UINT32(990U, session.timeout_timer);

    /* READY_SLEEP + repeat_msg_req */
    syn_cannm_request_repeat_msg(&session);
    syn_cannm_step(&session, 1, NULL);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_REPEAT_MSG, session.state);

    /* READY_SLEEP + node_comm_req */
    session.state = SYN_CANNM_STATE_READY_SLEEP;
    session.repeat_msg_req = false;
    session.node_comm_req = true;
    syn_cannm_step(&session, 1, NULL);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_NORMAL_OP, session.state);

    /* REPEAT_MSG -> READY_SLEEP when !node_comm_req */
    session.state = SYN_CANNM_STATE_REPEAT_MSG;
    session.repeat_msg_timer = 10U;
    session.node_comm_req = false;
    syn_cannm_step(&session, 10, NULL);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_READY_SLEEP, session.state);

    /* PRE_BUS_SLEEP wakeup via node_comm_req */
    session.state = SYN_CANNM_STATE_PRE_BUS_SLEEP;
    session.node_comm_req = true;
    syn_cannm_step(&session, 1, NULL);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_REPEAT_MSG, session.state);
}

void test_r5_a13_repeat_msg_cbv_and_active_wakeup_latch(void)
{
    SYN_CanNM_Session session;
    SYN_CAN_Frame tx_frame;
    syn_cannm_init(&session, NULL);

    /* 1. Passive wakeup from BUS_SLEEP must NOT set ACTIVE_WAKEUP_REQ even if network requested
     * later */
    SYN_CAN_Frame remote_nm = {.id = 0x402U, .dlc = 8U, .data = {0x02U, 0x00U}};
    TEST_ASSERT_TRUE(syn_cannm_process_rx_frame(&session, &remote_nm));
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_REPEAT_MSG, session.state);
    TEST_ASSERT_FALSE(session.active_wakeup);

    syn_cannm_request_network(&session);
    TEST_ASSERT_FALSE(session.active_wakeup);
    TEST_ASSERT_TRUE(syn_cannm_step(&session, 1U, &tx_frame));
    TEST_ASSERT_EQUAL_UINT8(0U, tx_frame.data[1] & SYN_CANNM_CBV_ACTIVE_WAKEUP_REQ);

    /* Advance to NORMAL_OP and request Repeat Message: must emit CBV 0x01 BEFORE clearing flag */
    TEST_ASSERT_TRUE(syn_cannm_step(&session, 1600U, &tx_frame));
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_NORMAL_OP, session.state);

    syn_cannm_request_repeat_msg(&session);
    TEST_ASSERT_TRUE(syn_cannm_step(&session, 1U, &tx_frame));
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_REPEAT_MSG, session.state);
    TEST_ASSERT_EQUAL_UINT8(SYN_CANNM_CBV_REPEAT_MSG_REQ,
                            tx_frame.data[1] & SYN_CANNM_CBV_REPEAT_MSG_REQ);
    TEST_ASSERT_TRUE(session.repeat_msg_req);

    /* Subsequent cycle in REPEAT_MSG keeps CBV_REPEAT_MSG_REQ set until state exit
     * (SWS_CanNm_00107) */
    TEST_ASSERT_TRUE(syn_cannm_step(&session, 100U, &tx_frame));
    TEST_ASSERT_EQUAL_UINT8(SYN_CANNM_CBV_REPEAT_MSG_REQ,
                            tx_frame.data[1] & SYN_CANNM_CBV_REPEAT_MSG_REQ);

    /* Exiting REPEAT_MSG clears repeat_msg_req */
    TEST_ASSERT_TRUE(syn_cannm_step(&session, 1500U, &tx_frame));
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_NORMAL_OP, session.state);
    TEST_ASSERT_FALSE(session.repeat_msg_req);
    TEST_ASSERT_EQUAL_UINT8(0U, tx_frame.data[1] & SYN_CANNM_CBV_REPEAT_MSG_REQ);

    /* Also verify READY_SLEEP -> REPEAT_MSG emits CBV 0x01 */
    syn_cannm_release_network(&session);
    (void)syn_cannm_step(&session, 10U, &tx_frame);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_READY_SLEEP, session.state);
    syn_cannm_request_repeat_msg(&session);
    TEST_ASSERT_TRUE(syn_cannm_step(&session, 1U, &tx_frame));
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_REPEAT_MSG, session.state);
    TEST_ASSERT_EQUAL_UINT8(SYN_CANNM_CBV_REPEAT_MSG_REQ,
                            tx_frame.data[1] & SYN_CANNM_CBV_REPEAT_MSG_REQ);

    /* Let session go all the way to BUS_SLEEP */
    (void)syn_cannm_step(&session, 1600U, &tx_frame);
    (void)syn_cannm_step(&session, 1000U, &tx_frame);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_PRE_BUS_SLEEP, session.state);
    (void)syn_cannm_step(&session, 1500U, &tx_frame);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_BUS_SLEEP, session.state);

    /* 2. Active wakeup from BUS_SLEEP latches ACTIVE_WAKEUP_REQ until BUS_SLEEP */
    syn_cannm_request_network(&session);
    TEST_ASSERT_TRUE(session.active_wakeup);
    /* Even if an Rx frame arrives before step(), active_wakeup stays latched */
    TEST_ASSERT_TRUE(syn_cannm_process_rx_frame(&session, &remote_nm));
    TEST_ASSERT_TRUE(session.active_wakeup);
    /* Release network while still in REPEAT_MSG: active_wakeup remains latched until BUS_SLEEP */
    syn_cannm_release_network(&session);
    TEST_ASSERT_TRUE(syn_cannm_step(&session, 1U, &tx_frame));
    TEST_ASSERT_EQUAL_UINT8(SYN_CANNM_CBV_ACTIVE_WAKEUP_REQ,
                            tx_frame.data[1] & SYN_CANNM_CBV_ACTIVE_WAKEUP_REQ);

    (void)syn_cannm_step(&session, 1600U, &tx_frame);
    (void)syn_cannm_step(&session, 1000U, &tx_frame);
    TEST_ASSERT_TRUE(session.active_wakeup);
    (void)syn_cannm_step(&session, 1500U, &tx_frame);
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_BUS_SLEEP, session.state);
    TEST_ASSERT_FALSE(session.active_wakeup);

    /* 3. Repeat Message timer expiring while msg_cycle_timer still has remaining time */
    SYN_CanNM_Config short_rep_cfg = {
        .node_id = 0x12U,
        .can_id_base = 0x500U,
        .can_id_mask = 0x7F0U,
        .msg_cycle_ms = 200U,
        .nm_timeout_ms = 1000U,
        .wait_bus_sleep_ms = 500U,
        .repeat_msg_time_ms = 50U,
    };
    SYN_CanNM_Session short_sess;
    syn_cannm_init(&short_sess, &short_rep_cfg);
    syn_cannm_request_network(&short_sess);
    TEST_ASSERT_TRUE(syn_cannm_step(&short_sess, 10U, &tx_frame));
    syn_cannm_request_repeat_msg(&short_sess);
    TEST_ASSERT_FALSE(syn_cannm_step(&short_sess, 50U, &tx_frame));
    TEST_ASSERT_EQUAL(SYN_CANNM_STATE_NORMAL_OP, short_sess.state);
    TEST_ASSERT_FALSE(short_sess.repeat_msg_req);
}

void run_cannm_tests(void)
{
    RUN_TEST(test_cannm_init);
    RUN_TEST(test_cannm_null_params);
    RUN_TEST(test_cannm_wakeup_and_tx);
    RUN_TEST(test_cannm_full_state_cycle);
    RUN_TEST(test_cannm_rx_filtering_and_passive_wakeup);
    RUN_TEST(test_cannm_edge_branches);
    RUN_TEST(test_r5_a13_repeat_msg_cbv_and_active_wakeup_latch);
}
