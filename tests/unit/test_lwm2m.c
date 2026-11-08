/**
 * @file test_lwm2m.c
 * @brief Unit tests for OMA LwM2M v1.1/v1.2 Core Client Engine.
 */

#include "syntropic/proto/syn_lwm2m.h"
#include "unity/unity.h"

#include <string.h>

/* ── 1. TLV Codec Tests ─────────────────────────────────────────────────── */

static void test_lwm2m_tlv_integer_codec(void)
{
    uint8_t buf[64];
    SYN_LwM2M_TLV tlv;
    size_t consumed = 0;
    int64_t val = 0;

    /* 1-byte integer (-42) */
    size_t len = syn_lwm2m_tlv_encode_int(1, -42, buf, sizeof(buf));
    TEST_ASSERT_EQUAL(3, len); /* 1 hdr + 1 id + 1 val */
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_EQUAL(len, consumed);
    TEST_ASSERT_EQUAL(1, tlv.id);
    TEST_ASSERT_EQUAL(1, tlv.len);
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_int(&tlv, &val));
    TEST_ASSERT_EQUAL(-42, val);

    /* 2-byte integer (1234 and -1234) */
    len = syn_lwm2m_tlv_encode_int(2, 1234, buf, sizeof(buf));
    TEST_ASSERT_EQUAL(4, len);
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_EQUAL(2, tlv.id);
    TEST_ASSERT_EQUAL(2, tlv.len);
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_int(&tlv, &val));
    TEST_ASSERT_EQUAL(1234, val);

    len = syn_lwm2m_tlv_encode_int(2, -1234, buf, sizeof(buf));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_int(&tlv, &val));
    TEST_ASSERT_EQUAL(-1234, val);

    /* 4-byte integer (-1000000 and 1000000) */
    len = syn_lwm2m_tlv_encode_int(300, -1000000, buf, sizeof(buf));
    TEST_ASSERT_EQUAL(7, len); /* 1 hdr + 2 id + 4 val */
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_EQUAL(300, tlv.id);
    TEST_ASSERT_EQUAL(4, tlv.len);
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_int(&tlv, &val));
    TEST_ASSERT_EQUAL(-1000000, val);

    len = syn_lwm2m_tlv_encode_int(300, 1000000, buf, sizeof(buf));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_int(&tlv, &val));
    TEST_ASSERT_EQUAL(1000000, val);

    /* 8-byte integer (0x123456789ABCDEF0 and negative) */
    int64_t big_val = 0x123456789ABCDEF0LL;
    len = syn_lwm2m_tlv_encode_int(4, big_val, buf, sizeof(buf));
    TEST_ASSERT_EQUAL(11, len); /* 1 hdr + 1 id + 1 len_byte + 8 val */
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_EQUAL(4, tlv.id);
    TEST_ASSERT_EQUAL(8, tlv.len);
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_int(&tlv, &val));
    TEST_ASSERT_EQUAL_HEX64((uint64_t)big_val, (uint64_t)val);

    int64_t neg_big = -9000000000000LL;
    len = syn_lwm2m_tlv_encode_int(4, neg_big, buf, sizeof(buf));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_int(&tlv, &val));
    TEST_ASSERT_EQUAL(neg_big, val);
}

static void test_lwm2m_tlv_float_bool_string_opaque_codec(void)
{
    uint8_t buf[128];
    SYN_LwM2M_TLV tlv;
    size_t consumed = 0;

    /* Float32 */
    double in_float = 23.75;
    double out_float = 0.0;
    size_t len = syn_lwm2m_tlv_encode_float(5700, in_float, buf, sizeof(buf));
    TEST_ASSERT_EQUAL(7, len); /* 1 hdr + 2 id + 4 val */
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_EQUAL(5700, tlv.id);
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_float(&tlv, &out_float));
    TEST_ASSERT_FLOAT_WITHIN(0.001, (float)in_float, (float)out_float);

    /* Float64 decoding test */
    uint8_t d64_buf[11];
    d64_buf[0] = 0xC8; /* Res, 8-bit ID, 1 byte length */
    d64_buf[1] = 10;   /* ID 10 */
    d64_buf[2] = 8;    /* Len 8 */
    double d_val = 123.456789;
    uint64_t d_u64;
    memcpy(&d_u64, &d_val, sizeof(d_u64));
    for (size_t i = 0; i < 8; i++) {
        d64_buf[3 + i] = (uint8_t)((d_u64 >> (56 - i * 8)) & 0xFF);
    }
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(d64_buf, sizeof(d64_buf), &tlv, &consumed));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_float(&tlv, &out_float));
    TEST_ASSERT_FLOAT_WITHIN(0.000001, (float)d_val, (float)out_float);

    /* Boolean (true & false) */
    bool out_bool = false;
    len = syn_lwm2m_tlv_encode_bool(5850, true, buf, sizeof(buf));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_bool(&tlv, &out_bool));
    TEST_ASSERT_TRUE(out_bool);

    len = syn_lwm2m_tlv_encode_bool(5850, false, buf, sizeof(buf));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_bool(&tlv, &out_bool));
    TEST_ASSERT_FALSE(out_bool);

    /* String */
    const char *str = "SyntropicOS";
    char out_str[32];
    len = syn_lwm2m_tlv_encode_string(0, str, buf, sizeof(buf));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_string(&tlv, out_str, sizeof(out_str)));
    TEST_ASSERT_EQUAL_STRING(str, out_str);

    /* Opaque */
    const uint8_t raw[5] = {0xDE, 0xAD, 0xBE, 0xEF, 0x42};
    len = syn_lwm2m_tlv_encode_opaque(1, raw, sizeof(raw), buf, sizeof(buf));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_EQUAL(5, tlv.len);
    TEST_ASSERT_EQUAL_MEMORY(raw, tlv.val, 5);

    /* Generic SYN_LwM2M_Value codec (String, Int, Float, Bool, Opaque, ObjLnk) */
    SYN_LwM2M_Value v_in, v_out;
    v_in.type = SYN_LWM2M_TYPE_OBJLNK;
    v_in.val.objlnk.obj_id = 3303;
    v_in.val.objlnk.inst_id = 0;
    len = syn_lwm2m_tlv_encode_value(2, &v_in, buf, sizeof(buf));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_value(&tlv, SYN_LWM2M_TYPE_OBJLNK, &v_out));
    TEST_ASSERT_EQUAL(3303, v_out.val.objlnk.obj_id);
    TEST_ASSERT_EQUAL(0, v_out.val.objlnk.inst_id);

    v_in.type = SYN_LWM2M_TYPE_OPAQUE;
    v_in.val.opaque.data = raw;
    v_in.val.opaque.len = sizeof(raw);
    len = syn_lwm2m_tlv_encode_value(3, &v_in, buf, sizeof(buf));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_value(&tlv, SYN_LWM2M_TYPE_OPAQUE, &v_out));
    TEST_ASSERT_EQUAL(5, v_out.val.opaque.len);
    TEST_ASSERT_EQUAL_MEMORY(raw, v_out.val.opaque.data, 5);

    /* Int / Time / Float / Bool / String via syn_lwm2m_tlv_encode_value */
    v_in.type = SYN_LWM2M_TYPE_INT;
    v_in.val.integer = 55;
    len = syn_lwm2m_tlv_encode_value(1, &v_in, buf, sizeof(buf));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_value(&tlv, SYN_LWM2M_TYPE_INT, &v_out));
    TEST_ASSERT_EQUAL(55, v_out.val.integer);

    v_in.type = SYN_LWM2M_TYPE_FLOAT;
    v_in.val.floating = 12.5;
    len = syn_lwm2m_tlv_encode_value(2, &v_in, buf, sizeof(buf));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_value(&tlv, SYN_LWM2M_TYPE_FLOAT, &v_out));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 12.5f, (float)v_out.val.floating);

    v_in.type = SYN_LWM2M_TYPE_BOOL;
    v_in.val.boolean = true;
    len = syn_lwm2m_tlv_encode_value(3, &v_in, buf, sizeof(buf));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(buf, len, &tlv, &consumed));
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_value(&tlv, SYN_LWM2M_TYPE_BOOL, &v_out));
    TEST_ASSERT_TRUE(v_out.val.boolean);

    v_in.type = SYN_LWM2M_TYPE_STRING;
    v_in.val.str = "test";
    len = syn_lwm2m_tlv_encode_value(4, &v_in, buf, sizeof(buf));
    TEST_ASSERT_TRUE(len > 0);

    /* Unsupported value type */
    v_in.type = (SYN_LwM2M_ValType)99;
    TEST_ASSERT_EQUAL(0, syn_lwm2m_tlv_encode_value(1, &v_in, buf, sizeof(buf)));
}

