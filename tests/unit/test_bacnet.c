#include "syntropic/proto/syn_bacnet.h"
#include "unity/unity.h"

#include <string.h>

static void test_bacnet_crc8(void)
{
    uint8_t header[] = {0x00, 0x01, 0x02, 0x00, 0x00};
    uint8_t crc = syn_bacnet_crc8(header, 5);
    TEST_ASSERT_NOT_EQUAL(0, crc);
}

static void test_bacnet_crc16(void)
{
    uint8_t payload[] = {0x10, 0x08};
    uint16_t crc = syn_bacnet_crc16(payload, 2);
    TEST_ASSERT_NOT_EQUAL(0, crc);
}

static void test_bacnet_mstp_frame_roundtrip(void)
{
    uint8_t raw_buf[64];
    uint8_t payload[] = {0x10, 0x08, 0xC4, 0x00, 0x01, 0x02};

    size_t encoded_len = syn_bacnet_mstp_encode_frame(
        SYN_BACNET_MSTP_FRAME_DATA_NOT_EXPECTING_REPLY, 10, 5, payload, 6, raw_buf);

    TEST_ASSERT_EQUAL_INT(8 + 6 + 2, encoded_len);

    SYN_BACnet_MSTP_Frame rx_frame;
    bool valid = syn_bacnet_mstp_decode_frame(raw_buf, encoded_len, &rx_frame);

    TEST_ASSERT_TRUE(valid);
    TEST_ASSERT_EQUAL_UINT8(SYN_BACNET_MSTP_FRAME_DATA_NOT_EXPECTING_REPLY, rx_frame.frame_type);
    TEST_ASSERT_EQUAL_UINT8(10, rx_frame.destination_mac);
    TEST_ASSERT_EQUAL_UINT8(5, rx_frame.source_mac);
    TEST_ASSERT_EQUAL_UINT16(6, rx_frame.data_len);
    TEST_ASSERT_EQUAL_MEMORY(payload, rx_frame.payload, 6);
}

static void test_bacnet_node_init_and_objects(void)
{
    SYN_BACnet_Node node;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_init(&node, 12, 123456));

    TEST_ASSERT_EQUAL_UINT8(12, node.mac_address);
    TEST_ASSERT_EQUAL_UINT32(123456, node.device_id);
    TEST_ASSERT_EQUAL_INT(1, node.object_count); /* Device object automatically added */

    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_bacnet_add_object(&node, SYN_BACNET_OBJ_ANALOG_INPUT, 1, 23.5f, "Temperature"));
    TEST_ASSERT_EQUAL_INT(2, node.object_count);
}

static void test_bacnet_node_who_is_process(void)
{
    SYN_BACnet_Node node;
    syn_bacnet_node_init(&node, 5, 555);

    /* Construct Who-Is request frame */
    SYN_BACnet_MSTP_Frame req = {.frame_type = SYN_BACNET_MSTP_FRAME_DATA_NOT_EXPECTING_REPLY,
                                 .destination_mac = SYN_BACNET_BROADCAST_MAC,
                                 .source_mac = 1,
                                 .data_len = 2,
                                 .payload = {0x10, SYN_BACNET_SERVICE_UNCONFIRMED_WHO_IS}};

    SYN_BACnet_MSTP_Frame tx_frame;
    bool has_tx = false;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_process(&node, &req, &tx_frame, &has_tx));
    TEST_ASSERT_TRUE(has_tx);
    TEST_ASSERT_EQUAL_UINT8(SYN_BACNET_BROADCAST_MAC, tx_frame.destination_mac);
    TEST_ASSERT_EQUAL_UINT8(5, tx_frame.source_mac);
    TEST_ASSERT_EQUAL_UINT8(SYN_BACNET_SERVICE_UNCONFIRMED_I_AM, tx_frame.payload[1]);
}

