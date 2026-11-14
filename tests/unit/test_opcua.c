/**
 * @file test_opcua.c
 * @brief Complete unit tests for Micro OPC UA Binary Server Engine (100% coverage).
 */

#include "unity/unity.h"

#include <string.h>
#include <syntropic/proto/syn_opcua.h>
#include <syntropic/sched/syn_sched.h>

/* ── Loopback / Mock Transport ───────────────────────────────────────────── */

typedef struct {
    uint8_t tx_buf[1024];
    size_t tx_len;
    uint8_t rx_buf[1024];
    size_t rx_len;
    bool fail_send;
    bool has_data_ready;
} MockOPCUATransport;

static bool mock_opcua_send(const uint8_t *data, size_t len, void *user_ctx)
{
    MockOPCUATransport *mock = (MockOPCUATransport *)user_ctx;
    if (mock->fail_send) {
        return false;
    }
    if (len > sizeof(mock->tx_buf)) {
        return false;
    }
    (void)memcpy(mock->tx_buf, data, len);
    mock->tx_len = len;
    return true;
}

static bool mock_opcua_recv(uint8_t *buf, size_t max_len, size_t *out_len, void *user_ctx)
{
    MockOPCUATransport *mock = (MockOPCUATransport *)user_ctx;
    if (mock->rx_len == 0U) {
        return false;
    }
    size_t copy_len = (mock->rx_len < max_len) ? mock->rx_len : max_len;
    (void)memcpy(buf, mock->rx_buf, copy_len);
    *out_len = copy_len;
    mock->rx_len = 0U;
    return true;
}

/* ── Callbacks ───────────────────────────────────────────────────────────── */

static bool g_node_read_called = false;
static bool g_node_write_called = false;

static SYN_Status on_temperature_read(SYN_OPCUA_Server *srv, const SYN_OPCUA_Node *node,
                                      SYN_OPCUA_DataValue *out_val, void *user_data)
{
    (void)srv;
    (void)node;
    (void)user_data;
    g_node_read_called = true;
    out_val->value.type = SYN_OPCUA_TYPE_FLOAT;
    out_val->value.val.float_val = 24.5f;
    out_val->status_code = SYN_OPCUA_STATUS_GOOD;
    return SYN_OK;
}

static SYN_Status on_setpoint_write(SYN_OPCUA_Server *srv, const SYN_OPCUA_Node *node,
                                    const SYN_OPCUA_DataValue *in_val, void *user_data)
{
    (void)srv;
    (void)node;
    (void)in_val;
    (void)user_data;
    g_node_write_called = true;
    return SYN_OK;
}

/* ── Tests ───────────────────────────────────────────────────────────────── */

void test_opcua_server_init_and_validation(void)
{
    SYN_OPCUA_Server srv;
    SYN_OPCUA_Config cfg;
    (void)memset(&cfg, 0, sizeof(cfg));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_opcua_server_init(NULL, &cfg));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_opcua_server_init(&srv, NULL));

    uint8_t rx[512], tx[512];
    cfg.rx_buf = rx;
    cfg.rx_buf_size = sizeof(rx);
    cfg.tx_buf = tx;
    cfg.tx_buf_size = sizeof(tx);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_init(&srv, &cfg));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_CLOSED, srv.state);
    TEST_ASSERT_GREATER_THAN_UINT(0, srv.node_count); /* Root, Objects, Server, ServerStatus */

    /* Small buffer check */
    cfg.rx_buf_size = 64;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_opcua_server_init(&srv, &cfg));
}

void test_opcua_server_address_space_and_variables(void)
{
    uint8_t rx[512], tx[512];
    SYN_OPCUA_Config cfg = {
        .rx_buf = rx,
        .rx_buf_size = sizeof(rx),
        .tx_buf = tx,
        .tx_buf_size = sizeof(tx),
    };

    SYN_OPCUA_Server srv;
    syn_opcua_server_init(&srv, &cfg);

    /* Null validations */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_opcua_server_register_node(NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_opcua_server_register_node(&srv, NULL));
    TEST_ASSERT_NULL(syn_opcua_server_find_node_num(NULL, 0, 0));
    TEST_ASSERT_NULL(syn_opcua_server_find_node_num(&srv, 99, 9999));

    SYN_OPCUA_Variant dummy_var = {.type = SYN_OPCUA_TYPE_FLOAT};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_opcua_server_write_variable(NULL, 0, 0, &dummy_var));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_opcua_server_write_variable(&srv, 0, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_opcua_server_write_variable(&srv, 99, 9999, &dummy_var));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_opcua_server_read_variable(NULL, 0, 0, &dummy_var));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_opcua_server_read_variable(&srv, 0, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_opcua_server_read_variable(&srv, 99, 9999, &dummy_var));

    /* Register Custom Sensor Variable Node */
    SYN_OPCUA_Node temp_node = {
        .node_id = {.ns_index = 1U, .id_type = SYN_OPCUA_NODEID_NUMERIC, .id = {.num = 5001U}},
        .parent_id = {.ns_index = 0U,
                      .id_type = SYN_OPCUA_NODEID_NUMERIC,
                      .id = {.num = SYN_OPCUA_NODEID_OBJECTS_FOLDER}},
        .node_class = SYN_OPCUA_NODECLASS_VARIABLE,
        .browse_name = "Temperature",
        .display_name = "Ambient Temperature",
        .data_type = SYN_OPCUA_TYPE_FLOAT,
        .value = {.value = {.type = SYN_OPCUA_TYPE_FLOAT, .val = {.float_val = 21.0f}},
                  .status_code = SYN_OPCUA_STATUS_GOOD},
        .access_level = 0x03U, /* Read/Write */
        .on_read = on_temperature_read,
        .on_write = on_setpoint_write,
    };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_register_node(&srv, &temp_node));

    /* Find Node */
    SYN_OPCUA_Node *found = syn_opcua_server_find_node_num(&srv, 1U, 5001U);
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_STRING("Temperature", found->browse_name);

    /* Write Variable */
    SYN_OPCUA_Variant new_val = {.type = SYN_OPCUA_TYPE_FLOAT, .val = {.float_val = 26.5f}};
    g_node_write_called = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_write_variable(&srv, 1U, 5001U, &new_val));
    TEST_ASSERT_TRUE(g_node_write_called);

    /* Read Variable */
    SYN_OPCUA_Variant read_back;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_read_variable(&srv, 1U, 5001U, &read_back));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_TYPE_FLOAT, read_back.type);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 26.5f, read_back.val.float_val);

    /* Register nodes to fill table */
    for (size_t i = srv.node_count; i < SYN_OPCUA_MAX_NODES; i++) {
        SYN_OPCUA_Node n = temp_node;
        n.node_id.id.num = 6000U + (uint32_t)i;
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_register_node(&srv, &n));
    }
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_opcua_server_register_node(&srv, &temp_node));
}

