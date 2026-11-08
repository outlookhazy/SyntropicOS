/**
 * @file test_ethercat.c
 * @brief Unity tests for EtherCAT Master/Slave Protocol Engine (syn_ethercat).
 */

#include "mocks/mock_port.h"
#include "syntropic/proto/syn_ethercat.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

#include <string.h>

static void test_ecat_init_and_esm(void)
{
    SYN_EcatNode node;
    syn_ecat_init(&node, 0x1001, NULL);

    TEST_ASSERT_EQUAL(SYN_ECAT_STATE_INIT, node.state);
    TEST_ASSERT_EQUAL(SYN_ECAT_STATE_INIT, node.target_state);
    TEST_ASSERT_EQUAL_UINT16(0x1001, node.station_addr);

    /* Request PREOP state */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_set_state(&node, SYN_ECAT_STATE_PREOP));
    TEST_ASSERT_EQUAL(SYN_ECAT_STATE_PREOP, node.target_state);

    syn_ecat_update(&node);
    TEST_ASSERT_EQUAL(SYN_ECAT_STATE_PREOP, node.state);

    /* Request SAFEOP state */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_set_state(&node, SYN_ECAT_STATE_SAFEOP));
    syn_ecat_update(&node);
    TEST_ASSERT_EQUAL(SYN_ECAT_STATE_SAFEOP, node.state);

    /* Request OP state */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_set_state(&node, SYN_ECAT_STATE_OP));
    syn_ecat_update(&node);
    TEST_ASSERT_EQUAL(SYN_ECAT_STATE_OP, node.state);

    /* Invalid state request */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_set_state(&node, (SYN_EcatState)0xFF));
}

static void test_ecat_datagram_build_and_parse(void)
{
    SYN_EcatNode node;
    syn_ecat_init(&node, 0x1001, NULL);

    uint8_t frame_buf[64];
    SYN_EcatDatagram dg = {.cmd = SYN_ECAT_CMD_FPRD,
                           .idx = 0x42,
                           .addr = 0x10010000,
                           .m = 0,
                           .circ = 0,
                           .irq = 0x0000};

    uint8_t payload[4] = {0x11, 0x22, 0x33, 0x44};
    size_t frame_len = syn_ecat_build_datagram_frame(frame_buf, sizeof(frame_buf), &dg, payload, 4);

    /* Total = 2 (header) + 10 (dg header) + 4 (data) + 2 (wkc) = 18 bytes */
    TEST_ASSERT_EQUAL_INT(18, frame_len);

    /* Slave ESC processes FPRD at station 0x1001 and increments WKC to 1 */
    uint16_t wkc = 0;
    SYN_Status st = syn_ecat_slave_process_frame(&node, frame_buf, frame_len, &wkc);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_UINT16(1, wkc);
    TEST_ASSERT_EQUAL_UINT16(1, node.wkc_last);

    /* Read-only master parser validates existing WKC without mutating frame */
    const uint8_t *ro_frame = frame_buf;
    wkc = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_parse_frame(&node, ro_frame, frame_len, &wkc));
    TEST_ASSERT_EQUAL_UINT16(1, wkc);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_parse_frame(&node, ro_frame, frame_len, NULL));

    /* Parse corrupted/short frame */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_parse_frame(&node, ro_frame, 5, &wkc));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_slave_process_frame(&node, frame_buf, 5, &wkc));

    /* Parse frame addressed to non-matching station (WKC remains 0 < wkc_expected) */
    dg.addr = 0x10020000U;
    frame_len = syn_ecat_build_datagram_frame(frame_buf, sizeof(frame_buf), &dg, payload, 4);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_slave_process_frame(&node, frame_buf, frame_len, &wkc));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_slave_process_frame(&node, frame_buf, frame_len, NULL));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_parse_frame(&node, frame_buf, frame_len, &wkc));
}

static void test_ecat_coe_sdo_mailbox(void)
{
    uint8_t mbox_buf[32];

    /* SDO Download (Write) Request */
    uint32_t val = 0x12345678;
    size_t len = syn_ecat_coe_encode_sdo_download(mbox_buf, sizeof(mbox_buf), 0x6040, 0x00, &val,
                                                  sizeof(val));
    TEST_ASSERT_EQUAL_INT(10, len);

    /* Check Mailbox CoE header (Type 3 SDO Req) */
    uint16_t coe_hdr = (uint16_t)(mbox_buf[0] | (mbox_buf[1] << 8));
    TEST_ASSERT_EQUAL_UINT16((SYN_ECAT_COE_TYPE_SDO_REQ & 0x0F) << 12, coe_hdr);
    TEST_ASSERT_EQUAL_HEX8(0x40, mbox_buf[3]); /* Index LSB (0x6040 -> 0x40) */
    TEST_ASSERT_EQUAL_HEX8(0x60, mbox_buf[4]); /* Index MSB (0x6040 -> 0x60) */
    TEST_ASSERT_EQUAL_HEX8(0x00, mbox_buf[5]); /* Subindex */

    /* SDO Upload (Read) Request */
    len = syn_ecat_coe_encode_sdo_upload(mbox_buf, sizeof(mbox_buf), 0x6041, 0x00);
    TEST_ASSERT_EQUAL_INT(10, len);
    TEST_ASSERT_EQUAL_HEX8(0x40, mbox_buf[2]); /* 0x40 Upload Command */
    TEST_ASSERT_EQUAL_HEX8(0x41, mbox_buf[3]);
    TEST_ASSERT_EQUAL_HEX8(0x60, mbox_buf[4]);
}

static void test_ecat_edge_cases(void)
{
    SYN_EcatNode node;
    syn_ecat_init(&node, 0x1001, NULL);

    uint8_t buf[64];
    SYN_EcatDatagram dg = {
        .cmd = SYN_ECAT_CMD_APWR, .idx = 0x01, .addr = 0x0000, .m = 1, .circ = 1, .irq = 0x0000};

    /* Buffer capacity error */
    TEST_ASSERT_EQUAL_INT(0, syn_ecat_build_datagram_frame(buf, 10, &dg, NULL, 0));

    /* Build datagram with m=1 and circ=1 flags set */
    size_t len = syn_ecat_build_datagram_frame(buf, sizeof(buf), &dg, NULL, 0);
    TEST_ASSERT_EQUAL_INT(14, len);

    /* Invalid frame type in parser */
    buf[1] = 0x20; /* Frame type = 2 instead of 1 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_parse_frame(&node, buf, len, NULL));

    /* Truncated datagram length in header */
    buf[0] = 0xFF; /* dg_len = 0x07FF (2047 bytes) */
    buf[1] = 0x17;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_parse_frame(&node, buf, len, NULL));

    /* SDO Download buffer capacity / payload size overflow */
    uint32_t val = 123;
    TEST_ASSERT_EQUAL_INT(
        0, syn_ecat_coe_encode_sdo_download(buf, sizeof(buf), 0x1000, 0x00, &val, 8));
    TEST_ASSERT_EQUAL_INT(0, syn_ecat_coe_encode_sdo_download(buf, 5, 0x1000, 0x00, &val, 4));

    /* SDO Upload buffer capacity overflow */
    TEST_ASSERT_EQUAL_INT(0, syn_ecat_coe_encode_sdo_upload(buf, 5, 0x1000, 0x00));
}

static void test_ecat_multi_datagram(void)
{
    uint8_t buf[128];
    syn_ecat_frame_begin(buf, sizeof(buf));

    uint8_t d1_payload[2] = {0xAB, 0xCD};
    uint8_t d2_payload[4] = {0x11, 0x22, 0x33, 0x44};

    size_t sz1 = syn_ecat_frame_add_datagram(buf, sizeof(buf), SYN_ECAT_CMD_APRD, 0x01, 0x00000000,
                                             d1_payload, sizeof(d1_payload), false);
    TEST_ASSERT_TRUE(sz1 > 0);

    size_t sz2 = syn_ecat_frame_add_datagram(buf, sizeof(buf), SYN_ECAT_CMD_FPRD, 0x02, 0x10010010,
                                             d2_payload, sizeof(d2_payload), true);
    TEST_ASSERT_TRUE(sz2 > sz1);

    size_t final_sz = syn_ecat_frame_finalize(buf);
    TEST_ASSERT_EQUAL_INT(sz2, final_sz);

    /* Simulate responses on frame (set WKCs) */
    /* Datagram 1 WKC offset = 2 + 10 + 2 = 14 */
    buf[14] = 0x01;
    buf[15] = 0x00;

    /* Datagram 2 WKC offset = 16 + 10 + 4 = 30 */
    buf[30] = 0x02;
    buf[31] = 0x00;

    size_t offset = 2;
    SYN_EcatDatagramResult res;

    TEST_ASSERT_TRUE(syn_ecat_frame_parse_next(buf, final_sz, &offset, &res));
    TEST_ASSERT_EQUAL_HEX8(SYN_ECAT_CMD_APRD, res.cmd);
    TEST_ASSERT_EQUAL_HEX8(0x01, res.idx);
    TEST_ASSERT_EQUAL_HEX32(0x00000000, res.addr);
    TEST_ASSERT_EQUAL_INT(2, res.data_len);
    TEST_ASSERT_EQUAL_HEX8(0xAB, res.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0xCD, res.data[1]);
    TEST_ASSERT_EQUAL_UINT16(1, res.wkc);

    TEST_ASSERT_TRUE(syn_ecat_frame_parse_next(buf, final_sz, &offset, &res));
    TEST_ASSERT_EQUAL_HEX8(SYN_ECAT_CMD_FPRD, res.cmd);
    TEST_ASSERT_EQUAL_HEX8(0x02, res.idx);
    TEST_ASSERT_EQUAL_HEX32(0x10010010, res.addr);
    TEST_ASSERT_EQUAL_INT(4, res.data_len);
    TEST_ASSERT_EQUAL_HEX8(0x11, res.data[0]);
    TEST_ASSERT_EQUAL_UINT16(2, res.wkc);

    TEST_ASSERT_FALSE(syn_ecat_frame_parse_next(buf, final_sz, &offset, &res));
}