static void test_bacnet_poll_for_master(void)
{
    SYN_BACnet_Node node;
    syn_bacnet_node_init(&node, 10, 100);

    SYN_BACnet_MSTP_Frame req = {.frame_type = SYN_BACNET_MSTP_FRAME_POLL_FOR_MASTER,
                                 .destination_mac = 10,
                                 .source_mac = 2,
                                 .data_len = 0};

    SYN_BACnet_MSTP_Frame tx_frame;
    bool has_tx = false;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_process(&node, &req, &tx_frame, &has_tx));
    TEST_ASSERT_TRUE(has_tx);
    TEST_ASSERT_EQUAL_UINT8(SYN_BACNET_MSTP_FRAME_REPLY_TO_POLL_FOR_MASTER, tx_frame.frame_type);
    TEST_ASSERT_EQUAL_UINT8(2, tx_frame.destination_mac);
    TEST_ASSERT_EQUAL_UINT8(10, tx_frame.source_mac);
}

static float decode_be_float(const uint8_t b[4])
{
    uint32_t bits =
        ((uint32_t)b[0] << 24U) | ((uint32_t)b[1] << 16U) | ((uint32_t)b[2] << 8U) | (uint32_t)b[3];
    float v = 0.0f;
    memcpy(&v, &bits, sizeof(v));
    return v;
}

static void encode_be_float(uint8_t b[4], float v)
{
    uint32_t bits = 0U;
    memcpy(&bits, &v, sizeof(bits));
    b[0] = (uint8_t)((bits >> 24U) & 0xFFU);
    b[1] = (uint8_t)((bits >> 16U) & 0xFFU);
    b[2] = (uint8_t)((bits >> 8U) & 0xFFU);
    b[3] = (uint8_t)(bits & 0xFFU);
}

static void test_bacnet_read_property(void)
{
    SYN_BACnet_Node node;
    syn_bacnet_node_init(&node, 7, 700);
    syn_bacnet_add_object(&node, SYN_BACNET_OBJ_ANALOG_INPUT, 1, 42.0f, "TempSensor");

    /* Read Property Request (Confirmed Request 0x00, invoke_id 0x01, Service 12) */
    SYN_BACnet_MSTP_Frame req = {.frame_type = SYN_BACNET_MSTP_FRAME_DATA_EXPECTING_REPLY,
                                 .destination_mac = 7,
                                 .source_mac = 3,
                                 .data_len = 9,
                                 .payload = {0x00, 0x05, 0x01,
                                             SYN_BACNET_SERVICE_CONFIRMED_READ_PROPERTY, 0x0C, 0x00,
                                             0x00, 0x00, 0x01}}; /* Object ID 1 */

    SYN_BACnet_MSTP_Frame tx_frame;
    bool has_tx = false;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_process(&node, &req, &tx_frame, &has_tx));
    TEST_ASSERT_TRUE(has_tx);
    TEST_ASSERT_EQUAL_UINT8(3, tx_frame.destination_mac);
    TEST_ASSERT_EQUAL_UINT8(SYN_BACNET_SERVICE_CONFIRMED_READ_PROPERTY, tx_frame.payload[2]);
    TEST_ASSERT_EQUAL_UINT8(0x42U, tx_frame.payload[4]);
    TEST_ASSERT_EQUAL_UINT8(0x28U, tx_frame.payload[5]);
    TEST_ASSERT_EQUAL_UINT8(0x00U, tx_frame.payload[6]);
    TEST_ASSERT_EQUAL_UINT8(0x00U, tx_frame.payload[7]);

    float val = decode_be_float(&tx_frame.payload[4]);
    TEST_ASSERT_EQUAL_FLOAT(42.0f, val);

    /* Test reading object with instance_id == 0 */
    syn_bacnet_add_object(&node, SYN_BACNET_OBJ_ANALOG_INPUT, 0, 99.0f, "AI_0");
    SYN_BACnet_MSTP_Frame req_inst0 = {.frame_type = SYN_BACNET_MSTP_FRAME_DATA_EXPECTING_REPLY,
                                       .destination_mac = 7,
                                       .source_mac = 3,
                                       .data_len = 9,
                                       .payload = {0x00, 0x05, 0x01,
                                                   SYN_BACNET_SERVICE_CONFIRMED_READ_PROPERTY, 0x0C,
                                                   0x00, 0x00, 0x00, 0x00}};
    has_tx = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_process(&node, &req_inst0, &tx_frame, &has_tx));
    TEST_ASSERT_TRUE(has_tx);
    val = decode_be_float(&tx_frame.payload[4]);
    TEST_ASSERT_EQUAL_FLOAT(99.0f, val);
}