void test_opcua_server_all_variant_types(void)
{
    uint8_t rx[512], tx[512];
    SYN_OPCUA_Config cfg = {
        .rx_buf = rx, .rx_buf_size = sizeof(rx), .tx_buf = tx, .tx_buf_size = sizeof(tx)};
    SYN_OPCUA_Server srv;
    syn_opcua_server_init(&srv, &cfg);

    SYN_OPCUA_DataType types[] = {
        SYN_OPCUA_TYPE_BOOLEAN,  SYN_OPCUA_TYPE_SBYTE,  SYN_OPCUA_TYPE_BYTE,
        SYN_OPCUA_TYPE_INT16,    SYN_OPCUA_TYPE_UINT16, SYN_OPCUA_TYPE_INT32,
        SYN_OPCUA_TYPE_UINT32,   SYN_OPCUA_TYPE_FLOAT,  SYN_OPCUA_TYPE_INT64,
        SYN_OPCUA_TYPE_UINT64,   SYN_OPCUA_TYPE_DOUBLE, SYN_OPCUA_TYPE_STRING,
        SYN_OPCUA_TYPE_DATETIME, SYN_OPCUA_TYPE_NULL};

    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
        SYN_OPCUA_Node node = {
            .node_id = {.ns_index = (i == 0) ? 0x200U : 1U,
                        .id_type = SYN_OPCUA_NODEID_NUMERIC,
                        .id = {.num = (i == 0) ? 0x20000U : (7000U + (uint32_t)i)}},
            .node_class = SYN_OPCUA_NODECLASS_VARIABLE,
            .browse_name = "Var",
            .display_name = "Var",
            .data_type = types[i],
            .value = {.value = {.type = types[i]},
                      .status_code =
                          (i == 1) ? SYN_OPCUA_STATUS_BAD_NOT_WRITABLE : SYN_OPCUA_STATUS_GOOD,
                      .source_ts_ms = (i == 2) ? 1000U : 0U},
            .access_level = 0x01U,
        };
        if (types[i] == SYN_OPCUA_TYPE_STRING) {
            node.value.value.val.string = "TestString";
        }
        syn_opcua_server_register_node(&srv, &node);

        SYN_OPCUA_Variant v;
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_read_variable(&srv, node.node_id.ns_index,
                                                                     node.node_id.id.num, &v));
    }

    /* Send ReadRequest for all nodes to test all variant encoders */
    srv.state = SYN_OPCUA_STATE_SESSION_ACTIVE;
    uint8_t rd_msg[512];
    (void)memset(rd_msg, 0, sizeof(rd_msg));
    rd_msg[0] = 'M';
    rd_msg[1] = 'S';
    rd_msg[2] = 'G';
    rd_msg[3] = 'F';
    uint32_t one = 1U;
    (void)memcpy(&rd_msg[8], &one, 4);
    (void)memcpy(&rd_msg[12], &one, 4);
    (void)memcpy(&rd_msg[16], &one, 4);
    (void)memcpy(&rd_msg[20], &one, 4);
    rd_msg[24] = 0x01U;
    rd_msg[25] = 0x00U;
    uint16_t req_rd = 631U;
    (void)memcpy(&rd_msg[26], &req_rd, 2);

    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
        uint32_t msg_sz = 60U;
        (void)memcpy(&rd_msg[4], &msg_sz, 4);
        uint32_t cnt = 1U;
        (void)memcpy(&rd_msg[44], &cnt, 4);
        rd_msg[48] = (i == 0) ? 0x02U : 0x01U; /* NodeId format */
        if (i == 0) {
            uint16_t ns = 0x200U;
            uint32_t num = 0x20000U;
            (void)memcpy(&rd_msg[49], &ns, 2);
            (void)memcpy(&rd_msg[51], &num, 4);
        } else {
            rd_msg[49] = 0x01U;
            uint16_t num = 7000U + (uint16_t)i;
            (void)memcpy(&rd_msg[50], &num, 2);
        }
        uint8_t resp_buf[512];
        size_t resp_len = 0U;
        (void)syn_opcua_server_process_message(&srv, rd_msg, 60U, resp_buf, sizeof(resp_buf),
                                               &resp_len);
    }

    /* TwoByte NodeId Read */
    uint32_t msg_sz = 55U;
    (void)memcpy(&rd_msg[4], &msg_sz, 4);
    rd_msg[48] = 0x00U;
    rd_msg[49] = 0x55U;
    uint8_t resp_buf2[512];
    size_t resp_len2 = 0U;
    (void)syn_opcua_server_process_message(&srv, rd_msg, 55U, resp_buf2, sizeof(resp_buf2),
                                           &resp_len2);

    /* NodeId reader underflow & unsupported encoding tests */
    msg_sz = 49U;
    (void)memcpy(&rd_msg[4], &msg_sz, 4);
    rd_msg[48] = 0x00U;
    (void)syn_opcua_server_process_message(&srv, rd_msg, 49U, resp_buf2, sizeof(resp_buf2),
                                           &resp_len2);
    msg_sz = 50U;
    (void)memcpy(&rd_msg[4], &msg_sz, 4);
    rd_msg[48] = 0x01U;
    (void)syn_opcua_server_process_message(&srv, rd_msg, 50U, resp_buf2, sizeof(resp_buf2),
                                           &resp_len2);
    msg_sz = 51U;
    (void)memcpy(&rd_msg[4], &msg_sz, 4);
    rd_msg[48] = 0x02U;
    (void)syn_opcua_server_process_message(&srv, rd_msg, 51U, resp_buf2, sizeof(resp_buf2),
                                           &resp_len2);
    msg_sz = 55U;
    (void)memcpy(&rd_msg[4], &msg_sz, 4);
    rd_msg[48] = 0x03U;
    (void)syn_opcua_server_process_message(&srv, rd_msg, 55U, resp_buf2, sizeof(resp_buf2),
                                           &resp_len2);

    /* Unknown variant type read */
    SYN_OPCUA_Node unk_node = {
        .node_id = {.ns_index = 1U, .id_type = SYN_OPCUA_NODEID_NUMERIC, .id = {.num = 9999U}},
        .browse_name = "Unk",
        .display_name = "Unk",
        .data_type = (SYN_OPCUA_DataType)99,
        .node_class = SYN_OPCUA_NODECLASS_VARIABLE,
        .value = {.value = {.type = (SYN_OPCUA_DataType)99}},
    };
    (void)syn_opcua_server_register_node(&srv, &unk_node);
    msg_sz = 60U;
    (void)memcpy(&rd_msg[4], &msg_sz, 4);
    uint32_t cnt = 1U;
    (void)memcpy(&rd_msg[44], &cnt, 4);
    rd_msg[48] = 0x01U;
    rd_msg[49] = 1U;
    uint16_t unk_num = 9999U;
    (void)memcpy(&rd_msg[50], &unk_num, 2);
    (void)syn_opcua_server_process_message(&srv, rd_msg, 60U, resp_buf2, sizeof(resp_buf2),
                                           &resp_len2);

    /* Write to variable without on_write callback */
    SYN_OPCUA_Variant wr_val = {.type = SYN_OPCUA_TYPE_INT32, .val = {.int32 = 42}};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_write_variable(&srv, 1U, 7005U, &wr_val));
}

