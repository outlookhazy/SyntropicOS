/**
 * @file test_sd.c
 * @brief Unity tests for syn_sd (SD card SPI block driver).
 *
 * Mock design: mock_spi_rx_buf holds canned bytes the card "sends" back.
 * rx_pos advances only when rx_buf != NULL in syn_port_spi_transfer(), so
 * 6-byte command sends (rx=NULL) do not consume receive slots.
 *
 * SDHC init rx sequence (34 bytes):
 *   CMD0 R1 poll      : 0xFF 0x01
 *   CMD8 R1 poll      : 0x01
 *   CMD8 R7 payload   : 0x00 0x00 0x01 0xAA
 *   CMD55 R1          : 0x01
 *   ACMD41 R1         : 0x00  (ready on first attempt)
 *   CMD58 R1          : 0x00
 *   CMD58 OCR         : 0xC0 0xFF 0x80 0x00  (CCS=1 -> SDHC)
 *   CMD9  R1          : 0x00
 *   CMD9  data token  : 0xFE
 *   CSD v2 (16 bytes) : 0x40 ... C_SIZE=0x3B7F -> 15,597,568 sectors
 *   CSD CRC           : 0xFF 0xFF
 *
 * SDSC init rx sequence (26 bytes):
 *   CMD0 R1 poll      : 0xFF 0x01
 *   CMD8 R1 poll      : 0x05  (illegal command -> SDSC path)
 *   CMD55 R1          : 0x01
 *   ACMD41 R1         : 0x00
 *   CMD16 R1          : 0x00
 *   CMD9  R1          : 0x00
 *   CMD9  data token  : 0xFE
 *   CSD v1 (16 bytes) : C_SIZE=3 C_SIZE_MULT=7 READ_BL_LEN=9 -> 2048 sectors
 *   CSD CRC           : 0xFF 0xFF
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/drivers/syn_sd.h"
#include <string.h>

/* ── Canned SDHC init response ──────────────────────────────────────────── */

/* C_SIZE = (0x00<<16)|(0x3B<<8)|0x7F = 15231
 * sector_count = (15231+1)*1024 = 15,597,568 */
static const uint8_t sdhc_init_rx[] = {
    /* CMD0 R1 poll        */ 0xFF, 0x01,
    /* CMD8 R1             */ 0x01,
    /* CMD8 R7             */ 0x00, 0x00, 0x01, 0xAA,
    /* CMD55 R1            */ 0x01,
    /* ACMD41 R1           */ 0x00,
    /* CMD58 R1            */ 0x00,
    /* CMD58 OCR (CCS=1)   */ 0xC0, 0xFF, 0x80, 0x00,
    /* CMD9 R1             */ 0x00,
    /* CMD9 data token     */ 0xFE,
    /* CSD v2 — 16 bytes   */
    0x40, 0x0E, 0x00, 0x32, 0x5B, 0x59,
    0x00,                   /* csd[6]  */
    0x00,                   /* csd[7]  C_SIZE[21:16]=0  */
    0x3B,                   /* csd[8]  C_SIZE[15:8]=0x3B */
    0x7F,                   /* csd[9]  C_SIZE[7:0]=0x7F  */
    0x32, 0x5A, 0x83, 0xC8, 0x96, 0x40,
    /* CSD CRC             */ 0xFF, 0xFF,
};

/* ── Canned SDSC init response ──────────────────────────────────────────── */

/* CSD v1: C_SIZE=3, C_SIZE_MULT=7, READ_BL_LEN=9
 * block_factor = 1<<(9-9)=1, mult=1<<(7+2)=512
 * sector_count = (3+1)*512*1 = 2048 */
static const uint8_t sdsc_init_rx[] = {
    /* CMD0 R1 poll        */ 0xFF, 0x01,
    /* CMD8 R1 (illegal)   */ 0x05,
    /* CMD55 R1            */ 0x01,
    /* ACMD41 R1           */ 0x00,
    /* CMD16 R1            */ 0x00,
    /* CMD9 R1             */ 0x00,
    /* CMD9 data token     */ 0xFE,
    /* CSD v1 — 16 bytes
     * csd[5]=0x59 -> READ_BL_LEN=9
     * csd[6]=0x80, csd[7]=0x00, csd[8]=0xC0 -> C_SIZE=3
     * csd[9]=0x03, csd[10]=0x80               -> C_SIZE_MULT=7  */
    0x00, 0x26, 0x00, 0x5A, 0x5B, 0x59,
    0x80, 0x00, 0xC0, 0x03,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* CSD CRC             */ 0xFF, 0xFF,
};