static void test_ecat_master_phase2(void)
{
    SYN_EcatMaster m;
    uint8_t tx[128];
    uint8_t rx[128];
    uint8_t out_img[32];
    uint8_t in_img[32];

    /* Invalid init params */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ecat_master_init(NULL, tx, sizeof(tx), rx, sizeof(rx), out_img,
                                           sizeof(out_img), in_img, sizeof(in_img)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ecat_master_init(&m, NULL, sizeof(tx), rx, sizeof(rx), out_img,
                                           sizeof(out_img), in_img, sizeof(in_img)));

    /* Valid init */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_init(&m, tx, sizeof(tx), rx, sizeof(rx), out_img,
                                                   sizeof(out_img), in_img, sizeof(in_img)));

    /* Scan bus encode / decode */
    size_t len = syn_ecat_encode_scan_bus(&m);
    TEST_ASSERT_TRUE(len > 0);

    /* Copy tx to rx, set WKC to 3 */
    memcpy(rx, tx, len);
    rx[len - 2] = 0x03;
    rx[len - 1] = 0x00;

    uint8_t count = syn_ecat_decode_scan_bus(&m, len);
    TEST_ASSERT_EQUAL_UINT8(3, count);
    TEST_ASSERT_EQUAL_UINT8(3, m.slave_count);

    /* Assign address */
    len = syn_ecat_encode_assign_addr(&m, 0, 0x1001);
    TEST_ASSERT_TRUE(len > 0);

    /* Read SII */
    len = syn_ecat_encode_read_sii(&m, 0x1001, 0x0008);
    TEST_ASSERT_TRUE(len > 0);

    /* Construct SII response in rx: payload 4 bytes = 0x000000E0 */
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t sii_resp[4] = {0xE0, 0x00, 0x00, 0x00};
    size_t rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPWR, 0x02, 0x10010502,
                                                sii_resp, 4, false);
    rx[rx_len - 2] = 0x01; /* WKC = 1 */

    uint32_t sii_val = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_decode_read_sii(&m, rx_len, &sii_val));
    TEST_ASSERT_EQUAL_HEX32(0x000000E0, sii_val);

    /* Write SM */
    SYN_EcatSMConfig sm_cfg = {
        .start_addr = 0x1000, .length = 128, .control = 0x26, .enable = 0x01};
    len = syn_ecat_encode_write_sm(&m, 0x1001, 0, &sm_cfg);
    TEST_ASSERT_TRUE(len > 0);

    /* Write FMMU */
    SYN_EcatFMMUConfig fmmu_cfg = {.logical_start = 0x00010000,
                                   .length = 16,
                                   .logical_start_bit = 0,
                                   .logical_end_bit = 7,
                                   .phys_start_addr = 0x1000,
                                   .phys_start_bit = 0,
                                   .type = 0x03,
                                   .enable = true};
    len = syn_ecat_encode_write_fmmu(&m, 0x1001, 0, &fmmu_cfg);
    TEST_ASSERT_TRUE(len > 0);

    /* Read AL Status */
    len = syn_ecat_encode_read_al_status(&m, 0x1001);
    TEST_ASSERT_TRUE(len > 0);

    /* Construct AL Status response in rx (state = PREOP 0x02, status_code = 0x0000) */
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t al_resp[4] = {0x02, 0x00, 0x00, 0x00};
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x05, 0x10010130,
                                         al_resp, 4, false);
    rx[rx_len - 2] = 0x01;

    SYN_EcatState state = SYN_ECAT_STATE_NONE;
    uint16_t status_code = 0xFFFF;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_decode_read_al_status(&m, rx_len, &state, &status_code));
    TEST_ASSERT_EQUAL(SYN_ECAT_STATE_PREOP, state);
    TEST_ASSERT_EQUAL_UINT16(0x0000, status_code);
}

static inline uint16_t ecat_test_load16_le(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static inline void ecat_test_store16_le(uint8_t *p, uint16_t val)
{
    p[0] = (uint8_t)(val & 0xFF);
    p[1] = (uint8_t)((val >> 8) & 0xFF);
}

static inline void ecat_test_store32_le(uint8_t *p, uint32_t val)
{
    p[0] = (uint8_t)(val & 0xFF);
    p[1] = (uint8_t)((val >> 8) & 0xFF);
    p[2] = (uint8_t)((val >> 16) & 0xFF);
    p[3] = (uint8_t)((val >> 24) & 0xFF);
}

static void test_ecat_master_phases3_to_6(void)
{
    SYN_EcatMaster m;
    uint8_t tx[256];
    uint8_t rx[256];
    uint8_t out_img[16] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t in_img[16] = {0};

    syn_ecat_master_init(&m, tx, sizeof(tx), rx, sizeof(rx), out_img, 4, in_img, 4);
    m.logical_addr = 0x00010000;
    m.wkc_expected = 1;

    /* Phase 3: Mailbox CoE SDO write / read (128B mailbox = 142B frame) */
    uint32_t val = 0x12345678;
    size_t len = syn_ecat_encode_coe_sdo_write(&m, 0x1001, 0x6040, 0x00, &val, 4);
    TEST_ASSERT_EQUAL_size_t(14U + SYN_ECAT_MBOX_SIZE, len);

    len = syn_ecat_encode_coe_sdo_read(&m, 0x1001, 0x6041, 0x00);
    TEST_ASSERT_EQUAL_size_t(14U + SYN_ECAT_MBOX_SIZE, len);

    /* Construct CoE SDO upload response in rx */
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t sdo_resp_payload[16] = {0};
    ecat_test_store16_le(&sdo_resp_payload[0], 10);
    ecat_test_store16_le(&sdo_resp_payload[2], 0x1001);
    sdo_resp_payload[4] = 0x00;
    sdo_resp_payload[5] = 0x03; /* Mailbox type CoE */
    ecat_test_store16_le(&sdo_resp_payload[6], (SYN_ECAT_COE_TYPE_SDO_RESP & 0x0F) << 12);
    sdo_resp_payload[8] = 0x43; /* 0x43 = expedited 4B */
    ecat_test_store16_le(&sdo_resp_payload[9], 0x6041);
    sdo_resp_payload[11] = 0x00;
    ecat_test_store32_le(&sdo_resp_payload[12], 0x87654321);

    size_t rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x07, 0x10011000,
                                                sdo_resp_payload, 16, false);
    rx[rx_len - 2] = 0x01;

    uint32_t read_val = 0;
    size_t out_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_decode_coe_sdo_response(&m, rx_len, &read_val,
                                                               sizeof(read_val), &out_len));
    TEST_ASSERT_EQUAL_INT(4, out_len);
    TEST_ASSERT_EQUAL_HEX32(0x87654321, read_val);

    /* Phase 4: Cyclic process exchange */
    len = syn_ecat_encode_cyclic(&m);
    TEST_ASSERT_TRUE(len > 0);

    /* Construct LRW response in rx with input image {0xCA, 0xFE, 0xBA, 0xBE} */
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t cyclic_resp[4] = {0xCA, 0xFE, 0xBA, 0xBE};
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_LRW, 0x10, m.logical_addr,
                                         cyclic_resp, 4, false);
    rx[rx_len - 2] = 0x01; /* WKC = 1 (matches expected) */

    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_decode_cyclic(&m, rx_len));
    TEST_ASSERT_EQUAL_HEX8(0xCA, in_img[0]);
    TEST_ASSERT_EQUAL_HEX8(0xFE, in_img[1]);
    TEST_ASSERT_EQUAL_UINT32(1, m.cycle_count);

    /* Phase 5: Set AL Control */
    len = syn_ecat_encode_set_al_control(&m, 0x1001, SYN_ECAT_STATE_OP);
    TEST_ASSERT_TRUE(len > 0);

    /* Phase 6: DC read system time and configure */
    len = syn_ecat_encode_dc_read_system_time(&m);
    TEST_ASSERT_TRUE(len > 0);

    len = syn_ecat_encode_dc_configure(&m, 0x1001, 1000000U, 0);
    TEST_ASSERT_TRUE(len > 0);
}

static void test_ecat_master_protothread_tasks(void)
{
    SYN_EcatMaster m;
    uint8_t tx[128] = {0};
    uint8_t rx[128] = {0};
    uint8_t out_img[16] = {0x11, 0x22};
    uint8_t in_img[16] = {0};

    syn_ecat_master_init(&m, tx, sizeof(tx), rx, sizeof(rx), out_img, 2, in_img, 2);
    m.logical_addr = 0x00010000;
    m.wkc_expected = 1;

    SYN_PT pt;
    PT_INIT(&pt);

    /* Null params safety */
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_scan_task(NULL, &m));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_transition_task(NULL, &m, SYN_ECAT_STATE_PREOP));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_cyclic_task(NULL, &m));

    /* 1. Step scan task (step 1: sends BRD scan) */
    SYN_PT_Status status = syn_ecat_master_scan_task(&pt, &m);
    TEST_ASSERT_EQUAL(PT_IDLE, status);

    const uint8_t *tx_ptr = NULL;
    size_t tx_len = 0;
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len));
    TEST_ASSERT_TRUE(tx_len > 0);

    /* Feed mock scan response (WKC = 1 slave) */
    memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2] = 0x01;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(&m, rx, tx_len));

    /* Step scan task (step 2: assigns address for slave 0) */
    status = syn_ecat_master_scan_task(&pt, &m);
    TEST_ASSERT_EQUAL(PT_IDLE, status);
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len));

    /* Feed mock assign address response */
    memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2] = 0x01;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(&m, rx, tx_len));

    /* Complete scan task */
    status = syn_ecat_master_scan_task(&pt, &m);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL_UINT8(1, m.slave_count);
    TEST_ASSERT_EQUAL_HEX16(0x1001, m.slaves[0].station_addr);

    /* 2. Transition task to PREOP */
    PT_INIT(&pt);
    status = syn_ecat_master_transition_task(&pt, &m, SYN_ECAT_STATE_PREOP);
    TEST_ASSERT_EQUAL(PT_IDLE, status);

    /* Process pending TX frames for SM0, SM1, AL Control, AL Status */
    int guard1 = 0;
    while (status == PT_IDLE && guard1++ < 1000) {
        if (syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len)) {
            memcpy(rx, tx_ptr, tx_len);
            /* If AL status read datagram (4B response), set AL status = PREOP 0x02 */
            if (tx_len >= 18 && rx[2] == SYN_ECAT_CMD_FPRD) {
                syn_ecat_frame_begin(rx, sizeof(rx));
                uint8_t al_resp[4] = {0x02, 0x00, 0x00, 0x00};
                tx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x05,
                                                     0x10010130, al_resp, 4, false);
            }
            rx[tx_len - 2] = 0x01;
            syn_ecat_master_set_rx_frame(&m, rx, tx_len);
        }
        status = syn_ecat_master_transition_task(&pt, &m, SYN_ECAT_STATE_PREOP);
    }
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL(SYN_ECAT_STATE_PREOP, m.master_state);

    /* 3. Cyclic task */
    PT_INIT(&pt);
    status = syn_ecat_master_cyclic_task(&pt, &m);
    TEST_ASSERT_EQUAL(PT_IDLE, status);
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len));

    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t c_resp[2] = {0x99, 0x88};
    size_t rx_l = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_LRW, 0x10,
                                              m.logical_addr, c_resp, 2, false);
    rx[rx_l - 2] = 0x01;
    syn_ecat_master_set_rx_frame(&m, rx, rx_l);

    status = syn_ecat_master_cyclic_task(&pt, &m);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL_HEX8(0x99, in_img[0]);
    TEST_ASSERT_EQUAL_HEX8(0x88, in_img[1]);
}