void test_opcua_server_uacp_uasc_and_services(void)
{
    MockOPCUATransport mock;
    (void)memset(&mock, 0, sizeof(mock));
    SYN_Transport transport = {.send = mock_opcua_send, .recv = mock_opcua_recv, .ctx = &mock};

    uint8_t rx[1024], tx[1024];
    SYN_OPCUA_Config cfg = {
        .transport = &transport,
        .rx_buf = rx,
        .rx_buf_size = sizeof(rx),
        .tx_buf = tx,
        .tx_buf_size = sizeof(tx),
    };

    SYN_OPCUA_Server srv;
    syn_opcua_server_init(&srv, &cfg);

    uint8_t resp_buf[1024];
    size_t resp_len = 0U;

    /* Process message null validations */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_opcua_server_process_message(NULL, rx, 10, tx, 10, &resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_opcua_server_process_message(&srv, NULL, 10, tx, 10, &resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_opcua_server_process_message(&srv, rx, 10, NULL, 10, &resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_opcua_server_process_message(&srv, rx, 10, tx, 10, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_opcua_server_process_message(&srv, rx, 4, tx, 10, &resp_len));

    /* 1. Send Hello Message (HELF) */
    uint8_t hel_msg[] = {
        'H',  'E',  'L',  'F',  28, 0, 0, 0, /* MessageSize = 28 */
        0,    0,    0,    0,                 /* ProtocolVersion = 0 */
        0x00, 0x04, 0x00, 0x00,              /* ReceiveBufferSize = 1024 */
        0x00, 0x04, 0x00, 0x00,              /* SendBufferSize = 1024 */
        0x00, 0x04, 0x00, 0x00,              /* MaxMessageSize = 1024 */
        1,    0,    0,    0,                 /* MaxChunkCount = 1 */
    };
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_opcua_server_process_message(&srv, hel_msg, sizeof(hel_msg), resp_buf,
                                                           sizeof(resp_buf), &resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_HELLO_RECEIVED, srv.state);
    TEST_ASSERT_EQUAL_HEX8('A', resp_buf[0]);
    TEST_ASSERT_EQUAL_HEX8('C', resp_buf[1]);
    TEST_ASSERT_EQUAL_HEX8('K', resp_buf[2]);
    TEST_ASSERT_EQUAL_HEX8('F', resp_buf[3]);

    /* 2. Send OpenSecureChannel (OPNF) */
    uint8_t opn_msg[64];
    (void)memset(opn_msg, 0, sizeof(opn_msg));
    opn_msg[0] = 'O';
    opn_msg[1] = 'P';
    opn_msg[2] = 'N';
    opn_msg[3] = 'F';
    uint32_t opn_sz = 32U;
    (void)memcpy(&opn_msg[4], &opn_sz, 4);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_process_message(&srv, opn_msg, 32U, resp_buf,
                                                                   sizeof(resp_buf), &resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_SECURE_CHANNEL, srv.state);
    TEST_ASSERT_EQUAL_HEX8('O', resp_buf[0]);

    /* 3. Send CreateSessionRequest (MSGF, NodeId 461) */
    uint8_t cs_msg[64];
    (void)memset(cs_msg, 0, sizeof(cs_msg));
    cs_msg[0] = 'M';
    cs_msg[1] = 'S';
    cs_msg[2] = 'G';
    cs_msg[3] = 'F';
    uint32_t cs_sz = 32U;
    (void)memcpy(&cs_msg[4], &cs_sz, 4);
    uint32_t one = 1U;
    (void)memcpy(&cs_msg[8], &one, 4);
    (void)memcpy(&cs_msg[12], &one, 4);
    (void)memcpy(&cs_msg[16], &one, 4);
    (void)memcpy(&cs_msg[20], &one, 4);
    cs_msg[24] = 0x01U;
    cs_msg[25] = 0x00U;
    uint16_t req_cs = 461U;
    (void)memcpy(&cs_msg[26], &req_cs, 2);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_process_message(&srv, cs_msg, 32U, resp_buf,
                                                                   sizeof(resp_buf), &resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_SESSION_CREATED, srv.state);

    /* Register large node to test Numeric NodeId encoding in Browse */
    SYN_OPCUA_Node large_node = {
        .node_id = {.ns_index = 0x200U,
                    .id_type = SYN_OPCUA_NODEID_NUMERIC,
                    .id = {.num = 0x20000U}},
        .browse_name = "LargeNum",
        .display_name = "LargeNum",
        .data_type = SYN_OPCUA_TYPE_INT32,
        .node_class = SYN_OPCUA_NODECLASS_VARIABLE,
    };
    (void)syn_opcua_server_register_node(&srv, &large_node);

    /* 4. Send ActivateSessionRequest (MSGF, NodeId 467) */
    uint16_t req_act = 467U;
    (void)memcpy(&cs_msg[26], &req_act, 2);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_process_message(&srv, cs_msg, 32U, resp_buf,
                                                                   sizeof(resp_buf), &resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_SESSION_ACTIVE, srv.state);

    /* Short MSG reader underflow test */
    uint32_t short_sz = 12U;
    (void)memcpy(&cs_msg[4], &short_sz, 4);
    (void)syn_opcua_server_process_message(&srv, cs_msg, 12U, resp_buf, sizeof(resp_buf),
                                           &resp_len);
    (void)memcpy(&cs_msg[4], &cs_sz, 4);

    /* 5. Send BrowseRequest (MSGF, NodeId 527) */
    uint8_t br_msg[64];
    (void)memset(br_msg, 0, sizeof(br_msg));
    br_msg[0] = 'M';
    br_msg[1] = 'S';
    br_msg[2] = 'G';
    br_msg[3] = 'F';
    uint32_t br_sz = 56U;
    (void)memcpy(&br_msg[4], &br_sz, 4);
    (void)memcpy(&br_msg[8], &one, 4);
    (void)memcpy(&br_msg[12], &one, 4);
    (void)memcpy(&br_msg[16], &one, 4);
    (void)memcpy(&br_msg[20], &one, 4);
    br_msg[24] = 0x01U;
    br_msg[25] = 0x00U;
    uint16_t req_br = 527U;
    (void)memcpy(&br_msg[26], &req_br, 2);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_process_message(&srv, br_msg, 56U, resp_buf,
                                                                   sizeof(resp_buf), &resp_len));
    TEST_ASSERT_GREATER_THAN_UINT(30, resp_len);

    /* Register writable node and node with on_read callback */
    SYN_OPCUA_Node rw_node = {
        .node_id = {.ns_index = 1U, .id_type = SYN_OPCUA_NODEID_NUMERIC, .id = {.num = 2000U}},
        .browse_name = "RWNode",
        .display_name = "RWNode",
        .data_type = SYN_OPCUA_TYPE_INT32,
        .node_class = SYN_OPCUA_NODECLASS_VARIABLE,
        .access_level = 0x03U, /* Read & Write */
        .on_read = on_temperature_read,
    };
    (void)syn_opcua_server_register_node(&srv, &rw_node);

    /* 6. Send ReadRequest with count = 20 (testing count > 16 clamp) */
    uint8_t rd_msg[128];
    (void)memset(rd_msg, 0, sizeof(rd_msg));
    rd_msg[0] = 'M';
    rd_msg[1] = 'S';
    rd_msg[2] = 'G';
    rd_msg[3] = 'F';
    uint32_t rd_sz = 56U;
    (void)memcpy(&rd_msg[4], &rd_sz, 4);
    (void)memcpy(&rd_msg[8], &one, 4);
    (void)memcpy(&rd_msg[12], &one, 4);
    (void)memcpy(&rd_msg[16], &one, 4);
    (void)memcpy(&rd_msg[20], &one, 4);
    rd_msg[24] = 0x01U;
    rd_msg[25] = 0x00U;
    uint16_t req_rd = 631U;
    (void)memcpy(&rd_msg[26], &req_rd, 2);
    uint32_t cnt20 = 20U;
    (void)memcpy(&rd_msg[44], &cnt20, 4);
    rd_msg[48] = 0x01U;
    rd_msg[49] = 0x01U;
    uint16_t rw_node_id = 2000U;
    (void)memcpy(&rd_msg[50], &rw_node_id, 2);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_process_message(&srv, rd_msg, 56U, resp_buf,
                                                                   sizeof(resp_buf), &resp_len));
    TEST_ASSERT_GREATER_THAN_UINT(30, resp_len);
    TEST_ASSERT_TRUE(g_node_read_called);

    /* 7. Send WriteRequest with count = 20, writable, non-writable, and unknown nodes */
    uint8_t wr_msg[128];
    (void)memset(wr_msg, 0, sizeof(wr_msg));
    wr_msg[0] = 'M';
    wr_msg[1] = 'S';
    wr_msg[2] = 'G';
    wr_msg[3] = 'F';
    uint32_t wr_sz = 66U;
    (void)memcpy(&wr_msg[4], &wr_sz, 4);
    (void)memcpy(&wr_msg[8], &one, 4);
    (void)memcpy(&wr_msg[12], &one, 4);
    (void)memcpy(&wr_msg[16], &one, 4);
    (void)memcpy(&wr_msg[20], &one, 4);
    wr_msg[24] = 0x01U;
    wr_msg[25] = 0x00U;
    uint16_t req_wr = 673U;
    (void)memcpy(&wr_msg[26], &req_wr, 2);
    (void)memcpy(&wr_msg[44], &cnt20, 4);
    /* Node 1: Writable (ns=1, num=2000), attr=13, null IndexRange, DataValue INT32=1234 */
    wr_msg[48] = 0x01U;
    wr_msg[49] = 0x01U;
    uint16_t rw_id = 2000U;
    (void)memcpy(&wr_msg[50], &rw_id, 2);
    uint32_t attr_val = SYN_OPCUA_ATTR_VALUE;
    uint32_t null_str = 0xFFFFFFFFU;
    int32_t wr_i32 = 1234;
    (void)memcpy(&wr_msg[52], &attr_val, 4);
    (void)memcpy(&wr_msg[56], &null_str, 4);
    wr_msg[60] = 0x01U;
    wr_msg[61] = (uint8_t)SYN_OPCUA_TYPE_INT32;
    (void)memcpy(&wr_msg[62], &wr_i32, 4);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_process_message(&srv, wr_msg, 66U, resp_buf,
                                                                   sizeof(resp_buf), &resp_len));

    /* Write to non-writable node (ns=0, num=2256) */
    uint32_t cnt1 = 1U;
    (void)memcpy(&wr_msg[44], &cnt1, 4);
    wr_msg[48] = 0x01U;
    wr_msg[49] = 0x00U;
    uint16_t srv_status_id = 2256U;
    (void)memcpy(&wr_msg[50], &srv_status_id, 2);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_process_message(&srv, wr_msg, 66U, resp_buf,
                                                                   sizeof(resp_buf), &resp_len));

    /* Write to unknown node (ns=1, num=9999) */
    uint16_t unk_wr_id = 9999U;
    wr_msg[49] = 0x01U;
    (void)memcpy(&wr_msg[50], &unk_wr_id, 2);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_process_message(&srv, wr_msg, 66U, resp_buf,
                                                                   sizeof(resp_buf), &resp_len));

    /* 8. Tiny Response Buffer tests for writer overflow handling (while SESSION_ACTIVE) */
    uint8_t micro_resp[2];
    size_t tiny_resp_len = 0U;
    uint8_t tiny_resp[8];
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_opcua_server_process_message(&srv, cs_msg, 32U, tiny_resp,
                                                           sizeof(tiny_resp), &tiny_resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_opcua_server_process_message(&srv, rd_msg, 56U, tiny_resp,
                                                           sizeof(tiny_resp), &tiny_resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_opcua_server_process_message(&srv, wr_msg, 66U, tiny_resp,
                                                           sizeof(tiny_resp), &tiny_resp_len));
    uint8_t mid_resp[43];
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_opcua_server_process_message(&srv, br_msg, 56U, mid_resp,
                                                           sizeof(mid_resp), &tiny_resp_len));
    uint8_t mid_resp2[30];
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_opcua_server_process_message(&srv, br_msg, 56U, mid_resp2,
                                                           sizeof(mid_resp2), &tiny_resp_len));

    /* 9. Send CloseSessionRequest (MSGF, NodeId 473) */
    uint16_t req_close = 473U;
    (void)memcpy(&cs_msg[26], &req_close, 2);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_process_message(&srv, cs_msg, 32U, resp_buf,
                                                                   sizeof(resp_buf), &resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_SECURE_CHANNEL, srv.state);

    /* 10. Send Unsupported Service (MSGF, NodeId 999) */
    uint16_t req_unsupported = 999U;
    (void)memcpy(&cs_msg[26], &req_unsupported, 2);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_process_message(&srv, cs_msg, 32U, resp_buf,
                                                                   sizeof(resp_buf), &resp_len));

    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_opcua_server_process_message(&srv, hel_msg, sizeof(hel_msg), micro_resp,
                                                    sizeof(micro_resp), &tiny_resp_len));
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_opcua_server_process_message(&srv, hel_msg, sizeof(hel_msg), tiny_resp,
                                                    sizeof(tiny_resp), &tiny_resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_opcua_server_process_message(&srv, opn_msg, 32U, tiny_resp,
                                                           sizeof(tiny_resp), &tiny_resp_len));

    /* Unknown message type header */
    uint8_t unk_hdr[] = {'X', 'X', 'X', 'F', 8, 0, 0, 0};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_opcua_server_process_message(&srv, unk_hdr, 8U, resp_buf,
                                                                      sizeof(resp_buf), &resp_len));

    /* 11. Step with incoming transport frame */
    (void)memcpy(mock.rx_buf, hel_msg, sizeof(hel_msg));
    mock.rx_len = sizeof(hel_msg);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_step(&srv, 100U));
    TEST_ASSERT_GREATER_THAN_UINT(0, mock.tx_len);
}