static void test_lwm2m_tlv_nested_instance_and_length_types(void)
{
    uint8_t child_buf[64];
    size_t child_len = 0;

    child_len += syn_lwm2m_tlv_encode_string(0, "Syntropic", child_buf + child_len,
                                             sizeof(child_buf) - child_len);
    child_len +=
        syn_lwm2m_tlv_encode_int(9, 95, child_buf + child_len, sizeof(child_buf) - child_len);

    uint8_t inst_buf[128];
    size_t inst_len =
        syn_lwm2m_tlv_encode_instance(0, child_buf, child_len, inst_buf, sizeof(inst_buf));
    TEST_ASSERT_TRUE(inst_len > 0);

    SYN_LwM2M_TLV inst_tlv;
    size_t consumed = 0;
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(inst_buf, inst_len, &inst_tlv, &consumed));
    TEST_ASSERT_EQUAL(SYN_LWM2M_TLV_OBJECT_INSTANCE, inst_tlv.type);
    TEST_ASSERT_EQUAL(0, inst_tlv.id);
    TEST_ASSERT_EQUAL(child_len, inst_tlv.len);

    /* 16-bit length and 24-bit length encoding test */
    uint8_t large_payload[300];
    memset(large_payload, 0xAA, sizeof(large_payload));
    SYN_LwM2M_TLV big_tlv = {
        .type = SYN_LWM2M_TLV_RESOURCE,
        .id = 1,
        .val = large_payload,
        .len = sizeof(large_payload),
    };
    uint8_t out_buf[350];
    size_t enc_len = syn_lwm2m_tlv_encode(&big_tlv, out_buf, sizeof(out_buf));
    TEST_ASSERT_EQUAL(1 + 1 + 2 + 300, enc_len); /* hdr + id + 2byte len + 300 */
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(out_buf, enc_len, &inst_tlv, &consumed));
    TEST_ASSERT_EQUAL(300, inst_tlv.len);
    TEST_ASSERT_EQUAL(1, inst_tlv.id);

    /* 24-bit length header encoding */
    SYN_LwM2M_TLV tlv24 = {
        .type = SYN_LWM2M_TLV_RESOURCE, .id = 500, .val = NULL, .len = 70000, /* > 0xFFFF */
    };
    static uint8_t out24[70100];
    size_t enc24 = syn_lwm2m_tlv_encode(&tlv24, out24, sizeof(out24));
    TEST_ASSERT_EQUAL(1 + 2 + 3, enc24); /* 1 hdr + 2 id + 3 len */

    /* 24-bit length decode test */
    uint8_t len24_buf[6] = {0xDB, 0x01, 0x00, 0x00, 0x01, 0xFF}; /* 24-bit len = 1 byte */
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(len24_buf, sizeof(len24_buf), &inst_tlv, &consumed));
    TEST_ASSERT_EQUAL(1, inst_tlv.len);
    TEST_ASSERT_EQUAL(0xFF, inst_tlv.val[0]);
}

/* ── 2. Link Format & Registration Tests ─────────────────────────────────── */

static void test_lwm2m_link_format_and_registration(void)
{
    SYN_Transport dummy_tr = {0};
    SYN_LwM2M_Client client;
    TEST_ASSERT_TRUE(syn_lwm2m_client_init(&client, "urn:imei:862415039201923", 300, &dummy_tr));

    /* Test null entry skipping in generate_link_format */
    client.object_count = 1;
    client.objects[0] = NULL;
    char empty_link[32];
    TEST_ASSERT_EQUAL(0, syn_lwm2m_generate_link_format(&client, empty_link, sizeof(empty_link)));
    client.object_count = 0;

    SYN_LwM2M_DeviceContext dev_ctx = {0};
    SYN_LwM2M_Object dev_obj = syn_lwm2m_make_device_object(&dev_ctx);
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &dev_obj));

    SYN_LwM2M_FirmwareContext fw_ctx = {0};
    SYN_LwM2M_Object fw_obj = syn_lwm2m_make_firmware_object(&fw_ctx);
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &fw_obj));

    SYN_LwM2M_SensorContext temp_ctx = {0};
    SYN_LwM2M_Object temp_obj = syn_lwm2m_make_temperature_object(&temp_ctx);
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &temp_obj));

    /* Buffer too small for link format */
    TEST_ASSERT_EQUAL(0, syn_lwm2m_generate_link_format(&client, empty_link, 5));

    char link_buf[128];
    size_t link_len = syn_lwm2m_generate_link_format(&client, link_buf, sizeof(link_buf));
    TEST_ASSERT_TRUE(link_len > 0);
    TEST_ASSERT_EQUAL_STRING("</3/0>,</5/0>,</3303/0>", link_buf);

    /* Build Register Request */
    SYN_CoapMsg req;
    SYN_CoapOption req_opts[8];
    size_t req_opt_cnt = 0;
    uint8_t payload_buf[256];
    uint8_t token[4] = {0x11, 0x22, 0x33, 0x44};

    /* Build register with buffer too small */
    TEST_ASSERT_EQUAL(0, syn_lwm2m_build_register_request(&client, 0x1234, token, 4, &req, req_opts,
                                                          8, &req_opt_cnt, payload_buf, 5));

    size_t p_len = syn_lwm2m_build_register_request(&client, 0x1234, token, 4, &req, req_opts, 8,
                                                    &req_opt_cnt, payload_buf, sizeof(payload_buf));
    TEST_ASSERT_TRUE(p_len > 0);
    TEST_ASSERT_EQUAL(COAP_CODE_POST, req.code);
    TEST_ASSERT_EQUAL(6, req_opt_cnt);
    TEST_ASSERT_EQUAL(0x1234, req.msg_id);

    /* Handle Register Response (with and without location options) */
    SYN_CoapMsg resp = {.code = COAP_RESP_CREATED};
    SYN_CoapOption resp_opts[2];
    resp_opts[0].num = COAP_OPT_LOCATION_PATH;
    resp_opts[0].val = (const uint8_t *)"rd";
    resp_opts[0].len = 2;
    resp_opts[1].num = COAP_OPT_LOCATION_PATH;
    resp_opts[1].val = (const uint8_t *)"42";
    resp_opts[1].len = 2;

    TEST_ASSERT_TRUE(syn_lwm2m_handle_register_response(&client, &resp, resp_opts, 2));
    TEST_ASSERT_EQUAL(SYN_LWM2M_STATE_REGISTERED, client.state);
    TEST_ASSERT_EQUAL_STRING("rd/42", client.location_path);

    /* Register response with no location opts -> defaults to rd/0 */
    TEST_ASSERT_TRUE(syn_lwm2m_handle_register_response(&client, &resp, NULL, 0));
    TEST_ASSERT_EQUAL_STRING("rd/0", client.location_path);

    /* Failed registration response */
    SYN_CoapMsg err_resp = {.code = COAP_RESP_BAD_REQ};
    TEST_ASSERT_FALSE(syn_lwm2m_handle_register_response(&client, &err_resp, NULL, 0));

    /* Build Update Request (lifetime-only -> 0 payload bytes, COAP_CODE_POST) */
    (void)memset(payload_buf, 0xA5, sizeof(payload_buf));
    size_t upd_len =
        syn_lwm2m_build_update_request(&client, 0x1235, token, 4, 600, &req, req_opts, 8,
                                       &req_opt_cnt, payload_buf, sizeof(payload_buf));
    TEST_ASSERT_EQUAL_UINT32(0U, upd_len);
    TEST_ASSERT_EQUAL(COAP_CODE_POST, req.code);
    TEST_ASSERT_NULL(req.payload);
    TEST_ASSERT_EQUAL_UINT32(0U, req.payload_len);
    TEST_ASSERT_EQUAL(3, req_opt_cnt);

    /* Update request with leading slash in location_path */
    strcpy(client.location_path, "/rd/99");
    upd_len = syn_lwm2m_build_update_request(&client, 0x1235, token, 4, 0, &req, req_opts, 8,
                                             &req_opt_cnt, payload_buf, sizeof(payload_buf));
    TEST_ASSERT_EQUAL_UINT32(0U, upd_len);
    TEST_ASSERT_EQUAL(COAP_CODE_POST, req.code);
    TEST_ASSERT_NULL(req.payload);
    TEST_ASSERT_EQUAL_UINT32(0U, req.payload_len);

    /* Update request with empty location_path clears req.code to 0 */
    client.location_path[0] = '\0';
    req.code = COAP_CODE_POST;
    upd_len = syn_lwm2m_build_update_request(&client, 0x1235, token, 4, 600, &req, req_opts, 8,
                                             &req_opt_cnt, payload_buf, sizeof(payload_buf));
    TEST_ASSERT_EQUAL_UINT32(0U, upd_len);
    TEST_ASSERT_EQUAL(0, req.code);
    strcpy(client.location_path, "/rd/99");

    /* Build Deregister Request */
    TEST_ASSERT_TRUE(syn_lwm2m_build_deregister_request(&client, 0x1236, token, 4, &req, req_opts,
                                                        8, &req_opt_cnt) > 0);
    TEST_ASSERT_EQUAL(COAP_CODE_DELETE, req.code);
    TEST_ASSERT_EQUAL(2, req_opt_cnt);

    /* Deregister with no leading slash */
    strcpy(client.location_path, "rd/99");
    TEST_ASSERT_TRUE(syn_lwm2m_build_deregister_request(&client, 0x1236, token, 4, &req, req_opts,
                                                        8, &req_opt_cnt) > 0);
}

/* ── 3. Request Dispatcher & Standard Objects Tests ──────────────────────── */

