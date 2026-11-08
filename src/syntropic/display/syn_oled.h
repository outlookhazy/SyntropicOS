/**
 * @file syn_oled.h
 * @brief Generic Monochrome OLED Display Driver (SSD1306, SH1106, SSD1309 over I2C).
 * @ingroup syn_display
 */

#ifndef SYN_OLED_H
#define SYN_OLED_H

#include "../common/syn_defs.h"
#include "../display/syn_canvas.h"
#include "../drivers/syn_soft_i2c.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief OLED Controller IC Type.
 */
typedef enum {
    SYN_OLED_SSD1306 = 0, /**< Standard SSD1306 controller (128x64, 128x32) */
    SYN_OLED_SH1106 = 1,  /**< SH1106 controller with 2-pixel column shift */
    SYN_OLED_SSD1309 = 2  /**< SSD1309 controller */
} SYN_OLEDType;

/**
 * @brief Generic OLED Display Instance Context.
 */
/**
 * @brief Generic OLED Display Instance Context.
 */
typedef struct {
    SYN_OLEDType type;     /**< Controller IC type enum */
    SYN_SoftI2C i2c;       /**< Software I2C bus context */
    uint8_t i2c_addr;      /**< 7-bit I2C device address */
    uint16_t width;        /**< Display width in pixels */
    uint16_t height;       /**< Display height in pixels */
    uint16_t flush_offset; /**< Incremental byte cursor for partial canvas flush */
    uint8_t col_offset;    /**< Column start offset (0 for SSD1306, 2 for SH1106) */
    bool inverted;         /**< True if display colors inverted */
    bool display_on;       /**< True if display powered on */
    uint8_t contrast;      /**< Contrast setting (0 to 255) */
} SYN_OLED;

/**
 * @brief Initialize Monochrome OLED display context.
 *
 * @param oled     OLED context.
 * @param scl      I2C SCL GPIO pin.
 * @param sda      I2C SDA GPIO pin.
 * @param i2c_addr I2C slave address (e.g. 0x3C or 0x3D).
 * @param w        Width in pixels (e.g. 128).
 * @param h        Height in pixels (e.g. 64 or 32).
 * @param type     OLED IC Controller type (SSD1306, SH1106, SSD1309).
 * @return SYN_OK on success.
 */
SYN_Status syn_oled_init(SYN_OLED *oled, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda, uint8_t i2c_addr,
                         uint16_t w, uint16_t h, SYN_OLEDType type);

/**
 * @brief Set display contrast level.
 *
 * @param oled     OLED context.
 * @param contrast Contrast level (0 to 255).
 */
void syn_oled_set_contrast(SYN_OLED *oled, uint8_t contrast);

/**
 * @brief Set display color inversion.
 *
 * @param oled   OLED context.
 * @param invert True for inverted display (black on white).
 */
void syn_oled_set_invert(SYN_OLED *oled, bool invert);

/**
 * @brief Turn display ON or OFF (power save mode).
 *
 * @param oled OLED context.
 * @param on   True for display ON.
 */
void syn_oled_set_display_on(SYN_OLED *oled, bool on);

/**
 * @brief Flush canvas pixel buffer to physical OLED display screen.
 *
 * @param oled OLED context.
 * @param c    Source Canvas context.
 */
void syn_oled_flush_canvas(SYN_OLED *oled, const SYN_Canvas *c);

/**
 * @brief Canvas flush callback matching SYN_Canvas_FlushFn signature.
 *
 * @param buf Framebuffer byte array.
 * @param len Byte array length.
 * @param ctx Context pointer (SYN_OLED instance).
 */
void syn_oled_canvas_flush_cb(const uint8_t *buf, size_t len, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_OLED_H */