/* ── test 1: SDHC card initializes correctly ────────────────────────────── */

static void test_sd_init_sdhc(void)
{
    SYN_SD sd;
    mock_spi_set_response(sdhc_init_rx, sizeof(sdhc_init_rx));

    TEST_ASSERT_EQUAL(SYN_OK, syn_sd_init(&sd, 0, (SYN_GPIO_Pin)0));
    TEST_ASSERT_TRUE(sd.initialized);
    TEST_ASSERT_EQUAL(SYN_SD_SDHC, syn_sd_type(&sd));
    TEST_ASSERT_EQUAL_UINT32(15597568UL, syn_sd_sectors(&sd));
}

/* ── test 2: SDSC card (no CMD8 support) initializes correctly ──────────── */

static void test_sd_init_sdsc(void)
{
    SYN_SD sd;
    mock_spi_set_response(sdsc_init_rx, sizeof(sdsc_init_rx));

    TEST_ASSERT_EQUAL(SYN_OK, syn_sd_init(&sd, 0, (SYN_GPIO_Pin)0));
    TEST_ASSERT_TRUE(sd.initialized);
    TEST_ASSERT_EQUAL(SYN_SD_SDSC, syn_sd_type(&sd));
    TEST_ASSERT_EQUAL_UINT32(2048UL, syn_sd_sectors(&sd));
}

/* ── test 3: No response from card -> init must return SYN_ERROR ────────── */

static void test_sd_init_timeout(void)
{
    SYN_SD sd;
    /* Leave rx buffer empty: all reads return 0xFF (no card response).
     * CMD0 R1 poll returns 0xFF -> SYN_ERROR before ACMD41 loop. */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_init(&sd, 0, (SYN_GPIO_Pin)0));
    TEST_ASSERT_FALSE(sd.initialized);
}

/* ── test 4: Read sector fills buffer with card data ────────────────────── */

static void test_sd_read_sector(void)
{
    /* Bypass init: configure struct directly as SDHC */
    SYN_SD sd;
    sd.spi_bus      = 0;
    sd.cs_pin       = (SYN_GPIO_Pin)0;
    sd.type         = SYN_SD_SDHC;
    sd.sector_count = 16384;
    sd.initialized  = true;

    /* Read rx: R1=0x00, data_token=0xFE, 512 data bytes, 2 CRC bytes */
    uint8_t read_rx[516];
    read_rx[0] = 0x00u;  /* R1: ready */
    read_rx[1] = 0xFEu;  /* data token */
    uint16_t k;
    for (k = 0; k < 512; k++) {
        read_rx[2 + k] = (uint8_t)(k & 0xFFu);  /* known pattern */
    }
    read_rx[514] = 0xFFu;  /* CRC */
    read_rx[515] = 0xFFu;

    mock_spi_set_response(read_rx, sizeof(read_rx));

    uint8_t buf[512];
    TEST_ASSERT_EQUAL(SYN_OK, syn_sd_read(&sd, 0, buf));

    /* Verify first, mid, and last data bytes */
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x7F, buf[127]);
    TEST_ASSERT_EQUAL_HEX8(0xFF, buf[255]);
    TEST_ASSERT_EQUAL_HEX8(0xFF & 511, buf[511]);
}

/* ── test 5: Write sector sends correct command + data bytes ────────────── */

