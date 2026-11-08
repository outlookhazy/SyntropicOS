/**
 * @file syn_spi.h
 * @brief Hardware-decoupled SPI driver supporting Master/Slave roles & full-duplex transfers.
 * @ingroup syn_drivers
 */

#ifndef SYN_SPI_H
#define SYN_SPI_H

#include "../common/syn_defs.h"
#include "../port/syn_port_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief SPI operational mode role */
typedef enum {
    SYN_SPI_ROLE_MASTER = 0, /**< Master mode */
    SYN_SPI_ROLE_SLAVE       /**< Slave mode */
} SYN_SPI_Role;

#ifndef SYN_SPI_MODE_DEFINED
#define SYN_SPI_MODE_DEFINED /**< Guard for SYN_SPI_Mode definition */
/** @brief SPI clock phase and polarity mode */
typedef enum {
    SYN_SPI_MODE_0 = 0, /**< CPOL=0, CPHA=0 */
    SYN_SPI_MODE_1 = 1, /**< CPOL=0, CPHA=1 */
    SYN_SPI_MODE_2 = 2, /**< CPOL=1, CPHA=0 */
    SYN_SPI_MODE_3 = 3  /**< CPOL=1, CPHA=1 */
} SYN_SPI_Mode;
/** @brief Legacy typedef alias for SYN_SPI_Mode. */
typedef SYN_SPI_Mode SYN_SPIMode;
#endif

/** @brief SPI driver instance configuration parameters */
typedef struct {
    uint8_t spi_id;       /**< Hardware SPI instance (0 = SPI1) */
    uint32_t baudrate_hz; /**< Clock frequency in Hz (e.g. 1000000) */
    SYN_SPI_Mode mode;    /**< SPI clock phase & polarity mode */
    SYN_SPI_Role role;    /**< Master or Slave mode */
    bool use_dma;         /**< Enable DMA transfers */
} SYN_SPI_Config;

/** @brief SPI driver handle structure */
typedef struct {
    SYN_SPI_Config cfg; /**< Instance configuration */
    bool initialized;   /**< Initialization state */
} SYN_SPI;

/**
 * @brief Initialize an SPI instance.
 *
 * @param spi Pointer to SYN_SPI handle.
 * @param cfg Configuration structure.
 * @return SYN_OK on success.
 */
SYN_Status syn_spi_init(SYN_SPI *spi, const SYN_SPI_Config *cfg);

/**
 * @brief De-initialize an SPI instance.
 *
 * @param spi Pointer to SYN_SPI handle.
 * @return SYN_OK on success.
 */
SYN_Status syn_spi_deinit(SYN_SPI *spi);

/**
 * @brief Full-duplex simultaneous SPI transmit and receive.
 *
 * @param spi  Pointer to initialized SPI handle.
 * @param tx   TX data buffer (can be NULL, transmits dummy 0xFF).
 * @param rx   RX destination buffer (can be NULL).
 * @param len  Transfer length in bytes.
 * @return SYN_OK on success.
 */
SYN_Status syn_spi_transfer(SYN_SPI *spi, const uint8_t *tx, uint8_t *rx, size_t len);

/**
 * @brief Write bytes out over SPI (discards incoming RX bytes).
 *
 * @param spi  Pointer to initialized SPI handle.
 * @param tx   TX data buffer.
 * @param len  TX byte count.
 * @return SYN_OK on success.
 */
SYN_Status syn_spi_write(SYN_SPI *spi, const uint8_t *tx, size_t len);

/**
 * @brief Read bytes in over SPI (transmits dummy 0xFF bytes).
 *
 * @param spi  Pointer to initialized SPI handle.
 * @param rx   RX destination buffer.
 * @param len  RX byte count.
 * @return SYN_OK on success.
 */
SYN_Status syn_spi_read(SYN_SPI *spi, uint8_t *rx, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SPI_H */