static void test_ecat_master_sii_and_sdo_tasks(void)
{
    SYN_EcatMaster m;
    uint8_t tx[256];
    uint8_t rx[256];
    uint8_t out_img[16] = {0};
    uint8_t in_img[16] = {0};

    syn_ecat_master_init(&m, tx, sizeof(tx), rx, sizeof(rx), out_img, 2, in_img, 2);

    SYN_PT pt;
    PT_INIT(&pt);

    uint32_t sii_data = 0;
    /* Null params safety */
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_read_sii_task(NULL, &m, 0x1001, 0x08, &sii_data));

    /* 1. SII Read task step 1: write address offset */
    SYN_PT_Status status = syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_data);
    TEST_ASSERT_EQUAL(PT_IDLE, status);

    const uint8_t *tx_ptr = NULL;
    size_t tx_len = 0;
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len));
    memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2] = 0x01;
    syn_ecat_master_set_rx_frame(&m, rx, tx_len);

    /* SII Read task step 2: write command */
    status = syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_data);
    TEST_ASSERT_EQUAL(PT_IDLE, status);
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len));
    memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2] = 0x01;
    syn_ecat_master_set_rx_frame(&m, rx, tx_len);

    /* SII Read task step 3: poll status (send 0x0000 = not busy) */
    status = syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_data);
    TEST_ASSERT_EQUAL(PT_IDLE, status);
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len));

    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t not_busy[2] = {0x00, 0x00};
    size_t rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x10010502,
                                                not_busy, 2, false);
    rx[rx_len - 2] = 0x01;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    /* SII Read task step 4: read data 0x12345678 */
    status = syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_data);
    TEST_ASSERT_EQUAL(PT_IDLE, status);
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len));

    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t sii_val[4] = {0x78, 0x56, 0x34, 0x12};
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x10010508,
                                         sii_val, 4, false);
    rx[rx_len - 2] = 0x01;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    status = syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_data);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL_HEX32(0x12345678, sii_data);

    /* 2. CoE SDO Read task */
    PT_INIT(&pt);
    uint32_t sdo_data = 0;
    size_t out_len = 0;
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_sdo_read_task(NULL, &m, 0x1001, 0x6041, 0,
                                                               &sdo_data, 4, &out_len));

    /* SDO read step 1: write request to SM0 */
    status = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x6041, 0, &sdo_data, 4, &out_len);
    TEST_ASSERT_EQUAL(PT_IDLE, status);
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len));
    memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2] = 0x01;
    syn_ecat_master_set_rx_frame(&m, rx, tx_len);

    /* SDO read step 2: poll SM1 status (0x08 = full) */
    status = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x6041, 0, &sdo_data, 4, &out_len);
    TEST_ASSERT_EQUAL(PT_IDLE, status);
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len));

    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t sm1_full[1] = {0x08};
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x1001080D,
                                         sm1_full, 1, false);
    rx[rx_len - 2] = 0x01;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    /* SDO read step 3: read SM1 response (0x87654321) */
    status = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x6041, 0, &sdo_data, 4, &out_len);
    TEST_ASSERT_EQUAL(PT_IDLE, status);
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len));

    uint8_t sdo_resp_payload[16] = {0};
    ecat_test_store16_le(&sdo_resp_payload[0], 0x000A);
    ecat_test_store16_le(&sdo_resp_payload[2], 0x1001);
    sdo_resp_payload[4] = 0x00;
    sdo_resp_payload[5] = 0x03;
    ecat_test_store16_le(&sdo_resp_payload[6], (SYN_ECAT_COE_TYPE_SDO_RESP & 0x0F) << 12);
    sdo_resp_payload[8] = 0x43;
    ecat_test_store16_le(&sdo_resp_payload[9], 0x6041);
    sdo_resp_payload[11] = 0x00;
    ecat_test_store32_le(&sdo_resp_payload[12], 0x87654321);

    syn_ecat_frame_begin(rx, sizeof(rx));
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x10011080,
                                         sdo_resp_payload, 16, false);
    rx[rx_len - 2] = 0x01;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    status = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x6041, 0, &sdo_data, 4, &out_len);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL_HEX32(0x87654321, sdo_data);
}

static void test_ecat_reg_and_task_edge_cases(void)
{
    SYN_EcatMaster m;
    uint8_t tx[256];
    uint8_t rx[256];
    uint8_t out_img[16] = {0};
    uint8_t in_img[16] = {0};

    syn_ecat_master_init(&m, tx, sizeof(tx), rx, sizeof(rx), out_img, 2, in_img, 2);

    /* Null / zero edge cases for reg encode/decode */
    TEST_ASSERT_EQUAL_INT(0, syn_ecat_encode_read_reg(NULL, 0x1001, 0x0502, 2));
    TEST_ASSERT_EQUAL_INT(0, syn_ecat_encode_read_reg(&m, 0x1001, 0x0502, 0));
    TEST_ASSERT_EQUAL_INT(0, syn_ecat_encode_write_reg(NULL, 0x1001, 0x0502, tx, 2));
    TEST_ASSERT_EQUAL_INT(0, syn_ecat_encode_write_reg(&m, 0x1001, 0x0502, NULL, 2));
    TEST_ASSERT_EQUAL_INT(0, syn_ecat_encode_write_reg(&m, 0x1001, 0x0502, tx, 0));

    uint8_t dummy[4] = {0};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ecat_decode_read_reg(NULL, 18, dummy, 2));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ecat_decode_read_reg(&m, 10, dummy, 2));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ecat_decode_read_reg(&m, 18, NULL, 2));

    /* Decode reg failure when WKC is 0 */
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t reg_val[2] = {0x12, 0x34};
    size_t rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x10010502,
                                                reg_val, 2, false);
    rx[rx_len - 2] = 0x00; /* WKC = 0 (failure) */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_decode_read_reg(&m, rx_len, dummy, 2));

    /* Test SII polling loop with 1 busy response (0x8000) then 1 clear response (0x0000) */
    SYN_PT pt;
    PT_INIT(&pt);
    uint32_t sii_val = 0;
    syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val); /* step 1 */
    const uint8_t *tx_ptr = NULL;
    size_t tx_len = 0;
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, tx_len);

    syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val); /* step 2 */
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, tx_len);

    syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val); /* step 3 poll (busy) */
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t busy[2] = {0x00, 0x80}; /* Busy bit set */
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x10010502, busy,
                                         2, false);
    rx[rx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    /* Poll again (not busy) */
    syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val);
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t idle[2] = {0x00, 0x00};
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x10010502, idle,
                                         2, false);
    rx[rx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    /* Read data */
    syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val);
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t val32[4] = {0x11, 0x22, 0x33, 0x44};
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x10010508, val32,
                                         4, false);
    rx[rx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    SYN_PT_Status status = syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL_HEX32(0x44332211, sii_val);

    /* SDO Read task 16-bit output buffer branch */
    PT_INIT(&pt);
    uint16_t sdo16 = 0;
    size_t out16 = 0;
    syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x1000, 0, &sdo16, 2, &out16); /* step 1 */
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, tx_len);

    syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x1000, 0, &sdo16, 2,
                                  &out16); /* step 2 poll empty then full */
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t sm1_empty[1] = {0x00};
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x1001080D,
                                         sm1_empty, 1, false);
    rx[rx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x1000, 0, &sdo16, 2, &out16);
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t sm1_ready[1] = {0x08};
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x1001080D,
                                         sm1_ready, 1, false);
    rx[rx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x1000, 0, &sdo16, 2,
                                  &out16); /* step 3 read response */
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    uint8_t payload16[16] = {0};
    ecat_test_store16_le(&payload16[0], 0x000A);
    ecat_test_store16_le(&payload16[2], 0x1001);
    payload16[5] = 0x03;
    ecat_test_store16_le(&payload16[6], (SYN_ECAT_COE_TYPE_SDO_RESP & 0x0F) << 12);
    payload16[8] = 0x47; /* 2-byte expedited */
    ecat_test_store16_le(&payload16[9], 0x1000);
    ecat_test_store16_le(&payload16[12], 0xABCD);
    syn_ecat_frame_begin(rx, sizeof(rx));
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x10011080,
                                         payload16, 16, false);
    rx[rx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    status = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x1000, 0, &sdo16, 2, &out16);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL_HEX16(0xABCD, sdo16);

    /* 8-bit output buffer branch */
    PT_INIT(&pt);
    uint8_t sdo8 = 0;
    size_t out8 = 0;
    syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x1000, 0, &sdo8, 1, &out8);
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, tx_len);
    syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x1000, 0, &sdo8, 1, &out8);
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t sm1_r[1] = {0x08};
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x1001080D, sm1_r,
                                         1, false);
    rx[rx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);
    syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x1000, 0, &sdo8, 1, &out8);
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    uint8_t payload8[16] = {0};
    ecat_test_store16_le(&payload8[0], 0x000A);
    ecat_test_store16_le(&payload8[2], 0x1001);
    payload8[5] = 0x03;
    ecat_test_store16_le(&payload8[6], (SYN_ECAT_COE_TYPE_SDO_RESP & 0x0F) << 12);
    payload8[8] = 0x4F;
    payload8[12] = 0x55;
    syn_ecat_frame_begin(rx, sizeof(rx));
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x10011080,
                                         payload8, 16, false);
    rx[rx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);
    status = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x1000, 0, &sdo8, 1, &out8);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL_HEX8(0x55, sdo8);
}

