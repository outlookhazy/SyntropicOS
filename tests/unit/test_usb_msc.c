/**
 * @file test_usb_msc.c
 * @brief Unity tests for Zero-Heap USB MSC Class Device Driver.
 */

#include "mocks/mock_port.h"
#include "syntropic/drivers/syn_usb_msc.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

#include <string.h>

static SYN_USB_MSC g_msc;
static SYN_USB_Device g_usb_dev;
static uint8_t s_ram_disk[8][SYN_USB_MSC_BLOCK_SIZE];
static int s_fail_read_on_lba = -1;
static int s_fail_write_on_lba = -1;

static SYN_Status mock_read_blocks(uint32_t lba, uint8_t *buf, uint16_t count)
{
    if ((int)lba == s_fail_read_on_lba) {
        return SYN_ERROR;
    }
    for (uint16_t i = 0U; i < count; i++) {
        memcpy(&buf[(size_t)i * SYN_USB_MSC_BLOCK_SIZE], s_ram_disk[lba + i],
               SYN_USB_MSC_BLOCK_SIZE);
    }
    return SYN_OK;
}

static SYN_Status mock_write_blocks(uint32_t lba, const uint8_t *buf, uint16_t count)
{
    if ((int)lba == s_fail_write_on_lba) {
        return SYN_ERROR;
    }
    for (uint16_t i = 0U; i < count; i++) {
        memcpy(s_ram_disk[lba + i], &buf[(size_t)i * SYN_USB_MSC_BLOCK_SIZE],
               SYN_USB_MSC_BLOCK_SIZE);
    }
    return SYN_OK;
}

static void build_cbw(uint8_t cbw[31], uint32_t tag, uint32_t xfer_len, uint8_t flags,
                      uint8_t cb_len, const uint8_t *cdb)
{
    memset(cbw, 0, 31U);
    cbw[0] = 'U';
    cbw[1] = 'S';
    cbw[2] = 'B';
    cbw[3] = 'C';
    cbw[4] = (uint8_t)(tag & 0xFFU);
    cbw[5] = (uint8_t)((tag >> 8) & 0xFFU);
    cbw[6] = (uint8_t)((tag >> 16) & 0xFFU);
    cbw[7] = (uint8_t)((tag >> 24) & 0xFFU);
    cbw[8] = (uint8_t)(xfer_len & 0xFFU);
    cbw[9] = (uint8_t)((xfer_len >> 8) & 0xFFU);
    cbw[10] = (uint8_t)((xfer_len >> 16) & 0xFFU);
    cbw[11] = (uint8_t)((xfer_len >> 24) & 0xFFU);
    cbw[12] = flags;
    cbw[13] = 0U;
    cbw[14] = cb_len;
    if ((cdb != NULL) && (cb_len > 0U)) {
        memcpy(&cbw[15], cdb, cb_len);
    }
}