static void test_lwm2m_request_dispatcher_read_write_exec(void)
{
    SYN_Transport dummy_tr = {0};
    SYN_LwM2M_Client client;
    syn_lwm2m_client_init(&client, "test_client", 300, &dummy_tr);

    SYN_LwM2M_DeviceContext dev_ctx = {
        .manufacturer = "Syntropic",
        .model_number = "SynNode-01",
        .serial_number = "SN-9876",
        .firmware_ver = "2.1.0",
        .battery_level = 88,
        .memory_free_kb = 120,
        .error_code = 0,
        .current_time = 1700000000LL,
        .utc_offset = "+02:00",
        .timezone = "Europe/Paris",
    };
    SYN_LwM2M_Object dev_obj = syn_lwm2m_make_device_object(&dev_ctx);
    syn_lwm2m_register_object(&client, &dev_obj);

    SYN_LwM2M_FirmwareContext fw_ctx = {
        .package_uri = "coap://fw.server/bin",
        .state = SYN_LWM2M_FW_STATE_IDLE,
        .result = SYN_LWM2M_FW_RESULT_DEFAULT,
        .pkg_name = "FW-Main",
        .pkg_version = "v1.2.0",
    };
    SYN_LwM2M_Object fw_obj = syn_lwm2m_make_firmware_object(&fw_ctx);
    syn_lwm2m_register_object(&client, &fw_obj);

    SYN_LwM2M_SensorContext temp_ctx = {
        .sensor_value = 24.5,
        .unit = "Cel",
        .min_measured_val = -10.0,
        .max_measured_val = 85.0,
    };
    SYN_LwM2M_Object temp_obj = syn_lwm2m_make_temperature_object(&temp_ctx);
    syn_lwm2m_register_object(&client, &temp_obj);

    /* 1. GET /3/0/0 (Read Device Manufacturer) */
    SYN_CoapMsg req = {.type = COAP_TYPE_CON, .code = COAP_CODE_GET, .msg_id = 1};
    SYN_CoapOption req_opts[4];
    req_opts[0].num = COAP_OPT_URI_PATH;
    req_opts[0].val = (const uint8_t *)"3";
    req_opts[0].len = 1;
    req_opts[1].num = COAP_OPT_URI_PATH;
    req_opts[1].val = (const uint8_t *)"0";
    req_opts[1].len = 1;
    req_opts[2].num = COAP_OPT_URI_PATH;
    req_opts[2].val = (const uint8_t *)"0";
    req_opts[2].len = 1;

    SYN_CoapMsg resp;
    SYN_CoapOption resp_opts[4];
    size_t resp_opt_cnt = 0;
    uint8_t resp_buf[256];

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CONTENT, resp.code);
    TEST_ASSERT_TRUE(resp.payload_len > 0);

    SYN_LwM2M_TLV tlv;
    size_t consumed = 0;
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(resp.payload, resp.payload_len, &tlv, &consumed));
    char man_str[32];
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_string(&tlv, man_str, sizeof(man_str)));
    TEST_ASSERT_EQUAL_STRING("Syntropic", man_str);

    /* 2. GET /3303/0/5700 with Observe Option */
    uint8_t obs_token[2] = {0xAA, 0xBB};
    req.token_len = 2;
    memcpy(req.token, obs_token, 2);
    req_opts[0].val = (const uint8_t *)"3303";
    req_opts[0].len = 4;
    req_opts[1].val = (const uint8_t *)"0";
    req_opts[1].len = 1;
    req_opts[2].val = (const uint8_t *)"5700";
    req_opts[2].len = 4;
    SYN_CoapOption req_opts_obs[4];
    memcpy(req_opts_obs, req_opts, 3 * sizeof(SYN_CoapOption));
    req_opts_obs[3].num = COAP_OPT_OBSERVE;
    static const uint8_t obs_zero = 0;
    req_opts_obs[3].val = &obs_zero;
    req_opts_obs[3].len = 1;

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts_obs, 4, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CONTENT, resp.code);

    /* Check observation was created */
    TEST_ASSERT_TRUE(client.observations[0].active);
    TEST_ASSERT_EQUAL(3303, client.observations[0].obj_id);
    TEST_ASSERT_EQUAL(5700, client.observations[0].res_id);

    /* Build Notification */
    temp_ctx.sensor_value = 26.25;
    size_t notif_len = syn_lwm2m_build_notify(&client, 3303, 0, 5700, 0x5555, &resp, resp_opts, 4,
                                              &resp_opt_cnt, resp_buf, sizeof(resp_buf));
    TEST_ASSERT_TRUE(notif_len > 0);
    TEST_ASSERT_EQUAL(COAP_RESP_CONTENT, resp.code);
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode(resp.payload, resp.payload_len, &tlv, &consumed));
    double val_d = 0.0;
    TEST_ASSERT_TRUE(syn_lwm2m_tlv_decode_float(&tlv, &val_d));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 26.25f, (float)val_d);

    /* Remove observation */
    TEST_ASSERT_TRUE(syn_lwm2m_observe_remove(&client, obs_token, 2));
    TEST_ASSERT_FALSE(client.observations[0].active);

    /* 3. GET /3/0 (Read Whole Instance) with Observe option */
    SYN_CoapOption inst_opts[3];
    inst_opts[0].num = COAP_OPT_URI_PATH;
    inst_opts[0].val = (const uint8_t *)"3";
    inst_opts[0].len = 1;
    inst_opts[1].num = COAP_OPT_URI_PATH;
    inst_opts[1].val = (const uint8_t *)"0";
    inst_opts[1].len = 1;
    inst_opts[2].num = COAP_OPT_OBSERVE;
    inst_opts[2].val = &obs_zero;
    inst_opts[2].len = 1;

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, inst_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CONTENT, resp.code);
    TEST_ASSERT_TRUE(resp.payload_len > 0);

    /* 4. PUT /3/0/13 (Write Current Time) */
    uint8_t write_payload[16];
    size_t w_len = syn_lwm2m_tlv_encode_int(13, 1750000000LL, write_payload, sizeof(write_payload));
    req.code = COAP_CODE_PUT;
    req.payload = write_payload;
    req.payload_len = w_len;
    req_opts[0].val = (const uint8_t *)"3";
    req_opts[0].len = 1;
    req_opts[1].val = (const uint8_t *)"0";
    req_opts[1].len = 1;
    req_opts[2].val = (const uint8_t *)"13";
    req_opts[2].len = 2;

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CHANGED, resp.code);
    TEST_ASSERT_EQUAL(1750000000LL, dev_ctx.current_time);

    /* 5. POST /3/0/4 (Execute Reboot) and /3/0/5 (Execute Factory Reset) */
    req.code = COAP_CODE_POST;
    req.payload = NULL;
    req.payload_len = 0;
    req_opts[2].val = (const uint8_t *)"4";
    req_opts[2].len = 1;

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CHANGED, resp.code);
    TEST_ASSERT_TRUE(dev_ctx.reboot_requested);

    req_opts[2].val = (const uint8_t *)"5";
    req_opts[2].len = 1;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CHANGED, resp.code);
    TEST_ASSERT_TRUE(dev_ctx.factory_reset_requested);

    /* 6. POST /5/0/2 (Execute Firmware Update) */
    req_opts[0].val = (const uint8_t *)"5";
    req_opts[0].len = 1;
    req_opts[1].val = (const uint8_t *)"0";
    req_opts[1].len = 1;
    req_opts[2].val = (const uint8_t *)"2";
    req_opts[2].len = 1;

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CHANGED, resp.code);
    TEST_ASSERT_TRUE(fw_ctx.update_requested);
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_STATE_UPDATING, fw_ctx.state);

    /* 7. DELETE /3/0 */
    req.code = COAP_CODE_DELETE;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 2, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_DELETED, resp.code);
}

static void test_lwm2m_standard_objects_complete_reads_and_writes(void)
{
    SYN_LwM2M_DeviceContext dev_ctx = {
        .manufacturer = NULL, /* default fallback */
        .model_number = NULL,
        .serial_number = NULL,
        .firmware_ver = NULL,
        .battery_level = 90,
        .memory_free_kb = 256,
        .error_code = 3,
        .current_time = 1234567,
        .utc_offset = NULL,
        .timezone = NULL,
    };
    SYN_LwM2M_Object dev_obj = syn_lwm2m_make_device_object(&dev_ctx);
    SYN_LwM2M_Value val;

    /* Read all Device resources (0..15) */
    for (uint16_t r = 0; r <= 15; r++) {
        if (r == 4 || r == 5 || (r >= 6 && r <= 8) || r == 12) {
            continue;
        }
        TEST_ASSERT_EQUAL(SYN_OK, dev_obj.read(0, r, &val, dev_obj.user_ctx));
    }
    TEST_ASSERT_EQUAL(SYN_ERROR, dev_obj.read(0, 99, &val, dev_obj.user_ctx));
    TEST_ASSERT_EQUAL(SYN_ERROR, dev_obj.write(0, 99, &val, dev_obj.user_ctx));
    TEST_ASSERT_EQUAL(SYN_ERROR, dev_obj.exec(0, 99, NULL, 0, dev_obj.user_ctx));

    /* Firmware Update Object (Obj 5) */
    SYN_LwM2M_FirmwareContext fw_ctx = {
        .package_uri = "coaps://update.org/fw.bin",
        .state = SYN_LWM2M_FW_STATE_IDLE,
        .result = SYN_LWM2M_FW_RESULT_DEFAULT,
        .pkg_name = "PkgA",
        .pkg_version = "1.0",
    };
    SYN_LwM2M_Object fw_obj = syn_lwm2m_make_firmware_object(&fw_ctx);
    TEST_ASSERT_EQUAL(SYN_OK, fw_obj.read(0, 1, &val, fw_obj.user_ctx));
    TEST_ASSERT_EQUAL(SYN_OK, fw_obj.read(0, 3, &val, fw_obj.user_ctx));
    TEST_ASSERT_EQUAL(SYN_OK, fw_obj.read(0, 5, &val, fw_obj.user_ctx));
    TEST_ASSERT_EQUAL(SYN_OK, fw_obj.read(0, 6, &val, fw_obj.user_ctx));
    TEST_ASSERT_EQUAL(SYN_OK, fw_obj.read(0, 7, &val, fw_obj.user_ctx));
    TEST_ASSERT_EQUAL(SYN_ERROR, fw_obj.read(0, 99, &val, fw_obj.user_ctx));

    SYN_LwM2M_Value uri_val = {
        .type = SYN_LWM2M_TYPE_STRING,
        .val.str = "coap://new.fw/v2",
    };
    TEST_ASSERT_EQUAL(SYN_OK, fw_obj.write(0, 1, &uri_val, fw_obj.user_ctx));
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_STATE_DOWNLOADING, fw_ctx.state);
    TEST_ASSERT_EQUAL(SYN_ERROR, fw_obj.write(0, 99, &uri_val, fw_obj.user_ctx));
    TEST_ASSERT_EQUAL(SYN_ERROR, fw_obj.exec(0, 99, NULL, 0, fw_obj.user_ctx));

    /* Temperature Sensor Object (Obj 3303) */
    SYN_LwM2M_SensorContext temp_ctx = {
        .sensor_value = 21.0,
        .unit = NULL, /* fallback to Cel */
        .min_measured_val = 0.0,
        .max_measured_val = 50.0,
    };
    SYN_LwM2M_Object temp_obj = syn_lwm2m_make_temperature_object(&temp_ctx);
    TEST_ASSERT_EQUAL(SYN_OK, temp_obj.read(0, 5700, &val, temp_obj.user_ctx));
    TEST_ASSERT_EQUAL(SYN_OK, temp_obj.read(0, 5701, &val, temp_obj.user_ctx));
    TEST_ASSERT_EQUAL_STRING("Cel", val.val.str);
    TEST_ASSERT_EQUAL(SYN_OK, temp_obj.read(0, 5601, &val, temp_obj.user_ctx));
    TEST_ASSERT_EQUAL(SYN_OK, temp_obj.read(0, 5602, &val, temp_obj.user_ctx));
    TEST_ASSERT_EQUAL(SYN_ERROR, temp_obj.read(0, 99, &val, temp_obj.user_ctx));
}