static void test_sd_write_sector(void)
{
    SYN_SD sd;
    sd.spi_bus      = 0;
    sd.cs_pin       = (SYN_GPIO_Pin)0;
    sd.type         = SYN_SD_SDHC;
    sd.sector_count = 16384;
    sd.initialized  = true;

    /* Write rx: R1=0x00, token_echo, CRC_echo x2, data_accepted, not_busy */
    static const uint8_t write_rx[] = {
        0x00u,  /* R1 for CMD24: ready                   */
        0xFFu,  /* echo of data token (discarded)         */
        0xFFu,  /* echo of dummy CRC byte 1 (discarded)  */
        0xFFu,  /* echo of dummy CRC byte 2 (discarded)  */
        0xE5u,  /* data response: 0xE5 & 0x1F = 0x05 (accepted) */
        0xFFu,  /* busy wait: not busy                   */
    };
    mock_spi_set_response(write_rx, sizeof(write_rx));

    uint8_t buf[512];
    uint16_t k;
    for (k = 0; k < 512; k++) {
        buf[k] = (uint8_t)(k & 0xFFu);
    }

    TEST_ASSERT_EQUAL(SYN_OK, syn_sd_write(&sd, 1, buf));

    /* TX[0] = 0x40|24 = 0x58 (CMD24 start byte) */
    TEST_ASSERT_EQUAL_HEX8(0x58, mock_spi_tx_buf[0]);

    /* TX[1..4] = sector address for SDHC: sector=1 -> 0x00 0x00 0x00 0x01 */
    TEST_ASSERT_EQUAL_HEX8(0x00, mock_spi_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00, mock_spi_tx_buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, mock_spi_tx_buf[3]);
    TEST_ASSERT_EQUAL_HEX8(0x01, mock_spi_tx_buf[4]);

    /* TX[7] = 0xFE (start data token, after 6-byte cmd + 1 R1 poll byte) */
    TEST_ASSERT_EQUAL_HEX8(0xFE, mock_spi_tx_buf[7]);

    /* TX[8] = first data byte (0x00 for k=0) */
    TEST_ASSERT_EQUAL_HEX8(0x00, mock_spi_tx_buf[8]);
    /* TX[9] = second data byte (0x01) */
    TEST_ASSERT_EQUAL_HEX8(0x01, mock_spi_tx_buf[9]);
}

/* ── test 6: Sync (CMD13) returns SYN_OK when card reports no error ─────── */

static void test_sd_sync(void)
{
    SYN_SD sd;
    sd.spi_bus      = 0;
    sd.cs_pin       = (SYN_GPIO_Pin)0;
    sd.type         = SYN_SD_SDHC;
    sd.sector_count = 16384;
    sd.initialized  = true;

    /* CMD13 returns R1=0x00, R2=0x00 -> no error */
    static const uint8_t sync_rx[] = { 0x00u, 0x00u };
    mock_spi_set_response(sync_rx, sizeof(sync_rx));

    TEST_ASSERT_EQUAL(SYN_OK, syn_sd_sync(&sd));

    /* CMD13 returns R1=0x00, R2=0x04 -> card error -> SYN_ERROR */
    static const uint8_t sync_rx_err[] = { 0x00u, 0x04u };
    mock_spi_set_response(sync_rx_err, sizeof(sync_rx_err));

    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_sync(&sd));
}

/** CMD0 fails (not IDLE) — exercises line 241 (cs deassert + error) */
static void test_sd_init_cmd0_fail(void)
{
    SYN_SD sd;
    /* CMD0 R1 poll returns error byte (not 0x01) */
    uint8_t rx[] = { 0xFF, 0x04 }; /* 0x04 = some error bit */
    mock_spi_set_response(rx, sizeof(rx));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_init(&sd, 0, (SYN_GPIO_Pin)0));
}

/** CMD8 R7 voltage mismatch — exercises line 248-249 */
static void test_sd_init_cmd8_voltage_mismatch(void)
{
    SYN_SD sd;
    uint8_t rx[] = {
        /* CMD0 R1 poll */ 0xFF, 0x01,
        /* CMD8 R1      */ 0x01,
        /* CMD8 R7 payload — wrong echo byte (0xBB != 0xAA) */
        0x00, 0x00, 0x01, 0xBB,
    };
    mock_spi_set_response(rx, sizeof(rx));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_init(&sd, 0, (SYN_GPIO_Pin)0));
}

/** CMD55 error during ACMD41 loop — exercises line 259-260 */
static void test_sd_init_cmd55_error(void)
{
    SYN_SD sd;
    uint8_t rx[] = {
        /* CMD0 R1 poll */ 0xFF, 0x01,
        /* CMD8 R1      */ 0x01,
        /* CMD8 R7      */ 0x00, 0x00, 0x01, 0xAA,
        /* CMD55 R1: error bits set (not just IDLE) */
        0x04,
    };
    mock_spi_set_response(rx, sizeof(rx));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_init(&sd, 0, (SYN_GPIO_Pin)0));
}

