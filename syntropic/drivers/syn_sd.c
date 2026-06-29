#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SD) || SYN_USE_SD

/**
 * @file syn_sd.c
 * @brief SD card SPI block driver implementation.
 *
 * Implements the SD simplified SPI spec:
 *   - CMD0  (GO_IDLE_STATE)   reset to SPI mode
 *   - CMD8  (SEND_IF_COND)    SDHC-capable detection
 *   - CMD9  (SEND_CSD)        capacity via CSD v1/v2 parse
 *   - CMD13 (SEND_STATUS)     sync / flush
 *   - CMD16 (SET_BLOCKLEN)    SDSC: force 512-byte blocks
 *   - CMD17 (READ_SINGLE_BLOCK)
 *   - CMD24 (WRITE_BLOCK)
 *   - CMD55 (APP_CMD)         ACMD prefix
 *   - CMD58 (READ_OCR)        card type confirmation
 *   - ACMD41 (SD_SEND_OP_COND) card initialization
 *
 * CRC: CMD0/CMD8 use hardcoded CRC7 constants.  All other commands and
 * data blocks use 0xFF dummy bytes (CRC mode off by default in SPI).
 */

#include "syn_sd.h"
#include "../util/syn_assert.h"

#include <string.h>

/** @name SD command codes
 * @{
 */
#define SD_CMD0    0u   /**< GO_IDLE_STATE           */
#define SD_CMD8    8u   /**< SEND_IF_COND            */
#define SD_CMD9    9u   /**< SEND_CSD                */
#define SD_CMD13  13u   /**< SEND_STATUS             */
#define SD_CMD16  16u   /**< SET_BLOCKLEN            */
#define SD_CMD17  17u   /**< READ_SINGLE_BLOCK       */
#define SD_CMD24  24u   /**< WRITE_BLOCK             */
#define SD_CMD55  55u   /**< APP_CMD prefix          */
#define SD_CMD58  58u   /**< READ_OCR                */
#define SD_ACMD41 41u   /**< SD_SEND_OP_COND (app)   */
/** @} */

/** @name R1 response flags
 * @{
 */
#define SD_R1_IDLE    0x01u   /**< Card in idle state during init          */
#define SD_R1_ILLCMD  0x04u   /**< Illegal command (SDSC: no CMD8 support) */
#define SD_R1_READY   0x00u   /**< No errors, card ready                   */
#define SD_R1_ERR_MSK 0xFEu   /**< Any bit other than IDLE = error         */
#define SD_R1_TIMEOUT 0xFFu   /**< No response from card                   */
/** @} */

/** @name Data tokens
 * @{
 */
#define SD_TOKEN_START    0xFEu  /**< Start block for single read/write     */
#define SD_TOKEN_ACCEPTED 0x05u  /**< Write data response: (xxx0_0101)      */
/** @} */

/** @name Retry limits
 * @{
 */
#define SD_R1_POLL_RETRIES   8u    /**< Max R1 polling attempts              */
#define SD_ACMD41_RETRIES 1000u    /**< Max ACMD41 init retries              */
#define SD_TOKEN_RETRIES  2000u    /**< Max token wait retries               */
#define SD_BUSY_RETRIES   2000u    /**< Max busy wait retries                */
/** @} */

/**
 * @brief Transfer one byte over SPI and return the received byte.
 * @param sd   SD card instance.
 * @param out  Byte to send.
 * @return Received byte.
 */
static uint8_t sd_xfer(const SYN_SD *sd, uint8_t out)
{
    uint8_t in = 0xFFu;
    syn_port_spi_transfer(sd->spi_bus, &out, &in, 1u);
    return in;
}

/**
 * @brief Send a 6-byte SD command and poll for R1 response.
 * @param sd   SD card instance.
 * @param cmd  Command index (e.g. CMD0).
 * @param arg  32-bit argument.
 * @param crc  CRC byte.
 * @return R1 response byte.
 */
static uint8_t sd_cmd(const SYN_SD *sd, uint8_t cmd, uint32_t arg, uint8_t crc)
{
    uint8_t buf[6];
    buf[0] = (uint8_t)(0x40u | cmd);
    buf[1] = (uint8_t)((arg >> 24) & 0xFFu);
    buf[2] = (uint8_t)((arg >> 16) & 0xFFu);
    buf[3] = (uint8_t)((arg >>  8) & 0xFFu);
    buf[4] = (uint8_t)( arg        & 0xFFu);
    buf[5] = crc;
    /* rx_buf=NULL: port drives MOSI=0xFF during any simultaneous card output */
    syn_port_spi_transfer(sd->spi_bus, buf, NULL, sizeof(buf));

    /* Poll up to 8 bytes for the first non-0xFF response */
    uint8_t r1 = SD_R1_TIMEOUT;
    uint8_t i;
    for (i = 0u; i < SD_R1_POLL_RETRIES && r1 == SD_R1_TIMEOUT; i++) {
        r1 = sd_xfer(sd, 0xFFu);
    }
    return r1;
}