void test_opcua_server_protothread(void)
{
    MockOPCUATransport mock;
    (void)memset(&mock, 0, sizeof(mock));
    SYN_Transport transport = {.send = mock_opcua_send, .recv = mock_opcua_recv, .ctx = &mock};

    uint8_t rx[256], tx[256];
    SYN_OPCUA_Config cfg = {
        .transport = &transport,
        .rx_buf = rx,
        .rx_buf_size = sizeof(rx),
        .tx_buf = tx,
        .tx_buf_size = sizeof(tx),
    };

    SYN_OPCUA_Server srv;
    syn_opcua_server_init(&srv, &cfg);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &srv};

    SYN_PT_Status st = syn_opcua_server_pt(&pt, &task);
    TEST_ASSERT_EQUAL_INT(PT_IDLE, st);

    /* Null checks */
    TEST_ASSERT_EQUAL_INT(PT_EXITED, syn_opcua_server_pt(NULL, &task));
    TEST_ASSERT_EQUAL_INT(PT_EXITED, syn_opcua_server_pt(&pt, NULL));
    SYN_Task null_task = {.user_data = NULL};
    TEST_ASSERT_EQUAL_INT(PT_EXITED, syn_opcua_server_pt(&pt, &null_task));

    /* Step error termination */
    srv.cfg.transport = NULL;
    st = syn_opcua_server_pt(&pt, &task);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, st);
}

static bool mock_opcua_has_data(const void *user_ctx)
{
    const MockOPCUATransport *mock = (const MockOPCUATransport *)user_ctx;
    return (mock != NULL) && mock->has_data_ready;
}

void test_opcua_server_pt_has_data_yielding(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_opcua_server_step(NULL, 0U));

    MockOPCUATransport mock;
    (void)memset(&mock, 0, sizeof(mock));
    SYN_Transport transport = {
        .send = mock_opcua_send,
        .recv = mock_opcua_recv,
        .has_data = mock_opcua_has_data,
        .ctx = &mock,
    };

    uint8_t rx[256], tx[256];
    SYN_OPCUA_Config cfg = {
        .transport = &transport,
        .rx_buf = rx,
        .rx_buf_size = sizeof(rx),
        .tx_buf = tx,
        .tx_buf_size = sizeof(tx),
    };

    SYN_OPCUA_Server srv;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_init(&srv, &cfg));

    SYN_Sched sched = {0};
    SYN_Task task = {0};
    task.user_data = &srv;
    task.sched = &sched;
    sched.running = &task;
    sched.share_contended = true;

    SYN_PT pt;
    PT_INIT(&pt);

    /* Idle has_data == true with no RX packet must yield PT_IDLE without spinning */
    sched.share_contended = false;
    mock.has_data_ready = true;
    mock.rx_len = 0U;
    TEST_ASSERT_EQUAL_INT(PT_IDLE, syn_opcua_server_pt(&pt, &task));

    /* Consumed packet + has_data == true while contended -> PT_CONTINUE_IF_NEEDED yields PT_MORE */
    uint8_t runt_pkt[4] = {0x48U, 0x45U, 0x4CU, 0x46U};
    (void)memcpy(mock.rx_buf, runt_pkt, sizeof(runt_pkt));
    mock.rx_len = sizeof(runt_pkt);
    sched.share_contended = true;
    mock.has_data_ready = true;
    TEST_ASSERT_EQUAL_INT(PT_MORE, syn_opcua_server_pt(&pt, &task));

    /* has_data returns false -> PT_PASS yields PT_IDLE */
    mock.has_data_ready = false;
    TEST_ASSERT_EQUAL_INT(PT_IDLE, syn_opcua_server_pt(&pt, &task));
}

void test_opcua_r4_p15_regressions(void)
{
    MockOPCUATransport mock;
    (void)memset(&mock, 0, sizeof(mock));
    SYN_Transport transport = {.send = mock_opcua_send, .recv = mock_opcua_recv, .ctx = &mock};

    uint8_t rx[256], tx[256];
    SYN_OPCUA_Config cfg = {
        .transport = &transport,
        .rx_buf = rx,
        .rx_buf_size = sizeof(rx),
        .tx_buf = tx,
        .tx_buf_size = sizeof(tx),
    };

    SYN_OPCUA_Server srv;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_init(&srv, &cfg));

    uint8_t resp[256];
    size_t resp_len = 0U;

    /* 1. Reject invalid MessageSize (< 8 or > rx_len) in process_message */
    uint8_t bad_sz_msg[28] = {'H', 'E', 'L', 'F', 4, 0, 0, 0};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_opcua_server_process_message(&srv, bad_sz_msg, sizeof(bad_sz_msg),
                                                           resp, sizeof(resp), &resp_len));
    bad_sz_msg[4] = 64U;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_opcua_server_process_message(&srv, bad_sz_msg, sizeof(bad_sz_msg),
                                                           resp, sizeof(resp), &resp_len));

    /* 2. Truncated ReadRequest / WriteRequest (32B < 44B header) guarded by opcua_skip */
    srv.state = SYN_OPCUA_STATE_SESSION_ACTIVE;
    uint8_t short_svc[32];
    (void)memset(short_svc, 0, sizeof(short_svc));
    (void)memcpy(short_svc, "MSGF", 4U);
    uint32_t svc_sz = 32U;
    uint32_t one = 1U;
    (void)memcpy(&short_svc[4], &svc_sz, 4U);
    (void)memcpy(&short_svc[8], &one, 4U);
    (void)memcpy(&short_svc[12], &one, 4U);
    (void)memcpy(&short_svc[16], &one, 4U);
    (void)memcpy(&short_svc[20], &one, 4U);
    short_svc[24] = 0x01U;
    short_svc[25] = 0x00U;
    uint16_t req_rd = 631U;
    (void)memcpy(&short_svc[26], &req_rd, 2U);
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_opcua_server_process_message(&srv, short_svc, sizeof(short_svc), resp,
                                                           sizeof(resp), &resp_len));
    TEST_ASSERT_GREATER_THAN_UINT(28U, resp_len);

    uint16_t req_wr = 673U;
    (void)memcpy(&short_svc[26], &req_wr, 2U);
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_opcua_server_process_message(&srv, short_svc, sizeof(short_svc), resp,
                                                           sizeof(resp), &resp_len));
    TEST_ASSERT_GREATER_THAN_UINT(28U, resp_len);

    /* 3. Coalesced TCP stream: HELF + OPNF + CreateSession + ActivateSession */
    srv.state = SYN_OPCUA_STATE_CLOSED;
    uint8_t hel_msg[28] = {
        'H',  'E',  'L',  'F',  28,   0,    0,    0,    0,    0,    0, 0, 0x00, 0x04,
        0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 1, 0, 0,    0,
    };
    uint8_t opn_msg[32];
    (void)memset(opn_msg, 0, sizeof(opn_msg));
    (void)memcpy(opn_msg, "OPNF", 4U);
    uint32_t opn_sz = 32U;
    (void)memcpy(&opn_msg[4], &opn_sz, 4U);

    uint8_t cs_msg[32];
    (void)memcpy(cs_msg, short_svc, sizeof(cs_msg));
    uint16_t req_cs = 461U;
    (void)memcpy(&cs_msg[26], &req_cs, 2U);

    uint8_t act_msg[32];
    (void)memcpy(act_msg, short_svc, sizeof(act_msg));
    uint16_t req_act = 467U;
    (void)memcpy(&act_msg[26], &req_act, 2U);

    (void)memcpy(&mock.rx_buf[0], hel_msg, sizeof(hel_msg));
    (void)memcpy(&mock.rx_buf[28], opn_msg, sizeof(opn_msg));
    (void)memcpy(&mock.rx_buf[60], cs_msg, sizeof(cs_msg));
    (void)memcpy(&mock.rx_buf[92], act_msg, sizeof(act_msg));
    mock.rx_len = 124U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_step(&srv, 200U));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_SESSION_ACTIVE, srv.state);
    TEST_ASSERT_EQUAL_UINT(0U, srv.rx_pending);

    /* 4. Complete HELF + partial trailing OPNF (10B), then remaining OPNF (22B) */
    srv.state = SYN_OPCUA_STATE_CLOSED;
    (void)memcpy(&mock.rx_buf[0], hel_msg, sizeof(hel_msg));
    (void)memcpy(&mock.rx_buf[28], opn_msg, 10U);
    mock.rx_len = 38U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_step(&srv, 210U));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_HELLO_RECEIVED, srv.state);
    TEST_ASSERT_EQUAL_UINT(10U, srv.rx_pending);

    (void)memcpy(mock.rx_buf, &opn_msg[10], 22U);
    mock.rx_len = 22U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_step(&srv, 220U));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_SECURE_CHANNEL, srv.state);
    TEST_ASSERT_EQUAL_UINT(0U, srv.rx_pending);

    /* 5. Unknown 8B frame coalesced before HELF + malformed msg_size stream recovery */
    srv.state = SYN_OPCUA_STATE_CLOSED;
    uint8_t unk8[8] = {'X', 'X', 'X', 'F', 8, 0, 0, 0};
    (void)memcpy(&mock.rx_buf[0], unk8, sizeof(unk8));
    (void)memcpy(&mock.rx_buf[8], hel_msg, sizeof(hel_msg));
    mock.rx_len = 36U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_step(&srv, 230U));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_HELLO_RECEIVED, srv.state);
    TEST_ASSERT_EQUAL_UINT(0U, srv.rx_pending);

    uint8_t corrupt_small[8] = {'H', 'E', 'L', 'F', 4, 0, 0, 0};
    (void)memcpy(mock.rx_buf, corrupt_small, sizeof(corrupt_small));
    mock.rx_len = sizeof(corrupt_small);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_step(&srv, 240U));
    TEST_ASSERT_EQUAL_UINT(0U, srv.rx_pending);

    uint8_t corrupt_huge[8] = {'H', 'E', 'L', 'F', 0xFF, 0xFF, 0x01, 0x00};
    (void)memcpy(mock.rx_buf, corrupt_huge, sizeof(corrupt_huge));
    mock.rx_len = sizeof(corrupt_huge);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_step(&srv, 250U));
    TEST_ASSERT_EQUAL_UINT(0U, srv.rx_pending);
}