static void test_bacnet_write_property(void)
{
    SYN_BACnet_Node node;
    syn_bacnet_node_init(&node, 7, 700);
    syn_bacnet_add_object(&node, SYN_BACNET_OBJ_ANALOG_OUTPUT, 1, 10.0f, "SetPoint");

    float new_val = 88.5f;
    /* Object ID: Analog Output (type 1 << 22) | instance 1 = 0x00400001 */
    uint8_t payload[16] = {0x00, 0x05, 0x02, SYN_BACNET_SERVICE_CONFIRMED_WRITE_PROPERTY,
                           0x0C, 0x00, 0x40, 0x00,
                           0x01, 0x44, 0,    0,
                           0,    0};
    encode_be_float(&payload[10], new_val);

    SYN_BACnet_MSTP_Frame req = {.frame_type = SYN_BACNET_MSTP_FRAME_DATA_EXPECTING_REPLY,
                                 .destination_mac = 7,
                                 .source_mac = 3,
                                 .data_len = 14,
                                 .payload = {0}};
    memcpy(req.payload, payload, 14);

    SYN_BACnet_MSTP_Frame tx_frame;
    bool has_tx = false;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_process(&node, &req, &tx_frame, &has_tx));
    TEST_ASSERT_TRUE(has_tx);
    TEST_ASSERT_EQUAL_UINT8(3, tx_frame.destination_mac);
    TEST_ASSERT_EQUAL_UINT8(0x20, tx_frame.payload[0]); /* Simple-ACK */
    TEST_ASSERT_EQUAL_UINT8(0x02, tx_frame.payload[1]); /* Invoke ID */
    TEST_ASSERT_EQUAL_UINT8(SYN_BACNET_SERVICE_CONFIRMED_WRITE_PROPERTY, tx_frame.payload[2]);

    TEST_ASSERT_EQUAL_FLOAT(88.5f, node.objects[1].present_value);
}

static void test_bacnet_edge_cases_and_nulls(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_bacnet_node_init(NULL, 1, 1));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_bacnet_add_object(NULL, 0, 0, 0.0f, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_bacnet_node_process(NULL, NULL, NULL, NULL));

    SYN_BACnet_Node node;
    syn_bacnet_node_init(&node, 5, 50);

    /* Mismatched destination MAC -> ignored */
    SYN_BACnet_MSTP_Frame req_mismatch = {.destination_mac = 99};
    SYN_BACnet_MSTP_Frame tx_frame;
    bool has_tx = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_bacnet_node_process(&node, &req_mismatch, &tx_frame, &has_tx));
    TEST_ASSERT_FALSE(has_tx);

    /* Token frame -> ignored */
    SYN_BACnet_MSTP_Frame req_token = {.frame_type = SYN_BACNET_MSTP_FRAME_TOKEN,
                                       .destination_mac = 5};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_process(&node, &req_token, &tx_frame, &has_tx));
    TEST_ASSERT_FALSE(has_tx);
}

static void test_bacnet_add_object_max_capacity(void)
{
    SYN_BACnet_Node node;
    syn_bacnet_node_init(&node, 1, 10);
    for (int i = 0; i < SYN_BACNET_MAX_OBJECTS - 1; i++) {
        TEST_ASSERT_EQUAL_INT(
            SYN_OK, syn_bacnet_add_object(&node, SYN_BACNET_OBJ_ANALOG_INPUT, i, 1.0f, "AI"));
    }
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_bacnet_add_object(&node, SYN_BACNET_OBJ_ANALOG_INPUT, 99, 1.0f, "AI"));
}

