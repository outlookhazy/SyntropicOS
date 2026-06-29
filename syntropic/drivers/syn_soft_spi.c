#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SOFT_SPI) || SYN_USE_SOFT_SPI

#include "syn_soft_spi.h"
#include "syn_gpio.h"
#include "../util/syn_assert.h"

static void spi_delay(const SYN_SoftSPI *spi) {
    for (volatile uint32_t i = 0; i < spi->delay_loops; i++) {
        // NOP loop
    }
}

void syn_soft_spi_init(SYN_SoftSPI *spi, SYN_GPIO_Pin sck, SYN_GPIO_Pin mosi, SYN_GPIO_Pin miso, SYN_SPIMode mode, uint32_t delay_loops) {
    SYN_ASSERT(spi != NULL);
    spi->sck = sck;
    spi->mosi = mosi;
    spi->miso = miso;
    spi->mode = mode;
    spi->delay_loops = delay_loops;

    syn_gpio_init(spi->sck, SYN_GPIO_OUTPUT);
    syn_gpio_init(spi->mosi, SYN_GPIO_OUTPUT);
    syn_gpio_init(spi->miso, SYN_GPIO_INPUT);

    bool cpol = (spi->mode == SYN_SPI_MODE_2 || spi->mode == SYN_SPI_MODE_3);
    spi->cpha = (spi->mode == SYN_SPI_MODE_1 || spi->mode == SYN_SPI_MODE_3);
    
    spi->idle_state = cpol ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
    spi->active_state = cpol ? SYN_GPIO_LOW : SYN_GPIO_HIGH;

    // Set idle clock state based on CPOL
    syn_gpio_write(spi->sck, spi->idle_state);
    
    syn_gpio_write(spi->mosi, SYN_GPIO_LOW);

    spi->cs_pin = (SYN_GPIO_Pin)-1; /* No CS by default */
}

uint8_t syn_soft_spi_transfer(const SYN_SoftSPI *spi, uint8_t data) {
    uint8_t rx_data = 0;
    
    SYN_GPIO_State idle = spi->idle_state;
    SYN_GPIO_State active = spi->active_state;

    for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
        if (!spi->cpha) {
            // Mode 0 or 2: Set data on idle-to-active edge
            syn_gpio_write(spi->mosi, (data & mask) ? SYN_GPIO_HIGH : SYN_GPIO_LOW);
            spi_delay(spi);
            syn_gpio_write(spi->sck, active);
            spi_delay(spi);
            if (syn_gpio_read(spi->miso) == SYN_GPIO_HIGH) {
                rx_data |= mask;
            }
            syn_gpio_write(spi->sck, idle);
            spi_delay(spi);
        } else {
            // Mode 1 or 3: Set data on active-to-idle edge
            syn_gpio_write(spi->sck, active);
            syn_gpio_write(spi->mosi, (data & mask) ? SYN_GPIO_HIGH : SYN_GPIO_LOW);
            spi_delay(spi);
            syn_gpio_write(spi->sck, idle);
            spi_delay(spi);
            if (syn_gpio_read(spi->miso) == SYN_GPIO_HIGH) {
                rx_data |= mask;
            }
            spi_delay(spi);
        }
    }
    return rx_data;
}

void syn_soft_spi_transfer_bulk(SYN_SoftSPI *spi, const uint8_t *tx, uint8_t *rx, size_t len) {
    SYN_ASSERT(spi != NULL);
    SYN_ASSERT(tx != NULL || rx != NULL);
    
    for (size_t i = 0; i < len; i++) {
        uint8_t tx_byte = tx ? tx[i] : 0xFF;
        uint8_t rx_byte = syn_soft_spi_transfer(spi, tx_byte);
        if (rx) {
            rx[i] = rx_byte;
        }
    }
}

void syn_soft_spi_set_cs(SYN_SoftSPI *spi, SYN_GPIO_Pin cs_pin, bool active_low) {
    SYN_ASSERT(spi != NULL);
    spi->cs_pin = cs_pin;
    spi->cs_active_low = active_low;
    syn_gpio_init(cs_pin, SYN_GPIO_OUTPUT);
    /* Start deasserted */
    syn_gpio_write(cs_pin, active_low ? SYN_GPIO_HIGH : SYN_GPIO_LOW);
}

void syn_soft_spi_select(SYN_SoftSPI *spi) {
    SYN_ASSERT(spi != NULL);
    if (spi->cs_pin == (SYN_GPIO_Pin)-1) return;
    syn_gpio_write(spi->cs_pin, spi->cs_active_low ? SYN_GPIO_LOW : SYN_GPIO_HIGH);
}

void syn_soft_spi_deselect(SYN_SoftSPI *spi) {
    SYN_ASSERT(spi != NULL);
    if (spi->cs_pin == (SYN_GPIO_Pin)-1) return;
    syn_gpio_write(spi->cs_pin, spi->cs_active_low ? SYN_GPIO_HIGH : SYN_GPIO_LOW);
}

#endif /* SYN_USE_SOFT_SPI */