static void test_ecat_pdo_mapping_discovery_task(void)
{
    SYN_EcatMaster m;
    uint8_t tx[256];
    uint8_t rx[256];
    uint8_t out_img[16] = {0};
    uint8_t in_img[16] = {0};

    syn_ecat_master_init(&m, tx, sizeof(tx), rx, sizeof(rx), out_img, 2, in_img, 2);

    SYN_PT pt;
    PT_INIT(&pt);

    uint16_t rx_bytes = 0;
    uint16_t tx_bytes = 0;

    /* Null params safety */
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_discover_pdo_mapping_task(NULL, &m, 0x1001,
                                                                           &rx_bytes, &tx_bytes));

    /* Run PDO discovery with 1 RxPDO (0x1600 mapping 16-bit) and 1 TxPDO (0x1A00 mapping 16-bit) */
    SYN_PT_Status status =
        syn_ecat_master_discover_pdo_mapping_task(&pt, &m, 0x1001, &rx_bytes, &tx_bytes);

    const uint8_t *tx_ptr = NULL;
    size_t tx_len = 0;
    uint16_t req_idx = 0;
    uint8_t req_sub = 0;

    int guard2 = 0;
    while (status == PT_IDLE && guard2++ < 1000) {
        if (syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len)) {
            memcpy(rx, tx_ptr, tx_len);
            /* If writing SM0 request, capture index/subindex */
            if (tx_len >= 24 && rx[2] == SYN_ECAT_CMD_FPWR &&
                ecat_test_load16_le(&rx[4]) == 0x1000) {
                req_idx = ecat_test_load16_le(&rx[21]);
                req_sub = rx[23];
            }
            /* If reading SM1 mailbox status (0x080D), return SM1 full (0x08) */
            else if (tx_len >= 15 && rx[2] == SYN_ECAT_CMD_FPRD &&
                     ecat_test_load16_le(&rx[4]) == 0x080D) {
                syn_ecat_frame_begin(rx, sizeof(rx));
                uint8_t ready[1] = {0x08};
                tx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01,
                                                     0x1001080D, ready, 1, false);
            }
            /* If reading SM1 mailbox buffer (0x1080), return appropriate response */
            else if (tx_len >= 18 && rx[2] == SYN_ECAT_CMD_FPRD &&
                     ecat_test_load16_le(&rx[4]) == 0x1080) {
                uint8_t payload[16] = {0};
                ecat_test_store16_le(&payload[0], 0x000A);
                ecat_test_store16_le(&payload[2], 0x1001);
                payload[5] = 0x03;
                ecat_test_store16_le(&payload[6], (SYN_ECAT_COE_TYPE_SDO_RESP & 0x0F) << 12);
                ecat_test_store16_le(&payload[9], req_idx);
                payload[11] = req_sub;

                if (req_idx == 0x1C12 && req_sub == 0) {
                    payload[8] = 0x4F;
                    payload[12] = 0x01; /* 1 RxPDO */
                } else if (req_idx == 0x1C12 && req_sub == 1) {
                    payload[8] = 0x47;
                    ecat_test_store16_le(&payload[12], 0x1600); /* mapping obj 0x1600 */
                } else if (req_idx == 0x1600 && req_sub == 0) {
                    payload[8] = 0x4F;
                    payload[12] = 0x01; /* 1 entry mapped */
                } else if (req_idx == 0x1600 && req_sub == 1) {
                    payload[8] = 0x43;
                    ecat_test_store32_le(&payload[12], 0x60400010); /* 16 bits */
                } else if (req_idx == 0x1C13 && req_sub == 0) {
                    payload[8] = 0x4F;
                    payload[12] = 0x01; /* 1 TxPDO */
                } else if (req_idx == 0x1C13 && req_sub == 1) {
                    payload[8] = 0x47;
                    ecat_test_store16_le(&payload[12], 0x1A00); /* mapping obj 0x1A00 */
                } else if (req_idx == 0x1A00 && req_sub == 0) {
                    payload[8] = 0x4F;
                    payload[12] = 0x01; /* 1 entry mapped */
                } else if (req_idx == 0x1A00 && req_sub == 1) {
                    payload[8] = 0x43;
                    ecat_test_store32_le(&payload[12], 0x60410010); /* 16 bits */
                }

                syn_ecat_frame_begin(rx, sizeof(rx));
                tx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01,
                                                     0x10011080, payload, 16, false);
            }
            rx[tx_len - 2] = 0x01;
            syn_ecat_master_set_rx_frame(&m, rx, tx_len);
        }
        status = syn_ecat_master_discover_pdo_mapping_task(&pt, &m, 0x1001, &rx_bytes, &tx_bytes);
    }

    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL_UINT16(2, rx_bytes);
    TEST_ASSERT_EQUAL_UINT16(2, tx_bytes);
}

static void test_ecat_task_poll_timeouts(void)
{
    SYN_EcatMaster m;
    uint8_t tx[256];
    uint8_t rx[256];
    uint8_t out_img[16] = {0};
    uint8_t in_img[16] = {0};

    syn_ecat_master_init(&m, tx, sizeof(tx), rx, sizeof(rx), out_img, 2, in_img, 2);

    /* 1. SII task timeout test */
    SYN_PT pt;
    PT_INIT(&pt);
    uint32_t sii_val = 0;
    syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val); /* step 1 write addr */
    const uint8_t *tx_ptr = NULL;
    size_t tx_len = 0;
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, tx_len);

    syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val); /* step 2 write cmd */
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, tx_len);

    /* Force poll_retries = 1 to test immediate timeout on next poll step */
    syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val); /* step 3 poll start */
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    m.poll_retries = 1;

    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t busy[2] = {0x00, 0x80}; /* Busy bit set */
    size_t rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x10010502,
                                                busy, 2, false);
    rx[rx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    SYN_PT_Status status = syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val);
    TEST_ASSERT_EQUAL(PT_EXITED, status);

    /* 2. SDO task timeout test */
    PT_INIT(&pt);
    uint32_t sdo_val = 0;
    size_t out_len = 0;
    syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x6040, 0, &sdo_val, 4,
                                  &out_len); /* step 1 write req */
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, tx_len);

    syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x6040, 0, &sdo_val, 4,
                                  &out_len); /* step 2 poll start */
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    m.poll_retries = 1;

    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t not_ready[1] = {0x00}; /* SM1 not ready */
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x1001080D,
                                         not_ready, 1, false);
    rx[rx_len - 2] = 1;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    status = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x6040, 0, &sdo_val, 4, &out_len);
    TEST_ASSERT_EQUAL(PT_EXITED, status);
}

void test_ecat_null_and_bounds_coverage(void)
{
    uint8_t buf[64] = {0};
    size_t offset = 0;
    SYN_EcatDatagramResult res;

    syn_ecat_frame_begin(NULL, 64);
    syn_ecat_frame_begin(buf, 1);
    TEST_ASSERT_EQUAL_size_t(
        0, syn_ecat_frame_add_datagram(NULL, 64, SYN_ECAT_CMD_NOP, 0, 0, NULL, 0, false));
    TEST_ASSERT_EQUAL_size_t(
        0, syn_ecat_frame_add_datagram(buf, 10, SYN_ECAT_CMD_NOP, 0, 0, NULL, 0, false));

    syn_ecat_frame_begin(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_size_t(
        0, syn_ecat_frame_add_datagram(buf, 14, SYN_ECAT_CMD_NOP, 0, 0, buf, 100, false));

    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_frame_finalize(NULL));

    TEST_ASSERT_FALSE(syn_ecat_frame_parse_next(NULL, 64, &offset, &res));
    TEST_ASSERT_FALSE(syn_ecat_frame_parse_next(buf, 10, &offset, &res));

    buf[0] = 0x00;
    buf[1] = 0x10;
    offset = 2;
    TEST_ASSERT_FALSE(syn_ecat_frame_parse_next(buf, 64, &offset, &res));

    syn_ecat_frame_begin(buf, sizeof(buf));
    buf[0] = 0x20;
    buf[1] = 0x00;
    offset = 2;
    TEST_ASSERT_FALSE(syn_ecat_frame_parse_next(buf, 14, &offset, &res));

    offset = 1;
    TEST_ASSERT_FALSE(syn_ecat_frame_parse_next(buf, 64, &offset, &res));
    offset = 100;
    TEST_ASSERT_FALSE(syn_ecat_frame_parse_next(buf, 64, &offset, &res));

    syn_ecat_frame_begin(buf, sizeof(buf));
    buf[0] = 0x05;
    buf[1] = 0x00;
    offset = 2;
    TEST_ASSERT_FALSE(syn_ecat_frame_parse_next(buf, 64, &offset, &res));

    /* Null & size checks for scan_bus, assign_addr, read_sii */
    SYN_EcatMaster m;
    uint32_t val = 0;
    uint8_t tx_cov[128] = {0};
    uint8_t rx_cov[128] = {0};
    syn_ecat_master_init(&m, tx_cov, sizeof(tx_cov), rx_cov, sizeof(rx_cov), buf, sizeof(buf), buf,
                         sizeof(buf));

    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_encode_scan_bus(NULL));
    TEST_ASSERT_EQUAL_UINT8(0, syn_ecat_decode_scan_bus(NULL, 64));
    TEST_ASSERT_EQUAL_UINT8(0, syn_ecat_decode_scan_bus(&m, 10));

    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_encode_assign_addr(NULL, 0, 0));
    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_encode_read_sii(NULL, 0, 0));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_ecat_decode_read_sii(NULL, 64, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_ecat_decode_read_sii(&m, 10, &val));

    /* Multiple datagram chain traversal (3 datagrams) */
    syn_ecat_frame_begin(buf, sizeof(buf));
    syn_ecat_frame_add_datagram(buf, sizeof(buf), SYN_ECAT_CMD_NOP, 0, 0, NULL, 0, false);
    syn_ecat_frame_add_datagram(buf, sizeof(buf), SYN_ECAT_CMD_NOP, 1, 0, NULL, 0, false);
    syn_ecat_frame_add_datagram(buf, sizeof(buf), SYN_ECAT_CMD_NOP, 2, 0, NULL, 0, false);

    /* Datagram length exceeds total datagram boundary */
    syn_ecat_frame_begin(buf, sizeof(buf));
    buf[0] = 0x0E;
    buf[1] = 0x00;
    buf[2] = SYN_ECAT_CMD_NOP;
    buf[3] = 0x00;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0x64;
    buf[9] = 0x00;
    offset = 2;
    TEST_ASSERT_FALSE(syn_ecat_frame_parse_next(buf, 64, &offset, &res));

    /* Null & parameter bounds checks for SM, FMMU, AL status, CoE SDO write */
    SYN_EcatSMConfig sm_cfg = {0};
    SYN_EcatFMMUConfig fmmu_cfg = {0};
    SYN_EcatState state;
    uint16_t status_code;

    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_encode_write_sm(NULL, 0, 0, NULL));
    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_encode_write_sm(&m, 0, 5, &sm_cfg));
    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_encode_write_fmmu(NULL, 0, 0, NULL));
    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_encode_write_fmmu(&m, 0, 5, &fmmu_cfg));
    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_encode_read_al_status(NULL, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_ecat_decode_read_al_status(NULL, 64, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_ecat_decode_read_al_status(&m, 10, &state, &status_code));
    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_encode_coe_sdo_write(NULL, 0, 0, 0, NULL, 5));

    /* Additional coverage for lines 154, 162, 172, 388, 394, 446, 524, 570, 601, 611, 630, 638 */
    /* Line 154: frame_len < 2 + total_dg_len */
    syn_ecat_frame_begin(buf, sizeof(buf));
    buf[0] = 0x14;
    buf[1] = 0x00;
    offset = 2;
    TEST_ASSERT_FALSE(syn_ecat_frame_parse_next(buf, 10, &offset, &res));

    /* Line 162: *offset + 12 > 2 + total_dg_len */
    syn_ecat_frame_begin(buf, sizeof(buf));
    buf[0] = 0x05;
    buf[1] = 0x10;
    offset = 2;
    TEST_ASSERT_FALSE(syn_ecat_frame_parse_next(buf, 20, &offset, &res));

    /* Line 172: *offset + 12 + data_len > 2 + total_dg_len */
    syn_ecat_frame_begin(buf, sizeof(buf));
    buf[0] = 0x0F;
    buf[1] = 0x10;
    buf[2] = SYN_ECAT_CMD_NOP;
    buf[8] = 0x0A;
    buf[9] = 0x00;
    offset = 2;
    TEST_ASSERT_FALSE(syn_ecat_frame_parse_next(buf, 30, &offset, &res));

    /* Line 388: wkc > SYN_ECAT_MAX_SLAVES cap in scan_bus */
    syn_ecat_frame_begin(buf, sizeof(buf));
    syn_ecat_frame_add_datagram(buf, sizeof(buf), SYN_ECAT_CMD_BRD, 0x01, SYN_ESC_REG_TYPE, buf, 2,
                                false);
    buf[14] = 0xFF;
    buf[15] = 0x00;
    size_t scan_len = syn_ecat_frame_finalize(buf);
    syn_ecat_master_set_rx_frame(&m, buf, scan_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_ECAT_MAX_SLAVES, syn_ecat_decode_scan_bus(&m, scan_len));

    /* Line 394: decode_scan_bus error return */
    TEST_ASSERT_EQUAL_UINT8(0, syn_ecat_decode_scan_bus(&m, 14));

    /* Line 446: decode_read_sii error return */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_ecat_decode_read_sii(&m, 14, &val));

    /* Line 524: decode_read_al_status error return */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_ecat_decode_read_al_status(&m, 14, &state, &status_code));

    /* Line 570: encode_coe_sdo_read null check */
    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_encode_coe_sdo_read(NULL, 0, 0, 0));

    /* Line 601, 611, 630: decode_coe_sdo_response error branches */
    size_t out_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_ecat_decode_coe_sdo_response(NULL, 0, NULL, 0, NULL));

    /* mbox_type != 3 */
    uint8_t sdo_payload[16] = {0};
    sdo_payload[5] = 0x01;
    syn_ecat_frame_begin(buf, sizeof(buf));
    syn_ecat_frame_add_datagram(buf, sizeof(buf), SYN_ECAT_CMD_FPRD, 0x01, 0x10011080, sdo_payload,
                                16, false);
    size_t sdo_len = syn_ecat_frame_finalize(buf);
    buf[sdo_len - 2U] = 1U;
    syn_ecat_master_set_rx_frame(&m, buf, sdo_len);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_ecat_decode_coe_sdo_response(&m, sdo_len, &val, 4, &out_len));

    /* sdo_hdr != 0x40 */
    sdo_payload[5] = 0x03;
    ecat_test_store16_le(&sdo_payload[6], (SYN_ECAT_COE_TYPE_SDO_RESP & 0x0FU) << 12U);
    sdo_payload[8] = 0x80;
    syn_ecat_frame_begin(buf, sizeof(buf));
    syn_ecat_frame_add_datagram(buf, sizeof(buf), SYN_ECAT_CMD_FPRD, 0x01, 0x10011080, sdo_payload,
                                16, false);
    sdo_len = syn_ecat_frame_finalize(buf);
    buf[sdo_len - 2U] = 1U;
    syn_ecat_master_set_rx_frame(&m, buf, sdo_len);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_ecat_decode_coe_sdo_response(&m, sdo_len, &val, 4, &out_len));

    /* Line 638: encode_cyclic null check */
    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_encode_cyclic(NULL));

    /* Lines 649, 663, 664, 668: decode_cyclic null & wkc error */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_ecat_decode_cyclic(NULL, 64));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_ecat_decode_cyclic(&m, 14));

    /* wkc < wkc_expected */
    m.wkc_expected = 1;
    syn_ecat_frame_begin(buf, sizeof(buf));
    syn_ecat_frame_add_datagram(buf, sizeof(buf), SYN_ECAT_CMD_LRW, 0x10, 0x10000, buf, 4, false);
    size_t cyc_len = syn_ecat_frame_finalize(buf);
    syn_ecat_master_set_rx_frame(&m, buf, cyc_len);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_ecat_decode_cyclic(&m, cyc_len));

    /* Lines 676, 691, 704: AL control & DC null checks */
    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_encode_set_al_control(NULL, 0, SYN_ECAT_STATE_INIT));
    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_encode_dc_read_system_time(NULL));
    TEST_ASSERT_EQUAL_size_t(0, syn_ecat_encode_dc_configure(NULL, 0, 0, 0));

    /* Lines 722, 734: set_rx_frame and pop_tx_frame null checks */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_ecat_master_set_rx_frame(NULL, NULL, 0));
    const uint8_t *dummy_ptr = NULL;
    size_t dummy_len = 0;
    TEST_ASSERT_FALSE(syn_ecat_master_pop_tx_frame(NULL, &dummy_ptr, &dummy_len));

    /* Line 777: get_slave_station_addr with slave_idx >= 32 */
    SYN_PT pt_trans;
    PT_INIT(&pt_trans);
    uint8_t trans_tx[256] = {0};
    uint8_t trans_rx[256] = {0};
    syn_ecat_master_init(&m, trans_tx, sizeof(trans_tx), trans_rx, sizeof(trans_rx), NULL, 0, NULL,
                         0);
    m.slave_count = 35;
    m.frame_rx_ready = true;
    while (syn_ecat_master_transition_task(&pt_trans, &m, SYN_ECAT_STATE_PREOP) == PT_IDLE) {
        syn_ecat_frame_begin(trans_rx, sizeof(trans_rx));
        uint8_t al_resp[4] = {0x02, 0x00, 0x00, 0x00};
        size_t tx_l = syn_ecat_frame_add_datagram(trans_rx, sizeof(trans_rx), SYN_ECAT_CMD_FPRD,
                                                  0x05, 0x10010130, al_resp, 4, false);
        trans_rx[tx_l - 2] = 0x01;
        syn_ecat_master_set_rx_frame(&m, trans_rx, tx_l);
        m.frame_rx_ready = true;
    }
}