static void test_bacnet_mstp_decode_error_cases(void)
{
    SYN_BACnet_MSTP_Frame frame;

    /* 1. NULL out_buf encode */
    TEST_ASSERT_EQUAL(0, syn_bacnet_mstp_encode_frame(0, 1, 2, NULL, 0, NULL));

    /* 2. NULL buf or frame or short len decode */
    TEST_ASSERT_FALSE(syn_bacnet_mstp_decode_frame(NULL, 10, &frame));
    TEST_ASSERT_FALSE(syn_bacnet_mstp_decode_frame((uint8_t *)"123", 3, &frame));

    /* 3. Bad preamble */
    uint8_t bad_preamble[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    TEST_ASSERT_FALSE(syn_bacnet_mstp_decode_frame(bad_preamble, 8, &frame));

    /* 4. Bad header CRC */
    uint8_t bad_hdr_crc[8] = {0x55, 0xFF, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00};
    TEST_ASSERT_FALSE(syn_bacnet_mstp_decode_frame(bad_hdr_crc, 8, &frame));

    /* 5. Oversized data len (> 501) */
    uint8_t over_len[8] = {0x55, 0xFF, 0x00, 0x01, 0x02, 0x02, 0x00, 0x00};
    over_len[7] = syn_bacnet_crc8(&over_len[2], 5);
    TEST_ASSERT_FALSE(syn_bacnet_mstp_decode_frame(over_len, 8, &frame));

    /* 6. Truncated payload len */
    uint8_t trunc_payload[10] = {0x55, 0xFF, 0x00, 0x01, 0x02, 0x00, 0x05, 0x00};
    trunc_payload[7] = syn_bacnet_crc8(&trunc_payload[2], 5);
    TEST_ASSERT_FALSE(syn_bacnet_mstp_decode_frame(trunc_payload, 10, &frame));

    /* 7. Bad data CRC */
    uint8_t bad_data_crc[15] = {0x55, 0xFF, 0x00, 0x01, 0x02, 0x00,
                                0x02, 0x00, 0xAA, 0xBB, 0x00, 0x00};
    bad_data_crc[7] = syn_bacnet_crc8(&bad_data_crc[2], 5);
    TEST_ASSERT_FALSE(syn_bacnet_mstp_decode_frame(bad_data_crc, 12, &frame));
}

static void test_bacnet_mstp_frame_header_checksum_errors(void)
{
    SYN_BACnet_Node node;
    syn_bacnet_node_init(&node, 10, 100);

    SYN_BACnet_MSTP_Frame tx_frame;
    bool has_tx = false;

    /* 1. Poll for master frame addressed to someone else */
    SYN_BACnet_MSTP_Frame req_poll_other = {.frame_type = SYN_BACNET_MSTP_FRAME_POLL_FOR_MASTER,
                                            .destination_mac = 99,
                                            .source_mac = 2,
                                            .data_len = 0};
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_bacnet_node_process(&node, &req_poll_other, &tx_frame, &has_tx));
    TEST_ASSERT_FALSE(has_tx);

    /* 2. Short APDU payload (data_len < 2) */
    SYN_BACnet_MSTP_Frame req_short = {.frame_type = SYN_BACNET_MSTP_FRAME_DATA_NOT_EXPECTING_REPLY,
                                       .destination_mac = 10,
                                       .source_mac = 2,
                                       .data_len = 1,
                                       .payload = {0x10}};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_process(&node, &req_short, &tx_frame, &has_tx));
    TEST_ASSERT_FALSE(has_tx);

    /* 3. ReadProperty request when no objects exist (object_count == 0) */
    SYN_BACnet_Node empty_node;
    memset(&empty_node, 0, sizeof(empty_node));
    empty_node.mac_address = 10;
    SYN_BACnet_MSTP_Frame req_read = {
        .frame_type = SYN_BACNET_MSTP_FRAME_DATA_EXPECTING_REPLY,
        .destination_mac = 10,
        .source_mac = 3,
        .data_len = 4,
        .payload = {0x00, 0x05, 0x01, SYN_BACNET_SERVICE_CONFIRMED_READ_PROPERTY}};
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_bacnet_node_process(&empty_node, &req_read, &tx_frame, &has_tx));
    TEST_ASSERT_FALSE(has_tx);

    /* 4. Unsupported service choice (0xFF) */
    SYN_BACnet_MSTP_Frame req_unsupported = {.frame_type =
                                                 SYN_BACNET_MSTP_FRAME_DATA_EXPECTING_REPLY,
                                             .destination_mac = 10,
                                             .source_mac = 3,
                                             .data_len = 4,
                                             .payload = {0x00, 0x05, 0x01, 0xFF}};
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_bacnet_node_process(&node, &req_unsupported, &tx_frame, &has_tx));
    TEST_ASSERT_FALSE(has_tx);
}