/** CSD read: CMD9 returns error R1 — exercises line 140 */
static void test_sd_read_sector_bad_token(void)
{
    SYN_SD sd;
    sd.spi_bus = 0;
    sd.cs_pin = (SYN_GPIO_Pin)0;
    sd.type = SYN_SD_SDHC;
    sd.sector_count = 16384;
    sd.initialized = true;

    /* CMD17 R1 = 0x00 (OK), but data token never arrives (all 0xFF) */
    uint8_t rx[520];
    rx[0] = 0x00u; /* R1 ready */
    memset(&rx[1], 0xFF, sizeof(rx) - 1); /* no data token */
    mock_spi_set_response(rx, sizeof(rx));
    uint8_t buf[512];
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_read(&sd, 0, buf));
}
/** Read: CMD17 returns error R1 — exercises lines 320-321 */
static void test_sd_read_cmd17_fail(void)
{
    SYN_SD sd;
    sd.spi_bus = 0;
    sd.cs_pin = (SYN_GPIO_Pin)0;
    sd.type = SYN_SD_SDHC;
    sd.sector_count = 16384;
    sd.initialized = true;

    /* CMD17 R1 = 0x04 (error) */
    uint8_t rx[] = { 0x04 };
    mock_spi_set_response(rx, sizeof(rx));
    uint8_t buf[512];
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_read(&sd, 0, buf));
}

/** Write: CMD24 returns error R1 — exercises lines 362-363 */
static void test_sd_write_cmd24_fail(void)
{
    SYN_SD sd;
    sd.spi_bus = 0;
    sd.cs_pin = (SYN_GPIO_Pin)0;
    sd.type = SYN_SD_SDHC;
    sd.sector_count = 16384;
    sd.initialized = true;

    /* CMD24 R1 = 0x04 (error) */
    uint8_t rx[] = { 0x04 };
    mock_spi_set_response(rx, sizeof(rx));
    uint8_t buf[512];
    memset(buf, 0xAA, sizeof(buf));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_write(&sd, 0, buf));
}

/** Write: data response rejected — exercises lines 379-380 */
static void test_sd_write_data_rejected(void)
{
    SYN_SD sd;
    sd.spi_bus = 0;
    sd.cs_pin = (SYN_GPIO_Pin)0;
    sd.type = SYN_SD_SDHC;
    sd.sector_count = 16384;
    sd.initialized = true;

    /* CMD24 R1 = 0x00 (OK), then data response = 0x0B (rejected, not 0x05) */
    uint8_t rx[520];
    rx[0] = 0x00;  /* R1 ready */
    /* After CMD24, card receives 512 data bytes + 2 CRC → then data response.
     * Mock doesn't consume TX bytes, so data response is at rx[1] in our mock */
    rx[1] = 0x0B;  /* data response: rejected (not 0x05) */
    mock_spi_set_response(rx, 2);
    uint8_t buf[512];
    memset(buf, 0xAA, sizeof(buf));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_write(&sd, 0, buf));
}

/* ── Registration ────────────────────────────────────────────────────────── */

static void test_sd_wait_ready_timeout(void)
{
    SYN_SD sd;
    /* Do a normal init first */
    mock_spi_set_response(sdhc_init_rx, sizeof(sdhc_init_rx));
    TEST_ASSERT_EQUAL(SYN_OK, syn_sd_init(&sd, 0, (SYN_GPIO_Pin)0));

    /* Provide write responses */
    uint8_t write_rx[] = {
        0xFF, /* Poll for CMD24 */
        0x00, /* CMD24 R1 */
        0xFF, /* Token start dummy read */
        0xFF, 0xFF, /* CRC dummy bytes */
        0x05, /* Data response (accepted) */
    };
    mock_spi_set_response(write_rx, sizeof(write_rx));

    /* Now try to write but wait_ready times out */
    mock_spi_infinite = true;
    mock_spi_infinite_byte = 0x00;
    uint8_t buf[512] = {0};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_write(&sd, 0, buf));
    mock_spi_infinite = false;
}

static void test_sd_read_csd_fails(void)
{
    SYN_SD sd;
    uint8_t rx[] = {
        0x01, /* CMD0 */
        0x01, 0x00, 0x00, 0x01, 0xAA, /* CMD8 */
        0x01, /* CMD55 */
        0x00, /* ACMD41 */
        0x00, 0xC0, 0xFF, 0x80, 0x00, /* CMD58 */
        0x05  /* CMD9 error R1 */
    };
    mock_spi_set_response(rx, sizeof(rx));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_init(&sd, 0, (SYN_GPIO_Pin)0));
}