static void test_ecat_discover_pdo_mapping_sdo_timeout(void)
{
    SYN_EcatMaster m;
    uint8_t tx[256];
    uint8_t rx[256];
    uint8_t out_img[16] = {0};
    uint8_t in_img[16] = {0};
    uint16_t rx_bytes = 99;
    uint16_t tx_bytes = 99;

    syn_ecat_master_init(&m, tx, sizeof(tx), rx, sizeof(rx), out_img, 2, in_img, 2);

    SYN_PT pt;
    PT_INIT(&pt);

    /* Start discover PDO mapping task */
    SYN_PT_Status status =
        syn_ecat_master_discover_pdo_mapping_task(&pt, &m, 0x1001, &rx_bytes, &tx_bytes);

    const uint8_t *tx_ptr = NULL;
    size_t tx_len = 0;
    int loop_guard = 0;

    /* Loop until task completes — force immediate timeout on each poll step */
    while (status == PT_IDLE && loop_guard++ < 50) {
        if (syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len)) {
            memcpy(rx, tx_ptr, tx_len);
            /* Clear payload so SM1 status byte is 0x00 (not ready) */
            if (tx_len > 14) {
                memset(&rx[12], 0, tx_len - 14);
            }
            rx[tx_len - 2] = 1;
            syn_ecat_master_set_rx_frame(&m, rx, tx_len);
            m.poll_retries = 1;
        }
        status = syn_ecat_master_discover_pdo_mapping_task(&pt, &m, 0x1001, &rx_bytes, &tx_bytes);
    }

    TEST_ASSERT_EQUAL(PT_EXITED, status);
    TEST_ASSERT_EQUAL_UINT16(0, rx_bytes);
    TEST_ASSERT_EQUAL_UINT16(0, tx_bytes);
}

static void run_pdo_discovery_abort_at_stage(uint16_t abort_idx, uint8_t abort_sub)
{
    SYN_EcatMaster m;
    uint8_t tx[256];
    uint8_t rx[256];
    uint8_t out_img[16] = {0};
    uint8_t in_img[16] = {0};
    uint16_t rx_bytes = 77;
    uint16_t tx_bytes = 88;

    syn_ecat_master_init(&m, tx, sizeof(tx), rx, sizeof(rx), out_img, 2, in_img, 2);

    SYN_PT pt;
    PT_INIT(&pt);

    SYN_PT_Status status =
        syn_ecat_master_discover_pdo_mapping_task(&pt, &m, 0x1001, &rx_bytes, &tx_bytes);

    const uint8_t *tx_ptr = NULL;
    size_t tx_len = 0;
    uint16_t req_idx = 0;
    uint8_t req_sub = 0;
    int guard = 0;

    while (status == PT_IDLE && guard++ < 200) {
        if (syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len)) {
            memcpy(rx, tx_ptr, tx_len);
            if (tx_len >= 24 && rx[2] == SYN_ECAT_CMD_FPWR &&
                ecat_test_load16_le(&rx[4]) == 0x1000) {
                req_idx = ecat_test_load16_le(&rx[21]);
                req_sub = rx[23];
            } else if (tx_len >= 15 && rx[2] == SYN_ECAT_CMD_FPRD &&
                       ecat_test_load16_le(&rx[4]) == 0x080D) {
                syn_ecat_frame_begin(rx, sizeof(rx));
                uint8_t ready[1] = {0x08};
                tx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01,
                                                     0x1001080D, ready, 1, false);
            } else if (tx_len >= 18 && rx[2] == SYN_ECAT_CMD_FPRD &&
                       ecat_test_load16_le(&rx[4]) == 0x1080) {
                uint8_t payload[16] = {0};
                ecat_test_store16_le(&payload[0], 0x000A);
                ecat_test_store16_le(&payload[2], 0x1001);
                payload[5] = 0x03;
                ecat_test_store16_le(&payload[6], (SYN_ECAT_COE_TYPE_SDO_RESP & 0x0F) << 12);
                ecat_test_store16_le(&payload[9], req_idx);
                payload[11] = req_sub;

                if (req_idx == abort_idx && req_sub == abort_sub) {
                    payload[8] = 0x80; /* SDO Abort Transfer -> sdo_ok == false */
                } else if (req_idx == 0x1C12 && req_sub == 0) {
                    payload[8] = 0x4F;
                    payload[12] = 0x01;
                } else if (req_idx == 0x1C12 && req_sub == 1) {
                    payload[8] = 0x47;
                    ecat_test_store16_le(&payload[12], 0x1600);
                } else if (req_idx == 0x1600 && req_sub == 0) {
                    payload[8] = 0x4F;
                    payload[12] = 0x01;
                } else if (req_idx == 0x1600 && req_sub == 1) {
                    payload[8] = 0x43;
                    ecat_test_store32_le(&payload[12], 0x60400010);
                } else if (req_idx == 0x1C13 && req_sub == 0) {
                    payload[8] = 0x4F;
                    payload[12] = 0x01;
                }
                syn_ecat_frame_begin(rx, sizeof(rx));
                tx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01,
                                                     0x10011080, payload, 16, false);
            }
            rx[tx_len - 2] = 0x01;
            syn_ecat_master_set_rx_frame(&m, rx, tx_len);
        }
        status = syn_ecat_master_discover_pdo_mapping_task(&pt, &m, 0x1001, &rx_bytes, &tx_bytes);
    }

    TEST_ASSERT_EQUAL(PT_EXITED, status);
    TEST_ASSERT_EQUAL_UINT16(0, rx_bytes);
    TEST_ASSERT_EQUAL_UINT16(0, tx_bytes);
}

