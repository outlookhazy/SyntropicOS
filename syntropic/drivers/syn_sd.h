/**
 * @file syn_sd.h
 * @brief SD card SPI block driver.
 *
 * Zero-allocation driver for SD/SDHC/SDXC cards over hardware SPI.
 * Implements the SD simplified SPI spec: raw 512-byte sector read/write.
 * Uses syn_port_spi.h for all bus transfers; CS is managed by the driver.
 *
 * @code
 *   static SYN_SD sd;
 *   if (syn_sd_init(&sd, 0, MY_SD_CS_PIN) == SYN_OK) {
 *       uint8_t buf[512];
 *       syn_sd_read(&sd, 0, buf);
 *       syn_sd_write(&sd, 1, buf);
 *       syn_sd_sync(&sd);
 *   }
 * @endcode
 *
 * The SPI bus must support Mode 0 (CPOL=0, CPHA=0). When tx_buf is NULL,
 * syn_port_spi_transfer() must drive MOSI HIGH (0xFF), as required by the
 * SD simplified spec during receive phases.
 * @ingroup syn_drivers
 */

#ifndef SYN_SD_H
#define SYN_SD_H

#include "../common/syn_defs.h"
#include "../port/syn_port_spi.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Fixed sector size in bytes. All SD cards expose 512-byte sectors in SPI mode. */
#define SYN_SD_SECTOR_SIZE  512u

/**
 * @brief SD card type, detected automatically during syn_sd_init().
 */
typedef enum {
    SYN_SD_UNKNOWN = 0, /**< Not yet initialized or unrecognized card          */
    SYN_SD_SDSC    = 1, /**< Standard Capacity (<=2 GB, byte-addressed)        */
    SYN_SD_SDHC    = 2, /**< High or Extended Capacity (>2 GB, sector-addressed) */
} SYN_SD_Type;

/**
 * @brief SD card driver handle (caller-owned, zero heap allocation).
 *
 * Allocate statically and pass to syn_sd_init(). Treat as opaque after init.
 */
typedef struct {
    uint8_t       spi_bus;      /**< SPI bus index (syn_port_spi_* argument)  */
    SYN_GPIO_Pin  cs_pin;       /**< Chip-select GPIO pin                     */
    SYN_SD_Type   type;         /**< Detected card type (SDSC or SDHC)        */
    uint32_t      sector_count; /**< Total 512-byte sectors (from CSD)        */
    bool          initialized;  /**< true after a successful syn_sd_init()    */
} SYN_SD;

/**
 * @brief Initialize the SD card over SPI.
 *
 * Performs power-up clocking, CMD0/CMD8/ACMD41 init, type detection
 * via CMD58, and CSD parsing for capacity.
 *
 * @param sd       Pointer to a caller-owned SYN_SD struct.
 * @param spi_bus  SPI bus index passed to syn_port_spi_*.
 * @param cs       Chip-select GPIO pin (active-low).
 * @return SYN_OK on success, SYN_ERROR if no card or init failed.
 */
SYN_Status syn_sd_init(SYN_SD *sd, uint8_t spi_bus, SYN_GPIO_Pin cs);

/**
 * @brief Read one 512-byte sector from the SD card (CMD17).
 *
 * @param sd      Initialized SD handle.
 * @param sector  Zero-based sector index.
 * @param buf     Output buffer — caller must provide at least 512 bytes.
 * @return SYN_OK on success, SYN_ERROR on timeout or card error.
 */
SYN_Status syn_sd_read(const SYN_SD *sd, uint32_t sector, uint8_t *buf);

/**
 * @brief Write one 512-byte sector to the SD card (CMD24).
 *
 * @param sd      Initialized SD handle.
 * @param sector  Zero-based sector index.
 * @param buf     Data to write — must be exactly 512 bytes.
 * @return SYN_OK on success, SYN_ERROR on card rejection or timeout.
 */
SYN_Status syn_sd_write(const SYN_SD *sd, uint32_t sector, const uint8_t *buf);

/**
 * @brief Flush the write pipeline — wait until card is idle (CMD13).
 *
 * @param sd  Initialized SD handle.
 * @return SYN_OK if card is idle and error-free, SYN_ERROR otherwise.
 */
SYN_Status syn_sd_sync(const SYN_SD *sd);

/**
 * @brief Return total sector count parsed from the CSD register.
 *
 * @param sd  Initialized SD handle.
 * @return Number of 512-byte sectors on the card.
 */
uint32_t syn_sd_sectors(const SYN_SD *sd);

/**
 * @brief Return the detected card type.
 *
 * @param sd  Initialized SD handle.
 * @return SYN_SD_SDSC or SYN_SD_SDHC.
 */
SYN_SD_Type syn_sd_type(const SYN_SD *sd);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SD_H */