static void test_sd_read_csd_token_timeout(void)
{
    SYN_SD sd;
    uint8_t rx[] = {
        0x01, /* CMD0 */
        0x01, 0x00, 0x00, 0x01, 0xAA, /* CMD8 */
        0x01, /* CMD55 */
        0x00, /* ACMD41 */
        0x00, 0xC0, 0xFF, 0x80, 0x00, /* CMD58 */
        0x00  /* CMD9 OK, but no 0xFE token follows (mock spi buffer exhausts, returns 0xFF) */
    };
    mock_spi_set_response(rx, sizeof(rx));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_init(&sd, 0, (SYN_GPIO_Pin)0));
}

static void test_sd_init_cmd8_invalid(void)
{
    SYN_SD sd;
    uint8_t rx[] = {
        0x01, /* CMD0 */
        0x02, /* CMD8 returns weird error bit, not ILLCMD (0x04) */
    };
    mock_spi_set_response(rx, sizeof(rx));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_init(&sd, 0, (SYN_GPIO_Pin)0));
}

static void test_sd_init_acmd41_timeout(void)
{
    SYN_SD sd;
    uint8_t rx[] = {
        0x01, /* CMD0 */
        0x01, 0x00, 0x00, 0x01, 0xAA, /* CMD8 */
    };
    mock_spi_set_response(rx, sizeof(rx));
    mock_spi_infinite = true;
    mock_spi_infinite_byte = 0x01; /* CMD55 / ACMD41 always return IDLE */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_init(&sd, 0, (SYN_GPIO_Pin)0));
    mock_spi_infinite = false;
}

static void test_sd_init_cmd58_fail(void)
{
    SYN_SD sd;
    uint8_t rx[] = {
        0x01, /* CMD0 */
        0x01, 0x00, 0x00, 0x01, 0xAA, /* CMD8 */
        0x01, /* CMD55 */
        0x00, /* ACMD41 */
        0x01  /* CMD58 returns not ready */
    };
    mock_spi_set_response(rx, sizeof(rx));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_init(&sd, 0, (SYN_GPIO_Pin)0));
}

static void test_sd_init_cmd16_fail(void)
{
    SYN_SD sd;
    uint8_t rx[] = {
        0x01, /* CMD0 */
        0x05, /* CMD8 returns ILLCMD (SDSC v1) */
        0x01, /* CMD55 */
        0x00, /* ACMD41 */
        0x01  /* CMD16 returns error */
    };
    mock_spi_set_response(rx, sizeof(rx));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_init(&sd, 0, (SYN_GPIO_Pin)0));
}


static void test_sd_spi_init_fail(void)
{
    SYN_SD sd;
    mock_spi_init_ok = false;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sd_init(&sd, 0, (SYN_GPIO_Pin)0));
    mock_spi_init_ok = true;
}

void run_sd_tests(void)
{
    RUN_TEST(test_sd_init_sdhc);
    RUN_TEST(test_sd_init_sdsc);
    RUN_TEST(test_sd_init_timeout);
    RUN_TEST(test_sd_read_sector);
    RUN_TEST(test_sd_write_sector);
    RUN_TEST(test_sd_sync);
    RUN_TEST(test_sd_init_cmd0_fail);
    RUN_TEST(test_sd_init_cmd8_voltage_mismatch);
    RUN_TEST(test_sd_init_cmd55_error);
    RUN_TEST(test_sd_read_sector_bad_token);
    RUN_TEST(test_sd_read_cmd17_fail);
    RUN_TEST(test_sd_write_cmd24_fail);
    RUN_TEST(test_sd_write_data_rejected);
    RUN_TEST(test_sd_wait_ready_timeout);
    RUN_TEST(test_sd_read_csd_fails);
    RUN_TEST(test_sd_read_csd_token_timeout);
    RUN_TEST(test_sd_init_cmd8_invalid);
    RUN_TEST(test_sd_init_acmd41_timeout);
    RUN_TEST(test_sd_init_cmd58_fail);
    RUN_TEST(test_sd_init_cmd16_fail);
    RUN_TEST(test_sd_spi_init_fail);
}