/* ── 4. Boundary & Error Conditions Tests ────────────────────────────────── */

static SYN_Status err_read_cb(uint16_t inst_id, uint16_t res_id, SYN_LwM2M_Value *out_val,
                              void *user_ctx)
{
    (void)inst_id;
    (void)res_id;
    (void)out_val;
    (void)user_ctx;
    return SYN_ERROR;
}

static SYN_Status err_write_cb(uint16_t inst_id, uint16_t res_id, const SYN_LwM2M_Value *in_val,
                               void *user_ctx)
{
    (void)inst_id;
    (void)res_id;
    (void)in_val;
    (void)user_ctx;
    return SYN_ERROR;
}

static SYN_Status err_exec_cb(uint16_t inst_id, uint16_t res_id, const uint8_t *args,
                              size_t args_len, void *user_ctx)
{
    (void)inst_id;
    (void)res_id;
    (void)args;
    (void)args_len;
    (void)user_ctx;
    return SYN_ERROR;
}

static void test_lwm2m_observations_and_dispatcher_coverage(void)
{
    SYN_Transport dummy_tr = {0};
    SYN_LwM2M_Client client;
    syn_lwm2m_client_init(&client, "cov_client", 300, &dummy_tr);

    /* 1. Observation table update & full table */
    uint8_t token[4] = {1, 2, 3, 4};
    TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&client, 3, 0, 0, token, 4, 10, 20));
    /* Add same token again -> updates slot */
    TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&client, 3, 0, 0, token, 4, 30, 40));

    /* Fill observation table completely */
    for (size_t i = 1; i < SYN_LWM2M_MAX_OBSERVERS; i++) {
        uint8_t tok[1] = {(uint8_t)(i + 10)};
        TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&client, 3, 0, 0, tok, 1, 0, 0));
    }
    /* Table is full -> returns false */
    uint8_t overflow_tok[1] = {0xFF};
    TEST_ASSERT_FALSE(syn_lwm2m_observe_add(&client, 3, 0, 0, overflow_tok, 1, 0, 0));

    /* Remove non-matching token -> returns false */
    uint8_t non_exist[2] = {0xDE, 0xAD};
    TEST_ASSERT_FALSE(syn_lwm2m_observe_remove(&client, non_exist, 2));

    /* 2. Custom multi-instance Object with explicit instances array */
    static const uint16_t inst_list[2] = {10, 20};
    static const SYN_LwM2M_ResourceDesc res_list[] = {
        {0, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_INT},
        {1, SYN_LWM2M_OP_W, SYN_LWM2M_TYPE_OPAQUE},
        {2, SYN_LWM2M_OP_E, SYN_LWM2M_TYPE_NONE},
    };
    SYN_LwM2M_Object custom_obj = {
        .id = 100,
        .instance_count = 2,
        .instances = inst_list,
        .resource_count = 3,
        .resources = res_list,
        .read = err_read_cb,
        .write = err_write_cb,
        .exec = err_exec_cb,
        .user_ctx = NULL,
    };
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &custom_obj));

    /* Build notify for non-matching observation */
    SYN_CoapMsg resp;
    SYN_CoapOption resp_opts[4];
    size_t resp_opt_cnt = 0;
    uint8_t resp_buf[64];
    TEST_ASSERT_EQUAL(0, syn_lwm2m_build_notify(&client, 9999, 0, 0, 1, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));

    /* Build notify when read callback fails */
    syn_lwm2m_observe_remove(&client, token, 4);
    TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&client, 100, 10, 0, token, 4, 0, 0));
    TEST_ASSERT_EQUAL(0, syn_lwm2m_build_notify(&client, 100, 10, 0, 1, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));

    /* Dispatcher: GET /100/10/0 when read callback returns error -> 5.00 Internal Server Error */
    SYN_CoapMsg req = {.type = COAP_TYPE_CON, .code = COAP_CODE_GET, .msg_id = 1};
    SYN_CoapOption req_opts[3];
    req_opts[0].num = COAP_OPT_URI_PATH;
    req_opts[0].val = (const uint8_t *)"100";
    req_opts[0].len = 3;
    req_opts[1].num = COAP_OPT_URI_PATH;
    req_opts[1].val = (const uint8_t *)"10";
    req_opts[1].len = 2;
    req_opts[2].num = COAP_OPT_URI_PATH;
    req_opts[2].val = (const uint8_t *)"0";
    req_opts[2].len = 1;

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_INTERNAL, resp.code);

    /* Dispatcher: PUT /100/10/1 with text opaque payload when write cb fails -> 5.00 Internal
     * Server Error */
    req.code = COAP_CODE_PUT;
    req_opts[2].val = (const uint8_t *)"1";
    req.payload = (const uint8_t *)"rawdata";
    req.payload_len = 7;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_INTERNAL, resp.code);

    /* Dispatcher: POST /100/10/2 when exec cb fails -> 5.00 Internal Server Error */
    req.code = COAP_CODE_POST;
    req_opts[2].val = (const uint8_t *)"2";
    req.payload = NULL;
    req.payload_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_INTERNAL, resp.code);

    /* Instance validation for explicit instance array */
    req_opts[1].val = (const uint8_t *)"15"; /* Not in {10, 20} */
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_NOT_FOUND, resp.code);

    /* Whole instance GET /100/10 when read returns error */
    req.code = COAP_CODE_GET;
    req_opts[1].val = (const uint8_t *)"10";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 2, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CONTENT, resp.code);

    /* Whole instance GET on object with read == NULL -> 4.04 */
    SYN_LwM2M_Object no_read_obj = {
        .id = 200,
        .instance_count = 1,
        .instances = NULL,
        .resource_count = 0,
        .resources = NULL,
        .read = NULL,
    };
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &no_read_obj));

    /* Whole instance GET on object with read == NULL -> 4.04 */
    req_opts[0].val = (const uint8_t *)"200";
    req_opts[1].val = (const uint8_t *)"0";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 2, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_NOT_FOUND, resp.code);

    /* Single resource GET on object with resources == NULL -> 4.04 */
    req_opts[2].val = (const uint8_t *)"0";
    req_opts[2].len = 1;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_NOT_FOUND, resp.code);
}