static SYN_Status on_setpoint_write_fail(SYN_OPCUA_Server *srv, const SYN_OPCUA_Node *node,
                                         const SYN_OPCUA_DataValue *in_val, void *user_data)
{
    (void)srv;
    (void)node;
    (void)in_val;
    (void)user_data;
    return SYN_ERROR;
}

static size_t build_opcua_msg_hdr(uint8_t *buf, uint32_t chan_id, uint32_t tok_id,
                                  uint16_t svc_type_id)
{
    uint32_t one = 1U;
    (void)memcpy(&buf[0], "MSGF", 4U);
    (void)memcpy(&buf[8], &chan_id, 4U);
    (void)memcpy(&buf[12], &tok_id, 4U);
    (void)memcpy(&buf[16], &one, 4U);
    (void)memcpy(&buf[20], &one, 4U);
    buf[24] = 0x01U;
    buf[25] = 0x00U;
    (void)memcpy(&buf[26], &svc_type_id, 2U);
    return 28U;
}

static size_t build_single_write_req(uint8_t *buf, uint16_t ns, uint16_t num_id, uint32_t attr_id,
                                     uint32_t idx_range_len, const char *idx_range, uint8_t dv_mask,
                                     const uint8_t *var_bytes, size_t var_len,
                                     const uint8_t *tail_bytes, size_t tail_len)
{
    size_t pos = build_opcua_msg_hdr(buf, 1U, 1U, 673U);
    (void)memset(&buf[pos], 0, 16U); /* RequestHeader */
    pos += 16U;
    uint32_t cnt = 1U;
    (void)memcpy(&buf[pos], &cnt, 4U);
    pos += 4U;
    buf[pos++] = 0x01U;
    buf[pos++] = (uint8_t)ns;
    (void)memcpy(&buf[pos], &num_id, 2U);
    pos += 2U;
    (void)memcpy(&buf[pos], &attr_id, 4U);
    pos += 4U;
    (void)memcpy(&buf[pos], &idx_range_len, 4U);
    pos += 4U;
    if ((idx_range != NULL) && (idx_range_len != 0xFFFFFFFFU) && (idx_range_len > 0U)) {
        (void)memcpy(&buf[pos], idx_range, idx_range_len);
        pos += idx_range_len;
    }
    buf[pos++] = dv_mask;
    if ((var_bytes != NULL) && (var_len > 0U)) {
        (void)memcpy(&buf[pos], var_bytes, var_len);
        pos += var_len;
    }
    if ((tail_bytes != NULL) && (tail_len > 0U)) {
        (void)memcpy(&buf[pos], tail_bytes, tail_len);
        pos += tail_len;
    }
    uint32_t total = (uint32_t)pos;
    (void)memcpy(&buf[4], &total, 4U);
    return pos;
}

static uint32_t extract_write_result_status(const uint8_t *resp, size_t resp_len)
{
    /* MSGF(24) + FourByte TypeId 676(4) + ts(8) + req_id(4) + svc_res(4) + count(4) = offset 48 */
    TEST_ASSERT_GREATER_OR_EQUAL_UINT(52U, resp_len);
    uint32_t st = 0U;
    (void)memcpy(&st, &resp[48], 4U);
    return st;
}

static uint32_t extract_service_fault_status(const uint8_t *resp, size_t resp_len)
{
    /* MSGF(24) + FourByte TypeId 397(4) + ts(8) + req_id(4) = offset 40 */
    TEST_ASSERT_EQUAL_UINT(44U, resp_len);
    uint16_t type_id = 0U;
    (void)memcpy(&type_id, &resp[26], 2U);
    TEST_ASSERT_EQUAL_UINT16(397U, type_id);
    uint32_t st = 0U;
    (void)memcpy(&st, &resp[40], 4U);
    return st;
}

