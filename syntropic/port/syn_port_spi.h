/**
 * @file syn_port_spi.h
 * @brief SPI port interface — implement these for your platform.
 *
 * The user provides implementations of these functions to connect
 * SyntropicOS to their MCU's SPI peripheral.
 * @ingroup syn_system
 */

#ifndef SYN_PORT_SPI_H
#define SYN_PORT_SPI_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── SPI mode (CPOL/CPHA) ───────────────────────────────────────────────── */

/** @brief SPI clock polarity/phase mode. */
typedef enum {
    SYN_SPI_MODE_0 = 0,  /**< CPOL=0, CPHA=0 */
    SYN_SPI_MODE_1 = 1,  /**< CPOL=0, CPHA=1 */
    SYN_SPI_MODE_2 = 2,  /**< CPOL=1, CPHA=0 */
    SYN_SPI_MODE_3 = 3,  /**< CPOL=1, CPHA=1 */
} SYN_SPI_Mode;

/* ── SPI configuration ──────────────────────────────────────────────────── */

/** @brief SPI bus configuration. */
typedef struct {
    uint8_t       bus;            /**< SPI bus index (0, 1, ...)           */
    uint32_t      clock_hz;       /**< SPI clock frequency                */
    SYN_SPI_Mode mode;           /**< Clock polarity / phase             */
    uint8_t       bit_order;      /**< 0 = MSB first (default), 1 = LSB  */
} SYN_SPI_Config;

/* ── Port functions (user implements) ───────────────────────────────────── */

/**
 * @brief Initialize an SPI bus.
 * @param cfg  SPI configuration.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_spi_init(const SYN_SPI_Config *cfg);

/**
 * @brief Deinitialize an SPI bus.
 * @param bus  SPI bus index.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_spi_deinit(uint8_t bus);

/**
 * @brief Full-duplex SPI transfer.
 *
 * Simultaneously transmits from tx_buf and receives into rx_buf.
 * Either buffer can be NULL for half-duplex operation.
 *
 * @param bus     SPI bus index.
 * @param tx_buf  Transmit buffer (NULL to send 0x00/0xFF).
 * @param rx_buf  Receive buffer (NULL to discard received data).
 * @param len     Number of bytes to transfer.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_spi_transfer(uint8_t bus,
                                   const uint8_t *tx_buf,
                                   uint8_t *rx_buf,
                                   size_t len);

/**
 * @brief Assert (pull low) an SPI chip-select pin.
 * @param bus     SPI bus index.
 * @param cs_pin  Chip-select GPIO pin.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_spi_cs_assert(uint8_t bus, SYN_GPIO_Pin cs_pin);

/**
 * @brief Deassert (release high) an SPI chip-select pin.
 * @param bus     SPI bus index.
 * @param cs_pin  Chip-select GPIO pin.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_spi_cs_deassert(uint8_t bus, SYN_GPIO_Pin cs_pin);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_SPI_H */