static void test_r5_bacnet_be_real_and_object_type_isolation(void)
{
    SYN_BACnet_Node node;
    syn_bacnet_node_init(&node, 9U, 900U);
    syn_bacnet_add_object(&node, SYN_BACNET_OBJ_ANALOG_INPUT, 1U, 12.5f, "AI_1");
    syn_bacnet_add_object(&node, SYN_BACNET_OBJ_ANALOG_OUTPUT, 1U, 34.5f, "AO_1");
    syn_bacnet_add_object(&node, SYN_BACNET_OBJ_BINARY_INPUT, 1U, 1.0f, "BI_1");

    SYN_BACnet_MSTP_Frame tx_frame;
    bool has_tx = false;

    /* Read AO:1 -> (1 << 22) | 1 = 0x00400001 -> must return 34.5f, not AI:1 (12.5f) */
    SYN_BACnet_MSTP_Frame req_ao = {.frame_type = SYN_BACNET_MSTP_FRAME_DATA_EXPECTING_REPLY,
                                    .destination_mac = 9U,
                                    .source_mac = 2U,
                                    .data_len = 9U,
                                    .payload = {0x00U, 0x05U, 0x03U,
                                                SYN_BACNET_SERVICE_CONFIRMED_READ_PROPERTY, 0x0CU,
                                                0x00U, 0x40U, 0x00U, 0x01U}};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_process(&node, &req_ao, &tx_frame, &has_tx));
    TEST_ASSERT_TRUE(has_tx);
    TEST_ASSERT_EQUAL_FLOAT(34.5f, decode_be_float(&tx_frame.payload[4]));

    /* Read BI:1 -> (3 << 22) | 1 = 0x00C00001 -> must return 1.0f */
    req_ao.payload[6] = 0xC0U;
    has_tx = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_process(&node, &req_ao, &tx_frame, &has_tx));
    TEST_ASSERT_TRUE(has_tx);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, decode_be_float(&tx_frame.payload[4]));

    /* Write BI:1 -> must update BI:1 without clobbering AI:1 or AO:1 */
    SYN_BACnet_MSTP_Frame wr_bi = {.frame_type = SYN_BACNET_MSTP_FRAME_DATA_EXPECTING_REPLY,
                                   .destination_mac = 9U,
                                   .source_mac = 2U,
                                   .data_len = 14U,
                                   .payload = {0x00U, 0x05U, 0x04U,
                                               SYN_BACNET_SERVICE_CONFIRMED_WRITE_PROPERTY, 0x0CU,
                                               0x00U, 0xC0U, 0x00U, 0x01U, 0x44U, 0U, 0U, 0U, 0U}};
    encode_be_float(&wr_bi.payload[10], 0.0f);
    has_tx = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_process(&node, &wr_bi, &tx_frame, &has_tx));
    TEST_ASSERT_TRUE(has_tx);
    TEST_ASSERT_EQUAL_FLOAT(12.5f, node.objects[1].present_value);
    TEST_ASSERT_EQUAL_FLOAT(34.5f, node.objects[2].present_value);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, node.objects[3].present_value);

    /* Write to nonexistent BO:99 & short WriteProperty (< 14 bytes) -> Simple-ACK, no clobber */
    wr_bi.payload[6] = 0x00U;
    wr_bi.payload[8] = 99U;
    wr_bi.data_len = 9U;
    has_tx = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_process(&node, &wr_bi, &tx_frame, &has_tx));
    TEST_ASSERT_TRUE(has_tx);
    TEST_ASSERT_EQUAL_FLOAT(12.5f, node.objects[1].present_value);
}