static void test_ecat_cyclic_zero_frame_and_pdo_stage_failures(void)
{
    /* 1. syn_ecat_master_cyclic_task when syn_ecat_encode_cyclic returns 0 (syn_ethercat.c:857) */
    SYN_EcatMaster m = {0};
    uint8_t tx[64] = {0};
    uint8_t rx[64] = {0};
    uint8_t out_img[8] = {0};
    uint8_t in_img[8] = {0};
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ecat_master_init(&m, tx, sizeof(tx), rx, sizeof(rx), out_img, 0, in_img, 4));

    SYN_PT pt;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_cyclic_task(&pt, &m));
    TEST_ASSERT_FALSE(m.frame_tx_pending);

    /* 2. SDO failure at each remaining PDO discovery stage (syn_ethercat.c:1058, 1067, 1077, 1112)
     */
    run_pdo_discovery_abort_at_stage(0x1C12, 1); /* Fail at assign_idx subindex 1 (1058) */
    run_pdo_discovery_abort_at_stage(0x1600, 0); /* Fail at pdo_map_idx subindex 0 (1067) */
    run_pdo_discovery_abort_at_stage(0x1600, 1); /* Fail at pdo_map_idx subindex 1 (1077) */
    run_pdo_discovery_abort_at_stage(0x1C13, 0); /* Succeed RxPDO (>0 bits), fail TxPDO (1112) */
}

static void test_ecat_sii_sdo_decode_failure_and_3byte_sdo(void)
{
    SYN_EcatMaster m;
    uint8_t tx[256];
    uint8_t rx[256];
    uint8_t out_img[16] = {0};
    uint8_t in_img[16] = {0};

    syn_ecat_master_init(&m, tx, sizeof(tx), rx, sizeof(rx), out_img, 2, in_img, 2);

    const uint8_t *tx_ptr = NULL;
    size_t tx_len = 0;

    /* 1. SII read task final register (0x0508) decode failure (WKC = 0) -> PT_EXITED */
    SYN_PT pt;
    PT_INIT(&pt);
    uint32_t sii_val = 0xDEADBEEFU;

    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val));
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    (void)memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2U] = 1U;
    syn_ecat_master_set_rx_frame(&m, rx, tx_len);

    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val));
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    (void)memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2U] = 1U;
    syn_ecat_master_set_rx_frame(&m, rx, tx_len);

    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val));
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t idle[2] = {0x00U, 0x00U};
    size_t rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x10010502,
                                                idle, 2, false);
    rx[rx_len - 2U] = 1U;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val));
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t bad_sii[4] = {0x11U, 0x22U, 0x33U, 0x44U};
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x10010508,
                                         bad_sii, 4, false);
    rx[rx_len - 2U] = 0U; /* WKC = 0 -> decode failure */
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_read_sii_task(&pt, &m, 0x1001, 0x08, &sii_val));
    TEST_ASSERT_FALSE(m.sdo_ok);
    TEST_ASSERT_EQUAL_HEX32(0xDEADBEEFU, sii_val);

    /* 2. SDO read task final decode failure (SDO Abort 0x80) -> PT_EXITED */
    PT_INIT(&pt);
    uint32_t sdo_out = 0xCAFEBABEU;
    size_t out_len = 99U;

    SYN_PT_Status st;
    st = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x2000, 0, &sdo_out, 4, &out_len);
    TEST_ASSERT_EQUAL(PT_IDLE, st);
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    (void)memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2U] = 1U;
    syn_ecat_master_set_rx_frame(&m, rx, tx_len);

    st = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x2000, 0, &sdo_out, 4, &out_len);
    TEST_ASSERT_EQUAL(PT_IDLE, st);
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t sm1_ready[1] = {0x08U};
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x1001080D,
                                         sm1_ready, 1, false);
    rx[rx_len - 2U] = 1U;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    st = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x2000, 0, &sdo_out, 4, &out_len);
    TEST_ASSERT_EQUAL(PT_IDLE, st);
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    uint8_t abort_pld[16] = {0};
    ecat_test_store16_le(&abort_pld[0], 0x000AU);
    ecat_test_store16_le(&abort_pld[2], 0x1001U);
    abort_pld[5] = 0x03U;
    ecat_test_store16_le(&abort_pld[6], (SYN_ECAT_COE_TYPE_SDO_RESP & 0x0FU) << 12U);
    abort_pld[8] = 0x80U; /* SDO Abort */
    syn_ecat_frame_begin(rx, sizeof(rx));
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x10011080,
                                         abort_pld, 16, false);
    rx[rx_len - 2U] = 1U;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    st = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x2000, 0, &sdo_out, 4, &out_len);
    TEST_ASSERT_EQUAL(PT_EXITED, st);
    TEST_ASSERT_FALSE(m.sdo_ok);
    TEST_ASSERT_EQUAL_HEX32(0xCAFEBABEU, sdo_out);

    /* 3. 3-byte expedited SDO read (0x47 -> n=1 -> 3 bytes) into max_len=3 */
    uint8_t sdo3_buf[4] = {0x00U, 0x00U, 0x00U, 0xEEU};
    out_len = 0U;
    PT_INIT(&pt);
    st = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x2001, 0, sdo3_buf, 3, &out_len);
    TEST_ASSERT_EQUAL(PT_IDLE, st);
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    (void)memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2U] = 1U;
    syn_ecat_master_set_rx_frame(&m, rx, tx_len);

    st = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x2001, 0, sdo3_buf, 3, &out_len);
    TEST_ASSERT_EQUAL(PT_IDLE, st);
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    syn_ecat_frame_begin(rx, sizeof(rx));
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x1001080D,
                                         sm1_ready, 1, false);
    rx[rx_len - 2U] = 1U;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    st = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x2001, 0, sdo3_buf, 3, &out_len);
    TEST_ASSERT_EQUAL(PT_IDLE, st);
    syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len);
    uint8_t pld3[16] = {0};
    ecat_test_store16_le(&pld3[0], 0x000AU);
    ecat_test_store16_le(&pld3[2], 0x1001U);
    pld3[5] = 0x03U;
    ecat_test_store16_le(&pld3[6], (SYN_ECAT_COE_TYPE_SDO_RESP & 0x0FU) << 12U);
    pld3[8] = 0x47U; /* n = 1 -> 4 - 1 = 3 bytes */
    ecat_test_store16_le(&pld3[9], 0x2001U);
    pld3[12] = 0xAAU;
    pld3[13] = 0xBBU;
    pld3[14] = 0xCCU;
    pld3[15] = 0xDDU;
    syn_ecat_frame_begin(rx, sizeof(rx));
    rx_len = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 0x01, 0x10011080, pld3,
                                         16, false);
    rx[rx_len - 2U] = 1U;
    syn_ecat_master_set_rx_frame(&m, rx, rx_len);

    st = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001, 0x2001, 0, sdo3_buf, 3, &out_len);
    TEST_ASSERT_EQUAL(PT_ENDED, st);
    TEST_ASSERT_TRUE(m.sdo_ok);
    TEST_ASSERT_EQUAL_size_t(3U, out_len);
    TEST_ASSERT_EQUAL_HEX8(0xAAU, sdo3_buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0xBBU, sdo3_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0xCCU, sdo3_buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0xEEU, sdo3_buf[3]);
}

static void ecat_test_feed_ack(SYN_EcatMaster *m, uint8_t *rx_buf)
{
    const uint8_t *tx_ptr = NULL;
    size_t tx_len = 0U;
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(m, &tx_ptr, &tx_len));
    (void)memcpy(rx_buf, tx_ptr, tx_len);
    rx_buf[tx_len - 2U] = 1U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(m, rx_buf, tx_len));
}