/**
 * @brief Wait until the SD card is no longer busy (MISO = 0xFF).
 * @param sd  SD card instance.
 * @return true if card became ready within timeout.
 */
static bool sd_wait_ready(const SYN_SD *sd)
{
    uint16_t i;
    for (i = 0u; i < SD_BUSY_RETRIES; i++) {
        if (sd_xfer(sd, 0xFFu) == 0xFFu) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Read the CSD register and parse sector count.
 * @param sd  SD card instance.
 * @return SYN_OK on success.
 */
static SYN_Status sd_read_csd(SYN_SD *sd)
{
    uint8_t r1 = sd_cmd(sd, SD_CMD9, 0u, 0xFFu);
    if (r1 != SD_R1_READY) {
        return SYN_ERROR;
    }

    /* Wait for start data token */
    uint8_t token = SD_R1_TIMEOUT;
    uint16_t i;
    for (i = 0u; i < SD_TOKEN_RETRIES && token == SD_R1_TIMEOUT; i++) {
        token = sd_xfer(sd, 0xFFu);
    }
    if (token != SD_TOKEN_START) {
        return SYN_ERROR;
    }

    /* Read 16 CSD bytes */
    uint8_t csd[16];
    uint8_t j;
    for (j = 0u; j < 16u; j++) {
        csd[j] = sd_xfer(sd, 0xFFu);
    }
    /* Discard 2-byte CRC (CRC mode off) */
    sd_xfer(sd, 0xFFu);
    sd_xfer(sd, 0xFFu);

    /* Parse by CSD structure version */
    uint8_t csd_ver = (csd[0] >> 6) & 0x03u;
    if (csd_ver == 0u) {
        /* CSD v1 (SDSC): decode C_SIZE, C_SIZE_MULT, READ_BL_LEN */
        uint8_t  read_bl_len  = csd[5] & 0x0Fu;
        uint32_t c_size       = (uint32_t)((csd[6] & 0x03u) << 10)
                              | ((uint32_t)csd[7] << 2)
                              | (uint32_t)((csd[8] >> 6) & 0x03u);
        uint8_t  c_size_mult  = (uint8_t)(((csd[9]  & 0x03u) << 1)
                              | ((csd[10] >> 7) & 0x01u));
        uint32_t mult         = (uint32_t)1u << ((uint32_t)c_size_mult + 2u);
        /* block_len / 512 avoids intermediate overflow for large SDSC cards */
        uint32_t bl_shift     = (read_bl_len >= 9u) ? (uint32_t)(read_bl_len - 9u) : 0u;
        uint32_t block_factor = (uint32_t)1u << bl_shift;
        sd->sector_count      = (c_size + 1u) * mult * block_factor;
    } else {
        /* CSD v2 (SDHC/SDXC): C_SIZE directly encodes 512KB units */
        uint32_t c_size  = ((uint32_t)(csd[7] & 0x3Fu) << 16)
                         | ((uint32_t)csd[8] << 8)
                         | (uint32_t)csd[9];
        sd->sector_count = (c_size + 1u) * 1024u;
    }

    return SYN_OK;
}

SYN_Status syn_sd_init(SYN_SD *sd, uint8_t spi_bus, SYN_GPIO_Pin cs)
{
    SYN_ASSERT(sd != NULL);

    sd->spi_bus      = spi_bus;
    sd->cs_pin       = cs;
    sd->type         = SYN_SD_UNKNOWN;
    sd->sector_count = 0u;
    sd->initialized  = false;

    /* Configure SPI at 400 kHz (SD spec: <= 400 kHz during init) */
    SYN_SPI_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.bus       = spi_bus;
    cfg.clock_hz  = 400000u;
    cfg.mode      = SYN_SPI_MODE_0;
    cfg.bit_order = 0u; /* MSB first */
    if (syn_port_spi_init(&cfg) != SYN_OK) {
        return SYN_ERROR;
    }

    /* Send >= 74 clock cycles with CS deasserted (SD power-up requirement) */
    syn_port_spi_cs_deassert(spi_bus, cs);
    {
        uint8_t clk_buf[10];
        memset(clk_buf, 0xFFu, sizeof(clk_buf));
        syn_port_spi_transfer(spi_bus, clk_buf, NULL, sizeof(clk_buf));
    }

    /* CMD0: reset card to SPI idle state.  CRC7 = 0x95 (hardcoded). */
    syn_port_spi_cs_assert(spi_bus, cs);
    uint8_t r1 = sd_cmd(sd, SD_CMD0, 0u, 0x95u);
    if (r1 != SD_R1_IDLE) {
        syn_port_spi_cs_deassert(spi_bus, cs);
        return SYN_ERROR;
    }

    /* CMD8: check interface condition — distinguishes V2/SDHC cards.
     * Arg = 0x000001AA: VHS=1 (2.7-3.6 V), check=0xAA.  CRC7 = 0x87. */
    bool is_v2 = false;
    r1 = sd_cmd(sd, SD_CMD8, 0x000001AAu, 0x87u);
    if (r1 == SD_R1_IDLE) {
        /* V2 card: read 4-byte R7 payload and verify voltage/echo fields */
        uint8_t r7[4];
        r7[0] = sd_xfer(sd, 0xFFu);
        r7[1] = sd_xfer(sd, 0xFFu);
        r7[2] = sd_xfer(sd, 0xFFu);
        r7[3] = sd_xfer(sd, 0xFFu);
        if (r7[3] == 0xAAu && (r7[2] & 0x0Fu) == 0x01u) {
            is_v2 = true;
        } else {
            /* Voltage range mismatch — card unsupported */
            syn_port_spi_cs_deassert(spi_bus, cs);
            return SYN_ERROR;
        }
    } else if ((r1 & SD_R1_ILLCMD) != 0u) {
        /* V1 / SDSC card: CMD8 not supported, proceed with byte-address path */
        is_v2 = false;
    } else {
        syn_port_spi_cs_deassert(spi_bus, cs);
        return SYN_ERROR;
    }

    /* ACMD41: activate card initialization.  HCS=1 for V2, 0 for V1. */
    uint32_t acmd41_arg = is_v2 ? 0x40000000u : 0x00000000u;
    uint16_t retries = SD_ACMD41_RETRIES;
    do {
        r1 = sd_cmd(sd, SD_CMD55, 0u, 0xFFu);
        if ((r1 & SD_R1_ERR_MSK) != 0u) {
            /* Any error bit besides IDLE is fatal */
            syn_port_spi_cs_deassert(spi_bus, cs);
            return SYN_ERROR;
        }
        r1 = sd_cmd(sd, SD_ACMD41, acmd41_arg, 0xFFu);
        retries--;
    } while (r1 == SD_R1_IDLE && retries > 0u);

    if (r1 != SD_R1_READY) {
        syn_port_spi_cs_deassert(spi_bus, cs);
        return SYN_ERROR;
    }

    if (is_v2) {
        /* CMD58: read OCR, check CCS bit (bit 30) to distinguish SDHC */
        r1 = sd_cmd(sd, SD_CMD58, 0u, 0xFFu);
        if (r1 != SD_R1_READY) {
            syn_port_spi_cs_deassert(spi_bus, cs);
            return SYN_ERROR;
        }
        uint8_t ocr[4];
        ocr[0] = sd_xfer(sd, 0xFFu);
        ocr[1] = sd_xfer(sd, 0xFFu);
        ocr[2] = sd_xfer(sd, 0xFFu);
        ocr[3] = sd_xfer(sd, 0xFFu);
        sd->type = ((ocr[0] & 0x40u) != 0u) ? SYN_SD_SDHC : SYN_SD_SDSC;
    } else {
        sd->type = SYN_SD_SDSC;
        /* CMD16: force block length to 512 bytes for V1 SDSC cards */
        r1 = sd_cmd(sd, SD_CMD16, SYN_SD_SECTOR_SIZE, 0xFFu);
        if (r1 != SD_R1_READY) {
            syn_port_spi_cs_deassert(spi_bus, cs);
            return SYN_ERROR;
        }
    }

    /* CMD9: read CSD register for capacity */
    if (sd_read_csd(sd) != SYN_OK) {
        syn_port_spi_cs_deassert(spi_bus, cs);
        return SYN_ERROR;
    }

    syn_port_spi_cs_deassert(spi_bus, cs);
    sd->initialized = true;
    return SYN_OK;
}

SYN_Status syn_sd_read(const SYN_SD *sd, uint32_t sector, uint8_t *buf)
{
    SYN_ASSERT(sd  != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(sd->initialized);

    /* SDSC: byte address (sector * 512).  SDHC: sector address. */
    uint32_t addr = (sd->type == SYN_SD_SDSC)
                  ? (sector * (uint32_t)SYN_SD_SECTOR_SIZE)
                  : sector;

    syn_port_spi_cs_assert(sd->spi_bus, sd->cs_pin);

    uint8_t r1 = sd_cmd(sd, SD_CMD17, addr, 0xFFu);
    if (r1 != SD_R1_READY) {
        syn_port_spi_cs_deassert(sd->spi_bus, sd->cs_pin);
        return SYN_ERROR;
    }

    /* Wait for start data token */
    uint8_t token = SD_R1_TIMEOUT;
    uint16_t i;
    for (i = 0u; i < SD_TOKEN_RETRIES && token == SD_R1_TIMEOUT; i++) {
        token = sd_xfer(sd, 0xFFu);
    }
    if (token != SD_TOKEN_START) {
        syn_port_spi_cs_deassert(sd->spi_bus, sd->cs_pin);
        return SYN_ERROR;
    }

    /* Read 512 data bytes */
    uint16_t j;
    for (j = 0u; j < (uint16_t)SYN_SD_SECTOR_SIZE; j++) {
        buf[j] = sd_xfer(sd, 0xFFu);
    }
    /* Discard 2-byte CRC */
    sd_xfer(sd, 0xFFu);
    sd_xfer(sd, 0xFFu);

    syn_port_spi_cs_deassert(sd->spi_bus, sd->cs_pin);
    return SYN_OK;
}

SYN_Status syn_sd_write(const SYN_SD *sd, uint32_t sector, const uint8_t *buf)
{
    SYN_ASSERT(sd  != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(sd->initialized);

    uint32_t addr = (sd->type == SYN_SD_SDSC)
                  ? (sector * (uint32_t)SYN_SD_SECTOR_SIZE)
                  : sector;

    syn_port_spi_cs_assert(sd->spi_bus, sd->cs_pin);

    uint8_t r1 = sd_cmd(sd, SD_CMD24, addr, 0xFFu);
    if (r1 != SD_R1_READY) {
        syn_port_spi_cs_deassert(sd->spi_bus, sd->cs_pin);
        return SYN_ERROR;
    }

    /* Send start token */
    sd_xfer(sd, SD_TOKEN_START);

    /* Send 512 data bytes */
    syn_port_spi_transfer(sd->spi_bus, buf, NULL, (size_t)SYN_SD_SECTOR_SIZE);

    /* Send dummy CRC (CRC mode disabled) */
    sd_xfer(sd, 0xFFu);
    sd_xfer(sd, 0xFFu);

    /* Read data response token: (xxx0_0101) = accepted */
    uint8_t resp = sd_xfer(sd, 0xFFu);
    if ((resp & 0x1Fu) != SD_TOKEN_ACCEPTED) {
        syn_port_spi_cs_deassert(sd->spi_bus, sd->cs_pin);
        return SYN_ERROR;
    }

    /* Wait until write completes */
    if (!sd_wait_ready(sd)) {
        syn_port_spi_cs_deassert(sd->spi_bus, sd->cs_pin);
        return SYN_ERROR;
    }

    syn_port_spi_cs_deassert(sd->spi_bus, sd->cs_pin);
    return SYN_OK;
}

SYN_Status syn_sd_sync(const SYN_SD *sd)
{
    SYN_ASSERT(sd != NULL);
    SYN_ASSERT(sd->initialized);

    syn_port_spi_cs_assert(sd->spi_bus, sd->cs_pin);

    /* CMD13: SEND_STATUS returns R2 (two bytes) */
    uint8_t r1 = sd_cmd(sd, SD_CMD13, 0u, 0xFFu);
    uint8_t r2 = sd_xfer(sd, 0xFFu);

    syn_port_spi_cs_deassert(sd->spi_bus, sd->cs_pin);

    return (r1 == SD_R1_READY && r2 == 0x00u) ? SYN_OK : SYN_ERROR;
}

/* ── Accessors ──────────────────────────────────────────────────────────── */

uint32_t syn_sd_sectors(const SYN_SD *sd)
{
    SYN_ASSERT(sd != NULL);
    return sd->sector_count;
}

SYN_SD_Type syn_sd_type(const SYN_SD *sd)
{
    SYN_ASSERT(sd != NULL);
    return sd->type;
}

#endif /* SYN_USE_SD */