static void test_pro12_bacnet_iam_4byte_obj_null_payload_and_broadcast_confirmed(void)
{
    /* 1. encode_frame rejects NULL payload when payload_len > 0 */
    uint8_t wire[32];
    TEST_ASSERT_EQUAL_size_t(
        0U, syn_bacnet_mstp_encode_frame(SYN_BACNET_MSTP_FRAME_DATA_NOT_EXPECTING_REPLY, 2U, 1U,
                                         NULL, 4U, wire));
    TEST_ASSERT_EQUAL_size_t(
        8U, syn_bacnet_mstp_encode_frame(SYN_BACNET_MSTP_FRAME_TOKEN, 2U, 1U, NULL, 0U, wire));

    /* 2. I-Am encodes all 4 big-endian bytes of ((8UL << 22) | (instance & 0x3FFFFF)) */
    SYN_BACnet_Node node;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_init(&node, 7U, 0x002ABCDEUL));
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_bacnet_add_object(&node, SYN_BACNET_OBJ_ANALOG_INPUT, 1U, 42.0f, "T"));

    SYN_BACnet_MSTP_Frame who_is = {
        .frame_type = SYN_BACNET_MSTP_FRAME_DATA_NOT_EXPECTING_REPLY,
        .destination_mac = SYN_BACNET_BROADCAST_MAC,
        .source_mac = 1U,
        .data_len = 2U,
        .payload = {0x10U, SYN_BACNET_SERVICE_UNCONFIRMED_WHO_IS},
    };
    SYN_BACnet_MSTP_Frame tx_frame;
    bool has_tx = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_process(&node, &who_is, &tx_frame, &has_tx));
    TEST_ASSERT_TRUE(has_tx);
    TEST_ASSERT_EQUAL_UINT16(9U, tx_frame.data_len);
    TEST_ASSERT_EQUAL_HEX8(0xC4U, tx_frame.payload[2]);
    /* (8 << 22) | 0x002ABCDE = 0x022ABCDE */
    TEST_ASSERT_EQUAL_HEX8(0x02U, tx_frame.payload[3]);
    TEST_ASSERT_EQUAL_HEX8(0x2AU, tx_frame.payload[4]);
    TEST_ASSERT_EQUAL_HEX8(0xBCU, tx_frame.payload[5]);
    TEST_ASSERT_EQUAL_HEX8(0xDEU, tx_frame.payload[6]);
    TEST_ASSERT_EQUAL_HEX8(0x21U, tx_frame.payload[7]);
    TEST_ASSERT_EQUAL_HEX8(0x05U, tx_frame.payload[8]);

    /* 3. Broadcast Confirmed-Request (dest == 0xFF) is ignored without ACK/mutation */
    SYN_BACnet_MSTP_Frame bcast_wr = {
        .frame_type = SYN_BACNET_MSTP_FRAME_DATA_EXPECTING_REPLY,
        .destination_mac = SYN_BACNET_BROADCAST_MAC,
        .source_mac = 2U,
        .data_len = 14U,
        .payload = {0x00U, 0x05U, 0x01U, SYN_BACNET_SERVICE_CONFIRMED_WRITE_PROPERTY, 0x0CU, 0x00U,
                    0x00U, 0x00U, 0x01U, 0x44U, 0x41U, 0x20U, 0x00U, 0x00U},
    };
    has_tx = false;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_process(&node, &bcast_wr, &tx_frame, &has_tx));
    TEST_ASSERT_FALSE(has_tx);
    TEST_ASSERT_EQUAL_FLOAT(42.0f, node.objects[1].present_value);

    /* Unicast short Confirmed-Request (< 4B) is also ignored */
    bcast_wr.destination_mac = 7U;
    bcast_wr.data_len = 3U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bacnet_node_process(&node, &bcast_wr, &tx_frame, &has_tx));
    TEST_ASSERT_FALSE(has_tx);
}

void run_bacnet_tests(void)
{
    RUN_TEST(test_bacnet_crc8);
    RUN_TEST(test_bacnet_crc16);
    RUN_TEST(test_bacnet_mstp_frame_roundtrip);
    RUN_TEST(test_bacnet_node_init_and_objects);
    RUN_TEST(test_bacnet_node_who_is_process);
    RUN_TEST(test_bacnet_poll_for_master);
    RUN_TEST(test_bacnet_read_property);
    RUN_TEST(test_bacnet_write_property);
    RUN_TEST(test_bacnet_edge_cases_and_nulls);
    RUN_TEST(test_bacnet_add_object_max_capacity);
    RUN_TEST(test_bacnet_mstp_decode_error_cases);
    RUN_TEST(test_bacnet_mstp_frame_header_checksum_errors);
    RUN_TEST(test_r5_bacnet_be_real_and_object_type_isolation);
    RUN_TEST(test_pro12_bacnet_iam_4byte_obj_null_payload_and_broadcast_confirmed);
}