void test_r5_a06_scada_write_read_and_session_guards(void)
{
    uint8_t rx[512], tx[512];
    SYN_OPCUA_Config cfg = {
        .rx_buf = rx, .rx_buf_size = sizeof(rx), .tx_buf = tx, .tx_buf_size = sizeof(tx)};
    SYN_OPCUA_Server srv;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_init(&srv, &cfg));

    uint8_t req[256];
    uint8_t resp[256];
    size_t resp_len = 0U;

    /* 1. Reject MSGF before OPNF, or with wrong channel_id / token_id */
    (void)memset(req, 0, sizeof(req));
    (void)build_opcua_msg_hdr(req, 1U, 1U, 461U);
    uint32_t sz32 = 32U;
    (void)memcpy(&req[4], &sz32, 4U);
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_opcua_server_process_message(&srv, req, 32U, resp, sizeof(resp), &resp_len));

    uint8_t hel_msg[28] = {
        'H',  'E',  'L',  'F',  28,   0,    0,    0,    0,    0,    0, 0, 0x00, 0x04,
        0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 1, 0, 0,    0,
    };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_process_message(&srv, hel_msg, sizeof(hel_msg),
                                                                   resp, sizeof(resp), &resp_len));
    /* Still HELLO_RECEIVED (< SECURE_CHANNEL) -> MSGF rejected */
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_opcua_server_process_message(&srv, req, 32U, resp, sizeof(resp), &resp_len));

    uint8_t opn_msg[32] = {0};
    (void)memcpy(opn_msg, "OPNF", 4U);
    (void)memcpy(&opn_msg[4], &sz32, 4U);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_process_message(&srv, opn_msg, 32U, resp,
                                                                   sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_SECURE_CHANNEL, srv.state);

    /* Wrong channel_id & wrong token_id */
    (void)build_opcua_msg_hdr(req, 99U, 1U, 461U);
    (void)memcpy(&req[4], &sz32, 4U);
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_opcua_server_process_message(&srv, req, 32U, resp, sizeof(resp), &resp_len));
    (void)build_opcua_msg_hdr(req, 1U, 99U, 461U);
    (void)memcpy(&req[4], &sz32, 4U);
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_opcua_server_process_message(&srv, req, 32U, resp, sizeof(resp), &resp_len));

    /* ActivateSession & CloseSession before CreateSession -> ServiceFault BAD_SESSION_ID_INVALID */
    (void)build_opcua_msg_hdr(req, 1U, 1U, 467U);
    (void)memcpy(&req[4], &sz32, 4U);
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_opcua_server_process_message(&srv, req, 32U, resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_SESSION_ID_INVALID,
                            extract_service_fault_status(resp, resp_len));

    (void)build_opcua_msg_hdr(req, 1U, 1U, 473U);
    (void)memcpy(&req[4], &sz32, 4U);
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_opcua_server_process_message(&srv, req, 32U, resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_SESSION_ID_INVALID,
                            extract_service_fault_status(resp, resp_len));

    /* CreateSession -> SESSION_CREATED; Read/Write/Browse still return BAD_SESSION_NOT_ACTIVATED */
    (void)build_opcua_msg_hdr(req, 1U, 1U, 461U);
    (void)memcpy(&req[4], &sz32, 4U);
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_opcua_server_process_message(&srv, req, 32U, resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_SESSION_CREATED, srv.state);

    const uint16_t gated_svcs[3] = {631U, 673U, 527U};
    for (size_t i = 0U; i < 3U; i++) {
        (void)build_opcua_msg_hdr(req, 1U, 1U, gated_svcs[i]);
        (void)memcpy(&req[4], &sz32, 4U);
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_process_message(&srv, req, 32U, resp,
                                                                       sizeof(resp), &resp_len));
        TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_SESSION_NOT_ACTIVATED,
                                extract_service_fault_status(resp, resp_len));
    }

    /* ActivateSession -> SESSION_ACTIVE */
    (void)build_opcua_msg_hdr(req, 1U, 1U, 467U);
    (void)memcpy(&req[4], &sz32, 4U);
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_opcua_server_process_message(&srv, req, 32U, resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_SESSION_ACTIVE, srv.state);

    /* 2. Register writable nodes of every Variant type and round-trip WriteRequest -> Read */
    const SYN_OPCUA_DataType wr_types[] = {
        SYN_OPCUA_TYPE_BOOLEAN, SYN_OPCUA_TYPE_SBYTE, SYN_OPCUA_TYPE_BYTE,   SYN_OPCUA_TYPE_INT16,
        SYN_OPCUA_TYPE_UINT16,  SYN_OPCUA_TYPE_INT32, SYN_OPCUA_TYPE_UINT32, SYN_OPCUA_TYPE_INT64,
        SYN_OPCUA_TYPE_UINT64,  SYN_OPCUA_TYPE_FLOAT, SYN_OPCUA_TYPE_DOUBLE, SYN_OPCUA_TYPE_STRING,
        SYN_OPCUA_TYPE_DATETIME};
    for (size_t i = 0U; i < sizeof(wr_types) / sizeof(wr_types[0]); i++) {
        SYN_OPCUA_Node n = {
            .node_id = {.ns_index = 1U,
                        .id_type = SYN_OPCUA_NODEID_NUMERIC,
                        .id = {.num = 3000U + (uint32_t)i}},
            .node_class = SYN_OPCUA_NODECLASS_VARIABLE,
            .browse_name = "WVar",
            .display_name = "WVar",
            .data_type = wr_types[i],
            .access_level = 0x03U,
            .on_write = (i == 9U) ? on_setpoint_write : NULL,
        };
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_register_node(&srv, &n));
    }
    /* Force fallback to node->data_type when value.value.type == NULL on node 3000 */
    srv.nodes[4].value.value.type = SYN_OPCUA_TYPE_NULL;

    /* BOOLEAN = true & false */
    uint8_t v_bool_t[2] = {(uint8_t)SYN_OPCUA_TYPE_BOOLEAN, 1U};
    uint8_t v_bool_f[2] = {(uint8_t)SYN_OPCUA_TYPE_BOOLEAN, 0U};
    size_t wlen = build_single_write_req(req, 1U, 3000U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_bool_t,
                                         sizeof(v_bool_t), NULL, 0U);
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_GOOD, extract_write_result_status(resp, resp_len));
    SYN_OPCUA_Variant out_v;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_read_variable(&srv, 1U, 3000U, &out_v));
    TEST_ASSERT_TRUE(out_v.val.boolean);
    wlen = build_single_write_req(req, 1U, 3000U, 13U, 0U, NULL, 0x01U, v_bool_f, sizeof(v_bool_f),
                                  NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    (void)syn_opcua_server_read_variable(&srv, 1U, 3000U, &out_v);
    TEST_ASSERT_FALSE(out_v.val.boolean);

    /* SBYTE, BYTE, INT16, UINT16, INT32, UINT32, INT64, UINT64, DATETIME */
    uint8_t v_s8[2] = {(uint8_t)SYN_OPCUA_TYPE_SBYTE, 0xFEU};
    uint8_t v_u8[2] = {(uint8_t)SYN_OPCUA_TYPE_BYTE, 0xABU};
    uint8_t v_i16[3] = {(uint8_t)SYN_OPCUA_TYPE_INT16, 0x34U, 0x12U};
    uint8_t v_u16[3] = {(uint8_t)SYN_OPCUA_TYPE_UINT16, 0xEFU, 0xBEU};
    uint8_t v_i32[5] = {(uint8_t)SYN_OPCUA_TYPE_INT32, 0x78U, 0x56U, 0x34U, 0x12U};
    uint8_t v_u32[5] = {(uint8_t)SYN_OPCUA_TYPE_UINT32, 0xEFU, 0xBEU, 0xADU, 0xDEU};
    uint8_t v_i64[9] = {(uint8_t)SYN_OPCUA_TYPE_INT64, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U};
    uint8_t v_u64[9] = {(uint8_t)SYN_OPCUA_TYPE_UINT64, 9U, 8U, 7U, 6U, 5U, 4U, 3U, 2U};
    uint8_t v_dt[9] = {(uint8_t)SYN_OPCUA_TYPE_DATETIME, 0xAAU, 0U, 0U, 0U, 0U, 0U, 0U, 0U};

    wlen =
        build_single_write_req(req, 1U, 3001U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_s8, 2U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_GOOD, extract_write_result_status(resp, resp_len));

    wlen =
        build_single_write_req(req, 1U, 3002U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_u8, 2U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_GOOD, extract_write_result_status(resp, resp_len));

    wlen =
        build_single_write_req(req, 1U, 3003U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_i16, 3U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_GOOD, extract_write_result_status(resp, resp_len));

    wlen =
        build_single_write_req(req, 1U, 3004U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_u16, 3U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_GOOD, extract_write_result_status(resp, resp_len));

    wlen =
        build_single_write_req(req, 1U, 3005U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_i32, 5U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_GOOD, extract_write_result_status(resp, resp_len));
    (void)syn_opcua_server_read_variable(&srv, 1U, 3005U, &out_v);
    TEST_ASSERT_EQUAL_HEX32(0x12345678U, out_v.val.uint32);

    wlen =
        build_single_write_req(req, 1U, 3006U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_u32, 5U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_GOOD, extract_write_result_status(resp, resp_len));

    wlen =
        build_single_write_req(req, 1U, 3007U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_i64, 9U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_GOOD, extract_write_result_status(resp, resp_len));

    wlen =
        build_single_write_req(req, 1U, 3008U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_u64, 9U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_GOOD, extract_write_result_status(resp, resp_len));

    wlen =
        build_single_write_req(req, 1U, 3012U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_dt, 9U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_GOOD, extract_write_result_status(resp, resp_len));

    /* FLOAT (with on_write callback + status/sourceTimestamp/extra timestamp flags 0x3F) */
    float fval = 42.25f;
    uint8_t v_flt[5];
    v_flt[0] = (uint8_t)SYN_OPCUA_TYPE_FLOAT;
    (void)memcpy(&v_flt[1], &fval, 4U);
    uint8_t dv_tail[24] = {0};
    g_node_write_called = false;
    wlen = build_single_write_req(req, 1U, 3009U, 13U, 0xFFFFFFFFU, NULL, 0x3FU, v_flt, 5U, dv_tail,
                                  24U);
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_GOOD, extract_write_result_status(resp, resp_len));
    TEST_ASSERT_TRUE(g_node_write_called);
    (void)syn_opcua_server_read_variable(&srv, 1U, 3009U, &out_v);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 42.25f, out_v.val.float_val);

    /* on_write failure rolls back value and returns BAD_INTERNAL_ERROR */
    SYN_OPCUA_Node *flt_node = syn_opcua_server_find_node_num(&srv, 1U, 3009U);
    flt_node->on_write = on_setpoint_write_fail;
    float bad_fval = 99.0f;
    (void)memcpy(&v_flt[1], &bad_fval, 4U);
    wlen =
        build_single_write_req(req, 1U, 3009U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_flt, 5U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_INTERNAL_ERROR,
                            extract_write_result_status(resp, resp_len));
    (void)syn_opcua_server_read_variable(&srv, 1U, 3009U, &out_v);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 42.25f, out_v.val.float_val);

    /* DOUBLE */
    double dval = 123.5;
    uint8_t v_dbl[9];
    v_dbl[0] = (uint8_t)SYN_OPCUA_TYPE_DOUBLE;
    (void)memcpy(&v_dbl[1], &dval, 8U);
    wlen =
        build_single_write_req(req, 1U, 3010U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_dbl, 9U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_GOOD, extract_write_result_status(resp, resp_len));
    (void)syn_opcua_server_read_variable(&srv, 1U, 3010U, &out_v);
    TEST_ASSERT_TRUE(out_v.val.double_val > 123.4 && out_v.val.double_val < 123.6);

    /* STRING: non-empty, empty (0), and null (-1), plus ReadRequest round-trip */
    uint8_t v_str[10] = {(uint8_t)SYN_OPCUA_TYPE_STRING, 5U, 0U, 0U, 0U, 'R', 'E', 'A', 'D', 'Y'};
    wlen =
        build_single_write_req(req, 1U, 3011U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_str, 10U, NULL, 0U);
    /* Overwrite stack req after process_message to prove server copied string into node->string_buf
     */
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_GOOD, extract_write_result_status(resp, resp_len));
    (void)memset(req, 0xAA, sizeof(req));
    (void)syn_opcua_server_read_variable(&srv, 1U, 3011U, &out_v);
    TEST_ASSERT_EQUAL_STRING("READY", out_v.val.string);

    uint8_t v_str_empty[5] = {(uint8_t)SYN_OPCUA_TYPE_STRING, 0U, 0U, 0U, 0U};
    wlen = build_single_write_req(req, 1U, 3011U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_str_empty, 5U,
                                  NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    (void)syn_opcua_server_read_variable(&srv, 1U, 3011U, &out_v);
    TEST_ASSERT_EQUAL_STRING("", out_v.val.string);

    uint8_t v_str_null[5] = {(uint8_t)SYN_OPCUA_TYPE_STRING, 0xFFU, 0xFFU, 0xFFU, 0xFFU};
    wlen = build_single_write_req(req, 1U, 3011U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_str_null, 5U,
                                  NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    (void)syn_opcua_server_read_variable(&srv, 1U, 3011U, &out_v);
    TEST_ASSERT_NULL(out_v.val.string);

    /* 3. Error StatusCodes: non-Value attr, IndexRange, Object node, type mismatch, NULL variant */
    wlen = build_single_write_req(req, 1U, 3005U, SYN_OPCUA_ATTR_BROWSE_NAME, 0xFFFFFFFFU, NULL,
                                  0x01U, v_i32, 5U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_NOT_WRITABLE,
                            extract_write_result_status(resp, resp_len));

    wlen = build_single_write_req(req, 1U, 3005U, 13U, 3U, "0:1", 0x01U, v_i32, 5U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_INDEX_RANGE_INVALID,
                            extract_write_result_status(resp, resp_len));

    wlen = build_single_write_req(req, 0U, SYN_OPCUA_NODEID_OBJECTS_FOLDER, 13U, 0xFFFFFFFFU, NULL,
                                  0x01U, v_i32, 5U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_NOT_WRITABLE,
                            extract_write_result_status(resp, resp_len));

    uint8_t v_null[1] = {(uint8_t)SYN_OPCUA_TYPE_NULL};
    wlen =
        build_single_write_req(req, 1U, 3005U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_null, 1U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_TYPE_MISMATCH,
                            extract_write_result_status(resp, resp_len));

    wlen =
        build_single_write_req(req, 1U, 3005U, 13U, 0xFFFFFFFFU, NULL, 0x00U, NULL, 0U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_TYPE_MISMATCH,
                            extract_write_result_status(resp, resp_len));

    /* 4. Decoder error branches abort WriteRequest with ServiceFault(Bad_DecodingError) */
    uint8_t v_arr[2] = {0x86U, 0U};
    wlen =
        build_single_write_req(req, 1U, 3005U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_arr, 2U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_DECODING_ERROR,
                            extract_service_fault_status(resp, resp_len));

    uint8_t v_unk[1] = {55U};
    wlen =
        build_single_write_req(req, 1U, 3005U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_unk, 1U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_DECODING_ERROR,
                            extract_service_fault_status(resp, resp_len));

    /* Multi-item WriteRequest where item 1 is malformed must abort without committing item 0 */
    uint8_t multi_wr[128];
    size_t mpos = build_opcua_msg_hdr(multi_wr, 1U, 1U, 673U);
    (void)memset(&multi_wr[mpos], 0, 16U);
    mpos += 16U;
    uint32_t cnt2 = 2U;
    (void)memcpy(&multi_wr[mpos], &cnt2, 4U);
    mpos += 4U;
    multi_wr[mpos++] = 0x01U;
    multi_wr[mpos++] = 1U;
    uint16_t nid_3005 = 3005U;
    uint32_t attr13 = 13U;
    uint32_t idx_null = 0xFFFFFFFFU;
    int32_t poisoned_i32 = 99999;
    (void)memcpy(&multi_wr[mpos], &nid_3005, 2U);
    mpos += 2U;
    (void)memcpy(&multi_wr[mpos], &attr13, 4U);
    mpos += 4U;
    (void)memcpy(&multi_wr[mpos], &idx_null, 4U);
    mpos += 4U;
    multi_wr[mpos++] = 0x01U;
    multi_wr[mpos++] = (uint8_t)SYN_OPCUA_TYPE_INT32;
    (void)memcpy(&multi_wr[mpos], &poisoned_i32, 4U);
    mpos += 4U;
    /* Item 1 truncated / malformed */
    multi_wr[mpos++] = 0x01U;
    multi_wr[mpos++] = 1U;
    uint32_t mtotal = (uint32_t)mpos;
    (void)memcpy(&multi_wr[4], &mtotal, 4U);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_process_message(&srv, multi_wr, mpos, resp,
                                                                   sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_DECODING_ERROR,
                            extract_service_fault_status(resp, resp_len));
    (void)syn_opcua_server_read_variable(&srv, 1U, 3005U, &out_v);
    TEST_ASSERT_EQUAL_HEX32(0x12345678U, out_v.val.uint32);

    uint8_t v_str_huge[5] = {(uint8_t)SYN_OPCUA_TYPE_STRING, 80U, 0U, 0U, 0U};
    wlen = build_single_write_req(req, 1U, 3011U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_str_huge, 5U,
                                  NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_DECODING_ERROR,
                            extract_service_fault_status(resp, resp_len));

    /* Truncated BOOLEAN, FLOAT, DOUBLE, STRING len, u64 hi, idx_range, dv_mask, status, ts, extra
     */
    const uint8_t trunc_types[5] = {(uint8_t)SYN_OPCUA_TYPE_BOOLEAN, (uint8_t)SYN_OPCUA_TYPE_FLOAT,
                                    (uint8_t)SYN_OPCUA_TYPE_DOUBLE, (uint8_t)SYN_OPCUA_TYPE_STRING,
                                    (uint8_t)SYN_OPCUA_TYPE_UINT64};
    for (size_t i = 0U; i < 5U; i++) {
        uint8_t vt[5] = {trunc_types[i], 1U, 0U, 0U, 0U};
        size_t vt_sz = (i == 4U) ? 5U : 1U;
        wlen = build_single_write_req(req, 1U, 3005U, 13U, 0xFFFFFFFFU, NULL, 0x01U, vt, vt_sz,
                                      NULL, 0U);
        (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
        TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_DECODING_ERROR,
                                extract_service_fault_status(resp, resp_len));
    }

    /* Truncated at attr_id, idx_len, idx skip, dv_mask, status, source_ts, and extra_ts */
    wlen = build_single_write_req(req, 1U, 3005U, 13U, 10U, NULL, 0x01U, v_i32, 5U, NULL, 0U);
    (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_DECODING_ERROR,
                            extract_service_fault_status(resp, resp_len));

    const size_t cut_sizes[4] = {50U, 58U, 60U, 61U};
    (void)build_single_write_req(req, 1U, 3005U, 13U, 0xFFFFFFFFU, NULL, 0x01U, v_i32, 5U, NULL,
                                 0U);
    for (size_t i = 0U; i < 4U; i++) {
        uint32_t csz = (uint32_t)cut_sizes[i];
        (void)memcpy(&req[4], &csz, 4U);
        (void)syn_opcua_server_process_message(&srv, req, cut_sizes[i], resp, sizeof(resp),
                                               &resp_len);
        TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_DECODING_ERROR,
                                extract_service_fault_status(resp, resp_len));
    }

    const uint8_t trunc_masks[3] = {0x02U, 0x04U, 0x10U};
    for (size_t i = 0U; i < 3U; i++) {
        wlen = build_single_write_req(req, 1U, 3005U, 13U, 0xFFFFFFFFU, NULL, trunc_masks[i], NULL,
                                      0U, NULL, 0U);
        (void)syn_opcua_server_process_message(&srv, req, wlen, resp, sizeof(resp), &resp_len);
        TEST_ASSERT_EQUAL_HEX32(SYN_OPCUA_STATUS_BAD_DECODING_ERROR,
                                extract_service_fault_status(resp, resp_len));
    }

    /* Null array count (-1) in WriteRequest */
    uint32_t neg1 = 0xFFFFFFFFU;
    (void)memcpy(&req[44], &neg1, 4U);
    uint32_t sz48 = 48U;
    (void)memcpy(&req[4], &sz48, 4U);
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_opcua_server_process_message(&srv, req, 48U, resp, sizeof(resp), &resp_len));

    /* CLOF CloseSecureChannel transitions state to CLOSED */
    uint8_t clof[8] = {'C', 'L', 'O', 'F', 8, 0, 0, 0};
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_opcua_server_process_message(&srv, clof, 8U, resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_CLOSED, srv.state);
}