static void test_lwm2m_boundary_and_error_handling(void)
{
    /* Null checks */
    TEST_ASSERT_EQUAL(0, syn_lwm2m_tlv_encode(NULL, NULL, 0));
    TEST_ASSERT_EQUAL(0, syn_lwm2m_tlv_encode_string(1, NULL, NULL, 0));
    TEST_ASSERT_EQUAL(0, syn_lwm2m_tlv_encode_value(1, NULL, NULL, 0));
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode(NULL, 0, NULL, NULL));
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode_int(NULL, NULL));
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode_float(NULL, NULL));
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode_bool(NULL, NULL));
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode_string(NULL, NULL, 0));
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode_value(NULL, SYN_LWM2M_TYPE_INT, NULL));
    TEST_ASSERT_EQUAL(0, syn_lwm2m_generate_link_format(NULL, NULL, 0));
    TEST_ASSERT_FALSE(syn_lwm2m_client_init(NULL, NULL, 0, NULL));
    TEST_ASSERT_FALSE(syn_lwm2m_register_object(NULL, NULL));
    TEST_ASSERT_EQUAL(
        0, syn_lwm2m_build_register_request(NULL, 0, NULL, 0, NULL, NULL, 0, NULL, NULL, 0));
    TEST_ASSERT_EQUAL(
        0, syn_lwm2m_build_update_request(NULL, 0, NULL, 0, 0, NULL, NULL, 0, NULL, NULL, 0));
    TEST_ASSERT_EQUAL(0, syn_lwm2m_build_deregister_request(NULL, 0, NULL, 0, NULL, NULL, 0, NULL));
    TEST_ASSERT_FALSE(syn_lwm2m_handle_register_response(NULL, NULL, NULL, 0));
    TEST_ASSERT_FALSE(syn_lwm2m_observe_add(NULL, 0, 0, 0, NULL, 0, 0, 0));
    TEST_ASSERT_FALSE(syn_lwm2m_observe_remove(NULL, NULL, 0));
    TEST_ASSERT_EQUAL(0, syn_lwm2m_build_notify(NULL, 0, 0, 0, 0, NULL, NULL, 0, NULL, NULL, 0));
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_lwm2m_process_request(NULL, NULL, NULL, 0, NULL, NULL, 0, NULL, NULL, 0));

    /* Buffer overflow checks */
    uint8_t tiny_buf[2];
    TEST_ASSERT_EQUAL(0, syn_lwm2m_tlv_encode_int(10, 1000, tiny_buf, sizeof(tiny_buf)));

    /* Malformed TLV decode */
    uint8_t malformed_tlv[3] = {0x08, 0x01, 0x10}; /* Declares 16 bytes, but only 3 total */
    SYN_LwM2M_TLV tlv;
    size_t consumed = 0;
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode(malformed_tlv, sizeof(malformed_tlv), &tlv, &consumed));

    /* Truncated header checks for decode */
    uint8_t tr_id16[2] = {0x20, 0x01};
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode(tr_id16, sizeof(tr_id16), &tlv, &consumed));
    uint8_t tr_len1[2] = {0x08, 0x01};
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode(tr_len1, sizeof(tr_len1), &tlv, &consumed));
    uint8_t tr_len2[3] = {0x10, 0x01, 0x00};
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode(tr_len2, sizeof(tr_len2), &tlv, &consumed));
    uint8_t tr_len3[4] = {0x18, 0x01, 0x00, 0x00};
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode(tr_len3, sizeof(tr_len3), &tlv, &consumed));

    /* Float / Bool / String decode edge cases */
    uint8_t bad_f_val[2] = {0x00, 0x00};
    SYN_LwM2M_TLV bad_f_tlv = {.type = SYN_LWM2M_TLV_RESOURCE, .id = 1, .val = bad_f_val, .len = 2};
    double out_d;
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode_float(&bad_f_tlv, &out_d));

    char small_str[3];
    uint8_t str_val[10] = "longstring";
    SYN_LwM2M_TLV str_tlv = {.type = SYN_LWM2M_TLV_RESOURCE, .id = 1, .val = str_val, .len = 10};
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode_string(&str_tlv, small_str, sizeof(small_str)));

    SYN_LwM2M_Value v_dummy;
    SYN_LwM2M_TLV bad_objlnk = {
        .type = SYN_LWM2M_TLV_RESOURCE, .id = 1, .val = bad_f_val, .len = 2};
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode_value(&bad_objlnk, SYN_LWM2M_TYPE_OBJLNK, &v_dummy));
    TEST_ASSERT_FALSE(syn_lwm2m_tlv_decode_value(&bad_objlnk, (SYN_LwM2M_ValType)99, &v_dummy));

    /* Max object registry limit */
    SYN_Transport dummy_tr = {0};
    SYN_LwM2M_Client client;
    syn_lwm2m_client_init(&client, "test", 300, &dummy_tr);
    SYN_LwM2M_DeviceContext dev_ctx = {0};
    SYN_LwM2M_Object obj = syn_lwm2m_make_device_object(&dev_ctx);
    for (size_t i = 0; i < SYN_LWM2M_MAX_OBJECTS; i++) {
        TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &obj));
    }
    TEST_ASSERT_FALSE(syn_lwm2m_register_object(&client, &obj)); /* Registry full */

    /* Request 4.04 Not Found & 4.00 Bad Request checks */
    SYN_CoapMsg req = {.type = COAP_TYPE_CON, .code = COAP_CODE_GET, .msg_id = 1};
    SYN_CoapOption req_opts[3];
    req_opts[0].num = COAP_OPT_URI_PATH;
    req_opts[0].val = (const uint8_t *)"999"; /* unknown object */
    req_opts[0].len = 3;
    SYN_CoapMsg resp;
    SYN_CoapOption resp_opts[4];
    size_t resp_opt_cnt = 0;
    uint8_t resp_buf[64];

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 1, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_NOT_FOUND, resp.code);

    /* 0 path segments -> 4.00 Bad Request */
    TEST_ASSERT_EQUAL(SYN_OK, syn_lwm2m_process_request(&client, &req, NULL, 0, &resp, resp_opts, 4,
                                                        &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_BAD_REQ, resp.code);

    /* Text fallback on PUT /3/0/13 */
    req_opts[0].num = COAP_OPT_URI_PATH;
    req_opts[0].val = (const uint8_t *)"3";
    req_opts[0].len = 1;
    req_opts[1].num = COAP_OPT_URI_PATH;
    req_opts[1].val = (const uint8_t *)"0";
    req_opts[1].len = 1;
    req_opts[2].num = COAP_OPT_URI_PATH;
    req_opts[2].val = (const uint8_t *)"13";
    req_opts[2].len = 2;
    req.code = COAP_CODE_PUT;
    const char *text_time = "1800000000";
    req.payload = (const uint8_t *)text_time;
    req.payload_len = strlen(text_time);

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CHANGED, resp.code);
    TEST_ASSERT_EQUAL(1800000000LL, dev_ctx.current_time);

    /* Text fallback on PUT string /5/0/1 Package URI */
    req_opts[0].val = (const uint8_t *)"5";
    req_opts[0].len = 1;
    req_opts[2].val = (const uint8_t *)"1";
    req_opts[2].len = 1;
    const char *new_uri = "coap://test.io/app.bin";
    req.payload = (const uint8_t *)new_uri;
    req.payload_len = strlen(new_uri);
    SYN_LwM2M_FirmwareContext fw_ctx = {0};
    SYN_LwM2M_Object fw_obj = syn_lwm2m_make_firmware_object(&fw_ctx);
    client.objects[1] = &fw_obj;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CHANGED, resp.code);
    TEST_ASSERT_EQUAL_STRING(new_uri, fw_ctx.package_uri);

    /* Write to read-only resource (e.g. /3/0/0 Manufacturer) -> 4.05 Method Not Allowed */
    req_opts[0].val = (const uint8_t *)"3";
    req_opts[0].len = 1;
    req_opts[2].val = (const uint8_t *)"0";
    req_opts[2].len = 1;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_METHOD_NA, resp.code);

    /* Read from execute-only resource (e.g. /3/0/4 Reboot) -> 4.05 Method Not Allowed */
    req.code = COAP_CODE_GET;
    req_opts[2].val = (const uint8_t *)"4";
    req_opts[2].len = 1;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_METHOD_NA, resp.code);

    /* Invalid instance ID */
    req_opts[1].val = (const uint8_t *)"9";
    req_opts[1].len = 1;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_NOT_FOUND, resp.code);

    /* Unsupported method */
    req.code = 15;
    req_opts[1].val = (const uint8_t *)"0";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 2, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_METHOD_NA, resp.code);

    /* Single resource GET for unknown resource -> 4.04 Not Found */
    req.code = COAP_CODE_GET;
    req_opts[0].val = (const uint8_t *)"3";
    req_opts[1].val = (const uint8_t *)"0";
    req_opts[2].val = (const uint8_t *)"99";
    req_opts[2].len = 2;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_NOT_FOUND, resp.code);

    /* Single resource GET with response buffer too small for TLV payload -> 5.00 Internal */
    req_opts[2].val = (const uint8_t *)"0";
    req_opts[2].len = 1;
    TEST_ASSERT_EQUAL(SYN_OK, syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp,
                                                        resp_opts, 4, &resp_opt_cnt, resp_buf, 1));
    TEST_ASSERT_EQUAL(COAP_RESP_INTERNAL, resp.code);

    /* 1-path-segment GET -> 4.00 Bad Request */
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 1, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_BAD_REQ, resp.code);

    /* Build notify with res_id < 0 -> returns 0 */
    uint8_t tok[1] = {9};
    syn_lwm2m_observe_add(&client, 3, 0, -1, tok, 1, 0, 0);
    TEST_ASSERT_EQUAL(0, syn_lwm2m_build_notify(&client, 3, 0, -1, 1, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));

    /* Build notify with small buffer -> returns 0 */
    syn_lwm2m_observe_add(&client, 3, 0, 0, tok, 1, 0, 0);
    TEST_ASSERT_EQUAL(0, syn_lwm2m_build_notify(&client, 3, 0, 0, 1, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, 1));

    /* PUT with unknown resource_id -> 4.04 Not Found */
    req.code = COAP_CODE_PUT;
    req_opts[0].val = (const uint8_t *)"3";
    req_opts[1].val = (const uint8_t *)"0";
    req_opts[2].val = (const uint8_t *)"99";
    req_opts[2].len = 2;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_NOT_FOUND, resp.code);

    /* PUT with invalid / empty payload on writable resource -> 4.00 Bad Request */
    req_opts[2].val = (const uint8_t *)"13";
    req_opts[2].len = 2;
    req.payload = NULL;
    req.payload_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_BAD_REQ, resp.code);

    /* POST on execute resource with exec == NULL -> 4.05 Method Not Allowed */
    static const SYN_LwM2M_ResourceDesc no_exec_res = {0, SYN_LWM2M_OP_E, SYN_LWM2M_TYPE_NONE};
    SYN_LwM2M_Object no_exec_obj = {
        .id = 300,
        .instance_count = 1,
        .instances = NULL,
        .resource_count = 1,
        .resources = &no_exec_res,
        .exec = NULL,
    };
    client.object_count = 1;
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &no_exec_obj));
    req.code = COAP_CODE_POST;
    req_opts[0].val = (const uint8_t *)"300";
    req_opts[0].len = 3;
    req_opts[1].val = (const uint8_t *)"0";
    req_opts[1].len = 1;
    req_opts[2].val = (const uint8_t *)"0";
    req_opts[2].len = 1;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, req_opts, 3, &resp, resp_opts, 4,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_METHOD_NA, resp.code);

    /* Standard object callback NULL user_ctx and NULL out_val checks */
    SYN_LwM2M_Object dev_null = syn_lwm2m_make_device_object(NULL);
    SYN_LwM2M_Value val_dummy;
    TEST_ASSERT_EQUAL(SYN_ERROR, dev_null.read(0, 0, &val_dummy, NULL));
    TEST_ASSERT_EQUAL(SYN_ERROR, dev_null.read(0, 0, NULL, &dev_ctx));
    TEST_ASSERT_EQUAL(SYN_ERROR, dev_null.write(0, 13, &val_dummy, NULL));
    TEST_ASSERT_EQUAL(SYN_ERROR, dev_null.write(0, 13, NULL, &dev_ctx));
    TEST_ASSERT_EQUAL(SYN_ERROR, dev_null.exec(0, 4, NULL, 0, NULL));

    SYN_LwM2M_Object fw_null = syn_lwm2m_make_firmware_object(NULL);
    TEST_ASSERT_EQUAL(SYN_ERROR, fw_null.read(0, 1, &val_dummy, NULL));
    TEST_ASSERT_EQUAL(SYN_ERROR, fw_null.read(0, 1, NULL, &fw_ctx));
    TEST_ASSERT_EQUAL(SYN_ERROR, fw_null.write(0, 1, &val_dummy, NULL));
    TEST_ASSERT_EQUAL(SYN_ERROR, fw_null.write(0, 1, NULL, &fw_ctx));
    TEST_ASSERT_EQUAL(SYN_ERROR, fw_null.exec(0, 2, NULL, 0, NULL));

    SYN_LwM2M_Object temp_null = syn_lwm2m_make_temperature_object(NULL);
    SYN_LwM2M_SensorContext temp_ctx = {0};
    TEST_ASSERT_EQUAL(SYN_ERROR, temp_null.read(0, 5700, &val_dummy, NULL));
    TEST_ASSERT_EQUAL(SYN_ERROR, temp_null.read(0, 5700, NULL, &temp_ctx));
}

