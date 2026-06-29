/**
 * @file syn_soft_spi.h
 * @brief Software SPI (bit-banging) driver.
 *
 * Implements SPI master functionality using any GPIO pins.
 * Relies on syn_port_gpio.h for pin manipulation.
 * @ingroup syn_drivers
 */

#ifndef SYN_SOFT_SPI_H
#define SYN_SOFT_SPI_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SPI Clock Mode
 */
typedef enum {
    SYN_SPI_MODE_0 = 0, /**< CPOL=0, CPHA=0 */
    SYN_SPI_MODE_1 = 1, /**< CPOL=0, CPHA=1 */
    SYN_SPI_MODE_2 = 2, /**< CPOL=1, CPHA=0 */
    SYN_SPI_MODE_3 = 3  /**< CPOL=1, CPHA=1 */
} SYN_SPIMode;

/**
 * @brief Software SPI instance.
 */
typedef struct {
    SYN_GPIO_Pin sck;              /**< SCK GPIO pin identifier */
    SYN_GPIO_Pin mosi;             /**< MOSI GPIO pin identifier */
    SYN_GPIO_Pin miso;             /**< MISO GPIO pin identifier */
    SYN_SPIMode mode;              /**< SPI phase and polarity mode */
    uint32_t delay_loops;          /**< Iteration count for half-clock software delay */
    
    // Cached states for faster transfer
    bool cpha;                     /**< Clock phase cache */
    SYN_GPIO_State idle_state;     /**< Cached idle pin state */
    SYN_GPIO_State active_state;   /**< Cached active pin state */

    // Optional chip select (set to (SYN_GPIO_Pin)-1 if unused)
    SYN_GPIO_Pin cs_pin;           /**< Chip select GPIO pin identifier */
    bool cs_active_low;            /**< CS active logic polarity */
} SYN_SoftSPI;

/**
 * @brief Initialize the soft SPI pins.
 * @param spi          Pointer to SPI instance.
 * @param sck          SCK pin.
 * @param mosi         MOSI pin.
 * @param miso         MISO pin.
 * @param mode         SPI Mode (0-3).
 * @param delay_loops  Number of iterations for a half-clock delay.
 */
void syn_soft_spi_init(SYN_SoftSPI *spi, SYN_GPIO_Pin sck, SYN_GPIO_Pin mosi, SYN_GPIO_Pin miso, SYN_SPIMode mode, uint32_t delay_loops);

/**
 * @brief Transfer a single byte (read and write simultaneously).
 * @param spi   Pointer to SPI instance.
 * @param data  Byte to write out on MOSI.
 * @return Byte read in from MISO.
 */
uint8_t syn_soft_spi_transfer(const SYN_SoftSPI *spi, uint8_t data);

/**
 * @brief Transfer multiple bytes.
 * @param spi   Pointer to SPI instance.
 * @param tx    Buffer to transmit (can be NULL if rx-only).
 * @param rx    Buffer to receive into (can be NULL if tx-only).
 * @param len   Number of bytes to transfer.
 */
void syn_soft_spi_transfer_bulk(SYN_SoftSPI *spi, const uint8_t *tx, uint8_t *rx, size_t len);

/**
 * @brief Set an optional chip select pin.
 *
 * When set, syn_soft_spi_select/deselect can be used to assert/deassert CS.
 *
 * @param spi         SPI instance.
 * @param cs_pin      Chip select GPIO pin.
 * @param active_low  true if CS is active-low (most common).
 */
void syn_soft_spi_set_cs(SYN_SoftSPI *spi, SYN_GPIO_Pin cs_pin, bool active_low);

/**
 * @brief Assert chip select (drive active).
 * @param spi  Soft SPI instance.
 */
void syn_soft_spi_select(SYN_SoftSPI *spi);

/**
 * @brief Deassert chip select (drive inactive).
 * @param spi  Soft SPI instance.
 */
void syn_soft_spi_deselect(SYN_SoftSPI *spi);

#ifdef __cplusplus
}
#endif
#endif // SYN_SOFT_SPI_H