static void test_ecat_r4_p09_regressions(void)
{
    /* 1. 11-bit datagram PDU length overflow & NULL data zero-fill */
    uint8_t big_buf[2400];
    (void)memset(big_buf, 0xAA, sizeof(big_buf));
    syn_ecat_frame_begin(big_buf, sizeof(big_buf));

    TEST_ASSERT_EQUAL_size_t(0U, syn_ecat_frame_add_datagram(big_buf, sizeof(big_buf),
                                                             SYN_ECAT_CMD_FPRD, 1U, 0U, NULL,
                                                             0x0800U, false));
    size_t sz1 = syn_ecat_frame_add_datagram(big_buf, sizeof(big_buf), SYN_ECAT_CMD_FPRD, 1U, 0U,
                                             NULL, 1020U, false);
    TEST_ASSERT_EQUAL_size_t(1034U, sz1);
    for (size_t i = 0U; i < 1020U; i++) {
        TEST_ASSERT_EQUAL_HEX8(0x00U, big_buf[12U + i]);
    }
    /* Second 1020B datagram would exceed 11-bit total_dg_bytes (1032 + 1032 > 2047) */
    TEST_ASSERT_EQUAL_size_t(0U, syn_ecat_frame_add_datagram(big_buf, sizeof(big_buf),
                                                             SYN_ECAT_CMD_FPRD, 2U, 0U, NULL, 1020U,
                                                             false));

    /* syn_ecat_build_datagram_frame 11-bit overflow & NULL payload zero-fill */
    SYN_EcatDatagram dg = {.cmd = SYN_ECAT_CMD_FPRD, .idx = 1U, .addr = 0x10010000U};
    (void)memset(big_buf, 0xCC, 64U);
    TEST_ASSERT_EQUAL_size_t(
        0U, syn_ecat_build_datagram_frame(big_buf, sizeof(big_buf), &dg, NULL, 2040U));
    TEST_ASSERT_EQUAL_size_t(22U, syn_ecat_build_datagram_frame(big_buf, 64U, &dg, NULL, 8U));
    for (size_t i = 0U; i < 8U; i++) {
        TEST_ASSERT_EQUAL_HEX8(0x00U, big_buf[12U + i]);
    }

    /* 2. SDO download NULL / 0-length / >4B rejection & 128B mailbox read/write */
    uint8_t mbox[32];
    uint32_t sdo_v = 0x11223344U;
    TEST_ASSERT_EQUAL_size_t(
        0U, syn_ecat_coe_encode_sdo_download(mbox, sizeof(mbox), 0x6040U, 0U, NULL, 4U));
    TEST_ASSERT_EQUAL_size_t(
        0U, syn_ecat_coe_encode_sdo_download(mbox, sizeof(mbox), 0x6040U, 0U, &sdo_v, 0U));

    SYN_EcatMaster m;
    uint8_t tx[256] = {0};
    uint8_t rx[256] = {0};
    uint8_t img[8] = {0};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ecat_master_init(&m, tx, sizeof(tx), rx, sizeof(rx), img, 4U, img, 4U));
    TEST_ASSERT_EQUAL_size_t(0U, syn_ecat_encode_coe_sdo_write(&m, 0x1001U, 0x6040U, 0U, NULL, 4U));
    TEST_ASSERT_EQUAL_size_t(0U,
                             syn_ecat_encode_coe_sdo_write(&m, 0x1001U, 0x6040U, 0U, &sdo_v, 0U));
    TEST_ASSERT_EQUAL_size_t(0U,
                             syn_ecat_encode_coe_sdo_write(&m, 0x1001U, 0x6040U, 0U, &sdo_v, 5U));

    /* syn_ecat_encode_read_reg supports full 128B mailbox without 16B truncation */
    TEST_ASSERT_EQUAL_size_t(14U + SYN_ECAT_MBOX_SIZE,
                             syn_ecat_encode_read_reg(&m, 0x1001U, 0x1080U, SYN_ECAT_MBOX_SIZE));

    /* 3. Guard tx_frame_len == 0 across all protothread tasks (no infinite PT_IDLE hang) */
    SYN_PT pt;

    /* Scan task site 1 & site 2 */
    PT_INIT(&pt);
    m.tx_buf_size = 10U;
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_scan_task(&pt, &m));
    TEST_ASSERT_FALSE(m.frame_tx_pending);

    PT_INIT(&pt);
    m.tx_buf_size = sizeof(tx);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_scan_task(&pt, &m));
    ecat_test_feed_ack(&m, rx);
    m.tx_buf_size = 10U;
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_scan_task(&pt, &m));

    /* Transition task sites 1..4 */
    m.slave_count = 1U;
    PT_INIT(&pt);
    m.tx_buf_size = 10U;
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_transition_task(&pt, &m, SYN_ECAT_STATE_PREOP));

    PT_INIT(&pt);
    m.tx_buf_size = sizeof(tx);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_transition_task(&pt, &m, SYN_ECAT_STATE_PREOP));
    ecat_test_feed_ack(&m, rx);
    m.tx_buf_size = 10U;
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_transition_task(&pt, &m, SYN_ECAT_STATE_PREOP));

    PT_INIT(&pt);
    m.tx_buf_size = 10U;
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_transition_task(&pt, &m, SYN_ECAT_STATE_SAFEOP));

    PT_INIT(&pt);
    m.tx_buf_size = sizeof(tx);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_transition_task(&pt, &m, SYN_ECAT_STATE_SAFEOP));
    ecat_test_feed_ack(&m, rx);
    m.tx_buf_size = 10U;
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_transition_task(&pt, &m, SYN_ECAT_STATE_SAFEOP));

    /* Read SII task sites 1..4 + poll decode error branch */
    uint32_t sii_out = 0U;
    PT_INIT(&pt);
    m.tx_buf_size = 10U;
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_read_sii_task(&pt, &m, 0x1001U, 8U, &sii_out));

    PT_INIT(&pt);
    m.tx_buf_size = sizeof(tx);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_read_sii_task(&pt, &m, 0x1001U, 8U, &sii_out));
    ecat_test_feed_ack(&m, rx);
    m.tx_buf_size = 10U;
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_read_sii_task(&pt, &m, 0x1001U, 8U, &sii_out));

    PT_INIT(&pt);
    m.tx_buf_size = sizeof(tx);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_read_sii_task(&pt, &m, 0x1001U, 8U, &sii_out));
    ecat_test_feed_ack(&m, rx);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_read_sii_task(&pt, &m, 0x1001U, 8U, &sii_out));
    ecat_test_feed_ack(&m, rx);
    m.tx_buf_size = 10U;
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_read_sii_task(&pt, &m, 0x1001U, 8U, &sii_out));

    PT_INIT(&pt);
    m.tx_buf_size = sizeof(tx);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_read_sii_task(&pt, &m, 0x1001U, 8U, &sii_out));
    ecat_test_feed_ack(&m, rx);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_read_sii_task(&pt, &m, 0x1001U, 8U, &sii_out));
    ecat_test_feed_ack(&m, rx);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_read_sii_task(&pt, &m, 0x1001U, 8U, &sii_out));
    /* First poll reply has WKC=0 (decode error in ecat_sii_poll_done), second is idle */
    const uint8_t *tx_ptr = NULL;
    size_t tx_len = 0U;
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len));
    (void)memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2U] = 0U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(&m, rx, tx_len));
    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_read_sii_task(&pt, &m, 0x1001U, 8U, &sii_out));
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len));
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t idle2[2] = {0U, 0U};
    size_t rlen = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 1U, 0x10010502U,
                                              idle2, 2U, false);
    rx[rlen - 2U] = 1U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(&m, rx, rlen));
    m.tx_buf_size = 10U;
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_read_sii_task(&pt, &m, 0x1001U, 8U, &sii_out));

    /* SDO read task sites 1..3 (including 64B tx_buf_size < 142B mailbox frame) + poll WKC=0 */
    uint32_t sdo_out = 0U;
    size_t out_len = 0U;
    PT_INIT(&pt);
    m.tx_buf_size = 64U;
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_sdo_read_task(&pt, &m, 0x1001U, 0x6041U, 0U,
                                                               &sdo_out, 4U, &out_len));
    TEST_ASSERT_FALSE(m.frame_tx_pending);

    PT_INIT(&pt);
    m.tx_buf_size = sizeof(tx);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_sdo_read_task(&pt, &m, 0x1001U, 0x6041U, 0U,
                                                             &sdo_out, 4U, &out_len));
    ecat_test_feed_ack(&m, rx);
    m.tx_buf_size = 10U;
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_sdo_read_task(&pt, &m, 0x1001U, 0x6041U, 0U,
                                                               &sdo_out, 4U, &out_len));

    PT_INIT(&pt);
    m.tx_buf_size = sizeof(tx);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_sdo_read_task(&pt, &m, 0x1001U, 0x6041U, 0U,
                                                             &sdo_out, 4U, &out_len));
    ecat_test_feed_ack(&m, rx);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_sdo_read_task(&pt, &m, 0x1001U, 0x6041U, 0U,
                                                             &sdo_out, 4U, &out_len));
    /* First poll reply has WKC=0 (decode error in ecat_sm1_poll_ready), second has 0x08 ready */
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len));
    (void)memcpy(rx, tx_ptr, tx_len);
    rx[tx_len - 2U] = 0U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(&m, rx, tx_len));
    TEST_ASSERT_EQUAL(PT_IDLE, syn_ecat_master_sdo_read_task(&pt, &m, 0x1001U, 0x6041U, 0U,
                                                             &sdo_out, 4U, &out_len));
    TEST_ASSERT_TRUE(syn_ecat_master_pop_tx_frame(&m, &tx_ptr, &tx_len));
    syn_ecat_frame_begin(rx, sizeof(rx));
    uint8_t sm1_full[1] = {0x08U};
    rlen = syn_ecat_frame_add_datagram(rx, sizeof(rx), SYN_ECAT_CMD_FPRD, 1U, 0x1001080DU, sm1_full,
                                       1U, false);
    rx[rlen - 2U] = 1U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(&m, rx, rlen));
    m.tx_buf_size = 64U;
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ecat_master_sdo_read_task(&pt, &m, 0x1001U, 0x6041U, 0U,
                                                               &sdo_out, 4U, &out_len));
}

static void ecat_loopback_step(SYN_EcatMaster *m, SYN_EcatNode *slave, uint8_t *wire)
{
    const uint8_t *tx_ptr = NULL;
    size_t tx_len = 0U;
    if (syn_ecat_master_pop_tx_frame(m, &tx_ptr, &tx_len)) {
        uint16_t wkc = 0U;
        (void)memcpy(wire, tx_ptr, tx_len);
        (void)syn_ecat_slave_process_frame(slave, wire, tx_len, &wkc);
        TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(m, wire, tx_len));
    }
}