typedef struct {
    double float_val;
    bool bool_val;
} R4CustomWriteCtx;

static SYN_Status r4_custom_write_cb(uint16_t inst_id, uint16_t res_id,
                                     const SYN_LwM2M_Value *in_val, void *user_ctx)
{
    (void)inst_id;
    R4CustomWriteCtx *ctx = (R4CustomWriteCtx *)user_ctx;
    if (res_id == 0U && in_val->type == SYN_LWM2M_TYPE_FLOAT) {
        ctx->float_val = in_val->val.floating;
        return SYN_OK;
    }
    if (res_id == 1U && in_val->type == SYN_LWM2M_TYPE_BOOL) {
        ctx->bool_val = in_val->val.boolean;
        return SYN_OK;
    }
    return SYN_ERROR;
}

static void test_lwm2m_r4_p01_p05_regressions(void)
{
    SYN_Transport dummy_tr = {0};
    SYN_LwM2M_Client client;
    TEST_ASSERT_TRUE(syn_lwm2m_client_init(&client, "r4-client", 300U, &dummy_tr));

    SYN_LwM2M_DeviceContext dev_ctx = {0};
    SYN_LwM2M_Object dev_obj = syn_lwm2m_make_device_object(&dev_ctx);
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &dev_obj));

    SYN_LwM2M_FirmwareContext fw_ctx = {0};
    SYN_LwM2M_Object fw_obj = syn_lwm2m_make_firmware_object(&fw_ctx);
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &fw_obj));

    SYN_LwM2M_SensorContext temp_ctx = {.sensor_value = 22.0, .unit = "Cel"};
    SYN_LwM2M_Object temp_obj = syn_lwm2m_make_temperature_object(&temp_ctx);
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &temp_obj));

    static const SYN_LwM2M_ResourceDesc r4_res[] = {
        {0U, SYN_LWM2M_OP_W, SYN_LWM2M_TYPE_FLOAT},
        {1U, SYN_LWM2M_OP_W, SYN_LWM2M_TYPE_BOOL},
    };
    R4CustomWriteCtx custom_ctx = {0};
    SYN_LwM2M_Object custom_obj = {
        .id = 400U,
        .instance_count = 1U,
        .instances = NULL,
        .resource_count = 2U,
        .resources = r4_res,
        .read = NULL,
        .write = r4_custom_write_cb,
        .exec = NULL,
        .user_ctx = &custom_ctx,
    };
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &custom_obj));

    SYN_CoapMsg req = {.type = COAP_TYPE_CON, .code = COAP_CODE_PUT, .msg_id = 0x42};
    SYN_CoapMsg resp = {0};
    SYN_CoapOption resp_opts[4];
    size_t resp_opt_cnt = 0U;
    uint8_t resp_buf[256];

    /* max_resp_opts < 2U guard */
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_lwm2m_process_request(&client, &req, NULL, 0U, &resp, resp_opts, 1U,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));

    /* 1. R4-P01: 100-byte URI write via TLV (Content-Format 11542) and plain-text */
    char uri_100[101];
    (void)memset(uri_100, 'a', 100U);
    (void)memcpy(uri_100, "coaps://ota.syntropic.local/", 28U);
    uri_100[100] = '\0';

    uint8_t tlv_uri[128];
    size_t tlv_uri_len = syn_lwm2m_tlv_encode_string(1U, uri_100, tlv_uri, sizeof(tlv_uri));
    TEST_ASSERT_TRUE(tlv_uri_len > 100U);

    uint8_t cf_tlv[2] = {(uint8_t)(SYN_LWM2M_FORMAT_TLV >> 8),
                         (uint8_t)(SYN_LWM2M_FORMAT_TLV & 0xFF)};
    SYN_CoapOption fw_opts[4] = {
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"5", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"1", .len = 1U},
        {.num = COAP_OPT_CONTENT_FORMAT, .val = cf_tlv, .len = 2U},
    };
    req.payload = tlv_uri;
    req.payload_len = tlv_uri_len;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, fw_opts, 4U, &resp, resp_opts, 4U,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CHANGED, resp.code);
    TEST_ASSERT_EQUAL_STRING(uri_100, fw_ctx.package_uri);

    /* Reject plain-text fallback when Content-Format == 11542 (TLV) */
    const char *plain_uri = "coap://plain.example/fw.bin";
    req.payload = (const uint8_t *)plain_uri;
    req.payload_len = strlen(plain_uri);
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, fw_opts, 4U, &resp, resp_opts, 4U,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_BAD_REQ, resp.code);

    /* Oversized TLV string (>= 128B) with Content-Format == 11542 -> 4.00 Bad Request */
    char uri_140[141];
    (void)memset(uri_140, 'x', 140U);
    uri_140[140] = '\0';
    uint8_t tlv_big[160];
    size_t tlv_big_len = syn_lwm2m_tlv_encode_string(1U, uri_140, tlv_big, sizeof(tlv_big));
    req.payload = tlv_big;
    req.payload_len = tlv_big_len;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, fw_opts, 4U, &resp, resp_opts, 4U,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_BAD_REQ, resp.code);

    /* 100-byte plain-text URI write with Content-Format = 0 (0-byte empty uint option) */
    (void)memset(&fw_ctx, 0, sizeof(fw_ctx));
    fw_opts[3].val = NULL;
    fw_opts[3].len = 0U;
    req.payload = (const uint8_t *)uri_100;
    req.payload_len = 100U;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, fw_opts, 4U, &resp, resp_opts, 4U,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CHANGED, resp.code);
    TEST_ASSERT_EQUAL_STRING(uri_100, fw_ctx.package_uri);

    /* 2. R4-P01: Non-NUL-terminated integer payload buffer + invalid text integer/float */
    uint8_t raw_digits[4] = {'9', '8', '7', '6'};
    SYN_CoapOption time_opts[3] = {
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"3", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"13", .len = 2U},
    };
    req.payload = raw_digits;
    req.payload_len = sizeof(raw_digits);
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, time_opts, 3U, &resp, resp_opts, 4U,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CHANGED, resp.code);
    TEST_ASSERT_EQUAL(9876LL, dev_ctx.current_time);

    const char *bad_int = "not_an_int";
    req.payload = (const uint8_t *)bad_int;
    req.payload_len = strlen(bad_int);
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, time_opts, 3U, &resp, resp_opts, 4U,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_BAD_REQ, resp.code);

    /* Plain-text float write (valid, malformed TLV fallback, invalid text, and unsupported bool) */
    SYN_CoapOption flt_opts[3] = {
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"400", .len = 3U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
    };
    uint8_t raw_float[5] = {'3', '6', '.', '2', '5'};
    req.payload = raw_float;
    req.payload_len = sizeof(raw_float);
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, flt_opts, 3U, &resp, resp_opts, 4U,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CHANGED, resp.code);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 36.25f, (float)custom_ctx.float_val);

    /* Valid TLV header for res 0 with 3-byte value (invalid float len) + Content-Format TLV */
    uint8_t bad_tlv_flt[5] = {0xC3U, 0x00U, 'x', 'y', 'z'};
    SYN_CoapOption flt_tlv_opts[4] = {
        flt_opts[0],
        flt_opts[1],
        flt_opts[2],
        {.num = COAP_OPT_CONTENT_FORMAT, .val = cf_tlv, .len = 2U},
    };
    req.payload = bad_tlv_flt;
    req.payload_len = sizeof(bad_tlv_flt);
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, flt_tlv_opts, 4U, &resp, resp_opts,
                                                4U, &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_BAD_REQ, resp.code);

    const char *bad_flt = "xyz";
    req.payload = (const uint8_t *)bad_flt;
    req.payload_len = 3U;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, flt_opts, 3U, &resp, resp_opts, 4U,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_BAD_REQ, resp.code);

    flt_opts[2].val = (const uint8_t *)"1";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, flt_opts, 3U, &resp, resp_opts, 4U,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_BAD_REQ, resp.code);

    /* Plain-text BOOL writes ("1", "0", "true", "false") + unsupported OBJLNK plain-text */
    const char *bool_toks[4] = {"1", "0", "true", "false"};
    const bool bool_exp[4] = {true, false, true, false};
    for (size_t bi = 0U; bi < 4U; bi++) {
        req.payload = (const uint8_t *)bool_toks[bi];
        req.payload_len = strlen(bool_toks[bi]);
        TEST_ASSERT_EQUAL(SYN_OK,
                          syn_lwm2m_process_request(&client, &req, flt_opts, 3U, &resp, resp_opts,
                                                    4U, &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
        TEST_ASSERT_EQUAL(COAP_RESP_CHANGED, resp.code);
        TEST_ASSERT_EQUAL(bool_exp[bi], custom_ctx.bool_val);
    }

    static const SYN_LwM2M_ResourceDesc objlnk_res[1] = {
        {2U, SYN_LWM2M_OP_W, SYN_LWM2M_TYPE_OBJLNK}};
    SYN_LwM2M_Object objlnk_obj = {
        .id = 401U,
        .instance_count = 1U,
        .resource_count = 1U,
        .resources = objlnk_res,
        .write = r4_custom_write_cb,
        .user_ctx = &custom_ctx,
    };
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &objlnk_obj));
    SYN_CoapOption lnk_opts[3] = {
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"401", .len = 3U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"2", .len = 1U},
    };
    req.payload = (const uint8_t *)"3:0";
    req.payload_len = 3U;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, lnk_opts, 3U, &resp, resp_opts, 4U,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_BAD_REQ, resp.code);

    /* Whole-instance GET (/3/0) with small resp_buf returns 5.00 Internal instead of truncated
     * TLV */
    SYN_CoapMsg inst_get = {.type = COAP_TYPE_CON, .code = COAP_CODE_GET, .msg_id = 0x43};
    SYN_CoapOption inst_get_opts[2] = {
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"3", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
    };
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &inst_get, inst_get_opts, 2U, &resp,
                                                resp_opts, 4U, &resp_opt_cnt, resp_buf, 8U));
    TEST_ASSERT_EQUAL(COAP_RESP_INTERNAL, resp.code);

    /* 3. R4-P05: Default pmax is 60s, and full observer table omits COAP_OPT_OBSERVE */
    for (size_t i = 0U; i < SYN_LWM2M_MAX_OBSERVERS; i++) {
        uint8_t tok = (uint8_t)(0x10U + i);
        TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&client, 3U, 0U, 0, &tok, 1U, 0U, 0U));
        TEST_ASSERT_EQUAL_UINT32(60U, client.observations[i].pmax);
    }

    uint8_t obs_zero = 0U;
    SYN_CoapOption obs_get_opts[4] = {
        {.num = COAP_OPT_OBSERVE, .val = &obs_zero, .len = 0U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"3303", .len = 4U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"5700", .len = 4U},
    };
    req.code = COAP_CODE_GET;
    req.token_len = 2U;
    req.token[0] = 0xFEU;
    req.token[1] = 0xEDU;
    req.payload = NULL;
    req.payload_len = 0U;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, obs_get_opts, 4U, &resp, resp_opts,
                                                4U, &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CONTENT, resp.code);
    TEST_ASSERT_EQUAL_UINT32(1U, resp_opt_cnt);
    TEST_ASSERT_EQUAL_UINT16(COAP_OPT_CONTENT_FORMAT, resp_opts[0].num);
}