static SYN_OPCUA_Server s_pro21_srv;
static uint8_t s_pro21_rx[512];
static uint8_t s_pro21_tx[512];
static uint8_t s_pro21_resp[512];

void test_pro21_opcua_browse_string_nodeid_no_union_pun(void)
{
    SYN_OPCUA_Config cfg = {
        .rx_buf = s_pro21_rx,
        .rx_buf_size = sizeof(s_pro21_rx),
        .tx_buf = s_pro21_tx,
        .tx_buf_size = sizeof(s_pro21_tx),
    };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_init(&s_pro21_srv, &cfg));
    s_pro21_srv.state = SYN_OPCUA_STATE_SESSION_ACTIVE;

    SYN_OPCUA_Node str_node = {
        .node_id = {.ns_index = 2U,
                    .id_type = SYN_OPCUA_NODEID_STRING,
                    .id = {.str = "Pump.Speed"}},
        .node_class = SYN_OPCUA_NODECLASS_VARIABLE,
        .browse_name = "PumpSpeed",
        .display_name = "Pump Speed",
        .data_type = SYN_OPCUA_TYPE_FLOAT,
        .access_level = 0x03U,
    };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_register_node(&s_pro21_srv, &str_node));

    uint8_t br_req[56];
    (void)memset(br_req, 0, sizeof(br_req));
    (void)build_opcua_msg_hdr(br_req, 1U, 1U, 527U);
    uint32_t sz56 = 56U;
    (void)memcpy(&br_req[4], &sz56, 4U);

    size_t resp_len = 0U;
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_opcua_server_process_message(&s_pro21_srv, br_req, sizeof(br_req), s_pro21_resp,
                                                 sizeof(s_pro21_resp), &resp_len));
    TEST_ASSERT_GREATER_THAN_UINT(70U, resp_len);

    /* Non-root reference count at offset 56 must be 4 (Objects, Server, ServerStatus, Pump.Speed)
     */
    uint32_t ref_count = 0U;
    (void)memcpy(&ref_count, &s_pro21_resp[56], 4U);
    TEST_ASSERT_EQUAL_UINT32(4U, ref_count);

    /* Verify wire String NodeId encoding: [0x03, ns=2 (u16 LE), len=10 (u32 LE), "Pump.Speed"] */
    const uint8_t expected_nid[17] = {0x03U, 0x02U, 0x00U, 10U, 0x00U, 0x00U, 0x00U, 'P', 'u',
                                      'm',   'p',   '.',   'S', 'p',   'e',   'e',   'd'};
    bool found_str_nid = false;
    for (size_t i = 0U; (i + sizeof(expected_nid)) <= resp_len; i++) {
        if (memcmp(&s_pro21_resp[i], expected_nid, sizeof(expected_nid)) == 0) {
            found_str_nid = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(found_str_nid);
}

static MockOPCUATransport s_burst_mock;

void test_opcua_server_pt_pipelined_burst_yielding(void)
{
    (void)memset(&s_burst_mock, 0, sizeof(s_burst_mock));
    SYN_Transport transport = {
        .send = mock_opcua_send,
        .recv = mock_opcua_recv,
        .has_data = mock_opcua_has_data,
        .ctx = &s_burst_mock,
    };

    SYN_OPCUA_Config cfg = {
        .transport = NULL,
        .rx_buf = s_pro21_rx,
        .rx_buf_size = sizeof(s_pro21_rx),
        .tx_buf = s_pro21_tx,
        .tx_buf_size = sizeof(s_pro21_tx),
    };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_init(&s_pro21_srv, &cfg));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_opcua_server_step(&s_pro21_srv, 0U));

    s_pro21_srv.cfg.transport = &transport;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_step(&s_pro21_srv, 0U));

    uint8_t hel_msg[28] = {
        'H',  'E',  'L',  'F',  28,   0,    0,    0,    0,    0,    0, 0, 0x00, 0x04,
        0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 1, 0, 0,    0,
    };
    uint8_t opn_msg[32];
    (void)memset(opn_msg, 0, sizeof(opn_msg));
    (void)memcpy(opn_msg, "OPNF", 4U);
    uint32_t opn_sz = 32U;
    (void)memcpy(&opn_msg[4], &opn_sz, 4U);

    uint8_t cs_msg[32];
    (void)memset(cs_msg, 0, sizeof(cs_msg));
    (void)build_opcua_msg_hdr(cs_msg, 1U, 1U, 461U);
    (void)memcpy(&cs_msg[4], &opn_sz, 4U);

    uint8_t act_msg[32];
    (void)memset(act_msg, 0, sizeof(act_msg));
    (void)build_opcua_msg_hdr(act_msg, 1U, 1U, 467U);
    (void)memcpy(&act_msg[4], &opn_sz, 4U);

    (void)memcpy(&s_burst_mock.rx_buf[0], hel_msg, sizeof(hel_msg));
    (void)memcpy(&s_burst_mock.rx_buf[28], opn_msg, sizeof(opn_msg));
    (void)memcpy(&s_burst_mock.rx_buf[60], cs_msg, sizeof(cs_msg));
    (void)memcpy(&s_burst_mock.rx_buf[92], act_msg, sizeof(act_msg));
    s_burst_mock.rx_len = 124U;
    s_burst_mock.has_data_ready = false;

    SYN_Sched sched = {0};
    SYN_Task task = {0};
    task.user_data = &s_pro21_srv;
    task.sched = &sched;
    sched.running = &task;
    sched.share_contended = true;

    SYN_PT pt;
    PT_INIT(&pt);

    /* Contended: processes at most 1 complete OPC UA frame per slice */
    TEST_ASSERT_EQUAL_INT(PT_MORE, syn_opcua_server_pt(&pt, &task));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_HELLO_RECEIVED, s_pro21_srv.state);
    TEST_ASSERT_EQUAL_UINT(96U, s_pro21_srv.rx_pending);

    TEST_ASSERT_EQUAL_INT(PT_MORE, syn_opcua_server_pt(&pt, &task));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_SECURE_CHANNEL, s_pro21_srv.state);
    TEST_ASSERT_EQUAL_UINT(64U, s_pro21_srv.rx_pending);

    TEST_ASSERT_EQUAL_INT(PT_MORE, syn_opcua_server_pt(&pt, &task));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_SESSION_CREATED, s_pro21_srv.state);
    TEST_ASSERT_EQUAL_UINT(32U, s_pro21_srv.rx_pending);

    TEST_ASSERT_EQUAL_INT(PT_IDLE, syn_opcua_server_pt(&pt, &task));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_SESSION_ACTIVE, s_pro21_srv.state);
    TEST_ASSERT_EQUAL_UINT(0U, s_pro21_srv.rx_pending);

    /* Uncontended: drains all 4 pipelined frames in one turn and parks with PT_IDLE */
    s_pro21_srv.state = SYN_OPCUA_STATE_CLOSED;
    (void)memcpy(&s_burst_mock.rx_buf[0], hel_msg, sizeof(hel_msg));
    (void)memcpy(&s_burst_mock.rx_buf[28], opn_msg, sizeof(opn_msg));
    (void)memcpy(&s_burst_mock.rx_buf[60], cs_msg, sizeof(cs_msg));
    (void)memcpy(&s_burst_mock.rx_buf[92], act_msg, sizeof(act_msg));
    s_burst_mock.rx_len = 124U;
    sched.share_contended = false;
    PT_INIT(&pt);

    TEST_ASSERT_EQUAL_INT(PT_IDLE, syn_opcua_server_pt(&pt, &task));
    TEST_ASSERT_EQUAL_INT(SYN_OPCUA_STATE_SESSION_ACTIVE, s_pro21_srv.state);
    TEST_ASSERT_EQUAL_UINT(0U, s_pro21_srv.rx_pending);
}

void run_opcua_tests(void)
{
    RUN_TEST(test_opcua_server_init_and_validation);
    RUN_TEST(test_opcua_server_address_space_and_variables);
    RUN_TEST(test_opcua_server_all_variant_types);
    RUN_TEST(test_opcua_server_uacp_uasc_and_services);
    RUN_TEST(test_opcua_server_protothread);
    RUN_TEST(test_opcua_server_pt_has_data_yielding);
    RUN_TEST(test_opcua_r4_p15_regressions);
    RUN_TEST(test_r5_a06_scada_write_read_and_session_guards);
    RUN_TEST(test_pro21_opcua_browse_string_nodeid_no_union_pun);
    RUN_TEST(test_opcua_server_pt_pipelined_burst_yielding);
}