void test_usb_msc_init_and_null_checks(void)
{
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_msc_init(NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_usb_msc_init(&g_msc));
    TEST_ASSERT_EQUAL(0x82U, g_msc.ep_in);
    TEST_ASSERT_EQUAL(0x02U, g_msc.ep_out);

    uint8_t desc23[SYN_USB_MSC_DESC_SIZE];
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_msc_get_config_desc(NULL, desc23));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_msc_get_config_desc(&g_msc, NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_usb_msc_get_config_desc(&g_msc, desc23));
    TEST_ASSERT_EQUAL_UINT8(0x09U, desc23[0]);
    TEST_ASSERT_EQUAL_UINT8(0x82U, desc23[11]);
    TEST_ASSERT_EQUAL_UINT8(0x02U, desc23[18]);

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_msc_register(NULL, &g_msc));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_msc_register(&g_usb_dev, NULL));

    static const uint8_t dev_desc[18] = {18,   1,    0x00, 0x02, 0, 0, 0, 64, 0xFE,
                                         0xCA, 0x01, 0x00, 0,    1, 1, 2, 0,  1};
    syn_usb_init(&g_usb_dev, dev_desc);
    TEST_ASSERT_EQUAL(SYN_OK, syn_usb_msc_register(&g_usb_dev, &g_msc));
    TEST_ASSERT_EQUAL_UINT16(9U + SYN_USB_MSC_DESC_SIZE, g_usb_dev.config_desc_len);

    /* Test setup callback */
    SYN_USB_ClassDriver *drv = &g_usb_dev.classes[0];
    SYN_USB_SetupPacket pkt = {.bRequest = 0xFEU};
    uint8_t resp[16] = {0};
    uint16_t rlen = 0;
    TEST_ASSERT_EQUAL(SYN_OK, drv->setup(&g_msc, &pkt, resp, &rlen));
    TEST_ASSERT_EQUAL(1, rlen);

    pkt.bRequest = 0xFFU; /* Bulk-Only Mass Storage Reset */
    TEST_ASSERT_EQUAL(SYN_OK, drv->setup(&g_msc, &pkt, resp, &rlen));
    TEST_ASSERT_EQUAL(SYN_USB_MSC_BOT_IDLE, g_msc.bot_state);

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, drv->setup(NULL, &pkt, resp, &rlen));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, drv->setup(&g_msc, NULL, resp, &rlen));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, drv->setup(&g_msc, &pkt, NULL, &rlen));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, drv->setup(&g_msc, &pkt, resp, NULL));

    pkt.bRequest = 0x00U;
    TEST_ASSERT_EQUAL(SYN_OK, drv->setup(&g_msc, &pkt, resp, &rlen));

    TEST_ASSERT_EQUAL(SYN_OK, drv->configured(&g_msc, 1));
    TEST_ASSERT_EQUAL(SYN_OK, drv->configured(NULL, 0));

    uint8_t cbw_dummy[31] = {'U', 'S', 'B', 'C'};
    drv->data_out(NULL, 0x02, cbw_dummy, 31);
    drv->data_out(&g_msc, 0x02, NULL, 31);
    drv->data_out(&g_msc, 0x01, cbw_dummy, 31);
    drv->data_out(&g_msc, 0x02, cbw_dummy, 10);
    drv->data_out(&g_msc, 0x02, cbw_dummy, 31);

    drv->data_in(NULL, 0x82);
    drv->data_in(&g_msc, 0x81);
    g_msc.tx_len = 5;
    drv->data_in(&g_msc, 0x82);
    TEST_ASSERT_EQUAL(0, g_msc.tx_len);

    SYN_USB_MSC_Media media = {
        .block_count = 8,
        .block_size = 512,
        .read_blocks = mock_read_blocks,
        .write_blocks = mock_write_blocks,
    };
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_msc_set_media(NULL, &media));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_msc_set_media(&g_msc, NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_usb_msc_set_media(&g_msc, &media));
}

void test_usb_msc_process_cbw_scsi_commands(void)
{
    syn_usb_msc_init(&g_msc);

    /* Construct valid CBW packet: Signature "USBC" = 0x43425355 */
    uint8_t cbw[31] = {0};
    cbw[0] = 'U';
    cbw[1] = 'S';
    cbw[2] = 'B';
    cbw[3] = 'C';
    cbw[4] = 0x01;  /* Tag = 1 */
    cbw[14] = 0x06; /* SCSI opcode length */
    cbw[15] = SYN_SCSI_TEST_UNIT_READY;

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_msc_process_cbw(NULL, cbw));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_usb_msc_process_cbw(&g_msc, NULL));

    TEST_ASSERT_EQUAL(SYN_OK, syn_usb_msc_process_cbw(&g_msc, cbw));
    TEST_ASSERT_EQUAL(0x53425355U, g_msc.csw.dCSWSignature); /* "USBS" */
    TEST_ASSERT_EQUAL(1U, g_msc.csw.dCSWTag);
    TEST_ASSERT_EQUAL(0U, g_msc.csw.bCSWStatus); /* Success */

    /* Test invalid CBW signature */
    cbw[0] = 'X';
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_usb_msc_process_cbw(&g_msc, cbw));

    /* Test all supported SCSI opcodes */
    uint8_t opcodes[] = {SYN_SCSI_TEST_UNIT_READY, SYN_SCSI_REQUEST_SENSE,    SYN_SCSI_INQUIRY,
                         SYN_SCSI_MODE_SENSE_6,    SYN_SCSI_READ_CAPACITY_10, SYN_SCSI_READ_10,
                         SYN_SCSI_WRITE_10};
    for (size_t i = 0; i < sizeof(opcodes); i++) {
        cbw[0] = 'U';
        cbw[15] = opcodes[i];
        TEST_ASSERT_EQUAL(SYN_OK, syn_usb_msc_process_cbw(&g_msc, cbw));
        TEST_ASSERT_EQUAL(0U, g_msc.csw.bCSWStatus);
    }

    /* Test unsupported opcode */
    cbw[0] = 'U';
    cbw[15] = 0xFF;
    TEST_ASSERT_EQUAL(SYN_OK, syn_usb_msc_process_cbw(&g_msc, cbw));
    TEST_ASSERT_EQUAL(1U, g_msc.csw.bCSWStatus); /* Failed status */
}