static void test_r5_n10_lwm2m_fw_uri_observe_cancel_and_instance_notify(void)
{
    SYN_Transport dummy_tr = {0};
    SYN_LwM2M_Client client;
    TEST_ASSERT_TRUE(syn_lwm2m_client_init(&client, "r5-client", 300U, &dummy_tr));

    SYN_LwM2M_DeviceContext dev_ctx = {0};
    SYN_LwM2M_Object dev_obj = syn_lwm2m_make_device_object(&dev_ctx);
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &dev_obj));

    SYN_LwM2M_FirmwareContext fw_ctx = {0};
    SYN_LwM2M_Object fw_obj = syn_lwm2m_make_firmware_object(&fw_ctx);
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &fw_obj));

    SYN_LwM2M_SensorContext temp_ctx = {.sensor_value = 24.5, .unit = "Cel"};
    SYN_LwM2M_Object temp_obj = syn_lwm2m_make_temperature_object(&temp_ctx);
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &temp_obj));

    /* 1. Writing /5/0/1 sets DOWNLOADING; writing empty string resets to IDLE */
    SYN_LwM2M_Value uri_val = {.type = SYN_LWM2M_TYPE_STRING, .val.str = "coaps://ota.local/fw"};
    TEST_ASSERT_EQUAL(SYN_OK, fw_obj.write(0U, 1U, &uri_val, fw_obj.user_ctx));
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_STATE_DOWNLOADING, fw_ctx.state);

    uri_val.val.str = "";
    TEST_ASSERT_EQUAL(SYN_OK, fw_obj.write(0U, 1U, &uri_val, fw_obj.user_ctx));
    TEST_ASSERT_EQUAL(SYN_LWM2M_FW_STATE_IDLE, fw_ctx.state);

    /* 2. CoAP Observe=0 registers, Observe=1 deregisters without Observe opt, Observe=2 ignored */
    SYN_CoapMsg req = {
        .type = COAP_TYPE_CON,
        .code = COAP_CODE_GET,
        .msg_id = 0x5100U,
        .token_len = 2U,
        .token = {0x44U, 0x55U},
    };
    uint8_t obs_reg = 0U;
    uint8_t obs_dereg = 1U;
    uint8_t obs_unknown = 2U;
    SYN_CoapOption opts[4] = {
        {.num = COAP_OPT_OBSERVE, .val = &obs_reg, .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"3303", .len = 4U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"5700", .len = 4U},
    };
    SYN_CoapMsg resp = {0};
    SYN_CoapOption resp_opts[4];
    size_t resp_opt_cnt = 0U;
    uint8_t resp_buf[256];

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, opts, 4U, &resp, resp_opts, 4U,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CONTENT, resp.code);
    TEST_ASSERT_EQUAL_UINT32(2U, (uint32_t)resp_opt_cnt);
    TEST_ASSERT_EQUAL_UINT16(COAP_OPT_OBSERVE, resp_opts[0].num);
    TEST_ASSERT_TRUE(client.observations[0].active);

    /* Observe=1 cancels active observation and returns 2.05 without COAP_OPT_OBSERVE */
    opts[0].val = &obs_dereg;
    opts[0].len = 1U;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, opts, 4U, &resp, resp_opts, 4U,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL(COAP_RESP_CONTENT, resp.code);
    TEST_ASSERT_EQUAL_UINT32(1U, (uint32_t)resp_opt_cnt);
    TEST_ASSERT_EQUAL_UINT16(COAP_OPT_CONTENT_FORMAT, resp_opts[0].num);
    TEST_ASSERT_FALSE(client.observations[0].active);

    /* Observe=2 ignored (neither adds nor emits Observe option) */
    opts[0].val = &obs_unknown;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_lwm2m_process_request(&client, &req, opts, 4U, &resp, resp_opts, 4U,
                                                &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL_UINT32(1U, (uint32_t)resp_opt_cnt);
    TEST_ASSERT_FALSE(client.observations[0].active);

    /* 3. Whole-instance observation (res_id == -1) in syn_lwm2m_build_notify */
    uint8_t inst_tok[2] = {0x99U, 0xAAU};
    TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&client, 3303U, 0U, -1, inst_tok, 2U, 0U, 60U));
    size_t inst_len = syn_lwm2m_build_notify(&client, 3303U, 0U, -1, 0x6001U, &resp, resp_opts, 4U,
                                             &resp_opt_cnt, resp_buf, sizeof(resp_buf));
    TEST_ASSERT_GREATER_THAN_UINT32(0U, (uint32_t)inst_len);
    TEST_ASSERT_EQUAL(COAP_RESP_CONTENT, resp.code);
    TEST_ASSERT_EQUAL_UINT32(2U, (uint32_t)resp_opt_cnt);
    TEST_ASSERT_EQUAL_UINT16(COAP_OPT_OBSERVE, resp_opts[0].num);

    /* Instance notify edge cases: invalid instance, NULL resources, and failing resource read */
    uint8_t tok_bad_inst[1] = {0xB1U};
    TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&client, 3303U, 5U, -1, tok_bad_inst, 1U, 0U, 60U));
    TEST_ASSERT_EQUAL_UINT32(
        0U, (uint32_t)syn_lwm2m_build_notify(&client, 3303U, 5U, -1, 0x6002U, &resp, resp_opts, 4U,
                                             &resp_opt_cnt, resp_buf, sizeof(resp_buf)));

    SYN_LwM2M_Object no_res_obj = {
        .id = 501U, .instance_count = 1U, .resources = NULL, .read = err_read_cb};
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &no_res_obj));
    uint8_t tok_no_res[1] = {0xB2U};
    TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&client, 501U, 0U, -1, tok_no_res, 1U, 0U, 60U));
    TEST_ASSERT_EQUAL_UINT32(
        0U, (uint32_t)syn_lwm2m_build_notify(&client, 501U, 0U, -1, 0x6003U, &resp, resp_opts, 4U,
                                             &resp_opt_cnt, resp_buf, sizeof(resp_buf)));

    static const SYN_LwM2M_ResourceDesc fail_res[1] = {{0U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_INT}};
    SYN_LwM2M_Object fail_read_obj = {.id = 502U,
                                      .instance_count = 1U,
                                      .resource_count = 1U,
                                      .resources = fail_res,
                                      .read = err_read_cb};
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&client, &fail_read_obj));
    uint8_t tok_fail_rd[1] = {0xB3U};
    TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&client, 502U, 0U, -1, tok_fail_rd, 1U, 0U, 60U));
    TEST_ASSERT_EQUAL_UINT32(
        0U, (uint32_t)syn_lwm2m_build_notify(&client, 502U, 0U, -1, 0x6004U, &resp, resp_opts, 4U,
                                             &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
}

static void test_pro06_lwm2m_hardening(void)
{
    /* 1. TLV encode > 0xFFFFFFU and subtraction overflow checks */
    uint8_t tlv_out[16];
    SYN_LwM2M_TLV huge_tlv = {
        .type = SYN_LWM2M_TLV_RESOURCE, .id = 1U, .val = NULL, .len = 0x1000000U};
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_lwm2m_tlv_encode(&huge_tlv, tlv_out, 16U));

    SYN_LwM2M_TLV small_hdr_tlv = {
        .type = SYN_LWM2M_TLV_RESOURCE, .id = 1U, .val = NULL, .len = 0U};
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_lwm2m_tlv_encode(&small_hdr_tlv, tlv_out, 1U));

    SYN_LwM2M_TLV max24_tlv = {
        .type = SYN_LWM2M_TLV_RESOURCE, .id = 1U, .val = NULL, .len = 0xFFFFFFU};
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_lwm2m_tlv_encode(&max24_tlv, tlv_out, 16U));

    /* 2. Multi-segment & edge location_path splitting in Update and Deregister */
    static SYN_Transport dummy_tr;
    static SYN_LwM2M_Client s_client;
    static SYN_LwM2M_DeviceContext s_dev_ctx;
    (void)memset(&dummy_tr, 0, sizeof(dummy_tr));
    (void)memset(&s_dev_ctx, 0, sizeof(s_dev_ctx));
    TEST_ASSERT_TRUE(syn_lwm2m_client_init(&s_client, "pro06-ep", 300U, &dummy_tr));
    SYN_LwM2M_Object dev_obj = syn_lwm2m_make_device_object(&s_dev_ctx);
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&s_client, &dev_obj));

    SYN_CoapMsg req = {0};
    SYN_CoapOption opts[8];
    size_t opt_cnt = 0U;
    uint8_t pld[64];

    (void)strcpy(s_client.location_path, "rd/tenant/1234/");
    (void)syn_lwm2m_build_update_request(&s_client, 10U, NULL, 0U, 120U, &req, opts, 8U, &opt_cnt,
                                         pld, sizeof(pld));
    TEST_ASSERT_EQUAL_UINT8(COAP_CODE_POST, req.code);
    TEST_ASSERT_EQUAL_UINT32(4U, (uint32_t)opt_cnt);
    TEST_ASSERT_EQUAL_UINT32(2U, (uint32_t)opts[0].len);
    TEST_ASSERT_EQUAL_MEMORY("rd", opts[0].val, 2U);
    TEST_ASSERT_EQUAL_UINT32(6U, (uint32_t)opts[1].len);
    TEST_ASSERT_EQUAL_MEMORY("tenant", opts[1].val, 6U);
    TEST_ASSERT_EQUAL_UINT32(4U, (uint32_t)opts[2].len);
    TEST_ASSERT_EQUAL_MEMORY("1234", opts[2].val, 4U);

    /* Insufficient max_req_opts for 3-segment location_path */
    (void)syn_lwm2m_build_update_request(&s_client, 11U, NULL, 0U, 120U, &req, opts, 3U, &opt_cnt,
                                         pld, sizeof(pld));
    TEST_ASSERT_EQUAL_UINT8(0U, req.code);
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_lwm2m_build_deregister_request(
                                     &s_client, 12U, NULL, 0U, &req, opts, 2U, &opt_cnt));
    TEST_ASSERT_EQUAL_UINT32(1U, (uint32_t)syn_lwm2m_build_deregister_request(
                                     &s_client, 13U, NULL, 0U, &req, opts, 8U, &opt_cnt));
    TEST_ASSERT_EQUAL_UINT32(3U, (uint32_t)opt_cnt);

    /* Slash-only location_path rejected */
    (void)strcpy(s_client.location_path, "///");
    (void)syn_lwm2m_build_update_request(&s_client, 14U, NULL, 0U, 120U, &req, opts, 8U, &opt_cnt,
                                         pld, sizeof(pld));
    TEST_ASSERT_EQUAL_UINT8(0U, req.code);
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_lwm2m_build_deregister_request(
                                     &s_client, 15U, NULL, 0U, &req, opts, 8U, &opt_cnt));

    /* 3. Observe NULL token w/ non-zero len rejected; zero-len token allowed */
    TEST_ASSERT_FALSE(syn_lwm2m_observe_add(&s_client, 3U, 0U, 0, NULL, 2U, 0U, 10U));
    TEST_ASSERT_FALSE(syn_lwm2m_observe_remove(&s_client, NULL, 2U));
    TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&s_client, 3U, 0U, 0, NULL, 0U, 0U, 10U));
    TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&s_client, 3U, 0U, 0, NULL, 0U, 5U, 20U));
    TEST_ASSERT_TRUE(syn_lwm2m_observe_remove(&s_client, NULL, 0U));

    /* 4. token_len > 8 rejected in lwm2m_prepare_response */
    SYN_CoapMsg resp = {0};
    SYN_CoapOption resp_opts[4];
    size_t resp_opt_cnt = 0U;
    uint8_t resp_buf[128];
    req.type = COAP_TYPE_CON;
    req.code = COAP_CODE_GET;
    req.token_len = 9U;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_lwm2m_process_request(&s_client, &req, opts, 0U, &resp, resp_opts, 4U,
                                                    &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    req.token_len = 0U;

    /* 5. Strict bounded Uri-Path decimal parsing (no strtoul overread, <= 65535, <= 3 segs) */
    const uint8_t non_nul_alpha[4] = {'3', 'a', '9', '9'};
    SYN_CoapOption bad_uri[4] = {
        {.num = COAP_OPT_URI_PATH, .val = NULL, .len = 0U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
    };
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_lwm2m_process_request(&s_client, &req, bad_uri, 1U, &resp, resp_opts,
                                                    4U, &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_BAD_REQ, resp.code);

    bad_uri[0].val = non_nul_alpha;
    bad_uri[0].len = 2U; /* "3a" without NUL */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_lwm2m_process_request(&s_client, &req, bad_uri, 2U, &resp, resp_opts,
                                                    4U, &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_BAD_REQ, resp.code);

    bad_uri[0].val = (const uint8_t *)"70000";
    bad_uri[0].len = 5U; /* > 65535 */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_lwm2m_process_request(&s_client, &req, bad_uri, 2U, &resp, resp_opts,
                                                    4U, &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_BAD_REQ, resp.code);

    bad_uri[0].val = (const uint8_t *)"100000";
    bad_uri[0].len = 6U; /* > 5 digits */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_lwm2m_process_request(&s_client, &req, bad_uri, 2U, &resp, resp_opts,
                                                    4U, &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_BAD_REQ, resp.code);

    bad_uri[0].val = (const uint8_t *)"3";
    bad_uri[0].len = 1U; /* 4 segments -> invalid */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_lwm2m_process_request(&s_client, &req, bad_uri, 4U, &resp, resp_opts,
                                                    4U, &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_BAD_REQ, resp.code);

    /* 6. PUT cannot execute Reboot (/3/0/4) or Factory Reset (/3/0/5) and requires 3 segments */
    SYN_CoapOption dev_path[3] = {
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"3", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"4", .len = 1U},
    };
    req.code = COAP_CODE_PUT;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_lwm2m_process_request(&s_client, &req, dev_path, 2U, &resp, resp_opts,
                                                    4U, &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_METHOD_NA, resp.code);

    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_lwm2m_process_request(&s_client, &req, dev_path, 3U, &resp, resp_opts,
                                                    4U, &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_METHOD_NA, resp.code);
    TEST_ASSERT_FALSE(s_dev_ctx.reboot_requested);

    dev_path[2].val = (const uint8_t *)"5";
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_lwm2m_process_request(&s_client, &req, dev_path, 3U, &resp, resp_opts,
                                                    4U, &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_METHOD_NA, resp.code);
    TEST_ASSERT_FALSE(s_dev_ctx.factory_reset_requested);

    /* 7. Device /3/0/14 (UTC Offset) & /3/0/15 (Timezone) bounded writes */
    dev_path[2].val = (const uint8_t *)"14";
    dev_path[2].len = 2U;
    req.payload = (const uint8_t *)"+05:30";
    req.payload_len = 6U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_lwm2m_process_request(&s_client, &req, dev_path, 3U, &resp, resp_opts,
                                                    4U, &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_CHANGED, resp.code);
    TEST_ASSERT_EQUAL_STRING("+05:30", s_dev_ctx.utc_offset);

    /* Overflow >= 16 bytes for UTC offset returns 5.00 */
    req.payload = (const uint8_t *)"+00:00-too-long!";
    req.payload_len = 16U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_lwm2m_process_request(&s_client, &req, dev_path, 3U, &resp, resp_opts,
                                                    4U, &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_INTERNAL, resp.code);

    dev_path[2].val = (const uint8_t *)"15";
    req.payload = (const uint8_t *)"America/New_York";
    req.payload_len = 16U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_lwm2m_process_request(&s_client, &req, dev_path, 3U, &resp, resp_opts,
                                                    4U, &resp_opt_cnt, resp_buf, sizeof(resp_buf)));
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_CHANGED, resp.code);
    TEST_ASSERT_EQUAL_STRING("America/New_York", s_dev_ctx.timezone);

    /* Direct callback non-string / NULL str rejection */
    SYN_LwM2M_Value bad_str_val = {.type = SYN_LWM2M_TYPE_INT, .val.integer = 1};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, dev_obj.write(0U, 14U, &bad_str_val, dev_obj.user_ctx));
}

void run_lwm2m_tests(void)
{
    RUN_TEST(test_lwm2m_tlv_integer_codec);
    RUN_TEST(test_lwm2m_tlv_float_bool_string_opaque_codec);
    RUN_TEST(test_lwm2m_tlv_nested_instance_and_length_types);
    RUN_TEST(test_lwm2m_link_format_and_registration);
    RUN_TEST(test_lwm2m_request_dispatcher_read_write_exec);
    RUN_TEST(test_lwm2m_standard_objects_complete_reads_and_writes);
    RUN_TEST(test_lwm2m_observations_and_dispatcher_coverage);
    RUN_TEST(test_lwm2m_boundary_and_error_handling);
    RUN_TEST(test_lwm2m_r4_p01_p05_regressions);
    RUN_TEST(test_r5_n10_lwm2m_fw_uri_observe_cancel_and_instance_notify);
    RUN_TEST(test_pro06_lwm2m_hardening);
}