static void test_r5_a07_master_slave_loopback_and_wkc_sdo_guards(void)
{
    uint16_t ctrl_word = 0U;
    uint32_t status_val = 0xCAFE1234U;
    const SYN_CANOpenODEntry od_table[] = {
        {0x6040U, 0U, SYN_CANOPEN_TYPE_U16, SYN_CANOPEN_ACCESS_RW, &ctrl_word, sizeof(ctrl_word)},
        {0x6041U, 0U, SYN_CANOPEN_TYPE_U32, SYN_CANOPEN_ACCESS_RO, &status_val, sizeof(status_val)},
    };
    SYN_CANOpenNode od_node;
    SYN_CANOpenNodeConfig co_cfg = {.node_id = 1U, .heartbeat_ms = 0U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_init(&od_node, &co_cfg, od_table, 2U));

    SYN_EcatNode slave;
    syn_ecat_init(&slave, 0x0000U, &od_node);
    slave.sii_words[8] = 0xDEADBEEFU;
    slave.al_status = 0x001BU;
    slave.pdo_in[0] = 0x5AU;
    slave.pdo_in[1] = 0xA5U;
    slave.pdo_in[2] = 0x3CU;
    slave.pdo_in[3] = 0xC3U;

    SYN_EcatMaster m;
    uint8_t tx[256] = {0};
    uint8_t rx[256] = {0};
    uint8_t wire[256] = {0};
    uint8_t out_img[4] = {0x11U, 0x22U, 0x33U, 0x44U};
    uint8_t in_img[4] = {0};
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ecat_master_init(&m, tx, sizeof(tx), rx, sizeof(rx), out_img, 4U, in_img, 4U));

    /* 1. Unanswered WKC == 0 frames MUST fail across all master decoders */
    uint32_t sii_out = 0U;
    SYN_EcatState al_st = SYN_ECAT_STATE_NONE;
    uint16_t al_code = 0U;
    size_t sdo_out_len = 0U;
    uint32_t sdo_val = 0U;

    size_t flen = syn_ecat_encode_read_sii(&m, 0x1001U, 8U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(&m, tx, flen));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_decode_read_sii(&m, flen, &sii_out));

    flen = syn_ecat_encode_read_al_status(&m, 0x1001U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(&m, tx, flen));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_decode_read_al_status(&m, flen, &al_st, &al_code));

    /* Unanswered or echoed SDO upload request (CoE type = SDO_REQ=2, sdo_hdr = 0x40) rejected */
    flen = syn_ecat_encode_coe_sdo_read(&m, 0x1001U, 0x6041U, 0U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(&m, tx, flen));
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_ecat_decode_coe_sdo_response(&m, flen, &sdo_val, 4U, &sdo_out_len));
    (void)memcpy(wire, tx, flen);
    wire[flen - 2U] = 1U; /* Even with WKC=1, echoed SDO_REQ (coe_type == 2) MUST fail */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(&m, wire, flen));
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_ecat_decode_coe_sdo_response(&m, flen, &sdo_val, 4U, &sdo_out_len));

    /* CoE SDO_RESP with non-expedited sdo_hdr == 0x40 rejected, 0x42 (s=0 expedited) accepted */
    wire[18] = 0x00U;
    wire[19] = (uint8_t)((SYN_ECAT_COE_TYPE_SDO_RESP & 0x0FU) << 4U);
    wire[20] = 0x40U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(&m, wire, flen));
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_ecat_decode_coe_sdo_response(&m, flen, &sdo_val, 4U, &sdo_out_len));
    wire[20] = 0x42U;
    ecat_test_store32_le(&wire[24], 0x13572468U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(&m, wire, flen));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ecat_decode_coe_sdo_response(&m, flen, &sdo_val, 4U, &sdo_out_len));
    TEST_ASSERT_EQUAL_HEX32(0x13572468U, sdo_val);

    /* Unanswered cyclic frame with WKC=0 fails even if wkc_expected == 0 */
    m.wkc_expected = 0U;
    flen = syn_ecat_encode_cyclic(&m);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_master_set_rx_frame(&m, tx, flen));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_decode_cyclic(&m, flen));

    /* 2. Full Master <-> Slave Protothread Loopback: Scan -> Assign 0x1001 -> PREOP -> SII -> SDO
     */
    SYN_PT pt;
    PT_INIT(&pt);
    while (syn_ecat_master_scan_task(&pt, &m) == PT_IDLE) {
        ecat_loopback_step(&m, &slave, wire);
    }
    TEST_ASSERT_EQUAL_UINT8(1U, m.slave_count);
    TEST_ASSERT_EQUAL_HEX16(0x1001U, slave.station_addr);

    PT_INIT(&pt);
    while (syn_ecat_master_transition_task(&pt, &m, SYN_ECAT_STATE_PREOP) == PT_IDLE) {
        ecat_loopback_step(&m, &slave, wire);
    }
    TEST_ASSERT_EQUAL(SYN_ECAT_STATE_PREOP, slave.state);
    TEST_ASSERT_EQUAL_HEX16(0x1000U, slave.sm[0].start_addr);
    TEST_ASSERT_EQUAL_HEX16(0x1080U, slave.sm[1].start_addr);

    /* SII Read via 6-byte FPWR to 0x0502 + FPRD 0x0508 AND via protothread task */
    flen = syn_ecat_encode_read_sii(&m, 0x1001U, 8U);
    uint16_t wkc = 0U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_slave_process_frame(&slave, tx, flen, &wkc));
    PT_INIT(&pt);
    while (syn_ecat_master_read_sii_task(&pt, &m, 0x1001U, 8U, &sii_out) == PT_IDLE) {
        ecat_loopback_step(&m, &slave, wire);
    }
    TEST_ASSERT_EQUAL_HEX32(0xDEADBEEFU, sii_out);

    /* CoE SDO Download (0x6040 = 0x000F) + Upload (0x6040 and 0x6041) + Abort on unknown 0x9999 */
    uint16_t wr_cw = 0x000FU;
    flen = syn_ecat_encode_coe_sdo_write(&m, 0x1001U, 0x6040U, 0U, &wr_cw, 2U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_slave_process_frame(&slave, tx, flen, &wkc));
    TEST_ASSERT_EQUAL_HEX16(0x000FU, ctrl_word);
    TEST_ASSERT_TRUE(slave.mbox_in_full);
    /* Drain download response from SM1 */
    flen = syn_ecat_encode_read_reg(&m, 0x1001U, 0x1080U, SYN_ECAT_MBOX_SIZE);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_slave_process_frame(&slave, tx, flen, &wkc));
    TEST_ASSERT_FALSE(slave.mbox_in_full);

    PT_INIT(&pt);
    sdo_val = 0U;
    while (syn_ecat_master_sdo_read_task(&pt, &m, 0x1001U, 0x6041U, 0U, &sdo_val, 4U,
                                         &sdo_out_len) == PT_IDLE) {
        ecat_loopback_step(&m, &slave, wire);
    }
    TEST_ASSERT_EQUAL_HEX32(0xCAFE1234U, sdo_val);

    /* Unknown OD index triggers slave SDO Abort (0x80) and master PT_EXITED */
    PT_INIT(&pt);
    SYN_PT_Status st = PT_IDLE;
    while (st == PT_IDLE) {
        ecat_loopback_step(&m, &slave, wire);
        st = syn_ecat_master_sdo_read_task(&pt, &m, 0x1001U, 0x9999U, 0U, &sdo_val, 4U,
                                           &sdo_out_len);
    }
    TEST_ASSERT_EQUAL(PT_EXITED, st);

    /* Non-CoE mailbox write ignored cleanly */
    flen = syn_ecat_encode_coe_sdo_read(&m, 0x1001U, 0x6041U, 0U);
    tx[17] = 0x01U; /* Mailbox type != 3 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_slave_process_frame(&slave, tx, flen, &wkc));
    TEST_ASSERT_FALSE(slave.mbox_in_full);

    /* 3. FMMU Configuration + SAFEOP/OP + Cyclic LRW / LRD / LWR + APRW / FPRW / BRW */
    SYN_EcatFMMUConfig fmmu_out = {.logical_start = 0x00010000U,
                                   .length = 4U,
                                   .logical_end_bit = 7U,
                                   .phys_start_addr = 0x1100U,
                                   .type = 0x02U,
                                   .enable = true};
    SYN_EcatFMMUConfig fmmu_in = {.logical_start = 0x00010000U,
                                  .length = 4U,
                                  .logical_end_bit = 7U,
                                  .phys_start_addr = 0x1140U,
                                  .type = 0x01U,
                                  .enable = true};
    SYN_EcatFMMUConfig fmmu_non_overlap = {.logical_start = 0x00020000U,
                                           .length = 4U,
                                           .logical_end_bit = 7U,
                                           .phys_start_addr = 0x1110U,
                                           .type = 0x03U,
                                           .enable = true};
    flen = syn_ecat_encode_write_fmmu(&m, 0x1001U, 0U, &fmmu_out);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_slave_process_frame(&slave, tx, flen, &wkc));
    flen = syn_ecat_encode_write_fmmu(&m, 0x1001U, 1U, &fmmu_in);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_slave_process_frame(&slave, tx, flen, &wkc));
    flen = syn_ecat_encode_write_fmmu(&m, 0x1001U, 2U, &fmmu_non_overlap);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_slave_process_frame(&slave, tx, flen, &wkc));

    PT_INIT(&pt);
    while (syn_ecat_master_transition_task(&pt, &m, SYN_ECAT_STATE_OP) == PT_IDLE) {
        ecat_loopback_step(&m, &slave, wire);
    }
    TEST_ASSERT_EQUAL(SYN_ECAT_STATE_OP, slave.state);

    m.logical_addr = 0x00010000U;
    m.wkc_expected = 3U;
    PT_INIT(&pt);
    while (syn_ecat_master_cyclic_task(&pt, &m) == PT_IDLE) {
        ecat_loopback_step(&m, &slave, wire);
    }
    TEST_ASSERT_EQUAL_UINT16(3U, m.wkc_last);
    TEST_ASSERT_EQUAL_HEX8(0x11U, slave.pdo_out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x44U, slave.pdo_out[3]);
    TEST_ASSERT_EQUAL_HEX8(0x5AU, in_img[0]);
    TEST_ASSERT_EQUAL_HEX8(0xC3U, in_img[3]);

    /* Exercise LRD, LWR, APRD, APRW, multi-byte FPRW, BRW, NOP, low-reg reads, & truncated dgrams
     */
    syn_ecat_frame_begin(wire, sizeof(wire));
    uint8_t tmp4[4] = {0x99U, 0x88U, 0x77U, 0x66U};
    uint8_t al_rw[2] = {0x04U, 0x00U};
    (void)syn_ecat_frame_add_datagram(wire, sizeof(wire), SYN_ECAT_CMD_LWR, 1U, 0x00010000U, tmp4,
                                      4U, false);
    (void)syn_ecat_frame_add_datagram(wire, sizeof(wire), SYN_ECAT_CMD_LRD, 2U, 0x00010000U, NULL,
                                      4U, false);
    (void)syn_ecat_frame_add_datagram(wire, sizeof(wire), SYN_ECAT_CMD_APRD, 3U, 0x00000010U, NULL,
                                      2U, false);
    (void)syn_ecat_frame_add_datagram(wire, sizeof(wire), SYN_ECAT_CMD_APRD, 4U, 0x00010010U, NULL,
                                      2U, false);
    (void)syn_ecat_frame_add_datagram(wire, sizeof(wire), SYN_ECAT_CMD_APRW, 5U, 0x00001100U, tmp4,
                                      2U, false);
    (void)syn_ecat_frame_add_datagram(wire, sizeof(wire), SYN_ECAT_CMD_FPRW, 6U, 0x10010120U, al_rw,
                                      2U, false);
    (void)syn_ecat_frame_add_datagram(wire, sizeof(wire), SYN_ECAT_CMD_BRW, 7U, 0x00001200U, tmp4,
                                      2U, false);
    (void)syn_ecat_frame_add_datagram(wire, sizeof(wire), SYN_ECAT_CMD_NOP, 8U, 0U, NULL, 0U,
                                      false);
    flen = syn_ecat_frame_finalize(wire);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_slave_process_frame(&slave, wire, flen, &wkc));
    TEST_ASSERT_TRUE(wkc >= 10U);
    TEST_ASSERT_EQUAL(SYN_ECAT_STATE_SAFEOP, slave.state);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_parse_frame(&slave, wire, flen, &wkc));

    /* Read AL status/code & SII low regs & RxPDO DPRAM */
    flen = syn_ecat_encode_read_reg(&m, 0x1001U, 0x0134U, 2U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_slave_process_frame(&slave, tx, flen, &wkc));
    flen = syn_ecat_encode_read_reg(&m, 0x1001U, 0x1100U, 4U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ecat_slave_process_frame(&slave, tx, flen, &wkc));

    /* Truncated nested datagram bounds in syn_ecat_parse_frame and syn_ecat_slave_process_frame */
    syn_ecat_frame_begin(wire, sizeof(wire));
    (void)syn_ecat_frame_add_datagram(wire, sizeof(wire), SYN_ECAT_CMD_BRD, 1U, 0U, NULL, 2U,
                                      false);
    ecat_test_store16_le(wire, (uint16_t)(0x1000U | 20U)); /* end = 22 -> 2nd dgram only 6B left */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_parse_frame(&slave, wire, 32U, &wkc));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_slave_process_frame(&slave, wire, 32U, &wkc));

    syn_ecat_frame_begin(wire, sizeof(wire));
    (void)syn_ecat_frame_add_datagram(wire, sizeof(wire), SYN_ECAT_CMD_BRD, 1U, 0U, NULL, 10U,
                                      false);
    ecat_test_store16_le(wire, (uint16_t)(0x1000U | 14U)); /* d_len=10 exceeds dg_len=14 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_parse_frame(&slave, wire, 32U, &wkc));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ecat_slave_process_frame(&slave, wire, 32U, &wkc));
}

void run_ethercat_tests(void)
{
    RUN_TEST(test_ecat_init_and_esm);
    RUN_TEST(test_ecat_datagram_build_and_parse);
    RUN_TEST(test_ecat_coe_sdo_mailbox);
    RUN_TEST(test_ecat_edge_cases);
    RUN_TEST(test_ecat_multi_datagram);
    RUN_TEST(test_ecat_master_phase2);
    RUN_TEST(test_ecat_master_phases3_to_6);
    RUN_TEST(test_ecat_master_protothread_tasks);
    RUN_TEST(test_ecat_master_sii_and_sdo_tasks);
    RUN_TEST(test_ecat_reg_and_task_edge_cases);
    RUN_TEST(test_ecat_pdo_mapping_discovery_task);
    RUN_TEST(test_ecat_task_poll_timeouts);
    RUN_TEST(test_ecat_discover_pdo_mapping_sdo_timeout);
    RUN_TEST(test_ecat_null_and_bounds_coverage);
    RUN_TEST(test_ecat_cyclic_zero_frame_and_pdo_stage_failures);
    RUN_TEST(test_ecat_sii_sdo_decode_failure_and_3byte_sdo);
    RUN_TEST(test_ecat_r4_p09_regressions);
    RUN_TEST(test_r5_a07_master_slave_loopback_and_wkc_sdo_guards);
}