static void test_r5_usb_msc_regressions(void)
{
    static const uint8_t dev_desc[18] = {18,   1,    0x00, 0x02, 0, 0, 0, 64, 0xFE,
                                         0xCA, 0x01, 0x00, 0,    1, 1, 2, 0,  1};
    memset(s_ram_disk, 0, sizeof(s_ram_disk));
    s_fail_read_on_lba = -1;
    s_fail_write_on_lba = -1;

    syn_usb_init(&g_usb_dev, dev_desc);
    syn_usb_msc_init(&g_msc);
    SYN_USB_MSC_Media media = {
        .block_count = 8U,
        .block_size = 512U,
        .read_blocks = mock_read_blocks,
        .write_blocks = mock_write_blocks,
    };
    syn_usb_msc_set_media(&g_msc, &media);
    syn_usb_msc_register(&g_usb_dev, &g_msc);
    SYN_USB_ClassDriver *drv = &g_usb_dev.classes[0];

    /* 1. INQUIRY (36 bytes) -> DATA_IN -> CSW -> IDLE */
    uint8_t cbw[31];
    uint8_t cdb_inq[6] = {SYN_SCSI_INQUIRY, 0, 0, 0, 36, 0};
    build_cbw(cbw, 0x10U, 36U, 0x80U, 6U, cdb_inq);
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_MSC_BOT_DATA_IN, g_msc.bot_state);
    TEST_ASSERT_EQUAL_UINT16(36U, g_msc.tx_len);
    TEST_ASSERT_EQUAL_MEMORY("SYNTROPI", &g_msc.tx_buf[8], 8U);

    drv->data_in(&g_msc, g_msc.ep_in);
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_MSC_BOT_CSW, g_msc.bot_state);
    TEST_ASSERT_EQUAL_UINT16(13U, g_msc.tx_len);
    TEST_ASSERT_EQUAL_MEMORY("USBS", g_msc.tx_buf, 4U);
    TEST_ASSERT_EQUAL_UINT8(0U, g_msc.tx_buf[12]);

    drv->data_in(&g_msc, g_msc.ep_in);
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_MSC_BOT_IDLE, g_msc.bot_state);

    /* 2. MODE_SENSE_6 writable & read-only + READ_CAPACITY_10 */
    uint8_t cdb_ms[6] = {SYN_SCSI_MODE_SENSE_6, 0, 0x3F, 0, 4, 0};
    build_cbw(cbw, 0x11U, 8U, 0x80U, 6U, cdb_ms);
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    TEST_ASSERT_EQUAL_UINT16(4U, g_msc.tx_len);
    TEST_ASSERT_EQUAL_UINT8(0x00U, g_msc.tx_buf[2]);
    drv->data_in(&g_msc, g_msc.ep_in);
    TEST_ASSERT_EQUAL_UINT32(4U, g_msc.csw.dCSWDataResidue);
    drv->data_in(&g_msc, g_msc.ep_in);

    g_msc.media.write_blocks = NULL;
    build_cbw(cbw, 0x12U, 4U, 0x80U, 6U, cdb_ms);
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    TEST_ASSERT_EQUAL_UINT8(0x80U, g_msc.tx_buf[2]); /* Write-Protected bit */
    drv->data_in(&g_msc, g_msc.ep_in);
    drv->data_in(&g_msc, g_msc.ep_in);
    g_msc.media.write_blocks = mock_write_blocks;

    uint8_t cdb_cap[10] = {SYN_SCSI_READ_CAPACITY_10, 0};
    build_cbw(cbw, 0x13U, 8U, 0x80U, 10U, cdb_cap);
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    TEST_ASSERT_EQUAL_UINT16(8U, g_msc.tx_len);
    TEST_ASSERT_EQUAL_UINT8(7U, g_msc.tx_buf[3]);    /* last_lba = 7 */
    TEST_ASSERT_EQUAL_UINT8(0x02U, g_msc.tx_buf[6]); /* block_size = 0x0200 */
    drv->data_in(&g_msc, g_msc.ep_in);
    drv->data_in(&g_msc, g_msc.ep_in);

    /* 3. Multi-sector WRITE_10 (2 sectors = 1024B at LBA 2) + READ_10 round-trip */
    uint8_t wr_pattern[1024];
    for (size_t i = 0U; i < sizeof(wr_pattern); i++) {
        wr_pattern[i] = (uint8_t)((i * 7U + 3U) & 0xFFU);
    }

    uint8_t cdb_wr[10] = {SYN_SCSI_WRITE_10, 0, 0, 0, 0, 2, 0, 0, 2, 0};
    build_cbw(cbw, 0x20U, 1024U, 0x00U, 10U, cdb_wr);
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_MSC_BOT_DATA_OUT, g_msc.bot_state);

    for (uint16_t off = 0U; off < 1024U; off = (uint16_t)(off + 64U)) {
        drv->data_out(&g_msc, g_msc.ep_out, &wr_pattern[off], 64U);
    }
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_MSC_BOT_CSW, g_msc.bot_state);
    TEST_ASSERT_EQUAL_UINT8(0U, g_msc.csw.bCSWStatus);
    drv->data_in(&g_msc, g_msc.ep_in);
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_MSC_BOT_IDLE, g_msc.bot_state);

    uint8_t rd_back[1024];
    memset(rd_back, 0, sizeof(rd_back));
    uint8_t cdb_rd[10] = {SYN_SCSI_READ_10, 0, 0, 0, 0, 2, 0, 0, 2, 0};
    build_cbw(cbw, 0x21U, 1024U, 0x80U, 10U, cdb_rd);
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);

    for (uint16_t off = 0U; off < 1024U; off = (uint16_t)(off + 64U)) {
        TEST_ASSERT_EQUAL_UINT8(SYN_USB_MSC_BOT_DATA_IN, g_msc.bot_state);
        TEST_ASSERT_EQUAL_UINT16(64U, g_msc.tx_len);
        memcpy(&rd_back[off], g_msc.tx_buf, 64U);
        drv->data_in(&g_msc, g_msc.ep_in);
    }
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_MSC_BOT_CSW, g_msc.bot_state);
    TEST_ASSERT_EQUAL_UINT8(0U, g_msc.csw.bCSWStatus);
    drv->data_in(&g_msc, g_msc.ep_in);
    TEST_ASSERT_EQUAL_MEMORY(wr_pattern, rd_back, sizeof(wr_pattern));

    /* 4. LBA out of bounds + REQUEST_SENSE + BOT STALL verification */
    mock_usb_ep_stall_count = 0U;
    mock_usb_last_stalled_ep = 0U;
    uint8_t cdb_oob_rd[10] = {SYN_SCSI_READ_10, 0, 0, 0, 0, 7, 0, 0, 2, 0};
    build_cbw(cbw, 0x30U, 1024U, 0x80U, 10U, cdb_oob_rd);
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    TEST_ASSERT_EQUAL_UINT8(1U, g_msc.csw.bCSWStatus);
    TEST_ASSERT_EQUAL_UINT32(1U, mock_usb_ep_stall_count);
    TEST_ASSERT_EQUAL_HEX8(g_msc.ep_in, mock_usb_last_stalled_ep);
    drv->data_in(&g_msc, g_msc.ep_in);

    uint8_t cdb_sense[6] = {SYN_SCSI_REQUEST_SENSE, 0, 0, 0, 18, 0};
    build_cbw(cbw, 0x31U, 18U, 0x80U, 6U, cdb_sense);
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    TEST_ASSERT_EQUAL_UINT16(18U, g_msc.tx_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_SCSI_SENSE_ILLEGAL_REQUEST, g_msc.tx_buf[2]);
    TEST_ASSERT_EQUAL_UINT8(0x21U, g_msc.tx_buf[12]);
    drv->data_in(&g_msc, g_msc.ep_in);
    drv->data_in(&g_msc, g_msc.ep_in);

    mock_usb_ep_stall_count = 0U;
    uint8_t cdb_oob_wr[10] = {SYN_SCSI_WRITE_10, 0, 0, 0, 0, 8, 0, 0, 1, 0};
    build_cbw(cbw, 0x32U, 512U, 0x00U, 10U, cdb_oob_wr);
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    TEST_ASSERT_EQUAL_UINT8(1U, g_msc.csw.bCSWStatus);
    TEST_ASSERT_EQUAL_UINT32(1U, mock_usb_ep_stall_count);
    TEST_ASSERT_EQUAL_HEX8(g_msc.ep_out, mock_usb_last_stalled_ep);
    drv->data_in(&g_msc, g_msc.ep_in);

    /* BOT Phase Error (status=2) on direction mismatch or host_len < expected */
    build_cbw(cbw, 0x33U, 256U, 0x80U, 10U, cdb_rd); /* Hi (256) < Di (1024) */
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    TEST_ASSERT_EQUAL_UINT8(2U, g_msc.csw.bCSWStatus);
    drv->data_in(&g_msc, g_msc.ep_in);

    build_cbw(cbw, 0x34U, 1024U, 0x80U, 10U, cdb_wr); /* Hi <> Do */
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    TEST_ASSERT_EQUAL_UINT8(2U, g_msc.csw.bCSWStatus);
    drv->data_in(&g_msc, g_msc.ep_in);

    build_cbw(cbw, 0x35U, 36U, 0x00U, 6U, cdb_inq); /* Ho <> Di on INQUIRY */
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    TEST_ASSERT_EQUAL_UINT8(2U, g_msc.csw.bCSWStatus);
    drv->data_in(&g_msc, g_msc.ep_in);

    uint8_t cdb_zero_rd[10] = {SYN_SCSI_READ_10, 0};
    build_cbw(cbw, 0x36U, 64U, 0x00U, 10U, cdb_zero_rd); /* blocks=0 with Ho */
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    TEST_ASSERT_EQUAL_UINT8(2U, g_msc.csw.bCSWStatus);
    drv->data_in(&g_msc, g_msc.ep_in);

    uint8_t cdb_zero_wr[10] = {SYN_SCSI_WRITE_10, 0};
    build_cbw(cbw, 0x37U, 64U, 0x80U, 10U, cdb_zero_wr); /* blocks=0 with Hi */
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    TEST_ASSERT_EQUAL_UINT8(2U, g_msc.csw.bCSWStatus);
    drv->data_in(&g_msc, g_msc.ep_in);

    uint8_t cdb_tur[6] = {SYN_SCSI_TEST_UNIT_READY, 0};
    build_cbw(cbw, 0x38U, 16U, 0x00U, 6U, cdb_tur); /* TUR with Ho */
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    TEST_ASSERT_EQUAL_UINT8(2U, g_msc.csw.bCSWStatus);
    drv->data_in(&g_msc, g_msc.ep_in);

    /* Ho > Do residue STALL on successful WRITE_10 */
    mock_usb_ep_stall_count = 0U;
    build_cbw(cbw, 0x39U, 2048U, 0x00U, 10U, cdb_wr);
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    drv->data_out(&g_msc, g_msc.ep_out, wr_pattern, 1024U);
    TEST_ASSERT_EQUAL_UINT8(0U, g_msc.csw.bCSWStatus);
    TEST_ASSERT_EQUAL_UINT32(1024U, g_msc.csw.dCSWDataResidue);
    TEST_ASSERT_EQUAL_UINT32(1U, mock_usb_ep_stall_count);
    TEST_ASSERT_EQUAL_HEX8(g_msc.ep_out, mock_usb_last_stalled_ep);
    drv->data_in(&g_msc, g_msc.ep_in);

    /* 5. Media read failure on second sector & write failure on first sector */
    s_fail_read_on_lba = 3;
    build_cbw(cbw, 0x40U, 2048U, 0x80U, 10U, cdb_rd);
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    for (uint16_t pkt_i = 0U; pkt_i < 8U; pkt_i++) {
        drv->data_in(&g_msc, g_msc.ep_in);
    }
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_MSC_BOT_CSW, g_msc.bot_state);
    TEST_ASSERT_EQUAL_UINT8(1U, g_msc.csw.bCSWStatus);
    TEST_ASSERT_EQUAL_UINT8(SYN_SCSI_SENSE_MEDIUM_ERROR, g_msc.sense_key);
    drv->data_in(&g_msc, g_msc.ep_in);
    s_fail_read_on_lba = -1;

    s_fail_write_on_lba = 2;
    build_cbw(cbw, 0x41U, 2048U, 0x00U, 10U, cdb_wr);
    drv->data_out(&g_msc, g_msc.ep_out, cbw, 31U);
    drv->data_out(&g_msc, g_msc.ep_out, wr_pattern, 512U);
    TEST_ASSERT_EQUAL_UINT8(SYN_USB_MSC_BOT_CSW, g_msc.bot_state);
    TEST_ASSERT_EQUAL_UINT8(1U, g_msc.csw.bCSWStatus);
    TEST_ASSERT_EQUAL_UINT8(SYN_SCSI_SENSE_MEDIUM_ERROR, g_msc.sense_key);
    drv->data_in(&g_msc, g_msc.ep_in);
    s_fail_write_on_lba = -1;
}

void run_usb_msc_tests(void)
{
    RUN_TEST(test_usb_msc_init_and_null_checks);
    RUN_TEST(test_usb_msc_process_cbw_scsi_commands);
    RUN_TEST(test_r5_usb_msc_regressions);
}
